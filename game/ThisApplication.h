#pragma once

#include "Dict\Dict.h"
#include "Scores.h"
#include "State\State.h"

namespace ff
{
	class IInputMapping;
}

class IEntityDomain;
class IEntityDomainProvider;

class ThisApplication : public ff::State
{
public:
	ThisApplication();
	virtual ~ThisApplication();

	static ThisApplication *Get(IEntityDomainProvider *pDomainProvider);

	// Options
	static ff::StaticString OPTION_DIFFICULTY;
	static ff::StaticString OPTION_FULL_SCREEN;
	static ff::StaticString OPTION_GAME_MODE;
	static ff::StaticString OPTION_HIGH_SCORES;
	static ff::StaticString OPTION_SOUND_ON;
	static ff::StaticString OPTION_FPS_ON;
	static ff::StaticString OPTION_WINDOW_PADDING;

	// State
	virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals *context) override;
	virtual void Render(ff::AppGlobals *context, ff::IRenderTarget *target) override;
	virtual void SaveState(ff::AppGlobals *context) override;
	virtual void LoadState(ff::AppGlobals *context) override;

	IEntityDomain *GetEntityDomain();
	HighScores &GetHighScores();
	ff::Dict &GetOptions();
	void AdvanceInputMapping(ff::IInputMapping *pMapping, bool allowWhilePaused = false);

	// Pausing
	void OnAppBarOpened();
	bool IsGamePaused() const;
	void OnPause(bool closeAppBars = true);
	void PauseGame();
	void UnpauseGame(bool closeAppBars = true);
	bool AllowGameAdvanceWhilePaused();
	bool DidAllowGameAdvanceWhilePaused() const;

private:
	void ToggleGamePaused();
	void DebugAdvanceWhilePaused();

	HighScores _highScores;
	ff::Dict _options;
	ff::ComPtr<ff::IInputMapping> _inputMap;
	ff::ComPtr<IEntityDomain> _domain;
	bool _paused;
	bool _allowPauseAdvance;
	bool _didAllowPauseAdvance;
};
