#pragma once

namespace ff
{
	class IRenderTarget;
}

class IEntityDomain;

// For ISystem::GetSystemPriority
enum SystemPriority : int
{
	SYS_PRI_INPUT_LOWEST = 1100,
	SYS_PRI_INPUT_LOW = 1200,
	SYS_PRI_INPUT_NORMAL = 1300,
	SYS_PRI_INPUT_HIGH = 1400,
	SYS_PRI_INPUT_HIGHEST = 1500,

	SYS_PRI_STATE_LOWEST = 2100,
	SYS_PRI_STATE_LOW = 2200,
	SYS_PRI_STATE_NORMAL = 2300,
	SYS_PRI_STATE_HIGH = 2400,
	SYS_PRI_STATE_HIGHEST = 2500,

	SYS_PRI_ADVANCE_LOWEST = 3100,
	SYS_PRI_ADVANCE_LOW = 3200,
	SYS_PRI_ADVANCE_NORMAL = 3300,
	SYS_PRI_ADVANCE_HIGH = 3400,
	SYS_PRI_ADVANCE_HIGHEST = 3500,

	SYS_PRI_RENDER_LOWEST = 4100,
	SYS_PRI_RENDER_LOW = 4200,
	SYS_PRI_RENDER_NORMAL = 4300,
	SYS_PRI_RENDER_HIGH = 4400,
	SYS_PRI_RENDER_HIGHEST = 4500,

	SYS_PRI_DEFAULT = SYS_PRI_ADVANCE_NORMAL,
};

enum PingResult
{
	PING_RESULT_UNKNOWN = 0x00,
	PING_RESULT_INIT = 0x01,
	PING_RESULT_RUNNING = 0x02,
	PING_RESULT_DEAD = 0x04,
};

class __declspec(uuid("f0c3bfae-d090-458c-b9ec-94fbf0826727")) __declspec(novtable)
	ISystem : public IUnknown
{
public:
	virtual int GetSystemPriority() const = 0;

	virtual PingResult Ping(IEntityDomain *pDomain) = 0; // don't update state
	virtual void Advance(IEntityDomain *pDomain) = 0; // update state
	virtual void Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget) = 0; // render current state

	virtual void OnAdded(IEntityDomain *pDomain);
	virtual void OnRemoved(IEntityDomain *pDomain);
};

