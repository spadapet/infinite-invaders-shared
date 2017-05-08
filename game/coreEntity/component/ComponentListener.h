#pragma once

class IEntity;

class __declspec(uuid("90b10ff0-b092-41bf-b732-cdaad9ec9911")) __declspec(novtable)
	IComponentListener : public IUnknown
{
public:
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) = 0;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) = 0;
};

class __declspec(uuid("ffb0e0c4-5554-40bb-8467-2bd8de896bb2")) __declspec(novtable)
	IProxyComponentListener : public IComponentListener
{
public:
	// Must call SetOwner(nullptr) when the owner is destroyed
	virtual void SetOwner(IComponentListener *pOwner) = 0;
};

bool CreateProxyComponentListener(IComponentListener *pListener, IProxyComponentListener **ppProxy);
