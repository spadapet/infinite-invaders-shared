#pragma once

#include "coreEntity\system\System.h"

namespace ff
{
	class ISprite;
}

class I2dLayerGroup;
class ITitleState;

class __declspec(uuid("29bd0086-cf06-444d-99d0-a0c46a6bc841")) __declspec(novtable)
	IInitState : public ISystem
{
	// Only used as a component ID
};

class __declspec(uuid("ab1fa798-f52e-47e9-a750-28703413bd45"))
	InitState : public ff::ComBase, public IInitState
{
public:
	DECLARE_HEADER(InitState);

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// ComBase
	HRESULT _Construct(IUnknown *unkOuter) override;

private:
	size_t _counter;
	ff::ComPtr<I2dLayerGroup> _loadingGroup;
	ff::ComPtr<ITitleState> _pTitle;
	ff::ComPtr<ff::ISprite> _pSplashSprite;
	Windows::Foundation::Rect _splashLocation;
};
