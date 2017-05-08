#pragma once

class IEntity;

// Just holds onto a pair of plain pointers - neither are AddRef'd
template<typename T>
struct EntityComponent
{
	IEntity* _entity;
	T* _component;

	EntityComponent();
	EntityComponent(const EntityComponent<T> &rhs);
	EntityComponent(IEntity* entity, T* pComponent);

	EntityComponent<T> &operator=(const EntityComponent<T> &rhs);

	bool operator==(const EntityComponent<T> &rhs) const;
	bool operator!=(const EntityComponent<T> &rhs) const;
	bool operator<(const EntityComponent<T> &rhs) const;
};

template<typename T>
EntityComponent<T>::EntityComponent()
{
}

template<typename T>
EntityComponent<T>::EntityComponent(const EntityComponent<T> &rhs)
	: _entity(rhs._entity)
	, _component(rhs._component)
{
}

template<typename T>
EntityComponent<T>::EntityComponent(IEntity* entity, T* pComponent)
	: _entity(entity)
	, _component(pComponent)
{
}

template<typename T>
EntityComponent<T> &EntityComponent<T>::operator=(const EntityComponent<T> &rhs)
{
	_entity = rhs._entity;
	_component = rhs._component;

	return *this;
}

template<typename T>
bool EntityComponent<T>::operator==(const EntityComponent<T> &rhs) const
{
	return ff::CompareObjects(*this, rhs) == 0;
}

template<typename T>
bool EntityComponent<T>::operator!=(const EntityComponent<T> &rhs) const
{
	return ff::CompareObjects(*this, rhs) != 0;
}

template<typename T>
bool EntityComponent<T>::operator<(const EntityComponent<T> &rhs) const
{
	return ff::CompareObjects(*this, rhs) < 0;
}

