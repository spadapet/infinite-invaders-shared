#include "pch.h"
#include "Globals.h"
#include "InputEvents.h"
#include "ThisApplication.h"
#include "COM\ServiceCollection.h"
#include "components\bullet\BulletComponent.h"
#include "components\player\PlayerAdvanceRender.h"
#include "components\player\PlayerComponent.h"
#include "components\core\PositionComponent.h"
#include "components\core\StateComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"
#include "Globals\MetroGlobals.h"
#include "Graph\Anim\AnimKeys.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Input\KeyboardDevice.h"
#include "Input\InputMapping.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"
#include "services\GlobalPlayerService.h"
#include "services\InvaderService.h"
#include "systems\RenderSystem.h"

static const size_t s_dyingWaitTicks = 180;

PlayerAdvanceRender::PlayerInfo::PlayerInfo()
{
	ff::ZeroObject(*this);
}

PlayerAdvanceRender::PlayerInfo::PlayerInfo(IEntity* entity, PlayerComponent* pPlayer)
	: _entity(entity)
	, _component(pPlayer)
	, _wheelsFrame(0)
	, _bodyFrame(0)
	, _hitSide(PLAYER_HIT_NONE)
{
}

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PlayerAdvanceRender");
	module.RegisterClassT<PlayerAdvanceRender>(name);
});

BEGIN_INTERFACES(PlayerAdvanceRender)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(I2dRenderComponent)
	HAS_INTERFACE(IEntityEventListener)
	HAS_INTERFACE(IComponentListener)
	HAS_INTERFACE(IPlayerService)
END_INTERFACES()

PlayerAdvanceRender::PlayerAdvanceRender()
{
}

PlayerAdvanceRender::~PlayerAdvanceRender()
{
}

HRESULT PlayerAdvanceRender::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	IEntityDomain* pDomain = pDomainProvider->GetDomain();
	ThisApplication* pApp = ThisApplication::Get(pDomain);

	_listener.Init(pDomainProvider->GetDomain(), this);
	_scoreListener.Init(pDomain, ENTITY_EVENT_ADD_SCORE, this);

	assertRetVal(GetService(pDomain, &_globalPlayers), false);
	assertRetVal(GetService(pDomain, &_factoryService), false);

	_standingAnim.Init(L"Player Standing");
	_movingAnim.Init(L"Player Moving");
	_shootingAnim.Init(L"Player Shooting");
	_shieldAnim.Init(L"Player Shield");
	_hitAnims[PLAYER_HIT_LEFT].Init(L"Player Hit Left");
	_hitAnims[PLAYER_HIT_RIGHT].Init(L"Player Hit Right");
	_hitAnims[PLAYER_HIT_MIDDLE].Init(L"Player Hit Middle");

	pDomain->GetServices()->AddService(__uuidof(IPlayerService), static_cast<IPlayerService*>(this));

	return __super::_Construct(unkOuter);
}

int PlayerAdvanceRender::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void PlayerAdvanceRender::Advance(IEntity *entity)
{
	for (size_t i = 0; i < _players.Size(); i++)
	{
		AdvancePlayer(entity->GetDomain(), _players[i]);
	}
}

bool PlayerAdvanceRender::IsPlayerAlive(PlayerState state, size_t index) const
{
	switch (state)
	{
	case PS_INIT:
		return _globalPlayers->GetPlayerGlobals(index)->IsActive();

	case PS_MOVING:
	case PS_SHOOTING:
	case PS_BACK_TO_LIFE:
		return true;

	default:
		return false;
	}
}

bool PlayerAdvanceRender::IsPlayerKillable(PlayerState state, size_t index, bool bGettingPowerup) const
{
#ifdef _DEBUG
	if (ff::MetroGlobals::Get()->GetKeys()->GetKey(VK_SHIFT))
	{
		return false;
	}
#endif

	return IsPlayerAlive(state, index) && (state != PS_BACK_TO_LIFE || bGettingPowerup);
}

