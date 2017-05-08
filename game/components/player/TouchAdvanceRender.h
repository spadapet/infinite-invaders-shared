#pragma once

#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "InputEvents.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class ISprite;
}

class IEntity;
class GlobalPlayerService;

class __declspec(uuid("877578b6-ce11-4c10-ac75-d0ff7b3a93f8"))
	TouchAdvanceRender
		: public ff::ComBase
		, public IAdvanceComponent
		, public I2dRenderComponent
		, public IEntityEventListener
		, public IExternalPlayerControl
{
public:
	DECLARE_HEADER(TouchAdvanceRender);

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// IExternalPlayerControl
	virtual int GetDigitalPlayerValue(PlayerInputValue type) const override;
	virtual float GetAnalogPlayerValue(PlayerInputValue type) const override;

	// ComBase
	HRESULT _Construct(IUnknown *unkOuter) override;

private:
	enum class ButtonType
	{
		BT_NONE,
		BT_LEFT,
		BT_RIGHT,
		BT_SHOOT,
		BT_PAUSE,
	};

	ButtonType GetButton(ff::PointFloat screenPos) const;
	ff::RectFloat GetButtonRect(ButtonType type) const;
	float GetMinShootCenter() const;

	ff::ComPtr<GlobalPlayerService> _globalPlayers;
	ff::ComPtr<ff::ISprite> _leftSprite;
	ff::ComPtr<ff::ISprite> _rightSprite;
	ff::ComPtr<ff::ISprite> _shootSprite;
	ff::ComPtr<ff::ISprite> _leftSpritePressed;
	ff::ComPtr<ff::ISprite> _rightSpritePressed;
	ff::ComPtr<ff::ISprite> _shootSpritePressed;
	ff::ComPtr<ff::ISprite> _pauseSprite;
	ff::ComPtr<ff::ISprite> _pauseSpritePressed;
	size_t _framesSinceTouch;
	ff::PointFloat _screenSize;
	float _dpiScale;
	float _moveCenterY;
	float _shootCenterY;
	bool _playerActive;
	bool _touchingLeft;
	bool _touchingRight;
	bool _touchingShoot;
	bool _touchingPause;
	bool _startedTouchingPause;
};
