#include "pch.h"
#include "App.xaml.h"
#include "COM\ComAlloc.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\system\System.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dEffect.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\GraphTexture.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "metro\MainPage.xaml.h"
#include "Module\ModuleFactory.h"
#include "states\TransitionState.h"
#include "ThisApplication.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"TransitionState");
	module.RegisterClassT<TransitionState>(name, __uuidof(ITransitionState));
});

BEGIN_INTERFACES(TransitionState)
	HAS_INTERFACE(ITransitionState)
	HAS_INTERFACE(ISystem)
END_INTERFACES()

TransitionState::TransitionState()
	: _type(TRANSITION_WIPE_HORIZONTAL)
	, _newSysId(GUID_NULL)
	, _color(ff::GetColorWhite())
	, _frame(0)
	, _totalFrames(2 * 60)
	, _bAdvanceOldSystem(false)
	, _bAdvanceNewSystem(false)
	, _bWaitingForNewSystem(true)
{
}

TransitionState::~TransitionState()
{
}

bool TransitionState::Create(
	IEntityDomain *pDomain,
	ISystem *pOld,
	ISystem *pNew,
	REFGUID newSysId,
	TransitionType type,
	const DirectX::XMFLOAT4 *pColor,
	float seconds,
	ISystem **ppTrans)
{
	assertRetVal(ppTrans, false);

	ff::ComPtr<TransitionState, ITransitionState> pTrans;
	assertRetVal(SUCCEEDED(ff::ComAllocator<TransitionState>::CreateInstance(
		pDomain, GUID_NULL, __uuidof(TransitionState), (void**)&pTrans)), false);

	assertRetVal(pTrans->Init(pOld, pNew, newSysId, type, pColor, seconds), false);

	*ppTrans = ff::GetAddRef(pTrans.Interface());
	return true;
}

bool TransitionState::Init(
	ISystem *pOld,
	ISystem *pNew,
	REFGUID newSysId,
	TransitionType type,
	const DirectX::XMFLOAT4 *pColor,
	float seconds)
{
	_pOldSystem = pOld;
	_pNewSystem = pNew;
	_newSysId = newSysId;
	_type = type;
	_color = pColor ? *pColor : ff::GetColorWhite();

	_totalFrames = (size_t)(std::max(seconds, 0.0f) * Globals::GetAdvancesPerSecondF());

	return true;
}

HRESULT TransitionState::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	return __super::_Construct(unkOuter);
}

void TransitionState::SetAdvance(bool bAdvanceOldSystem, bool bAdvanceNewSystem)
{
	_bAdvanceOldSystem = bAdvanceOldSystem;
	_bAdvanceNewSystem = bAdvanceNewSystem;
}

int TransitionState::GetSystemPriority() const
{
	return SYS_PRI_STATE_NORMAL;
}

PingResult TransitionState::Ping(IEntityDomain *pDomain)
{
	PingResult result = PING_RESULT_RUNNING;

	if (_pOldSystem)
	{
		result = (PingResult)(result | _pOldSystem->Ping(pDomain));
	}
		
	if (_pNewSystem)
	{
		result = (PingResult)(result | _pNewSystem->Ping(pDomain));
	}

	return result;
}

void TransitionState::Advance(IEntityDomain *pDomain)
{
	AdvanceFrame(pDomain);

	if (_frame > _totalFrames)
	{
		assertRet(pDomain->GetSystemManager()->HasSystem(this));

		pDomain->GetSystemManager()->RemoveSystem(this);

		if (_pNewSystem)
		{
			pDomain->GetSystemManager()->AddSystem(_newSysId, _pNewSystem);
		}
	}
	else
	{
		if (_bAdvanceOldSystem && _pOldSystem)
		{
			_pOldSystem->Advance(pDomain);
		}

		if (_bAdvanceNewSystem && _pNewSystem)
		{
			_pNewSystem->Advance(pDomain);
		}
	}
}

