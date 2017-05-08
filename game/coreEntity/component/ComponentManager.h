#pragma once

#include "coreEntity\component\ComponentListener.h"

class IEntity;
class IEntityDomain;

class __declspec(uuid("6960fcd0-92cd-4949-8a02-0d80ce9db138")) __declspec(novtable)
	IComponentManager : public IComponentListener
{
public:
	template<class T> bool CreateComponent(IEntity *entity, T** ppComp);
	template<class T, class I> bool CreateComponent2(IEntity *entity, I** ppComp);
	virtual bool CreateComponent(IEntity *entity, REFGUID compid, IUnknown **ppComp) = 0;

	template<class T> bool AddListener(IComponentListener *pListener);
	template<class T> bool AddProxyListener(IComponentListener *pListener, IProxyComponentListener **ppProxy);
	template<class T> bool RemoveListener(IComponentListener *pListener);

protected:
	virtual bool CreateComponent(REFGUID compid, REFGUID iid, IEntity *entity, void** ppComp) = 0;
	virtual bool AddListener(REFGUID compid, IComponentListener *pListener) = 0;
	virtual bool AddProxyListener(REFGUID compid, IComponentListener *pListener, IProxyComponentListener **ppProxy) = 0;
	virtual bool RemoveListener(REFGUID compid, IComponentListener *pListener) = 0;
};

bool CreateComponentManager(IEntityDomain *pDomain, IComponentManager **ppMan);

template<class T>
bool IComponentManager::CreateComponent(IEntity *entity, T** ppComp)
{
	return CreateComponent(__uuidof(T), __uuidof(T), entity, (void**)ppComp);
}

template<class T, class I>
bool IComponentManager::CreateComponent2(IEntity *entity, I** ppComp)
{
	return CreateComponent(__uuidof(T), __uuidof(I), entity, (void**)ppComp);
}

template<class T>
bool IComponentManager::AddListener(IComponentListener *pListener)
{
	return AddListener(__uuidof(T), pListener);
}

template<class T>
bool IComponentManager::AddProxyListener(IComponentListener *pListener, IProxyComponentListener **ppProxy)
{
	return AddProxyListener(__uuidof(T), pListener, ppProxy);
}

template<class T>
bool IComponentManager::RemoveListener(IComponentListener *pListener)
{
	return RemoveListener(__uuidof(T), pListener);
}

