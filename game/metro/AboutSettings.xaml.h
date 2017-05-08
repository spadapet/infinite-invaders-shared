#pragma once

#include "metro\AboutSettings.g.h"

namespace Invader
{
	public ref class AboutSettings sealed
	{
	public:
		AboutSettings();

	private:
		void OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnBackClicked(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);

		bool _fromGameSettings;
	};
}