void TransitionState::AdvanceFrame(IEntityDomain *pDomain)
{
	bool bHalfDone = _frame >= (_totalFrames / 2);
	bool bNewSystemReady = IsNewSystemReady(pDomain);

	switch (_type)
	{
	case TRANSITION_WIPE_HORIZONTAL:
	case TRANSITION_WIPE_VERTICAL:
	case TRANSITION_FADE:
		if (bNewSystemReady)
		{
			_frame++;
		}
		break;

	case TRANSITION_FADE_TO_COLOR:
		if (!bHalfDone || bNewSystemReady)
		{
			_frame++;
		}
		break;
	}

	if (bNewSystemReady && _bWaitingForNewSystem)
	{
		_bWaitingForNewSystem = false;
		Invader::App::Page->StopWaiting();
	}
}

void TransitionState::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	if (!_pTexture || pTarget->GetRotatedSize() != _pTexture->GetSize())
	{
		_pTexture = nullptr;
		_pTarget = nullptr;
		_sprite = nullptr;

		assertRet(ff::CreateGraphTexture(pTarget->GetDevice(), pTarget->GetRotatedSize(), DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 0, &_pTexture));
		assertRet(ff::CreateRenderTargetTexture(pTarget->GetDevice(), _pTexture, 0, 1, 0, &_pTarget));
		assertRet(ff::CreateSprite(_pTexture, &_sprite));
	}

	switch (_type)
	{
	case TRANSITION_WIPE_HORIZONTAL:
	case TRANSITION_WIPE_VERTICAL:
		RenderWipe(pDomain, pTarget);
		break;

	case TRANSITION_FADE:
		RenderFade(pDomain, pTarget);
		break;

	case TRANSITION_FADE_TO_COLOR:
		RenderFadeToColor(pDomain, pTarget);
		break;
	}
}

bool TransitionState::IsNewSystemReady(IEntityDomain *pDomain) const
{
	return !_pNewSystem || !(_pNewSystem->Ping(pDomain) & PING_RESULT_INIT);
}

void TransitionState::RenderWipe(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	// Render the old system
	if (_pOldSystem)
	{
		_pOldSystem->Render(pDomain, pTarget);
	}

	// Render the new system to its own texture
	{
		_pTarget->Clear(&ff::GetColorNone());
		ff::MetroGlobals::Get()->GetDepth()->Clear();

		if (_pNewSystem)
		{
			_pNewSystem->Render(pDomain, _pTarget);
		}
	}

	// Render a fade rect over the new system
	{
		ff::PointFloat targetSize = pTarget->GetRotatedSize().ToFloat();
		float percent = (float)_frame / _totalFrames;
		float fadeSize = pTarget->GetRotatedSize().x / 7.0f;

		ff::RectFloat fadeRect;
		ff::RectFloat solidRect;
		DirectX::XMFLOAT4* pFadeColors = nullptr;

		if (_type == TRANSITION_WIPE_HORIZONTAL)
		{
			float fadeRight = (targetSize.x + fadeSize) * (1.0f - percent);

			static DirectX::XMFLOAT4 fadeColors[] =
			{
				ff::GetColorNone(),
				ff::GetColorWhite(),
				ff::GetColorWhite(),
				ff::GetColorNone(),
			};

			pFadeColors = fadeColors;

			fadeRect.SetRect(fadeRight - fadeSize, 0, fadeRight, targetSize.y);
			solidRect.SetRect(fadeRight, 0, fadeRight + targetSize.x, targetSize.y);
		}
		else
		{
			float fadeTop = (-fadeSize) + (targetSize.y + fadeSize) * percent;

			static DirectX::XMFLOAT4 fadeColors[] =
			{
				ff::GetColorWhite(),
				ff::GetColorWhite(),
				ff::GetColorNone(),
				ff::GetColorNone(),
			};

			pFadeColors = fadeColors;

			fadeRect.SetRect(0, fadeTop, targetSize.x, fadeTop + fadeSize);
			solidRect.SetRect(0, fadeTop - targetSize.y, targetSize.x, fadeTop);
		}

		ff::I2dRenderer *render = ff::MetroGlobals::Get()->Get2dRender();

		if (render->BeginRender(
			_pTarget, nullptr, _pTarget->GetRotatedSize().ToFloat(),
			ff::PointFloat(0, 0), ff::PointFloat(1, 1),
			ff::MetroGlobals::Get()->Get2dEffect()))
		{
			render->GetEffect()->PushDrawType((ff::DrawType2d)(ff::DRAW_DEPTH_DISABLE | ff::DRAW_BLEND_COPY_ALPHA));

			render->DrawFilledRectangle(&fadeRect, pFadeColors, 4);
			render->DrawFilledRectangle(&solidRect, &ff::GetColorWhite(), 1);

			render->Flush();
			render->GetEffect()->PopDrawType();
			render->EndRender();
		}
	}

	// Copy the new system over the old one
	{
		ff::I2dRenderer *render = ff::MetroGlobals::Get()->Get2dRender();
		ff::MetroGlobals::Get()->GetDepth()->Clear();

		if (render->BeginRender(
			pTarget, ff::MetroGlobals::Get()->GetDepth(), pTarget->GetRotatedSize().ToFloat(),
			ff::PointFloat(0, 0), ff::PointFloat(1, 1),
			ff::MetroGlobals::Get()->Get2dEffect()))
		{
			render->DrawSprite(_sprite, nullptr, nullptr, 0, nullptr);

			render->EndRender();
		}
	}
}

