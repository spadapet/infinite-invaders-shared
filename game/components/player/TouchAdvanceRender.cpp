#include "pch.h"
#include "App.xaml.h"
#include "components\player\PlayerComponent.h"
#include "components\player\TouchAdvanceRender.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "Globals\MetroGlobals.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteList.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Input\PointerDevice.h"
#include "Module\ModuleFactory.h"
#include "services\GlobalPlayerService.h"

static const float TOUCH_BOX_SIZE = 100;
static const float TOUCH_PAUSE_SIZE = 50;
static const float TOUCH_PAUSE_TOP_MARGIN = 50;
static const float TOUCH_BOX_LEFT_MARGIN = 10;
static const float TOUCH_BOX_RIGHT_MARGIN = 20;
static const ff::PointFloat TOUCH_BOX_POINT(TOUCH_BOX_SIZE, TOUCH_BOX_SIZE);
static const ff::PointFloat TOUCH_PAUSE_POINT(TOUCH_PAUSE_SIZE, TOUCH_PAUSE_SIZE);

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"TouchAdvanceRender");
	module.RegisterClassT<TouchAdvanceRender>(name);
});

BEGIN_INTERFACES(TouchAdvanceRender)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(I2dRenderComponent)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

TouchAdvanceRender::TouchAdvanceRender()
	: _framesSinceTouch(ff::INVALID_SIZE)
	, _screenSize(0, 0)
	, _dpiScale(0)
	, _moveCenterY(0)
	, _shootCenterY(0)
	, _playerActive(false)
	, _touchingLeft(false)
	, _touchingRight(false)
	, _touchingShoot(false)
	, _touchingPause(false)
{
}

TouchAdvanceRender::~TouchAdvanceRender()
{
}

HRESULT TouchAdvanceRender::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	IEntityDomain *pDomain = pDomainProvider->GetDomain();
	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	assertRetVal(GetService(pDomain, &_globalPlayers), false);

	ff::TypedResource<ff::ISpriteList> pSprites;
	pSprites.Init(L"Sprites");
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Touch Left"), &_leftSprite), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Touch Right"), &_rightSprite), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Touch Shoot"), &_shootSprite), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Touch Left Pressed"), &_leftSpritePressed), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Touch Right Pressed"), &_rightSpritePressed), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Touch Shoot Pressed"), &_shootSpritePressed), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Touch Pause"), &_pauseSprite), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Touch Pause Pressed"), &_pauseSpritePressed), E_FAIL);

	return __super::_Construct(unkOuter);
}

int TouchAdvanceRender::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void TouchAdvanceRender::Advance(IEntity *entity)
{
	ff::IPointerDevice *pTouch = ff::MetroGlobals::Get()->GetPointer();

	if (_framesSinceTouch != ff::INVALID_SIZE)
	{
		_framesSinceTouch++;
	}

	_touchingLeft = false;
	_touchingRight = false;
	_touchingShoot = false;
	_touchingPause = false;

	PlayerComponent *pPlayer = entity->GetComponent<PlayerComponent>();
	PlayerGlobals *pPlayerGlobals = (pPlayer != nullptr)
		? _globalPlayers->GetPlayerGlobals(pPlayer->GetIndex())
		: nullptr;

	_playerActive = pPlayerGlobals != nullptr && pPlayerGlobals->IsActive();

	if (_playerActive && _dpiScale > 0 && pTouch != nullptr)
	{
		for (size_t i = 0; i < pTouch->GetTouchCount(); i++)
		{
			const ff::TouchInfo &info = pTouch->GetTouchInfo(i);

			if (info.type == ff::TouchType::TOUCH_TYPE_FINGER)
			{
				_framesSinceTouch = 0;

				ff::PointFloat pos = info.pos.ToFloat();
				ButtonType type = GetButton(pos);

				switch (type)
				{
				case ButtonType::BT_LEFT:
					_touchingLeft = true;
					_moveCenterY = pos.y;
					break;

				case ButtonType::BT_RIGHT:
					_touchingRight = true;
					_moveCenterY = pos.y;
					break;

				case ButtonType::BT_SHOOT:
					_touchingShoot = true;
					_shootCenterY = std::max(GetMinShootCenter(), pos.y);
					break;

				case ButtonType::BT_PAUSE:
					_touchingPause = true;
					break;
				}
			}
		}
	}

	if (_touchingPause)
	{
		entity->GetDomain()->GetApp()->PauseGame();
	}
}

