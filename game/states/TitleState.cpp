#include "pch.h"
#include "App.xaml.h"
#include "Audio\AudioDevice.h"
#include "Audio\AudioEffect.h"
#include "COM\ServiceCollection.h"
#include "components\bullet\BulletAdvance.h"
#include "components\core\CollisionAdvanceRender.h"
#include "components\core\LoadingComponent.h"
#include "components\graph\SpriteAnimationAdvance.h"
#include "components\graph\SpriteAnimationRender.h"
#include "components\graph\MultiSpriteRender.h"
#include "components\invader\InvaderTitleAdvanceRender.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Graph\2D\SpriteList.h"
#include "Graph\Anim\AnimKeys.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\Font\SpriteFont.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Input\InputMapping.h"
#include "Input\PointerDevice.h"
#include "InputEvents.h"
#include "metro\MainPage.xaml.h"
#include "Module\Module.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"
#include "states\PlayGameState.h"
#include "states\ShowScoresState.h"
#include "states\TitleState.h"
#include "states\TransitionState.h"
#include "systems\AdvanceSystem.h"
#include "systems\AudioSystem.h"
#include "systems\LevelSystem.h"
#include "systems\RenderSystem.h"
#include "ThisApplication.h"

TitleState::SButtonInfo::SButtonInfo()
	: _pos(0, 0)
	, _moveLeft(TITLE_BUTTON_COUNT)
	, _moveRight(TITLE_BUTTON_COUNT)
	, _moveUp(TITLE_BUTTON_COUNT)
	, _moveDown(TITLE_BUTTON_COUNT)
	, _action(nullptr)
	, _visible(false)
{
}

static TitleButtonContext GetContextForButton(TitleButtonType type)
{
	return TITLE_CONTEXT_MAIN;
}

static TitleButtonType GetSelectionForButton(TitleButtonType type)
{
	switch (type)
	{
		default:
			return type;

		case TITLE_BUTTON_OPTION_EASY:
		case TITLE_BUTTON_OPTION_HARD:
			return TITLE_BUTTON_OPTION_NORMAL;

		case TITLE_BUTTON_OPTION_FXOFF:
			return TITLE_BUTTON_OPTION_FXON;

		case TITLE_BUTTON_OPTION_VSYNCOFF:
			return TITLE_BUTTON_OPTION_VSYNCON;

		case TITLE_BUTTON_OPTION_FULLSCREENOFF:
			return TITLE_BUTTON_OPTION_FULLSCREENON;
	}
}

static TitleButtonContext GetContextForSubMenu(TitleButtonType type)
{
	switch (type)
	{
		case TITLE_BUTTON_PLAY:
			return TITLE_CONTEXT_PLAY;

		case TITLE_BUTTON_OPTIONS:
			return TITLE_CONTEXT_OPTIONS;

		case TITLE_BUTTON_MORE:
			return TITLE_CONTEXT_MORE;

		default:
			return TITLE_CONTEXT_UNKNOWN;
	}
}

class __declspec(uuid("8302ef7f-4e9e-415b-b38d-51e35b6de6bb"))
	TitleRenderOptions : public ff::ComBase, public I2dRenderComponent
{
public:
	DECLARE_HEADER(TitleRenderOptions);

	// ComBase
	HRESULT _Construct(IUnknown *unkOuter) override;

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

private:
	ff::TypedResource<ff::ISpriteFont> _font;
};

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"TitleRenderOptions");
	module.RegisterClassT<TitleRenderOptions>(name, __uuidof(I2dRenderComponent));
});

BEGIN_INTERFACES(TitleRenderOptions)
	HAS_INTERFACE(I2dRenderComponent)
END_INTERFACES()

TitleRenderOptions::TitleRenderOptions()
{
}

TitleRenderOptions::~TitleRenderOptions()
{
}

HRESULT TitleRenderOptions::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	_font.Init(L"Space");

	return __super::_Construct(unkOuter);
}

int TitleRenderOptions::Get2dRenderPriority() const
{
	return LAYER_PRI_NORMAL + 6;
}

