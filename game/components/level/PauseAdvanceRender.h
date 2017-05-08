#pragma once

#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class IAudioEffect;
	class ISpriteAnimation;
	class ISpriteList;
	class ISpriteFont;
}

class LoadingComponent;
class MultiSpriteRender;
class IEntity;

enum PauseButtonType
{
	PAUSE_BUTTON_CONTINUE,
	PAUSE_BUTTON_EXIT,
	PAUSE_BUTTON_EXIT2,
	PAUSE_BUTTON_SOUND,
	PAUSE_BUTTON_REPLAY,
	PAUSE_BUTTON_REPLAY2,

	PAUSE_BUTTON_COUNT
};

class __declspec(uuid("e447a2d9-3a0d-4e9b-8a15-c3bac6322b97"))
	PauseAdvanceRender
		: public ff::ComBase
		, public IAdvanceComponent
		, public I2dRenderComponent
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(PauseAdvanceRender);

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// ComBase
	HRESULT _Construct(IUnknown *unkOuter) override;

private:
	typedef void (PauseAdvanceRender::*FButtonAction)(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType);

	struct ButtonInfo
	{
		ButtonInfo();

		ff::ComPtr<MultiSpriteRender> _spriteRender;
		ff::TypedResource<ff::ISpriteAnimation> _outlineAnim;

		PauseButtonType _moveUp;
		PauseButtonType _moveDown;
		ff::String _text;
		ff::PointFloat _pos;
		FButtonAction _action;
		bool _visible;
	};

	bool CreateButtonSprites(IEntity *pPauseEntity, ButtonInfo &info, bool bArrowKeys);
	bool CreateSoundButton(IEntity *pPauseEntity, ButtonInfo &info);
	bool OnLoadingComplete(IEntity *pPauseEntity);
	void SelectButton(IEntity *pPauseEntity, PauseButtonType type, bool bPlayEffect = true);

	void HandleEventStart(IEntity *pPauseEntity, ff::hash_t type);
	void HandleEventStop(IEntity *pPauseEntity, ff::hash_t type);

	void HandleContinueButton(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType);
	void HandleExitButton(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType);
	void HandleSoundButton(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType);
	void HandleReplayButton(IEntity *pPauseEntity, PauseButtonType type, ff::hash_t actionType);

	EntityEventListener _invadersDancingListener;
	ff::TypedResource<ff::ISpriteFont> _font;
	ff::TypedResource<ff::ISpriteList> _spriteList;
	ff::TypedResource<ff::ISpriteFont> _spaceFont;
	ff::TypedResource<ff::IAudioEffect> _effectExecute;
	ff::TypedResource<ff::IAudioEffect> _effectMain;
	ff::ComPtr<ff::IInputMapping> _inputMapping;
	ff::ComPtr<ff::IInputMapping> _pausedInputMapping;
	ff::ComPtr<LoadingComponent> _loading;
	ButtonInfo _buttons[PAUSE_BUTTON_COUNT];
	PauseButtonType _selectedButton;
	ff::PointFloat _mousePos;
	size_t _paused;
	float _outlineFrame;
	bool _loaded;
	bool _wasGamePaused;
	bool _wereInvadersDancing;
	size_t _invadersDancing;
};
