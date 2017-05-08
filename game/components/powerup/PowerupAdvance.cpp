#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteAnimationRender.h"
#include "components\player\PlayerComponent.h"
#include "components\powerup\PowerupAdvance.h"
#include "components\powerup\PowerupComponent.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "entities\EntityEvents.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PowerupAdvance");
	module.RegisterClassT<PowerupAdvance>(name);
});

BEGIN_INTERFACES(PowerupAdvance)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(IComponentListener)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

PowerupAdvance::PowerupAdvance()
{
}

PowerupAdvance::~PowerupAdvance()
{
}

HRESULT PowerupAdvance::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	ThisApplication* pApp = ThisApplication::Get(pDomainProvider);
	IEntityDomain* pDomain = pDomainProvider->GetDomain();

	_powerupListener.Init(pDomain, this);

	return __super::_Construct(unkOuter);
}

int PowerupAdvance::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void PowerupAdvance::Advance(IEntity *entity)
{
	ff::Vector<ff::ComPtr<IEntity>, 16> removed;

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityPowerup& ep = _entities[i];
		SpriteAnimationRender* render = ep._entity->GetComponent<SpriteAnimationRender>();

		ep._pPosComp->SetPos(ep._pPosInfo->_translate + ep._pPosInfo->_velocity);
		ep._pPosComp->SetRotate(render->GetFrame() * ff::PI_F);

		ff::RectFloat levelBounds = Globals::GetLevelRectF();
		levelBounds.top -= 200;

		if (!levelBounds.PointInRect(ep._pPosInfo->_translate))
		{
			removed.Push(ep._entity);

			ep._entity->TriggerEvent(ENTITY_EVENT_DIED);
		}
	}
}

void PowerupAdvance::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<PowerupComponent> pPowerup;
	assertRet(pPowerup.QueryFrom(pComp));

	_entities.Push(EntityPowerup(entity, pPowerup, true));

	IEntityEventListeners::AddListener(entity, this);
}

void PowerupAdvance::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<PowerupComponent> pPowerup;
	assertRet(pPowerup.QueryFrom(pComp));

	size_t i = _entities.Find(EntityPowerup(entity, pPowerup, false));
	assertRet(i != ff::INVALID_SIZE);

	_entities[i] = _entities.GetLast();
	_entities.Pop();

	IEntityEventListeners::RemoveListener(entity, this);
}

void PowerupAdvance::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;
		HandleCollision(entity, eventArgs2._pOther);
	}
}

void PowerupAdvance::HandleCollision(IEntity *entity, IEntity *otherEntity)
{
}
