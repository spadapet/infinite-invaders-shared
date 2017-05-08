#include "pch.h"
#include "COM\ComAlloc.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityEvents.h"
#include "coreEntity\entity\EntityListener.h"
#include "coreEntity\entity\EntityManager.h"

class __declspec(uuid("7011a4fb-e373-4bd7-9bc3-3ad71c94f0fe"))
	Entity : public ff::ComBaseUnsafe , public IEntity
{
public:
	DECLARE_HEADER(Entity);

	bool Init(IEntityDomain *pDomain);

	// ComBase
	virtual void _Destruct() override;

	// IEntityDomainProvider
	virtual IEntityDomain* GetDomain() const override;

	// IEntity
	virtual void TriggerEvent(ff::hash_t eventName, void *eventArgs) override;
	virtual void RemoveAllComponents() override;
	virtual size_t GetComponentCount() override;
	virtual IUnknown* GetComponentByIndex(size_t index) override;

protected:
	virtual bool AddUnknown(const GUID* pCompIds, IUnknown** ppComps, size_t nCount) override;
	virtual bool RemoveUnknown(IUnknown* pComp) override;
	virtual IUnknown* GetUnknown(ff::hash_t nHashId, size_t nDupeCount) override;

private:
	// Types
	struct ComponentInfo
	{
		ff::hash_t _nHashCompId;
		ff::hash_t _nHashClassId;
		const GUID* _pCompId;
		const GUID* _pClassId;
		ff::ComPtr<IUnknown> _pComp;
		ff::ComPtr<IEntityEventListener> _listener;
	};

	typedef ff::SharedObject<ff::Vector<ComponentInfo>> EntityComponents;
	typedef ff::SmartPtr<EntityComponents> EntityComponentsPtr;
	typedef ff::Vector<IEntityEventListener*, 16> AddRemoveListeners;

	void OnAddComponents(const GUID* pCompIds, IUnknown **ppComps, size_t nCount, const AddRemoveListeners &addedListeners);
	void OnRemovedComponents(const GUID* pCompIds, IUnknown **ppComps, size_t nCount, const AddRemoveListeners &removedListeners);

	// Data
	IEntityDomain* _domain;
	EntityComponentsPtr _comps;
};

BEGIN_INTERFACES(Entity)
	HAS_INTERFACE(IEntity)
	HAS_INTERFACE(IEntityDomainProvider)
END_INTERFACES()

// A sneaky back door for the entity manager
bool InternalCreateEntity(IEntityDomain *pDomain, IEntity **pentity)
{
	ff::ComPtr<Entity, IEntity> obj;
	assertHrRetVal(ff::ComAllocator<Entity>::CreateInstance(&obj), false);
	assertRetVal(obj->Init(pDomain), false);

	*pentity = ff::GetAddRef(obj.Interface());
	return true;
}

Entity::Entity()
	: _domain(nullptr)
{
}

Entity::~Entity()
{
}

bool Entity::Init(IEntityDomain *pDomain)
{
	assertRetVal(pDomain, false);
	_domain = pDomain;

	assertRetVal(SUCCEEDED(_Construct(nullptr)), false);

	return true;
}

void Entity::_Destruct()
{
	TriggerEvent(ENTITY_EVENT_DESTROY, nullptr);
	RemoveAllComponents();

	__super::_Destruct();
}

IEntityDomain *Entity::GetDomain() const
{
	return _domain;
}

void Entity::TriggerEvent(ff::hash_t eventName, void *eventArgs)
{
	ff::ComPtr<IEntity> pKeepAlive = this;

	// Global entity listeners
	_domain->GetEntityManager()->OnEntityEvent(this, eventName, eventArgs);

	// All my components
	EntityComponents::GetUnshared(_comps.Address());
	EntityComponentsPtr comps = _comps;

	for (size_t i = 0; i < comps->Size(); i++)
	{
		IEntityEventListener *pListener = comps->GetAt(i)._listener;

		if (pListener)
		{
			pListener->OnEntityEvent(this, eventName, eventArgs);
		}
	}
}

void Entity::RemoveAllComponents()
{
	EntityComponents::GetUnshared(_comps.Address());
	if (_comps->Size())
	{
		ff::Vector<GUID, 32> ids;
		ff::Vector<ff::ComPtr<IUnknown>, 32> comps;
		AddRemoveListeners removedListeners;

		for (size_t i = 0; i < _comps->Size(); i++)
		{
			const ComponentInfo &info = _comps->GetAt(i);

			ids.Push(*info._pCompId);
			comps.Push(info._pComp);

			if (info._nHashCompId != info._nHashClassId)
			{
				ids.Push(*info._pClassId);
				comps.Push(info._pComp);
			}

			if (info._listener)
			{
				removedListeners.Push(info._listener);
			}
		}

		_comps->Clear();

		OnRemovedComponents(ids.Data(), comps[0].Address(), comps.Size(), removedListeners);
	}
}

size_t Entity::GetComponentCount()
{
	EntityComponents::GetUnshared(_comps.Address());
	return _comps->Size();
}

IUnknown *Entity::GetComponentByIndex(size_t index)
{
	EntityComponents::GetUnshared(_comps.Address());
	assertRetVal(index >= 0 && index < _comps->Size(), nullptr);
	return _comps->GetAt(index)._pComp;
}

