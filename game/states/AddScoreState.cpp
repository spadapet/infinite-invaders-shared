#include "pch.h"
#include "COM\ComAlloc.h"
#include "Globals.h"
#include "Input\InputMapping.h"
#include "InputEvents.h"
#include "ThisApplication.h"
#include "components\core\LoadingComponent.h"
#include "components\graph\SpriteAnimationAdvance.h"
#include "components\graph\SpriteAnimationRender.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\EntityManager.h"
#include "coreEntity\system\System.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Graph\Anim\AnimKeys.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\Font\SpriteFont.h"
#include "Input\PointerDevice.h"
#include "Module\Module.h"
#include "Module\ModuleFactory.h"
#include "states\AddScoreState.h"
#include "systems\AdvanceSystem.h"
#include "systems\RenderSystem.h"

// STATIC_DATA(pod)
static const size_t s_nSpaceLetter = 26;
static const size_t s_nPeriodLetter = 27;
static const size_t s_nBackLetter = 28;
static const size_t s_nEndLetter = 29;
static const size_t s_nMaxNameSize = 9;
static const ff::PointFloat s_letterSpacing(80, 80);
static const ff::PointFloat s_lettersTopLeft(676, 625);
static const ff::PointFloat s_lettersTouchTopLeft(676 - 25, 625 - 25);

static size_t GetLetterIndex(TCHAR ch)
{
	if (ch >= 'A' && ch <= 'Z')
	{
		ch = tolower((TCHAR)(ch & 0xFF));
	}

	if (ch >= 'a' && ch <= 'z')
	{
		return ch - 'a';
	}

	assertRetVal(false, 0);
}

enum AddScoreInputEvent
{
	GIE_LETTER_A = GIE_CUSTOM + 1,
	GIE_LETTER_B,
	GIE_LETTER_C,
	GIE_LETTER_D,
	GIE_LETTER_E,
	GIE_LETTER_F,
	GIE_LETTER_G,
	GIE_LETTER_H,
	GIE_LETTER_I,
	GIE_LETTER_J,
	GIE_LETTER_K,
	GIE_LETTER_L,
	GIE_LETTER_M,
	GIE_LETTER_N,
	GIE_LETTER_O,
	GIE_LETTER_P,
	GIE_LETTER_Q,
	GIE_LETTER_R,
	GIE_LETTER_S,
	GIE_LETTER_T,
	GIE_LETTER_U,
	GIE_LETTER_V,
	GIE_LETTER_W,
	GIE_LETTER_X,
	GIE_LETTER_Y,
	GIE_LETTER_Z,
	GIE_LETTER_SPACE,
	GIE_LETTER_PERIOD,
};

static const ff::InputEventMapping s_letterInputEvents[] =
{
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'A' } }, GIE_LETTER_A },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'B' } }, GIE_LETTER_B },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'C' } }, GIE_LETTER_C },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'D' } }, GIE_LETTER_D },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'E' } }, GIE_LETTER_E },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'F' } }, GIE_LETTER_F },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'G' } }, GIE_LETTER_G },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'H' } }, GIE_LETTER_H },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'I' } }, GIE_LETTER_I },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'J' } }, GIE_LETTER_J },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'K' } }, GIE_LETTER_K },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'L' } }, GIE_LETTER_L },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'M' } }, GIE_LETTER_M },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'N' } }, GIE_LETTER_N },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'O' } }, GIE_LETTER_O },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'P' } }, GIE_LETTER_P },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'Q' } }, GIE_LETTER_Q },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'R' } }, GIE_LETTER_R },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'S' } }, GIE_LETTER_S },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'T' } }, GIE_LETTER_T },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'U' } }, GIE_LETTER_U },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'V' } }, GIE_LETTER_V },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'W' } }, GIE_LETTER_W },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'X' } }, GIE_LETTER_X },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'Y' } }, GIE_LETTER_Y },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, 'Z' } }, GIE_LETTER_Z },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_SPACE } }, GIE_LETTER_SPACE },
	{ { { ff::INPUT_DEVICE_KEYBOARD, ff::INPUT_PART_BUTTON, ff::INPUT_VALUE_PRESSED, VK_OEM_PERIOD } }, GIE_LETTER_PERIOD },
};

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"AddScoreState");
	module.RegisterClassT<AddScoreState>(name, __uuidof(IAddScoreState));
});

