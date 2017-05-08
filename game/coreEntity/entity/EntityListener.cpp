#include "pch.h"
#include "COM\ComAlloc.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\EntityListener.h"
#include "coreEntity\entity\EntityManager.h"

class __declspec(uuid("c2f21f27-3826-40e9-b3d9-2e5156e41579"))
	ProxyEntityEventListener : public ff::ComBase, public IProxyEntityEventListener
{
public:
	DECLARE_HEADER(ProxyEntityEventListener);

	// IProxyEntityEventListener
	virtual void SetOwner(IEntityEventListener *pOwner) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

private:
	IEntityEventListener *_pOwner;
};

BEGIN_INTERFACES(ProxyEntityEventListener)
	HAS_INTERFACE(IProxyEntityEventListener)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

bool CreateProxyEntityEventListener(IEntityEventListener *pListener, IProxyEntityEventListener **ppProxy)
{
	assertRetVal(pListener && ppProxy, false);

	ff::ComPtr<ProxyEntityEventListener> pProxy;
	assertRetVal(SUCCEEDED(ff::ComAllocator<ProxyEntityEventListener>::CreateInstance(&pProxy)), false);
	pProxy->SetOwner(pListener);

	*ppProxy = ff::GetAddRef(pProxy.Interface());

	return true;
}

ProxyEntityEventListener::ProxyEntityEventListener()
	: _pOwner(nullptr)
{
}

ProxyEntityEventListener::~ProxyEntityEventListener()
{
	assert(!_pOwner);
}

void ProxyEntityEventListener::SetOwner(IEntityEventListener *pOwner)
{
	_pOwner = pOwner;
}

void ProxyEntityEventListener::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (_pOwner)
	{
		_pOwner->OnEntityEvent(entity, eventName, eventArgs);
	}
}

EntityEventListener::EntityEventListener()
	: _domain(nullptr)
	, _listener(nullptr)
{
}

EntityEventListener::~EntityEventListener()
{
	if (_listener)
	{
		_listener->SetOwner(nullptr);
		_domain->GetEntityManager()->RemoveListener(_eventName, _listener);
	}
}

void EntityEventListener::Init(IEntityDomain *pDomain, ff::hash_t eventName, IEntityEventListener *pOwner)
{
	assertRet(!_listener && pOwner && pDomain);
	_domain = pDomain;
	_eventName = eventName;
	_domain->GetEntityManager()->AddProxyListener(_eventName, pOwner, &_listener);
}

IEntityEventListener *EntityEventListener::GetListener()
{
	return _listener;
}

IEntityDomain *EntityEventListener::GetDomain()
{
	return _domain;
}

