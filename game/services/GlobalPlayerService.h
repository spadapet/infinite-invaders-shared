#pragma once

#include "Scores.h"

enum GameMode;
enum Difficulty;

class __declspec(uuid("612806ea-3ff9-4e35-8498-3d39e7d4896a"))
	GlobalPlayerService : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(GlobalPlayerService);

	REFGUID GetGameId() const;
	GameMode GetGameMode() const;
	Difficulty GetDifficulty() const;
	void RequestQuit();
	bool WasQuitRequested();
	void RequestRestart();
	bool WasRestartRequested();

	// All possible players
	size_t GetPlayerCount() const;
	PlayerGlobals* GetPlayerGlobals(size_t index, bool bUseCoop = false);
	PlayerGlobals* GetCoopGlobals();

	// Currently active players only
	size_t GetCurrentPlayerCount() const;
	size_t GetCurrentPlayer(size_t index) const;
	PlayerGlobals* GetCurrentPlayerGlobals(size_t index, bool bUseCoop = false);
	bool SetCurrentPlayers(size_t index, size_t nCount);

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	GUID _id;
	size_t _nCurrentPlayer;
	size_t _nCurrentPlayerCount;
	GameMode _gameMode;
	Difficulty _difficulty;
	PlayerGlobals _playerGlobals[2];
	PlayerGlobals _coopGlobals;
	bool _bQuit;
	bool _bRestart;
};
