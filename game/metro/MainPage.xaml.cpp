#include "pch.h"	
#include "App.xaml.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals\MetroGlobals.h"
#include "metro\AboutSettings.xaml.h"
#include "metro\MainPage.xaml.h"
#include "metro\ControlSettings.xaml.h"
#include "metro\GameSettings.xaml.h"
#include "services\GlobalPlayerService.h"
#include "states\PlayGameState.h"
#include "Thread\ThreadDispatch.h"

Invader::MainPage::MainPage()
	: _showingSettings(false)
{
	InitializeComponent();
}

Invader::MainPage::~MainPage()
{
}

bool Invader::MainPage::IsSettingsOpen()
{
	return _showingSettings;
}

void Invader::MainPage::CloseSettings()
{
	ff::GetMainThreadDispatch()->Post([this]
	{
		SettingsPopup->IsOpen = false;
	}, true);
}

void Invader::MainPage::ShowAboutSettingsPane()
{
	ShowSettings([] { return ref new AboutSettings(); });
}

void Invader::MainPage::ShowControlSettingsPane()
{
	ShowSettings([] { return ref new ControlSettings(); });
}

void Invader::MainPage::ShowGameSettingsPane()
{
	ShowSettings([] { return ref new GameSettings(); });
}

Windows::UI::Xaml::Controls::Page ^Invader::MainPage::GetCurrentSettingsPane()
{
	assertRetVal(ff::GetMainThreadDispatch()->IsCurrentThread(), nullptr);

	if (SettingsPopup->IsOpen && SettingsPopup->Child != nullptr)
	{
		return dynamic_cast<Windows::UI::Xaml::Controls::Page ^>(SettingsPopup->Child);
	}

	return nullptr;
}

void Invader::MainPage::StartWaiting()
{
	ff::GetMainThreadDispatch()->Post([this]
	{
		Windows::UI::Xaml::VisualStateManager::GoToState(this, WaitingState->Name, true);
	}, true);
}

void Invader::MainPage::StopWaiting()
{
	ff::GetMainThreadDispatch()->Post([this]
	{
		Windows::UI::Xaml::VisualStateManager::GoToState(this, NotWaitingState->Name, true);
	}, true);
}

void Invader::MainPage::OnSettingsPopupOpened(Platform::Object ^sender, Platform::Object ^arg)
{
	_showingSettings = true;

	ff::MetroGlobals::Get()->GetGameDispatch()->Post([]
	{
		if (App::InvaderApp)
		{
			App::InvaderApp->OnAppBarOpened();
		}
	}, true);
}

void Invader::MainPage::OnSettingsPopupClosed(Platform::Object ^sender, Platform::Object ^arg)
{
	_showingSettings = false;

	ff::GetMainThreadDispatch()->Post([=]
	{
		if (!SettingsPopup->IsOpen)
		{
			// Delete the old popup from memory
			SettingsPopup->Child = nullptr;
		}
	});
}

void Invader::MainPage::ShowSettings(std::function<Windows::UI::Xaml::Controls::Page ^()> pageFactory)
{
	ff::GetMainThreadDispatch()->Post([=]
	{
		Windows::UI::Xaml::Controls::Page ^page = pageFactory();
		assertRet(page);

		SettingsPopup->Width = 346;
		SettingsPopup->Height = Windows::UI::Xaml::Window::Current->Bounds.Height;
		SettingsPopup->HorizontalAlignment = Windows::UI::Xaml::HorizontalAlignment::Right;
		SettingsPopupTransition->Edge = Windows::UI::Xaml::Controls::Primitives::EdgeTransitionLocation::Right;

		page->Width = SettingsPopup->Width;
		page->Height = SettingsPopup->Height;
		SettingsPopup->Child = page;
		SettingsPopup->IsOpen = true;
	}, true);
}
