#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\entity\EntityComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "components\bullet\BulletComponent.h"
#include "components\shield\ShieldComponent.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class ISprite;
	class ISpriteList;
}

class IEntity;

class __declspec(uuid("a42f8b2b-f129-442b-8677-71d09b882185"))
	ShieldAdvance
		: public ff::ComBase
		, public IAdvanceComponent
		, public IComponentListener
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(ShieldAdvance);

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
	void LoadSprites();

	typedef EntityComponent<ShieldComponent> EntityShield;

	ComponentListener<ShieldComponent> _shieldListener;
	ff::Vector<EntityShield> _entities;
	ff::TypedResource<ff::ISpriteList> _sprites;
	ff::ComPtr<ff::ISprite> _bulletMasks[BULLET_TYPE_COUNT];
	bool _loadedSprites;
};
