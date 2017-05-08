#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "components\bullet\\BulletComponent.h"
#include "components\invader\InvaderComponent.h"
#include "entities\EntityPosComp.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class ISpriteAnimation;
}

class IEntity;

class __declspec(uuid("cf5c3dd6-74e3-4060-acb5-9cf88d82b159"))
	InvaderTitleAdvanceRender
		: public ff::ComBase
		, public IAdvanceComponent
		, public I2dRenderComponent
		, public IEntityEventListener
		, public IComponentListener
{
public:
	DECLARE_HEADER(InvaderTitleAdvanceRender);

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
	typedef EntityPosComp<InvaderComponent> EntityInvader;

	void HandleCollision(IEntity *pInvaderEntity, IEntity *otherEntity);

	ComponentListener<InvaderComponent> _invaderListener;
	ff::Vector<EntityInvader> _invaders;
	size_t _frames;

	// Graphics
	ff::TypedResource<ff::ISpriteAnimation> _invaderAnims[INVADER_TYPE_COUNT];
};
