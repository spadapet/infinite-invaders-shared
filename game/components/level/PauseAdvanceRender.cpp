#include "pch.h"

#include "App.xaml.h"
#include "Audio\AudioDevice.h"
#include "Audio\AudioEffect.h"
#include "components\core\LoadingComponent.h"
#include "components\graph\MultiSpriteRender.h"
#include "components\level\PauseAdvanceRender.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\system\SystemManager.h"
#include "entities\EntityEvents.h"
#include "Globals.h"
#include "Globals\MetroGlobals.h"
#include "Graph\Anim\AnimKeys.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteList.h"
#include "Graph\Font\SpriteFont.h"
#include "Input\InputMapping.h"
#include "Input\Joystick\JoystickDevice.h"
#include "Input\PointerDevice.h"
#include "InputEvents.h"
#include "Module\Module.h"
#include "Module\ModuleFactory.h"
#include "metro\MainPage.xaml.h"
#include "services\GlobalPlayerService.h"
#include "systems\RenderSystem.h"
#include "ThisApplication.h"

enum PausedEvent
{
	PE_PAUSE_BACK,
	PE_PAUSE_START,
};

static const ff::InputEventMapping s_pausedInputEvents[] =
{
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_SPECIAL_BUTTON, ff::INPUT_VALUE_PRESSED, ff::JOYSTICK_BUTTON_BACK } }, PE_PAUSE_BACK },
	{ { { ff::INPUT_DEVICE_JOYSTICK, ff::INPUT_PART_SPECIAL_BUTTON, ff::INPUT_VALUE_PRESSED, ff::JOYSTICK_BUTTON_START } }, PE_PAUSE_START },
};

PauseAdvanceRender::ButtonInfo::ButtonInfo()
	: _pos(0, 0)
	, _moveUp(PAUSE_BUTTON_COUNT)
	, _moveDown(PAUSE_BUTTON_COUNT)
	, _action(nullptr)
	, _visible(false)
{
}

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PauseAdvanceRender");
	module.RegisterClassT<PauseAdvanceRender>(name);
});

BEGIN_INTERFACES(PauseAdvanceRender)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(I2dRenderComponent)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

PauseAdvanceRender::PauseAdvanceRender()
	: _paused(0)
	, _loaded(false)
	, _wasGamePaused(false)
	, _wereInvadersDancing(false)
	, _invadersDancing(0)
	, _outlineFrame(0)
	, _selectedButton(PAUSE_BUTTON_COUNT)
	, _mousePos(0, 0)
{
}

PauseAdvanceRender::~PauseAdvanceRender()
{
}

HRESULT PauseAdvanceRender::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	IEntityDomain* pDomain = pDomainProvider->GetDomain();
	ThisApplication* pApp = ThisApplication::Get(pDomainProvider);

	_invadersDancingListener.Init(pDomain, ENTITY_EVENT_INVADERS_DANCING, this);
	_mousePos = ff::MetroGlobals::Get()->GetPointer()->GetPos().ToFloat();

	_font.Init(L"Classic");
	_spriteList.Init(L"Title Sprites");
	_spaceFont.Init(L"Space");
	_effectExecute.Init(L"Menu Execute");
	_effectMain.Init(L"Menu Main");

	// Input mapping
	assertRetVal(CreateInputMapping(true, true, false, &_inputMapping), E_FAIL);
	assertRetVal(AddDefaultInputEventsAndValues(_inputMapping), E_FAIL);

	assertRetVal(CreateInputMapping(false, true, false, &_pausedInputMapping), false);
	assertRetVal(_pausedInputMapping->MapEvents(s_pausedInputEvents, _countof(s_pausedInputEvents)), false);

	assertRetVal(pDomain->GetComponentManager()->CreateComponent<LoadingComponent>(nullptr, &_loading), E_FAIL);

	return __super::_Construct(unkOuter);
}

