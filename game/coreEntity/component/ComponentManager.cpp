#include "pch.h"
#include "COM\ComAlloc.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\domain\EntityDomain.h"
#include "Globals\ProcessGlobals.h"

class __declspec(uuid("dc0136c6-9971-4a7d-9d1f-759826fad6eb"))
	ComponentManager : public ff::ComBase, public IComponentManager
{
public:
	DECLARE_HEADER(ComponentManager);

	bool Init(IEntityDomain *pDomain);

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

protected:
	// IComponentManager
	virtual bool CreateComponent (IEntity *entity, REFGUID compid, IUnknown **ppComp) override;
	virtual bool CreateComponent (REFGUID compid, REFGUID iid, IEntity *entity, void** ppComp) override;
	virtual bool AddListener(REFGUID compid, IComponentListener *pListener) override;
	virtual bool AddProxyListener(REFGUID compid, IComponentListener *pListener, IProxyComponentListener **ppProxy) override;
	virtual bool RemoveListener(REFGUID compid, IComponentListener *pListener) override;

private:
	typedef ff::SharedObject<ff::Vector<ff::ComPtr<IComponentListener>>> ComponentListeners;
	typedef ff::SmartPtr<ComponentListeners> ComponentListenersPtr;

	ff::Map<GUID, ff::ComPtr<IClassFactory>> _factories;
	ff::Map<GUID, ComponentListenersPtr> _listeners;
	IEntityDomain* _domain;
};

BEGIN_INTERFACES(ComponentManager)
	HAS_INTERFACE(IComponentManager)
	HAS_INTERFACE(IComponentListener)
END_INTERFACES()

bool CreateComponentManager(IEntityDomain *pDomain, IComponentManager **ppMan)
{
	assertRetVal(ppMan, false);
	*ppMan = nullptr;

	ff::ComPtr<ComponentManager> pMan;
	assertRetVal(SUCCEEDED(ff::ComAllocator<ComponentManager>::CreateInstance(&pMan)), false);
	assertRetVal(pMan->Init(pDomain), false);

	*ppMan = ff::GetAddRef(pMan.Interface());
	return true;
}

ComponentManager::ComponentManager()
	: _domain(nullptr)
{
}

ComponentManager::~ComponentManager()
{
}

bool ComponentManager::Init(IEntityDomain *pDomain)
{
	assertRetVal(pDomain, false);
	_domain = pDomain;

	return true;
}

bool ComponentManager::AddListener(REFGUID compid, IComponentListener *pListener)
{
	assertRetVal(pListener && compid != GUID_NULL, false);

	ff::BucketIter i = _listeners.Get(compid);
	if (i == ff::INVALID_ITER)
	{
		ComponentListenersPtr listeners;
		ComponentListeners::GetUnshared(listeners.Address());
		i = _listeners.SetKey(compid, listeners);
	}

	ComponentListenersPtr &listeners = _listeners.ValueAt(i);
	listeners->Push(pListener);

	return true;
}

bool ComponentManager::AddProxyListener(REFGUID compid, IComponentListener *pListener, IProxyComponentListener **ppProxy)
{
	assertRetVal(CreateProxyComponentListener(pListener, ppProxy), false);
	AddListener(compid, *ppProxy);

	return true;
}

bool ComponentManager::RemoveListener(REFGUID compid, IComponentListener *pListener)
{
	assertRetVal(pListener, false);

	ff::BucketIter i = _listeners.Get(compid);
	assertRetVal(i != ff::INVALID_ITER, false);

	ComponentListenersPtr &listeners = _listeners.ValueAt(i);
	ComponentListeners::GetUnshared(listeners.Address());

	for (size_t h = 0; h < listeners->Size(); h++)
	{
		if (listeners->GetAt(h) == pListener)
		{
			listeners->Delete(h);
			return true;
		}
	}

	assertRetVal(false, false);
}

void ComponentManager::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::BucketIter iter = _listeners.Get(compId);

	if (iter != ff::INVALID_ITER)
	{
		ComponentListenersPtr &listeners = _listeners.ValueAt(iter);
		ComponentListeners::GetUnshared(listeners.Address());
		ComponentListenersPtr listeners2 = listeners;

		for (size_t i = 0; i < listeners2->Size(); i++)
		{
			listeners2->GetAt(i)->OnAddComponent(entity, compId, pComp);
		}
	}
}

void ComponentManager::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::BucketIter iter = _listeners.Get(compId);

	if (iter != ff::INVALID_ITER)
	{
		ComponentListenersPtr &listeners = _listeners.ValueAt(iter);
		ComponentListeners::GetUnshared(listeners.Address());
		ComponentListenersPtr listeners2 = listeners;

		for (size_t i = 0; i < listeners2->Size(); i++)
		{
			listeners2->GetAt(i)->OnRemoveComponent(entity, compId, pComp);
		}
	}
}

bool ComponentManager::CreateComponent(IEntity *entity, REFGUID compid, IUnknown **ppComp)
{
	return CreateComponent(compid, __uuidof(IUnknown), entity, (void**)ppComp);
}

bool ComponentManager::CreateComponent(REFGUID compid, REFGUID iid, IEntity *entity, void** ppComp)
{
	ff::BucketIter i = _factories.Get(compid);

	if (i == ff::INVALID_ITER)
	{
		// See if compid is actually a class ID

		ff::ComPtr<IClassFactory> pFactory;
		if (ff::ProcessGlobals::Get()->GetModules().FindClassFactory(compid, &pFactory))
		{
			i = _factories.SetKey(compid, pFactory);
		}
	}

	if (i == ff::INVALID_ITER)
	{
		const ff::ModuleClassInfo *info = ff::ProcessGlobals::Get()->GetModules().FindClassInfoForInterface(compid);

		if (info)
		{
			ff::ComPtr<IClassFactory> pFactory;
			if (ff::ProcessGlobals::Get()->GetModules().FindClassFactory(info->_classId, &pFactory))
			{
				_factories.SetKey(info->_classId, pFactory);
				i = _factories.SetKey(compid, pFactory);
			}
		}
	}

	assertRetVal(i != ff::INVALID_ITER, false);
	assertRetVal(SUCCEEDED(_factories.ValueAt(i)->CreateInstance(
		entity ? (IUnknown*)entity : _domain, iid, (void**)ppComp)), false);

	return true;
}

