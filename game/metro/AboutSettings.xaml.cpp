#include "pch.h"
#include "App.xaml.h"
#include "metro\AboutSettings.xaml.h"
#include "metro\GameSettings.xaml.h"
#include "metro\MainPage.xaml.h"

Invader::AboutSettings::AboutSettings()
	: _fromGameSettings(dynamic_cast<GameSettings ^>(Invader::App::Page->GetCurrentSettingsPane()) != nullptr)
{
	InitializeComponent();

	auto package = Windows::ApplicationModel::Package::Current;
	auto id = package->Id;

	GameNameText->Text = package->DisplayName;
	GameByText->Text = package->PublisherDisplayName;
	GameVersionText->Text = "Version " + id->Version.Major.ToString() + "." + id->Version.Minor.ToString();
}

void Invader::AboutSettings::OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	this->FirstLink->Focus(Windows::UI::Xaml::FocusState::Programmatic);
}

void Invader::AboutSettings::OnBackClicked(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
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
