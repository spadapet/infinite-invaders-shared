#include "pch.h"
#include "Audio\AudioDevice.h"
#include "Audio\AudioEffect.h"
#include "Audio\AudioPlaying.h"
#include "components\audio\AudioEffectComponent.h"
#include "components\bullet\BulletComponent.h"
#include "components\invader\InvaderComponent.h"
#include "components\player\PlayerComponent.h"
#include "components\saucer\SaucerComponent.h"
#include "components\shield\ShieldComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "entities\EntityEvents.h"
#include "Module\ModuleFactory.h"
#include "services\GameBeatService.h"
#include "services\GlobalPlayerService.h"
#include "services\InvaderService.h"
#include "services\SaucerService.h"
#include "systems\AudioSystem.h"
#include "ThisApplication.h"

AudioSystem::EffectInfo::EffectInfo()
	: _priority(EFFECT_PRI_NORMAL_LOW)
	, _volume(1)
	, _bStoppable(true)
{
}

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"AudioSystem");
	module.RegisterClassT<AudioSystem>(name);
});

BEGIN_INTERFACES(AudioSystem)
	HAS_INTERFACE(ISystem)
	HAS_INTERFACE(IComponentListener)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

AudioSystem::AudioSystem()
	: _domain(nullptr)
	, _invaderMove(0)
	, _playerShotBullet(ff::INVALID_SIZE)
	, _baseHitCount(0)
	, _playingLevel(true)
	, _drumBeat(false)
{
}

AudioSystem::~AudioSystem()
{
}

HRESULT AudioSystem::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	_domain = pDomainProvider->GetDomain();

	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	size_t nLevel = Globals::GetCurrentLevel(_domain);
	Difficulty difficulty = Globals::GetDifficulty(_domain);
	_drumBeat = Globals::IsInvaderBonusLevel(difficulty, nLevel);

	_bulletShootListener.Init(_domain, ENTITY_EVENT_BULLET_SHOOT, this);
	_bulletHitListener.Init(_domain, ENTITY_EVENT_BULLET_HIT, this);
	_bornListener.Init(_domain, ENTITY_EVENT_BORN, this);
	_diedListener.Init(_domain, ENTITY_EVENT_DIED, this);
	_invadersMovedSidewaysListener.Init(_domain, ENTITY_EVENT_INVADERS_MOVED_SIDEWAYS, this);
	_invadersMovedDownListener.Init(_domain, ENTITY_EVENT_INVADERS_MOVED_DOWN, this);
	_invadersHitListener.Init(_domain, ENTITY_EVENT_INVADER_HIT, this);
	_levelStartListener.Init(_domain, ENTITY_EVENT_LEVEL_START, this);
	_levelCompleteListener.Init(_domain, ENTITY_EVENT_LEVEL_COMPLETE, this);
	_levelBaseHitListener.Init(_domain, ENTITY_EVENT_LEVEL_BASE_HIT, this);
	_powerupListener.Init(_domain, ENTITY_EVENT_COLLECT_POWERUP, this);
	_lifeListener.Init(_domain, ENTITY_EVENT_GOT_FREE_LIFE, this);
	_playStartListener.Init(_domain, ENTITY_EVENT_PLAYING_START, this);
	_playStopListener.Init(_domain, ENTITY_EVENT_PLAYING_STOP, this);
	_effectListener.Init(_domain, this);

	struct
	{
		EAudioEffect _effect;
		LPCTSTR _name;
		EAudioPriority _priority;
		bool _bStoppable;
	}
	effects[] =
	{
		{ EFFECT_BONUS_SAUCER, L"Bonus Saucer Effect", EFFECT_PRI_BACKGROUND_HIGH, true },
		{ EFFECT_BONUS_SAUCER_FAST, L"Bonus Saucer Fast", EFFECT_PRI_BACKGROUND_HIGH, true },
		{ EFFECT_BORN_SAUCER_0, L"Born Saucer 0", EFFECT_PRI_BACKGROUND_HIGH, true },
		{ EFFECT_FREE_LIFE_0, L"Free Life 0", EFFECT_PRI_NORMAL_HIGH, true },
		{ EFFECT_HIT_BULLET_0, L"Hit Bullet 0", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_INVADER_0, L"Hit Invader 0", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_INVADER_1, L"Hit Invader 1", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_INVADER_2, L"Hit Invader 2", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_INVADER_3, L"Hit Invader 3", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_INVADER_4, L"Hit Invader 4", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_INVADER_5, L"Hit Invader 5", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_INVADER_6, L"Hit Invader 6", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_INVADER_7, L"Hit Invader 7", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_PLAYER_0, L"Hit Player 0", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_HIT_SAUCER_0, L"Hit Saucer 0", EFFECT_PRI_NORMAL_LOW, false },
		{ EFFECT_HIT_SHIELD_0, L"Hit Shield 0", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_LEVEL_START_0, L"Level Start 0", EFFECT_PRI_BACKGROUND_HIGH, true },
		{ EFFECT_LEVEL_COMPLETE_0,	L"Level Win 0", EFFECT_PRI_BACKGROUND_HIGH, false },
		{ EFFECT_MOVE_INVADER_0, L"Move Invader 0", EFFECT_PRI_BACKGROUND_LOW, true },
		{ EFFECT_MOVE_INVADER_1, L"Move Invader 1", EFFECT_PRI_BACKGROUND_LOW, true },
		{ EFFECT_DRUM_BEAT_0, L"Drum Beat 0", EFFECT_PRI_BACKGROUND_LOW, true },
		{ EFFECT_DRUM_BEAT_1, L"Drum Beat 1", EFFECT_PRI_BACKGROUND_LOW, true },
		{ EFFECT_DRUM_BEAT_2, L"Drum Beat 2", EFFECT_PRI_BACKGROUND_LOW, true },
		{ EFFECT_DRUM_BEAT_3, L"Drum Beat 3", EFFECT_PRI_BACKGROUND_LOW, true },
		{ EFFECT_POWERUP_0, L"Powerup 0 Effect", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_SHOT_INVADER_0, L"Shot Invader 0", EFFECT_PRI_NORMAL_LOW, true },
		{ EFFECT_SHOT_PLAYER_0, L"Shot Player 0", EFFECT_PRI_NORMAL_LOW, true },
	};

	for (size_t i = 0; i < _countof(effects); i++)
	{
		EffectInfo &info = _effects[effects[i]._effect];
		info._effect.Init(effects[i]._name);
		info._priority = effects[i]._priority;
		info._bStoppable = effects[i]._bStoppable;
	}

	return __super::_Construct(unkOuter);
}

