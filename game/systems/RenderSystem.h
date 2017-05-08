#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\system\System.h"
#include "entities\Entity2dRenderComp.h"

namespace ff
{
	class ISprite;
}

class LayerProxy2d;
class MouseLayer;
class IEntity;

class __declspec(uuid("d71ed12e-df07-4440-bc49-74b9ff685973"))
	RenderSystem
		: public LayerBase2d
		, public ISystem
		, public IComponentListener
{
public:
	DECLARE_HEADER(RenderSystem);

	ff::PointFloat WindowClientToLevel(ff::PointFloat client);
	void SetRenderer(ff::I2dRenderer *render);
	void SetMouseCursor(ff::ISprite *pSprite);

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render (IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

	// LayerBase2d
	virtual void Render(I2dLayerGroup *group, ff::I2dRenderer *render) override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	struct SLayer
	{
		int _minPriority;
		int _maxPriority;
		ff::ComPtr<I2dLayerGroup> _group;
		ff::ComPtr<LayerProxy2d> _layer;
		ff::Vector<Entity2dRenderComp> _objects;
	};

	void UpdateViewports(IEntityDomain *pDomain, ff::IRenderTarget *pTarget);
	void UpdateLevelLayerGroup(IEntityDomain *pDomain, I2dLayerGroup *group);

	ComponentListener<I2dRenderComponent> _renderListener;
	ff::ComPtr<ff::I2dRenderer> _pRender;
	ff::ComPtr<MouseLayer> _mouseLayer;
	ff::ComPtr<ff::ISprite> _cursor;
	ff::Vector<SLayer> _layers;
	ff::PointFloat _debugCenter; // for right click zooming
	bool _rendering;
};
