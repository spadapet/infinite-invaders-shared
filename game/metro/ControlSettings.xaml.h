#pragma once

#include "metro\ControlSettings.g.h"

namespace Invader
{
	public ref class ControlSettings sealed
	{
	public:
		ControlSettings();
		virtual ~ControlSettings();

	private:
		void OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);
		void OnBackClicked(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);
		void OnGameModeChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^args);

		void UpdateSettings();
		void UpdateControls();
		void UpdateControlsEnabled();
		void UpdateVisualState();

		bool _fromGameSettings;
		bool _loaded;
	};
}
