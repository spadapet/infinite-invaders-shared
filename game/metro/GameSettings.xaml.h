#pragma once

#include "metro\GameSettings.g.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class IAudioEffect;
}

namespace Invader
{
	public ref class GameSettings sealed
	{
	public:
		GameSettings();
		virtual ~GameSettings();

	private:
		void OnLoaded(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);
		void OnBackClicked(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);
		void OnSoundEffectToggled(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);
		void OnFullScreenToggled(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);
		void OnVolumeChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs ^args);
		void OnGameModeChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^args);
		void OnDifficultyChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^args);
		void OnControlsClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);
		void OnAboutClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args);

		void UpdateSettings();
		void UpdateControls();
		void UpdateControlsEnabled();

		void PlayTestSound();

		bool _loaded;
		ff::TypedResource<ff::IAudioEffect> _effect;
	};
}
