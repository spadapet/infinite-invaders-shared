#include "pch.h"
#include "COM\ComAlloc.h"
#include "coreEntity\component\ComponentListener.h"

class __declspec(uuid("90fae27c-6320-4eae-93c5-b9931fed0cb2"))
	CProxyComponentListener : public ff::ComBase, public IProxyComponentListener
{
public:
	DECLARE_HEADER(CProxyComponentListener);

	// IProxyComponentListener
	virtual void SetOwner(IComponentListener *pOwner) override;

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

private:
	IComponentListener *_pOwner;
};

BEGIN_INTERFACES(CProxyComponentListener)
	HAS_INTERFACE(IComponentListener)
	HAS_INTERFACE(IProxyComponentListener)
END_INTERFACES()

bool CreateProxyComponentListener(IComponentListener *pListener, IProxyComponentListener **ppProxy)
{
	assertRetVal(pListener && ppProxy, false);

	ff::ComPtr<CProxyComponentListener> pProxy;
	assertRetVal(SUCCEEDED(ff::ComAllocator<CProxyComponentListener>::CreateInstance(&pProxy)), false);
	pProxy->SetOwner(pListener);

	*ppProxy = ff::GetAddRef(pProxy.Interface());
	return true;
}

CProxyComponentListener::CProxyComponentListener()
	: _pOwner(nullptr)
{
}

CProxyComponentListener::~CProxyComponentListener()
{
	assert(!_pOwner);
}

void CProxyComponentListener::SetOwner(IComponentListener *pOwner)
{
	_pOwner = pOwner;
}

void CProxyComponentListener::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	if (_pOwner)
	{
		_pOwner->OnAddComponent(entity, compId, pComp);
	}
}

void CProxyComponentListener::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	if (_pOwner)
	{
		_pOwner->OnRemoveComponent(entity, compId, pComp);
	}
}

