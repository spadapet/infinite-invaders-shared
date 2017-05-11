#include "pch.h"
#include "App.xaml.h"
#include "COM\ServiceCollection.h"
#include "components\core\LoadingComponent.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\Font\SpriteFont.h"
#include "Input\InputMapping.h"
#include "Input\KeyboardDevice.h"
#include "Input\Joystick\JoystickDevice.h"
#include "InputEvents.h"
#include "Module\ModuleFactory.h"
#include "services\GlobalPlayerService.h"
#include "services\PlayerService.h"
#include "states\AddScoreState.h"
#include "states\PlayGameState.h"
#include "states\PlayLevelState.h"
#include "states\TitleState.h"
#include "states\TransitionState.h"
#include "systems\AudioSystem.h"
#include "ThisApplication.h"
#include "Types\Timer.h"

enum CustomInputEvent
{
	CIE_PAUSE = GIE_CUSTOM,
	CIE_PAUSE_START,
};

static const ff::InputEventMapping s_gameInputEvents[] =
{
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'P' } }, CIE_PAUSE },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_SPECIAL_BUTTON, ff::INPUT_VALUE_PRESSED, ff::JOYSTICK_BUTTON_START } }, CIE_PAUSE_START },
};

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PlayGameState");
	module.RegisterClassT<PlayGameState>(name, __uuidof(IPlayGameState));
});

BEGIN_INTERFACES(PlayGameState)
	HAS_INTERFACE(IPlayGameState)
	HAS_INTERFACE(ISystem)
END_INTERFACES()

PlayGameState::PlayGameState()
	: _state(GS_PLAYER_READY)
	, _stateCounter(0)
	, _nPlayer(0)
{
}

PlayGameState::~PlayGameState()
{
}

HRESULT PlayGameState::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	ThisApplication* pApp = ThisApplication::Get(pDomainProvider);

	_font.Init(L"Classic");

	// Input mapping
	{
		assertRetVal(CreateInputMapping(true, true, false, &_inputMapping), false);
		assertRetVal(AddDefaultInputEventsAndValues(_inputMapping), false);
		assertRetVal(_inputMapping->MapEvents(s_gameInputEvents, _countof(s_gameInputEvents)), false);
	}

	// Child domain
	{
		assertRetVal(CreateEntityDomain(pApp, &_pChildDomain), E_FAIL);
		assertRetVal(_pChildDomain->GetComponentManager()->CreateComponent<LoadingComponent>(nullptr, &_loading), E_FAIL);
	}

	assertRetVal(_pChildDomain->GetComponentManager()->CreateComponent<GlobalPlayerService>(nullptr, &_globalPlayers), E_FAIL);
	assertRetVal(_pChildDomain->GetServices()->AddService(__uuidof(GlobalPlayerService), _globalPlayers), E_FAIL);

	assertRetVal(Create2dLayerGroup(_pChildDomain, &_pRenderLayer), E_FAIL);
	_pRenderLayer->SetWorldSize(Globals::GetLevelSizeF());
	_pRenderLayer->GetViewport()->SetAutoSizePadding(pApp->GetOptions().GetRect(ThisApplication::OPTION_WINDOW_PADDING));
	_pRenderLayer->GetViewport()->SetAutoSizeAspect(Globals::GetLevelSize());
	_pRenderLayer->GetViewport()->SetUseDepth(false);

	for (size_t i = 0; i < _globalPlayers->GetPlayerCount(); i++)
	{
		assertRetVal(CreatePlayLevel(i), E_FAIL);
	}

	SetCurrentPlayer(0);

	assertRetVal(_pChildDomain->GetSystemManager()->AddSystem<IPlayLevelState>(_playLevel[_nPlayer]), E_FAIL);
	_playLevel[_nPlayer]->OnStartPlaying();

	pApp->UnpauseGame();

	return __super::_Construct(unkOuter);
}

GlobalPlayerService *PlayGameState::GetGlobalPlayerService()
{
	return _globalPlayers;
}

