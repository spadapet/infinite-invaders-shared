#include "pch.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityListener.h"
#include "Module\ModuleFactory.h"

class __declspec(uuid("bcc7723f-1c8c-4f87-b84a-8f655ab88f67"))
	EntityListeners
		: public ff::ComBase
		, public IEntityEventListeners
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(EntityListeners);

	bool AddListener(IEntityEventListener *pListener);
	bool RemoveListener(IEntityEventListener *pListener);
	bool HasListener(IEntityEventListener *pListener);

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

private:
	typedef ff::SharedObject<ff::Vector<ff::ComPtr<IEntityEventListener>>> SharedListeners;
	typedef ff::SmartPtr<SharedListeners> SharedListenersPtr;
	SharedListenersPtr _listeners;
};

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"EntityListeners");
	module.RegisterClassT<EntityListeners>(name, __uuidof(IEntityEventListeners));
});;

BEGIN_INTERFACES(EntityListeners)
	HAS_INTERFACE(IEntityEventListeners)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

EntityListeners::EntityListeners()
{
}

EntityListeners::~EntityListeners()
{
}

// static
bool IEntityEventListeners::AddListener(IEntity *entity, IEntityEventListener *pListener)
{
	assertRetVal(entity && pListener, false);

	EntityListeners *pThis = (EntityListeners*)entity->EnsureComponent<IEntityEventListeners>();
	return pThis->AddListener(pListener);
}

// static
bool IEntityEventListeners::AddProxyListener(IEntity *entity, IEntityEventListener *pListener, IProxyEntityEventListener **ppProxy)
{
	ff::ComPtr<IProxyEntityEventListener> pProxy;
	assertRetVal(CreateProxyEntityEventListener(pListener, &pProxy), false);
	assertRetVal(AddListener(entity, pProxy), false);

	*ppProxy = ff::GetAddRef(pProxy.Interface());

	return true;
}

// static
bool IEntityEventListeners::RemoveListener(IEntity *entity, IEntityEventListener *pListener)
{
	assertRetVal(entity, false);

	EntityListeners *pThis = (EntityListeners*)entity->GetComponent<IEntityEventListeners>();
	
	return pThis && pThis->RemoveListener(pListener);
}

// static
bool IEntityEventListeners::HasListener(IEntity *entity, IEntityEventListener *pListener)
{
	EntityListeners *pThis = (EntityListeners*)entity->GetComponent<IEntityEventListeners>();

	return pThis && pThis->HasListener(pListener);
}

bool EntityListeners::AddListener(IEntityEventListener *pListener)
{
	assertRetVal(pListener, false);
	SharedListeners::GetUnshared(_listeners.Address());
	_listeners->Push(pListener);

	return true;
}

bool EntityListeners::RemoveListener(IEntityEventListener *pListener)
{
	SharedListeners::GetUnshared(_listeners.Address());
	for (size_t i = 0; i < _listeners->Size(); i++)
	{
		if (_listeners->GetAt(i) == pListener)
		{
			_listeners->Delete(i);
			return true;
		}
	}

	return false;
}

bool EntityListeners::HasListener(IEntityEventListener *pListener)
{
	SharedListeners::GetUnshared(_listeners.Address());
	for (size_t i = 0; i < _listeners->Size(); i++)
	{
		if (_listeners->GetAt(i) == pListener)
		{
			return true;
		}
	}

	return false;
}

void EntityListeners::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	SharedListeners::GetUnshared(_listeners.Address());
	SharedListenersPtr listeners = _listeners;

	for (size_t i = 0; i < listeners->Size(); i++)
	{
		listeners->GetAt(i)->OnEntityEvent(entity, eventName, eventArgs);
	}
}