BEGIN_INTERFACES(AddScoreState)
	HAS_INTERFACE(IAddScoreState)
	HAS_INTERFACE(ISystem)
END_INTERFACES()

AddScoreState::AddScoreState()
	: _underSystemId(GUID_NULL)
	, _gameMode(GAME_MODE_SINGLE)
	, _nPlayerIndex(0)
	, _nScoreIndex(0)
	, _nLetter(0)
	, _counter(0)
{
	_szIntro2 = ff::GetThisModule().GetString(Globals::IDS_ADD_SCORE_INTRO2);
}

AddScoreState::~AddScoreState()
{
}

// static
bool AddScoreState::Create(
	IEntityDomain* pDomain,
	REFGUID underSystemId,
	ISystem* pUnderSystem,
	size_t nPlayerIndex,
	PlayerGlobals* pPlayerGlobals,
	ISystem** ppState)
{
	assertRetVal(ppState, false);

	ff::ComPtr<AddScoreState, IAddScoreState> pState;
	assertRetVal(SUCCEEDED(ff::ComAllocator<AddScoreState>::CreateInstance(
		pDomain, GUID_NULL, __uuidof(AddScoreState), (void**)&pState)), false);

	assertRetVal(pState->Init(pDomain, underSystemId, pUnderSystem, nPlayerIndex, pPlayerGlobals), false);

	*ppState = ff::GetAddRef(pState.Interface());
	return true;
}

bool AddScoreState::Init(
	IEntityDomain* pDomain,
	REFGUID underSystemId,
	ISystem* pUnderSystem,
	size_t nPlayerIndex,
	PlayerGlobals* pPlayerGlobals)
{
	assertRetVal(pDomain && pPlayerGlobals && pUnderSystem, false);

	ThisApplication *pApp = ThisApplication::Get(pDomain);

	_pUnderSystem = pUnderSystem;
	_underSystemId = underSystemId;
	_gameMode = pApp->GetOptions().GetEnum<GameMode>(ThisApplication::OPTION_GAME_MODE, GAME_MODE_DEFAULT);
	_playerGlobals = *pPlayerGlobals;
	_nPlayerIndex = nPlayerIndex;

	bool isCoop = (_gameMode == GAME_MODE_COOP);
	assertRetVal(pApp->GetHighScores().IsHighScore(isCoop, _playerGlobals, &_nScoreIndex), false);
	_szIntro.format(ff::GetThisModule().GetString(Globals::IDS_ADD_SCORE_INTRO).c_str(), _nScoreIndex + 1);
	_szScore.format(L"%9lu", _playerGlobals.GetScore());

	// Input mapping
	assertRetVal(InitInputMapping(pDomain), false);

	// Load stuff
	_pBackAnim.Init(L"Add Score BG");
	_pOrbAnim.Init(L"Letter Orb");
	_pClassicFont.Init(L"Classic");
	_spaceFont.Init(L"Space");

	// Child domain
	{
		assertRetVal(CreateEntityDomain(pApp, &_pChildDomain), false);
		assertRetVal(_pChildDomain->GetComponentManager()->CreateComponent<LoadingComponent>(nullptr, &_loading), false);
	}

	// Systems
	{
		_pChildDomain->GetSystemManager()->EnsureSystem<AdvanceSystem>();
		_pChildDomain->GetSystemManager()->EnsureSystem<RenderSystem>();
	}

	// Create global advance/render components
	{
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pBackEntity), false);

		SpriteAnimationRender::Create(_pBackEntity, _pBackAnim.Flush(), ff::POSE_TWEEN_SPLINE_LOOP, LAYER_PRI_LOW);
		SpriteAnimationAdvance *pAnimAdvance = SpriteAnimationAdvance::Create(_pBackEntity);
		pAnimAdvance->SetLoopingAnim(0, 4, 2);
	}

	// Custom render layer
	{
		assertRetVal(Create2dLayerGroup(_pChildDomain, &_pRenderLayer), false);
		_pRenderLayer->SetWorldSize(Globals::GetLevelSizeF());
		_pRenderLayer->GetViewport()->SetAutoSizePadding(pApp->GetOptions().GetRect(ThisApplication::OPTION_WINDOW_PADDING));
		_pRenderLayer->GetViewport()->SetAutoSizeAspect(Globals::GetLevelSize());
		_pRenderLayer->GetViewport()->SetUseDepth(false);
	}

	assertRetVal(InitLetters(), false);

	return true;
}

