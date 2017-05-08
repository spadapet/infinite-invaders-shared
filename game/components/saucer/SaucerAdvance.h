#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "components\saucer\SaucerComponent.h"
#include "entities\EntityPosComp.h"
#include "Resource\ResourceValue.h"
#include "services\SaucerService.h"

namespace ff
{
	class ISpriteFont;
}

class IEntity;
class IInvaderService;
class IPlayerService;

class __declspec(uuid("c8fca256-6606-4f27-aec2-81a4ced55732"))
	SaucerAdvance
		: public ff::ComBase
		, public IAdvanceComponent
		, public IComponentListener
		, public IEntityEventListener
		, public ISaucerService
{
public:
	DECLARE_HEADER(SaucerAdvance);

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// ISaucerService
	virtual size_t GetSaucerCount() const override;
	virtual IEntity* GetSaucer(size_t index) const override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	void HandleCollision(IEntity *entity, IEntity *otherEntity);
	bool CanCountDown() const;

	typedef EntityPosComp<SaucerComponent> EntitySaucer;

	ComponentListener<SaucerComponent> _saucerListener;
	ff::TypedResource<ff::ISpriteFont> _font;
	ff::Vector<EntitySaucer> _entities;
	ff::ComPtr<IPlayerService> _players;
	ff::ComPtr<IInvaderService> _invaders;
	size_t _countdown;
};