int PauseAdvanceRender::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void PauseAdvanceRender::Advance(IEntity *entity)
{
	ThisApplication *pApp = ThisApplication::Get(entity);
	bool bGamePaused = pApp->IsGamePaused();
	bool bInvadersDancing = _invadersDancing != 0;

	if (_loading)
	{
		_loading->Advance(nullptr);

		if (!_loading->IsLoading())
		{
			_loading = nullptr;
		}
	}

	if (!_loading && !_loaded)
	{
		_loaded = true;
		verify(OnLoadingComplete(entity));
	}

	if (!_loading && bGamePaused)
	{
		_paused++;
	}
	else if (_paused)
	{
		_paused = std::min<size_t>(_paused - 1, 14);

		if (!_paused)
		{
			_outlineFrame = 0;
		}
	}

	if (bInvadersDancing)
	{
		_buttons[PAUSE_BUTTON_CONTINUE]._visible = false;
		_buttons[PAUSE_BUTTON_EXIT]._visible = false;
		_buttons[PAUSE_BUTTON_EXIT2]._visible = true;
		_buttons[PAUSE_BUTTON_REPLAY]._visible = false;
		_buttons[PAUSE_BUTTON_REPLAY2]._visible = true;

		if (_selectedButton == PAUSE_BUTTON_COUNT || !_buttons[_selectedButton]._visible)
		{
			_selectedButton = PAUSE_BUTTON_REPLAY2;
		}

		if (bGamePaused)
		{
			pApp->UnpauseGame(false);
		}
	}
	else if (bGamePaused)
	{
		_buttons[PAUSE_BUTTON_CONTINUE]._visible = true;
		_buttons[PAUSE_BUTTON_EXIT]._visible = true;
		_buttons[PAUSE_BUTTON_EXIT2]._visible = false;
		_buttons[PAUSE_BUTTON_REPLAY]._visible = true;
		_buttons[PAUSE_BUTTON_REPLAY2]._visible = false;

		if (_selectedButton == PAUSE_BUTTON_COUNT || !_buttons[_selectedButton]._visible)
		{
			_selectedButton = PAUSE_BUTTON_CONTINUE;
		}
	}

	if (_wereInvadersDancing || _wasGamePaused)
	{
		if (_paused >= 20 || _wereInvadersDancing)
		{
			ff::ISpriteAnimation *anim = _buttons[_selectedButton]._outlineAnim.GetObject();
			if (anim)
			{
				_outlineFrame += anim->GetFPS() / Globals::GetAdvancesPerSecondF();
			}
		}

		pApp->AdvanceInputMapping(_inputMapping);

		bool hasEvents = false;
		for (const ff::InputEvent &ie : _inputMapping->GetEvents())
		{
			hasEvents = true;

			if (ie.IsStart())
			{
				HandleEventStart(entity, ie._eventID);
			}
			else if (ie.IsStop())
			{
				HandleEventStop(entity, ie._eventID);
			}
		}

		if (!hasEvents)
		{
			ff::PointFloat mousePos = ff::MetroGlobals::Get()->GetPointer()->GetPos().ToFloat();
			bool bUpdateSelected = false;

			if (_mousePos != mousePos)
			{
				_mousePos = mousePos;
				// Filter out accidental touch movements when first showing buttons
				if (_invadersDancing >= 30 || _paused >= 20)
				{
					bUpdateSelected = true;
				}
			}

			RenderSystem* render = entity->GetDomain()->GetSystemManager()->GetSystem<RenderSystem>();
			ff::PointFloat levelPos = render->WindowClientToLevel(mousePos);
			bool bHandled = false;

			for (size_t i = 0; i < _countof(_buttons); i++)
			{
				const ButtonInfo &button = _buttons[i];

				if (button._visible)
				{
					ff::PointInt buttonSize = button._spriteRender->GetSprite(0)->GetSpriteData().GetTextureRect().Size();
					ff::RectFloat buttonRect(button._spriteRender->GetPos()._translate, ff::PointFloat(0, 0));

					buttonRect.right = buttonRect.left + buttonSize.x;
					buttonRect.bottom = buttonRect.top + buttonSize.y;

					PauseButtonType newButton = (PauseButtonType)i;

					if (buttonRect.PointInRect(levelPos))
					{
						bool bButtonClicked = ff::MetroGlobals::Get()->GetPointer()->GetButtonClickCount(VK_LBUTTON) > 0;

						if (bUpdateSelected)
						{
							SelectButton(entity, newButton, !bButtonClicked);
						}

						if (!bHandled && bButtonClicked)
						{
							bHandled = true;
							HandleEventStart(entity, GIE_ACTION);
						}
					}
				}
			}
		}
	}

	if (_wasGamePaused)
	{
		pApp->AdvanceInputMapping(_pausedInputMapping, true);

		for (const ff::InputEvent &ie : _pausedInputMapping->GetEvents())
		{
			if (ie.IsStart())
			{
				switch (ie._eventID)
				{
				case PE_PAUSE_BACK:
				case PE_PAUSE_START:
					pApp->UnpauseGame();
					break;
				}
			}
		}
	}

	if (_invadersDancing != 0)
	{
		_invadersDancing++;
	}

	_wereInvadersDancing = bInvadersDancing;
	_wasGamePaused = bGamePaused;
}

