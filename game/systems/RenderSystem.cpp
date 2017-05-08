#include "pch.h"
#include "coreEntity\component\standard\MouseLayer.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "Globals.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteList.h"
#include "Graph\2D\SpritePos.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Module\ModuleFactory.h"
#include "Resource\ResourceValue.h"
#include "systems\RenderSystem.h"
#include "ThisApplication.h"

class __declspec(uuid("97cda548-a98f-4131-9623-671230cb4ef1"))
	LayerProxy2d : public LayerBase2d
{
public:
	DECLARE_HEADER(LayerProxy2d);

	void SetOwner(LayerBase2d *pOwner);

	virtual void Render(I2dLayerGroup *group, ff::I2dRenderer *render) override;

private:
	LayerBase2d *_pOwner;
};

BEGIN_INTERFACES(LayerProxy2d)
	PARENT_INTERFACES(LayerBase2d)
END_INTERFACES()

LayerProxy2d::LayerProxy2d()
	: _pOwner(nullptr)
{
}

LayerProxy2d::~LayerProxy2d()
{
	assert(!_pOwner);
}

void LayerProxy2d::SetOwner(LayerBase2d *pOwner)
{
	_pOwner = pOwner;
}

void LayerProxy2d::Render(I2dLayerGroup *group, ff::I2dRenderer *render)
{
	if (_pOwner)
	{
		_pOwner->Render(group, render);
	}
}

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"RenderSystem");
	module.RegisterClassT<RenderSystem>(name);
});

BEGIN_INTERFACES(RenderSystem)
	HAS_INTERFACE(ISystem)
	HAS_INTERFACE(IComponentListener)
	PARENT_INTERFACES(LayerBase2d)
END_INTERFACES()

RenderSystem::RenderSystem()
	: _debugCenter(-1, -1)
	, _rendering(false)
{
}

RenderSystem::~RenderSystem()
{
	for (size_t i = 0; i < _layers.Size(); i++)
	{
		_layers[i]._layer->SetOwner(nullptr);
	}
}

HRESULT RenderSystem::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	IEntityDomain* pDomain = pDomainProvider->GetDomain();
	ThisApplication* pApp = pDomain->GetApp();

	_renderListener.Init(pDomain, this);

	_pRender = ff::MetroGlobals::Get()->Get2dRender();

	// Level layer group
	{
		SLayer levelLayer;
		levelLayer._minPriority = LAYER_PRI_LOWEST;
		levelLayer._maxPriority = LAYER_PRI_HIGHEST;

		assertRetVal(Create2dLayerGroup(pDomain, &levelLayer._group), E_FAIL);
		levelLayer._group->SetWorldSize(Globals::GetLevelSizeF());
		levelLayer._group->GetViewport()->SetAutoSizePadding(pApp->GetOptions().GetRect(ThisApplication::OPTION_WINDOW_PADDING));
		levelLayer._group->GetViewport()->SetAutoSizeAspect(Globals::GetLevelSize());

		assertRetVal(SUCCEEDED(ff::ComAllocator<LayerProxy2d>::CreateInstance(&levelLayer._layer)), E_FAIL);
		levelLayer._layer->SetOwner(this);
		assertRetVal(levelLayer._group->AddLayer(levelLayer._layer), E_FAIL);

		_layers.Push(levelLayer);
	}

	// Overlay layer group
	{
		SLayer overlayLayer;
		overlayLayer._minPriority = LAYER_PRI_OVERLAY_LOWEST;
		overlayLayer._maxPriority = LAYER_PRI_OVERLAY_HIGHEST;

		assertRetVal(Create2dLayerGroup(pDomain, &overlayLayer._group), E_FAIL);

		assertRetVal(SUCCEEDED(ff::ComAllocator<LayerProxy2d>::CreateInstance(&overlayLayer._layer)), E_FAIL);
		overlayLayer._layer->SetOwner(this);
		assertRetVal(overlayLayer._group->AddLayer(overlayLayer._layer), E_FAIL);

		ff::TypedResource<ff::ISpriteList> pSprites(L"Cursors");
		assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Arrow"), &_cursor), E_FAIL);

		_layers.Push(overlayLayer);
	}

	return __super::_Construct(unkOuter);
}

