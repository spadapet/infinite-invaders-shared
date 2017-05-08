#include "pch.h"
#include "Globals.h"
#include "Globals\MetroGlobals.h"
#include "Input\InputMapping.h"
#include "Input\Joystick\JoystickDevice.h"
#include "Input\Joystick\JoystickInput.h"
#include "Input\KeyboardDevice.h"
#include "Input\PointerDevice.h"
#include "InputEvents.h"
#include "ThisApplication.h"

static const ff::InputEventMapping s_defaultInputEvents[] =
{
	// quit
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_ESCAPE } }, GIE_QUIT },

	// back
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_SPECIAL_BUTTON, ff::INPUT_VALUE_PRESSED, ff::JOYSTICK_BUTTON_BACK } }, GIE_BACK },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_SPECIAL_BUTTON, ff::INPUT_VALUE_PRESSED, ff::JOYSTICK_BUTTON_B } }, GIE_BACK },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_BACK } }, GIE_BACK },

	// action
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 0 } }, GIE_ACTION },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_SPECIAL_BUTTON, ff::INPUT_VALUE_PRESSED, ff::JOYSTICK_BUTTON_START } }, GIE_ACTION },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_RETURN } }, GIE_ACTION },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_SPACE } }, GIE_ACTION },

	// left stick
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_LEFT, ff::INPUT_INDEX_LEFT_STICK } }, GIE_LEFT },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_RIGHT, ff::INPUT_INDEX_LEFT_STICK } }, GIE_RIGHT },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_UP, ff::INPUT_INDEX_LEFT_STICK } }, GIE_UP },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_DOWN, ff::INPUT_INDEX_LEFT_STICK } }, GIE_DOWN },

	// right stick
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_LEFT, ff::INPUT_INDEX_RIGHT_STICK } }, GIE_LEFT },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_RIGHT, ff::INPUT_INDEX_RIGHT_STICK } }, GIE_RIGHT },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_UP, ff::INPUT_INDEX_RIGHT_STICK } }, GIE_UP },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_DOWN, ff::INPUT_INDEX_RIGHT_STICK } }, GIE_DOWN },

	// d-pad
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_LEFT } }, GIE_LEFT },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_RIGHT } }, GIE_RIGHT },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_UP } }, GIE_UP },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_DOWN } }, GIE_DOWN },

	// keyboard arrows
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_LEFT } }, GIE_LEFT },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_RIGHT } }, GIE_RIGHT },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_UP } }, GIE_UP },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_DOWN } }, GIE_DOWN },

	// keyboard number pad
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_NUMPAD4 } }, GIE_LEFT },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_NUMPAD6 } }, GIE_RIGHT },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_NUMPAD8 } }, GIE_UP },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_NUMPAD2 } }, GIE_DOWN },
};

static const ff::InputValueMapping s_defaultInputValues[] =
{
	// action
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 0 }, GIV_ACTION },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_SPECIAL_BUTTON, ff::INPUT_VALUE_PRESSED, ff::JOYSTICK_BUTTON_START }, GIV_ACTION },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_RETURN }, GIV_ACTION },

	// left stick
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_LEFT, ff::INPUT_INDEX_LEFT_STICK }, GIV_LEFT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_RIGHT, ff::INPUT_INDEX_LEFT_STICK }, GIV_RIGHT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_UP, ff::INPUT_INDEX_LEFT_STICK }, GIV_UP },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_DOWN, ff::INPUT_INDEX_LEFT_STICK }, GIV_DOWN },

	// right stick
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_LEFT, ff::INPUT_INDEX_RIGHT_STICK }, GIV_LEFT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_RIGHT, ff::INPUT_INDEX_RIGHT_STICK }, GIV_RIGHT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_UP, ff::INPUT_INDEX_RIGHT_STICK }, GIV_UP },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_DOWN, ff::INPUT_INDEX_RIGHT_STICK }, GIV_DOWN },

	// d-pad
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_LEFT }, GIV_LEFT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_RIGHT }, GIV_RIGHT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_UP }, GIV_UP },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_DOWN }, GIV_DOWN },

	// keyboard arrows
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_LEFT }, GIV_LEFT },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_RIGHT }, GIV_RIGHT },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_UP }, GIV_UP },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_DOWN }, GIV_DOWN },

	// keyboard number pad
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_NUMPAD4 }, GIV_LEFT },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_NUMPAD6 }, GIV_RIGHT },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_NUMPAD8 }, GIV_UP },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_NUMPAD2 }, GIV_DOWN },
};

bool AddDefaultInputEventsAndValues(ff::IInputMapping *pInputMap, bool bIncludeNumPad)
{
	assertRetVal(pInputMap, false);
	assertRetVal(pInputMap->MapEvents(s_defaultInputEvents, _countof(s_defaultInputEvents) - (bIncludeNumPad ? 0 : 4)), false);
	assertRetVal(pInputMap->MapValues(s_defaultInputValues, _countof(s_defaultInputValues) - (bIncludeNumPad ? 0 : 4)), false);

	return true;
}

static const ff::InputValueMapping s_playerInputValuesKeyboardLeft[] =
{
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'A' }, PIV_MOVE_LEFT },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'D' }, PIV_MOVE_RIGHT },

	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'W' }, PIV_SHOOT },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_SPACE }, PIV_SHOOT },
};