void PlayerAdvanceRender::AdvancePlayer(IEntityDomain *pDomain, PlayerInfo &info)
{
	ThisApplication* pApp = ThisApplication::Get(pDomain);
	IStateComponent* pState = info._entity->GetComponent<IStateComponent>();
	PlayerState state = pState->GetEnumState<PlayerState>();

	pApp->AdvanceInputMapping(info._component->GetInputMapping());
	info._component->AdvanceCounters();
	pState->IncrementStateCounter();

	switch (state)
	{
	case PS_INIT:
		{
			const size_t index = info._component->GetIndex();
			PlayerGlobals* pPlayerGlobals = _globalPlayers->GetPlayerGlobals(index);

			state = pPlayerGlobals->IsActive() ? PS_MOVING : PS_DEAD;
			pState->SetEnumState<PlayerState>(state);
		}
		break;

	case PS_MOVING:
		assert(_globalPlayers->GetPlayerGlobals(info._component->GetIndex())->IsActive());
		AdvanceBodyFrame(info, _standingAnim.GetObject());
		break;

	case PS_SHOOTING:
		if (AdvanceBodyFrame(info, _shootingAnim.GetObject()))
		{
			info._bodyFrame = 0;
			pState->SetEnumState<PlayerState>(state = PS_MOVING);
		}
		break;

	case PS_BACK_TO_LIFE:
		if (pState->GetStateCounter() >= 120)
		{
			pState->SetEnumState<PlayerState>(state = PS_MOVING);
		}
		break;

	case PS_DYING:
		AdvanceBodyFrame(info, _hitAnims[info._hitSide].GetObject());

		if (pState->GetStateCounter() >= s_dyingWaitTicks)
		{
			pState->SetEnumState<PlayerState>(state = PS_DEAD);
		}
		else if (pState->GetStateCounter() < s_dyingWaitTicks - 30)
		{
			_factoryService->CreatePlayerDyingSmoke(info._entity, info._hitSide);
		}
		break;

	case PS_REBUILDING:
		if (pState->GetStateCounter() >= 180)
		{
			pState->SetEnumState<PlayerState>(state = PS_BACK_TO_LIFE);
		}
		break;

	case PS_DEAD:
		{
			const size_t index = info._component->GetIndex();
			PlayerGlobals* pPlayerGlobals = _globalPlayers->GetPlayerGlobals(index, true);

			if (pPlayerGlobals->GetLives() || pPlayerGlobals->IsActive())
			{
				if (!pPlayerGlobals->IsActive())
				{
					pPlayerGlobals->RemoveLives(1);
				}

				pState->SetEnumState<PlayerState>(state = PS_REBUILDING);
			}
		}
		break;
	}

	bool bCanMove = IsPlayerAlive(state, info._component->GetIndex());
	bool bCanShoot = !info._component->HasBullet();
	size_t totalAdvances = ff::MetroGlobals::Get()->GetGlobalTime()._advanceCount;

	if (!bCanShoot && info._component->HasPowerup(POWERUP_TYPE_AMMO) && !(totalAdvances % 8))
	{
		bCanShoot = true;
	}

	if (bCanShoot && bCanMove)
	{
		int shoot = info._component->GetInputMapping()->GetDigitalValue(PIV_SHOOT);

		for (size_t i = 0; !shoot; i++)
		{
			IExternalPlayerControl *pExternalControl = info._entity->GetComponent<IExternalPlayerControl>(i);

			if (pExternalControl != nullptr)
			{
				shoot = pExternalControl->GetDigitalPlayerValue(PIV_SHOOT);
			}
			else
			{
				break;
			}
		}

		if (shoot != 0)
		{
			ShootBullet(info, 0);

			if (info._component->HasPowerup(POWERUP_TYPE_MULTI_SHOT) ||
				info._component->HasPowerup(POWERUP_TYPE_SPREAD_SHOT))
			{
				ShootBullet(info, 1);
				ShootBullet(info, 2);
			}
		}
	}

	info._entity->GetComponent<IPositionComponent>()->SetVelocity(ff::PointFloat(0, 0));

	if (bCanMove)
	{
		AdvancePlayerMovement(info);
	}
}