bool Entity::RemoveUnknown(IUnknown* pComp)
{
	EntityComponents::GetUnshared(_comps.Address());

	for (size_t i = 0; i < _comps->Size(); i++)
	{
		const ComponentInfo &info = _comps->GetAt(i);

		if (info._pComp == pComp)
		{
			ff::Vector<GUID, 2> ids;
			ff::Vector<ff::ComPtr<IUnknown>, 2> comps;
			AddRemoveListeners removedListeners;

			ids.Push(*info._pCompId);
			comps.Push(info._pComp);

			// If the class ID is different than the component ID, then send another event for the class ID
			if (info._nHashCompId != info._nHashClassId)
			{
				ids.Push(*info._pClassId);
				comps.Push(info._pComp);
			}

			if (info._listener)
			{
				removedListeners.Push(info._listener);
			}

			_comps->Delete(i);

			OnRemovedComponents(ids.Data(), comps[0].Address(), comps.Size(), removedListeners);

			return true;
		}
	}

	assertRetVal(false, false);
}

bool Entity::AddUnknown(const GUID* pCompIds, IUnknown** ppComps, size_t nCount)
{
	if (nCount)
	{
		assertRetVal(pCompIds && ppComps, false);

		ff::Vector<GUID, 32> ids;
		ff::Vector<ff::ComPtr<IUnknown>, 32> comps;
		AddRemoveListeners addedListeners;

		for (size_t i = 0; i < nCount; i++)
		{
			ComponentInfo info;
			info._nHashCompId = ff::HashFunc(pCompIds[i]);
			info._pCompId = &pCompIds[i];
			info._pComp = ppComps[i];

			ids.Push(*info._pCompId);
			comps.Push(info._pComp);

			// If the class ID is different than the component ID, then send another event for the class ID
			ff::ComPtr<ff::IComObject> pComObject;
			if (pComObject.QueryFrom(ppComps[i]))
			{
				info._pClassId = &pComObject->GetComClassID();
				info._nHashClassId = ff::HashFunc(*info._pClassId);
			}
			else
			{
				info._pClassId = info._pCompId;
				info._nHashClassId = info._nHashCompId;
			}

			if (info._nHashClassId != info._nHashCompId)
			{
				ids.Push(*info._pClassId);
				comps.Push(info._pComp);
			}

			if (info._listener.QueryFrom(ppComps[i]))
			{
				addedListeners.Push(info._listener);
			}

			EntityComponents::GetUnshared(_comps.Address());
			_comps->Push(info);
		}

		OnAddComponents(ids.Data(), comps[0].Address(), comps.Size(), addedListeners);
	}

	return true;
}

IUnknown *Entity::GetUnknown(ff::hash_t nHashId, size_t nDupeCount)
{
	EntityComponents::GetUnshared(_comps.Address());
	size_t nCount = _comps->Size();
	size_t i = 0;

	for (const ComponentInfo *pInfo = nCount ? &_comps->GetAt(0) : nullptr; i < nCount; i++, pInfo++)
	{
		if (pInfo->_nHashCompId == nHashId || pInfo->_nHashClassId == nHashId)
		{
			if (!nDupeCount)
			{
				return pInfo->_pComp;
			}
			else
			{
				nDupeCount--;
			}
		}
	}

	return nullptr;
}

void Entity::OnAddComponents(const GUID* pCompIds, IUnknown **ppComps, size_t nCount, const AddRemoveListeners &addedListeners)
{
	ff::ComPtr<IEntity> pKeepAlive = this;

	// Global component listeners
	for (size_t i = 0; i < nCount; i++)
	{
		_domain->GetComponentManager()->OnAddComponent(this, pCompIds[i], ppComps[i]);
	}

	for (size_t i = 0; i < addedListeners.Size(); i++)
	{
		addedListeners[i]->OnEntityEvent(this, ENTITY_EVENT_ADD_COMPONENT, nullptr);
	}

	AddRemoveComponentsEventArgs params;
	params._pCompIds = pCompIds;
	params._ppComps = ppComps;
	params._nCount = nCount;

	TriggerEvent(ENTITY_EVENT_ADD_COMPONENTS, &params);
}

void Entity::OnRemovedComponents(const GUID* pCompIds, IUnknown **ppComps, size_t nCount, const AddRemoveListeners &removedListeners)
{
	ff::ComPtr<IEntity> pKeepAlive = this;

	// Global component listeners
	for (size_t i = 0; i < nCount; i++)
	{
		_domain->GetComponentManager()->OnRemoveComponent(this, pCompIds[i], ppComps[i]);
	}

	AddRemoveComponentsEventArgs params;
	params._pCompIds = pCompIds;
	params._ppComps = ppComps;
	params._nCount = nCount;

	TriggerEvent(ENTITY_EVENT_REMOVE_COMPONENTS, &params);

	// My removed components
	for (size_t i = 0; i < removedListeners.Size(); i++)
	{
		removedListeners[i]->OnEntityEvent(this, ENTITY_EVENT_REMOVE_COMPONENTS, &params);
		removedListeners[i]->OnEntityEvent(this, ENTITY_EVENT_REMOVE_COMPONENT, nullptr);
	}
}
