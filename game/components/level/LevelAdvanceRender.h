#pragma once

#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class ISprite;
	class ISpriteAnimation;
	class ISpriteFont;
}

class IEntity;

class __declspec(uuid("edad4310-f3fa-45e7-a35d-1a2201cb4488"))
	LevelAdvanceRender
		: public ff::ComBase
		, public IAdvanceComponent
		, public I2dRenderComponent
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(LevelAdvanceRender);

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	void HandleCollision(IEntity *entity, IEntity *otherEntity);
	void OnInvadersWin();

	static const size_t GROUND_ROWS = 3;
	static const size_t GROUND_COLS = 30;

	enum LevelType
	{
		LEVEL_TYPE_INVADERS,
		LEVEL_TYPE_BONUS_SAUCER,
		LEVEL_TYPE_BONUS_INVADERS,
	};

	IEntityDomain* _domain;
	EntityEventListener _invadersWinListener;
	LevelType _levelType;
	float _spaceFrame;
	ff::ComPtr<IEntity> _groundEntity;
	ff::TypedResource<ff::ISprite> _groundSprite;
	ff::TypedResource<ff::ISpriteAnimation> _bgAnim;
	ff::TypedResource<ff::ISpriteFont> _font;
	ff::ComPtr<IProxyEntityEventListener> _listener;
};
