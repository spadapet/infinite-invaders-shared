#pragma once

#include "coreEntity\entity\EntityComponent.h"

class I2dRenderComponent;
class IEntity;

struct Entity2dRenderComp : public EntityComponent<I2dRenderComponent>
{
	int _priority;

	Entity2dRenderComp();
	Entity2dRenderComp(const Entity2dRenderComp &rhs);
	Entity2dRenderComp(IEntity* entity, I2dRenderComponent* pComponent, bool bCachePriority);

	Entity2dRenderComp &operator=(const Entity2dRenderComp &rhs);

	bool operator==(const Entity2dRenderComp &rhs) const;
	bool operator!=(const Entity2dRenderComp &rhs) const;
	bool operator<(const Entity2dRenderComp &rhs) const;
};

MAKE_POD(Entity2dRenderComp);