int PauseAdvanceRender::Get2dRenderPriority() const
{
	return LAYER_PRI_HIGHEST;
}

void PauseAdvanceRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	ThisApplication *pApp = ThisApplication::Get(entity);

	if (_paused && !pApp->DidAllowGameAdvanceWhilePaused())
	{
		float percent = (_paused < 15) ? _paused / 15.0f : 1.0f;

		DirectX::XMFLOAT4 s_colors[] =
		{
			DirectX::XMFLOAT4(0, 0, 0, 0.125f * percent),
			DirectX::XMFLOAT4(0, 0, 0, 0.75f * percent),
			DirectX::XMFLOAT4(0, 0, 0, 1.00f * percent),
			DirectX::XMFLOAT4(0, 0, 0, 0.75f * percent),
		};

		render->DrawFilledRectangle(&Globals::GetLevelRectF(), s_colors, _countof(s_colors));

		ff::ISpriteFont *font = _font.GetObject();
		if (font)
		{
			static ff::StaticString szText(L"PAUSED");
			ff::PointFloat scale(1 + percent * 2, 1 + percent * 2);
			ff::PointFloat textSize = font->MeasureText(szText, scale, ff::PointFloat(0, 0));
			ff::PointFloat pos(ff::PointFloat(950, 500) - textSize / 2);

			font->DrawText(render, szText, ff::PointFloat(4 * percent + pos.x, 4 * percent + pos.y), scale, ff::PointFloat(0, 0), &DirectX::XMFLOAT4(0, 0, 0, percent));
			render->Flush();

			font->DrawText(render, szText, pos, scale, ff::PointFloat(0, 0), &DirectX::XMFLOAT4(1, 1, 1, percent));
		}
	}

	if (_wereInvadersDancing || (_paused >= 20 && !pApp->DidAllowGameAdvanceWhilePaused()))
	{
		for (size_t i = 0; i < _countof(_buttons); i++)
		{
			ButtonInfo &button = _buttons[i];

			if (button._visible)
			{
				bool bSelected = (_selectedButton == i);

				static DirectX::XMFLOAT4 s_selected[] = { ff::GetColorWhite(), ff::GetColorBlack(), DirectX::XMFLOAT4(0.612f, 0.855f, 0.804f, 1) };
				static DirectX::XMFLOAT4 *s_unselected = s_selected;
	
				button._spriteRender->Render(entity, group, render);
				button._spriteRender->SetColors(bSelected ? s_selected : s_unselected, _countof(s_selected));

				if (bSelected)
				{
					ff::ISpriteAnimation *anim = button._outlineAnim.GetObject();
					if (anim)
					{
						anim->Render(render, ff::POSE_TWEEN_SPLINE_LOOP, _outlineFrame, button._pos, nullptr, 0, nullptr);
					}
				}
			}
		}
	}
}

void PauseAdvanceRender::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_INVADERS_DANCING)
	{
		_invadersDancing = 1;
	}
}