void PlayerAdvanceRender::AdvancePlayerMovement(PlayerInfo &info)
{
	float left = info._component->GetInputMapping()->GetAnalogValue(PIV_MOVE_LEFT);
	float right = info._component->GetInputMapping()->GetAnalogValue(PIV_MOVE_RIGHT);
	float force = 0;
	const float deadZone = Globals::GetJoystickDeadZone();

	for (size_t i = 0; ; i++)
	{
		IExternalPlayerControl *pExternalControl = info._entity->GetComponent<IExternalPlayerControl>(i);

		if (pExternalControl != nullptr)
		{
			left = std::max(pExternalControl->GetAnalogPlayerValue(PIV_MOVE_LEFT), left);
			right = std::max(pExternalControl->GetAnalogPlayerValue(PIV_MOVE_RIGHT), right);
		}
		else
		{
			break;
		}
	}

	if (left >= deadZone && right < deadZone)
	{
		force = -(left - deadZone) / (1.0f - deadZone);
	}
	else if (left < deadZone && right >= deadZone)
	{
		force = (right - deadZone) / (1.0f - deadZone);
	}

	ff::ISpriteAnimation *movingAnim = _movingAnim.GetObject();
	if (movingAnim)
	{
		if (force < 0)
		{
			info._wheelsFrame -= movingAnim->GetFPS() / Globals::GetAdvancesPerSecondF();

			if (info._wheelsFrame < 0)
			{
				info._wheelsFrame += movingAnim->GetLastFrame();
			}
		}
		else if (force > 0)
		{
			info._wheelsFrame += movingAnim->GetFPS() / Globals::GetAdvancesPerSecondF();

			if (info._wheelsFrame >= movingAnim->GetLastFrame())
			{
				info._wheelsFrame -= movingAnim->GetLastFrame();
			}
		}
	}

	ff::ComPtr<IInvaderService> _invaders;
	GetService(info._entity, &_invaders);

	if (info._component->HasPowerup(POWERUP_TYPE_SPEED_BOOST) ||
		!_invaders || !_invaders->GetInvaderCount())
	{
		force *= Globals::GetBonusSpeed();
	}

	if (force != 0)
	{
		info._entity->TriggerEvent(ENTITY_EVENT_APPLY_FORCE, &ff::PointFloat(force, 0));

		_factoryService->CreatePlayerDust(info._entity);
	}

#ifdef _DEBUG
	if (ff::MetroGlobals::Get()->GetKeys()->GetKey('9'))
	{
		info._hitSide = PLAYER_HIT_MIDDLE;
		info._entity->GetComponent<IStateComponent>()->SetEnumState<PlayerState>(PS_DYING);
	}
#endif
}

bool PlayerAdvanceRender::AdvanceBodyFrame(PlayerInfo &info, ff::ISpriteAnimation *pAnim)
{
	if (pAnim)
	{
		info._bodyFrame += pAnim->GetFPS() / Globals::GetAdvancesPerSecondF();

		if (info._bodyFrame >= pAnim->GetLastFrame())
		{
			info._bodyFrame -= pAnim->GetLastFrame();
			return true;
		}
	}
	else
	{
		info._bodyFrame = 0;
	}

	return false;
}

int PlayerAdvanceRender::Get2dRenderPriority() const
{
	return LAYER_PRI_NORMAL;
}

void PlayerAdvanceRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	ThisApplication *pApp = ThisApplication::Get(entity);

	for (size_t i = 0; i < _players.Size(); i++)
	{
		const PlayerInfo& info = _players[i];
		const IPositionComponent* pPos = info._entity->GetComponent<IPositionComponent>();
		const IStateComponent* pState = info._entity->GetComponent<IStateComponent>();
		const PlayerState state = pState->GetEnumState<PlayerState>();
		const size_t index = info._component->GetIndex();
		const ff::PointFloat pos = pPos->GetPos(); // pApp->GetTime()._bankScale

		if (IsPlayerAlive(state, index))
		{
			// Player flashes when coming back to life
			if (state != PS_BACK_TO_LIFE || pState->GetStateCounter() % 16 < 8)
			{
				const ff::GlobalTime &appTime = ff::MetroGlobals::Get()->GetGlobalTime();
				ff::ISpriteAnimation* pBodyAnim = (state == PS_SHOOTING) ? _shootingAnim.GetObject() : _standingAnim.GetObject();
				DirectX::XMFLOAT4 wheelsColor = Globals::GetPlayerWheelsColor(index);
				DirectX::XMFLOAT4 bodyColor = Globals::GetPlayerBodyColor(index);
				bool bPowerFading = false;
				bool bBlink = false;

				if (info._component->GetBestPowerup() != POWERUP_TYPE_NONE)
				{
					DirectX::XMFLOAT4 bonusColor = Globals::GetBonusColor(info._component->GetBestPowerup());

					bPowerFading = (info._component->GetPowerupCounter() < 128);

					bBlink = bPowerFading
						? ((info._component->GetPowerupCounter() % 16 < 8) ? false : true)
						: false;

					float alpha = bPowerFading ? (bBlink ? 0.5f : 0.0f) : 0.375f;

					DirectX::XMStoreFloat4(&bodyColor,
						DirectX::XMVectorLerp(
							DirectX::XMLoadFloat4(&bodyColor),
							DirectX::XMLoadFloat4(&bonusColor),
							alpha));

					DirectX::XMStoreFloat4(&wheelsColor,
						DirectX::XMVectorLerp(
							DirectX::XMLoadFloat4(&wheelsColor),
							DirectX::XMLoadFloat4(&bonusColor),
							alpha));

					bodyColor.w = 1;
					wheelsColor.w = 1;
				}

				ff::ISpriteAnimation *movingAnim = _movingAnim.GetObject();
				if (movingAnim)
				{
					movingAnim->Render(render, ff::POSE_TWEEN_LINEAR_CLAMP, info._wheelsFrame, pos, nullptr, 0, &wheelsColor);
				}

				if (pBodyAnim)
				{
					pBodyAnim->Render(render, ff::POSE_TWEEN_LINEAR_CLAMP, info._bodyFrame, pos, nullptr, 0, &bodyColor);
				}

				ff::ISpriteAnimation *shieldAnim = _shieldAnim.GetObject();
				if (!bBlink && info._component->HasPowerup(POWERUP_TYPE_SHIELD) && shieldAnim)
				{
					float shieldFrame = (float)std::fmod(appTime._absoluteSeconds * shieldAnim->GetFPS(), (double)shieldAnim->GetLastFrame());
					shieldAnim->Render(render, ff::POSE_TWEEN_LINEAR_LOOP, shieldFrame, pos, nullptr, 0, &Globals::GetPlayerBodyColor(index));
				}
			}
		}
		else if (state == PS_DYING && _hitAnims[info._hitSide].GetObject())
		{
			DirectX::XMFLOAT4 color = Globals::GetPlayerBodyColor(index);

			// Fade out
			if (pState->GetStateCounter() > s_dyingWaitTicks - 60 && pState->GetStateCounter() <= s_dyingWaitTicks)
			{
				color.w = (s_dyingWaitTicks - pState->GetStateCounter()) / 60.0f;
			}

			ff::ISpriteAnimation *hitAnim = _hitAnims[info._hitSide].GetObject();
			hitAnim->Render(render, ff::POSE_TWEEN_LINEAR_CLAMP, info._bodyFrame, pos, nullptr, 0, &color);
		}
	}
}

void PlayerAdvanceRender::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_APPLY_FORCE)
	{
		OnEntityApplyForce(entity, *(ff::PointFloat*)eventArgs);
	}

	if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;

		HandleCollision(entity, eventArgs2._pOther);
	}

	if (eventName == ENTITY_EVENT_ADD_SCORE)
	{
		OnAddScore(*(const ScoreEventArgs*)eventArgs);
	}

	if (eventName == ENTITY_EVENT_COLLECT_POWERUP)
	{
		OnCollectPowerup(entity, *(const PowerupEventArgs*)eventArgs);
	}

	if (eventName == ENTITY_EVENT_STATE_CHANGED)
	{
		OnStateChanged(entity, *(const StateEventArgs*)eventArgs);
	}
}

void PlayerAdvanceRender::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<PlayerComponent> pPlayer;
	assertRet(pPlayer.QueryFrom(pComp));

	_players.Push(PlayerInfo(entity, pPlayer));

	IEntityEventListeners::AddListener(entity, this);
}

void PlayerAdvanceRender::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<PlayerComponent> pPlayer;
	assertRet(pPlayer.QueryFrom(pComp));

	for (size_t i = 0; i < _players.Size(); i++)
	{
		if (_players[i]._entity == entity &&
			_players[i]._component == pPlayer)
		{
			_players.Delete(i);
			break;
		}
	}

	IEntityEventListeners::RemoveListener(entity, this);
}

size_t PlayerAdvanceRender::GetPlayerCount() const
{
	return _players.Size();
}

IEntity *PlayerAdvanceRender::GetPlayer(size_t index) const
{
	assertRetVal(index >= 0 && index < _players.Size(), nullptr);

	return _players[index]._entity;
}