void PlayGameState::TrackStartGame()
{
}

int PlayGameState::GetSystemPriority() const
{
	return SYS_PRI_STATE_NORMAL;
}

PingResult PlayGameState::Ping(IEntityDomain *pDomain)
{
	if (_loading)
	{
		_loading->Advance(nullptr);

		if (!_loading->IsLoading())
		{
			_loading = nullptr;
		}
	}

	PingResult childResult = _pChildDomain->GetSystemManager()->Ping();

	return _loading ? PING_RESULT_INIT : childResult;
}

void PlayGameState::Advance(IEntityDomain *pDomain)
{
	// Handle input events
	{
		ThisApplication *pApp = ThisApplication::Get(pDomain);
		pApp->AdvanceInputMapping(_inputMapping);

		for (const ff::InputEvent &ie : _inputMapping->GetEvents())
		{
			if (ie.IsStart())
			{
				HandleEventStart(pDomain, ie._eventID);
			}
			else if (ie.IsStop())
			{
				HandleEventStop(pDomain, ie._eventID);
			}
		}
	}

	if (_loading)
	{
		// do nothing until loading is done
	}
	else switch (_state)
	{
	case GS_PLAYER_READY:
		if (_stateCounter > 60)
		{
			SetState(GS_READY);
		}
		break;

	case GS_READY:
		if (_stateCounter > 60)
		{
			SetState(GS_PLAYING);
		}
		break;

	case GS_PLAYING:
		{
			ThisApplication *pApp = ThisApplication::Get(pDomain);
			bool bGamePaused = pApp->IsGamePaused();

			_pChildDomain->GetSystemManager()->Advance();

			if (bGamePaused)
			{
				CheckRequests(pDomain);
			}
			else if (DidCompleteLevel())
			{
				if (!OnCompletedLevel())
				{
					OnQuit(pDomain);
				}
			}
			else
			{
				CheckDeath(pDomain);
			}
		}
		break;

	case GS_BETWEEN_LEVELS:
	case GS_SWITCHING_PLAYERS:
		_pChildDomain->GetSystemManager()->Advance();

		// Waiting for the transition to finish
		if (_pChildDomain->GetSystemManager()->GetSystem<IPlayLevelState>())
		{
			bool bBonusLevel = Globals::IsBonusLevel(Globals::GetDifficulty(_pChildDomain), Globals::GetCurrentLevel(_pChildDomain));
			EGameState newState = (_state == GS_SWITCHING_PLAYERS)
				? GS_PLAYER_READY
				: (bBonusLevel ? GS_PLAYING : GS_READY);

			SetState(newState);
		}
		break;

	case GS_CHECK_HIGH_SCORE:
		if (CheckHighScore(pDomain))
		{
			_playLevel[_nPlayer]->OnStopPlaying();
			SetState(GS_ENTER_HIGH_SCORE);
		}
		else
		{
			SwitchPlayers();
		}
		break;

	case GS_ENTER_HIGH_SCORE:
		_playLevel[_nPlayer]->OnStartPlaying();
		SwitchPlayers();
		break;

	case GS_GAME_OVER:
		if (!CheckRequests(pDomain))
		{
			_pChildDomain->GetSystemManager()->Advance();
		}
		break;
	}

	_stateCounter++;
}