int AddScoreState::GetSystemPriority() const
{
	return SYS_PRI_STATE_NORMAL;
}

PingResult AddScoreState::Ping(IEntityDomain *pDomain)
{
	if (_loading)
	{
		_loading->Advance(nullptr);

		if (!_loading->IsLoading())
		{
			_loading = nullptr;
		}
	}

	if (_loading)
	{
		return PING_RESULT_INIT;
	}

	return PING_RESULT_RUNNING;
}

void AddScoreState::Advance(IEntityDomain *pDomain)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);
	pApp->AdvanceInputMapping(_inputMapping);

	bool hasEvent = false;
	for (const ff::InputEvent &ie : _inputMapping->GetEvents())
	{
		hasEvent = true;

		if (ie.IsStart())
		{
			HandleEventStart(pDomain, ie._eventID);
		}
		else if (ie.IsStop())
		{
			HandleEventStop(pDomain, ie._eventID);
		}
	}

	ff::IPointerDevice *pointer = ff::MetroGlobals::Get()->GetPointer();
	if (!hasEvent && pointer->GetButtonClickCount(VK_LBUTTON))
	{
		ff::PointFloat mousePos = pointer->GetPos().ToFloat();
		RenderSystem* render = _pChildDomain->GetSystemManager()->GetSystem<RenderSystem>();
		ff::PointFloat levelPos = render->WindowClientToLevel(mousePos);
		const ff::RectFloat bounds(350, 100, 1425, 1000);

		ff::RectFloat lettersRect(
			s_lettersTouchTopLeft.x,
			s_lettersTouchTopLeft.y,
			s_lettersTouchTopLeft.x + 7 * s_letterSpacing.x,
			s_lettersTouchTopLeft.y + 5 * s_letterSpacing.y);

		if (lettersRect.PointInRect(levelPos))
		{
			size_t col = (size_t)((levelPos.x - s_lettersTouchTopLeft.x) / s_letterSpacing.x);
			size_t row = (size_t)((levelPos.y - s_lettersTouchTopLeft.y) / s_letterSpacing.y);
			size_t index = row * 7 + col;

			if (index < 26)
			{
				_nLetter = index;
				HandleEventStart(pDomain, GIE_ACTION);
			}
			else if (index == 26)
			{
				_nLetter = s_nSpaceLetter;
				HandleEventStart(pDomain, GIE_ACTION);
			}
			else if (index == 27)
			{
				_nLetter = s_nPeriodLetter;
				HandleEventStart(pDomain, GIE_ACTION);
			}
			else if (index >= 28 && index <= 30)
			{
				_nLetter = s_nBackLetter;
				HandleEventStart(pDomain, GIE_ACTION);
			}
			else if (index >= 33 && index <= 34)
			{
				_nLetter = s_nEndLetter;
				HandleEventStop(pDomain, GIE_ACTION);
			}
		}
		else if (!bounds.PointInRect(levelPos))
		{
			_nLetter = s_nEndLetter;
		}
	}

	_pChildDomain->GetSystemManager()->Advance();

	_counter++;
}

