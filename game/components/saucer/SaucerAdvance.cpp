#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "COM\ServiceCollection.h"
#include "components\bullet\BulletComponent.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteAnimationAdvance.h"
#include "components\graph\SpriteAnimationRender.h"
#include "components\powerup\PowerupComponent.h"
#include "components\saucer\SaucerAdvance.h"
#include "components\saucer\SaucerComponent.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityEvents.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"
#include "Graph\Font\SpriteFont.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"
#include "services\GlobalPlayerService.h"
#include "services\InvaderService.h"
#include "services\PlayerService.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"SaucerAdvance");
	module.RegisterClassT<SaucerAdvance>(name);
});

BEGIN_INTERFACES(SaucerAdvance)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(IComponentListener)
	HAS_INTERFACE(IEntityEventListener)
	HAS_INTERFACE(ISaucerService)
END_INTERFACES()

SaucerAdvance::SaucerAdvance()
	: _countdown(0)
{
}

SaucerAdvance::~SaucerAdvance()
{
}

HRESULT SaucerAdvance::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	ThisApplication* pApp = ThisApplication::Get(pDomainProvider);
	IEntityDomain* pDomain = pDomainProvider->GetDomain();

	if (Globals::IsBonusLevel(Globals::GetDifficulty(pDomain), Globals::GetCurrentLevel(pDomain)))
	{
		_countdown = ff::INVALID_SIZE;
	}

	_saucerListener.Init(pDomain, this);

	_font.Init(L"Classic");

	// OK for this to fail if there are no players
	GetService(pDomain, &_players);
	GetService(pDomain, &_invaders);

	pDomain->GetServices()->AddService(__uuidof(ISaucerService), static_cast<ISaucerService*>(this));

	return __super::_Construct(unkOuter);
}

int SaucerAdvance::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

#ifdef _DEBUG
static size_t s_nNextSaucerType = SAUCER_TYPE_SHIELD;
#endif

void SaucerAdvance::Advance(IEntity *entity)
{
	if (CanCountDown() && !_countdown)
	{
		if (!_entities.Size())
		{
#ifdef _DEBUG
			_countdown = (size_t)rand() % 5 + 5;
#else
			size_t nMin = Globals::GetSaucerMinSeconds();
			size_t nMax = Globals::GetSaucerMaxSeconds();

			if (nMax > nMin)
			{
				_countdown = (size_t)rand() % (nMax - nMin) + nMin;
			}
			else
			{
				_countdown = nMin;
			}
#endif
			_countdown *= 60;
		}
	}
	else if (CanCountDown() && !--_countdown)
	{
		ff::ComPtr<EntityFactoryService> pFactory;

		if (GetService(entity, &pFactory))
		{
			SaucerEntityType type = SAUCER_TYPE_NONE;
			Difficulty diff = Globals::GetDifficulty(entity);

			if (!Globals::IsHardDifficulty(diff) || (rand() % 5))
			{
#ifdef _DEBUG
				type = (SaucerEntityType)(s_nNextSaucerType++ % SAUCER_TYPE_NONE);
#else
				type = (SaucerEntityType)(rand() % SAUCER_TYPE_NONE);
#endif
			}

			ff::PointFloat pos(0, Globals::GetSaucerRect(type).Center().y);
			ff::PointFloat vel = Globals::GetSaucerSpeed(type, diff);

			if (rand() % 2)
			{
				pos.x = Globals::GetSaucerRect(type).left;
			}
			else
			{
				pos.x = Globals::GetSaucerRect(type).right;
				vel.x = -vel.x;
			}

			ff::ComPtr<IEntity> pSaucerEntity;
			pFactory->CreateSaucerEntity(type, pos, vel, &pSaucerEntity);
		}
	}

	ff::Vector<ff::ComPtr<IEntity>, 16> removedSaucers;

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntitySaucer& es = _entities[i];
		SaucerEntityType type = es._component->GetType();

		es._pPosComp->SetPos(es._pPosInfo->_translate + es._pPosInfo->_velocity);

		ff::RectFloat saucerLimits = Globals::GetSaucerRect(type);
		saucerLimits.top = Globals::GetLevelRectF().top;

		if (!saucerLimits.PointInRect(es._pPosInfo->_translate))
		{
			removedSaucers.Push(es._entity);

			es._entity->TriggerEvent(ENTITY_EVENT_DIED);
		}
	}
}

void SaucerAdvance::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<SaucerComponent> pSaucer;
	assertRet(pSaucer.QueryFrom(pComp));

	_entities.Push(EntitySaucer(entity, pSaucer, true));

	IEntityEventListeners::AddListener(entity, this);
}

void SaucerAdvance::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<SaucerComponent> pSaucer;
	assertRet(pSaucer.QueryFrom(pComp));

	size_t i = _entities.Find(EntitySaucer(entity, pSaucer, false));
	assertRet(i != ff::INVALID_SIZE);

	_entities[i] = _entities.GetLast();
	_entities.Pop();

	IEntityEventListeners::RemoveListener(entity, this);
}

void SaucerAdvance::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;

		HandleCollision(entity, eventArgs2._pOther);
	}
	else if (eventName == ENTITY_EVENT_DIED || eventName == ENTITY_EVENT_DESTROY)
	{
		eventName = eventName;
	}
}

size_t SaucerAdvance::GetSaucerCount() const
{
	return _entities.Size();
}

IEntity *SaucerAdvance::GetSaucer(size_t index) const
{
	assertRetVal(index >= 0 && index < _entities.Size(), nullptr);

	return _entities[index]._entity;
}

