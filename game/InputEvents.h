#pragma once

namespace ff
{
	class IInputMapping;
}

class ThisApplication;
enum GameMode;

enum GameController : int
{
	GAME_CONTROLLER_NONE,
	GAME_CONTROLLER_KEYBOARD,
	GAME_CONTROLLER_JOYSTICK0,
	GAME_CONTROLLER_JOYSTICK1,
	GAME_CONTROLLER_JOYSTICK2,
	GAME_CONTROLLER_JOYSTICK3,
};

enum GameInputEvent
{
	GIE_QUIT = 1000,
	GIE_BACK,
	GIE_ACTION,
	GIE_LEFT,
	GIE_RIGHT,
	GIE_UP,
	GIE_DOWN,

	GIE_CUSTOM = 1500
};

enum GameInputValue
{
	GIV_ACTION = 2000,
	GIV_LEFT,
	GIV_RIGHT,
	GIV_UP,
	GIV_DOWN,
};

bool AddDefaultInputEventsAndValues(ff::IInputMapping *pInputMap, bool bIncludeNumPad = false);

enum PlayerInputValue
{
	PIV_MOVE_LEFT = 3000,
	PIV_MOVE_RIGHT,
	PIV_SHOOT,
};

bool CreateInputMapping(bool bKeyboards, bool bJoysticks, bool bMice, ff::IInputMapping **ppInputMap);
bool CreateBlankPlayerInputMapping(size_t nPlayerIndex, GameMode gameMode, ff::IInputMapping **ppInputMap);
bool CreatePlayerInputMapping(size_t nPlayerIndex, GameMode gameMode, ff::IInputMapping **ppInputMap);
bool AddPlayerShootValues(ff::IInputMapping *pInputMap);

class __declspec(uuid("0118ba40-0f12-4781-b0fb-543a8a36fcc3")) __declspec(novtable)
	IExternalPlayerControl : public IUnknown
{
public:
	virtual int GetDigitalPlayerValue(PlayerInputValue type) const = 0;
	virtual float GetAnalogPlayerValue(PlayerInputValue type) const = 0;
};