void TitleRenderOptions::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	ThisApplication* pApp = ThisApplication::Get(entity);
	GameMode gameMode = pApp->GetOptions().GetEnum<GameMode>(ThisApplication::OPTION_GAME_MODE, GAME_MODE_DEFAULT);
	Difficulty diff = pApp->GetOptions().GetEnum<Difficulty>(ThisApplication::OPTION_DIFFICULTY, DIFFICULTY_DEFAULT);

	ff::String modeId = Globals::IDS_TITLE_1P_METRO;
	switch (gameMode)
	{
	case GAME_MODE_TURNS:
		modeId = Globals::IDS_TITLE_2P_METRO;
		break;

	case GAME_MODE_COOP:
		modeId = Globals::IDS_TITLE_COOP_METRO;
		break;
	}

	ff::String diffId = Globals::IDS_TITLE_NORMAL_METRO;
	switch (diff)
	{
	case DIFFICULTY_BABY:
		diffId = Globals::IDS_TITLE_BABY_METRO;
		break;

	case DIFFICULTY_EASY:
		diffId = Globals::IDS_TITLE_EASY_METRO;
		break;

	case DIFFICULTY_HARD:
		diffId = Globals::IDS_TITLE_HARD_METRO;
		break;
	}

	ff::String szText = ff::GetThisModule().GetString(modeId);
	szText += L"\n";
	szText += ff::GetThisModule().GetString(diffId);

	ff::ISpriteFont *font = _font.GetObject();
	if (font)
	{
		DirectX::XMFLOAT4 color(1, 1, 1, 0.625);
		font->DrawText(
			render,
			szText,
			ff::PointFloat(750, 1080),
			ff::PointFloat(0.3125, 0.3125),
			ff::PointFloat(0, 0),
			&color);
	}
}

static ff::ModuleStartup RegisterModuleClass2([](ff::Module &module)
{
	ff::StaticString name(L"TitleState");
	module.RegisterClassT<TitleState>(name, __uuidof(ITitleState));
});

BEGIN_INTERFACES(TitleState)
	HAS_INTERFACE(ITitleState)
	HAS_INTERFACE(ISystem)
END_INTERFACES()

TitleState::TitleState()
	: _selectedButton(TITLE_BUTTON_COUNT)
	, _buttonContext(TITLE_CONTEXT_MAIN)
	, _mousePos(0, 0)
	, _nResetFullScreen(0)
	, _bExpectedEffectsOn(false)
	, _bHasActiveGame(false)
{
}

TitleState::~TitleState()
{
}

HRESULT TitleState::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	// Title resource package
	{
		_spriteList.Init(L"Title Sprites");
		_pStripeAnim.Init(L"Title Stripes");
		_pTextAnim.Init(L"Title Text");
		_pLogoAnim.Init(L"Logo");
		_pNameAnim.Init(L"Company Name");
		_spaceFont.Init(L"Space");
		_effectExecute.Init(L"Menu Execute");
		_effectMain.Init(L"Menu Main");
		_effectSub.Init(L"Menu Sub");

		ff::TypedResource<ff::ISpriteList> pCursorSprites(L"Cursors");
		assertRetVal(ff::CreateSpriteResource(pCursorSprites.GetResourceValue(), ff::String(L"Hand"), &_pHandCursor), E_FAIL);
	}

	// Input mapping
	assertRetVal(CreateInputMapping(true, true, false, &_inputMapping), E_FAIL);
	assertRetVal(AddDefaultInputEventsAndValues(_inputMapping), E_FAIL);

	// Child domain
	{
		assertRetVal(CreateEntityDomain(pApp, &_pChildDomain), E_FAIL);
		assertRetVal(_pChildDomain->GetComponentManager()->CreateComponent<LoadingComponent>(nullptr, &_loading), E_FAIL);
	}

	// Services
	ff::ComPtr<EntityFactoryService> entityFactory;
	{
		assertRetVal(SUCCEEDED(ff::ComAllocator<EntityFactoryService>::CreateInstance(_pChildDomain, &entityFactory)), E_FAIL);
		assertRetVal(_pChildDomain->GetServices()->AddService(__uuidof(EntityFactoryService), entityFactory), E_FAIL);
	}

	// Systems
	{
		_pChildDomain->GetSystemManager()->EnsureSystem<LevelSystem>();
		_pChildDomain->GetSystemManager()->EnsureSystem<AdvanceSystem>();
		_pChildDomain->GetSystemManager()->EnsureSystem<RenderSystem>();
	}

	// Create global advance/render components
	{
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pChildEntity), E_FAIL);
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pOutlineEntity), E_FAIL);
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pStripeEntity), E_FAIL);
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pTextEntity), E_FAIL);
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pLogoEntity), E_FAIL);
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_pNameEntity), E_FAIL);

		ff::ComPtr<CollisionAdvanceRender, IAdvanceComponent> pCollisionAR;
		assertRetVal(_pChildEntity->CreateComponent<CollisionAdvanceRender>(&pCollisionAR), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pCollisionAR);
		_pChildEntity->AddComponent<I2dRenderComponent>(pCollisionAR);

		ff::ComPtr<InvaderTitleAdvanceRender, IAdvanceComponent> pInvaderAR;
		assertRetVal(_pChildEntity->CreateComponent<InvaderTitleAdvanceRender>(&pInvaderAR), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pInvaderAR);
		_pChildEntity->AddComponent<I2dRenderComponent>(pInvaderAR);

		ff::ComPtr<BulletAdvance, IAdvanceComponent> pBulletA;
		assertRetVal(_pChildEntity->CreateComponent<BulletAdvance>(&pBulletA), E_FAIL);
		_pChildEntity->AddComponent<IAdvanceComponent>(pBulletA);

		ff::ComPtr<TitleRenderOptions, I2dRenderComponent> pRenderOptions;
		assertRetVal(_pChildEntity->CreateComponent<TitleRenderOptions>(&pRenderOptions), E_FAIL);
		_pChildEntity->AddComponent<I2dRenderComponent>(pRenderOptions);
	}

	return __super::_Construct(unkOuter);
}

