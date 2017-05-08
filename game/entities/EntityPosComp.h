#pragma once

#include "components\core\PositionComponent.h"
#include "coreEntity\entity\EntityComponent.h"

class IEntity;

template<typename T>
struct EntityPosComp : public EntityComponent<T>
{
	IPositionComponent* _pPosComp;
	const PositionInfo* _pPosInfo;

	EntityPosComp();
	EntityPosComp(const EntityPosComp<T> &rhs);
	EntityPosComp(IEntity* entity, T* pComponent, bool bGetPos);

	EntityPosComp<T> &operator=(const EntityPosComp<T> &rhs);

	bool operator==(const EntityPosComp<T> &rhs) const;
	bool operator!=(const EntityPosComp<T> &rhs) const;
	bool operator<(const EntityPosComp<T> &rhs) const;
};

template<typename T>
EntityPosComp<T>::EntityPosComp()
{
}

template<typename T>
EntityPosComp<T>::EntityPosComp(const EntityPosComp<T> &rhs)
	: EntityComponent<T>(rhs)
	, _pPosComp(rhs._pPosComp)
	, _pPosInfo(rhs._pPosInfo)
{
}

template<typename T>
EntityPosComp<T>::EntityPosComp(IEntity* entity, T* pComponent, bool bGetPos)
	: EntityComponent<T>(entity, pComponent)
	, _pPosComp(nullptr)
	, _pPosInfo(nullptr)
{
	if (bGetPos)
	{
		_pPosComp = entity->EnsureComponent<IPositionComponent>();
		_pPosInfo = &_pPosComp->GetPosInfo();
	}
}

template<typename T>
EntityPosComp<T> &EntityPosComp<T>::operator=(const EntityPosComp<T> &rhs)
{
	__super::operator=(rhs);
	_pPosComp = rhs._pPosComp;
	_pPosInfo = rhs._pPosInfo;

	return *this;
}

template<typename T>
bool EntityPosComp<T>::operator==(const EntityPosComp<T> &rhs) const
{
	return __super::operator==(rhs);
}

template<typename T>
bool EntityPosComp<T>::operator!=(const EntityPosComp<T> &rhs) const
{
	return __super::operator!=(rhs);
}

template<typename T>
bool EntityPosComp<T>::operator<(const EntityPosComp<T> &rhs) const
{
	return __super::operator<(rhs);
}
