#include "pch.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "entities\Entity2dRenderComp.h"

Entity2dRenderComp::Entity2dRenderComp()
{
}

Entity2dRenderComp::Entity2dRenderComp(const Entity2dRenderComp &rhs)
	: EntityComponent<I2dRenderComponent>(rhs)
	, _priority(rhs._priority)
{
}

Entity2dRenderComp::Entity2dRenderComp(IEntity* entity, I2dRenderComponent* pComponent, bool bCachePriority)
	: EntityComponent<I2dRenderComponent>(entity, pComponent)
	, _priority(0)
{
	if (bCachePriority)
	{
		_priority = pComponent->Get2dRenderPriority();
	}
}

Entity2dRenderComp &Entity2dRenderComp::operator=(const Entity2dRenderComp &rhs)
{
	__super::operator=(rhs);
	_priority = rhs._priority;

	return *this;
}

bool Entity2dRenderComp::operator==(const Entity2dRenderComp &rhs) const
{
	return _priority == rhs._priority && __super::operator==(rhs);
}

bool Entity2dRenderComp::operator!=(const Entity2dRenderComp &rhs) const
{
	return _priority != rhs._priority || __super::operator!=(rhs);
}

bool Entity2dRenderComp::operator<(const Entity2dRenderComp &rhs) const
{
	int comp = _priority - rhs._priority;

	return !comp ? __super::operator<(rhs) : (comp < 0);
}
