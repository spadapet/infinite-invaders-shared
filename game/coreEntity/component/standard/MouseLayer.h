#pragma once

#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "Graph\2D\SpritePos.h"

namespace ff
{
	class I2dRenderer;
	class IPointerDevice;
	class ISprite;
}

class __declspec(uuid("68383030-4094-4cec-9c52-438510259418"))
	MouseLayer : public LayerBase2d
{
public:
	DECLARE_HEADER(MouseLayer);

	bool Init(ff::IPointerDevice *pMouse);

	void SetCursor(ff::ISprite* pCursor, const ff::SpritePos &pos);
	void SetFullScreenRender(bool bFullScreen);
	void SetMaxVisibleTime(double seconds, double fadeAfter); // zero to clear max time

	// LayerBase2d
	virtual void Render(I2dLayerGroup *group, ff::I2dRenderer *render) override;

private:
	ff::ComPtr<ff::IPointerDevice> _mouse;
	ff::ComPtr<ff::ISprite> _cursor;
	ff::SpritePos _spritePos;
	double _maxVisibleSeconds;
	double _startVisibleSeconds;
	double _fadeAfterSeconds;
	bool _fullScreen;
};

bool CreateMouseLayer(ff::IPointerDevice *pDevice, MouseLayer **ppLayer);

