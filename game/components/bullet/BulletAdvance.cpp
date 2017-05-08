#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "components\bullet\BulletAdvance.h"
#include "components\player\PlayerComponent.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteAnimationAdvance.h"
#include "components\graph\SpriteAnimationRender.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityListener.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"
#include "services\InvaderService.h"
#include "services\PlayerService.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"BulletAdvance");
	module.RegisterClassT<BulletAdvance>(name);
});

BEGIN_INTERFACES(BulletAdvance)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(IComponentListener)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

BulletAdvance::BulletAdvance()
	: _difficulty(DIFFICULTY_NORMAL)
{
}

BulletAdvance::~BulletAdvance()
{
}

HRESULT BulletAdvance::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	IEntityDomain *pDomain = pDomainProvider->GetDomain();
	_difficulty = Globals::GetDifficulty(pDomain);

	_bulletListener.Init(pDomain, this);
	_completeListener.Init(pDomain, ENTITY_EVENT_LEVEL_COMPLETE, this);

	return __super::_Construct(unkOuter);
}

int BulletAdvance::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void BulletAdvance::Advance(IEntity *entity)
{
	ff::Vector<ff::ComPtr<IEntity>, 16> removedBullets;

	ff::ComPtr<IPlayerService> pPlayers;
	GetService(entity, &pPlayers);

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityBullet& eb = _entities[i];

		if (eb._component->IsInvader())
		{
			UpdateVelocityInvader(eb, pPlayers);
		}
		else if (eb._component->IsPlayer())
		{
			UpdateVelocityPlayer(eb);
		}

		eb._pPosComp->SetPos(eb._pPosInfo->_translate + eb._pPosInfo->_velocity);

		if (Globals::GetLevelPlayableRect().DoesTouch(eb._pPosInfo->_bounds))
		{
			ff::ComPtr<EntityFactoryService> pFactory;
			if (GetService(entity, &pFactory))
			{
				pFactory->CreateBulletSmoke(eb._entity);
			}
		}
		else
		{
			removedBullets.Push(eb._entity);

			eb._entity->TriggerEvent(ENTITY_EVENT_DIED);
		}
	}
}

void BulletAdvance::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<BulletComponent> pBullet;
	assertRet(pBullet.QueryFrom(pComp));

	_entities.Push(EntityBullet(entity, pBullet, true));

	IEntityEventListeners::AddListener(entity, this);
}

void BulletAdvance::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<BulletComponent> pBullet;
	assertRet(pBullet.QueryFrom(pComp));

	size_t i = _entities.Find(EntityBullet(entity, pBullet, false));
	assertRet(i != ff::INVALID_SIZE);

	_entities[i] = _entities.GetLast();
	_entities.Pop();

	IEntityEventListeners::RemoveListener(entity, this);
}

void BulletAdvance::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;

		HandleCollision(entity, eventArgs2._pOther);
	}
	else if (eventName == ENTITY_EVENT_LEVEL_COMPLETE)
	{
		HandleLevelComplete(entity);
	}
}

void BulletAdvance::HandleCollision(IEntity *entity, IEntity *otherEntity)
{
	BulletComponent* pBullet = entity->GetComponent<BulletComponent>();
	BulletComponent* pOtherBullet = otherEntity->GetComponent<BulletComponent>();

	if (pBullet && pBullet->CanHitInvader(entity) &&
		pOtherBullet && !pOtherBullet->IsPlayer())
	{
		BulletEventArgs eventArgs;
		eventArgs._pBullet = entity;
		eventArgs._pSource = otherEntity;

		otherEntity->TriggerEvent(ENTITY_EVENT_BULLET_HIT, &eventArgs);
		entity->TriggerEvent(ENTITY_EVENT_DIED);
		otherEntity->TriggerEvent(ENTITY_EVENT_DIED);

		ff::ComPtr<EntityFactoryService> pFactory;
		if (GetService(entity, &pFactory))
		{
			pFactory->CreateBulletExplosion(otherEntity);
		}

		if (pBullet->IsPlayer() && pOtherBullet->IsInvader())
		{
			ScoreEventArgs eventArgs;
			eventArgs._nPlayer = pBullet->GetPlayer();
			eventArgs._nAddScore = Globals::GetBulletPoints(pOtherBullet->GetType());

			entity->GetDomain()->GetEntityManager()->TriggerEvent(ENTITY_EVENT_ADD_SCORE, &eventArgs);

			DirectX::XMFLOAT4 color(1, 1, 1, 0.375f);
			pFactory->CreatePoints(entity, eventArgs._nAddScore, 4, ff::PointFloat(0, -25), &color);
		}
	}
}

void BulletAdvance::HandleLevelComplete(IEntity *entity)
{
	for (size_t i = ff::PreviousSize(_entities.Size()); i != ff::INVALID_SIZE; i = ff::PreviousSize(i))
	{
		if (_entities[i]._component->IsInvader())
		{
			_entities[i]._entity->TriggerEvent(ENTITY_EVENT_DIED);
		}
	}
}

