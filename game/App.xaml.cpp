#include "pch.h"
#include "App.xaml.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals\MetroGlobals.h"
#include "Globals\ProcessGlobals.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "metro\FailurePage.xaml.h"
#include "metro\MainPage.xaml.h"
#include "states\TitleState.h"
#include "states\PlayGameState.h"
#include "String\StringUtil.h"
#include "ThisApplication.h"
#include "Thread\ThreadDispatch.h"
#include "Windows\FileUtil.h"

Invader::App::App()
{
	InitializeComponent();
}

Invader::App::~App()
{
	if (_globals)
	{
		_globals->Shutdown();
		_globals.reset();
	}

	if (_processGlobals)
	{
		_processGlobals->Shutdown();
		_processGlobals.reset();
	}
}

void Invader::App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^args)
{
	InitializeProcess();

	switch (args->PreviousExecutionState)
	{
	case Windows::ApplicationModel::Activation::ApplicationExecutionState::Running:
	case Windows::ApplicationModel::Activation::ApplicationExecutionState::Suspended:
		// great, keep on running
		break;

	default:
	case Windows::ApplicationModel::Activation::ApplicationExecutionState::NotRunning:
	case Windows::ApplicationModel::Activation::ApplicationExecutionState::ClosedByUser:
	case Windows::ApplicationModel::Activation::ApplicationExecutionState::Terminated:
		ff::GetMainThreadDispatch()->Post([this, args]()
		{
			_originalSplashScreen = args->SplashScreen;
			InitializeGlobals();
		});
		break;
	}
}

void Invader::App::InitializeProcess()
{
	noAssertRet(!_processGlobals);
	_processGlobals = std::make_unique<ff::ProcessGlobals>();
	_processGlobals->Startup();
}

void Invader::App::InitializeGlobals()
{
	noAssertRet(!_globals);
	_globals = std::make_unique<ff::MetroGlobals>();

	auto window = Windows::UI::Xaml::Window::Current;
	auto displayInfo = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
	auto page = ref new MainPage();
	auto panel = safe_cast<Windows::UI::Xaml::Controls::SwapChainPanel ^>(page->Content);
	auto pointerSettings = Windows::UI::Input::PointerVisualizationSettings::GetForCurrentView();

	pointerSettings->IsBarrelButtonFeedbackEnabled = false;
	pointerSettings->IsContactFeedbackEnabled = false;

	auto finishInitFunc = [](ff::MetroGlobals *globals)
	{
		if (globals->GetTarget() && globals->GetTarget()->CanSetFullScreen())
		{
			ff::Dict options = globals->GetState();
			bool fullScreen = options.GetBool(ThisApplication::OPTION_FULL_SCREEN);
			globals->GetTarget()->SetFullScreen(fullScreen);
		}

		return true;
	};

	auto initialStateFactory = [this](ff::MetroGlobals *globals)
	{
		_invaderApp = std::make_shared<ThisApplication>();
		return _invaderApp;
	};

	_page = page;
	window->Content = page;

	if (!_processGlobals->IsValid() ||
		!_globals->Startup(
			ff::MetroGlobalsFlags::All,
			window,
			panel,
			displayInfo,
			finishInitFunc,
			initialStateFactory))
	{
		ff::String errorText;
		if (ff::ReadWholeFile(_globals->GetLogFile(), errorText))
		{
			ff::ReplaceAll(errorText, ff::String(L"\r\n"), ff::String(L"\n"));
			ff::ReplaceAll(errorText, ff::String(L"\n"), ff::String(L"\r\n"));
			window->Content = ref new FailurePage(errorText.pstring());
		}
	}

	window->Activate();
}

Invader::App ^Invader::App::Current::get()
{
	return safe_cast<App ^>(Application::Current);
}

Invader::MainPage ^Invader::App::Page::get()
{
	return Invader::App::Current->_page;
}

ThisApplication *Invader::App::InvaderApp::get()
{
	return Invader::App::Current->_invaderApp.get();
}

bool Invader::App::IsOnTitleScreen::get()
{
	ThisApplication *pApp = Invader::App::InvaderApp;
	ISystemManager *pSystems = (pApp != nullptr) ? pApp->GetEntityDomain()->GetSystemManager() : nullptr;
	ITitleState *pTitle = (pSystems != nullptr) ? pSystems->GetSystem<ITitleState>() : nullptr;

	return pTitle != nullptr;
}

bool Invader::App::IsOnGameScreen::get()
{
	ThisApplication *pApp = Invader::App::InvaderApp;
	ISystemManager *pSystems = (pApp != nullptr) ? pApp->GetEntityDomain()->GetSystemManager() : nullptr;
	IPlayGameState *pPlayGame = (pSystems != nullptr) ? pSystems->GetSystem<IPlayGameState>() : nullptr;

	return pPlayGame != nullptr;
}

Windows::ApplicationModel::Activation::SplashScreen ^Invader::App::OriginalSplashScreen::get()
{
	assert(ff::GetMainThreadDispatch()->IsCurrentThread());

	return _originalSplashScreen;
}