bool TitleState::CreateButtonSprites(IEntityDomain *pDomain, SButtonInfo &info, LPCTSTR szButtonSprite, LPCTSTR szOutlineAnim, ff::PointFloat textScale)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);

	ff::MetroGlobals *globals = ff::MetroGlobals::Get();
	ff::ISpriteList *spriteList = _spriteList.Flush();
	ff::ISpriteFont *spaceFont = _spaceFont.Flush();
	assertRetVal(spriteList && spaceFont, false);

	ff::ComPtr<ff::ISprite> pButtonSprite = spriteList->Get(ff::String(szButtonSprite));
	assertRetVal(pButtonSprite, false);

	ff::ComPtr<ff::ISprite> pTextSprite;
	ff::ComPtr<ff::ISprite> pTextSprite2;

	assertRetVal(ff::CreateSpriteFromText(
		globals->Get2dRender(),
		globals->Get2dEffect(),
		spaceFont,
		info._text,
		pButtonSprite->GetSpriteData().GetTextureRect().Size(),
		textScale,
		ff::PointFloat(0, 0), // spacing
		ff::PointFloat(2, -8), // offset
		true, // center
		&pTextSprite), false);

	assertRetVal(ff::CreateSpriteFromText(
		globals->Get2dRender(),
		globals->Get2dEffect(),
		spaceFont,
		info._text,
		pButtonSprite->GetSpriteData().GetTextureRect().Size(),
		textScale,
		ff::PointFloat(0, 0), // spacing
		ff::PointFloat(0, -10), // offset
		true, // center
		&pTextSprite2), false);

	ff::ISprite *sprites[] = { pButtonSprite, pTextSprite, pTextSprite2 };

	info._spriteRender = MultiSpriteRender::Create(
		_pChildEntity, sprites, _countof(sprites), nullptr, 0, LAYER_PRI_NORMAL + 5);

	info._spriteRender->GetPos()._translate = info._pos;
	info._outlineAnim.Init(szOutlineAnim);

	return true;
}

bool TitleState::InitButtons(IEntityDomain *pDomain)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);

	for (size_t i = 0; i < _countof(_buttons); i++)
	{
		assertRetVal(_pChildDomain->GetEntityManager()->CreateEntity(&_buttons[i]._entity), false);
	}

	// Play button
	{
		SButtonInfo &info = _buttons[TITLE_BUTTON_PLAY];
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_PLAY);
		info._pos.SetPoint(325, 955);
		info._visible = true;
		info._action = &TitleState::HandlePlayButton;

		if (_bHasActiveGame)
		{
			info._moveUp = TITLE_BUTTON_PLAY_CONTINUE;
		}

		info._moveRight = TITLE_BUTTON_OPTIONS;

		assertRetVal(CreateButtonSprites(pDomain, info, L"Button Play Blank", L"Outline Play"), false);
	}

	// Options button
	{
		SButtonInfo &info = _buttons[TITLE_BUTTON_OPTIONS];
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_OPTIONS);
		info._pos.SetPoint(725, 955);
		info._visible = true;
		info._action = &TitleState::HandleOptionsButton;
		info._moveLeft = TITLE_BUTTON_PLAY;
		info._moveRight = TITLE_BUTTON_SCORES;

		assertRetVal(CreateButtonSprites(pDomain, info, L"Button Options Blank", L"Outline Options"), false);
	}

	// Scores button
	{
		SButtonInfo &info = _buttons[TITLE_BUTTON_SCORES];
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_SCORES);
		info._pos.SetPoint(1225, 955);
		info._visible = true;
		info._action = &TitleState::HandleScoreButton;

		info._moveLeft = TITLE_BUTTON_OPTIONS;
		assertRetVal(CreateButtonSprites(pDomain, info, L"Button Scores Blank", L"Outline Scores"), false);
	}

	// Play CONTINUE button
	{
		SButtonInfo &info = _buttons[TITLE_BUTTON_PLAY_CONTINUE];
		info._text = ff::GetThisModule().GetString(Globals::IDS_TITLE_CONTINUE);
		info._action = &TitleState::HandleStartGameButton;
		info._pos.SetPoint(325, 815);
		info._moveDown = TITLE_BUTTON_PLAY;
		info._visible = _bHasActiveGame;

		assertRetVal(CreateButtonSprites(pDomain, info, L"Button Play Blank", L"Outline Play", ff::PointFloat(.5, .5)), false);
	}

	for (size_t i = 0; i < _countof(_buttons); i++)
	{
		UpdateButton((TitleButtonType)i);
	}

	SelectButton(_bHasActiveGame ? TITLE_BUTTON_PLAY_CONTINUE : TITLE_BUTTON_PLAY, false);

	return true;
}

