#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\entity\EntityListener.h"
#include "Resource\ResourceValue.h"
#include "services\PlayerService.h"

namespace ff
{
	class ISpriteAnimation;
}

class EntityFactoryService;
class GlobalPlayerService;
class PlayerComponent;
class IEntity;
struct PowerupEventArgs;
struct ScoreEventArgs;
struct StateEventArgs;

enum PlayerState : int
{
	// Alive
	PS_INIT,
	PS_MOVING,
	PS_SHOOTING,
	PS_BACK_TO_LIFE,

	// Not alive or dead
	PS_DYING,
	PS_REBUILDING,

	// Dead
	PS_DEAD,
};

enum PlayerHitSide
{
	PLAYER_HIT_NONE,
	PLAYER_HIT_LEFT,
	PLAYER_HIT_RIGHT,
	PLAYER_HIT_MIDDLE,

	PLAYER_HIT_COUNT
};

class __declspec(uuid("f55cff50-a1b3-4d53-aec1-af71fc35442a"))
	PlayerAdvanceRender
		: public ff::ComBase
		, public IAdvanceComponent
		, public I2dRenderComponent
		, public IEntityEventListener
		, public IComponentListener
		, public IPlayerService
{
public:
	DECLARE_HEADER(PlayerAdvanceRender);

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

	// IPlayerService
	virtual size_t GetPlayerCount() const override;
	virtual IEntity* GetPlayer(size_t index) const override;
	virtual bool IsPlayerAlive(size_t index) const override;
	virtual bool IsAnyPlayerAlive() const override;
	virtual bool AreAllPlayersDead() const override;
	virtual bool AreAllPlayersOutOfLives() const override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	struct PlayerInfo
	{
		PlayerInfo();
		PlayerInfo(IEntity* entity, PlayerComponent* pPlayer);

		IEntity* _entity;
		PlayerComponent* _component;
		float _wheelsFrame;
		float _bodyFrame;
		PlayerHitSide _hitSide;
	};

	bool IsPlayerAlive(PlayerState state, size_t index) const;
	bool IsPlayerKillable(PlayerState state, size_t index, bool bGettingPowerup = false) const;

	void AdvancePlayer(IEntityDomain *pDomain, PlayerInfo &info);
	void AdvancePlayerMovement(PlayerInfo &info);
	bool AdvanceBodyFrame(PlayerInfo &info, ff::ISpriteAnimation *pAnim);
	void ShootBullet(PlayerInfo &info, size_t nGun);

	// Event handlers
	void HandleCollision(IEntity *pPlayerEntity, IEntity *otherEntity);
	void OnEntityApplyForce(IEntity *entity, const ff::PointFloat &force);
	void OnAddScore(const ScoreEventArgs &eventArgs);
	void OnCollectPowerup(IEntity *entity, const PowerupEventArgs &eventArgs);
	void OnStateChanged(IEntity *entity, const StateEventArgs &eventArgs);

	ComponentListener<PlayerComponent> _listener;
	EntityEventListener _scoreListener;
	ff::Vector<PlayerInfo> _players;
	ff::ComPtr<GlobalPlayerService> _globalPlayers;
	ff::ComPtr<EntityFactoryService> _factoryService;
	ff::TypedResource<ff::ISpriteAnimation> _standingAnim;
	ff::TypedResource<ff::ISpriteAnimation> _movingAnim;
	ff::TypedResource<ff::ISpriteAnimation> _shootingAnim;
	ff::TypedResource<ff::ISpriteAnimation> _shieldAnim;
	ff::TypedResource<ff::ISpriteAnimation> _hitAnims[PLAYER_HIT_COUNT];
};
