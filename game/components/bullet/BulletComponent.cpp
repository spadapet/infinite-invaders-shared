#include "pch.h"
#include "Globals.h"
#include "components\bullet\BulletComponent.h"
#include "components\core\PositionComponent.h"
#include "coreEntity\entity\Entity.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"BulletComponent");
	module.RegisterClassT<BulletComponent>(name);
});

BEGIN_INTERFACES(BulletComponent)
END_INTERFACES()

BulletComponent::BulletComponent()
	: _type(BULLET_TYPE_PLAYER_0)
{
}

BulletComponent::~BulletComponent()
{
}

BulletEntityType BulletComponent::GetType() const
{
	return _type;
}

void BulletComponent::SetType(BulletEntityType type)
{
	_type = type;
}

bool BulletComponent::CanDeflect() const
{
	return _type != BULLET_TYPE_INVADER_LOSE_GAME && IsInvader();
}

bool BulletComponent::IsInvader() const
{
	switch (_type)
	{
	default:
		return false;

	case BULLET_TYPE_INVADER_SMALL:
	case BULLET_TYPE_INVADER_LARGE:
	case BULLET_TYPE_INVADER_LOSE_GAME:
		return true;
	}
}

bool BulletComponent::IsInvaderHoming(Difficulty diff) const
{
	switch (_type)
	{
	default:
	case BULLET_TYPE_INVADER_SMALL:
		return false;

	case BULLET_TYPE_INVADER_LARGE:
		return !Globals::IsEasyDifficulty(diff);

	case BULLET_TYPE_INVADER_LOSE_GAME:
		return true;
	}
}

bool BulletComponent::CanHitInvader(const IEntity *pBulletEntity) const
{
	if (IsPlayer())
	{
		return true;
	}

	IPositionComponent *pPos = pBulletEntity ? pBulletEntity->GetComponent<IPositionComponent>() : nullptr;

	if (pPos && pPos->GetVelocity().y < 0)
	{
		return true;
	}

	return false;
}

bool BulletComponent::IsPlayer() const
{
	switch (_type)
	{
	default:
		return false;

	case BULLET_TYPE_PLAYER_0:
	case BULLET_TYPE_PLAYER_1:
	case BULLET_TYPE_HOMING_PLAYER_0:
	case BULLET_TYPE_HOMING_PLAYER_1:
	case BULLET_TYPE_FAST_PLAYER_0:
	case BULLET_TYPE_FAST_PLAYER_1:
	case BULLET_TYPE_SPREAD_PLAYER_0:
	case BULLET_TYPE_SPREAD_PLAYER_1:
	case BULLET_TYPE_PUSH_PLAYER_0:
	case BULLET_TYPE_PUSH_PLAYER_1:
		return true;
	}
}

bool BulletComponent::IsPlayerHoming() const
{
	return _type == BULLET_TYPE_HOMING_PLAYER_0 || _type == BULLET_TYPE_HOMING_PLAYER_1;
}

size_t BulletComponent::GetPlayer() const
{
	switch (_type)
	{
	default:
		assert(false);
		__fallthrough;

	case BULLET_TYPE_PLAYER_0:
	case BULLET_TYPE_HOMING_PLAYER_0:
	case BULLET_TYPE_FAST_PLAYER_0:
	case BULLET_TYPE_SPREAD_PLAYER_0:
	case BULLET_TYPE_PUSH_PLAYER_0:
		return 0;

	case BULLET_TYPE_PLAYER_1:
	case BULLET_TYPE_HOMING_PLAYER_1:
	case BULLET_TYPE_FAST_PLAYER_1:
	case BULLET_TYPE_SPREAD_PLAYER_1:
	case BULLET_TYPE_PUSH_PLAYER_1:
		return 1;
	}
}
