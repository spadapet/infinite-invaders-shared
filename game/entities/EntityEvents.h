#pragma once

class IEntity;
class IEntityManager;

extern const ff::hash_t ENTITY_EVENT_APPLY_FORCE; // param: ff::PointFloat*

// Bullet events
extern const ff::hash_t ENTITY_EVENT_BULLET_SHOOT; // param: BulletEventArgs*
extern const ff::hash_t ENTITY_EVENT_BULLET_HIT; // param: BulletEventArgs*

// Game events
extern const ff::hash_t ENTITY_EVENT_BORN; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_DIED; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_POSITION_CHANGED; // param: PositionChangedEventArgs*
extern const ff::hash_t ENTITY_EVENT_COLLISION; // param: CollisionEventArgs*
extern const ff::hash_t ENTITY_EVENT_ADD_SCORE; // param: ScoreEventArgs*
extern const ff::hash_t ENTITY_EVENT_GOT_FREE_LIFE; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_STATE_CHANGED; // param: StateEventArgs*
extern const ff::hash_t ENTITY_EVENT_COLLECT_POWERUP; // param: PowerupEventArgs*

extern const ff::hash_t ENTITY_EVENT_LEVEL_START; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_LEVEL_COMPLETE; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_LEVEL_BASE_HIT; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_PLAYING_START; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_PLAYING_STOP; // param: nullptr

// Invader events
extern const ff::hash_t ENTITY_EVENT_INVADERS_WIN; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_INVADERS_DANCING; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_INVADERS_TURN_AROUND; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_INVADERS_MOVED_SIDEWAYS; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_INVADERS_MOVED_DOWN; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_INVADER_MOVED_SIDEWAYS; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_INVADER_MOVED_DOWN; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_INVADER_HIT; // param: CollisionEventArgs*

// Animation events
extern const ff::hash_t ENTITY_EVENT_ANIM_LOOPED; // param: AnimLoopedEventArgs*

class IPositionComponent;
class SpriteAnimationAdvance;
struct PositionInfo;

struct AnimLoopedEventArgs
{
	SpriteAnimationAdvance* _pAdvance;
	int _loopsLeft;
};

struct BulletEventArgs
{
	IEntity *_pSource;
	IEntity *_pBullet;
};

struct CollisionEventArgs
{
	IEntity *_pOther;
};

struct PlayerEventArgs
{
	size_t _nPlayer;
};

struct PositionChangedEventArgs
{
	IPositionComponent* _component;
	const PositionInfo* _pOldInfo;
	const PositionInfo* _pNewInfo;
};

struct PowerupEventArgs
{
	IEntity *_pPowerup;
};


struct ScoreEventArgs
{
	size_t _nPlayer;
	size_t _nAddScore;
};

struct StateEventArgs
{
	size_t _nOldCounter;
	int _nOldState;
	int _nNewState;
};
