#pragma once

#include "metro\MainPage.g.h"

namespace Invader
{
	public ref class MainPage sealed
	{
	public:
		MainPage();
		virtual ~MainPage();

		// Settings
		bool IsSettingsOpen();
		void CloseSettings();
		void ShowAboutSettingsPane();
		void ShowControlSettingsPane();
		void ShowGameSettingsPane();
		Windows::UI::Xaml::Controls::Page ^GetCurrentSettingsPane();

		// Spin waiting animation
		void StartWaiting();
		void StopWaiting();

	private:
		// Settings
		void OnSettingsPopupOpened(Platform::Object ^sender, Platform::Object ^args);
		void OnSettingsPopupClosed(Platform::Object ^sender, Platform::Object ^args);
		void ShowSettings(std::function<Windows::UI::Xaml::Controls::Page ^()> pageFactory);

		bool _showingSettings;
	};
}