void AddScoreState::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	if (_pUnderSystem)
	{
		_pUnderSystem->Render(pDomain, pTarget);
	}

	_pChildDomain->GetSystemManager()->Render(pTarget);

	// Render text
	{
		ff::I2dRenderer *render = ff::MetroGlobals::Get()->Get2dRender();
		_pRenderLayer->GetViewport()->SetRenderTargets(&pTarget, 1);

		if (_pRenderLayer->BeginCustomRender(render))
		{
			DirectX::XMFLOAT4 color(0.82f, 0.45f, 0.17f, 1);

			if (_spaceFont.GetObject())
			{
				_spaceFont.GetObject()->DrawText(render, _szIntro,
					ff::PointFloat(425, 175), ff::PointFloat(.875, .875), ff::PointFloat(0, 0), &color);
			}

			if (_spaceFont.GetObject())
			{
				_spaceFont.GetObject()->DrawText(render, _szIntro2,
					ff::PointFloat(425, 292), ff::PointFloat(.5, .5), ff::PointFloat(0, 0), &ff::GetColorWhite());
			}

			if (_pClassicFont.GetObject())
			{
				_pClassicFont.GetObject()->DrawText(render, _szScore,
					ff::PointFloat(450, 450), ff::PointFloat(1.5, 1.5), ff::PointFloat(0, 0), &ff::GetColorWhite());
			}

			// Render name
			if (_pClassicFont.GetObject())
			{
				bool bShowDash = _szName.size() >= s_nMaxNameSize || (_nLetter > s_nPeriodLetter) || (_counter % 60 >= 40);
				ff::String name = _szName;
				ff::String szLetter;

				if (!bShowDash)
				{
					szLetter.append(_szName.size(), ' ');
					name.append(L" ");

					szLetter += _letters[_nLetter]._letter;
				}

				if (name.size() < s_nMaxNameSize)
				{
					name.append(s_nMaxNameSize - name.size(), '-');
				}

				DirectX::XMFLOAT4 color(1, 0, 1, 1);
				_pClassicFont.GetObject()->DrawText(render, name, ff::PointFloat(988, 450), ff::PointFloat(1.5, 1.5), ff::PointFloat(0, 0), &ff::GetColorWhite());
				_pClassicFont.GetObject()->DrawText(render, szLetter, ff::PointFloat(988, 450), ff::PointFloat(1.5, 1.5), ff::PointFloat(0, 0), &color);
			}

			if (_pClassicFont.GetObject())
			{
				ff::ISpriteFont *font = _pClassicFont.GetObject();

				for (size_t i = 0; i < _letters.Size(); i++)
				{
					DirectX::XMFLOAT4 color = ff::GetColorWhite();

					if (i == _nLetter)
					{
						float percent = fmod(_counter * 2 / Globals::GetAdvancesPerSecondF(), 2.0f) / 2;
						percent = sin(percent * ff::PI_F);

						DirectX::XMStoreFloat4(&color, DirectX::XMVectorLerp(DirectX::XMLoadFloat4(&color), DirectX::XMVectorSet(1, 0, 1, 1), percent));
					}

					font->DrawText(render, ff::StaticString(_letters[i]._letter, wcslen(_letters[i]._letter)), _letters[i]._pos,
						ff::PointFloat(1, 1), ff::PointFloat(0, 0), &color);
				}
			}

			render->Flush();

			if (_pOrbAnim.GetObject())
			{
				_pOrbAnim.GetObject()->Render(render, ff::POSE_TWEEN_SPLINE_LOOP,
					_counter * 2 / Globals::GetAdvancesPerSecondF(),
					GetLetterPos(_nLetter) + ff::PointFloat(14, 14),
					nullptr, 0, nullptr);
			}
#if 0
			for (int yy = 0; yy < 5; yy++)
			{
				for (int xx = 0; xx < 7; xx++)
				{
					render->DrawRectangle(&ff::RectFloat(
						s_lettersTouchTopLeft.x + s_letterSpacing.x * xx,
						s_lettersTouchTopLeft.y + s_letterSpacing.y * yy,
						s_lettersTouchTopLeft.x + s_letterSpacing.x * (xx + 1),
						s_lettersTouchTopLeft.y + s_letterSpacing.y * (yy + 1)),
						1, &GetColorWhite());
				}
			}
#endif
			_pRenderLayer->EndCustomRender(render);
		}
	}
}

