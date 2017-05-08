#pragma once

namespace ff
{
	class I2dEffect;
	class I2dRenderer;
	class IRenderDepth;
	class IRenderTarget;
}

class I2dLayerGroup;
class IEntityDomain;

enum LayerPriority : int
{
	LAYER_PRI_CLEAR = 1000,

	LAYER_PRI_UNDERLAY_LOWEST = 2000,
	LAYER_PRI_UNDERLAY_LOW = 2100,
	LAYER_PRI_UNDERLAY_NORMAL = 2200,
	LAYER_PRI_UNDERLAY_HIGH = 2300,
	LAYER_PRI_UNDERLAY_HIGHEST = 2400,

	LAYER_PRI_LOWEST = 3000,
	LAYER_PRI_LOW = 3100,
	LAYER_PRI_NORMAL = 3200,
	LAYER_PRI_HIGH = 3300,
	LAYER_PRI_HIGHEST = 3400,

	LAYER_PRI_OVERLAY_LOWEST = 4000,
	LAYER_PRI_OVERLAY_LOW = 4100,
	LAYER_PRI_OVERLAY_NORMAL = 4200,
	LAYER_PRI_OVERLAY_HIGH = 4300,
	LAYER_PRI_OVERLAY_HIGHEST = 4400,

	LAYER_PRI_CURSOR = 5000,
};

class __declspec(uuid("03fc6b34-9708-4e88-bcec-06e985520ef9"))
	LayerBase2d : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(LayerBase2d);

	int GetLayerPriority() const;
	void SetLayerPriority(int nPriority);

	ff::I2dEffect* GetEffect();
	void SetEffect(ff::I2dEffect* pEffect);

	virtual void OnAddedToGroup(I2dLayerGroup *group);
	virtual void OnRemovedFromGroup(I2dLayerGroup *group);

	virtual void Advance(I2dLayerGroup *group);
	virtual void Render(I2dLayerGroup *group, ff::I2dRenderer *render) = 0;

private:
	ff::I2dEffect* _effect;
	int _priority;
};

class __declspec(uuid("dd8e7a6a-27cc-4023-ada0-98ed5316b601")) __declspec(novtable)
	ILayerViewport : public IUnknown
{
public:
	virtual IEntityDomain* GetDomain() const = 0;

	virtual bool IsVisible() const = 0;
	virtual void SetVisible(bool bVisible) = 0;

	virtual const ff::RectInt* GetViewRects() const = 0;
	virtual size_t GetViewRectCount() const = 0;
	virtual void SetViewRects(const ff::RectInt *pRects, size_t nCount) = 0;
	virtual void SetAutoSizePadding(ff::RectInt padding) = 0;
	virtual void SetAutoSizeAspect(ff::PointInt size) = 0;

	virtual ff::IRenderTarget** GetRenderTargets() const = 0;
	virtual size_t GetRenderTargetCount() const = 0;
	virtual void SetRenderTargets(ff::IRenderTarget** ppTargets, size_t nCount) = 0;

	virtual ff::IRenderDepth* GetDepth() const = 0;
	virtual void SetDepth(ff::IRenderDepth *pDepth) = 0;
	virtual bool IsUsingDepth() const = 0;
	virtual void SetUseDepth(bool bUseDepth) = 0;

	virtual bool UpdateForRender() = 0;
};

bool CreateLayerViewport(IEntityDomain *pDomain, ILayerViewport **ppViewport);

class __declspec(uuid("5044d86d-30a3-4714-a86d-3773377fbe56")) __declspec(novtable)
	I2dLayerGroup : public IUnknown
{
public:
	virtual ILayerViewport* GetViewport() const = 0;

	virtual void Advance() = 0;
	virtual void Render(ff::I2dRenderer *render) = 0;

	virtual bool BeginCustomRender(ff::I2dRenderer *render) = 0;
	virtual void EndCustomRender(ff::I2dRenderer *render) = 0;

	virtual bool AddLayer(LayerBase2d *pLayer) = 0;
	virtual bool RemoveLayer(LayerBase2d *pLayer) = 0;
	virtual LayerBase2d* GetLayer(size_t index) const = 0;
	virtual size_t GetLayerCount() const = 0;

	virtual ff::I2dEffect* GetEffect() = 0;
	virtual void SetEffect(ff::I2dEffect* pEffect) = 0;

	virtual ff::PointFloat GetWorldTopLeft() const = 0;
	virtual void SetWorldTopLeft(ff::PointFloat origin) = 0;

	virtual ff::PointFloat GetWorldScale() const = 0;
	virtual void SetWorldScale(ff::PointFloat scale) = 0;

	virtual ff::PointFloat GetWorldSize() const = 0;
	virtual void SetWorldSize(ff::PointFloat size) = 0;
};

bool Create2dLayerGroup(ILayerViewport *pViewport, I2dLayerGroup **ppGroup);
bool Create2dLayerGroup(IEntityDomain *pDomain, I2dLayerGroup **ppGroup);
