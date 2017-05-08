#include "pch.h"
#include "Globals.h"
#include "InputEvents.h"
#include "ThisApplication.h"
#include "entities\EntityEvents.h"
#include "components\bullet\BulletComponent.h"
#include "components\player\PlayerAdvanceRender.h"
#include "components\player\PlayerComponent.h"
#include "components\powerup\PowerupComponent.h"
#include "components\core\PositionComponent.h"
#include "components\core\StateComponent.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityEvents.h"
#include "Globals\MetroGlobals.h"
#include "Input\InputMapping.h"
#include "Input\KeyboardDevice.h"
#include "Module\ModuleFactory.h"
#include "services\GlobalPlayerService.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PlayerComponent");
	module.RegisterClassT<PlayerComponent>(name);
});

BEGIN_INTERFACES(PlayerComponent)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

PlayerComponent::PlayerComponent()
	: _index(0)
	, _bulletCounter(0)
	, _powerupCounter(0)
	, _origPowerupCounter(0)
	, _powerup(POWERUP_TYPE_NONE)
{
}

PlayerComponent::~PlayerComponent()
{
	if (_listener)
	{
		_listener->SetOwner(nullptr);
	}
}

bool PlayerComponent::Init(IEntity *entity, size_t index)
{
	assertRetVal(entity && index >= 0 && index < 2, false);

	_index = index;

	ff::ComPtr<GlobalPlayerService> pGlobalPlayers;
	assertRetVal(GetService(entity, &pGlobalPlayers), false);

	assertRetVal(CreateProxyEntityEventListener(this, &_listener), false);
	assertRetVal(CreatePlayerInputMapping(index, pGlobalPlayers->GetGameMode(), &_inputMap), false);

	PlayerGlobals* pPlayerGlobals = pGlobalPlayers->GetPlayerGlobals(index);
	IStateComponent* pState = entity->EnsureComponent<IStateComponent>();

	if (!pPlayerGlobals->IsActive())
	{
		pState->SetEnumState<PlayerState>(PS_DEAD);
	}

	return true;
}

size_t PlayerComponent::GetIndex() const
{
	return _index;
}

ff::IInputMapping *PlayerComponent::GetInputMapping() const
{
	return _inputMap;
}

bool PlayerComponent::HasBullet() const
{
	return !_bullets.IsEmpty();
}

void PlayerComponent::AddBullet(IEntity *pBulletEntity)
{
	BulletComponent *pBullet = pBulletEntity->GetComponent<BulletComponent>();
	assertRet(pBullet);

	_bulletCounter = 0;
	_bullets.Push(EntityBullet(pBulletEntity, pBullet));

	IEntityEventListeners::AddListener(pBulletEntity, _listener);
}

bool PlayerComponent::HasPowerup(PoweruentityType type) const
{
#ifdef _DEBUG
	ff::IKeyboardDevice *keys = ff::MetroGlobals::Get()->GetKeys();
	if (type == POWERUP_TYPE_HOMING_SHOT && keys->GetKey(VK_CONTROL))
	{
		return true;
	}
	else if (type == POWERUP_TYPE_SHIELD && keys->GetKey(VK_CONTROL))
	{
		return true;
	}
	else if (type == POWERUP_TYPE_AMMO && keys->GetKey(VK_SHIFT))
	{
		return true;
	}
#endif

	return _powerupCounter && _powerup != POWERUP_TYPE_NONE && _powerup == type;
}

size_t PlayerComponent::GetPowerupCounter() const
{
	return _powerup != POWERUP_TYPE_NONE ? _powerupCounter : 0;
}

float PlayerComponent::GetPowerupCounterPercent() const
{
	return (_powerup != POWERUP_TYPE_NONE && _origPowerupCounter > 0)
		? _powerupCounter / _origPowerupCounter : 0;
}

void PlayerComponent::SetPowerup(PoweruentityType type, size_t nCounter)
{
	_origPowerupCounter = (float)nCounter;
	_powerupCounter = nCounter;
	_powerup = nCounter ? type : POWERUP_TYPE_NONE;
}

PoweruentityType PlayerComponent::GetBestPowerup() const
{
	return _powerupCounter ? _powerup : POWERUP_TYPE_NONE;
}

void PlayerComponent::AdvanceCounters()
{
	if (_powerupCounter && !--_powerupCounter)
	{
		_origPowerupCounter = 0;
		_powerup = POWERUP_TYPE_NONE;
	}

	if (_bullets.Size())
	{
		if (++_bulletCounter >= Globals::GetBulletReshootFrames())
		{
			// homing bullets could stay on the screen forever,
			// so stop paying attention to them after a while
			_bullets.Clear();
		}
		else if (_bulletCounter >= 30) // spread shots allow quicker shots
		{
			bool bAllSpread = true;

			for (size_t i = 0; i < _bullets.Size(); i++)
			{
				if (_bullets[i]._component->GetType() != BULLET_TYPE_SPREAD_PLAYER_0 &&
					_bullets[i]._component->GetType() != BULLET_TYPE_SPREAD_PLAYER_1)
				{
					bAllSpread = false;
					break;
				}
			}

			if (bAllSpread)
			{
				_bullets.Clear();
			}
		}
	}
}

void PlayerComponent::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_DIED || eventName == ENTITY_EVENT_DESTROY)
	{
		BulletComponent *pBullet = entity->GetComponent<BulletComponent>();
		size_t i = pBullet ? _bullets.Find(EntityBullet(entity, pBullet)) : ff::INVALID_SIZE;

		if (i != ff::INVALID_SIZE)
		{
			_bullets.Delete(i);
		}
	}
}