void TitleState::OnLoadingComplete(IEntityDomain *pDomain)
{
	InitButtons(pDomain);

	SpriteAnimationRender* render = nullptr;
	SpriteAnimationAdvance* pAdvance = nullptr;

	// Stripes
	if (_pStripeAnim.GetObject())
	{
		render = SpriteAnimationRender::Create(_pStripeEntity, _pStripeAnim.GetObject(), ff::POSE_TWEEN_SPLINE_LOOP, LAYER_PRI_NORMAL - 32);
		pAdvance = SpriteAnimationAdvance::Create(_pStripeEntity);

		pAdvance->SetLoopingAnim(0, render->GetAnim()->GetLastFrame(), render->GetAnim()->GetFPS());
	}

	// Title text
	if (_pTextAnim.GetObject())
	{
		render = SpriteAnimationRender::Create(_pTextEntity, _pTextAnim.GetObject(), ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_NORMAL + 2);
		pAdvance = SpriteAnimationAdvance::Create(_pTextEntity);

		pAdvance->SetInfiniteAnim(0, render->GetAnim()->GetFPS());
	}

	// Logo
	if (_pLogoAnim.GetObject())
	{
		render = SpriteAnimationRender::Create(_pLogoEntity, _pLogoAnim.GetObject(), ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_NORMAL + 2);
		pAdvance = SpriteAnimationAdvance::Create(_pLogoEntity);

		pAdvance->SetInfiniteAnim(0, render->GetAnim()->GetFPS());
	}

	// Company Name
	if (_pNameAnim.GetObject())
	{
		render = SpriteAnimationRender::Create(_pNameEntity, _pNameAnim.GetObject(), ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_NORMAL + 2);
		pAdvance = SpriteAnimationAdvance::Create(_pNameEntity);

		render->GetPos()._color.w = 0;
		pAdvance->SetInfiniteAnim(0, render->GetAnim()->GetFPS());
	}

	if (!_pActiveGame)
	{
		// Start loading a playing state in the background (to preload graphics and sounds)
		pDomain->GetSystemManager()->CreateSystem(__uuidof(IPlayGameState), &_pActiveGame);
	}
}

void TitleState::SetActiveGame(ISystem *pActiveGame)
{
	// When coming back from playing a level, make sure no effects are playing
	ff::MetroGlobals::Get()->GetAudio()->StopEffects();

	_bHasActiveGame = (pActiveGame != nullptr);
	_pActiveGame = pActiveGame;
}

int TitleState::GetSystemPriority() const
{
	return SYS_PRI_STATE_NORMAL;
}

PingResult TitleState::Ping(IEntityDomain *pDomain)
{
	// Check if the level is still loading
	if (_loading)
	{
		_loading->Advance(nullptr);

		if (!_loading->IsLoading())
		{
			_loading = nullptr;

			OnLoadingComplete(pDomain);
		}
	}

	return _loading ? PING_RESULT_INIT : PING_RESULT_RUNNING;
}