void PlayGameState::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	_pChildDomain->GetSystemManager()->Render(pTarget);

	switch (_state)
	{
	case GS_PLAYER_READY:
	case GS_READY:
		{
			ff::I2dRenderer *render = ff::MetroGlobals::Get()->Get2dRender();
			_pRenderLayer->GetViewport()->SetRenderTargets(&pTarget, 1);

			if (_pRenderLayer->BeginCustomRender(render))
			{
				ff::String szText;
				
				if (_state == GS_PLAYER_READY)
				{
					szText = (_globalPlayers->GetGameMode() == GAME_MODE_COOP)
						? L"PLAYERS"
						: L"PLAYER";

					if (_globalPlayers->GetGameMode() == GAME_MODE_TURNS)
					{
						szText.append(L" ");
						szText.append(1, (TCHAR)(_nPlayer + '1'));
					}
				}
				else
				{
					size_t nLevel = Globals::GetCurrentLevel(_pChildDomain);
					size_t nInvaderLevel = Globals::GetInvaderLevel(nLevel);

					if (nInvaderLevel)
					{
						szText.format(L"LEVEL %lu", nInvaderLevel + 1);
					}
					else
					{
						szText = L"READY";
					}
				}

				ff::ISpriteFont *font = _font.GetObject();
				if (szText.size() && font)
				{
					float percent = sin(_stateCounter * ff::PI_F / Globals::GetAdvancesPerSecondF());
					ff::PointFloat scale(2 + percent * 2, 2 + percent * 2);
					ff::PointFloat textSize = font->MeasureText(szText, scale, ff::PointFloat(0, 0));
					ff::PointFloat pos((Globals::GetLevelSizeF() - textSize) / 2);

					// Put a shadow behind the text
					font->DrawText(render, szText, pos + ff::PointFloat(4, 4), scale, ff::PointFloat(0, 0), &DirectX::XMFLOAT4(0, 0, 0, percent));
					render->Flush();

					font->DrawText(render, szText, pos, scale, ff::PointFloat(0, 0), &DirectX::XMFLOAT4(1, 1, 1, percent));
				}

				_pRenderLayer->EndCustomRender(render);
			}
		}
		break;
	}
}
void PlayGameState::HandleEventStart(IEntityDomain *pDomain, ff::hash_t type)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);

	switch (type)
	{
	case CIE_PAUSE: // quick toggle pause ('P' key)
		if (_state == GS_PLAYING || pApp->IsGamePaused())
		{
			pApp->OnPause();
		}
		break;

	case CIE_PAUSE_START: // only go into "paused" state
		if (_state == GS_PLAYING && !pApp->IsGamePaused())
		{
			pApp->OnPause();
		}
		break;
	}
}

void PlayGameState::HandleEventStop(IEntityDomain *pDomain, ff::hash_t type)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);

	switch (type)
	{
	case GIE_QUIT:
		if (_state != GS_PLAYING)
		{
			OnQuit(pDomain);
		}
		else // ESC key always toggles pause
		{
			pApp->OnPause();
		}
		break;

	case GIE_BACK:
		if (_state == GS_GAME_OVER)
		{
			OnQuit(pDomain);
		}
	}
}

bool PlayGameState::DidCompleteLevel()
{
#ifdef _DEBUG
	if (ff::MetroGlobals::Get()->GetKeys()->GetKey('0'))
	{
		return true;
	}
#endif

	return _playLevel[_nPlayer]->DidCompleteLevel();
}

void PlayGameState::SetState(EGameState state)
{
	if (state != _state)
	{
		_state = state;
		_stateCounter = 0;
	}
}

bool PlayGameState::CreatePlayLevel(size_t nPlayer)
{
	SetCurrentPlayer(nPlayer);

	if (_playLevel[nPlayer])
	{
		_pChildDomain->GetSystemManager()->RemoveAllSystems();
		_playLevel[nPlayer] = nullptr;
	}

	ff::ComPtr<IPlayLevelState> pPlayLevel;
	assertRetVal(_pChildDomain->GetSystemManager()->CreateSystem<IPlayLevelState>(&pPlayLevel), false);
	assertRetVal(_playLevel[nPlayer].QueryFrom(pPlayLevel), false);

	return true;
}

void PlayGameState::SetCurrentPlayer(size_t nPlayer)
{
	_nPlayer = nPlayer;

	if (_globalPlayers->GetGameMode() == GAME_MODE_COOP)
	{
		_globalPlayers->SetCurrentPlayers(0, 2);
	}
	else
	{
		_globalPlayers->SetCurrentPlayers(nPlayer, 1);
	}
}

