#include "pch.h"
#include "State/State.h"

ff::State::State()
{
}

ff::State::~State()
{
}

std::shared_ptr<ff::State> ff::State::Advance(AppGlobals *context)
{
	return nullptr;
}

void ff::State::Render(AppGlobals *context, IRenderTarget *target)
{
}

void ff::State::SaveState(AppGlobals *context)
{
}

void ff::State::LoadState(AppGlobals *context)
{
}

ff::State::Status ff::State::GetStatus()
{
	return Status::Alive;
}
