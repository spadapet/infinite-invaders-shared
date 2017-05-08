#include "pch.h"
#include "COM\ComAlloc.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\EntityManager.h"
#include "coreEntity\system\System.h"
#include "coreEntity\system\SystemManager.h"
#include "Graph\RenderTarget\RenderTarget.h"

class __declspec(uuid("f6d6445c-f7b5-45cb-94f9-48c014e53e00"))
	SystemManager : public ff::ComBase, public ISystemManager
{
public:
	DECLARE_HEADER(SystemManager);

	bool Init(IEntityDomain *pDomain);

	// ISystemManager

	virtual PingResult Ping() override;
	virtual void Advance() override;
	virtual void Render(ff::IRenderTarget *pTarget) override;

	virtual bool CreateSystem(REFGUID sysid, ISystem **ppSystem) override;
	virtual bool AddSystem(REFGUID sysid, ISystem* pSystem) override;
	virtual bool RemoveSystem(ISystem* pSystem) override;
	virtual void RemoveAllSystems() override;

	virtual size_t GetSystemCount() override;
	virtual ISystem* GetSystemByIndex(size_t index) override;
	virtual REFGUID GetSystemId(ISystem *pSystem) override;
	virtual bool HasSystem(ISystem *pSystem) override;

protected:
	virtual IEntityDomain* GetDomain() const override;
	virtual ISystem* InternalGet(ff::hash_t nHashId, size_t nDupeCount) override;

private:
	struct SystemInfo
	{
		ff::hash_t _nHashId;
		const GUID* _pSysId;
		ff::ComPtr<ISystem> _pSystem;

		bool operator<(const SystemInfo &rhs) const
		{
			return _pSystem->GetSystemPriority() < rhs._pSystem->GetSystemPriority();
		}
	};

	typedef ff::SharedObject<ff::Vector<SystemInfo>> SystemInfos;
	typedef ff::SmartPtr<SystemInfos> SystemInfosPtr;

	SystemInfosPtr _systems;
	IEntityDomain* _domain;
};

BEGIN_INTERFACES(SystemManager)
	HAS_INTERFACE(ISystemManager)
END_INTERFACES()

bool CreateSystemManager(IEntityDomain *pDomain, ISystemManager **ppMan)
{
	assertRetVal(ppMan, false);
	*ppMan = nullptr;

	ff::ComPtr<SystemManager, ISystemManager> pMan;
	assertRetVal(SUCCEEDED(ff::ComAllocator<SystemManager>::CreateInstance(&pMan)), false);
	assertRetVal(pMan->Init(pDomain), false);

	*ppMan = ff::GetAddRef(pMan.Interface());

	return true;
}

SystemManager::SystemManager()
	: _domain(nullptr)
{
}

SystemManager::~SystemManager()
{
	RemoveAllSystems();
}

bool SystemManager::Init(IEntityDomain *pDomain)
{
	assertRetVal(pDomain, false);
	_domain = pDomain;

	return true;
}

PingResult SystemManager::Ping()
{
	PingResult result = PING_RESULT_UNKNOWN;

	SystemInfos::GetUnshared(_systems.Address());
	SystemInfosPtr systems = _systems;

	for (size_t i = 0; i < systems->Size(); i++)
	{
		ISystem *pSystem = systems->GetAt(i)._pSystem;
		result = (PingResult)(result | pSystem->Ping(_domain));
	}

	return result;
}

void SystemManager::Advance()
{
	SystemInfos::GetUnshared(_systems.Address());
	SystemInfosPtr systems = _systems;

	for (size_t i = 0; i < systems->Size(); i++)
	{
		ISystem *pSystem = systems->GetAt(i)._pSystem;
		pSystem->Advance(_domain);
	}
}

void SystemManager::Render(ff::IRenderTarget *pTarget)
{
	SystemInfos::GetUnshared(_systems.Address());
	SystemInfosPtr systems = _systems;

	for (size_t i = 0; i < systems->Size(); i++)
	{
		ISystem *pSystem = systems->GetAt(i)._pSystem;
		pSystem->Render(_domain, pTarget);
	}
}