bool AddScoreState::InitInputMapping(IEntityDomain *pDomain)
{
	ThisApplication* pApp = ThisApplication::Get(pDomain);

	assertRetVal(CreateBlankPlayerInputMapping(_nPlayerIndex, GAME_MODE_SINGLE, &_inputMapping), false);
	assertRetVal(AddDefaultInputEventsAndValues(_inputMapping), false);
	assertRetVal(_inputMapping->MapEvents(s_letterInputEvents, _countof(s_letterInputEvents)), false);

	return true;
}

void AddScoreState::HandleEventStart(IEntityDomain *pDomain, ff::hash_t type)
{
	switch (type)
	{
	case GIE_UP:
		MoveLetter(ff::PointFloat(0, -1));
		break;

	case GIE_DOWN:
		MoveLetter(ff::PointFloat(0, 1));
		break;

	case GIE_LEFT:
		MoveLetter(ff::PointFloat(-1, 0));
		break;

	case GIE_RIGHT:
		MoveLetter(ff::PointFloat(1, 0));
		break;

	case GIE_ACTION:
		if (_nLetter != s_nEndLetter)
		{
			AddLetter(pDomain, _nLetter);
		}
		break;

	case GIE_BACK:
		AddLetter(pDomain, s_nBackLetter);
		break;

	default:
		if (type >= GIE_LETTER_A && type <= GIE_LETTER_PERIOD)
		{
			AddLetter(pDomain, (size_t)type - GIE_LETTER_A);
			_nLetter = s_nEndLetter;
		}
		break;
	}
}

void AddScoreState::HandleEventStop(IEntityDomain *pDomain, ff::hash_t type)
{
	switch (type)
	{
	case GIE_ACTION:
		if (_nLetter == s_nEndLetter)
		{
			AddLetter(pDomain, _nLetter);
		}
		break;

	case GIE_QUIT:
		assertRet(pDomain->GetSystemManager()->HasSystem(this));
		pDomain->GetSystemManager()->RemoveSystem(this);

		if (_pUnderSystem)
		{
			pDomain->GetSystemManager()->AddSystem(_underSystemId, _pUnderSystem);
		}
		break;
	}
}

ff::PointFloat AddScoreState::GetLetterPos(size_t index) const
{
	switch (index)
	{
	default:
		assertRetVal(index < s_nSpaceLetter, ff::PointFloat(0, 0));
		return ff::PointFloat(
			s_lettersTopLeft.x + (index % 7) * s_letterSpacing.x,
			s_lettersTopLeft.y + (index / 7) * s_letterSpacing.y);

	case s_nBackLetter:
		return GetLetterPos(GetLetterIndex('v')) + ff::PointFloat(0, s_letterSpacing.y);

	case s_nEndLetter:
		return GetLetterPos(GetLetterIndex('z')) + s_letterSpacing;

	case s_nPeriodLetter:
		return GetLetterPos(GetLetterIndex('z')) + ff::PointFloat(s_letterSpacing.x * 2, 0);

	case s_nSpaceLetter:
		return GetLetterPos(GetLetterIndex('z')) + ff::PointFloat(s_letterSpacing.x, 0);
	}
}

