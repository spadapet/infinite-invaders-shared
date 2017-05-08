#pragma once

#include "coreEntity\entity\EntityListener.h"

class IEntity;

class __declspec(uuid("f1d62638-dc66-46e1-93a9-bb9316b01362")) __declspec(novtable)
	IEntityManager : public IEntityEventListener
{
public:
	virtual bool CreateEntity(IEntity** pentity) = 0;
	virtual void TriggerEvent(ff::hash_t eventName, void *eventArgs = nullptr) = 0;

	// Listen for a specific event on any entity
	virtual bool AddListener(ff::hash_t eventName, IEntityEventListener *pListener) = 0;
	virtual bool AddProxyListener(ff::hash_t eventName, IEntityEventListener *pListener, IProxyEntityEventListener **ppProxy) = 0;
	virtual bool RemoveListener(ff::hash_t eventName, IEntityEventListener *pListener) = 0;
};

bool CreateEntityManager(IEntityDomain *pDomain, IEntityManager **ppMan);
