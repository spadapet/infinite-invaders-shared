#pragma once

class IEntity;
class IEntityDomain;

class __declspec(uuid("593d074f-c424-482d-bcf9-9ec9110b0fdb")) __declspec(novtable)
	IEntityEventListener : public IUnknown
{
public:
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) = 0;
};

class __declspec(uuid("5371a1bf-df43-48d5-a31f-4047b3715d1e")) __declspec(novtable)
	IProxyEntityEventListener : public IEntityEventListener
{
public:
	// Must call SetOwner(nullptr) when the owner is destroyed
	virtual void SetOwner(IEntityEventListener *pOwner) = 0;
};

bool CreateProxyEntityEventListener(IEntityEventListener *pListener, IProxyEntityEventListener **ppProxy);

class EntityEventListener
{
public:
	EntityEventListener();
	~EntityEventListener();

	void Init(IEntityDomain *pDomain, ff::hash_t eventName, IEntityEventListener *pOwner);
	IEntityEventListener *GetListener();
	IEntityDomain* GetDomain();

private:
	IEntityDomain* _domain;
	ff::ComPtr<IProxyEntityEventListener> _listener;
	ff::hash_t _eventName;
};
