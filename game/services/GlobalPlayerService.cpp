#include "pch.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "Module\ModuleFactory.h"
#include "Scores.h"
#include "services\GlobalPlayerService.h"
#include "ThisApplication.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"GlobalPlayerService");
	module.RegisterClassT<GlobalPlayerService>(name);
});

BEGIN_INTERFACES(GlobalPlayerService)
END_INTERFACES()

GlobalPlayerService::GlobalPlayerService()
	: _nCurrentPlayer(0)
	, _nCurrentPlayerCount(1)
	, _gameMode(GAME_MODE_SINGLE)
	, _difficulty(DIFFICULTY_NORMAL)
	, _bQuit(false)
	, _bRestart(false)
	, _id(ff::CreateGuid())
{
}

GlobalPlayerService::~GlobalPlayerService()
{
}

HRESULT GlobalPlayerService::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	ThisApplication* pApp = ThisApplication::Get(pDomainProvider);

	_gameMode = pApp->GetOptions().GetEnum<GameMode>(ThisApplication::OPTION_GAME_MODE, GAME_MODE_DEFAULT);
	_difficulty = pApp->GetOptions().GetEnum<Difficulty>(ThisApplication::OPTION_DIFFICULTY, DIFFICULTY_DEFAULT);

	for (size_t i = 0; i < _countof(_playerGlobals); i++)
	{
		_playerGlobals[i].SetDifficulty(_difficulty);
		_playerGlobals[i].SetGameMode(_gameMode);
		_playerGlobals[i].SetActive(i < GetPlayerCount());

		if (_gameMode != GAME_MODE_COOP)
		{
			_playerGlobals[i].SetLives(Globals::GetStartingLives(_gameMode, _difficulty));

			_playerGlobals[i].SetFreeLifeScore(
				Globals::GetFirstFreeLife(_gameMode, _difficulty),
				Globals::GetNextFreeLife(_gameMode, _difficulty));
		}
	}

	if (_gameMode == GAME_MODE_COOP)
	{
		_coopGlobals.SetDifficulty(_difficulty);
		_coopGlobals.SetGameMode(_gameMode);
		_coopGlobals.SetLives(Globals::GetStartingLives(_gameMode, _difficulty));

		_coopGlobals.SetFreeLifeScore(
			Globals::GetFirstFreeLife(_gameMode, _difficulty),
			Globals::GetNextFreeLife(_gameMode, _difficulty));
	}

	return __super::_Construct(unkOuter);
}

REFGUID GlobalPlayerService::GetGameId() const
{
	return _id;
}

GameMode GlobalPlayerService::GetGameMode() const
{
	return _gameMode;
}

Difficulty GlobalPlayerService::GetDifficulty() const
{
	return _difficulty;
}

size_t GlobalPlayerService::GetPlayerCount() const
{
	return (_gameMode == GAME_MODE_SINGLE) ? 1 : 2;
}

PlayerGlobals *GlobalPlayerService::GetPlayerGlobals(size_t index, bool bUseCoop)
{
	if (bUseCoop && _gameMode == GAME_MODE_COOP)
	{
		return &_coopGlobals;
	}
	else
	{
		assertRetVal(index >= 0 && index < GetPlayerCount(), nullptr);
		return &_playerGlobals[index];
	}
}

PlayerGlobals *GlobalPlayerService::GetCoopGlobals()
{
	return (_gameMode == GAME_MODE_COOP) ? &_coopGlobals : nullptr;
}

size_t GlobalPlayerService::GetCurrentPlayerCount() const
{
	return _nCurrentPlayerCount;
}

size_t GlobalPlayerService::GetCurrentPlayer(size_t index) const
{
	assertRetVal(index < _nCurrentPlayerCount, 0);
	return _nCurrentPlayer + index;
}

PlayerGlobals *GlobalPlayerService::GetCurrentPlayerGlobals(size_t index, bool bUseCoop)
{
	if (bUseCoop && _gameMode == GAME_MODE_COOP)
	{
		return &_coopGlobals;
	}
	else
	{
		assertRetVal(index >= 0 && index < GetCurrentPlayerCount(), nullptr);
		return GetPlayerGlobals(GetCurrentPlayer(index));
	}
}

bool GlobalPlayerService::SetCurrentPlayers(size_t index, size_t nCount)
{
	assertRetVal(index >= 0 && index < GetPlayerCount(), false);
	assertRetVal(nCount > 0 && index + nCount <= GetPlayerCount(), false);

	_nCurrentPlayer = index;
	_nCurrentPlayerCount = nCount;

	return true;
}

void GlobalPlayerService::RequestQuit()
{
	_bQuit = true;
	_bRestart = false;
}

bool GlobalPlayerService::WasQuitRequested()
{
	bool bQuit = _bQuit;
	_bQuit = false;

	return bQuit;
}

void GlobalPlayerService::RequestRestart()
{
	_bQuit = false;
	_bRestart = true;
}

bool GlobalPlayerService::WasRestartRequested()
{
	bool bRestart = _bRestart;
	_bRestart = false;

	return bRestart;
}
