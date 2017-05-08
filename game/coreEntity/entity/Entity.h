#pragma once

#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"

class __declspec(uuid("efd68020-1fdb-4dd0-8a9c-9892ac82c4a7")) __declspec(novtable)
	IEntity : public IEntityDomainProvider
{
public:
	virtual void TriggerEvent(ff::hash_t eventName, void *eventArgs = nullptr) = 0;

	// Components (T must be auto-convertible to IUnknown)
	template<class T> bool AddComponent(T* pComp);
	template<class T> bool RemoveComponent(T* pComp);
	template<class T> bool CreateComponent(T** ppComp);
	template<class T> T* GetComponent(size_t nDupeCount = 0) const;
	template<class T> T* EnsureComponent();

	virtual void RemoveAllComponents() = 0;

	virtual size_t GetComponentCount() = 0;
	virtual IUnknown* GetComponentByIndex(size_t index) = 0;

protected:
	virtual bool AddUnknown(const GUID* pCompIds, IUnknown** ppComps, size_t nCount) = 0;
	virtual bool RemoveUnknown(IUnknown* pComp) = 0;
	virtual IUnknown* GetUnknown(ff::hash_t nHashId, size_t nDupeCount) = 0;
};

template<class T>
T* IEntity::GetComponent(size_t nDupeCount) const
{
	IEntity *entity = const_cast<IEntity*>(this);
	return static_cast<T*>(entity->GetUnknown(ff::HashFunc(__uuidof(T)), nDupeCount));
}

template<class T>
bool IEntity::RemoveComponent(T *pComp)
{
	return RemoveUnknown(pComp);
}

template<class T>
bool IEntity::AddComponent(T* pComp)
{
	IUnknown *pUnk = pComp;
	return AddUnknown(&__uuidof(T), &pUnk, 1);
}

template<class T>
bool IEntity::CreateComponent(T** ppComp)
{
	assertRetVal(GetDomain()->GetComponentManager()->CreateComponent<T>(this, ppComp), false);
	return true;
}

template<class T>
T *IEntity::EnsureComponent()
{
	T *pComp = GetComponent<T>();

	if (!pComp)
	{
		ff::ComPtr<T> pNewComp;
		assertRetVal(CreateComponent<T>(&pNewComp), nullptr);
		assertRetVal(AddComponent<T>(pNewComp), nullptr);

		pComp = pNewComp;
	}

	return pComp;
}

