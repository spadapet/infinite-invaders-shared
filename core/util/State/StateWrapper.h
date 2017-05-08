#pragma once

#include "State/State.h"

namespace ff
{
	// Automatically updates the state object based on requests from its Advance() call
	class StateWrapper : public ff::State
	{
	public:
		StateWrapper(std::shared_ptr<State> state);
		virtual ~StateWrapper();

		virtual std::shared_ptr<State> Advance(AppGlobals *context) override;
		virtual void Render(AppGlobals *context, IRenderTarget *target) override;
		virtual void SaveState(AppGlobals *context) override;
		virtual void LoadState(AppGlobals *context) override;
		virtual Status GetStatus() override;

	private:
		void CheckState();

		std::shared_ptr<ff::State> _state;
	};
}
