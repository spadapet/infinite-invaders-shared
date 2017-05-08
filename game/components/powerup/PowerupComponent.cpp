#include "pch.h"
#include "components\powerup\PowerupComponent.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PowerupComponent");
	module.RegisterClassT<PowerupComponent>(name);
});

BEGIN_INTERFACES(PowerupComponent)
END_INTERFACES()

PowerupComponent::PowerupComponent()
	: _type(POWERUP_TYPE_BONUS_POINTS)
	, _nPlayerIndex(ff::INVALID_SIZE)
{
}

PowerupComponent::~PowerupComponent()
{
}

PoweruentityType PowerupComponent::GetType() const
{
	return _type;
}

void PowerupComponent::SetType(PoweruentityType type)
{
	_type = type;
}

size_t PowerupComponent::GetPlayerIndex() const
{
	return _nPlayerIndex;
}

void PowerupComponent::SetPlayerIndex(size_t index)
{
	_nPlayerIndex = index;
}
