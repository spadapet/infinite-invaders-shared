#pragma once

namespace ff
{
	class AppGlobals;
	class IRenderTarget;

	// A State will advance 60hz and render up to 60fps
	class State : public std::enable_shared_from_this<State>
	{
	public:
		UTIL_API State();
		UTIL_API virtual ~State();

		UTIL_API virtual std::shared_ptr<State> Advance(AppGlobals *context);
		UTIL_API virtual void Render(AppGlobals *context, IRenderTarget *target);
		UTIL_API virtual void SaveState(AppGlobals *context);
		UTIL_API virtual void LoadState(AppGlobals *context);

		enum class Status
		{
			Loading,
			Alive,
			Dead,
			Ignore,
		};

		UTIL_API virtual Status GetStatus();
	};
}
