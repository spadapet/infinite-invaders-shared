#pragma once

#include "coreEntity\entity\EntityComponent.h"

class IAdvanceComponent;
class IEntity;

struct EntityAdvanceComp : public EntityComponent<IAdvanceComponent>
{
	int _priority;

	EntityAdvanceComp();
	EntityAdvanceComp(const EntityAdvanceComp &rhs);
	EntityAdvanceComp(IEntity* entity, IAdvanceComponent* pComponent, bool bCachePriority);

	EntityAdvanceComp &operator=(const EntityAdvanceComp &rhs);

	bool operator==(const EntityAdvanceComp &rhs) const;
	bool operator!=(const EntityAdvanceComp &rhs) const;
	bool operator<(const EntityAdvanceComp &rhs) const;
};

MAKE_POD(EntityAdvanceComp);
