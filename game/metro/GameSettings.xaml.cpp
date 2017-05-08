#include "pch.h"
#include "App.xaml.h"
#include "Audio\AudioDevice.h"
#include "Audio\AudioEffect.h"
#include "Globals\MetroGlobals.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "metro\GameSettings.xaml.h"
#include "metro\MainPage.xaml.h"
#include "ThisApplication.h"
#include "Thread\ThreadDispatch.h"

Invader::GameSettings::GameSettings()
	: _loaded(false)
{
	InitializeComponent();

	_effect.Init(L"Menu Sub");
}

Invader::GameSettings::~GameSettings()
{
	ff::MetroGlobals::Get()->GetGameDispatch()->Post([]
	{
		ff::MetroGlobals::Get()->SaveState();
	}, true);
}

void Invader::GameSettings::OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	UpdateControls();
	this->GameModeCombo->Focus(Windows::UI::Xaml::FocusState::Programmatic);

	_loaded = true;
}

void Invader::GameSettings::OnBackClicked(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	App::Page->CloseSettings();
}

void Invader::GameSettings::OnSoundEffectToggled(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	UpdateSettings();
	PlayTestSound();
}

void Invader::GameSettings::OnFullScreenToggled(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	UpdateSettings();
}

void Invader::GameSettings::OnVolumeChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs ^args)
{
	UpdateSettings();
	PlayTestSound();
}

void Invader::GameSettings::OnGameModeChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^args)
{
	UpdateSettings();
}

void Invader::GameSettings::OnDifficultyChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^args)
{
	UpdateSettings();
}

void Invader::GameSettings::OnControlsClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	App::Page->ShowControlSettingsPane();
}

void Invader::GameSettings::OnAboutClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	App::Page->ShowAboutSettingsPane();
}

static Difficulty SelectedIndexToDifficulty(int index)
{
	switch (index)
	{
	//case 0: return DIFFICULTY_BABY;
	case 0: return DIFFICULTY_EASY;
	case 1: return DIFFICULTY_NORMAL;
	case 2: return DIFFICULTY_HARD;
	default: assert(false); return DIFFICULTY_NORMAL;
	}
}

static int DifficultyToSelectedIndex(Difficulty diff)
{
	switch (diff)
	{
	//case DIFFICULTY_BABY: return 0;
	case DIFFICULTY_EASY: return 0;
	case DIFFICULTY_NORMAL: return 1;
	case DIFFICULTY_HARD: return 2;
	default: assert(false); return 1;
	}
}

void Invader::GameSettings::UpdateSettings()
{
	if (_loaded)
	{
		ff::MetroGlobals *globals = ff::MetroGlobals::Get();
		bool soundEffectToggle = SoundEffectToggle->IsOn;
		bool fullScreenToggle = FullScreenToggle->IsOn;
		int gameModeCombo = GameModeCombo->SelectedIndex;
		int difficultyCombo = DifficultyCombo->SelectedIndex;
		double volumeSlider = VolumeSlider->Value;

		globals->GetGameDispatch()->Post([=]
		{
			ThisApplication *pApp = App::InvaderApp;
			if (pApp != nullptr)
			{
				pApp->GetOptions().SetBool(ThisApplication::OPTION_SOUND_ON, soundEffectToggle);
				pApp->GetOptions().SetBool(ThisApplication::OPTION_FULL_SCREEN, fullScreenToggle);
				pApp->GetOptions().SetInt(ThisApplication::OPTION_GAME_MODE, gameModeCombo);
				pApp->GetOptions().SetInt(ThisApplication::OPTION_DIFFICULTY, SelectedIndexToDifficulty(difficultyCombo));
			}

			if (globals->GetAudio() != nullptr)
			{
				float volume = (float)(volumeSlider / 100.0);
				globals->GetAudio()->SetVolume(ff::AudioVoiceType::MASTER, volume);
			}

			if (globals->GetTarget() && globals->GetTarget()->IsFullScreen() != fullScreenToggle)
			{
				globals->GetTarget()->SetFullScreen(fullScreenToggle);
			}
		});

		UpdateControlsEnabled();
	}
}