void SaucerAdvance::HandleCollision(IEntity *entity, IEntity *otherEntity)
{
	BulletComponent* pBullet = otherEntity->GetComponent<BulletComponent>();
	SaucerComponent* pSaucer = entity->GetComponent<SaucerComponent>();
	assertRet(pSaucer);

	if (pBullet && pBullet->IsPlayer() && pSaucer->IsBonus())
	{
		IPositionComponent* pSaucerPos = entity->GetComponent<IPositionComponent>();

		const float bonusLevel = (float)Globals::GetSaucerBonusLevel(Globals::GetCurrentLevel(entity));
		const float deltaY = 10.0f + std::min(50.0f, bonusLevel * 12);
		const float minY = Globals::GetLevelPlayableRect().top + Globals::GetSaucerRect(pSaucer->GetType()).Height() / 2;
		const float scale = 0.875f;
		const size_t nPoints = Globals::GetSaucerBonusPoints(pSaucer->GetHitCount());
		ff::PointFloat newPos = pSaucerPos->GetPos();
		ff::PointFloat newVel = pSaucerPos->GetVelocity() * ff::PointFloat(-1, 0);

		// Move up and turn around

		newPos.y = std::max(newPos.y - deltaY, minY);

		pSaucerPos->SetPos(newPos);
		pSaucerPos->SetVelocity(newVel);
		pSaucerPos->SetScale(pSaucerPos->GetScale() * ff::PointFloat(-scale, scale));

		// Change speed

		pSaucer->AddHitCount();

		if (pSaucer->GetHitCount() == 1)
		{
			bool bRight = pSaucerPos->GetVelocity().x >= 0;
			Difficulty diff = Globals::GetDifficulty(entity);

			pSaucer->SetType(SAUCER_TYPE_BONUS_LEVEL_FAST);
			pSaucerPos->SetVelocity(Globals::GetSaucerSpeed(SAUCER_TYPE_BONUS_LEVEL_FAST, diff) * (bRight ? 1.0f : -1.0f));
		}

		// Add points
		{
			ScoreEventArgs eventArgs;
			eventArgs._nPlayer = pBullet->GetPlayer();
			eventArgs._nAddScore = nPoints;

			entity->GetDomain()->GetEntityManager()->TriggerEvent(ENTITY_EVENT_ADD_SCORE, &eventArgs);

			ff::ComPtr<EntityFactoryService> pFactory;
			if (GetService(entity, &pFactory))
			{
				pFactory->CreatePoints(entity, nPoints);
			}
		}

		// Kill bullet and maybe saucer
		{
			BulletEventArgs eventArgs;
			eventArgs._pBullet = otherEntity;
			eventArgs._pSource = entity;

			entity->TriggerEvent(ENTITY_EVENT_BULLET_HIT, &eventArgs);
			otherEntity->TriggerEvent(ENTITY_EVENT_DIED);

			if (pSaucer->GetHitCount() >= 24)
			{
				entity->TriggerEvent(ENTITY_EVENT_DIED);
			}
		}
	}
	else if (pBullet && pBullet->CanHitInvader(otherEntity))
	{
		ff::ComPtr<EntityFactoryService> pFactory;
		if (GetService(entity, &pFactory))
		{
			pFactory->CreateSaucerExplosion(entity, otherEntity);
		}

		size_t nPoints = 100;
		IPositionComponent* pSaucerPos = entity->GetComponent<IPositionComponent>();
		IPositionComponent* pBulletPos = otherEntity->GetComponent<IPositionComponent>();

		// Add score
		if (pBullet->IsPlayer())
		{
			size_t nDist = (size_t)fabs(pSaucerPos->GetPos().x - pBulletPos->GetPos().x);

			if (nDist < 70)
			{
				nPoints = (70 - nDist) * 900 / 70 + 100;
				nPoints = (nPoints + 5) / 10 * 10;
			}

			ScoreEventArgs eventArgs;
			eventArgs._nPlayer = pBullet->GetPlayer();
			eventArgs._nAddScore = nPoints;

			entity->GetDomain()->GetEntityManager()->TriggerEvent(ENTITY_EVENT_ADD_SCORE, &eventArgs);

			pFactory->CreatePoints(entity, nPoints);
		}

		// Drop powerup
		if (CanCountDown())
		{
			IPositionComponent* pPos = entity->GetComponent<IPositionComponent>();
			SaucerComponent* pSaucer = entity->GetComponent<SaucerComponent>();
			PoweruentityType type = (PoweruentityType)pSaucer->GetType();

			if (type < POWERUP_TYPE_COUNT)
			{
				ff::ComPtr<IEntity> pPoweruentity;
				size_t nPlayer = pBullet->IsPlayer() ? pBullet->GetPlayer() : ff::INVALID_SIZE;

				pFactory->CreatePowerup(type, nPlayer, pPos->GetPos(), ff::PointFloat(0, Globals::GetBonusDropSpeed()), &pPoweruentity);
			}
		}

		// Kill saucer and bullet
		{
			BulletEventArgs eventArgs;
			eventArgs._pBullet = otherEntity;
			eventArgs._pSource = entity;

			entity->TriggerEvent(ENTITY_EVENT_BULLET_HIT, &eventArgs);
			entity->TriggerEvent(ENTITY_EVENT_DIED);
			otherEntity->TriggerEvent(ENTITY_EVENT_DIED);
		}
	}
}

bool SaucerAdvance::CanCountDown() const
{
	return
		_countdown != ff::INVALID_SIZE &&
		_invaders && _invaders->GetInvaderCount() &&
		_players && _players->IsAnyPlayerAlive();
}
