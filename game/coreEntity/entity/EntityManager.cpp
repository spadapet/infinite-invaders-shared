#include "pch.h"
#include "COM\ComAlloc.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityEvents.h"
#include "coreEntity\entity\EntityManager.h"
#include "String\StringCache.h"

class __declspec(uuid("228b761b-8cb9-4b6e-bdc6-625624d065b9"))
	EntityManager : public ff::ComBase, public IEntityManager
{
public:
	DECLARE_HEADER(EntityManager);

	bool Init(IEntityDomain *pDomain);

	// IEntityManager

	virtual bool CreateEntity(IEntity** pentity) override;
	virtual void TriggerEvent(ff::hash_t eventName, void *eventArgs) override;

	virtual bool AddListener(ff::hash_t eventName, IEntityEventListener *pListener) override;
	virtual bool AddProxyListener(ff::hash_t eventName, IEntityEventListener *pListener, IProxyEntityEventListener **ppProxy) override;
	virtual bool RemoveListener(ff::hash_t eventName, IEntityEventListener *pListener) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

private:
	typedef ff::SharedObject<ff::Vector<ff::ComPtr<IEntityEventListener>>> EventListeners;
	typedef ff::SmartPtr<EventListeners> EventListenersPtr;

	ff::Mutex _cs;
	IEntityDomain* _domain;
	ff::ComPtr<IComponentManager> _pComponentManager;
	ff::Map<ff::hash_t, EventListenersPtr> _listeners;
	ff::ComPtr<IEntity> _pGlobalEntity;
};

BEGIN_INTERFACES(EntityManager)
	HAS_INTERFACE(IEntityManager)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

bool CreateEntityManager(IEntityDomain *pDomain, IEntityManager **ppMan)
{
	assertRetVal(ppMan, false);
	*ppMan = nullptr;

	ff::ComPtr<EntityManager> pMan;
	assertRetVal(SUCCEEDED(ff::ComAllocator<EntityManager>::CreateInstance(&pMan)), false);
	assertRetVal(pMan->Init(pDomain), false);

	*ppMan = ff::GetAddRef(pMan.Interface());
	return true;
}

const ff::hash_t ENTITY_EVENT_NULL = ff::HashFunc(L"Entity.Null");
const ff::hash_t ENTITY_EVENT_DESTROY = ff::HashFunc(L"Entity.Destroy");
const ff::hash_t ENTITY_EVENT_ADD_COMPONENTS = ff::HashFunc(L"Entity.AddComponents");
const ff::hash_t ENTITY_EVENT_REMOVE_COMPONENTS = ff::HashFunc(L"Entity.RemoveComponents");
const ff::hash_t ENTITY_EVENT_ADD_COMPONENT = ff::HashFunc(L"Entity.AddComponent");
const ff::hash_t ENTITY_EVENT_REMOVE_COMPONENT = ff::HashFunc(L"Entity.RemoveComponent");

EntityManager::EntityManager()
	: _domain(nullptr)
{
}

EntityManager::~EntityManager()
{
}

bool EntityManager::Init(IEntityDomain *pDomain)
{
	assertRetVal(pDomain, false);
	_domain = pDomain;

	assertRetVal(CreateEntity(&_pGlobalEntity), false);

	return true;
}

// From Entity.cpp:
bool InternalCreateEntity(IEntityDomain *pDomain, IEntity **pentity);

bool EntityManager::CreateEntity(IEntity **pentity)
{
	return InternalCreateEntity(_domain, pentity);
}

void EntityManager::TriggerEvent(ff::hash_t eventName, void *eventArgs)
{
	OnEntityEvent(nullptr, eventName, eventArgs);
}

bool EntityManager::AddListener(ff::hash_t eventName, IEntityEventListener *pListener)
{
	assertRetVal(pListener, false);

	ff::BucketIter i = _listeners.Get(eventName);
	if (i == ff::INVALID_ITER)
	{
		EventListenersPtr listeners;
		EventListeners::GetUnshared(listeners.Address());
		i = _listeners.SetKey(eventName, listeners);
	}

	EventListenersPtr &listeners = _listeners.ValueAt(i);
	listeners->Push(pListener);

	return true;
}

bool EntityManager::AddProxyListener(ff::hash_t eventName, IEntityEventListener *pListener, IProxyEntityEventListener **ppProxy)
{
	assertRetVal(CreateProxyEntityEventListener(pListener, ppProxy), false);
	AddListener(eventName, *ppProxy);

	return true;
}

bool EntityManager::RemoveListener(ff::hash_t eventName, IEntityEventListener *pListener)
{
	assertRetVal(pListener, false);

	ff::BucketIter i = _listeners.Get(eventName);
	assertRetVal(i != ff::INVALID_ITER, false);

	EventListenersPtr &listeners = _listeners.ValueAt(i);
	EventListeners::GetUnshared(listeners.Address());

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

void EntityManager::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	ff::BucketIter iter = _listeners.Get(eventName);

	if (iter != ff::INVALID_ITER)
	{
		EventListenersPtr &listeners = _listeners.ValueAt(iter);
		EventListeners::GetUnshared(listeners.Address());
		EventListenersPtr listeners2 = listeners;

		for (size_t i = 0; i < listeners2->Size(); i++)
		{
			listeners2->GetAt(i)->OnEntityEvent(entity, eventName, eventArgs);
		}
	}
}
