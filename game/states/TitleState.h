#pragma once

#include "coreEntity\system\System.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class IAudioEffect;
	class ISprite;
	class ISpriteAnimation;
	class ISpriteFont;
	class ISpriteList;
}

class LoadingComponent;
class MultiSpriteRender;
class IEntity;

class __declspec(uuid("a184fd5c-4443-453c-b2fa-a2a3c33c583a")) __declspec(novtable)
	ITitleState : public ISystem
{
	// Only used as a component ID
};

enum TitleButtonContext
{
	TITLE_CONTEXT_UNKNOWN,
	TITLE_CONTEXT_MAIN,
	TITLE_CONTEXT_PLAY,
	TITLE_CONTEXT_OPTIONS,
	TITLE_CONTEXT_MORE,

	TITLE_CONTEXT_COUNT
};

enum TitleButtonType
{
	TITLE_BUTTON_PLAY,
	TITLE_BUTTON_OPTIONS,
	TITLE_BUTTON_SCORES,
	TITLE_BUTTON_MORE,

	TITLE_BUTTON_PLAY_1P,
	TITLE_BUTTON_PLAY_2P,
	TITLE_BUTTON_PLAY_COOP,
	TITLE_BUTTON_PLAY_CONTINUE,

	TITLE_BUTTON_OPTION_EASY,
	TITLE_BUTTON_OPTION_NORMAL,
	TITLE_BUTTON_OPTION_HARD,
	TITLE_BUTTON_OPTION_FXON,
	TITLE_BUTTON_OPTION_FXOFF,
	TITLE_BUTTON_OPTION_VSYNCON,
	TITLE_BUTTON_OPTION_VSYNCOFF,
	TITLE_BUTTON_OPTION_FULLSCREENON,
	TITLE_BUTTON_OPTION_FULLSCREENOFF,

	TITLE_BUTTON_MORE_HELP,
	TITLE_BUTTON_MORE_QUIT,

	TITLE_BUTTON_COUNT
};

class __declspec(uuid("8dd20b37-9ac0-4a65-a074-1a987c0124e3"))
	TitleState : public ff::ComBase , public ITitleState
{
public:
	DECLARE_HEADER(TitleState);

	void SetActiveGame(ISystem *pActiveGame);

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// ComBase
	HRESULT _Construct(IUnknown *unkOuter) override;

private:
	typedef void (TitleState::*FButtonAction)(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);

	struct SButtonInfo
	{
		SButtonInfo();

		ff::ComPtr<IEntity> _entity;
		ff::ComPtr<IEntity> _pHelentity;
		ff::ComPtr<MultiSpriteRender> _spriteRender;
		ff::TypedResource<ff::ISpriteAnimation> _outlineAnim;
		ff::TypedResource<ff::ISpriteAnimation> _pHelpAnim;

		TitleButtonType _moveLeft;
		TitleButtonType _moveRight;
		TitleButtonType _moveUp;
		TitleButtonType _moveDown;
		ff::String _text;
		ff::PointFloat _pos;
		FButtonAction _action;
		bool _visible;
	};

	bool CreateButtonSprites(IEntityDomain *pDomain, SButtonInfo &info, LPCTSTR szButtonSprite, LPCTSTR szOutlineAnim, ff::PointFloat textScale = ff::PointFloat(.75, .75));
	void OnLoadingComplete(IEntityDomain *pDomain);
	bool InitButtons(IEntityDomain *pDomain);
	void UpdateButton(TitleButtonType type);
	void UpdateButtonVisibility();
	void SelectButton(TitleButtonType type, bool bPlayEffect = true);
	void HandleEventStart(IEntityDomain *pDomain, ff::hash_t type);
	void HandleEventStop(IEntityDomain *pDomain, ff::hash_t type);

	void HandlePlayButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleOptionsButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleScoreButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleMoreButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleHelpButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleQuitButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleStartGameButton(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleToggleDifficulty(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleToggleSound(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleToggleVsync(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);
	void HandleToggleFullScreen(IEntityDomain *pDomain, TitleButtonType type, ff::hash_t actionType);

	ff::ComPtr<ff::IInputMapping> _inputMapping;
	ff::TypedResource<ff::ISpriteList> _spriteList;
	ff::TypedResource<ff::ISpriteAnimation> _pStripeAnim;
	ff::TypedResource<ff::ISpriteAnimation> _pTextAnim;
	ff::TypedResource<ff::ISpriteAnimation> _pLogoAnim;
	ff::TypedResource<ff::ISpriteAnimation> _pNameAnim;
	ff::ComPtr<ff::ISprite> _pHandCursor;
	TitleButtonType _selectedButton;
	TitleButtonContext _buttonContext;
	ff::PointFloat _mousePos;

	ff::ComPtr<IEntityDomain> _pChildDomain; // must be before entities
	ff::ComPtr<IEntity> _pChildEntity;
	ff::ComPtr<IEntity> _pStripeEntity;
	ff::ComPtr<IEntity> _pTextEntity;
	ff::ComPtr<IEntity> _pLogoEntity;
	ff::ComPtr<IEntity> _pNameEntity;
	ff::ComPtr<IEntity> _pOutlineEntity;
	ff::TypedResource<ff::ISpriteFont> _spaceFont;
	ff::TypedResource<ff::IAudioEffect> _effectExecute;
	ff::TypedResource<ff::IAudioEffect> _effectMain;
	ff::TypedResource<ff::IAudioEffect> _effectSub;
	SButtonInfo _buttons[TITLE_BUTTON_COUNT];

	size_t _nResetFullScreen;
	bool _bExpectedEffectsOn;
	bool _bHasActiveGame;
	ff::ComPtr<ISystem> _pActiveGame;
	ff::ComPtr<LoadingComponent> _loading;
};
