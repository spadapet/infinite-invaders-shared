#pragma once

#include "coreEntity\component\ComponentManager.h"

namespace ff
{
	class IRenderTarget;
}

enum PingResult;
class ISystem;

class __declspec(uuid("f793ca5d-e283-4e5d-8f23-fb8947faff32")) __declspec(novtable)
	ISystemManager : public IUnknown
{
public:
	virtual PingResult Ping() = 0;
	virtual void Advance() = 0;
	virtual void Render(ff::IRenderTarget *pTarget) = 0;

	template<class T> bool CreateSystem(T** ppSystem);
	template<class T> bool AddSystem(T* pSystem);
	template<class T> T* GetSystem(size_t nDupeCount = 0) const;
	template<class T> T* EnsureSystem();

	virtual bool CreateSystem(REFGUID sysid, ISystem **ppSystem) = 0;
	virtual bool AddSystem(REFGUID sysid, ISystem* pSystem) = 0;
	virtual bool RemoveSystem(ISystem* pSystem) = 0;
	virtual void RemoveAllSystems() = 0;

	virtual size_t GetSystemCount() = 0;
	virtual ISystem* GetSystemByIndex(size_t index) = 0;
	virtual REFGUID GetSystemId(ISystem *pSystem) = 0;
	virtual bool HasSystem(ISystem *pSystem) = 0;

protected:
	virtual IEntityDomain* GetDomain() const = 0;
	virtual ISystem* InternalGet(ff::hash_t nHashId, size_t nDupeCount) = 0;
};

bool CreateSystemManager(IEntityDomain *pDomain, ISystemManager **ppMan);

template<class T>
bool ISystemManager::CreateSystem(T** ppSystem)
{
	return GetDomain()->GetComponentManager()->CreateComponent<T>(nullptr, ppSystem);
}

template<class T>
T* ISystemManager::GetSystem(size_t nDupeCount) const
{
	ISystemManager *self = const_cast<ISystemManager*>(this);
	return static_cast<T*>(self->InternalGet(ff::HashFunc(__uuidof(T)), nDupeCount));
}

template<class T>
bool ISystemManager::AddSystem(T* pSystem)
{
	return AddSystem(__uuidof(T), pSystem);
}

template<class T>
T *ISystemManager::EnsureSystem()
{
	T *pSystem = GetSystem<T>();

	if (!pSystem)
	{
		ff::ComPtr<T, ISystem> pNewSystem;
		assertRetVal(CreateSystem<T>(&pNewSystem), nullptr);
		assertRetVal(AddSystem<T>(pNewSystem), nullptr);

		pSystem = pNewSystem;
	}

	return pSystem;
}
