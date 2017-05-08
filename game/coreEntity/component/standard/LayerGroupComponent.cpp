#include "pch.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dEffect.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Module\ModuleFactory.h"
#include "ThisApplication.h"

BEGIN_INTERFACES(LayerBase2d)
END_INTERFACES()

LayerBase2d::LayerBase2d()
	: _priority(LAYER_PRI_NORMAL)
	, _effect(nullptr)
{
}

LayerBase2d::~LayerBase2d()
{
}

int LayerBase2d::GetLayerPriority() const
{
	return _priority;
}

void LayerBase2d::SetLayerPriority(int nPriority)
{
	_priority = nPriority;
}

ff::I2dEffect *LayerBase2d::GetEffect()
{
	return _effect;
}

void LayerBase2d::SetEffect(ff::I2dEffect* pEffect)
{
	_effect = pEffect;
}

void LayerBase2d::OnAddedToGroup(I2dLayerGroup *group)
{
}

void LayerBase2d::OnRemovedFromGroup(I2dLayerGroup *group)
{
}

void LayerBase2d::Advance(I2dLayerGroup *group)
{
}

class __declspec(uuid("d79f3576-a8f9-4333-9af3-286657ad2965"))
	LayerViewport : public ff::ComBase, public ILayerViewport
{
public:
	DECLARE_HEADER(LayerViewport);

	bool Init(IEntityDomain *pDomain);

	// ILayerViewport

	virtual IEntityDomain* GetDomain() const override;

	virtual bool IsVisible() const override;
	virtual void SetVisible(bool bVisible) override;

	virtual const ff::RectInt* GetViewRects() const override;
	virtual size_t GetViewRectCount() const override;
	virtual void SetViewRects(const ff::RectInt *pRects, size_t nCount) override;
	virtual void SetAutoSizePadding(ff::RectInt padding) override;
	virtual void SetAutoSizeAspect(ff::PointInt size) override;

	virtual ff::IRenderTarget** GetRenderTargets() const override;
	virtual size_t GetRenderTargetCount() const override;
	virtual void SetRenderTargets(ff::IRenderTarget** ppTargets, size_t nCount) override;

	virtual ff::IRenderDepth* GetDepth() const override;
	virtual void SetDepth(ff::IRenderDepth *pDepth) override;
	virtual bool IsUsingDepth() const override;
	virtual void SetUseDepth(bool bUseDepth) override;

	virtual bool UpdateForRender() override;

private:
	ff::Vector<ff::ComPtr<ff::IRenderTarget>> _renderTargets;
	ff::ComPtr<ff::IRenderTargetWindow> _renderWindow;
	ff::ComPtr<ff::IRenderDepth> _depth;
	IEntityDomain* _domain;
	ff::Vector<ff::RectInt> _viewRects;
	ff::RectInt _autoSizePadding;
	ff::PointInt _autoSizeAspect;
	ff::PointInt _autoSizePreviousTarget;
	ff::PointInt _autoSizePreviousClient;
	bool _visible;
	bool _autoSize;
	bool _autoSizeComputed;
	bool _useDepth;
};

BEGIN_INTERFACES(LayerViewport)
	HAS_INTERFACE(ILayerViewport)
END_INTERFACES()

bool CreateLayerViewport(IEntityDomain *pDomain, ILayerViewport **ppViewport)
{
	assertRetVal(ppViewport, false);
	*ppViewport = nullptr;

	ff::ComPtr<LayerViewport> pViewport;
	assertRetVal(SUCCEEDED(ff::ComAllocator<LayerViewport>::CreateInstance(&pViewport)), false);
	assertRetVal(pViewport->Init(pDomain), false);

	*ppViewport = ff::GetAddRef(pViewport.Interface());
	return true;
}

LayerViewport::LayerViewport()
	: _domain(nullptr)
	, _autoSizePadding(0, 0, 0, 0)
	, _autoSizeAspect(0, 0)
	, _autoSizePreviousTarget(0, 0)
	, _autoSizePreviousClient(0, 0)
	, _visible(true)
	, _autoSize(true)
	, _autoSizeComputed(false)
	, _useDepth(true)
{
}

LayerViewport::~LayerViewport()
{
}

bool LayerViewport::Init(IEntityDomain *pDomain)
{
	assertRetVal(pDomain, false);
	_domain = pDomain;

	// By default, render to the main window
	ff::IRenderTarget *pRenderTarget = ff::MetroGlobals::Get()->GetTarget();

	if (pRenderTarget)
	{
		SetRenderTargets(&pRenderTarget, 1);
	}

	return true;
}

IEntityDomain *LayerViewport::GetDomain() const
{
	return _domain;
}

bool LayerViewport::IsVisible() const
{
	return _visible;
}

void LayerViewport::SetVisible(bool bVisible)
{
	_visible = bVisible;
}