bool PlayerAdvanceRender::IsPlayerAlive(size_t index) const
{
	assertRetVal(index >= 0 && index < GetPlayerCount(), false);

	IStateComponent* pState = _players[index]._entity->GetComponent<IStateComponent>();

	return IsPlayerAlive(pState->GetEnumState<PlayerState>(), _players[index]._component->GetIndex());
}

bool PlayerAdvanceRender::IsAnyPlayerAlive() const
{
	for (size_t i = 0; i < _players.Size(); i++)
	{
		if (IsPlayerAlive(i))
		{
			return true;
		}
	}

	return false;
}

bool PlayerAdvanceRender::AreAllPlayersDead() const
{
	for (size_t i = 0; i < _players.Size(); i++)
	{
		IStateComponent* pState = _players[i]._entity->GetComponent<IStateComponent>();

		switch (pState->GetEnumState<PlayerState>())
		{
		case PS_DEAD:
			break;

		default:
			return false;
		}
	}

	return true;
}

bool PlayerAdvanceRender::AreAllPlayersOutOfLives() const
{
	for (size_t i = 0; i < _players.Size(); i++)
	{
		IStateComponent* pState = _players[i]._entity->GetComponent<IStateComponent>();

		switch (pState->GetEnumState<PlayerState>())
		{
		case PS_DEAD:
			{
				const size_t index = _players[i]._component->GetIndex();
				PlayerGlobals* pPlayerGlobals = _globalPlayers->GetPlayerGlobals(index, true);

				if (pPlayerGlobals->GetLives())
				{
					return false;
				}
			}
			break;

		default:
			return false;
		}
	}

	return true;
}

void PlayerAdvanceRender::ShootBullet(PlayerInfo &info, size_t nGun)
{
	IEntityDomain *pDomain = info._entity->GetDomain();

	BulletEntityType type = info._component->GetIndex()
		? BULLET_TYPE_PLAYER_1
		: BULLET_TYPE_PLAYER_0;

	if (info._component->HasPowerup(POWERUP_TYPE_HOMING_SHOT))
	{
		type = info._component->GetIndex()
			? BULLET_TYPE_HOMING_PLAYER_1
			: BULLET_TYPE_HOMING_PLAYER_0;
	}
	else if (info._component->HasPowerup(POWERUP_TYPE_SPEED_BOOST))
	{
		type = info._component->GetIndex()
			? BULLET_TYPE_FAST_PLAYER_1
			: BULLET_TYPE_FAST_PLAYER_0;
	}
	else if (info._component->HasPowerup(POWERUP_TYPE_SPREAD_SHOT))
	{
		type = info._component->GetIndex()
			? BULLET_TYPE_SPREAD_PLAYER_1
			: BULLET_TYPE_SPREAD_PLAYER_0;
	}
	else if (info._component->HasPowerup(POWERUP_TYPE_PUSH_SHOT))
	{
		type = info._component->GetIndex()
			? BULLET_TYPE_PUSH_PLAYER_1
			: BULLET_TYPE_PUSH_PLAYER_0;
	}

	ff::ComPtr<IEntity> pBulletEntity;
	ff::PointFloat velocity(0, -Globals::GetBulletSpeed(type, DIFFICULTY_NORMAL, 0));

	ff::PointFloat pos = info._entity->GetComponent<IPositionComponent>()->GetPos();
	pos += Globals::GetPlayerBulletOffset();

	if (nGun == 1)
	{
		pos.Offset(-32, 8);

		if (type == BULLET_TYPE_SPREAD_PLAYER_0 || type == BULLET_TYPE_SPREAD_PLAYER_1)
		{
			float angle = (90 - 20) / 180.0f * ff::PI_F;

			velocity.x = velocity.y * cos(angle);
			velocity.y = velocity.y * sin(angle);
		}
	}
	else if (nGun == 2)
	{
		pos.Offset(32, 8);

		if (type == BULLET_TYPE_SPREAD_PLAYER_0 || type == BULLET_TYPE_SPREAD_PLAYER_1)
		{
			float angle = (90 + 20) / 180.0f * ff::PI_F;

			velocity.x = velocity.y * cos(angle);
			velocity.y = velocity.y * sin(angle);
		}
	}

	if (_factoryService->CreateBulletEntity(type, pos, velocity, &pBulletEntity))
	{
		info._component->AddBullet(pBulletEntity);
		info._bodyFrame = 0;

		IStateComponent *pState = info._entity->GetComponent<IStateComponent>();
		pState->SetEnumState<PlayerState>(PS_SHOOTING);

		BulletEventArgs eventArgs;
		eventArgs._pSource = info._entity;
		eventArgs._pBullet = pBulletEntity;

		pDomain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_BULLET_SHOOT, &eventArgs);
	}
}