void BulletAdvance::UpdateVelocityInvader(const EntityBullet &eb, IPlayerService *pPlayers)
{
	bool bDeflected = false;

	if (pPlayers)
	{
		for (size_t i = 0; i < pPlayers->GetPlayerCount(); i++)
		{
			PlayerComponent *pPlayer = pPlayers->GetPlayer(i)->GetComponent<PlayerComponent>();

			if (eb._component->CanDeflect() && pPlayer->HasPowerup(POWERUP_TYPE_SHIELD))
			{
				IPositionComponent* pPlayerPos = pPlayers->GetPlayer(i)->GetComponent<IPositionComponent>();
				ff::PointFloat diff = eb._pPosInfo->_translate - pPlayerPos->GetPos();

				if (diff.y < -50.0f) // bullet can't be too low (below the shield)
				{
					const float maxDist2 = 150.0f * 150.0f;
					const float minDist2 = 25.0f * 25.0f;

					float dist2 = diff.x * diff.x + diff.y * diff.y;

					if (dist2 >= minDist2 && dist2 < maxDist2)
					{
						RotateTowardsAngle(eb, ff::PI_F / 2, 0.8f, pPlayerPos, false);
						bDeflected = true;
					}
				}
			}
		}
	}

	if (!bDeflected && pPlayers && eb._component->IsInvaderHoming(_difficulty))
	{
		bool bLoseGame = (eb._component->GetType() == BULLET_TYPE_INVADER_LOSE_GAME);
		IPositionComponent* pBestPlayerPos = nullptr;
		ff::PointFloat bestDiff(0, 0);

		for (size_t i = 0; i < pPlayers->GetPlayerCount(); i++)
		{
			if (pPlayers->IsPlayerAlive(i))
			{
				IPositionComponent* pPlayerPos = pPlayers->GetPlayer(i)->GetComponent<IPositionComponent>();
				ff::PointFloat diff = eb._pPosInfo->_translate - pPlayerPos->GetPos();

				if ((bLoseGame || diff.y < -50.0f) &&
					(bestDiff.y == 0 || fabs(bestDiff.x) > fabs(diff.x)))
				{
					bestDiff = diff;
					pBestPlayerPos = pPlayerPos;
				}
			}
		}

		if (pBestPlayerPos)
		{
			float strength = bLoseGame ? 1.0f : 0.0f;
			RotateTowardsAngle(eb, atan2(-bestDiff.y, bestDiff.x), strength, pBestPlayerPos, true);
		}
	}
}

void BulletAdvance::UpdateVelocityPlayer(const EntityBullet &eb)
{
	ff::ComPtr<IInvaderService> pInvaders;

	if (eb._component->IsPlayerHoming() && GetService(eb._entity, &pInvaders))
	{
		ff::PointFloat bulletPos = eb._pPosInfo->_translate;

		if (bulletPos.y < Globals::GetShieldRect(0).Center().y)
		{
			ff::PointFloat bestDiff(0, 0);
			float bestDist = 0;

			for (size_t i = 0; i < pInvaders->GetInvaderCount(); i++)
			{
				IEntity* pInvaderEntity = pInvaders->GetInvader(i);
				IPositionComponent* pInvaderPos = pInvaderEntity->GetComponent<IPositionComponent>();
				ff::PointFloat diff = pInvaderPos->GetPos() - bulletPos;
				float dist = diff.x * diff.x * 2 + diff.y * diff.y;

				if (!bestDist || dist < bestDist)
				{
					bestDiff = diff;
					bestDist = dist;
				}
			}

			if (bestDist)
			{
				RotateTowardsAngle(eb, atan2(-bestDiff.y, bestDiff.x), 0.75f);
			}
		}
	}
}

void BulletAdvance::RotateTowardsAngle(
	const EntityBullet& eb,
	float destAngle,
	float strength,
	IPositionComponent* pPlayerPos,
	bool bTowardsPlayer)
{
	assertRet(strength >= 0 && strength <= 1);
	strength = 24.0f - 23.0f * strength;

	if (pPlayerPos && bTowardsPlayer)
	{
		strength *= 50;
	}

	float speed = eb._pPosComp->GetVelocityMagnitude();
	float angle = eb._pPosComp->GetVelocityAngle();
	ff::PointFloat newVel;

	if (fabs(destAngle - angle) > ff::PI_F)
	{
		destAngle += (destAngle < angle)
			? 2 * ff::PI_F
			: -2 * ff::PI_F;
	}

	if (pPlayerPos)
	{
		float diffAngle1 = destAngle - angle;
		float diffAngle2 = (diffAngle1 > 0) ? (-2 * ff::PI_F + diffAngle1) : (2 * ff::PI_F + diffAngle1);

		float newAngle1 = angle + diffAngle1 / strength;
		float newAngle2 = angle + diffAngle2 / strength;

		ff::PointFloat newVel1(cos(newAngle1) * speed, sin(newAngle1) * -speed);
		ff::PointFloat newVel2(cos(newAngle2) * speed, sin(newAngle2) * -speed);

		ff::PointFloat newPos1 = eb._pPosInfo->_translate + newVel1;
		ff::PointFloat newPos2 = eb._pPosInfo->_translate + newVel2;

		ff::PointFloat diff1 = pPlayerPos->GetPos() - newPos1;
		ff::PointFloat diff2 = pPlayerPos->GetPos() - newPos2;

		float dist1 = diff1.x * diff1.x + diff1.y * diff1.y;
		float dist2 = diff2.x * diff2.x + diff2.y * diff2.y;

		newVel = (dist1 > dist2)
			? (bTowardsPlayer ? newVel2 : newVel1)
			: (bTowardsPlayer ? newVel1 : newVel2);
	}
	else
	{
		float newAngle = angle + (destAngle - angle) / strength;
		newVel.SetPoint(cos(newAngle) * speed, sin(newAngle) * -speed);
	}

	eb._pPosComp->SetVelocity(newVel);
}