int AudioSystem::GetSystemPriority() const
{
	return SYS_PRI_ADVANCE_NORMAL;
}

PingResult AudioSystem::Ping(IEntityDomain *pDomain)
{
	return PING_RESULT_RUNNING;
}

void AudioSystem::Advance(IEntityDomain *pDomain)
{
	// Clean up any entities that have stopped effects

	for (size_t i = ff::PreviousSize(_entities.Size()); i != ff::INVALID_SIZE; i = ff::PreviousSize(i))
	{
		EntityEffect ee = _entities[i];
		ff::IAudioPlaying* playing = ee._component->GetEffectPlaying();

		if (playing && playing->IsStopped())
		{
			ee._entity->RemoveComponent(ee._component);
		}
	}

	if (_playingShieldHit && _playingShieldHit->IsStopped())
	{
		_playingShieldHit = nullptr;
	}

	if (_playingSaucer && _playingSaucer->IsStopped())
	{
		_playingSaucer = nullptr;
	}

	_playerShotBullet = ff::INVALID_SIZE;
}

void AudioSystem::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
}

void AudioSystem::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_BORN)
	{
		OnEntityBorn(entity);
	}
	else if (eventName == ENTITY_EVENT_DIED)
	{
		OnEntityDied(entity);
	}
	else if (eventName == ENTITY_EVENT_BULLET_SHOOT)
	{
		OnBulletShoot(*(const BulletEventArgs*)eventArgs);
	}
	else if (eventName == ENTITY_EVENT_BULLET_HIT)
	{
		OnBulletHit(*(const BulletEventArgs*)eventArgs);
	}
	else if (eventName == ENTITY_EVENT_INVADERS_MOVED_SIDEWAYS ||
		eventName == ENTITY_EVENT_INVADERS_MOVED_DOWN)
	{
		OnInvadersMoved();
	}
	else if (eventName == ENTITY_EVENT_INVADER_HIT)
	{
		if (entity->GetComponent<PlayerComponent>())
		{
			PlayEffect(EFFECT_HIT_PLAYER_0);
		}
	}
	else if (eventName == ENTITY_EVENT_COLLECT_POWERUP)
	{
		PlayEffect(EFFECT_POWERUP_0);
	}
	else if (eventName == ENTITY_EVENT_GOT_FREE_LIFE)
	{
		PlayEffect(EFFECT_FREE_LIFE_0);
	}
	else if (eventName == ENTITY_EVENT_LEVEL_START)
	{
		OnLevelStart();
	}
	else if (eventName == ENTITY_EVENT_PLAYING_START)
	{
		_playingLevel = true;
	}
	else if (eventName == ENTITY_EVENT_PLAYING_STOP)
	{
		StopAllEffects();
		_playingLevel = false;
	}
	else if (eventName == ENTITY_EVENT_LEVEL_COMPLETE)
	{
		StopAllEffects();
	}
	else if (eventName == ENTITY_EVENT_LEVEL_BASE_HIT)
	{
		if ((_baseHitCount++) % 3 == 0)
		{
			PlayEffect(EFFECT_HIT_BULLET_0);
		}
	}
}

