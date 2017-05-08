#pragma once

#include "App.g.h"
#include "ThisApplication.h"

namespace ff
{
	class MetroGlobals;
	class ProcessGlobals;
}

namespace Invader
{
	ref class MainPage;

	ref class App sealed
	{
	public:
		App();
		virtual ~App();

		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^args) override;

	internal:
		static property App ^Current { App ^get(); }
		static property MainPage ^Page { MainPage ^get(); }
		static property ThisApplication *InvaderApp { ThisApplication *get(); }
		static property bool IsOnTitleScreen { bool get(); }
		static property bool IsOnGameScreen { bool get(); }

		property Windows::ApplicationModel::Activation::SplashScreen ^OriginalSplashScreen
		{
			Windows::ApplicationModel::Activation::SplashScreen ^get();
		}

	private:
		void InitializeProcess();
		void InitializeGlobals();

		std::unique_ptr<ff::ProcessGlobals> _processGlobals;
		std::unique_ptr<ff::MetroGlobals> _globals;
		std::shared_ptr<ThisApplication> _invaderApp;
		Windows::ApplicationModel::Activation::SplashScreen ^_originalSplashScreen;
		MainPage ^_page;
	};
}