bool SystemManager::CreateSystem(REFGUID sysid, ISystem **ppSystem)
{
	assertRetVal(ppSystem, false);

	ff::ComPtr<IUnknown> pSystem;
	assertRetVal(GetDomain()->GetComponentManager()->CreateComponent(nullptr, sysid, &pSystem), false);
	assertRetVal(SUCCEEDED(pSystem->QueryInterface(__uuidof(ISystem), (void**)ppSystem)), false);

	return true;
}

bool SystemManager::AddSystem(REFGUID sysid, ISystem* pSystem)
{
	assertRetVal(pSystem, false);

	SystemInfo info;
	info._pSysId = &sysid;
	info._nHashId = ff::HashFunc(sysid);
	info._pSystem = pSystem;

	SystemInfos::GetUnshared(_systems.Address());
	_systems->SortInsert(info);
	pSystem->OnAdded(_domain);

	return true;
}

bool SystemManager::RemoveSystem(ISystem* pSystem)
{
	SystemInfos::GetUnshared(_systems.Address());
	for (size_t i = 0; i < _systems->Size(); i++)
	{
		const SystemInfo &info = _systems->GetAt(i);

		if (info._pSystem == pSystem)
		{
			_systems->Delete(i);
			pSystem->OnRemoved(_domain);

			return true;
		}
	}

	assertRetVal(false, false);
}

void SystemManager::RemoveAllSystems()
{
	SystemInfos::GetUnshared(_systems.Address());
	SystemInfosPtr systems = _systems;

	for (size_t i = 0; i < systems->Size(); i++)
	{
		ISystem *pSystem = systems->GetAt(i)._pSystem;
		RemoveSystem(pSystem);
	}

	assert(!_systems->Size());
}

size_t SystemManager::GetSystemCount()
{
	SystemInfos::GetUnshared(_systems.Address());
	return _systems->Size();
}

ISystem *SystemManager::GetSystemByIndex(size_t index)
{
	SystemInfos::GetUnshared(_systems.Address());
	assertRetVal(index >= 0 && index < _systems->Size(), nullptr);
	return _systems->GetAt(index)._pSystem;
}

REFGUID SystemManager::GetSystemId(ISystem *pSystem)
{
	SystemInfos::GetUnshared(_systems.Address());
	size_t nCount = _systems->Size();
	size_t i = 0;

	for (const SystemInfo *pInfo = nCount ? &_systems->GetAt(0) : nullptr; i < nCount; i++, pInfo++)
	{
		if (pInfo->_pSystem == pSystem)
		{
			return *pInfo->_pSysId;
		}
	}

	assertRetVal(false, GUID_NULL);
}

bool SystemManager::HasSystem(ISystem *pSystem)
{
	SystemInfos::GetUnshared(_systems.Address());
	size_t nCount = _systems->Size();
	size_t i = 0;

	for (const SystemInfo *pInfo = nCount ? &_systems->GetAt(0) : nullptr; i < nCount; i++, pInfo++)
	{
		if (pInfo->_pSystem == pSystem)
		{
			return true;
		}
	}

	return false;
}

IEntityDomain *SystemManager::GetDomain() const
{
	return _domain;
}

ISystem *SystemManager::InternalGet(ff::hash_t nHashId, size_t nDupeCount)
{
	SystemInfos::GetUnshared(_systems.Address());
	size_t nCount = _systems->Size();
	size_t i = 0;

	for (const SystemInfo *pInfo = nCount ? &_systems->GetAt(0) : nullptr; i < nCount; i++, pInfo++)
	{
		if (pInfo->_nHashId == nHashId)
		{
			if (!nDupeCount)
			{
				return pInfo->_pSystem;
			}
			else
			{
				nDupeCount--;
			}
		}
	}

	return nullptr;
}
