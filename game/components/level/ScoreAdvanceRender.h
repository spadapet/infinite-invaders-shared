#pragma once

#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class ISprite;
	class ISpriteFont;
}

class GlobalPlayerService;
class IEntity;
class IPlayerService;

class __declspec(uuid("abc05672-93f0-49d0-8a81-03fa7d45ad69"))
	ScoreAdvanceRender
		: public ff::ComBase
		, public IAdvanceComponent
		, public I2dRenderComponent
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(ScoreAdvanceRender);

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
	void UpdateLivesCount();

	EntityEventListener _playStartListener;
	ff::TypedResource<ff::ISpriteFont> _font;
	ff::ComPtr<GlobalPlayerService> _globalPlayers;
	ff::ComPtr<IPlayerService> _players;
	ff::ComPtr<ff::ISprite> _playerBody;
	ff::ComPtr<ff::ISprite> _playerWheels;
	ff::ComPtr<ff::ISprite> _playerTurret;
	size_t _counter;
	size_t _highScore;
	size_t _lives[2];
	size_t _coopLives;
};