bool PauseAdvanceRender::CreateButtonSprites(IEntity *pPauseEntity, ButtonInfo &info, bool bArrowKeys)
{
	bool wide = &info == &_buttons[PAUSE_BUTTON_CONTINUE] ||
		&info == &_buttons[PAUSE_BUTTON_REPLAY] ||
		&info == &_buttons[PAUSE_BUTTON_EXIT];

	if (!info._spriteRender &&
		_spaceFont.Flush() &&
		_spriteList.Flush())
	{
		ff::String text = wide ? ff::String(L"Button Options Blank") : ff::String(L"Button Play Blank");
		ff::ComPtr<ff::ISprite> pButtonSprite = _spriteList.GetObject()->Get(text);
		assertRetVal(pButtonSprite, false);

		ff::ComPtr<ff::ISprite> pTextSprite;
		ff::ComPtr<ff::ISprite> pTextSprite2;
		ff::MetroGlobals *app = ff::MetroGlobals::Get();

		assertRetVal(ff::CreateSpriteFromText(
			app->Get2dRender(),
			app->Get2dEffect(),
			_spaceFont.GetObject(),
			info._text,
			pButtonSprite->GetSpriteData().GetTextureRect().Size(),
			ff::PointFloat(0.5, 0.5), // scale
			ff::PointFloat(0, 0), // spacing
			ff::PointFloat(2, -8), // offset
			true, // center
			&pTextSprite), false);

		assertRetVal(ff::CreateSpriteFromText(
			app->Get2dRender(),
			app->Get2dEffect(),
			_spaceFont.GetObject(),
			info._text,
			pButtonSprite->GetSpriteData().GetTextureRect().Size(),
			ff::PointFloat(0.5, 0.5), // scale
			ff::PointFloat(0, 0), // spacing
			ff::PointFloat(0, -10), // offset
			true, // center
			&pTextSprite2), false);

		ff::ISprite *sprites[] = { pButtonSprite, pTextSprite, pTextSprite2 };

		MultiSpriteRender::Create(
			pPauseEntity->GetDomain(),
			sprites, _countof(sprites),
			nullptr, 0, LAYER_PRI_NORMAL,
			&info._spriteRender);

		info._spriteRender->GetPos()._translate = info._pos;
	}

	if (!info._outlineAnim.DidInit())
	{
		info._outlineAnim.Init(wide ? L"Outline Options" : L"Outline Play");
	}

	return true;
}

bool PauseAdvanceRender::CreateSoundButton(IEntity *pPauseEntity, ButtonInfo &info)
{
	return true;
}

bool PauseAdvanceRender::OnLoadingComplete(IEntity *pPauseEntity)
{
	// CONTINUE button
	{
		ButtonInfo &info = _buttons[PAUSE_BUTTON_CONTINUE];
		info._action = &PauseAdvanceRender::HandleContinueButton;
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_CONTINUE);
		info._pos.SetPoint(750, 650);
		info._moveDown = PAUSE_BUTTON_REPLAY;
		assertRetVal(CreateButtonSprites(pPauseEntity, info, false), false);
	}

	// REPLAY button
	{
		ButtonInfo &info = _buttons[PAUSE_BUTTON_REPLAY];
		info._visible = true;
		info._action = &PauseAdvanceRender::HandleReplayButton;
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_REPLAY);
		info._pos.SetPoint(750, 800);
		info._moveUp = PAUSE_BUTTON_CONTINUE;
		info._moveDown = PAUSE_BUTTON_EXIT;

		assertRetVal(CreateButtonSprites(pPauseEntity, info, false), false);
	}

	// REPLAY2 button
	{
		ButtonInfo &info = _buttons[PAUSE_BUTTON_REPLAY2];
		info._visible = true;
		info._action = &PauseAdvanceRender::HandleReplayButton;
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_REPLAY);
		info._pos.SetPoint(625, 950);
		info._moveDown = PAUSE_BUTTON_EXIT2;

		assertRetVal(CreateButtonSprites(pPauseEntity, info, false), false);
	}

	// EXIT button
	{
		ButtonInfo &info = _buttons[PAUSE_BUTTON_EXIT];
		info._visible = true;
		info._action = &PauseAdvanceRender::HandleExitButton;
		info._pos.SetPoint(750, 950);
		info._moveUp = PAUSE_BUTTON_REPLAY;
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_HOME);
		assertRetVal(CreateButtonSprites(pPauseEntity, info, false), false);
	}

	// EXIT2 button
	{
		ButtonInfo &info = _buttons[PAUSE_BUTTON_EXIT2];
		info._visible = true;
		info._action = &PauseAdvanceRender::HandleExitButton;
		info._pos.SetPoint(1000, 950);
		info._moveUp = PAUSE_BUTTON_REPLAY2;
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_HOME);
		assertRetVal(CreateButtonSprites(pPauseEntity, info, false), false);
	}

	CreateSoundButton(pPauseEntity, _buttons[PAUSE_BUTTON_SOUND]);

	return true;
}