void AudioSystem::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<AudioEffectComponent> pEffect;
	assertRet(pEffect.QueryFrom(pComp));

	_entities.Push(EntityEffect(entity, pEffect));
}

void AudioSystem::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<AudioEffectComponent> pEffect;
	assertRet(pEffect.QueryFrom(pComp));

	size_t i = _entities.Find(EntityEffect(entity, pEffect));
	assertRet(i != ff::INVALID_SIZE);

	_entities[i] = _entities.GetLast();
	_entities.Pop();
}

void AudioSystem::PlayEffect(EAudioEffect effect, float freqRatio, IEntity *entity, ff::IAudioPlaying **ppPlaying)
{
	bool bEffectsOn = _domain->GetApp()->GetOptions().GetBool(ThisApplication::OPTION_SOUND_ON, true);

	if (bEffectsOn &&
		_playingLevel &&
		effect >= 0 &&
		effect < EFFECT_COUNT &&
		_effects[effect]._effect.Flush() &&
		_effects[effect]._volume > 0)
	{
		EAudioPriority stopPriority = EFFECT_PRI_NONE;
		EAudioPriority bailPriority = EFFECT_PRI_NONE;
		ff::ComPtr<ff::IAudioPlaying> playing;

		switch (_effects[effect]._priority)
		{
		case EFFECT_PRI_BACKGROUND_HIGH:
			stopPriority = EFFECT_PRI_BACKGROUND_LOW;
			break;

		case EFFECT_PRI_BACKGROUND_LOW:
			bailPriority = EFFECT_PRI_BACKGROUND_HIGH;
			break;

		case EFFECT_PRI_NORMAL_HIGH:
			stopPriority = EFFECT_PRI_NORMAL_LOW;
			break;

		case EFFECT_PRI_NORMAL_LOW:
			bailPriority = EFFECT_PRI_NORMAL_HIGH;
			break;
		}

		if (stopPriority != EFFECT_PRI_NONE)
		{
			for (size_t i = 0; i < _countof(_effects); i++)
			{
				if (_effects[i]._priority == stopPriority && _effects[i]._effect.GetObject())
				{
					_effects[i]._effect.GetObject()->StopAll();
				}
			}
		}

		if (bailPriority != EFFECT_PRI_NONE)
		{
			for (size_t i = 0; i < _countof(_effects); i++)
			{
				if (_effects[i]._priority == bailPriority &&
					_effects[i]._effect.GetObject() &&
					_effects[i]._effect.GetObject()->IsPlaying())
				{
					return;
				}
			}
		}

		if (_effects[effect]._effect.GetObject() &&
			_effects[effect]._effect.GetObject()->Play(true, _effects[effect]._volume, freqRatio, &playing))
		{
			if (entity)
			{
				ff::ComPtr<AudioEffectComponent> pEffect;

				if (entity->CreateComponent<AudioEffectComponent>(&pEffect))
				{
					pEffect->SetEffectPlaying(playing);
					entity->AddComponent<AudioEffectComponent>(pEffect);
				}
			}

			if (ppPlaying)
			{
				*ppPlaying = ff::GetAddRef(playing.Interface());
			}
		}
	}
}

void AudioSystem::StopEffect(EAudioEffect effect)
{
	if (effect >= 0 && effect < EFFECT_COUNT && _effects[effect]._effect.GetObject())
	{
		_effects[effect]._effect.GetObject()->StopAll();
	}
}