bool PlayGameState::OnCompletedLevel()
{
	// Advance every player by one level
	PlayerGlobals *currentPlayer = nullptr;
	size_t currentPlayerIndex = _globalPlayers->GetCurrentPlayer(0);

	for (size_t i = 0; i < _globalPlayers->GetCurrentPlayerCount(); i++)
	{
		currentPlayer = _globalPlayers->GetCurrentPlayerGlobals(i);
		currentPlayer->SetLevel(currentPlayer->GetLevel() + 1);
	}

	if (_globalPlayers->GetCoopGlobals())
	{
		currentPlayer = _globalPlayers->GetCoopGlobals();
		size_t nLevel = currentPlayer->GetLevel();
		currentPlayer->SetLevel(nLevel + 1);
	}

	// Create a new level

	ff::ComPtr<PlayLevelState, IPlayLevelState> pOldPlayLevel = _playLevel[_nPlayer];
	assertRetVal(CreatePlayLevel(_nPlayer), false);

	pOldPlayLevel->OnStopPlaying();
	_playLevel[_nPlayer]->OnStartPlaying();

	// Transition between the old and new level

	ff::ComPtr<ISystem> pTransitionSystem;
	assertRetVal(TransitionState::Create(
		_pChildDomain, pOldPlayLevel, _playLevel[_nPlayer], __uuidof(IPlayLevelState),
		TRANSITION_FADE_TO_COLOR, &ff::GetColorWhite(), Globals::GetTransitionTime(), &pTransitionSystem), false);

	ff::ComPtr<TransitionState> pTransition;
	pTransition.QueryFrom(pTransitionSystem);
	pTransition->SetAdvance(true, false);

	_pChildDomain->GetSystemManager()->AddSystem(__uuidof(ITransitionState), pTransition);

	SetState(GS_BETWEEN_LEVELS);

	return true;
}

bool PlayGameState::CheckDeath(IEntityDomain *pDomain)
{
	ff::ComPtr<IPlayerService> pPlayers;

	if (GetService(_playLevel[_nPlayer]->GetChildDomain(), &pPlayers))
	{
		if (pPlayers->AreAllPlayersOutOfLives())
		{
			SetState(GS_CHECK_HIGH_SCORE);
			return true;
		}

		if (pPlayers->AreAllPlayersDead())
		{
			SwitchPlayers();
			return true;
		}

		if (CheckRequests(pDomain))
		{
			return true;
		}
	}

	return false;
}

bool PlayGameState::CheckRequests(IEntityDomain *pDomain)
{
	ff::ComPtr<IPlayerService> pPlayers;

	if (GetService(_playLevel[_nPlayer]->GetChildDomain(), &pPlayers))
	{
		if (_globalPlayers->WasRestartRequested())
		{
			ThisApplication *pApp = ThisApplication::Get(pDomain);
			pApp->UnpauseGame();

			RestartGame(pDomain);
			return true;
		}

		if (_globalPlayers->WasQuitRequested())
		{
			OnQuit(pDomain);
			return true;
		}
	}

	return false;
}

bool PlayGameState::CheckHighScore(IEntityDomain *pDomain)
{
	assertRetVal(pDomain->GetSystemManager()->HasSystem(this), false);

	ThisApplication* pApp = ThisApplication::Get(pDomain);
	bool isCoop = (_globalPlayers->GetGameMode() == GAME_MODE_COOP);
	PlayerGlobals* pPlayerGlobals = _globalPlayers->GetPlayerGlobals(_nPlayer, true);

	if (pApp->GetHighScores().IsHighScore(isCoop, *pPlayerGlobals))
	{
		ff::ComPtr<ISystem> pScoreState;
		assertRetVal(AddScoreState::Create(pDomain, __uuidof(IPlayGameState), this, _nPlayer, pPlayerGlobals, &pScoreState), false);

		pDomain->GetSystemManager()->RemoveSystem(this);
		pDomain->GetSystemManager()->AddSystem(__uuidof(IAddScoreState), pScoreState);

		return true;
	}

	return false;
}