void PlayerAdvanceRender::HandleCollision(IEntity *pPlayerEntity, IEntity *otherEntity)
{
	PlayerComponent* pPlayer = pPlayerEntity->GetComponent<PlayerComponent>();
	IStateComponent* pPlayerState = pPlayerEntity->GetComponent<IStateComponent>();
	PowerupComponent* pPowerup = otherEntity->GetComponent<PowerupComponent>();
	PlayerState state = pPlayerState->GetEnumState<PlayerState>();

	if (pPlayer && IsPlayerKillable(state, pPlayer->GetIndex(), pPowerup != nullptr))
	{
		BulletComponent* pBullet = otherEntity->GetComponent<BulletComponent>();
		InvaderComponent* pInvader = otherEntity->GetComponent<InvaderComponent>();

		if ((pBullet && pBullet->IsInvader()) || pInvader)
		{
			IPositionComponent* pPlayerPos = pPlayerEntity->GetComponent<IPositionComponent>();
			IPositionComponent* pOtherPos = otherEntity->GetComponent<IPositionComponent>();

			// only check if the very tip of the bullet touches the player
			if (!pBullet || pPlayerPos->GetBounds().PointInRect(pOtherPos->GetPos()))
			{
				_factoryService->CreatePlayerExplosion(pPlayerEntity, otherEntity);
				pPlayerState->SetEnumState<PlayerState>(state = PS_DYING);

				for (size_t i = 0; i < _players.Size(); i++)
				{
					PlayerInfo &info = _players[i];

					if (info._entity == pPlayerEntity)
					{
						info._bodyFrame = 0;
						info._component->SetPowerup(POWERUP_TYPE_NONE, 0);

						float hitOffset = (pOtherPos->GetPos().x - pPlayerPos->GetPos().x);

						if (hitOffset < -20)
						{
							info._hitSide = PLAYER_HIT_LEFT;
						}
						else if (hitOffset > 20)
						{
							info._hitSide = PLAYER_HIT_RIGHT;
						}
						else
						{
							info._hitSide = PLAYER_HIT_MIDDLE;
						}
					}
				}

				if (pBullet)
				{
					BulletEventArgs eventArgs;
					eventArgs._pBullet = otherEntity;
					eventArgs._pSource = pPlayerEntity;

					pPlayerEntity->TriggerEvent(ENTITY_EVENT_BULLET_HIT, &eventArgs);
				}
				else if (pInvader)
				{
					_factoryService->CreateInvaderExplosion(otherEntity);

					CollisionEventArgs eventArgs;
					eventArgs._pOther = otherEntity;

					pPlayerEntity->TriggerEvent(ENTITY_EVENT_INVADER_HIT, &eventArgs);
				}

				otherEntity->TriggerEvent(ENTITY_EVENT_DIED);
			}
		}
		else if (pPowerup)
		{
			bool bCollect = true;
			IPositionComponent *pPowerupPos = otherEntity->GetComponent<IPositionComponent>();

			// Check for multiple collisions (prefer to use the player that actually shot the saucer)
			for (size_t i = 0; i < _players.Size(); i++)
			{
				PlayerInfo& info = _players[i];
				IPositionComponent* pPlayerPos = info._entity->GetComponent<IPositionComponent>();

				if (info._component->GetIndex() != pPlayer->GetIndex() &&
					info._component->GetIndex() == pPowerup->GetPlayerIndex() &&
					pPlayerPos->HitTest(pPowerupPos))
				{
					bCollect = false;
				}
			}

			if (bCollect)
			{
				PowerupEventArgs eventArgs;
				eventArgs._pPowerup = otherEntity;

				pPlayerEntity->TriggerEvent(ENTITY_EVENT_COLLECT_POWERUP, &eventArgs);
				eventArgs._pPowerup->TriggerEvent(ENTITY_EVENT_DIED);

				ff::ComPtr<IEntity> pPowerupCollect;
				_factoryService->CreatePowerupCollect(eventArgs._pPowerup, &pPowerupCollect);
			}
		}
	}
}

