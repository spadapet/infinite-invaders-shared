#include "pch.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"

static ff::StaticString STR_ENTITY_EVENT_APPLY_FORCE(L"Entity.ApplyForce");
static ff::StaticString STR_ENTITY_EVENT_BULLET_SHOOT(L"Entity.BulletShoot");
static ff::StaticString STR_ENTITY_EVENT_BULLET_HIT(L"Entity.BulletHit");
static ff::StaticString STR_ENTITY_EVENT_BORN(L"Entity.Born");
static ff::StaticString STR_ENTITY_EVENT_DIED(L"Entity.Died");
static ff::StaticString STR_ENTITY_EVENT_POSITION_CHANGED(L"Entity.PositionChanged");
static ff::StaticString STR_ENTITY_EVENT_COLLISION(L"Entity.Collision");
static ff::StaticString STR_ENTITY_EVENT_ADD_SCORE(L"Entity.AddScore");
static ff::StaticString STR_ENTITY_EVENT_GOT_FREE_LIFE(L"Entity.GotFreeLife");
static ff::StaticString STR_ENTITY_EVENT_STATE_CHANGED(L"Entity.StateChanged");
static ff::StaticString STR_ENTITY_EVENT_COLLECT_POWERUP(L"Entity.CollectPowerup");
static ff::StaticString STR_ENTITY_EVENT_LEVEL_START(L"Entity.LevelStart");
static ff::StaticString STR_ENTITY_EVENT_LEVEL_COMPLETE(L"Entity.LevelComplete");
static ff::StaticString STR_ENTITY_EVENT_LEVEL_BASE_HIT(L"Entity.LevelBaseHit");
static ff::StaticString STR_ENTITY_EVENT_PLAYING_START(L"Entity.PlayStart");
static ff::StaticString STR_ENTITY_EVENT_PLAYING_STOP(L"Entity.PlayStop");
static ff::StaticString STR_ENTITY_EVENT_INVADERS_WIN(L"Entity.InvadersWin");
static ff::StaticString STR_ENTITY_EVENT_INVADERS_DANCING(L"Entity.InvadersDancing");
static ff::StaticString STR_ENTITY_EVENT_INVADERS_TURN_AROUND(L"Entity.InvadersTurnAround");
static ff::StaticString STR_ENTITY_EVENT_INVADERS_MOVED_SIDEWAYS(L"Entity.InvadersMovedSideways");
static ff::StaticString STR_ENTITY_EVENT_INVADERS_MOVED_DOWN(L"Entity.InvadersMovedDown");
static ff::StaticString STR_ENTITY_EVENT_INVADER_MOVED_SIDEWAYS(L"Entity.InvaderMovedSideways");
static ff::StaticString STR_ENTITY_EVENT_INVADER_MOVED_DOWN(L"Entity.InvaderMovedDown");
static ff::StaticString STR_ENTITY_EVENT_INVADER_HIT(L"Entity.InvaderHit");
static ff::StaticString STR_ENTITY_EVENT_ANIM_LOOPED(L"Entity.AnimLooped");

