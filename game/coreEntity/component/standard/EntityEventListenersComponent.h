#pragma once

class IEntity;
class IEntityEventListener;
class IProxyEntityEventListener;

class __declspec(uuid("72018e24-da35-4304-9d8f-8f8fd4fa44dc")) __declspec(novtable)
	IEntityEventListeners : public IUnknown
{
public:
	static bool AddListener(IEntity *entity, IEntityEventListener *pListener);
	static bool AddProxyListener(IEntity *entity, IEntityEventListener *pListener, IProxyEntityEventListener **ppProxy);
	static bool RemoveListener(IEntity *entity, IEntityEventListener *pListener);
	static bool HasListener(IEntity *entity, IEntityEventListener *pListener);
};

