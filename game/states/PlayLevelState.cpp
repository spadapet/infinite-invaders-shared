#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "COM\ServiceCollection.h"
#include "components\bullet\BulletAdvance.h"
#include "components\core\StateComponent.h"
#include "components\invader\InvaderAdvanceRender.h"
#include "components\level\PauseAdvanceRender.h"
#include "components\level\ScoreAdvanceRender.h"
#include "components\player\PlayerAdvanceRender.h"
#include "components\powerup\PowerupAdvance.h"
#include "components\saucer\SaucerAdvance.h"
#include "components\shield\ShieldAdvance.h"
#include "components\core\CollisionAdvanceRender.h"
#include "components\core\LoadingComponent.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "coreEntity\system\SystemManager.h"
#include "entities\EntityEvents.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"
#include "services\GlobalPlayerService.h"
#include "states\PlayLevelState.h"
#include "systems\AdvanceSystem.h"
#include "systems\AudioSystem.h"
#include "systems\LevelSystem.h"
#include "systems\RenderSystem.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PlayLevelState");
	module.RegisterClassT<PlayLevelState>(name, __uuidof(IPlayLevelState));
});

BEGIN_INTERFACES(PlayLevelState)
	HAS_INTERFACE(IPlayLevelState)
	HAS_INTERFACE(ISystem)
END_INTERFACES()

PlayLevelState::PlayLevelState()
	: _nCompletedCounter(0)
	, _completedLevel(false)
{
}

PlayLevelState::~PlayLevelState()
{
}

HRESULT PlayLevelState::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	IEntityDomain* pDomain = pDomainProvider->GetDomain();
	ThisApplication* pApp = ThisApplication::Get(pDomain);

	// Child domain
	{
		assertRetVal(CreateEntityDomain(pApp, &_pChildDomain), E_FAIL);
		assertRetVal(_pChildDomain->GetComponentManager()->CreateComponent<LoadingComponent>(nullptr, &_loading), E_FAIL);

		_pChildDomain->GetServices()->AddProvider(pDomain->GetServices());
	}

	ff::ComPtr<EntityFactoryService> entityFactory;
	{
		assertRetVal(SUCCEEDED(ff::ComAllocator<EntityFactoryService>::CreateInstance(_pChildDomain, &entityFactory)), E_FAIL);
		assertRetVal(_pChildDomain->GetServices()->AddService(__uuidof(EntityFactoryService), entityFactory), E_FAIL);
	}

	// Systems
	{
		_pChildDomain->GetSystemManager()->EnsureSystem<LevelSystem>();
		_pChildDomain->GetSystemManager()->EnsureSystem<AdvanceSystem>();
		_pChildDomain->GetSystemManager()->EnsureSystem<RenderSystem>();
		_pChildDomain->GetSystemManager()->EnsureSystem<AudioSystem>();
	}

	// Create global advance/render components
	{
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pChildEntity), E_FAIL);
		_pChildEntity->TriggerEvent(ENTITY_EVENT_BORN);

		ff::ComPtr<CollisionAdvanceRender, IAdvanceComponent> pCollisionAR;
		assertRetVal(_pChildEntity->CreateComponent<CollisionAdvanceRender>(&pCollisionAR), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pCollisionAR);
		_pChildEntity->AddComponent<I2dRenderComponent>(pCollisionAR);

		ff::ComPtr<PlayerAdvanceRender, IAdvanceComponent> pPlayerAR;
		assertRetVal(_pChildEntity->CreateComponent<PlayerAdvanceRender>(&pPlayerAR), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pPlayerAR);
		_pChildEntity->AddComponent<I2dRenderComponent>(pPlayerAR);

		ff::ComPtr<InvaderAdvanceRender, IAdvanceComponent> pInvaderAR;
		assertRetVal(_pChildEntity->CreateComponent<InvaderAdvanceRender>(&pInvaderAR), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pInvaderAR);
		_pChildEntity->AddComponent<I2dRenderComponent>(pInvaderAR);

		ff::ComPtr<BulletAdvance, IAdvanceComponent> pBulletA;
		assertRetVal(_pChildEntity->CreateComponent<BulletAdvance>(&pBulletA), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pBulletA);

		ff::ComPtr<ShieldAdvance, IAdvanceComponent> pShieldA;
		assertRetVal(_pChildEntity->CreateComponent<ShieldAdvance>(&pShieldA), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pShieldA);

		ff::ComPtr<SaucerAdvance, IAdvanceComponent> pSaucerA;
		assertRetVal(_pChildEntity->CreateComponent<SaucerAdvance>(&pSaucerA), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pSaucerA);

		ff::ComPtr<PowerupAdvance, IAdvanceComponent> pPowerupA;
		assertRetVal(_pChildEntity->CreateComponent<PowerupAdvance>(&pPowerupA), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pPowerupA);

		ff::ComPtr<ScoreAdvanceRender, IAdvanceComponent> pScoreAR;
		assertRetVal(_pChildEntity->CreateComponent<ScoreAdvanceRender>(&pScoreAR), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pScoreAR);
		_pChildEntity->AddComponent<I2dRenderComponent>(pScoreAR);

		assertRetVal(_pChildEntity->CreateComponent<PauseAdvanceRender>(&_pPauseAR), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(_pPauseAR);
		_pChildEntity->AddComponent<I2dRenderComponent>(_pPauseAR);
	}

	ff::ComPtr<GlobalPlayerService> pGlobalPlayers;
	assertRetVal(GetService(_pChildDomain, &pGlobalPlayers), E_FAIL);
	assertRetVal(pGlobalPlayers->GetCurrentPlayerCount(), E_FAIL);

	// Create the level
	{
		size_t nLevel = Globals::GetCurrentLevel(_pChildDomain);
		Difficulty diff = Globals::GetDifficulty(_pChildDomain);

		ff::ComPtr<IEntity> pLevelEntity;
		assertRetVal(entityFactory->CreateLevelEntity(diff, nLevel, &pLevelEntity), E_FAIL);
	}

	// Create the players
	for (size_t i = 0; i < pGlobalPlayers->GetCurrentPlayerCount(); i++)
	{
		size_t nPlayer = pGlobalPlayers->GetCurrentPlayer(i);
		PlayerGlobals* pPlayerGlobals = pGlobalPlayers->GetPlayerGlobals(nPlayer);

		ff::ComPtr<IEntity> pPlayerEntity;
		assertRetVal(entityFactory->CreatePlayerEntity(nPlayer, &pPlayerEntity), E_FAIL);
	}

	return __super::_Construct(unkOuter);
}