void TitleState::Advance(IEntityDomain *pDomain)
{
	if (Invader::App::Page->IsSettingsOpen())
	{
		return;
	}

	ThisApplication *pApp = ThisApplication::Get(pDomain);
	pApp->AdvanceInputMapping(_inputMapping);

	bool hasEvents = false;
	for (const ff::InputEvent &ie : _inputMapping->GetEvents())
	{
		hasEvents = true;

		if (ie.IsStart())
		{
			HandleEventStart(pDomain, ie._eventID);
		}
		else if (ie.IsStop())
		{
			HandleEventStop(pDomain, ie._eventID);
		}
	}

	if (!hasEvents)
	{
		ff::IPointerDevice *pointer = ff::MetroGlobals::Get()->GetPointer();
		ff::PointFloat mousePos = pointer->GetPos().ToFloat();
		bool bUpdateSelected = false;

		if (_mousePos != mousePos)
		{
			_mousePos = mousePos;
			bUpdateSelected = true;
		}

		RenderSystem* render = _pChildDomain->GetSystemManager()->GetSystem<RenderSystem>();
		ff::PointFloat levelPos = render->WindowClientToLevel(mousePos);
		bool bHandled = false;
		bool bCursorInLogo = ff::RectFloat(1810, 1140, 1900, 1200).PointInRect(levelPos);

		if (bUpdateSelected)
		{
			render->SetMouseCursor(bCursorInLogo ? _pHandCursor : nullptr);

			SpriteAnimationRender *pAnimRender = _pNameEntity->GetComponent<SpriteAnimationRender>();

			pAnimRender->GetPos()._color.w = (bCursorInLogo ||
				GetContextForButton(_selectedButton) == TITLE_CONTEXT_MORE ||
				_selectedButton == TITLE_BUTTON_MORE) ? 1.0f : 0.0f;
		}

		for (DWORD i = 0; i < _countof(_buttons); i++)
		{
			if (_buttons[i]._visible)
			{
				ff::PointInt buttonSize = _buttons[i]._spriteRender->GetSprite(0)->GetSpriteData().GetTextureRect().Size();
				ff::RectFloat buttonRect(_buttons[i]._spriteRender->GetPos()._translate, ff::PointFloat(0, 0));

				buttonRect.right = buttonRect.left + buttonSize.x;
				buttonRect.bottom = buttonRect.top + buttonSize.y;

				TitleButtonType newButton = (TitleButtonType)i;

				if (buttonRect.PointInRect(levelPos) &&
					GetContextForButton(_selectedButton) != GetContextForSubMenu(newButton))
				{
					if (bUpdateSelected)
					{
						SelectButton(newButton);
					}

					if (!bHandled && pointer->GetButtonClickCount(VK_LBUTTON))
					{
						bHandled = true;
						HandleEventStart(pDomain, GIE_ACTION);
					}
				}
			}
		}

		if (!bHandled && bCursorInLogo && pointer->GetButtonClickCount(VK_LBUTTON))
		{
			HandleHelpButton(pDomain, TITLE_BUTTON_MORE, GIE_ACTION);
		}
	}

	ff::IRenderTargetWindow *target = ff::MetroGlobals::Get()->GetTarget();

	if (!_loading)
	{
		_pChildDomain->GetSystemManager()->Advance();
		bool bEffects = pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);

		if (_bExpectedEffectsOn != bEffects ||
			(target->IsFullScreen() && _buttons[TITLE_BUTTON_OPTION_FULLSCREENOFF]._visible) ||
			(!target->IsFullScreen() && _buttons[TITLE_BUTTON_OPTION_FULLSCREENON]._visible))
		{
			UpdateButtonVisibility();
		}
	}

	if (_nResetFullScreen && !--_nResetFullScreen)
	{
		target->SetFullScreen(true);
	}
}

void TitleState::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	_pChildDomain->GetSystemManager()->Render(pTarget);
}

void TitleState::UpdateButton(TitleButtonType type)
{
	assertRet(type < TITLE_BUTTON_COUNT);

	SButtonInfo &info = _buttons[type];
	TitleButtonType checkType = type;

	switch (checkType)
	{
	case TITLE_BUTTON_OPTION_EASY:
	case TITLE_BUTTON_OPTION_HARD:
		checkType = TITLE_BUTTON_OPTION_NORMAL;
		break;

	case TITLE_BUTTON_OPTION_FXOFF:
		checkType = TITLE_BUTTON_OPTION_FXON;
		break;

	case TITLE_BUTTON_OPTION_VSYNCOFF:
		checkType = TITLE_BUTTON_OPTION_VSYNCON;
		break;

	case TITLE_BUTTON_OPTION_FULLSCREENOFF:
		checkType = TITLE_BUTTON_OPTION_FULLSCREENON;
		break;

	case TITLE_BUTTON_OPTION_NORMAL:
		UpdateButton(TITLE_BUTTON_OPTION_EASY);
		UpdateButton(TITLE_BUTTON_OPTION_HARD);
		break;

	case TITLE_BUTTON_OPTION_FXON:
		UpdateButton(TITLE_BUTTON_OPTION_FXOFF);
		break;

	case TITLE_BUTTON_OPTION_VSYNCON:
		UpdateButton(TITLE_BUTTON_OPTION_VSYNCOFF);
		break;

	case TITLE_BUTTON_OPTION_FULLSCREENON:
		UpdateButton(TITLE_BUTTON_OPTION_FULLSCREENOFF);
		break;
	}

	if (info._spriteRender)
	{
		static DirectX::XMFLOAT4 s_selected[] = { ff::GetColorWhite(), ff::GetColorBlack(), DirectX::XMFLOAT4(0.612f, 0.855f, 0.804f, 1) };
		static DirectX::XMFLOAT4 s_hidden[] = { ff::GetColorNone(), ff::GetColorNone(), ff::GetColorNone() };
		static DirectX::XMFLOAT4 *s_unselected = s_selected;

		bool bSelected = (checkType == _selectedButton);

		if (info._visible)
		{
			info._spriteRender->SetColors(bSelected ? s_selected : s_unselected, _countof(s_selected));
		}
		else
		{
			info._spriteRender->SetColors(s_hidden, _countof(s_hidden));
		}
	}
	else
	{
		info._visible = false;
	}
}

