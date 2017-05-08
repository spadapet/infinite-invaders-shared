#include "pch.h"
#include "App.xaml.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\system\SystemManager.h"
#include "Globals.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\GraphTexture.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Input\PointerDevice.h"
#include "metro\MainPage.xaml.h"
#include "Module\ModuleFactory.h"
#include "states\InitState.h"
#include "states\TitleState.h"
#include "states\TransitionState.h"
#include "ThisApplication.h"
#include "Thread\ThreadDispatch.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"InitState");
	module.RegisterClassT<InitState>(name, __uuidof(IInitState));
});

BEGIN_INTERFACES(InitState)
	HAS_INTERFACE(IInitState)
	HAS_INTERFACE(ISystem)
END_INTERFACES()

InitState::InitState()
	: _counter(0)
{
	_splashLocation = Invader::App::Current->OriginalSplashScreen->ImageLocation;
}

InitState::~InitState()
{
}

HRESULT InitState::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	if (Create2dLayerGroup(pDomainProvider->GetDomain(), &_loadingGroup))
	{
		// The transition screen will stop waiting when the title screen is done loading
		Invader::App::Page->StartWaiting();

		auto rm = Windows::ApplicationModel::Resources::Core::ResourceManager::Current;
		auto rc = rm->MainResourceMap->GetValue("Files/Assets/SplashScreen.png",
			Windows::ApplicationModel::Resources::Core::ResourceContext::GetForCurrentView());
		assert(rc != nullptr);

		if (rc != nullptr && rc->ValueAsString != nullptr)
		{
			ff::String szFile = ff::String::from_pstring(rc->ValueAsString);
			ff::ComPtr<ff::IGraphTexture> pTexture;

			if (ff::CreateGraphTexture(ff::MetroGlobals::Get()->GetGraph(), szFile, DXGI_FORMAT_R8G8B8A8_UNORM, 1, &pTexture))
			{
				verify(ff::CreateSprite(pTexture, &_pSplashSprite));
			}
		}
	}

	return __super::_Construct(unkOuter);
}

int InitState::GetSystemPriority() const
{
	return SYS_PRI_STATE_NORMAL;
}

PingResult InitState::Ping(IEntityDomain *pDomain)
{
	return PING_RESULT_RUNNING;
}

void InitState::Advance(IEntityDomain *pDomain)
{
	if (!_pTitle)
	{
		ff::ComPtr<ISystem> pTitleSystem;
		assertRet(pDomain->GetSystemManager()->CreateSystem(__uuidof(ITitleState), &pTitleSystem));
		assertRet(_pTitle.QueryFrom(pTitleSystem));
	}

	if (++_counter > 15 && _pTitle)
	{
		assertRet(pDomain->GetSystemManager()->HasSystem(this));

		ff::ComPtr<ISystem> pTransition;
		assertRet(TransitionState::Create(
			pDomain, this, _pTitle, __uuidof(ITitleState),
			TRANSITION_WIPE_HORIZONTAL, nullptr, Globals::GetTransitionTime(), &pTransition));

		verify(pDomain->GetSystemManager()->AddSystem(__uuidof(ITransitionState), pTransition));
		verify(pDomain->GetSystemManager()->RemoveSystem(this));
	}

	if (_counter % 10 == 0)
	{
		Windows::Foundation::Rect splashLocation;
		bool valid = false;

		ff::GetMainThreadDispatch()->Send([&splashLocation, &valid]
		{
			try
			{
				splashLocation = Invader::App::Current->OriginalSplashScreen->ImageLocation;
				valid = true;
			}
			catch (...)
			{
				assertSz(false, L"SplashScreen.ImageLocation failed");
			}
		});

		if (valid)
		{
			_splashLocation = splashLocation;
		}
	}
}

void InitState::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	if (_loadingGroup)
	{
		ff::I2dRenderer* render = ff::MetroGlobals::Get()->Get2dRender();

		static const DirectX::XMFLOAT4 bgColor(
			16.0f / 255.0f,
			8.0f / 255.0f,
			60.0f / 255.0f,
			1.0f);

		pTarget->Clear(&bgColor);
		_loadingGroup->GetViewport()->SetRenderTargets(&pTarget, 1);

		if (_loadingGroup->BeginCustomRender(render))
		{
			if (_pSplashSprite != nullptr)
			{
				try
				{
					float dpiScale = (float)ff::MetroGlobals::Get()->GetDpiScale();
					ff::PointFloat pos(_splashLocation.Left * dpiScale, _splashLocation.Top * dpiScale);

					render->DrawSprite(_pSplashSprite, &pos, nullptr, 0, nullptr);
				}
				catch (...)
				{
					assertSz(false, L"SplashScreen.ImageLocation failed");
				}
			}

			_loadingGroup->EndCustomRender(render);
		}
	}
}
