#include "pch.h"
#include "App.xaml.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\system\SystemManager.h"
#include "Data\Data.h"
#include "entities\EntityEvents.h"
#include "Globals\MetroGlobals.h"
#include "Globals.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Input\InputMapping.h"
#include "InputEvents.h"
#include "metro\MainPage.xaml.h"
#include "states\InitState.h"
#include "ThisApplication.h"
#include "Thread\ThreadDispatch.h"

ff::StaticString ThisApplication::OPTION_DIFFICULTY(L"Difficulty");
ff::StaticString ThisApplication::OPTION_FULL_SCREEN(L"FullScreen");
ff::StaticString ThisApplication::OPTION_GAME_MODE(L"GameMode");
ff::StaticString ThisApplication::OPTION_HIGH_SCORES(L"HighScores");
ff::StaticString ThisApplication::OPTION_SOUND_ON(L"SoundOn");
ff::StaticString ThisApplication::OPTION_WINDOW_PADDING(L"WindowPadding");

enum GameAppEvent
{
	GE_DEBUG_ADVANCE,
	GE_DEBUG_FAST_ADVANCE,
	GE_DEBUG_SLOW_ADVANCE,
	GE_DEBUG_CONSOLE,
};

static const ff::InputEventMapping s_appInputEvents[] =
{
	// Debug
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, '1' } }, GE_DEBUG_ADVANCE, 0, 0.25 },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_OEM_3 } }, GE_DEBUG_CONSOLE }, // tilde
};

static const ff::InputValueMapping s_appInputValues[] =
{
	// Debug
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, '1' }, GE_DEBUG_FAST_ADVANCE },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, '2' }, GE_DEBUG_SLOW_ADVANCE },
};

ThisApplication::ThisApplication()
	: _paused(false)
	, _allowPauseAdvance(false)
	, _didAllowPauseAdvance(false)
{
	CreateInputMapping(true, false, false, &_inputMap);
	_inputMap->MapEvents(s_appInputEvents, _countof(s_appInputEvents));
	_inputMap->MapValues(s_appInputValues, _countof(s_appInputValues));

	CreateEntityDomain(this, &_domain);
	_domain->GetSystemManager()->EnsureSystem<IInitState>();
}

ThisApplication::~ThisApplication()
{
}

// static
ThisApplication *ThisApplication::Get(IEntityDomainProvider *pDomainProvider)
{
	return pDomainProvider->GetDomain()->GetApp();
}

std::shared_ptr<ff::State> ThisApplication::Advance(ff::AppGlobals *context)
{
	AdvanceInputMapping(_inputMap);

	for (const ff::InputEvent &ie : _inputMap->GetEvents())
	{
		if (ie.IsStart() || ie.IsRepeat())
		{
			switch (ie._eventID)
			{
			case GE_DEBUG_ADVANCE:
				DebugAdvanceWhilePaused();
				break;
			}
		}
	}

	_domain->GetSystemManager()->Advance();

	return nullptr;
}

void ThisApplication::Render(ff::AppGlobals *context, ff::IRenderTarget *target)
{
	_domain->GetSystemManager()->Render(target);
}

void ThisApplication::SaveState(ff::AppGlobals *context)
{
	if (context->GetTarget())
	{
		bool fullScreen = context->GetTarget()->IsFullScreen();
		_options.SetBool(OPTION_FULL_SCREEN, fullScreen);
	}

	ff::ComPtr<ff::IDataVector> highScoreData;
	if (ff::CreateDataVector(0, &highScoreData))
	{
		highScoreData->GetVector().Push((const BYTE *)&_highScores, sizeof(_highScores));
		_options.SetData(OPTION_HIGH_SCORES, highScoreData);
	}

	ff::MetroGlobals::Get()->SetState(_options);
}

void ThisApplication::LoadState(ff::AppGlobals *context)
{
	_options = ff::MetroGlobals::Get()->GetState();

	ff::ComPtr<ff::IData> highScoreData = _options.GetData(OPTION_HIGH_SCORES);
	if (highScoreData && highScoreData->GetSize() == sizeof(_highScores))
	{
		::memcpy(&_highScores, highScoreData->GetMem(), highScoreData->GetSize());
	}
}

IEntityDomain *ThisApplication::GetEntityDomain()
{
	return _domain;
}

HighScores &ThisApplication::GetHighScores()
{
	assert(!ff::GetMainThreadDispatch()->IsCurrentThread());
	return _highScores;
}

ff::Dict &ThisApplication::GetOptions()
{
	assert(!ff::GetMainThreadDispatch()->IsCurrentThread());
	return _options;
}

void ThisApplication::OnAppBarOpened()
{
	if (Invader::App::IsOnGameScreen)
	{
		PauseGame();
	}
}

bool ThisApplication::IsGamePaused() const
{
	return _paused;
}

void ThisApplication::OnPause(bool closeAppBars)
{
	ToggleGamePaused();

	if (closeAppBars && !_paused)
	{
		Invader::App::Page->CloseSettings();
	}
}

void ThisApplication::PauseGame()
{
	if (!_paused)
	{
		OnPause();
	}
}

void ThisApplication::UnpauseGame(bool closeAppBars)
{
	if (_paused)
	{
		OnPause(closeAppBars);
	}
}

bool ThisApplication::AllowGameAdvanceWhilePaused()
{
	if (_allowPauseAdvance)
	{
		_allowPauseAdvance = false;
		_didAllowPauseAdvance = true;

		return true;
	}

	return false;
}

bool ThisApplication::DidAllowGameAdvanceWhilePaused() const
{
	return _didAllowPauseAdvance;
}

void ThisApplication::AdvanceInputMapping(ff::IInputMapping *pMapping, bool allowWhilePaused)
{
	assertRet(pMapping);

	if (!allowWhilePaused && Invader::App::Page->IsSettingsOpen())
	{
		pMapping->ClearEvents();
	}
	else
	{
		double time = ff::MetroGlobals::Get()->GetGlobalTime()._secondsPerAdvance;
		pMapping->Advance(time);
	}
}

void ThisApplication::ToggleGamePaused()
{
	_paused = !_paused;
	_allowPauseAdvance = false;
	_didAllowPauseAdvance = false;
}

void ThisApplication::DebugAdvanceWhilePaused()
{
#ifdef _DEBUG
	_allowPauseAdvance = IsGamePaused();
#endif
}
