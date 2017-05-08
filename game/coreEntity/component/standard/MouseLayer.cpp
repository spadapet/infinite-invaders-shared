#include "pch.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\component\standard\MouseLayer.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Input\PointerDevice.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"MouseLayer");
	module.RegisterClassT<MouseLayer>(name, __uuidof(LayerBase2d));
});;

BEGIN_INTERFACES(MouseLayer)
	PARENT_INTERFACES(LayerBase2d)
END_INTERFACES()

bool CreateMouseLayer(ff::IPointerDevice *pMouse, MouseLayer **ppLayer)
{
	assertRetVal(ppLayer, false);
	*ppLayer = nullptr;

	ff::ComPtr<MouseLayer> pLayer;
	assertRetVal(SUCCEEDED(ff::ComAllocator<MouseLayer>::CreateInstance(&pLayer)), false);
	assertRetVal(pLayer->Init(pMouse), false);

	*ppLayer = ff::GetAddRef(pLayer.Interface());
	return true;
}

MouseLayer::MouseLayer()
	: _fullScreen(true)
	, _spritePos(ff::GetIdentitySpritePos())
	, _maxVisibleSeconds(0)
	, _startVisibleSeconds(0)
	, _fadeAfterSeconds(0)
{
	SetLayerPriority(LAYER_PRI_CURSOR);
}

MouseLayer::~MouseLayer()
{
}

bool MouseLayer::Init(ff::IPointerDevice *pMouse)
{
	assertRetVal(pMouse, false);
	_mouse = pMouse;
	_spritePos._translate = _mouse->GetPos().ToFloat();

	return true;
}

void MouseLayer::SetCursor(ff::ISprite* pCursor, const ff::SpritePos &pos)
{
	ff::PointFloat translate = _spritePos._translate;

	_cursor = pCursor;
	_spritePos = pos;

	_spritePos._translate = translate;
}

void MouseLayer::SetFullScreenRender(bool bFullScreen)
{
	_fullScreen = bFullScreen;
}

void MouseLayer::SetMaxVisibleTime(double seconds, double fadeAfter)
{
	_maxVisibleSeconds = std::max(0.0, seconds);
	_fadeAfterSeconds = std::max(0.0, (fadeAfter >= _maxVisibleSeconds) ? 0.0 : fadeAfter);
}

void MouseLayer::Render(I2dLayerGroup *group, ff::I2dRenderer *render)
{
	if (_mouse->IsInWindow())
	{
		if (!_fullScreen)
		{
			ff::ComPtr<ff::IRenderTargetWindow> pTargetWindow;
			if (pTargetWindow.QueryFrom(render->GetTarget()) && pTargetWindow->IsFullScreen())
			{
				return;
			}
		}

		ff::MetroGlobals *app = ff::MetroGlobals::Get();
		ff::PointFloat mousePos = _mouse->GetPos().ToFloat();

		if (_spritePos._translate != mousePos ||
			_mouse->GetButton(VK_LBUTTON) ||
			_mouse->GetButton(VK_MBUTTON) ||
			_mouse->GetButton(VK_RBUTTON))
		{
			_spritePos._translate = mousePos;
			_startVisibleSeconds = app->GetGlobalTime()._absoluteSeconds;
		}

		double visibleSeconds = app->GetGlobalTime()._absoluteSeconds - _startVisibleSeconds;

		if (_maxVisibleSeconds == 0 || visibleSeconds < _maxVisibleSeconds)
		{
			DirectX::XMFLOAT4 color = _spritePos._color;

			if (_maxVisibleSeconds != 0 && _fadeAfterSeconds != 0 && visibleSeconds > _fadeAfterSeconds)
			{
				color.w *= (visibleSeconds < _maxVisibleSeconds)
					? (float)((_maxVisibleSeconds - visibleSeconds) / (_maxVisibleSeconds - _fadeAfterSeconds))
					: 0.0f;
			}

			if (_cursor)
			{
				render->DrawSprite(_cursor, &mousePos, &_spritePos._scale, _spritePos._rotate, &color);
			}
			else
			{
				ff::PointFloat points[] =
				{
					mousePos,
					mousePos + ff::PointFloat(10, 5),
					mousePos + ff::PointFloat(5, 10),
					mousePos,
				};

				render->DrawPolyLine(points, _countof(points), &color);
			}
		}
	}
}