void PlayerAdvanceRender::OnEntityApplyForce(IEntity *entity, const ff::PointFloat &force)
{
	IPositionComponent *pPos = entity->GetComponent<IPositionComponent>();

	ff::PointFloat vel(force * Globals::GetPlayerMaxSpeed());
	ff::PointFloat oldPos = pPos->GetPos();
	ff::PointFloat pos = oldPos + vel;
	ff::RectFloat bounds = Globals::GetPlayerMoveArea();

	pos.x = std::max(pos.x, bounds.left);
	pos.x = std::min(pos.x, bounds.right);

	pos.y = std::max(pos.y, bounds.top);
	pos.y = std::min(pos.y, bounds.bottom);

	pPos->SetPos(pos);
	pPos->SetVelocity(pos - oldPos);
}

void PlayerAdvanceRender::OnAddScore(const ScoreEventArgs &eventArgs)
{
	if (eventArgs._nAddScore)
	{
		PlayerGlobals* pPlayerGlobals = nullptr;
		PlayerInfo* pInfo = nullptr;
		bool bAnyActive = false;

		for (size_t i = 0; i < _players.Size(); i++)
		{
			PlayerInfo &info = _players[i];

			if (info._component->GetIndex() == eventArgs._nPlayer)
			{
				pInfo = &info;
				pPlayerGlobals = _globalPlayers->GetPlayerGlobals(eventArgs._nPlayer, true);
			}

			if (!bAnyActive)
			{
				bAnyActive = _globalPlayers->GetPlayerGlobals(info._component->GetIndex())->IsActive();
			}
		}

		if (pPlayerGlobals && pInfo && bAnyActive)
		{
			size_t nOldLives = pPlayerGlobals->GetLives();
			pPlayerGlobals->AddScore(eventArgs._nAddScore);

			if (pPlayerGlobals->GetLives() > nOldLives)
			{
				pInfo->_entity->TriggerEvent(ENTITY_EVENT_GOT_FREE_LIFE);
			}
		}
	}
}

void PlayerAdvanceRender::OnCollectPowerup(IEntity *entity, const PowerupEventArgs &eventArgs)
{
	PlayerComponent *pPlayer = entity->GetComponent<PlayerComponent>();

	if (pPlayer)
	{
		PowerupComponent *pPowerup = eventArgs._pPowerup->GetComponent<PowerupComponent>();
		assertRet(pPowerup);

		switch (pPowerup->GetType())
		{
		case POWERUP_TYPE_BONUS_POINTS:
			{
				ScoreEventArgs scoreEventArgs;
				scoreEventArgs._nPlayer = pPlayer->GetIndex();
				scoreEventArgs._nAddScore = Globals::GetBonusPoints();

				entity->GetDomain()->GetEntityManager()->TriggerEvent(ENTITY_EVENT_ADD_SCORE, &scoreEventArgs);
			}
			break;
		}

		size_t nLevel = Globals::GetCurrentLevel(entity);

		Difficulty diff = Globals::GetDifficulty(entity);
		size_t nBonusFrames = (size_t)(Globals::GetBonusTime(pPowerup->GetType(), diff, nLevel) * Globals::GetAdvancesPerSecondF());

		if (nBonusFrames)
		{
			for (size_t i = 0; i < _players.Size(); i++)
			{
				if (pPlayer == _players[i]._component)
				{
					pPlayer->SetPowerup(pPowerup->GetType(), nBonusFrames);
				}
			}
		}
	}
}


void PlayerAdvanceRender::OnStateChanged(IEntity *entity, const StateEventArgs &eventArgs)
{
	PlayerComponent *pPlayer = entity->GetComponent<PlayerComponent>();

	if (pPlayer)
	{
		PlayerGlobals *pPlayerGlobals = _globalPlayers->GetPlayerGlobals(pPlayer->GetIndex());

		switch (eventArgs._nNewState)
		{
		case PS_REBUILDING:
			pPlayerGlobals->SetActive(true);
			_factoryService->CreatePlayerRebuild(entity);
			break;

		case PS_DYING:
			pPlayerGlobals->SetActive(false);
			break;

		case PS_DEAD:
			assert(!pPlayerGlobals->IsActive());

			for (size_t i = 0; i < _players.Size(); i++)
			{
				PlayerInfo &info = _players[i];

				if (info._component == pPlayer)
				{
					info._hitSide = PLAYER_HIT_NONE;
					info._bodyFrame = 0;
					info._wheelsFrame = 0;
				}
			}
			break;
		}
	}
}
