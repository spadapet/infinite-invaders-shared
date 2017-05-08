#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "components\bullet\BulletComponent.h"
#include "entities\EntityPosComp.h"

class IEntity;
class IPlayerService;
class IPositionComponent;

class __declspec(uuid("79a15be6-364c-4a7c-8a86-35baa5c63dea"))
	BulletAdvance
		: public ff::ComBase
		, public IAdvanceComponent
		, public IComponentListener
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(BulletAdvance);

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
	typedef EntityPosComp<BulletComponent> EntityBullet;

	void HandleCollision(IEntity *entity, IEntity *otherEntity);
	void HandleLevelComplete(IEntity *entity);
	void UpdateVelocityInvader(const EntityBullet &eb, IPlayerService *pPlayers);
	void UpdateVelocityPlayer(const EntityBullet &eb);
	void RotateTowardsAngle(const EntityBullet &eb, float destAngle, float strength, IPositionComponent* pPlayerPos = nullptr, bool bTowardsPlayer = false);

	ComponentListener<BulletComponent> _bulletListener;
	EntityEventListener _completeListener;
	ff::Vector<EntityBullet> _entities;
	Difficulty _difficulty;
};