void TitleState::UpdateButtonVisibility()
{
	ThisApplication* pApp = ThisApplication::Get(_pChildDomain);
	ff::IRenderTargetWindow *target = ff::MetroGlobals::Get()->GetTarget();
	TitleButtonContext context = GetContextForButton(_selectedButton);
	Difficulty diff = pApp->GetOptions().GetEnum<Difficulty>(ThisApplication::OPTION_DIFFICULTY, DIFFICULTY_DEFAULT);
	bool bEffects = pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);
	bool bVsync = true;
	bool bFullscreen = target->IsFullScreen();
	bool bCanSetFullScreen = target->CanSetFullScreen();

	_buttons[TITLE_BUTTON_PLAY_1P]._visible = (context == TITLE_CONTEXT_PLAY);
	_buttons[TITLE_BUTTON_PLAY_2P]._visible = (context == TITLE_CONTEXT_PLAY);
	_buttons[TITLE_BUTTON_PLAY_COOP]._visible = (context == TITLE_CONTEXT_PLAY);
	_buttons[TITLE_BUTTON_PLAY_CONTINUE]._visible = _bHasActiveGame;

	_buttons[TITLE_BUTTON_OPTION_EASY]._visible = (context == TITLE_CONTEXT_OPTIONS && diff == DIFFICULTY_EASY);
	_buttons[TITLE_BUTTON_OPTION_NORMAL]._visible = (context == TITLE_CONTEXT_OPTIONS && diff == DIFFICULTY_NORMAL);
	_buttons[TITLE_BUTTON_OPTION_HARD]._visible = (context == TITLE_CONTEXT_OPTIONS && diff == DIFFICULTY_HARD);
	_buttons[TITLE_BUTTON_OPTION_FXON]._visible = (context == TITLE_CONTEXT_OPTIONS && bEffects);
	_buttons[TITLE_BUTTON_OPTION_FXOFF]._visible = (context == TITLE_CONTEXT_OPTIONS && !bEffects);
	_buttons[TITLE_BUTTON_OPTION_VSYNCON]._visible = (context == TITLE_CONTEXT_OPTIONS && bVsync);
	_buttons[TITLE_BUTTON_OPTION_VSYNCOFF]._visible = (context == TITLE_CONTEXT_OPTIONS && !bVsync);
	_buttons[TITLE_BUTTON_OPTION_FULLSCREENON]._visible = (context == TITLE_CONTEXT_OPTIONS && bFullscreen && bCanSetFullScreen);
	_buttons[TITLE_BUTTON_OPTION_FULLSCREENOFF]._visible = (context == TITLE_CONTEXT_OPTIONS && !bFullscreen && bCanSetFullScreen);

	_buttons[TITLE_BUTTON_MORE_HELP]._visible = (context == TITLE_CONTEXT_MORE);
	_buttons[TITLE_BUTTON_MORE_QUIT]._visible = (context == TITLE_CONTEXT_MORE);

	for (size_t i = 0; i < _countof(_buttons); i++)
	{
		UpdateButton((TitleButtonType)i);
	}

	_bExpectedEffectsOn = bEffects;
}

void TitleState::SelectButton(TitleButtonType type, bool bPlayEffect)
{
	type = GetSelectionForButton(type);

	if (type < TITLE_BUTTON_COUNT &&
		(type != _selectedButton || !_pOutlineEntity->GetComponentCount()))
	{
		TitleButtonType oldSelectedButton = _selectedButton;
		_selectedButton = type;

		_pOutlineEntity->RemoveAllComponents();

		if (_selectedButton >= 0 && _selectedButton < TITLE_BUTTON_COUNT)
		{
			if (_buttons[_selectedButton]._pHelpAnim.Flush())
			{
				SpriteAnimationRender::Create(
					_pOutlineEntity,
					_buttons[_selectedButton]._pHelpAnim.Flush(),
					ff::POSE_TWEEN_SPLINE_LOOP,
					LAYER_PRI_NORMAL + 3);
			}

			SpriteAnimationRender *render = SpriteAnimationRender::Create(
				_pOutlineEntity,
				_buttons[_selectedButton]._outlineAnim.Flush(),
				ff::POSE_TWEEN_SPLINE_LOOP,
				LAYER_PRI_NORMAL + 3);

			SpriteAnimationAdvance *pAdvance = SpriteAnimationAdvance::Create(_pOutlineEntity);

			render->GetPos()._translate = _buttons[_selectedButton]._spriteRender->GetPos()._translate;
			pAdvance->SetLoopingAnim(0, render->GetAnim()->GetLastFrame(), render->GetAnim()->GetFPS());
		}

		if (GetContextForButton(oldSelectedButton) != GetContextForButton(_selectedButton))
		{
			UpdateButtonVisibility();
		}
		else
		{
			if (oldSelectedButton < TITLE_BUTTON_COUNT)
			{
				UpdateButton(oldSelectedButton);
			}

			UpdateButton(_selectedButton);
		}

		// play sound effect
		if (_pChildDomain)
		{
			ThisApplication *pApp = ThisApplication::Get(_pChildDomain);

			if (bPlayEffect && pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true))
			{
				if (GetContextForButton(_selectedButton) == TITLE_CONTEXT_MAIN)
				{
					if (_effectMain.GetObject())
					{
						_effectMain.GetObject()->Play();
					}
				}
				else if (_effectSub.GetObject())
				{
					_effectSub.GetObject()->Play();
				}
			}
		}

		// don't show the company name
		if (_pNameEntity)
		{
			SpriteAnimationRender *pAnimRender = _pNameEntity->GetComponent<SpriteAnimationRender>();

			if (pAnimRender)
			{
				pAnimRender->GetPos()._color.w = (GetContextForButton(_selectedButton) == TITLE_CONTEXT_MORE ||
					_selectedButton == TITLE_BUTTON_MORE) ? 1.0f : 0.0f;
			}
		}
	}
}

