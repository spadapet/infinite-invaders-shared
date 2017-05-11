#pragma once

#include "coreEntity\system\System.h"
#include "Resource\ResourceValue.h"
#include "Types\Timer.h"

namespace ff
{
	class ISpriteFont;
}

class LoadingComponent;
class GlobalPlayerService;
class PlayLevelState;
class I2dLayerGroup;
class IPlayLevelState;

class __declspec(uuid("1858bdc0-4190-482c-86d0-6602879e1f08")) __declspec(novtable)
	IPlayGameState : public ISystem
{
public:
	virtual GlobalPlayerService *GetGlobalPlayerService() = 0;
	virtual void TrackStartGame() = 0;
};

class __declspec(uuid("84a6932a-4414-4104-8630-7977e8a6e70b"))
	PlayGameState : public ff::ComBase, public IPlayGameState
{
public:
	DECLARE_HEADER(PlayGameState);

	// IPlayGameState
	virtual GlobalPlayerService *GetGlobalPlayerService() override;
	virtual void TrackStartGame() override;

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// ComBase
	HRESULT _Construct(IUnknown *unkOuter) override;

private:
	enum EGameState
	{
		GS_LOADING,
		GS_PLAYER_READY,
		GS_READY,
		GS_PLAYING,
		GS_BETWEEN_LEVELS,
		GS_SWITCHING_PLAYERS,
		GS_CHECK_HIGH_SCORE,
		GS_ENTER_HIGH_SCORE,
		GS_GAME_OVER,
	};

	void HandleEventStart(IEntityDomain *pDomain, ff::hash_t type);
	void HandleEventStop(IEntityDomain *pDomain, ff::hash_t type);
	bool DidCompleteLevel();
	void SetState(EGameState state);
	bool CreatePlayLevel(size_t nPlayer);
	void SetCurrentPlayer(size_t nPlayer);
	bool OnCompletedLevel();
	bool CheckDeath(IEntityDomain *pDomain);
	bool CheckRequests(IEntityDomain *pDomain);
	bool CheckHighScore(IEntityDomain *pDomain);
	void SwitchPlayers();
	void RestartGame(IEntityDomain *pDomain);
	void OnQuit(IEntityDomain *pDomain);

	EGameState _state;
	size_t _stateCounter;
	size_t _nPlayer;
	ff::ComPtr<ff::IInputMapping> _inputMapping;
	ff::ComPtr<IEntityDomain> _pChildDomain;
	ff::ComPtr<GlobalPlayerService> _globalPlayers;
	ff::ComPtr<LoadingComponent> _loading;
	ff::ComPtr<I2dLayerGroup> _pRenderLayer;
	ff::TypedResource<ff::ISpriteFont> _font;

	ff::ComPtr<PlayLevelState, IPlayLevelState> _playLevel[2];
};