const ff::hash_t ENTITY_EVENT_APPLY_FORCE = ff::HashFunc(STR_ENTITY_EVENT_APPLY_FORCE);
const ff::hash_t ENTITY_EVENT_BULLET_SHOOT = ff::HashFunc(STR_ENTITY_EVENT_BULLET_SHOOT);
const ff::hash_t ENTITY_EVENT_BULLET_HIT = ff::HashFunc(STR_ENTITY_EVENT_BULLET_HIT);
const ff::hash_t ENTITY_EVENT_BORN = ff::HashFunc(STR_ENTITY_EVENT_BORN);
const ff::hash_t ENTITY_EVENT_DIED = ff::HashFunc(STR_ENTITY_EVENT_DIED);
const ff::hash_t ENTITY_EVENT_POSITION_CHANGED = ff::HashFunc(STR_ENTITY_EVENT_POSITION_CHANGED);
const ff::hash_t ENTITY_EVENT_COLLISION = ff::HashFunc(STR_ENTITY_EVENT_COLLISION);
const ff::hash_t ENTITY_EVENT_ADD_SCORE = ff::HashFunc(STR_ENTITY_EVENT_ADD_SCORE);
const ff::hash_t ENTITY_EVENT_GOT_FREE_LIFE = ff::HashFunc(STR_ENTITY_EVENT_GOT_FREE_LIFE);
const ff::hash_t ENTITY_EVENT_STATE_CHANGED = ff::HashFunc(STR_ENTITY_EVENT_STATE_CHANGED);
const ff::hash_t ENTITY_EVENT_COLLECT_POWERUP = ff::HashFunc(STR_ENTITY_EVENT_COLLECT_POWERUP);
const ff::hash_t ENTITY_EVENT_LEVEL_START = ff::HashFunc(STR_ENTITY_EVENT_LEVEL_START);
const ff::hash_t ENTITY_EVENT_LEVEL_COMPLETE = ff::HashFunc(STR_ENTITY_EVENT_LEVEL_COMPLETE);
const ff::hash_t ENTITY_EVENT_LEVEL_BASE_HIT = ff::HashFunc(STR_ENTITY_EVENT_LEVEL_BASE_HIT);
const ff::hash_t ENTITY_EVENT_PLAYING_START = ff::HashFunc(STR_ENTITY_EVENT_PLAYING_START);
const ff::hash_t ENTITY_EVENT_PLAYING_STOP = ff::HashFunc(STR_ENTITY_EVENT_PLAYING_STOP);
const ff::hash_t ENTITY_EVENT_INVADERS_WIN = ff::HashFunc(STR_ENTITY_EVENT_INVADERS_WIN);
const ff::hash_t ENTITY_EVENT_INVADERS_DANCING = ff::HashFunc(STR_ENTITY_EVENT_INVADERS_DANCING);
const ff::hash_t ENTITY_EVENT_INVADERS_TURN_AROUND = ff::HashFunc(STR_ENTITY_EVENT_INVADERS_TURN_AROUND);
const ff::hash_t ENTITY_EVENT_INVADERS_MOVED_SIDEWAYS = ff::HashFunc(STR_ENTITY_EVENT_INVADERS_MOVED_SIDEWAYS);
const ff::hash_t ENTITY_EVENT_INVADERS_MOVED_DOWN = ff::HashFunc(STR_ENTITY_EVENT_INVADERS_MOVED_DOWN);
const ff::hash_t ENTITY_EVENT_INVADER_MOVED_SIDEWAYS = ff::HashFunc(STR_ENTITY_EVENT_INVADER_MOVED_SIDEWAYS);
const ff::hash_t ENTITY_EVENT_INVADER_MOVED_DOWN = ff::HashFunc(STR_ENTITY_EVENT_INVADER_MOVED_DOWN);
const ff::hash_t ENTITY_EVENT_INVADER_HIT = ff::HashFunc(STR_ENTITY_EVENT_INVADER_HIT);
const ff::hash_t ENTITY_EVENT_ANIM_LOOPED = ff::HashFunc(STR_ENTITY_EVENT_ANIM_LOOPED);

static ff::StaticString STR_ENTITY_EVENT_NULL(L"CoreEntity.Null");
static ff::StaticString STR_ENTITY_EVENT_DESTROY(L"CoreEntity.Destroy");
static ff::StaticString STR_ENTITY_EVENT_ADD_COMPONENTS(L"CoreEntity.AddComponents");
static ff::StaticString STR_ENTITY_EVENT_REMOVE_COMPONENTS(L"CoreEntity.RemoveComponents");
static ff::StaticString STR_ENTITY_EVENT_ADD_COMPONENT(L"CoreEntity.AddComponent");
static ff::StaticString STR_ENTITY_EVENT_REMOVE_COMPONENT(L"CoreEntity.RemoveComponents");

const ff::hash_t ENTITY_EVENT_NULL = ff::HashFunc(STR_ENTITY_EVENT_NULL);
const ff::hash_t ENTITY_EVENT_DESTROY = ff::HashFunc(STR_ENTITY_EVENT_DESTROY);
const ff::hash_t ENTITY_EVENT_ADD_COMPONENTS = ff::HashFunc(STR_ENTITY_EVENT_ADD_COMPONENTS);
const ff::hash_t ENTITY_EVENT_REMOVE_COMPONENTS = ff::HashFunc(STR_ENTITY_EVENT_REMOVE_COMPONENTS);
const ff::hash_t ENTITY_EVENT_ADD_COMPONENT = ff::HashFunc(STR_ENTITY_EVENT_ADD_COMPONENT);
const ff::hash_t ENTITY_EVENT_REMOVE_COMPONENT = ff::HashFunc(STR_ENTITY_EVENT_REMOVE_COMPONENT);