void TitleState::HandleEventStart(IEntityDomain *pDomain, ff::hash_t type)
{
	const SButtonInfo& buttonInfo = _buttons[_selectedButton];
	ff::hash_t origType = type;

	switch (_selectedButton)
	{
	case TITLE_BUTTON_OPTION_FXOFF:
	case TITLE_BUTTON_OPTION_FXON:
	case TITLE_BUTTON_OPTION_VSYNCOFF:
	case TITLE_BUTTON_OPTION_VSYNCON:
	case TITLE_BUTTON_OPTION_FULLSCREENOFF:
	case TITLE_BUTTON_OPTION_FULLSCREENON:
	case TITLE_BUTTON_OPTION_EASY:
	case TITLE_BUTTON_OPTION_HARD:
	case TITLE_BUTTON_OPTION_NORMAL:
		if (type == GIE_LEFT || type == GIE_RIGHT)
		{
			type = GIE_ACTION;
		}
		break;
	}

	switch (type)
	{
	case GIE_ACTION:
		if (buttonInfo._action)
		{
			TitleButtonType oldSelectedButton = _selectedButton;
			ThisApplication* pApp = ThisApplication::Get(pDomain);

			(this->*buttonInfo._action)(pDomain, _selectedButton, origType);

			if (_selectedButton != TITLE_BUTTON_MORE_QUIT &&
				_selectedButton != TITLE_BUTTON_MORE_HELP &&
				GetSelectionForButton(_selectedButton) == GetSelectionForButton(oldSelectedButton) &&
				pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true))
			{
				if (_effectMain.GetObject())
				{
					_effectMain.GetObject()->StopAll();
				}

				if (_effectSub.GetObject())
				{
					_effectSub.GetObject()->StopAll();
				}

				if (_effectExecute.GetObject())
				{
					_effectExecute.GetObject()->Play();
				}
			}
		}
		break;

	case GIE_LEFT:
		SelectButton(buttonInfo._moveLeft);
		break;

	case GIE_RIGHT:
		SelectButton(buttonInfo._moveRight);
		break;

	case GIE_UP:
		SelectButton(buttonInfo._moveUp);
		break;

	case GIE_DOWN:
		SelectButton(buttonInfo._moveDown);
		break;
	}
}

void TitleState::HandleEventStop(IEntityDomain *pDomain, ff::hash_t type)
{
	switch (type)
	{
	case GIE_QUIT:
	case GIE_BACK:
		if (GetContextForButton(_selectedButton) == TITLE_CONTEXT_PLAY)
		{
			SelectButton(TITLE_BUTTON_PLAY);
		}
		else if (GetContextForButton(_selectedButton) == TITLE_CONTEXT_OPTIONS)
		{
			SelectButton(TITLE_BUTTON_OPTIONS);
		}
		else if (GetContextForButton(_selectedButton) == TITLE_CONTEXT_MORE)
		{
			SelectButton(TITLE_BUTTON_MORE);
		}
		else if (type == GIE_QUIT)
		{
			// pDomain->GetApp()->Close();
		}
		break;
	}
}

void TitleState::HandlePlayButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	ThisApplication* pApp = ThisApplication::Get(pDomain);
	GameMode gameMode = pApp->GetOptions().GetEnum<GameMode>(ThisApplication::OPTION_GAME_MODE, GAME_MODE_DEFAULT);

	switch (gameMode)
	{
	default:
	case GAME_MODE_SINGLE:
		HandleStartGameButton(pDomain, TITLE_BUTTON_PLAY_1P, actionType);
		break;

	case GAME_MODE_TURNS:
		HandleStartGameButton(pDomain, TITLE_BUTTON_PLAY_2P, actionType);
		break;

	case GAME_MODE_COOP:
		HandleStartGameButton(pDomain, TITLE_BUTTON_PLAY_COOP, actionType);
		break;
	}
}

