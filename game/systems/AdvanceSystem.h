#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\system\System.h"
#include "entities\EntityAdvanceComp.h"

namespace ff
{
	class IRenderTarget;
}

class IEntity;

class __declspec(uuid("034ff93c-ae00-4f18-9c17-01d82fd2fd38"))
	AdvanceSystem
		: public ff::ComBase
		, public ISystem
		, public IComponentListener
{
public:
	DECLARE_HEADER(AdvanceSystem);

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	ComponentListener<IAdvanceComponent> _advanceListener;
	ff::Vector<EntityAdvanceComp> _entities;
	ff::Vector<EntityAdvanceComp> _pendingAdd;
	ff::Vector<EntityAdvanceComp> _pendingRemove;
};