ff::PointFloat RenderSystem::WindowClientToLevel(ff::PointFloat client)
{
	I2dLayerGroup* group = _layers[0]._group;
	ILayerViewport* pView = group->GetViewport();

	if (pView && pView->GetViewRectCount())
	{
		ff::PointInt topleft = pView->GetViewRects()->TopLeft();
		ff::PointFloat offset(client.x - topleft.x, client.y - topleft.y);
		ff::PointFloat level = offset * group->GetWorldScale() + group->GetWorldTopLeft();

		return level;
	}

	return ff::PointFloat(0, 0);
}

void RenderSystem::SetRenderer(ff::I2dRenderer *render)
{
	_pRender = render;
}

void RenderSystem::SetMouseCursor(ff::ISprite *pSprite)
{
	if (_mouseLayer != nullptr)
	{
		_mouseLayer->SetCursor(pSprite ? pSprite : _cursor, ff::GetIdentitySpritePos());
	}
}

int RenderSystem::GetSystemPriority() const
{
	return SYS_PRI_RENDER_NORMAL;
}

PingResult RenderSystem::Ping(IEntityDomain *pDomain)
{
	return PING_RESULT_RUNNING;
}

void RenderSystem::Advance(IEntityDomain *pDomain)
{
	for (size_t i = 0; i < _layers.Size(); i++)
	{
		_layers[i]._group->Advance();
	}
}

void RenderSystem::UpdateViewports(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	if (!pTarget)
	{
		pTarget = ff::MetroGlobals::Get()->GetTarget();
	}

	for (size_t i = 0; i < _layers.Size(); i++)
	{
		ILayerViewport *pViewport = _layers[i]._group->GetViewport();
		pViewport->SetRenderTargets(&pTarget, 1);
	}
}

void RenderSystem::UpdateLevelLayerGroup(IEntityDomain *pDomain, I2dLayerGroup *group)
{
}

void RenderSystem::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
	_rendering = true;

	UpdateViewports(pDomain, pTarget);

	UpdateLevelLayerGroup(pDomain, _layers[0]._group);

	for (size_t i = 0; i < _layers.Size(); i++)
	{
		I2dLayerGroup *group = _layers[i]._group;

		if (i)
		{
			ff::IRenderDepth *pDepth = group->GetViewport()->GetDepth();

			if (pDepth)
			{
				pDepth->Clear();
			}
		}

		group->Render(_pRender);
	}

	_rendering = false;
}

void RenderSystem::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	assert(!_rendering);

	ff::ComPtr<I2dRenderComponent> render;
	assertRet(render.QueryFrom(pComp));

	Entity2dRenderComp er(entity, render, true);

	for (size_t i = 0; i < _layers.Size(); i++)
	{
		SLayer &layer = _layers[i];

		if (layer._minPriority <= er._priority && layer._maxPriority >= er._priority)
		{
			layer._objects.SortInsert(er);
			break;
		}
	}
}

void RenderSystem::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	assert(!_rendering);

	ff::ComPtr<I2dRenderComponent> render;
	assertRet(render.QueryFrom(pComp));

	Entity2dRenderComp er(entity, render, true);

	for (size_t i = 0; i < _layers.Size(); i++)
	{
		SLayer &layer = _layers[i];

		if (layer._minPriority <= er._priority && layer._maxPriority >= er._priority)
		{
			size_t index;
			assertRet(layer._objects.SortFind(er, &index));
			assert(layer._objects[index] == er);

			layer._objects.Delete(index);
			break;
		}
	}
}

void RenderSystem::Render(I2dLayerGroup *group, ff::I2dRenderer *render)
{
	for (size_t i = 0; i < _layers.Size(); i++)
	{
		SLayer &layer = _layers[i];

		if (group == layer._group)
		{
			int nPrevPriority = 0;
			int nPriority = 0;

			for (size_t h = 0; h < layer._objects.Size(); h++, nPrevPriority = nPriority)
			{
				const Entity2dRenderComp &er = layer._objects[h];
				nPriority = er._priority;

				if (nPriority != nPrevPriority && h != 0)
				{
					render->Flush();
				}

				er._component->Render(er._entity, group, render);
			}

			break;
		}
	}
}
