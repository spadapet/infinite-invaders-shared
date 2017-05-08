#include "pch.h"
#include "COM\ComAlloc.h"
#include "COM\ServiceCollection.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\entity\EntityManager.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals\ProcessGlobals.h"

class __declspec(uuid("45683ff1-5425-44e9-a956-3374c0995fff"))
	EntityDomain
		: public ff::ComBase
		, public IEntityDomain
{
public:
	DECLARE_HEADER(EntityDomain);

	bool Init(ThisApplication *pApp);

	// IEntityDomain

	virtual ThisApplication* GetApp() const override;
	virtual ISystemManager* GetSystemManager() const override;
	virtual IEntityManager* GetEntityManager() const override;
	virtual IComponentManager* GetComponentManager() const override;
	virtual ff::IServiceCollection* GetServices() const override;

	// IEntityDomainProvider
	virtual IEntityDomain* GetDomain() const override;

private:
	ThisApplication* _pApp;
	ff::ComPtr<IEntityManager> _entityManager;
	ff::ComPtr<IComponentManager> _pComponentManager;
	ff::ComPtr<ISystemManager> _pSystemManager; // must die after entities are all released
	ff::ComPtr<ff::IServiceCollection> _pServices; // must die before systems in case they hold onto entities
};

BEGIN_INTERFACES(EntityDomain)
	HAS_INTERFACE(IEntityDomain)
	HAS_INTERFACE(IEntityDomainProvider)
END_INTERFACES()

bool CreateEntityDomain(ThisApplication *pApp, IEntityDomain **ppDomain)
{
	assertRetVal(ppDomain, false);
	*ppDomain = nullptr;

	ff::ComPtr<EntityDomain, IEntityDomain> pDomain;
	assertRetVal(SUCCEEDED(ff::ComAllocator<EntityDomain>::CreateInstance(&pDomain)), false);
	assertRetVal(pDomain->Init(pApp), false);

	*ppDomain = ff::GetAddRef(pDomain.Interface());
	return true;
}

EntityDomain::EntityDomain()
{
}

EntityDomain::~EntityDomain()
{
}

bool EntityDomain::Init(ThisApplication *pApp)
{
	assertRetVal(pApp, false);
	_pApp = pApp;

	assertRetVal(CreateSystemManager(this, &_pSystemManager), false);
	assertRetVal(CreateEntityManager(this, &_entityManager), false);
	assertRetVal(CreateComponentManager(this, &_pComponentManager), false);

	// DOMAIN_THREAD_SAFE(false)
	assertRetVal(CreateServiceCollection(false, &_pServices), false);
	assertRetVal(_pServices->AddProvider(ff::ProcessGlobals::Get()->GetServices()), false);

	return true;
}

ThisApplication *EntityDomain::GetApp() const
{
	return _pApp;
}

ISystemManager *EntityDomain::GetSystemManager() const
{
	return _pSystemManager;
}

IEntityManager *EntityDomain::GetEntityManager() const
{
	return _entityManager;
}

IComponentManager *EntityDomain::GetComponentManager() const
{
	return _pComponentManager;
}

ff::IServiceCollection *EntityDomain::GetServices() const
{
	return _pServices;
}

IEntityDomain *EntityDomain::GetDomain() const
{
	const IEntityDomain *pDomain = this;
	return const_cast<IEntityDomain*>(pDomain);
}
