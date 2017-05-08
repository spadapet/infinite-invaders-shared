#pragma once

#include "components\bullet\BulletComponent.h"
#include "components\invader\InvaderComponent.h"
#include "components\powerup\PowerupComponent.h"
#include "components\saucer\SaucerComponent.h"
#include "components\shield\ShieldComponent.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class ISprite;
	class ISpriteAnimation;
	class ISpriteFont;
	class ISpriteList;
}

class IEntity;
enum PlayerHitSide;

class __declspec(uuid("2181c42d-bcf4-43a5-80d7-1a2742bb187d"))
	EntityFactoryService : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(EntityFactoryService);

	bool CreateBulletEntity(BulletEntityType type, ff::PointFloat pos, ff::PointFloat velocity, IEntity **pentity);
	bool CreateBulletSmoke(IEntity *pBulletEntity);
	bool CreateBulletExplosion(IEntity *pBulletEntity);

	bool CreateInvaderEntity(InvaderEntityType type, ff::PointInt cell, ff::PointFloat pos, ff::PointFloat vel, IEntity **pentity);
	bool CreateInvaderExplosion(IEntity *pInvaderEntity);

	bool CreatePlayerEntity(size_t index, IEntity **pentity);
	bool CreatePlayerDust(IEntity *pPlayerEntity);
	bool CreatePlayerExplosion(IEntity *pPlayerEntity, IEntity *otherEntity);
	bool CreatePlayerDyingSmoke(IEntity *pPlayerEntity, PlayerHitSide hitSide);
	bool CreatePlayerRebuild(IEntity *pPlayerEntity);

	bool CreateShieldEntity(ShieldEntityType type, ff::PointFloat pos, IEntity **pentity);
	bool CreateShieldExplosion(ShieldEntityType type, IEntity *pBulletEntity);

	bool CreateSaucerEntity(SaucerEntityType type, ff::PointFloat pos, ff::PointFloat velocity, IEntity **pentity);
	bool CreateSaucerExplosion(IEntity *pSaucerEntity, IEntity *pBulletEntity);

	bool CreatePoints(IEntity *entity, size_t nPoints, float timeScale = 1, ff::PointFloat offset = ff::PointFloat(0, 0), const DirectX::XMFLOAT4 *pColor = nullptr);
	bool CreatePowerup(PoweruentityType type, size_t nPlayerIndex, ff::PointFloat pos, ff::PointFloat velocity, IEntity **pentity);
	bool CreatePowerupCollect(IEntity *pPoweruentity, IEntity **pentity);
	bool CreateFreeLifeIndicator(IEntityDomain *pDomain, ff::PointFloat pos);

	bool CreateLevelEntity(Difficulty difficulty, size_t nLevel, IEntity **pentity);

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	bool InitSprites();
	bool AddLevelBonusSaucer(size_t nLevel);
	bool AddLevelBonusInvaders(size_t nLevel);
	bool AddLevelInvaders(size_t nLevel);
	bool AddLevelShields(size_t nLevel);

	IEntityDomain *_domain;

	ff::TypedResource<ff::ISpriteAnimation> _bulletAnims[BULLET_TYPE_COUNT];
	ff::TypedResource<ff::ISpriteAnimation> _bulletHitAnims[BULLET_TYPE_COUNT];
	ff::TypedResource<ff::ISpriteAnimation> _powerupAnims[POWERUP_TYPE_COUNT];
	ff::TypedResource<ff::ISpriteAnimation> _saucerAnims[SAUCER_TYPE_COUNT];
	ff::TypedResource<ff::ISpriteAnimation> _pSaucerLightsAnim;
	ff::TypedResource<ff::ISpriteAnimation> _playerOrbAnim;
	ff::TypedResource<ff::ISpriteAnimation> _shieldHitAnims[BULLET_TYPE_COUNT];
	ff::TypedResource<ff::ISpriteAnimation> _smokeAnims[2];
	ff::TypedResource<ff::ISpriteAnimation> _dustAnims[2];
	ff::TypedResource<ff::ISpriteAnimation> _invaderHitAnims[INVADER_TYPE_COUNT];
	ff::ComPtr<ff::ISprite> _shields[SHIELD_TYPE_COUNT][5];
	ff::TypedResource<ff::ISpriteList> _pShieldSprites;
	ff::TypedResource<ff::ISpriteFont> _font;
	ff::ComPtr<ff::ISprite> _playerBody;
	ff::ComPtr<ff::ISprite> _playerWheels;
	ff::ComPtr<ff::ISprite> _playerTurret;
};