bool AddScoreState::InitLetters()
{
	_letters.Reserve(32);

	for (TCHAR ch = 'A'; ch <= 'Z'; ch++)
	{
		SLetter letter;
		ff::ZeroObject(letter);

		letter._letter[0] = ch;
		letter._pos = GetLetterPos(GetLetterIndex(ch));

		_letters.Push(letter);
	}

	// Space
	{
		assertRetVal(_letters.Size() == s_nSpaceLetter, false);

		SLetter letter;
		ff::ZeroObject(letter);

		letter._letter[0] = ' ';
		letter._pos = GetLetterPos(s_nSpaceLetter);

		_letters.Push(letter);
	}

	// Period
	{
		assertRetVal(_letters.Size() == s_nPeriodLetter, false);

		SLetter letter;
		ff::ZeroObject(letter);

		letter._letter[0] = '.';
		letter._pos = GetLetterPos(s_nPeriodLetter);

		_letters.Push(letter);
	}

	// Back
	{
		assertRetVal(_letters.Size() == s_nBackLetter, false);

		SLetter letter;
		ff::ZeroObject(letter);

		_tcscpy_s(letter._letter, L"Back");
		letter._pos = GetLetterPos(s_nBackLetter);

		_letters.Push(letter);
	}

	// End
	{
		assertRetVal(_letters.Size() == s_nEndLetter, false);

		SLetter letter;
		ff::ZeroObject(letter);

		_tcscpy_s(letter._letter, L"End");
		letter._pos = GetLetterPos(s_nEndLetter);

		_letters.Push(letter);
	}

	return true;
}

void AddScoreState::MoveLetter(ff::PointFloat dir)
{
	SLetter *pNewLetter = nullptr;
	ff::PointFloat pos = _letters[_nLetter]._pos;
	ff::PointFloat newPos = _letters[_nLetter]._pos + dir * s_letterSpacing;

	// Find the closest letter

	float dist = -1;

	for (size_t i = 0; i < _letters.Size(); i++)
	{
		ff::PointFloat letterPos = _letters[i]._pos;

		if ((!dir.y && letterPos.y != pos.y) ||
			(!dir.x && letterPos.x != pos.x) ||
			( dir.x > 0 && letterPos.x <= pos.x) ||
			( dir.x < 0 && letterPos.x >= pos.x) ||
			( dir.y > 0 && letterPos.y <= pos.y) ||
			( dir.y < 0 && letterPos.y >= pos.y))
		{
			continue;
		}

		float newDist =
			(letterPos.x - newPos.x) * (letterPos.x - newPos.x) +
			(letterPos.y - newPos.y) * (letterPos.y - newPos.y);

		if (dist < 0 || newDist < dist)
		{
			dist = newDist;
			_nLetter = i;
		}
	}

	if (_letters[_nLetter]._pos == pos)
	{
		// Didn't move, maybe force a move

		if (dir.y > 0 && _nLetter <= 'w' - 'a')
		{
			_nLetter = s_nBackLetter;
		}
		else if (dir.y > 0 && _nLetter > 'w' - 'a')
		{
			_nLetter = s_nEndLetter;
		}
	}
}

void AddScoreState::AddLetter(IEntityDomain *pDomain, size_t index)
{
	switch (index)
	{
	default:
		assertRet(index < s_nSpaceLetter);
		__fallthrough;

	case s_nPeriodLetter:
	case s_nSpaceLetter:
		if (_szName.size() + _tcslen(_letters[index]._letter) <= s_nMaxNameSize)
		{
			_szName += _letters[index]._letter;
		}
		break;

	case s_nBackLetter:
		if (_szName.size())
		{
			_szName.erase(_szName.size() - 1);
		}
		break;

	case s_nEndLetter:
		if (_szName.size())
		{
			ThisApplication *pApp = ThisApplication::Get(pDomain);
			verify(pApp->GetHighScores().InsertHighScore(_gameMode == GAME_MODE_COOP, _playerGlobals, _szName.c_str()));
		}

		HandleEventStop(pDomain, GIE_QUIT);
		break;
	}
}