const ff::RectInt* LayerViewport::GetViewRects() const
{
	return _viewRects.Size() ? _viewRects.Data() : nullptr;
}

size_t LayerViewport::GetViewRectCount() const
{
	return _viewRects.Size();
}

void LayerViewport::SetViewRects(const ff::RectInt *pRects, size_t nCount)
{
	if ((!pRects || !nCount) && _autoSize)
	{
		// didn't change
		return;
	}
	else if (pRects && nCount && _viewRects.Size() == nCount &&
		!memcmp(pRects, _viewRects.Data(), _viewRects.ByteSize()))
	{
		// didn't change
		return;
	}

	_viewRects.Clear();
	_autoSizeComputed = false;
	_autoSize = !pRects || !nCount;

	if (pRects && nCount)
	{
		_viewRects.Push(pRects, nCount);
	}
}

void LayerViewport::SetAutoSizePadding(ff::RectInt padding)
{
	_autoSizeComputed = false;

	if (padding.left >= 0 &&
		padding.top >= 0 &&
		padding.right >= 0 &&
		padding.bottom >= 0)
	{
		_autoSizePadding = padding;
	}
}

void LayerViewport::SetAutoSizeAspect(ff::PointInt size)
{
	_autoSizeComputed = false;

	if (size.x >= 0 && size.y >= 0)
	{
		_autoSizeAspect = size;
	}
}

ff::IRenderTarget **LayerViewport::GetRenderTargets() const
{
	return _renderTargets.Size()
		? const_cast<ff::ComPtr<ff::IRenderTarget>&>(_renderTargets[0]).Address()
		: nullptr;
}

size_t LayerViewport::GetRenderTargetCount() const
{
	return _renderTargets.Size();
}

void LayerViewport::SetRenderTargets(ff::IRenderTarget** ppTargets, size_t nCount)
{
	bool bChanged = false;

	if (!ppTargets || !nCount)
	{
		ff::IRenderTarget *pRenderTarget = ff::MetroGlobals::Get()->GetTarget();

		if (pRenderTarget)
		{
			SetRenderTargets(&pRenderTarget, 1);
		}
		else
		{
			_renderTargets.Clear();
			bChanged = true;
		}
	}
	else if (nCount != _renderTargets.Size() ||
		memcmp(ppTargets, _renderTargets.Data(), _renderTargets.ByteSize()))
	{
		_renderTargets.Clear();
		bChanged = true;

		for (size_t i = 0; i < nCount; i++)
		{
			_renderTargets.Push(ppTargets[i]);
		}
	}

	if (bChanged)
	{
		_renderWindow = nullptr;
		_autoSizeComputed = false;

		if (_renderTargets.Size())
		{
			_renderWindow.QueryFrom(_renderTargets[0]);
		}

		SetViewRects(nullptr, 0);
	}
}

ff::IRenderDepth *LayerViewport::GetDepth() const
{
	ff::IRenderDepth *pDepth = nullptr;

	if (_useDepth)
	{
		pDepth = (!_depth && _renderTargets.Size())
			? ff::MetroGlobals::Get()->GetDepth()
			: _depth;
	}

	return pDepth;
}

void LayerViewport::SetDepth(ff::IRenderDepth *pDepth)
{
	_depth = pDepth;
}

bool LayerViewport::IsUsingDepth() const
{
	return _useDepth;
}

void LayerViewport::SetUseDepth(bool bUseDepth)
{
	_useDepth = bUseDepth;
}

bool LayerViewport::UpdateForRender()
{
	if (_autoSize && _renderTargets.Size())
	{
		ff::PointInt targetSize = _renderTargets[0]->GetRotatedSize();
		ff::PointInt clientSize = _renderWindow != nullptr
			? _renderWindow->GetRotatedSize()
			: targetSize;

		if (clientSize.x && clientSize.y)
		{
			if (!_autoSizeComputed ||
				_autoSizePreviousTarget != targetSize ||
				_autoSizePreviousClient != clientSize)
			{
				_viewRects.Clear();
				_autoSizePreviousTarget = targetSize;
				_autoSizePreviousClient = clientSize;
				_autoSizeComputed = true;

				ff::RectInt totalArea(0, 0, targetSize.x, targetSize.y);
				ff::RectInt safeArea = totalArea;

				if (safeArea.Width() > _autoSizePadding.left + _autoSizePadding.right &&
					safeArea.Height() > _autoSizePadding.top + _autoSizePadding.bottom)
				{
					// Adjust for padding
					safeArea.Deflate(_autoSizePadding);
				}

				ff::RectInt rect = (_autoSizeAspect.x && _autoSizeAspect.y)
					? ff::RectInt(0, 0, _autoSizeAspect.x, _autoSizeAspect.y)
					: safeArea;

				if (targetSize != clientSize)
				{
					// While resizing the main window, the aspect ratio needs to be adjusted
					rect.left = rect.left * targetSize.x / clientSize.x;
					rect.right = rect.right * targetSize.x / clientSize.x;
					rect.top = rect.top * targetSize.y / clientSize.y;
					rect.bottom = rect.bottom * targetSize.y / clientSize.y;
				}

				if (rect != safeArea)
				{
					rect.ScaleToFit(safeArea);
					rect.CenterWithin(safeArea);
				}

				if (rect.IsInside(totalArea) && rect.Width() >= 16 && rect.Height() >= 16)
				{
					_viewRects.Push(rect);
				}
				else
				{
					int i = 0;
				}
			}
		}
	}

	return true;
}