int TouchAdvanceRender::Get2dRenderPriority() const
{
	return LAYER_PRI_OVERLAY_NORMAL;
}

void TouchAdvanceRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	ThisApplication *pApp = ThisApplication::Get(entity);

	if (!_playerActive || pApp->IsGamePaused())
	{
		return;
	}

	ff::PointInt size = render->GetTarget()->GetRotatedSize();
	_screenSize.SetPoint((float)size.x, (float)size.y);
	float dpiScale = (float)ff::MetroGlobals::Get()->GetDpiScale();

	if (_screenSize.x >= 640 * dpiScale) // wider than snapped mode
	{
		if (_dpiScale == 0)
		{
			_moveCenterY = _screenSize.y * 3 / 4;
			_shootCenterY = _screenSize.y * 3 / 4;
		}

		_dpiScale = dpiScale;

		_moveCenterY = std::max((TOUCH_BOX_SIZE / 2) * _dpiScale, _moveCenterY);
		_moveCenterY = std::min(_screenSize.y - (TOUCH_BOX_SIZE / 2) * _dpiScale, _moveCenterY);

		_shootCenterY = std::max((TOUCH_BOX_SIZE / 2) * _dpiScale, _shootCenterY);
		_shootCenterY = std::min(_screenSize.y - (TOUCH_BOX_SIZE / 2) * _dpiScale, _shootCenterY);

		if (_framesSinceTouch != ff::INVALID_SIZE && _framesSinceTouch < 1200)
		{
			float alpha = 0.375;
			if (_framesSinceTouch >= 600)
			{
				alpha = (1.0f - (_framesSinceTouch - 600) / 600.0f) * alpha;
			}

			ff::RectFloat rect = GetButtonRect(ButtonType::BT_LEFT);
			render->DrawSprite(
				_touchingLeft ? _leftSpritePressed : _leftSprite,
				&rect.TopLeft(),
				&(rect.Size() / TOUCH_BOX_POINT), // scale
				0, // rotate
				_touchingLeft ? &ff::GetColorWhite() : &DirectX::XMFLOAT4(1, 1, 1, alpha));

			rect = GetButtonRect(ButtonType::BT_RIGHT);
			render->DrawSprite(
				_touchingRight ? _rightSpritePressed : _rightSprite,
				&rect.TopLeft(),
				&(rect.Size() / TOUCH_BOX_POINT), // scale
				0, // rotate
				_touchingRight ? &ff::GetColorWhite() : &DirectX::XMFLOAT4(1, 1, 1, alpha));

			rect = GetButtonRect(ButtonType::BT_SHOOT);
			render->DrawSprite(
				_touchingShoot ? _shootSpritePressed : _shootSprite,
				&rect.TopLeft(),
				&(rect.Size() / TOUCH_BOX_POINT), // scale
				0, // rotate
				_touchingShoot ? &ff::GetColorWhite() : &DirectX::XMFLOAT4(1, 1, 1, alpha));

			rect = GetButtonRect(ButtonType::BT_PAUSE);
			render->DrawSprite(
				_touchingPause ? _pauseSpritePressed : _pauseSprite,
				&rect.TopLeft(),
				&(rect.Size() / TOUCH_PAUSE_POINT), // scale
				0, // rotate
				_touchingPause ? &ff::GetColorWhite() : &DirectX::XMFLOAT4(1, 1, 1, alpha));
		}
	}
	else
	{
		_dpiScale = 0;
	}
}

void TouchAdvanceRender::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
}

int TouchAdvanceRender::GetDigitalPlayerValue(PlayerInputValue type) const
{
	return GetAnalogPlayerValue(type) >= 0.5f ? 1 : 0;
}

