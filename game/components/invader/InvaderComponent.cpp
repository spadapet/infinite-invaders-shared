#include "pch.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteAnimationRender.h"
#include "components\invader\InvaderComponent.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"InvaderComponent");
	module.RegisterClassT<InvaderComponent>(name);
});

BEGIN_INTERFACES(InvaderComponent)
END_INTERFACES()

InvaderComponent::InvaderComponent()
	: _type(INVADER_TYPE_DEFAULT)
	, _moveType(INVADER_MOVE_TOGETHER)
	, _repeat(0)
	, _cell(0, 0)
{
}

InvaderComponent::~InvaderComponent()
{
}

InvaderEntityType InvaderComponent::GetType() const
{
	return _type;
}

void InvaderComponent::SetType(InvaderEntityType type)
{
	_type = type;
}

size_t InvaderComponent::GetRepeat() const
{
	return _repeat;
}

void InvaderComponent::SetRepeat(size_t nRepeat)
{
	_repeat = nRepeat;
}

ff::PointInt InvaderComponent::GetCell() const
{
	return _cell;
}

void InvaderComponent::SetCell(ff::PointInt cell)
{
	_cell = cell;
}

InvaderMoveType InvaderComponent::GetMoveType() const
{
	return _moveType;
}

void InvaderComponent::SetMoveType(InvaderMoveType type)
{
	_moveType = type;
}

bool InvaderComponent::CanShoot() const
{
	switch(_type)
	{
	case INVADER_TYPE_BONUS_0:
	case INVADER_TYPE_BONUS_1:
	case INVADER_TYPE_BONUS_2:
		return false;
	}

	return true;
}

bool InvaderComponent::HasCustomHitBox() const
{
	switch (_type)
	{
	case INVADER_TYPE_4:
	case INVADER_TYPE_6:
		return true;

	default:
		return false;
	}
}