class __declspec(uuid("5044d86d-30a3-4714-a86d-3773377fbe56")) __declspec(novtable)
	C2dLayerGroup : public ff::ComBase, public I2dLayerGroup
{
public:
	DECLARE_HEADER(C2dLayerGroup);

	bool Init(ILayerViewport *pViewport);

	// I2dLayerGroup

	virtual ILayerViewport* GetViewport() const override;

	virtual void Advance() override;
	virtual void Render(ff::I2dRenderer *render) override;

	virtual bool BeginCustomRender(ff::I2dRenderer *render) override;
	virtual void EndCustomRender(ff::I2dRenderer *render) override;

	virtual bool AddLayer(LayerBase2d *pLayer) override;
	virtual bool RemoveLayer(LayerBase2d *pLayer) override;
	virtual LayerBase2d* GetLayer(size_t index) const override;
	virtual size_t GetLayerCount() const override;

	virtual ff::I2dEffect* GetEffect() override;
	virtual void SetEffect(ff::I2dEffect* pEffect) override;

	virtual ff::PointFloat GetWorldTopLeft() const override;
	virtual void SetWorldTopLeft(ff::PointFloat origin) override;

	virtual ff::PointFloat GetWorldScale() const override;
	virtual void SetWorldScale(ff::PointFloat scale) override;

	virtual ff::PointFloat GetWorldSize() const override;
	virtual void SetWorldSize(ff::PointFloat size) override;

private:
	static int CompareLayers(const void *p1, const void *p2);

	ff::ComPtr<ILayerViewport> _pViewport;
	ff::ComPtr<ff::I2dEffect> _effect;
	ff::ComPtr<IEntity> _pLayersEntity;
	ff::Vector<ff::ComPtr<LayerBase2d>> _layers;
	ff::PointFloat _worldTopLeft;
	ff::PointFloat _worldScale;
	ff::PointFloat _worldSize;
	bool _bLayersChanged;
};

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"C2dLayerGroup");
	module.RegisterClassT<C2dLayerGroup>(name, __uuidof(I2dLayerGroup));
});;

BEGIN_INTERFACES(C2dLayerGroup)
	HAS_INTERFACE(I2dLayerGroup)
END_INTERFACES()

bool Create2dLayerGroup(ILayerViewport *pViewport, I2dLayerGroup **ppGroup)
{
	assertRetVal(ppGroup, false);
	*ppGroup = nullptr;

	ff::ComPtr<C2dLayerGroup, I2dLayerGroup> group;
	assertRetVal(SUCCEEDED(ff::ComAllocator<C2dLayerGroup>::CreateInstance(&group)), false);
	assertRetVal(group->Init(pViewport), false);

	*ppGroup = ff::GetAddRef(group.Interface());
	return true;
}

bool Create2dLayerGroup(IEntityDomain *pDomain, I2dLayerGroup **ppGroup)
{
	ff::ComPtr<ILayerViewport> pGroupViewport;

	assertRetVal(CreateLayerViewport(pDomain, &pGroupViewport), false);
	assertRetVal(Create2dLayerGroup(pGroupViewport, ppGroup), false);

	return true;
}

C2dLayerGroup::C2dLayerGroup()
	: _worldTopLeft(0, 0)
	, _worldScale(1, 1)
	, _worldSize(0, 0)
	, _bLayersChanged(false)
{
}

C2dLayerGroup::~C2dLayerGroup()
{
}

bool C2dLayerGroup::Init(ILayerViewport *pViewport)
{
	assertRetVal(pViewport, false);
	_pViewport = pViewport;

	assertRetVal(_pViewport->GetDomain()->GetEntityManager()->CreateEntity(&_pLayersEntity), false);

	return true;
}

ILayerViewport *C2dLayerGroup::GetViewport() const
{
	return _pViewport;
}

bool C2dLayerGroup::AddLayer(LayerBase2d *pLayer)
{
	assertRetVal(pLayer && _pLayersEntity->AddComponent<LayerBase2d>(pLayer), false);
	_layers.Push(pLayer);

	pLayer->OnAddedToGroup(this);

	_bLayersChanged = true;

	return true;
}

