#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\entity\EntityComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "components\core\PositionComponent.h"

namespace ff
{
	class I2dRenderer;
	class IInputMapping;
}

class I2dLayerGroup;
class IEntity;

class __declspec(uuid("789c2c96-2221-4613-bf78-bfbaf4e12262"))
	CollisionAdvanceRender
		: public ff::ComBase
		, public IAdvanceComponent
		, public I2dRenderComponent
		, public IEntityEventListener
		, public IComponentListener
{
public:
	DECLARE_HEADER(CollisionAdvanceRender);

	typedef std::tuple<IEntity*, IEntity*> CollisionPair;

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	typedef EntityComponent<IPositionComponent> EntityPosition;

	void AdvanceInput(IEntity *entity);
	void AddCells(EntityPosition entityPos, const ff::RectInt &cellRect);
	void RemoveCells(EntityPosition entityPos);
	void GetCellRect(const ff::RectFloat &rect, ff::RectInt &cellRect);

	void DetectCollisions();
	void TriggerCollisionEvents();
	void HandleCollision(IEntity *entity, IEntity *pOther);

	ComponentListener<IPositionComponent> _positionListener;
	EntityEventListener _positionChangedListener;
	EntityEventListener _collisionListener;
	ff::Map<ff::PointShort, EntityPosition> _cellToEntity;
	ff::Map<EntityPosition, ff::RectInt> _entityToCells;
	ff::Set<CollisionPair> _collisions;
	ff::ComPtr<ff::IInputMapping> _inputMap;
	bool _renderGrid;
};