bool PlayLevelState::DidCompleteLevel() const
{
	return _completedLevel;
}

IEntityDomain *PlayLevelState::GetChildDomain()
{
	return _pChildDomain;
}

void PlayLevelState::OnStartPlaying()
{
	_pChildDomain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_PLAYING_START);
}

void PlayLevelState::OnStopPlaying()
{
	_pChildDomain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_PLAYING_STOP);
}

int PlayLevelState::GetSystemPriority() const
{
	return SYS_PRI_STATE_NORMAL;
}

PingResult PlayLevelState::Ping(IEntityDomain *pDomain)
{
	// Check if the level is still loading
	if (_loading)
	{
		_loading->Advance(nullptr);

		if (!_loading->IsLoading())
		{
			_loading = nullptr;
		}
	}

	if (_loading)
	{
		return PING_RESULT_INIT;
	}

	if (_completedLevel)
	{
		return PING_RESULT_DEAD;
	}

	return PING_RESULT_RUNNING;
}

void PlayLevelState::Advance(IEntityDomain *pDomain)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);

	// Don't advance anything while the level is loading
	if (!_loading)
	{
		if (pApp->IsGamePaused() && !pApp->AllowGameAdvanceWhilePaused())
		{
			_pPauseAR->Advance(_pChildEntity);
		}
		else
		{
			_pChildDomain->GetSystemManager()->Advance();
		}
	}

	// Check if the level is complete
	{
		LevelSystem *pLevelSystem = _pChildDomain->GetSystemManager()->GetSystem<LevelSystem>();

		if (pLevelSystem->DidCompleteLevel())
		{
			if (++_nCompletedCounter > 15)
			{
				_completedLevel = true;
			}
		}
		else
		{
			_nCompletedCounter = 0;
		}
	}
}

void PlayLevelState::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	_pChildDomain->GetSystemManager()->Render(pTarget);
}