bool C2dLayerGroup::RemoveLayer(LayerBase2d *pLayer)
{
	assertRetVal(_pLayersEntity->RemoveComponent<LayerBase2d>(pLayer), false);

	size_t i = _layers.Find(pLayer);
	assertRetVal(i != ff::INVALID_SIZE, false);

	_layers.Delete(i);

	pLayer->OnRemovedFromGroup(this);

	return true;
}

LayerBase2d *C2dLayerGroup::GetLayer(size_t index) const
{
	assertRetVal(index >= 0 && index < _layers.Size(), nullptr);
	return _layers[index];
}

size_t C2dLayerGroup::GetLayerCount() const
{
	return _layers.Size();
}

ff::I2dEffect *C2dLayerGroup::GetEffect()
{
	return _effect ? _effect : ff::MetroGlobals::Get()->Get2dEffect();
}

void C2dLayerGroup::SetEffect(ff::I2dEffect* pEffect)
{
	_effect = pEffect;
}

ff::PointFloat C2dLayerGroup::GetWorldTopLeft() const
{
	return _worldTopLeft;
}

void C2dLayerGroup::SetWorldTopLeft(ff::PointFloat origin)
{
	_worldTopLeft = origin;
}

ff::PointFloat C2dLayerGroup::GetWorldScale() const
{
	return _worldScale;
}

void C2dLayerGroup::SetWorldScale(ff::PointFloat scale)
{
	if (scale.x != 0 && scale.y != 0)
	{
		_worldScale = scale;
		_worldSize.SetPoint(0, 0);
	}
}

ff::PointFloat C2dLayerGroup::GetWorldSize() const
{
	return _worldSize;
}

void C2dLayerGroup::SetWorldSize(ff::PointFloat size)
{
	if (size.x != 0 && size.y != 0)
	{
		_worldSize = size;
		_worldScale.SetPoint(0, 0);
	}
}

void C2dLayerGroup::Advance()
{
	for (size_t i = 0; i < _layers.Size(); i++)
	{
		_layers[i]->Advance(this);
	}
}

void C2dLayerGroup::Render(ff::I2dRenderer *render)
{
	if (BeginCustomRender(render))
	{
		// Sort the layers

		if (_bLayersChanged)
		{
			_bLayersChanged = false;

			qsort(_layers.Data(), _layers.Size(), sizeof(LayerBase2d*), CompareLayers);
		}

		// Render each layer

		for (size_t i = 0; i < _layers.Size(); i++)
		{
			LayerBase2d *pLayer = _layers[i];
			bool bPushedEffect = false;

			if (pLayer->GetEffect())
			{
				if (!render->PushEffect(pLayer->GetEffect()))
				{
					assert(false);
					break;
				}

				bPushedEffect = true;
			}

			pLayer->Render(this, render);
			render->Flush();

			if (bPushedEffect)
			{
				render->PopEffect();
			}
		}

		EndCustomRender(render);
	}
}

bool C2dLayerGroup::BeginCustomRender(ff::I2dRenderer *render)
{
	if (!render)
	{
		render = ff::MetroGlobals::Get()->Get2dRender();;
	}

	if (render &&
		_pViewport->IsVisible() &&
		_pViewport->UpdateForRender() &&
		_pViewport->GetRenderTargetCount() &&
		_pViewport->GetViewRectCount())
	{
		// Begin rendering

		ff::IRenderTarget* pTarget = _pViewport->GetRenderTargets()[0];
		ff::IRenderDepth* pDepth = _pViewport->GetDepth();
		const ff::RectInt& rect = _pViewport->GetViewRects()[0];

		if (_worldSize.x && _worldSize.y && rect.Width() && rect.Height())
		{
			_worldScale.SetPoint(_worldSize.x / rect.Width(), _worldSize.y / rect.Height());
		}

		ff::PointFloat renderScale = _worldScale;
		renderScale.x = (renderScale.x != 0) ? 1.0f / renderScale.x : 0.0f;
		renderScale.y = (renderScale.y != 0) ? 1.0f / renderScale.y : 0.0f;

		if (render->BeginRender(pTarget, pDepth, rect.ToFloat(), _worldTopLeft, renderScale, GetEffect()))
		{
			return true;
		}
	}

	return false;
}

void C2dLayerGroup::EndCustomRender(ff::I2dRenderer *render)
{
	render->EndRender();
}

// static
int C2dLayerGroup::CompareLayers(const void *p1, const void *p2)
{
	LayerBase2d *l1 = *(LayerBase2d**)p1;
	LayerBase2d *l2 = *(LayerBase2d**)p2;

	return l1->GetLayerPriority() - l2->GetLayerPriority();
}
