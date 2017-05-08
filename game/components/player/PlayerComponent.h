#pragma once

#include "coreEntity\entity\EntityComponent.h"
#include "coreEntity\entity\EntityListener.h"

namespace ff
{
	class IInputMapping;
}

class BulletComponent;
class IEntity;
class IExternalPlayerControl;
enum PoweruentityType;

class __declspec(uuid("e8af8ae3-a0dd-493f-b14d-36b03cabc758"))
	PlayerComponent : public ff::ComBase, public IEntityEventListener
{
public:
	DECLARE_HEADER(PlayerComponent);

	bool Init(IEntity *entity, size_t index);

	size_t GetIndex() const;
	ff::IInputMapping* GetInputMapping() const;

	bool HasBullet() const;
	void AddBullet(IEntity *pBulletEntity);

	bool HasPowerup(PoweruentityType type) const;
	size_t GetPowerupCounter() const;
	float GetPowerupCounterPercent() const;
	void SetPowerup(PoweruentityType type, size_t nCounter);
	PoweruentityType GetBestPowerup() const;

	void AdvanceCounters();

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

private:
	typedef EntityComponent<BulletComponent> EntityBullet;

	size_t _index;
	size_t _bulletCounter;
	size_t _powerupCounter;
	float _origPowerupCounter;
	PoweruentityType _powerup;
	ff::ComPtr<ff::IInputMapping> _inputMap;
	ff::ComPtr<IProxyEntityEventListener> _listener;
	ff::Vector<EntityBullet> _bullets;
};