void TitleState::HandleOptionsButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	Invader::App::Page->ShowGameSettingsPane();
}

void TitleState::HandleScoreButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	assertRet(pDomain->GetSystemManager()->HasSystem(this));

	ff::ComPtr<ISystem> pScoreState;
	assertRet(ShowScoresState::Create(pDomain, __uuidof(ITitleState), this, &pScoreState));

	pDomain->GetSystemManager()->RemoveSystem(this);
	pDomain->GetSystemManager()->AddSystem(__uuidof(IShowScoresState), pScoreState);
}

void TitleState::HandleMoreButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	SelectButton(TITLE_BUTTON_MORE_HELP);
}

void TitleState::HandleQuitButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	// ThisApplication *pApp = ThisApplication::Get(pDomain);
	// pApp->Close();
}

void TitleState::HandleHelpButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	Invader::App::Page->ShowAboutSettingsPane();
}

void TitleState::HandleStartGameButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	assertRet(pDomain->GetSystemManager()->HasSystem(this));

	ThisApplication* pApp = ThisApplication::Get(pDomain);
	ff::ComPtr<ISystem> pTransition;
	ff::ComPtr<IPlayGameState> pPlayGame;

	switch (type)
	{
	case TITLE_BUTTON_PLAY_1P:
		pApp->GetOptions().SetInt(ThisApplication::OPTION_GAME_MODE, GAME_MODE_SINGLE);
		break;

	case TITLE_BUTTON_PLAY_2P:
		pApp->GetOptions().SetInt(ThisApplication::OPTION_GAME_MODE, GAME_MODE_TURNS);
		break;

	case TITLE_BUTTON_PLAY_COOP:
		pApp->GetOptions().SetInt(ThisApplication::OPTION_GAME_MODE, GAME_MODE_COOP);
		break;

	case TITLE_BUTTON_PLAY_CONTINUE:
		if (_bHasActiveGame)
		{
			pPlayGame.QueryFrom(_pActiveGame);
		}
		break;
	}

	if (!pPlayGame)
	{
		assertRet(pDomain->GetSystemManager()->CreateSystem(&pPlayGame));
		pPlayGame->TrackStartGame();
	}

	assertRet(TransitionState::Create(
		pDomain, this, pPlayGame, __uuidof(IPlayGameState),
		TRANSITION_WIPE_HORIZONTAL, nullptr, Globals::GetTransitionTime(), &pTransition));

	pDomain->GetSystemManager()->RemoveSystem(this);
	pDomain->GetSystemManager()->AddSystem(__uuidof(ITransitionState), pTransition);
}

void TitleState::HandleToggleDifficulty(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	ThisApplication *pApp = ThisApplication::Get(pDomain);
	Difficulty diff = pApp->GetOptions().GetEnum<Difficulty>(ThisApplication::OPTION_DIFFICULTY, DIFFICULTY_DEFAULT);

	switch (actionType)
	{
	case GIE_LEFT:
		if (diff)
		{
			diff = (Difficulty)(diff - 1);
		}
		break;

	case GIE_RIGHT:
		if (diff + 1 < DIFFICULTY_HARD)
		{
			diff = (Difficulty)(diff + 1);
		}
		break;

	default:
		diff = (Difficulty)(diff + 1);

		if (diff >= DIFFICULTY_HARD)
		{
			diff = (Difficulty)0;
		}
		break;
	}

	pApp->GetOptions().SetInt(ThisApplication::OPTION_DIFFICULTY, diff);

	UpdateButtonVisibility();
}

void TitleState::HandleToggleSound(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	ThisApplication* pApp = ThisApplication::Get(pDomain);
	bool bOn = !pApp->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);

	pApp->GetOptions().SetBool(ThisApplication::OPTION_SOUND_ON, bOn);

	if (!bOn)
	{
		ff::MetroGlobals::Get()->GetAudio()->StopEffects();
	}

	UpdateButtonVisibility();
}

void TitleState::HandleToggleVsync(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	//ThisApplication *pApp = ThisApplication::Get(pDomain);
	//
	//pApp->GetOptions().SetBool(OPTION_GRAPH_VSYNC_ON,
	//	!pApp->GetOptions().GetBool(OPTION_GRAPH_VSYNC_ON));

	UpdateButtonVisibility();
}

void TitleState::HandleToggleFullScreen(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType)
{
	ff::IRenderTargetWindow *target = ff::MetroGlobals::Get()->GetTarget();
	target->SetFullScreen(!target->IsFullScreen());

	UpdateButtonVisibility();
}
