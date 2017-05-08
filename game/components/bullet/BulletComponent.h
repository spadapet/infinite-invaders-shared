#pragma once

class IEntity;
enum Difficulty;

enum BulletEntityType
{
	// Player
	BULLET_TYPE_PLAYER_0,
	BULLET_TYPE_PLAYER_1,
	BULLET_TYPE_HOMING_PLAYER_0,
	BULLET_TYPE_HOMING_PLAYER_1,
	BULLET_TYPE_FAST_PLAYER_0,
	BULLET_TYPE_FAST_PLAYER_1,
	BULLET_TYPE_SPREAD_PLAYER_0,
	BULLET_TYPE_SPREAD_PLAYER_1,
	BULLET_TYPE_PUSH_PLAYER_0,
	BULLET_TYPE_PUSH_PLAYER_1,

	// Invader
	BULLET_TYPE_INVADER_LARGE,
	BULLET_TYPE_INVADER_SMALL,
	BULLET_TYPE_INVADER_LOSE_GAME,

	BULLET_TYPE_COUNT
};

class __declspec(uuid("9d8c639c-5f71-413e-915b-b5216175f95e"))
	BulletComponent : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(BulletComponent);

	BulletEntityType GetType() const;
	void SetType(BulletEntityType type);

	bool IsInvader() const;
	bool IsInvaderHoming(Difficulty diff) const;
	bool IsPlayer() const;
	bool IsPlayerHoming() const;
	size_t GetPlayer() const;

	bool CanDeflect() const;
	bool CanHitInvader(const IEntity *pBulletEntity) const;

private:
	BulletEntityType _type;
};
