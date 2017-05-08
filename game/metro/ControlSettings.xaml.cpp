#include "pch.h"
#include "App.xaml.h"
#include "Globals\MetroGlobals.h"
#include "metro\ControlSettings.xaml.h"
#include "metro\GameSettings.xaml.h"
#include "metro\MainPage.xaml.h"
#include "Thread\ThreadDispatch.h"

Invader::ControlSettings::ControlSettings()
	: _fromGameSettings(dynamic_cast<GameSettings ^>(Invader::App::Page->GetCurrentSettingsPane()) != nullptr)
	, _loaded(false)
{
	InitializeComponent();
}

Invader::ControlSettings::~ControlSettings()
{
	ff::MetroGlobals::Get()->GetGameDispatch()->Post([]
	{
		ff::MetroGlobals::Get()->SaveState();
	}, true);
}

void Invader::ControlSettings::OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	UpdateControls();

	this->GameModeCombo->Focus(Windows::UI::Xaml::FocusState::Programmatic);

	_loaded = true;
}

void Invader::ControlSettings::OnBackClicked(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	if (_fromGameSettings)
	{
		Invader::App::Page->ShowGameSettingsPane();
	}
	else
	{
		Invader::App::Page->CloseSettings();
	}
}

void Invader::ControlSettings::OnGameModeChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^args)
{
	UpdateSettings();
}

void Invader::ControlSettings::UpdateSettings()
{
	if (_loaded)
	{
		int selectedIndex = GameModeCombo->SelectedIndex;

		ff::MetroGlobals::Get()->GetGameDispatch()->Post([selectedIndex]
		{
			if (Invader::App::InvaderApp)
			{
				Invader::App::InvaderApp->GetOptions().SetInt(ThisApplication::OPTION_GAME_MODE, selectedIndex);
			}
		});

		UpdateControlsEnabled();
		UpdateVisualState();
	}
}

void Invader::ControlSettings::UpdateControls()
{
	int gameMode = GAME_MODE_DEFAULT;

	ff::MetroGlobals::Get()->GetGameDispatch()->Send([&gameMode]
	{
		if (Invader::App::InvaderApp)
		{
			gameMode = Invader::App::InvaderApp->GetOptions().GetInt(ThisApplication::OPTION_GAME_MODE, GAME_MODE_DEFAULT);
		}
	});

	if (GameModeCombo->SelectedIndex != gameMode)
	{
		GameModeCombo->SelectedIndex = gameMode;
	}

	UpdateControlsEnabled();
	UpdateVisualState();
}

void Invader::ControlSettings::UpdateControlsEnabled()
{
	bool title = false;

	ff::MetroGlobals::Get()->GetGameDispatch()->Send([&title]
	{
		title = Invader::App::IsOnTitleScreen;
	});

	GameModeCombo->Visibility = title
		? Windows::UI::Xaml::Visibility::Visible
		: Windows::UI::Xaml::Visibility::Collapsed;

	GameModeText->Visibility = title
		? Windows::UI::Xaml::Visibility::Collapsed
		: Windows::UI::Xaml::Visibility::Visible;

	GameModeCombo->IsEnabled = title;
}

void Invader::ControlSettings::UpdateVisualState()
{
	GameMode gameMode = GAME_MODE_DEFAULT;

	ff::MetroGlobals::Get()->GetGameDispatch()->Send([&gameMode]
	{
		if (Invader::App::InvaderApp)
		{
			gameMode = Invader::App::InvaderApp->GetOptions().GetEnum<GameMode>(ThisApplication::OPTION_GAME_MODE, GAME_MODE_DEFAULT);
		}
	});

	switch (gameMode)
	{
	case GAME_MODE_SINGLE:
		Windows::UI::Xaml::VisualStateManager::GoToState(this, GameMode0->Name, true);
		break;

	case GAME_MODE_TURNS:
		Windows::UI::Xaml::VisualStateManager::GoToState(this, GameMode1->Name, true);
		break;

	case GAME_MODE_COOP:
		Windows::UI::Xaml::VisualStateManager::GoToState(this, GameMode2->Name, true);
		break;
	}
}
