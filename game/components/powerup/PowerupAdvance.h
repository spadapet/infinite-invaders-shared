#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "components\powerup\PowerupComponent.h"
#include "entities\EntityPosComp.h"

class IEntity;

class __declspec(uuid("1e89b533-0655-4d8f-b602-f4a13c124381"))
	PowerupAdvance
		: public ff::ComBase
		, public IAdvanceComponent
		, public IComponentListener
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(PowerupAdvance);

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	void HandleCollision(IEntity *entity, IEntity *otherEntity);

	typedef EntityPosComp<PowerupComponent> EntityPowerup;

	ComponentListener<PowerupComponent> _powerupListener;
	ff::Vector<EntityPowerup> _entities;
};