void Invader::GameSettings::UpdateControls()
{
	ff::MetroGlobals *globals = ff::MetroGlobals::Get();
	bool soundOn = true;
	bool fullScreenOn = false;
	double volume = 1.0;
	int gameMode = GAME_MODE_DEFAULT;
	int difficulty = DIFFICULTY_DEFAULT;

	globals->GetGameDispatch()->Send([&]
	{
		ThisApplication *pApp = App::InvaderApp;
		soundOn = pApp && pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);
		fullScreenOn = globals->GetTarget() && globals->GetTarget()->IsFullScreen();

		volume = globals->GetAudio() != nullptr
			? globals->GetAudio()->GetVolume(ff::AudioVoiceType::MASTER) * 100.0
			: 1.0;

		gameMode = pApp != nullptr
			? pApp->GetOptions().GetInt(ThisApplication::OPTION_GAME_MODE, GAME_MODE_DEFAULT)
			: GAME_MODE_DEFAULT;

		difficulty = pApp != nullptr
			? DifficultyToSelectedIndex(pApp->GetOptions().GetEnum<Difficulty>(ThisApplication::OPTION_DIFFICULTY, DIFFICULTY_DEFAULT))
			: DIFFICULTY_DEFAULT;

		volume = std::max(0.0, volume);
		volume = std::min(100.0, volume);
	});

	if (SoundEffectToggle->IsOn != soundOn)
	{
		SoundEffectToggle->IsOn = soundOn;
	}

	if (FullScreenToggle->IsOn != fullScreenOn)
	{
		FullScreenToggle->IsOn = fullScreenOn;
	}

	if (VolumeSlider->Value != volume)
	{
		VolumeSlider->Value = volume;
	}

	if (GameModeCombo->SelectedIndex != gameMode)
	{
		GameModeCombo->SelectedIndex = gameMode;
	}

	if (DifficultyCombo->SelectedIndex != difficulty)
	{
		DifficultyCombo->SelectedIndex = difficulty;
	}

	UpdateControlsEnabled();
}

void Invader::GameSettings::UpdateControlsEnabled()
{
	bool title = false;
	bool soundOn = false;

	ff::MetroGlobals::Get()->GetGameDispatch()->Send([&]
	{
		title = App::IsOnTitleScreen;

		ThisApplication *pApp = App::InvaderApp;
		soundOn = pApp && pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);
	});

	VolumeHeader->Opacity = soundOn ? 1.0 : 0.5;
	VolumeSlider->IsEnabled = soundOn;

	DisabledHeaderPanel->Visibility = title
		? Windows::UI::Xaml::Visibility::Collapsed
		: Windows::UI::Xaml::Visibility::Visible;

	GameModeCombo->Visibility = title
		? Windows::UI::Xaml::Visibility::Visible
		: Windows::UI::Xaml::Visibility::Collapsed;

	GameModeText->Visibility = title
		? Windows::UI::Xaml::Visibility::Collapsed
		: Windows::UI::Xaml::Visibility::Visible;

	DifficultyCombo->Visibility = title
		? Windows::UI::Xaml::Visibility::Visible
		: Windows::UI::Xaml::Visibility::Collapsed;

	DifficultyText->Visibility = title
		? Windows::UI::Xaml::Visibility::Collapsed
		: Windows::UI::Xaml::Visibility::Visible;
}

void Invader::GameSettings::PlayTestSound()
{
	ff::ComPtr<ff::IAudioEffect> effect = _effect.GetObject();
	if (_loaded && effect)
	{
		ff::MetroGlobals::Get()->GetGameDispatch()->Post([effect]
		{
			ThisApplication *pApp = App::InvaderApp;
			bool soundOn = pApp && pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);

			if (soundOn && !effect->IsPlaying())
			{
				effect->Play();
			}
		});
	}
}
