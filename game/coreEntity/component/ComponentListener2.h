#pragma once

#include "coreEntity\component\ComponentListener.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\domain\EntityDomain.h"

class IEntity;

template<class T>
class ComponentListener
{
public:
	ComponentListener();
	~ComponentListener();

	void Init(IEntityDomain *pDomain, IComponentListener *pOwner);
	IComponentListener *GetListener();

private:
	IEntityDomain* _domain;
	ff::ComPtr<IProxyComponentListener> _listener;
};

template<class T>
ComponentListener<T>::ComponentListener()
	: _domain(nullptr)
	, _listener(nullptr){
}

template<class T>
ComponentListener<T>::~ComponentListener()
{
	if (_listener)
	{
		_listener->SetOwner(nullptr);
		_domain->GetComponentManager()->RemoveListener<T>(_listener);
	}
}

template<class T>
void ComponentListener<T>::Init(IEntityDomain *pDomain, IComponentListener *pOwner)
{
	assertRet(!_listener && pOwner && pDomain);
	_domain = pDomain;
	_domain->GetComponentManager()->AddProxyListener<T>(pOwner, &_listener);
}

template<class T>
IComponentListener *ComponentListener<T>::GetListener()
{
	return _listener;
}
