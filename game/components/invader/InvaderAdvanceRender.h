#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "components\invader\InvaderComponent.h"
#include "entities\EntityPosComp.h"
#include "Resource\ResourceValue.h"
#include "services\GameBeatService.h"
#include "services\InvaderService.h"

namespace ff
{
	class ISprite;
	class ISpriteAnimation;
}

class IEntity;
class IPlayerService;

enum InvaderAdvanceState
{
	INVADER_STATE_INIT,
	INVADER_STATE_MOVE,
	INVADER_STATE_MOVE_DOWN,
	INVADER_STATE_PRE_DESTROY_BASE,
	INVADER_STATE_DESTROY_BASE,
	INVADER_STATE_DANCING,
};

class __declspec(uuid("9e0ade72-b32d-4673-bdcf-c252f18255fa"))
	InvaderAdvanceRender
		: public ff::ComBase
		, public IAdvanceComponent
		, public I2dRenderComponent
		, public IEntityEventListener
		, public IComponentListener
		, public IGameBeatService
		, public IInvaderService
{
public:
	DECLARE_HEADER(InvaderAdvanceRender);

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

	// IGameBeatService
	virtual size_t GetFrameCount() const override;
	virtual size_t GetFramesPerBeat() const override;
	virtual size_t GetBeatsPerMeasure() const override;
	virtual size_t GetBeatsPerShot() const override;

	virtual size_t GetMeasureCount() const override;
	virtual size_t GetBeatCount() const override;

	virtual double GetMeasureReal() const override;
	virtual double GetBeatReal() const override;

	// IInvaderService
	virtual size_t GetInvaderCount() const override;
	virtual IEntity* GetInvader(size_t index) const override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	typedef EntityPosComp<InvaderComponent> EntityInvader;

	void RenderFadedInvaders(bool bFade, InvaderMoveType moveType);
	void RenderInvaders(ff::I2dRenderer *render, InvaderMoveType moveType);

	float GetInvaderFrame() const;

	void SetState(InvaderAdvanceState state);
	void OnStateChanged(InvaderAdvanceState oldState, InvaderAdvanceState newState);

	void HandleCollision(IEntity *pInvaderEntity, IEntity *otherEntity);
	void MoveInvaders(IPlayerService *pPlayers);
	void MoveInvadersSideways(IPlayerService *pPlayers);
	bool MoveInvadersDown();
	bool MoveInvadersAlongPath();
	bool MoveDestroyBaseInvader();
	void TurnAroundInvaders();
	void ShootBullet();
	IEntity* GetDestroyBaseInvader();

	static const size_t BEATS_PER_MEASURE = 4;

	// Listeners
	ComponentListener<InvaderComponent> _invaderListener;
	EntityEventListener _turnAroundListener;

	// Entities
	IEntityDomain* _domain;
	ff::Vector<EntityInvader> _entities;
	IEntity* _destroyBaseInvader;

	// Time counters
	size_t _frames;
	size_t _subBeat;
	size_t _beats;
	size_t _shots;

	// State
	InvaderAdvanceState _state;
	size_t _stateCounter;
	size_t _stateSubCounter;
	Difficulty _difficulty;
	size_t _invaderLevel;
	bool _invaderBonus;

	// Caches
	mutable size_t _cachedSpeed;
	mutable size_t _cachedBeatsPerShot;
	mutable float _fadeSpeed;

	// Graphics
	ff::TypedResource<ff::ISpriteAnimation> _invaderAnims[INVADER_TYPE_COUNT];
	ff::ComPtr<ff::IRenderTarget> _fadeTarget;
	ff::ComPtr<ff::ISprite> _fadeSprite;
};