void TransitionState::RenderFade(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	// Render the old system
	if (_pOldSystem)
	{
		_pOldSystem->Render(pDomain, pTarget);
	}

	// Render the new system to its own texture
	{
		float percent = (float)_frame / _totalFrames;

		_pTarget->Clear(&DirectX::XMFLOAT4(0, 0, 0, percent));
		ff::MetroGlobals::Get()->GetDepth()->Clear();

		if (_pNewSystem)
		{
			_pNewSystem->Render(pDomain, _pTarget);
		}
	}

	// Copy the new system over the old one
	{
		ff::I2dRenderer *render = ff::MetroGlobals::Get()->Get2dRender();
		ff::MetroGlobals::Get()->GetDepth()->Clear();

		if (render->BeginRender(
			pTarget, ff::MetroGlobals::Get()->GetDepth(), pTarget->GetRotatedSize().ToFloat(),
			ff::PointFloat(0, 0), ff::PointFloat(1, 1),
			ff::MetroGlobals::Get()->Get2dEffect()))
		{
			render->DrawSprite(_sprite, nullptr, nullptr, 0, nullptr);

			render->EndRender();
		}
	}
}

void TransitionState::RenderFadeToColor(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	float percent = (float)_frame / _totalFrames;
	percent = std::min(1.0f, percent);

	// Render the old or new system
	{
		if (percent < 0.5f)
		{
			if (_pOldSystem)
			{
				_pOldSystem->Render(pDomain, pTarget);
			}
		}
		else
		{
			if (_pNewSystem)
			{
				_pNewSystem->Render(pDomain, pTarget);
			}
		}
	}

	// Draw a solid color over the target
	{
		ff::I2dRenderer *render = ff::MetroGlobals::Get()->Get2dRender();
		ff::MetroGlobals::Get()->GetDepth()->Clear();

		if (render->BeginRender(
			pTarget, nullptr, pTarget->GetRotatedSize().ToFloat(),
			ff::PointFloat(0, 0), ff::PointFloat(1, 1),
			ff::MetroGlobals::Get()->Get2dEffect()))
		{
			ff::PointFloat targetSize = pTarget->GetRotatedSize().ToFloat();

			render->DrawFilledRectangle(
				&ff::RectFloat(0, 0, targetSize.x, targetSize.y),
				&DirectX::XMFLOAT4(_color.x, _color.y, _color.z, (percent < 0.5f) ? percent * 2 : 1 - (percent - 0.5f) * 2), 1);

			render->EndRender();
		}
	}
}