void PlayGameState::SwitchPlayers()
{
	bool bGameOver = true;

	if (_globalPlayers->GetGameMode() == GAME_MODE_TURNS)
	{
		size_t nOtherPlayer = _nPlayer ? 0 : 1;
		PlayerGlobals* pOtherGlobals = _globalPlayers->GetPlayerGlobals(nOtherPlayer);

		if (pOtherGlobals->IsActive() || pOtherGlobals->GetLives())
		{
			ff::ComPtr<PlayLevelState, IPlayLevelState> pOldPlayLevel = _playLevel[_nPlayer];
			ff::ComPtr<ISystem> pTransition;

			SetCurrentPlayer(nOtherPlayer);

			pOldPlayLevel->OnStopPlaying();
			_playLevel[_nPlayer]->OnStartPlaying();

			if (TransitionState::Create(
				_pChildDomain, pOldPlayLevel, _playLevel[_nPlayer], __uuidof(IPlayLevelState),
				TRANSITION_FADE_TO_COLOR, &ff::GetColorBlack(), Globals::GetTransitionTime() * 2, &pTransition))
			{
				_pChildDomain->GetSystemManager()->RemoveSystem(pOldPlayLevel);
				_pChildDomain->GetSystemManager()->AddSystem(__uuidof(ITransitionState), pTransition);

				SetState(GS_SWITCHING_PLAYERS);

				bGameOver = false;
			}
		}
	}

	ff::ComPtr<IPlayerService> pPlayers;

	if (bGameOver &&
		GetService(_playLevel[_nPlayer]->GetChildDomain(), &pPlayers) &&
		!pPlayers->AreAllPlayersOutOfLives())
	{
		// Can't switch players, but the current player still has lives
		bGameOver = false;
	}

	if (bGameOver)
	{
		SetState(GS_GAME_OVER);
	}
}

void PlayGameState::RestartGame(IEntityDomain *pDomain)
{
	assertRet(pDomain->GetSystemManager()->HasSystem(this));

	ff::ComPtr<ISystem> pTransition;
	ff::ComPtr<ISystem> pPlayGame;

	assertRet(pDomain->GetSystemManager()->CreateSystem(__uuidof(IPlayGameState), &pPlayGame));

	assertRet(TransitionState::Create(
		pDomain, this, pPlayGame, __uuidof(IPlayGameState),
		TRANSITION_FADE_TO_COLOR, &ff::GetColorWhite(), Globals::GetTransitionTime() * 2, &pTransition));

	pDomain->GetSystemManager()->RemoveSystem(this);
	pDomain->GetSystemManager()->AddSystem(__uuidof(ITransitionState), pTransition);
}

void PlayGameState::OnQuit(IEntityDomain *pDomain)
{
	// Go to the title screen
	ff::ComPtr<ISystem> pTitleSystem;
	ff::ComPtr<ISystem> pTransition;
	ff::ComPtr<TitleState, ITitleState> pTitle;

	assertRet(pDomain->GetSystemManager()->HasSystem(this));
	assertRet(pDomain->GetSystemManager()->CreateSystem(__uuidof(ITitleState), &pTitleSystem));

	if (_state != GS_GAME_OVER && pTitle.QueryFrom(pTitleSystem))
	{
		// can continue playing later
		pTitle->SetActiveGame(this);
	}
	else
	{
		_playLevel[_nPlayer]->OnStopPlaying();
	}

	assertRet(TransitionState::Create(
		pDomain, this, pTitleSystem, __uuidof(ITitleState),
		TRANSITION_WIPE_HORIZONTAL, nullptr, Globals::GetTransitionTime(), &pTransition));

	pDomain->GetSystemManager()->RemoveSystem(this);
	pDomain->GetSystemManager()->AddSystem(__uuidof(ITransitionState), pTransition);
}
