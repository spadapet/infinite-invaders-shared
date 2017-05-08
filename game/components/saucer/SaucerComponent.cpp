#include "pch.h"
#include "components\saucer\SaucerComponent.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"SaucerComponent");
	module.RegisterClassT<SaucerComponent>(name);
});

BEGIN_INTERFACES(SaucerComponent)
END_INTERFACES()

SaucerComponent::SaucerComponent()
	: _type(SAUCER_TYPE_NONE)
	, _hitCount(0)
{
}

SaucerComponent::~SaucerComponent()
{
}

SaucerEntityType SaucerComponent::GetType() const
{
	return _type;
}

void SaucerComponent::SetType(SaucerEntityType type)
{
	_type = type;
}

bool SaucerComponent::IsBonus() const
{
	return _type == SAUCER_TYPE_BONUS_LEVEL || _type == SAUCER_TYPE_BONUS_LEVEL_FAST;
}

size_t SaucerComponent::GetHitCount() const
{
	return _hitCount;
}

void SaucerComponent::AddHitCount()
{
	_hitCount++;
}