void AudioSystem::StopAllEffects()
{
	for (size_t i = 0; i < _countof(_effects); i++)
	{
		if (_effects[i]._bStoppable && _effects[i]._effect.GetObject())
		{
			_effects[i]._effect.GetObject()->StopAll();
		}
	}

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		ff::IAudioPlaying *playing = _entities[i]._component->GetEffectPlaying();

		if (playing)
		{
			playing->Stop();
		}
	}

	if (_playingSaucer)
	{
		_playingSaucer->Stop();
	}

	if (_playingShieldHit)
	{
		_playingShieldHit->Stop();
	}
}

void AudioSystem::OnEntityBorn(IEntity *entity)
{
	SaucerComponent *pSaucer = entity->GetComponent<SaucerComponent>();

	if (pSaucer && !pSaucer->IsBonus())
	{
		if (_playingSaucer)
		{
			_playingSaucer->Stop();
			_playingSaucer = nullptr;
		}

		PlayEffect(EFFECT_BORN_SAUCER_0, 1, entity, &_playingSaucer);
	}
}

void AudioSystem::OnEntityDied(IEntity *entity)
{
}

void AudioSystem::OnBulletShoot(const BulletEventArgs &eventArgs)
{
	PlayerComponent* pPlayer = eventArgs._pSource->GetComponent<PlayerComponent>();
	InvaderComponent* pInvader = eventArgs._pSource->GetComponent<InvaderComponent>();

	if (pPlayer)
	{
		if (_playerShotBullet != pPlayer->GetIndex())
		{
			// If the player shoots multiple bullets, only make a sound for one of them
			_playerShotBullet = pPlayer->GetIndex();

			PlayEffect(EFFECT_SHOT_PLAYER_0, 1, eventArgs._pBullet);
		}
	}
	else if (pInvader)
	{
		PlayEffect(EFFECT_SHOT_INVADER_0, 1, eventArgs._pBullet);
	}
}

void AudioSystem::OnBulletHit(const BulletEventArgs &eventArgs)
{
	InvaderComponent* pInvader = eventArgs._pSource->GetComponent<InvaderComponent>();
	PlayerComponent* pPlayer = eventArgs._pSource->GetComponent<PlayerComponent>();
	SaucerComponent* pSaucer = eventArgs._pSource->GetComponent<SaucerComponent>();
	ShieldComponent* pShield = eventArgs._pSource->GetComponent<ShieldComponent>();
	BulletComponent* pBullet = eventArgs._pSource->GetComponent<BulletComponent>();

	if (pInvader)
	{
		bool bWinLevel = false;

		ff::ComPtr<IInvaderService> pInvaders;
		if (GetService(_domain, &pInvaders))
		{
			bWinLevel = pInvaders->GetInvaderCount() == 1;
		}

		if (bWinLevel)
		{
			PlayEffect(EFFECT_LEVEL_COMPLETE_0);
		}
		else
		{
			switch (pInvader->GetType())
			{
			default: assert(false); __fallthrough;
			case INVADER_TYPE_0: PlayEffect(EFFECT_HIT_INVADER_0); break;
			case INVADER_TYPE_1: PlayEffect(EFFECT_HIT_INVADER_1); break;
			case INVADER_TYPE_2: PlayEffect(EFFECT_HIT_INVADER_2); break;
			case INVADER_TYPE_3: PlayEffect(EFFECT_HIT_INVADER_3); break;
			case INVADER_TYPE_4: PlayEffect(EFFECT_HIT_INVADER_4); break;
			case INVADER_TYPE_5: PlayEffect(EFFECT_HIT_INVADER_5); break;
			case INVADER_TYPE_6: PlayEffect(EFFECT_HIT_INVADER_6); break;
			case INVADER_TYPE_7: PlayEffect(EFFECT_HIT_INVADER_7); break;
			case INVADER_TYPE_BONUS_0: PlayEffect(EFFECT_HIT_INVADER_0); break;
			case INVADER_TYPE_BONUS_1: PlayEffect(EFFECT_HIT_INVADER_1); break;
			case INVADER_TYPE_BONUS_2: PlayEffect(EFFECT_HIT_INVADER_2); break;
			}
		}
	}
	else if (pPlayer)
	{
		PlayEffect(EFFECT_HIT_PLAYER_0);
	}
	else if (pShield)
	{
		// See if an invader was treated as a bullet
		pInvader = eventArgs._pBullet->GetComponent<InvaderComponent>();

		if (pInvader)
		{
			if (!_playingShieldHit)
			{
				PlayEffect(EFFECT_HIT_SHIELD_0, 1, nullptr, &_playingShieldHit);
			}
		}
		else
		{
			PlayEffect(EFFECT_HIT_SHIELD_0);
		}
	}
	else if (pBullet)
	{
		PlayEffect(EFFECT_HIT_BULLET_0);
	}
	else if (pSaucer && pSaucer->IsBonus())
	{
		if (_playingSaucer)
		{
			_playingSaucer->Stop();
			_playingSaucer = nullptr;
		}

		PlayEffect(EFFECT_BONUS_SAUCER_FAST, 1, eventArgs._pSource, &_playingSaucer);
	}
	else if (pSaucer && !pSaucer->IsBonus())
	{
		PlayEffect(EFFECT_HIT_SAUCER_0);
	}
}

