#include "pch.h"
#include "State/States.h"
#include "State/StateWrapper.h"

ff::States::States()
{
}

ff::States::~States()
{
}

void ff::States::AddTop(std::shared_ptr<State> state)
{
	if (state != nullptr)
	{
		_states.push_back(std::make_shared<StateWrapper>(state));
	}
}

void ff::States::AddBottom(std::shared_ptr<State> state)
{
	if (state != nullptr)
	{
		_states.push_front(std::make_shared<StateWrapper>(state));
	}
}

std::shared_ptr<ff::State> ff::States::Advance(AppGlobals *context)
{
	for (auto i = _states.begin(); i != _states.end(); )
	{
		(*i)->Advance(context);

		if ((*i)->GetStatus() == State::Status::Dead)
		{
			i = _states.erase(i);
		}
		else
		{
			i++;
		}
	}

	if (_states.size() == 1)
	{
		return _states.front();
	}

	return nullptr;
}

void ff::States::Render(AppGlobals *context, IRenderTarget *target)
{
	for (const std::shared_ptr<State> &state : _states)
	{
		state->Render(context, target);
	}
}

void ff::States::SaveState(AppGlobals *context)
{
	for (const std::shared_ptr<State> &state : _states)
	{
		state->SaveState(context);
	}
}

void ff::States::LoadState(AppGlobals *context)
{
	for (const std::shared_ptr<State> &state : _states)
	{
		state->LoadState(context);
	}
}

ff::State::Status ff::States::GetStatus()
{
	size_t ignoreCount = 0;
	size_t deadCount = 0;

	for (const std::shared_ptr<State> &state : _states)
	{
		switch (state->GetStatus())
		{
		case State::Status::Loading:
			return State::Status::Loading;

		case State::Status::Dead:
			deadCount++;
			break;

		case State::Status::Ignore:
			ignoreCount++;
			break;
		}
	}

	return (deadCount == _states.size() - ignoreCount)
		? State::Status::Dead
		: Status::Alive;
}