void PauseAdvanceRender::HandleEventStart(IEntity *pPauseEntity, ff::hash_t type)
{
	assertRet(_selectedButton < PAUSE_BUTTON_COUNT);

	const ButtonInfo& buttonInfo = _buttons[_selectedButton];

	switch (type)
	{
	case GIE_ACTION:
		if (buttonInfo._action)
		{
			PauseButtonType oldSelectedButton = _selectedButton;
			ThisApplication* pApp = ThisApplication::Get(pPauseEntity);

			(this->*buttonInfo._action)(pPauseEntity, _selectedButton, type);

			if ((_selectedButton == oldSelectedButton || _selectedButton == PAUSE_BUTTON_COUNT) &&
				pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true))
			{
				ff::IAudioEffect *effect = _effectExecute.GetObject();
				if (effect)
				{
					effect->Play();
				}
			}
		}
		break;

	case GIE_LEFT:
		if (_wereInvadersDancing && buttonInfo._moveUp != PAUSE_BUTTON_COUNT)
		{
			SelectButton(pPauseEntity, buttonInfo._moveUp);
		}
		break;

	case GIE_RIGHT:
		if (_wereInvadersDancing && buttonInfo._moveDown != PAUSE_BUTTON_COUNT)
		{
			SelectButton(pPauseEntity, buttonInfo._moveDown);
		}
		break;

	case GIE_UP:
		if (!_wereInvadersDancing && buttonInfo._moveUp != PAUSE_BUTTON_COUNT)
		{
			SelectButton(pPauseEntity, buttonInfo._moveUp);
		}
		break;

	case GIE_DOWN:
		if (!_wereInvadersDancing && buttonInfo._moveDown != PAUSE_BUTTON_COUNT)
		{
			SelectButton(pPauseEntity, buttonInfo._moveDown);
		}
		break;
	}
}

void PauseAdvanceRender::HandleEventStop(IEntity *pPauseEntity, ff::hash_t type)
{
	switch (type)
	{
	case GIE_QUIT:
		// the play game state is responsible for this
		break;
	}
}

void PauseAdvanceRender::HandleContinueButton(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType)
{
	ThisApplication *pApp = ThisApplication::Get(pPauseEntity);

	pApp->UnpauseGame();
}

void PauseAdvanceRender::HandleExitButton(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType)
{
	ff::ComPtr<GlobalPlayerService> pGlobalPlayers;

	if (GetService(pPauseEntity, &pGlobalPlayers))
	{
		SelectButton(pPauseEntity, PAUSE_BUTTON_COUNT, false);

		pGlobalPlayers->RequestQuit();
	}
}

void PauseAdvanceRender::HandleSoundButton(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType)
{
	ThisApplication* pApp = ThisApplication::Get(pPauseEntity);
	bool bOn = !pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);

	pApp->GetOptions().SetBool(ThisApplication::OPTION_SOUND_ON, bOn);

	if (!bOn)
	{
		ff::MetroGlobals::Get()->GetAudio()->StopEffects();
	}

	CreateSoundButton(pPauseEntity, _buttons[PAUSE_BUTTON_SOUND]);
}

void PauseAdvanceRender::HandleReplayButton(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType)
{
	ff::ComPtr<GlobalPlayerService> pGlobalPlayers;

	if (GetService(pPauseEntity, &pGlobalPlayers))
	{
		SelectButton(pPauseEntity, PAUSE_BUTTON_COUNT, false);

		pGlobalPlayers->RequestRestart();
	}
}

void PauseAdvanceRender::SelectButton(IEntity *pPauseEntity, PauseButtonType type, bool bPlayEffect)
{
	if (type <= PAUSE_BUTTON_COUNT && type != _selectedButton)
	{
		PauseButtonType oldSelectedButton = _selectedButton;
		_selectedButton = type;

		ff::ISpriteAnimation *anim = _buttons[PAUSE_BUTTON_CONTINUE]._outlineAnim.Flush();
		_outlineFrame = (_selectedButton != PAUSE_BUTTON_COUNT && anim) ? 0.0f : anim->GetLastFrame() / 2.0f;

		// play sound effect
		if (pPauseEntity && _selectedButton != PAUSE_BUTTON_COUNT)
		{
			ThisApplication *pApp = ThisApplication::Get(pPauseEntity);

			if (bPlayEffect && pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true))
			{
				ff::IAudioEffect *effect = _effectMain.GetObject();
				if (effect)
				{
					effect->Play();
				}
			}
		}
	}
}
