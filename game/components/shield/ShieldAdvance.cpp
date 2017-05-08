#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "components\bullet\BulletComponent.h"
#include "components\invader\InvaderComponent.h"
#include "components\shield\ShieldAdvance.h"
#include "components\core\PositionComponent.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "entities\EntityEvents.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteList.h"
#include "Graph\2D\SpritePos.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"ShieldAdvance");
	module.RegisterClassT<ShieldAdvance>(name);
});

BEGIN_INTERFACES(ShieldAdvance)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(IComponentListener)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

ShieldAdvance::ShieldAdvance()
	: _loadedSprites(false)
{
}

ShieldAdvance::~ShieldAdvance()
{
}

HRESULT ShieldAdvance::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	_shieldListener.Init(pDomainProvider->GetDomain(), this);

	_sprites.Init(L"Sprites");

	return __super::_Construct(unkOuter);
}

int ShieldAdvance::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void ShieldAdvance::Advance(IEntity *entity)
{
}

void ShieldAdvance::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<ShieldComponent> pShield;
	assertRet(pShield.QueryFrom(pComp));

	_entities.Push(EntityShield(entity, pShield));

	IEntityEventListeners::AddListener(entity, this);
}

void ShieldAdvance::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<ShieldComponent> pShield;
	assertRet(pShield.QueryFrom(pComp));

	size_t i = _entities.Find(EntityShield(entity, pShield));
	assertRet(i != ff::INVALID_SIZE);

	_entities[i] = _entities.GetLast();
	_entities.Pop();

	IEntityEventListeners::RemoveListener(entity, this);
}

void ShieldAdvance::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;
		HandleCollision(entity, eventArgs2._pOther);
	}
}

void ShieldAdvance::HandleCollision(IEntity *entity, IEntity *otherEntity)
{
	ShieldComponent* pShield = entity->GetComponent<ShieldComponent>();
	BulletComponent* pBullet = otherEntity->GetComponent<BulletComponent>();
	InvaderComponent* pInvader = otherEntity->GetComponent<InvaderComponent>();

	if (pShield && (pBullet || pInvader))
	{
		if (pShield->HitTest(entity, otherEntity))
		{
			LoadSprites();

			IPositionComponent* pOtherPos = otherEntity->GetComponent<IPositionComponent>();

			if (pBullet)
			{
				ff::SpritePos spritePos = ff::GetIdentitySpritePos();
				spritePos._translate = pOtherPos->GetPos();
				spritePos._rotate = pOtherPos->GetVelocityAngle();

				pShield->Erase(entity, otherEntity, _bulletMasks[pBullet->GetType()], spritePos);

				ff::ComPtr<EntityFactoryService> pFactory;
				if (GetService(entity, &pFactory))
				{
					pFactory->CreateShieldExplosion(pShield->GetType(), otherEntity);
				}

				otherEntity->TriggerEvent(ENTITY_EVENT_DIED);
			}
			else
			{
				pShield->Erase(entity, otherEntity, nullptr, ff::GetIdentitySpritePos());
			}

			BulletEventArgs eventArgs;
			eventArgs._pBullet = otherEntity;
			eventArgs._pSource = entity;

			entity->TriggerEvent(ENTITY_EVENT_BULLET_HIT, &eventArgs);
		}
	}
}

void ShieldAdvance::LoadSprites()
{
	if (!_loadedSprites)
	{
		_loadedSprites = true;

		ff::ISpriteList *sprites = _sprites.Flush();
		assertRet(sprites);

		_bulletMasks[BULLET_TYPE_PLAYER_0] = sprites->Get(ff::String(L"Player Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_PLAYER_1] = sprites->Get(ff::String(L"Player Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_HOMING_PLAYER_0] = sprites->Get(ff::String(L"Player Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_HOMING_PLAYER_1] = sprites->Get(ff::String(L"Player Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_FAST_PLAYER_0] = sprites->Get(ff::String(L"Player Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_FAST_PLAYER_1] = sprites->Get(ff::String(L"Player Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_SPREAD_PLAYER_0] = sprites->Get(ff::String(L"Player Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_SPREAD_PLAYER_1] = sprites->Get(ff::String(L"Player Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_PUSH_PLAYER_0] = sprites->Get(ff::String(L"Invader Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_PUSH_PLAYER_1] = sprites->Get(ff::String(L"Invader Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_INVADER_LARGE] = sprites->Get(ff::String(L"Invader Bullet Mask 0"));
		_bulletMasks[BULLET_TYPE_INVADER_SMALL] = sprites->Get(ff::String(L"Invader Bullet Mask 1"));
		_bulletMasks[BULLET_TYPE_INVADER_LOSE_GAME] = sprites->Get(ff::String(L"Invader Bullet Mask 0"));

		for (size_t i = 0; i < _countof(_bulletMasks); i++)
		{
			assert(_bulletMasks[i]);
		}
	}
}
