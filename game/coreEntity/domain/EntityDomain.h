#pragma once

#include "COM\ServiceCollection.h"
#include "coreEntity\domain\EntityDomainProvider.h"

namespace ff
{
	class IServiceCollection;
}

class ThisApplication;
class IComponentManager;
class IEntity;
class IEntityManager;
class ISystemManager;

class __declspec(uuid("3a4d68ce-f143-4b69-b7d2-a09bb3372e3a")) __declspec(novtable)
	IEntityDomain : public IEntityDomainProvider
{
public:
	virtual ThisApplication* GetApp() const = 0;
	virtual ISystemManager* GetSystemManager() const = 0;
	virtual IEntityManager* GetEntityManager() const = 0;
	virtual IComponentManager* GetComponentManager() const = 0;
	virtual ff::IServiceCollection* GetServices() const = 0;
};

bool CreateEntityDomain(ThisApplication *pApp, IEntityDomain **ppDomain);

template<typename T>
bool GetService(IEntityDomainProvider *pDomainProvider, T **ppService)
{
	return ff::GetService<T>(pDomainProvider->GetDomain()->GetServices(), ppService);
}
