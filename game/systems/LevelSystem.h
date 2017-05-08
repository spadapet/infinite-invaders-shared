#pragma once

#include "coreEntity\entity\EntityListener.h"
#include "coreEntity\system\System.h"

class IEntity;

class __declspec(uuid("1e73ad5d-0439-4af3-b6c9-8b00381d4136"))
	LevelSystem
		: public ff::ComBase
		, public ISystem
		, public IEntityEventListener
{
public:
	DECLARE_HEADER(LevelSystem);

	bool DidCompleteLevel() const;

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render (IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	EntityEventListener _bornListener;
	EntityEventListener _diedListener;
	ff::Set<ff::ComPtr<IEntity>> _aliveEntities;
	bool _startedLevel;
	bool _completedLevel;
};