static const ff::InputValueMapping s_playerInputValuesKeyboardRight[] =
{
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_LEFT }, PIV_MOVE_LEFT },
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_RIGHT }, PIV_MOVE_RIGHT },

	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_UP }, PIV_SHOOT },
};

static const ff::InputValueMapping s_playerInputValuesKeyboardSingle[] =
{
	{ { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'Z' }, PIV_SHOOT },
};

static const ff::InputValueMapping s_playerInputValuesJoystick[] =
{
	// left stick
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_LEFT, ff::INPUT_INDEX_LEFT_STICK }, PIV_MOVE_LEFT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_STICK, ff::INPUT_VALUE_RIGHT, ff::INPUT_INDEX_LEFT_STICK }, PIV_MOVE_RIGHT },

	// d-pad
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_LEFT }, PIV_MOVE_LEFT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_DPAD, ff::INPUT_VALUE_RIGHT }, PIV_MOVE_RIGHT },

	// buttons
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 0 }, PIV_SHOOT },
	{ { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_TRIGGER, ff::INPUT_VALUE_PRESSED, ff::INPUT_INDEX_RIGHT_TRIGGER }, PIV_SHOOT },
};

bool CreateInputMapping(bool bKeyboards, bool bJoysticks, bool bMice, ff::IInputMapping** ppInputMap)
{
	ff::MetroGlobals *app = ff::MetroGlobals::Get();
	ff::Vector<ff::IInputDevice*> devices;

	if (bKeyboards && app->GetKeys())
	{
		devices.Push(app->GetKeys());
	}

	if (bJoysticks && app->GetJoysticks())
	{
		for (size_t i = 0; i < app->GetJoysticks()->GetCount(); i++)
		{
			devices.Push(app->GetJoysticks()->GetJoystick(i));
		}
	}

	if (bMice && app->GetPointer())
	{
		devices.Push(app->GetPointer());
	}

	assertRetVal(devices.Size(), false);

	return ff::CreateInputMapping(devices.Data(), devices.Size(), ppInputMap);
}

bool CreateBlankPlayerInputMapping(size_t nPlayerIndex, GameMode gameMode, ff::IInputMapping **ppInputMap)
{
	assertRetVal(ppInputMap && nPlayerIndex >= 0 && nPlayerIndex < 2, false);

	ff::MetroGlobals *app = ff::MetroGlobals::Get();
	size_t nJoys = app->GetJoysticks()->GetCount();

	ff::Vector<ff::IInputDevice*> devices;
	devices.Push(app->GetKeys());

	switch (gameMode)
	{
	case GAME_MODE_SINGLE:
	case GAME_MODE_TURNS:
		for (size_t i = 0; i < nJoys; i++)
		{
			devices.Push(app->GetJoysticks()->GetJoystick(i));;
		}
		break;

	case GAME_MODE_COOP:
		if (nPlayerIndex == 0 && nJoys > 1)
		{
			devices.Push(app->GetJoysticks()->GetJoystick(1));
		}

		if (nPlayerIndex == 1 && nJoys > 0)
		{
			devices.Push(app->GetJoysticks()->GetJoystick(0));
		}
		break;

	default:
		assertRetVal(false, false);
	}

	ff::ComPtr<ff::IInputMapping> pInputMap;
	assertRetVal(CreateInputMapping(devices.Data(), devices.Size(), &pInputMap), false);

	*ppInputMap = ff::GetAddRef(pInputMap.Interface());
	return true;
}

bool CreatePlayerInputMapping(size_t nPlayerIndex, GameMode gameMode, ff::IInputMapping **ppInputMap)
{
	ff::ComPtr<ff::IInputMapping> pInputMap;
	assertRetVal(CreateBlankPlayerInputMapping(nPlayerIndex, gameMode, &pInputMap), false);

	switch (gameMode)
	{
	case GAME_MODE_SINGLE:
	case GAME_MODE_TURNS:
		assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardLeft, _countof(s_playerInputValuesKeyboardLeft)), false);
		assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardRight, _countof(s_playerInputValuesKeyboardRight)), false);
		assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardSingle, _countof(s_playerInputValuesKeyboardSingle)), false);
		assertRetVal(pInputMap->MapValues(s_playerInputValuesJoystick, _countof(s_playerInputValuesJoystick)), false);
		break;

	case GAME_MODE_COOP:
		if (nPlayerIndex == 0)
		{
			assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardRight, _countof(s_playerInputValuesKeyboardRight)), false);
			assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardSingle, _countof(s_playerInputValuesKeyboardSingle)), false);
		}

		if (nPlayerIndex == 1)
		{
			assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardLeft, _countof(s_playerInputValuesKeyboardLeft)), false);
		}

		assertRetVal(pInputMap->MapValues(s_playerInputValuesJoystick, _countof(s_playerInputValuesJoystick)), false);
		break;
	}

	*ppInputMap = ff::GetAddRef(pInputMap.Interface());
	return true;
}

bool AddPlayerShootValues(ff::IInputMapping *pInputMap)
{
	assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardLeft, _countof(s_playerInputValuesKeyboardLeft)), false);
	assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardRight, _countof(s_playerInputValuesKeyboardRight)), false);
	assertRetVal(pInputMap->MapValues(s_playerInputValuesKeyboardSingle, _countof(s_playerInputValuesKeyboardSingle)), false);
	assertRetVal(pInputMap->MapValues(s_playerInputValuesJoystick, _countof(s_playerInputValuesJoystick)), false);

	return true;
}
