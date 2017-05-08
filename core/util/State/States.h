#pragma once

#include "State/State.h"

namespace ff
{
	// Deals with a list of State objects that all advance and render together
	class States : public ff::State
	{
	public:
		States();
		virtual ~States();

		void AddTop(std::shared_ptr<State> state);
		void AddBottom(std::shared_ptr<State> state);

		virtual std::shared_ptr<State> Advance(AppGlobals *context) override;
		virtual void Render(AppGlobals *context, IRenderTarget *target) override;
		virtual void SaveState(AppGlobals *context) override;
		virtual void LoadState(AppGlobals *context) override;
		virtual Status GetStatus() override;

	private:
		std::list<std::shared_ptr<State>> _states;
	};
}
