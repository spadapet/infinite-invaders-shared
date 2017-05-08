#include "pch.h"
#include "Globals.h"
#include "components\invader\InvaderComponent.h"
#include "components\core\PositionComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"
#include "Module\ModuleFactory.h"
#include "services\InvaderService.h"
#include "services\PlayerService.h"
#include "services\SaucerService.h"
#include "systems\LevelSystem.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"LevelSystem");
	module.RegisterClassT<LevelSystem>(name);
});

BEGIN_INTERFACES(LevelSystem)
	HAS_INTERFACE(ISystem)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

LevelSystem::LevelSystem()
	: _startedLevel(false)
	, _completedLevel(false)
{
}

LevelSystem::~LevelSystem()
{
}

HRESULT LevelSystem::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	_bornListener.Init(pDomainProvider->GetDomain(), ENTITY_EVENT_BORN, this);
	_diedListener.Init(pDomainProvider->GetDomain(), ENTITY_EVENT_DIED, this);

	return __super::_Construct(unkOuter);
}

bool LevelSystem::DidCompleteLevel() const
{
	return _completedLevel;
}

int LevelSystem::GetSystemPriority() const
{
	return SYS_PRI_ADVANCE_NORMAL;
}

PingResult LevelSystem::Ping(IEntityDomain *pDomain)
{
	return PING_RESULT_RUNNING;
}

void LevelSystem::Advance(IEntityDomain *pDomain)
{
	if (!_startedLevel)
	{
		_startedLevel = true;

		pDomain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_LEVEL_START);
	}

	if (_startedLevel && !_completedLevel)
	{
		ff::ComPtr<IInvaderService> pInvaderService;
		ff::ComPtr<ISaucerService> pSaucerService;
		ff::ComPtr<IPlayerService> pPlayerService;

		GetService(pDomain, &pInvaderService);
		GetService(pDomain, &pSaucerService);
		GetService(pDomain, &pPlayerService);

		if ((!pInvaderService || !pInvaderService->GetInvaderCount()) &&
			(!pSaucerService || !pSaucerService->GetSaucerCount()) &&
			(!pPlayerService || pPlayerService->IsAnyPlayerAlive()))
		{
			_completedLevel = true;

			pDomain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_LEVEL_COMPLETE);
		}
	}
}

void LevelSystem::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
}

void LevelSystem::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_BORN)
	{
		_aliveEntities.Insert(entity);
	}
	else if (eventName == ENTITY_EVENT_DIED)
	{
		_aliveEntities.DeleteKey(entity);
	}
}
