#include "pch.h"
#include "State/StateWrapper.h"

ff::StateWrapper::StateWrapper(std::shared_ptr<ff::State> state)
	: _state(state)
{
	CheckState();
}

ff::StateWrapper::~StateWrapper()
{
}

std::shared_ptr<ff::State> ff::StateWrapper::Advance(AppGlobals *context)
{
	noAssertRetVal(_state != nullptr, nullptr);

	std::shared_ptr<State> newState = _state->Advance(context);
	if (newState != nullptr && newState != _state)
	{
		_state->SaveState(context);
		_state = newState;
		_state->LoadState(context);

		CheckState();
	}

	if (_state->GetStatus() == State::Status::Dead)
	{
		_state->SaveState(context);
		_state = nullptr;
	}

	return nullptr;
}

void ff::StateWrapper::Render(AppGlobals *context, IRenderTarget *target)
{
	noAssertRet(_state != nullptr);
	_state->Render(context, target);
}

void ff::StateWrapper::SaveState(AppGlobals *context)
{
	noAssertRet(_state != nullptr);
	_state->SaveState(context);
}

void ff::StateWrapper::LoadState(AppGlobals *context)
{
	noAssertRet(_state != nullptr);
	_state->LoadState(context);
}

ff::State::Status ff::StateWrapper::GetStatus()
{
	noAssertRetVal(_state != nullptr, State::Status::Dead);
	return _state->GetStatus();
}

void ff::StateWrapper::CheckState()
{
	// Don't need nested wrappers
	while (true)
	{
		std::shared_ptr<StateWrapper> wrapper = std::dynamic_pointer_cast<StateWrapper>(_state);
		if (wrapper == nullptr)
		{
			break;
		}

		_state = wrapper->_state;
	}
}
