#pragma once

#include "coreEntity\component\ComponentListener2.h"
#include "coreEntity\entity\EntityComponent.h"
#include "coreEntity\system\System.h"
#include "components\audio\AudioEffectComponent.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class IAudioEffect;
}

class IEntity;
struct BulletEventArgs;

enum EAudioEffect
{
	EFFECT_BONUS_SAUCER,
	EFFECT_BONUS_SAUCER_FAST,
	EFFECT_BORN_SAUCER_0,
	EFFECT_FREE_LIFE_0,
	EFFECT_HIT_BULLET_0,
	EFFECT_HIT_INVADER_0,
	EFFECT_HIT_INVADER_1,
	EFFECT_HIT_INVADER_2,
	EFFECT_HIT_INVADER_3,
	EFFECT_HIT_INVADER_4,
	EFFECT_HIT_INVADER_5,
	EFFECT_HIT_INVADER_6,
	EFFECT_HIT_INVADER_7,
	EFFECT_HIT_PLAYER_0,
	EFFECT_HIT_SAUCER_0,
	EFFECT_HIT_SHIELD_0,
	EFFECT_LEVEL_START_0,
	EFFECT_LEVEL_COMPLETE_0,
	EFFECT_MOVE_INVADER_0,
	EFFECT_MOVE_INVADER_1,
	EFFECT_DRUM_BEAT_0,
	EFFECT_DRUM_BEAT_1,
	EFFECT_DRUM_BEAT_2,
	EFFECT_DRUM_BEAT_3,
	EFFECT_POWERUP_0,
	EFFECT_SHOT_INVADER_0,
	EFFECT_SHOT_PLAYER_0,

	EFFECT_COUNT
};

enum EAudioPriority
{
	EFFECT_PRI_NONE,
	EFFECT_PRI_NORMAL_LOW,
	EFFECT_PRI_NORMAL_HIGH,
	EFFECT_PRI_BACKGROUND_LOW,
	EFFECT_PRI_BACKGROUND_HIGH,
};

class __declspec(uuid("d7c110df-4743-4e9e-ae7c-12ea23f6d192"))
	AudioSystem
		: public ff::ComBase
		, public ISystem
		, public IEntityEventListener
		, public IComponentListener
{
public:
	DECLARE_HEADER(AudioSystem);

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render (IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

	// IComponentListener
	virtual void OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;
	virtual void OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp) override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	void PlayEffect(EAudioEffect effect, float freqRatio = 1, IEntity *entity = nullptr, ff::IAudioPlaying **ppPlaying = nullptr);
	void StopEffect(EAudioEffect effect);
	void StopAllEffects();

	void OnEntityBorn(IEntity *entity);
	void OnEntityDied(IEntity *entity);
	void OnBulletShoot(const BulletEventArgs &eventArgs);
	void OnBulletHit(const BulletEventArgs &eventArgs);
	void OnInvadersMoved();
	void OnLevelStart();

	typedef EntityComponent<AudioEffectComponent> EntityEffect;

	struct EffectInfo
	{
		EffectInfo();

		ff::TypedResource<ff::IAudioEffect> _effect;
		EAudioPriority _priority;
		float _volume;
		bool _bStoppable;
	};

	IEntityDomain* _domain;
	EntityEventListener _bulletShootListener;
	EntityEventListener _bulletHitListener;
	EntityEventListener _bornListener;
	EntityEventListener _diedListener;
	EntityEventListener _invadersMovedSidewaysListener;
	EntityEventListener _invadersMovedDownListener;
	EntityEventListener _invadersHitListener;
	EntityEventListener _levelStartListener;
	EntityEventListener _levelCompleteListener;
	EntityEventListener _levelBaseHitListener;
	EntityEventListener _powerupListener;
	EntityEventListener _lifeListener;
	EntityEventListener _playStartListener;
	EntityEventListener _playStopListener;
	ComponentListener<AudioEffectComponent> _effectListener;
	EffectInfo _effects[EFFECT_COUNT];
	size_t _invaderMove;
	size_t _playerShotBullet;
	size_t _baseHitCount;
	ff::ComPtr<ff::IAudioPlaying> _playingShieldHit;
	ff::ComPtr<ff::IAudioPlaying> _playingSaucer;
	ff::Vector<EntityEffect> _entities;
	bool _playingLevel;
	bool _drumBeat;
};
