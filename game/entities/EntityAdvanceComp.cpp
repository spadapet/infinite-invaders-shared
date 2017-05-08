#include "pch.h"
#include "coreEntity\component\standard\AdvanceComponent.h"
#include "entities\EntityAdvanceComp.h"

EntityAdvanceComp::EntityAdvanceComp()
{
}

EntityAdvanceComp::EntityAdvanceComp(const EntityAdvanceComp &rhs)
	: EntityComponent<IAdvanceComponent>(rhs)
	, _priority(rhs._priority)
{
}

EntityAdvanceComp::EntityAdvanceComp(IEntity* entity, IAdvanceComponent* pComponent, bool bCachePriority)
	: EntityComponent<IAdvanceComponent>(entity, pComponent)
	, _priority(0)
{
	if (bCachePriority)
	{
		_priority = pComponent->GetAdvancePriority();
	}
}

EntityAdvanceComp &EntityAdvanceComp::operator=(const EntityAdvanceComp &rhs)
{
	__super::operator=(rhs);
	_priority = rhs._priority;

	return *this;
}

bool EntityAdvanceComp::operator==(const EntityAdvanceComp &rhs) const
{
	return _priority == rhs._priority && __super::operator==(rhs);
}

bool EntityAdvanceComp::operator!=(const EntityAdvanceComp &rhs) const
{
	return _priority != rhs._priority || __super::operator!=(rhs);
}

bool EntityAdvanceComp::operator<(const EntityAdvanceComp &rhs) const
{
	int comp = _priority - rhs._priority;

	return !comp ? __super::operator<(rhs) : (comp < 0);
}