float TouchAdvanceRender::GetAnalogPlayerValue(PlayerInputValue type) const
{
	switch (type)
	{
	case PlayerInputValue::PIV_MOVE_LEFT:
		return (_touchingLeft && !_touchingRight) ? 1.0f : 0.0f;

	case PlayerInputValue::PIV_MOVE_RIGHT:
		return (!_touchingLeft && _touchingRight) ? 1.0f : 0.0f;

	case PlayerInputValue::PIV_SHOOT:
		return _touchingShoot ? 1.0f : 0.0f;

	default:
		return 0.0f;
	}
}

TouchAdvanceRender::ButtonType TouchAdvanceRender::GetButton(ff::PointFloat screenPos) const
{
	if (_dpiScale > 0 && _screenSize.x >= 640 * _dpiScale)
	{
		if (screenPos.x < (TOUCH_BOX_LEFT_MARGIN + TOUCH_BOX_SIZE) * _dpiScale)
		{
			return ButtonType::BT_LEFT;
		}

		if (screenPos.x < (TOUCH_BOX_LEFT_MARGIN + TOUCH_BOX_SIZE) * 2 * _dpiScale)
		{
			return ButtonType::BT_RIGHT;
		}

		ff::RectFloat pauseRect = GetButtonRect(ButtonType::BT_PAUSE);
		if (pauseRect.PointInRect(screenPos))
		{
			return ButtonType::BT_PAUSE;
		}

		if (screenPos.x >= _screenSize.x - TOUCH_BOX_SIZE * 2 * _dpiScale)
		{
			if (screenPos.y >= GetMinShootCenter() - TOUCH_BOX_SIZE / 2.0 * _dpiScale)
			{
				return ButtonType::BT_SHOOT;
			}
		}
	}

	return ButtonType::BT_NONE;
}

ff::RectFloat TouchAdvanceRender::GetButtonRect(ButtonType type) const
{
	if (_dpiScale > 0 && _screenSize.x >= 640 * _dpiScale)
	{
		switch (type)
		{
		case ButtonType::BT_LEFT:
			return ff::RectFloat(
				TOUCH_BOX_LEFT_MARGIN * _dpiScale,
				_moveCenterY - (TOUCH_BOX_SIZE / 2) * _dpiScale,
				(TOUCH_BOX_SIZE + TOUCH_BOX_LEFT_MARGIN) * _dpiScale,
				_moveCenterY + (TOUCH_BOX_SIZE / 2) * _dpiScale);

		case ButtonType::BT_RIGHT:
			return ff::RectFloat(
				(TOUCH_BOX_LEFT_MARGIN + TOUCH_BOX_SIZE) * _dpiScale,
				_moveCenterY - (TOUCH_BOX_SIZE / 2) * _dpiScale,
				(TOUCH_BOX_LEFT_MARGIN + TOUCH_BOX_SIZE * 2) * _dpiScale,
				_moveCenterY + (TOUCH_BOX_SIZE / 2) * _dpiScale);

		case ButtonType::BT_SHOOT:
			return ff::RectFloat(
				_screenSize.x - (TOUCH_BOX_RIGHT_MARGIN + TOUCH_BOX_SIZE) * _dpiScale,
				_shootCenterY - (TOUCH_BOX_SIZE / 2) * _dpiScale,
				_screenSize.x - TOUCH_BOX_RIGHT_MARGIN * _dpiScale,
				_shootCenterY + (TOUCH_BOX_SIZE / 2) * _dpiScale);

		case ButtonType::BT_PAUSE:
			return ff::RectFloat(
				_screenSize.x - (TOUCH_BOX_RIGHT_MARGIN + TOUCH_PAUSE_SIZE) * _dpiScale,
				TOUCH_PAUSE_TOP_MARGIN,
				_screenSize.x - TOUCH_BOX_RIGHT_MARGIN * _dpiScale,
				TOUCH_PAUSE_TOP_MARGIN + TOUCH_PAUSE_SIZE * _dpiScale);
		}
	}

	return ff::RectFloat(0, 0, 0, 0);
}

float TouchAdvanceRender::GetMinShootCenter() const
{
	return (TOUCH_PAUSE_TOP_MARGIN + TOUCH_PAUSE_SIZE + TOUCH_BOX_RIGHT_MARGIN + TOUCH_BOX_SIZE / 2.0f) * _dpiScale;
}