void AudioSystem::OnInvadersMoved()
{
	if (!_playingSaucer)
	{
		float freq = 1.0f;

		ff::ComPtr<IGameBeatService> pBeat;
		if (GetService(_domain, &pBeat) && pBeat->GetFramesPerBeat() < 12)
		{
			if (_drumBeat)
			{
				freq = (2.0f - pBeat->GetFramesPerBeat() / 6.0f) + 1.0f;
			}
			else
			{
				freq = (1.5f - pBeat->GetFramesPerBeat() / 12.0f) + 1.0f;
			}
		}

		EAudioEffect bassEffect = EFFECT_COUNT;
		EAudioEffect drumEffect = EFFECT_COUNT;

		if (_drumBeat)
		{
			if (++_invaderMove == 4)
			{
				drumEffect = EFFECT_DRUM_BEAT_0;
				bassEffect = EFFECT_MOVE_INVADER_0;
			}
			else if (_invaderMove == 8)
			{
				drumEffect = EFFECT_DRUM_BEAT_1;
				bassEffect = EFFECT_MOVE_INVADER_1;
			}
			else if (_invaderMove == 12)
			{
				drumEffect = EFFECT_DRUM_BEAT_2;
				bassEffect = EFFECT_MOVE_INVADER_0;
			}
			else if (_invaderMove == 16)
			{
				drumEffect = EFFECT_DRUM_BEAT_3;
				bassEffect = EFFECT_MOVE_INVADER_1;

				_invaderMove = 0;
			}
		}
		else
		{
			if (++_invaderMove == 4)
			{
				bassEffect = EFFECT_MOVE_INVADER_0;
			}
			else if (_invaderMove == 8)
			{
				bassEffect = EFFECT_MOVE_INVADER_1;

				_invaderMove = 0;
			}
		}

		if (bassEffect != EFFECT_COUNT)
		{
			StopEffect(EFFECT_MOVE_INVADER_0);
			StopEffect(EFFECT_MOVE_INVADER_1);

			PlayEffect(bassEffect, freq);
		}

		if (drumEffect != EFFECT_COUNT)
		{
			StopEffect(EFFECT_DRUM_BEAT_0);
			StopEffect(EFFECT_DRUM_BEAT_1);
			StopEffect(EFFECT_DRUM_BEAT_2);
			StopEffect(EFFECT_DRUM_BEAT_3);

			PlayEffect(drumEffect, freq);
		}
	}
}

void AudioSystem::OnLevelStart()
{
	ff::ComPtr<GlobalPlayerService> pPlayers;
	ff::ComPtr<ISaucerService> pSaucers;

	if (GetService(_domain, &pPlayers) &&
		GetService(_domain, &pSaucers))
	{
		size_t nLevel = pPlayers->GetCurrentPlayerGlobals(0, true)->GetLevel();
		Difficulty difficulty = pPlayers->GetCurrentPlayerGlobals(0, true)->GetDifficulty();

		if (Globals::IsSaucerBonusLevel(difficulty, nLevel) && pSaucers->GetSaucerCount())
		{
			if (_playingSaucer)
			{
				_playingSaucer->Stop();
				_playingSaucer = nullptr;
			}

			PlayEffect(EFFECT_BONUS_SAUCER, 1, pSaucers->GetSaucer(0), &_playingSaucer);
		}
	}
}
