#include "pch.h"
#include "ThisApplication.h"
#include "components\core\StateComponent.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityEvents.h"
#include "coreEntity\entity\EntityListener.h"
#include "entities\EntityEvents.h"
#include "Module\ModuleFactory.h"

class __declspec(uuid("31355a19-6b8d-45ba-94fb-ecb73edfce96"))
	StateComponent
		: public ff::ComBase
		, public IStateComponent
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(StateComponent);

	// IStateComponent
	virtual int GetState() const override;
	virtual void SetState(int state) override;

	virtual size_t GetStateCounter() const override;
	virtual size_t IncrementStateCounter() override;
	virtual void ResetStateCounter() override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

private:
	IEntity* _entity;
	size_t _counter;
	int _state;
};

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"StateComponent");
	module.RegisterClassT<StateComponent>(name, __uuidof(IStateComponent));
});;

BEGIN_INTERFACES(StateComponent)
	HAS_INTERFACE(IStateComponent)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

StateComponent::StateComponent()
	: _entity(nullptr)
	, _counter(0)
	, _state(0)
{
}

StateComponent::~StateComponent()
{
	assert(!_entity);
}

int StateComponent::GetState() const
{
	return _state;
}

void StateComponent::SetState(int state)
{
	if (state != _state)
	{
		StateEventArgs eventArgs;
		eventArgs._nOldCounter = _counter;
		eventArgs._nOldState = _state;
		eventArgs._nNewState = state;

		_state = state;
		_counter = 0;

		if (_entity)
		{
			_entity->TriggerEvent(ENTITY_EVENT_STATE_CHANGED, &eventArgs);
		}
	}
}

size_t StateComponent::GetStateCounter() const
{
	return _counter;
}

size_t StateComponent::IncrementStateCounter()
{
	return ++_counter;
}

void StateComponent::ResetStateCounter()
{
	_counter = 0;
}

void StateComponent::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_ADD_COMPONENT)
	{
		assertSz(!_entity, L"Can't add a StateComponent to more than entity");
		_entity = entity;
	}
	else if (eventName == ENTITY_EVENT_REMOVE_COMPONENT)
	{
		_entity = nullptr;
	}
}
