#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "components\core\PathComponent.h"
#include "components\invader\InvaderComponent.h"
#include "components\level\LevelAdvanceRender.h"
#include "components\player\PlayerAdvanceRender.h"
#include "components\player\PlayerComponent.h"
#include "components\player\TouchAdvanceRender.h"
#include "components\powerup\PowerupComponent.h"
#include "components\core\PositionComponent.h"
#include "components\core\StateComponent.h"
#include "components\graph\SpriteAnimationAdvance.h"
#include "components\graph\SpriteAnimationRender.h"
#include "components\graph\SpriteRender.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"
#include "Globals\MetroGlobals.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Graph\2D\SpriteList.h"
#include "Graph\Font\SpriteFont.h"
#include "Graph\GraphTexture.h"
#include "Module\Module.h"
#include "services\EntityFactoryService.h"
#include "services\GlobalPlayerService.h"

BEGIN_INTERFACES(EntityFactoryService)
END_INTERFACES()

EntityFactoryService::EntityFactoryService()
	: _domain(nullptr)
{
}

EntityFactoryService::~EntityFactoryService()
{
}

HRESULT EntityFactoryService::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	assertRetVal(pDomainProvider->GetDomain(), E_INVALIDARG);

	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	_domain = pDomainProvider->GetDomain();
	assertRetVal(_domain, E_FAIL);

	_bulletAnims[BULLET_TYPE_PLAYER_0].Init(L"Player Bullet 0");
	_bulletAnims[BULLET_TYPE_PLAYER_1].Init(L"Player Bullet 0");
	_bulletAnims[BULLET_TYPE_HOMING_PLAYER_0].Init(L"Player Bullet 0");
	_bulletAnims[BULLET_TYPE_HOMING_PLAYER_1].Init(L"Player Bullet 0");
	_bulletAnims[BULLET_TYPE_FAST_PLAYER_0].Init(L"Player Bullet 0");
	_bulletAnims[BULLET_TYPE_FAST_PLAYER_1].Init(L"Player Bullet 0");
	_bulletAnims[BULLET_TYPE_SPREAD_PLAYER_0].Init(L"Player Bullet 0");
	_bulletAnims[BULLET_TYPE_SPREAD_PLAYER_1].Init(L"Player Bullet 0");
	_bulletAnims[BULLET_TYPE_PUSH_PLAYER_0].Init(L"Invader Bullet 0");
	_bulletAnims[BULLET_TYPE_PUSH_PLAYER_1].Init(L"Invader Bullet 0");
	_bulletAnims[BULLET_TYPE_INVADER_LARGE].Init(L"Invader Bullet 0");
	_bulletAnims[BULLET_TYPE_INVADER_SMALL].Init(L"Invader Bullet 1");
	_bulletAnims[BULLET_TYPE_INVADER_LOSE_GAME].Init(L"Invader Bullet 1");

	_bulletHitAnims[BULLET_TYPE_PLAYER_0].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_PLAYER_1].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_HOMING_PLAYER_0].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_HOMING_PLAYER_1].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_FAST_PLAYER_0].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_FAST_PLAYER_1].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_SPREAD_PLAYER_0].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_SPREAD_PLAYER_1].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_PUSH_PLAYER_0].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_PUSH_PLAYER_1].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_INVADER_LARGE].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_INVADER_SMALL].Init(L"Bullet Explosion 0");
	_bulletHitAnims[BULLET_TYPE_INVADER_LOSE_GAME].Init(L"Bullet Explosion 0");

	_shieldHitAnims[BULLET_TYPE_PLAYER_0].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_PLAYER_1].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_HOMING_PLAYER_0].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_HOMING_PLAYER_1].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_FAST_PLAYER_0].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_FAST_PLAYER_1].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_SPREAD_PLAYER_0].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_SPREAD_PLAYER_1].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_PUSH_PLAYER_0].Init(L"Shield Explosion 1");
	_shieldHitAnims[BULLET_TYPE_PUSH_PLAYER_1].Init(L"Shield Explosion 1");
	_shieldHitAnims[BULLET_TYPE_INVADER_LARGE].Init(L"Shield Explosion 1");
	_shieldHitAnims[BULLET_TYPE_INVADER_SMALL].Init(L"Shield Explosion 0");
	_shieldHitAnims[BULLET_TYPE_INVADER_LOSE_GAME].Init(L"Shield Explosion 0");

	_smokeAnims[0].Init(L"Bullet Smoke 0");
	_smokeAnims[1].Init(L"Bullet Smoke 1");

	_dustAnims[0].Init(L"Player Dust 0");
	_dustAnims[1].Init(L"Player Dust 1");

	_invaderHitAnims[INVADER_TYPE_0].Init(L"Invader Hit 0");
	_invaderHitAnims[INVADER_TYPE_1].Init(L"Invader Hit 1");
	_invaderHitAnims[INVADER_TYPE_2].Init(L"Invader Hit 2");
	_invaderHitAnims[INVADER_TYPE_3].Init(L"Invader Hit 3");
	_invaderHitAnims[INVADER_TYPE_4].Init(L"Invader Hit 4");
	_invaderHitAnims[INVADER_TYPE_5].Init(L"Invader Hit 5");
	_invaderHitAnims[INVADER_TYPE_6].Init(L"Invader Hit 6");
	_invaderHitAnims[INVADER_TYPE_7].Init(L"Invader Hit 7");

	_invaderHitAnims[INVADER_TYPE_BONUS_0].Init(L"Invader Hit Bonus 0");
	_invaderHitAnims[INVADER_TYPE_BONUS_1].Init(L"Invader Hit Bonus 1");
	_invaderHitAnims[INVADER_TYPE_BONUS_2].Init(L"Invader Hit Bonus 2");

	_saucerAnims[SAUCER_TYPE_MULTI_SHOT].Init(L"Saucer 0");
	_saucerAnims[SAUCER_TYPE_AMMO].Init(L"Saucer 1");
	_saucerAnims[SAUCER_TYPE_HOMING_SHOT].Init(L"Saucer 2");
	_saucerAnims[SAUCER_TYPE_SHIELD].Init(L"Saucer 3");
	_saucerAnims[SAUCER_TYPE_SPEED_BOOST].Init(L"Saucer 4");
	_saucerAnims[SAUCER_TYPE_POINTS].Init(L"Saucer 5");
	_saucerAnims[SAUCER_TYPE_SPREAD_SHOT].Init(L"Saucer 6");
	_saucerAnims[SAUCER_TYPE_PUSH_SHOT].Init(L"Saucer 7");
	_saucerAnims[SAUCER_TYPE_NONE].Init(L"Saucer None");
	_saucerAnims[SAUCER_TYPE_BONUS_LEVEL].Init(L"Bonus Saucer");
	_saucerAnims[SAUCER_TYPE_BONUS_LEVEL_FAST].Init(L"Bonus Saucer");

	_powerupAnims[POWERUP_TYPE_MULTI_SHOT].Init(L"Powerup 0");
	_powerupAnims[POWERUP_TYPE_AMMO].Init(L"Powerup 1");
	_powerupAnims[POWERUP_TYPE_HOMING_SHOT].Init(L"Powerup 2");
	_powerupAnims[POWERUP_TYPE_SHIELD].Init(L"Powerup 3");
	_powerupAnims[POWERUP_TYPE_SPEED_BOOST].Init(L"Powerup 4");
	_powerupAnims[POWERUP_TYPE_BONUS_POINTS].Init(L"Powerup 5");
	_powerupAnims[POWERUP_TYPE_SPREAD_SHOT].Init(L"Powerup 6");
	_powerupAnims[POWERUP_TYPE_PUSH_SHOT].Init(L"Powerup 7");

	_pSaucerLightsAnim.Init(L"Saucer Lights");
	_playerOrbAnim.Init(L"Player Orb");

	ff::TypedResource<ff::ISpriteList> pSprites(L"Sprites");
	_pShieldSprites.Init(L"Shield Sprites");
	_font.Init(L"Classic");

	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Player Body Standing"), &_playerBody), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Player Turret Standing"), &_playerTurret), E_FAIL);
	assertRetVal(ff::CreateSpriteResource(pSprites.GetResourceValue(), ff::String(L"Player Wheels Standing"), &_playerWheels), E_FAIL);

	return __super::_Construct(unkOuter);
}

static ff::RectFloat GetBulletPosToBox(BulletEntityType type)
{
	switch (type)
	{
	default:
		assert(false);
		__fallthrough;

	case BULLET_TYPE_PLAYER_0:
	case BULLET_TYPE_PLAYER_1:
	case BULLET_TYPE_HOMING_PLAYER_0:
	case BULLET_TYPE_HOMING_PLAYER_1:
	case BULLET_TYPE_SPREAD_PLAYER_0:
	case BULLET_TYPE_SPREAD_PLAYER_1:
		return ff::RectFloat(-16, -5, 0, 5);

	case BULLET_TYPE_FAST_PLAYER_0:
	case BULLET_TYPE_FAST_PLAYER_1:
		return ff::RectFloat(-24, -5, 0, 5);

	case BULLET_TYPE_INVADER_SMALL:
	case BULLET_TYPE_INVADER_LOSE_GAME:
		return ff::RectFloat(-10, -5, 0, 5);

	case BULLET_TYPE_INVADER_LARGE:
	case BULLET_TYPE_PUSH_PLAYER_0:
	case BULLET_TYPE_PUSH_PLAYER_1:
		return ff::RectFloat(-26, -12, -4, 12);
	}
}

bool EntityFactoryService::CreateBulletEntity(BulletEntityType type, ff::PointFloat pos, ff::PointFloat velocity, IEntity **pentity)
{
	assertRetVal(pentity, false);

	ff::ComPtr<IEntity> entity;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);

	// Init components
	{
		// hit box must be at least as long as the speed
		float speed = DirectX::XMVectorGetX(DirectX::XMVector2Length(DirectX::XMLoadFloat2((DirectX::XMFLOAT2*)&velocity)));
		ff::RectFloat posToBox = GetBulletPosToBox(type);

		if (posToBox.Width() < speed)
		{
			posToBox.left = posToBox.right - speed;
		}

		// Position
		IPositionComponent *pPos = IPositionComponent::Create(entity, pos, velocity, &posToBox, true);
		pPos->SetRotateFromVelocity();

		BulletComponent *pBullet = entity->EnsureComponent<BulletComponent>();
		pBullet->SetType(type);
	}

	// Init sprite animation
	{
		ff::ISpriteAnimation* pAnim = _bulletAnims[type].Flush();
		SpriteAnimationRender* render = SpriteAnimationRender::Create(entity, pAnim, ff::POSE_TWEEN_LINEAR_LOOP, LAYER_PRI_NORMAL - 1);
		SpriteAnimationAdvance* pAdvance = SpriteAnimationAdvance::Create(entity);

		pAdvance->SetInfiniteAnim(0, pAnim->GetFPS());
	}

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	*pentity = ff::GetAddRef(entity.Interface());
	return true;
}

bool EntityFactoryService::CreateBulletSmoke(IEntity *pBulletEntity)
{
	BulletComponent* pBulletComp = pBulletEntity->GetComponent<BulletComponent>();
	IPositionComponent* pBulletPos = pBulletEntity->GetComponent<IPositionComponent>();
	assertRetVal(pBulletComp && pBulletPos, false);

	for (size_t i = 0; i < 2 && pBulletComp->IsPlayer(); i++)
	{
		ff::ComPtr<IEntity> pSmokeEntity;
		assertRetVal(pBulletEntity->GetDomain()->GetEntityManager()->CreateEntity(&pSmokeEntity), false);

		IPositionComponent* pSmokePos = pSmokeEntity->EnsureComponent<IPositionComponent>();
		ff::ISpriteAnimation* pAnim = _smokeAnims[rand() % _countof(_smokeAnims)].Flush();
		SpriteAnimationRender* pSmokeRender = SpriteAnimationRender::Create(pSmokeEntity, pAnim, ff::POSE_TWEEN_LINEAR_LOOP, LAYER_PRI_NORMAL - 2);
		SpriteAnimationAdvance* pSmokeAdvance = SpriteAnimationAdvance::Create(pSmokeEntity);

		ff::PointFloat smokePos = pBulletPos->GetPos();
		float bulletAngle = pBulletPos->GetVelocityAngle();

		smokePos.x += -24 * cos(bulletAngle) + (rand() % 13) - 6.0f;
		smokePos.y += 24 * sin(bulletAngle) + (rand() % 9) - 4.0f;
		pSmokePos->SetPos(smokePos);

		pSmokeAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), (rand() % 8) + 20.0f);

		DirectX::XMFLOAT4 smokeColor(1, (rand() % 64 + 64) / 127.0f, (rand() % 64 + 64) / 127.0f, 1);
		pSmokeRender->GetPos()._color = smokeColor;

		float smokeScale = (rand() % 10 + 11) / 10.0f * 1.5f;
		pSmokeRender->GetPos()._scale = ff::PointFloat(smokeScale, smokeScale);

		pSmokeEntity->TriggerEvent(ENTITY_EVENT_BORN);

		return true;
	}

	return false;
}

bool EntityFactoryService::CreateBulletExplosion(IEntity *pBulletEntity)
{
	BulletComponent* pBullet = pBulletEntity->GetComponent<BulletComponent>();
	IPositionComponent* pBulletPos = pBulletEntity->GetComponent<IPositionComponent>();
	assertRetVal(pBullet && pBulletPos, false);

	ff::ComPtr<IEntity> pHitEntity;
	assertRetVal(pBulletEntity->GetDomain()->GetEntityManager()->CreateEntity(&pHitEntity), false);

	IPositionComponent* pHitPos = pHitEntity->EnsureComponent<IPositionComponent>();
	ff::ISpriteAnimation* pAnim = _bulletHitAnims[pBullet->GetType() % _countof(_bulletHitAnims)].Flush();
	SpriteAnimationRender* pHitRender = SpriteAnimationRender::Create(pHitEntity, pAnim, ff::POSE_TWEEN_LINEAR_LOOP, LAYER_PRI_NORMAL + 1);
	SpriteAnimationAdvance* pHitAdvance = SpriteAnimationAdvance::Create(pHitEntity);

	pHitPos->SetPos(pBulletPos->GetPos());

	pHitAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), pAnim->GetFPS());

	pHitEntity->TriggerEvent(ENTITY_EVENT_BORN);

	return true;
}

static ff::RectFloat GetInvaderPosToBox(InvaderEntityType type)
{
	switch (type)
	{
	default:
	case INVADER_TYPE_0: // brown
	case INVADER_TYPE_BONUS_0:
		return ff::RectFloat(-32, -32, 32, 32);

	case INVADER_TYPE_1: // blue
	case INVADER_TYPE_BONUS_1:
		return ff::RectFloat(-28, -28, 28, 28);

	case INVADER_TYPE_2: // green
	case INVADER_TYPE_BONUS_2:
		return ff::RectFloat(-22, -32, 22, 22);

	case INVADER_TYPE_3: // yellow
		return ff::RectFloat(-26, -32, 26, 10);

	case INVADER_TYPE_4: // blue
		return ff::RectFloat(-32, -32, 32, 32);

	case INVADER_TYPE_5: // pink
		return ff::RectFloat(-26, -26, 26, 26);

	case INVADER_TYPE_6: // green
		return ff::RectFloat(-12, 0, 12, 22);

	case INVADER_TYPE_7: // yellow
		return ff::RectFloat(-34, -24, 34, 22);
	}
}

bool EntityFactoryService::CreateInvaderEntity(InvaderEntityType type, ff::PointInt cell, ff::PointFloat pos, ff::PointFloat vel, IEntity **pentity)
{
	assertRetVal(pentity, false);

	ff::ComPtr<IEntity> entity;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);

	// Init components
	{
		// Position
		IPositionComponent::Create(entity, pos, vel, &GetInvaderPosToBox(type), true);

		InvaderComponent *pInvader = entity->EnsureComponent<InvaderComponent>();
		pInvader->SetType(type);
		pInvader->SetCell(cell);
	}

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	*pentity = ff::GetAddRef(entity.Interface());
	return true;
}

bool EntityFactoryService::CreateInvaderExplosion(IEntity *pInvaderEntity)
{
	InvaderComponent* pInvader = pInvaderEntity->GetComponent<InvaderComponent>();
	IPositionComponent* pInvaderPos = pInvaderEntity->GetComponent<IPositionComponent>();
	assertRetVal(pInvader && pInvaderPos, false);

	ff::ComPtr<IEntity> pHitEntity;
	assertRetVal(pInvaderEntity->GetDomain()->GetEntityManager()->CreateEntity(&pHitEntity), false);

	IPositionComponent* pHitPos = pHitEntity->EnsureComponent<IPositionComponent>();
	ff::ISpriteAnimation* pAnim = _invaderHitAnims[pInvader->GetType() % _countof(_invaderHitAnims)].Flush();
	SpriteAnimationRender* pHitRender = SpriteAnimationRender::Create(pHitEntity, pAnim, ff::POSE_TWEEN_LINEAR_LOOP, LAYER_PRI_NORMAL + 1);
	SpriteAnimationAdvance* pHitAdvance = SpriteAnimationAdvance::Create(pHitEntity);

	pHitPos->SetPos(pInvaderPos->GetPos());

	pHitAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), pAnim->GetFPS());

	pHitEntity->TriggerEvent(ENTITY_EVENT_BORN);

	return true;
}

bool EntityFactoryService::CreatePlayerEntity(size_t index, IEntity **pentity)
{
	assertRetVal(pentity && index < 2, false);

	ff::ComPtr<GlobalPlayerService> pGlobalPlayers;
	assertRetVal(GetService(_domain, &pGlobalPlayers), false);

	PlayerGlobals* pPlayerGlobals = pGlobalPlayers->GetPlayerGlobals(index);
	assertRetVal(pPlayerGlobals, false);

	ff::ComPtr<IEntity> entity;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);

	// Position
	{
		ff::ComPtr<GlobalPlayerService> pPlayers;
		assertRetVal(GetService(_domain, &pPlayers), false);

		ff::RectFloat moveRect = Globals::GetPlayerMoveArea();
		ff::PointFloat pos = moveRect.Center();

		if (pPlayers->GetGameMode() == GAME_MODE_COOP)
		{
			pos.x += index
				? (moveRect.left - pos.x) / 2
				: (moveRect.right - pos.x) / 2;
		}

		IPositionComponent::Create(entity, pos, ff::PointFloat(0, 0), &ff::RectFloat(-38, -54, 38, -8), true);
	}

	// Init components
	{
		entity->EnsureComponent<IStateComponent>();

		if (index == 0 || pGlobalPlayers->GetGameMode() != GAME_MODE_COOP)
		{
			ff::ComPtr<TouchAdvanceRender, IAdvanceComponent> pTouchAR;
			entity->CreateComponent<TouchAdvanceRender>(&pTouchAR);
			entity->AddComponent<IAdvanceComponent>(pTouchAR);
			entity->AddComponent<I2dRenderComponent>(pTouchAR);
			entity->AddComponent<IExternalPlayerControl>(pTouchAR);
		}

		PlayerComponent* pPlayer = entity->EnsureComponent<PlayerComponent>();
		assertRetVal(pPlayer->Init(entity, index), false);
	}

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	*pentity = ff::GetAddRef(entity.Interface());
	return true;
}

bool EntityFactoryService::CreatePlayerDust(IEntity *pPlayerEntity)
{
	IPositionComponent* pPlayerPos = pPlayerEntity->GetComponent<IPositionComponent>();
	assertRetVal(pPlayerPos, false);

	ff::PointFloat playerVel = pPlayerPos->GetVelocity();

	if (playerVel.x != 0)
	{
		ff::ComPtr<IEntity> pDustEntity;
		assertRetVal(pPlayerEntity->GetDomain()->GetEntityManager()->CreateEntity(&pDustEntity), false);

		IPositionComponent* pDustPos = pDustEntity->EnsureComponent<IPositionComponent>();
		ff::ISpriteAnimation* pAnim = _dustAnims[rand() % _countof(_dustAnims)].Flush();
		SpriteAnimationRender* pDustRender = SpriteAnimationRender::Create(pDustEntity, pAnim, ff::POSE_TWEEN_LINEAR_LOOP, LAYER_PRI_NORMAL + 1);
		SpriteAnimationAdvance* pDustAdvance = SpriteAnimationAdvance::Create(pDustEntity);

		ff::PointFloat dustPos = pPlayerPos->GetPos();
		float dir = (playerVel.x > 0) ? 1.0f : -1.0f;

		dustPos.x += -25.0f * dir + (rand() % 9) - 4.0f;
		dustPos.y -= (rand() % 4);
		pDustPos->SetPos(dustPos);

		pDustAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), (rand() % 8) + 16.0f);

		DirectX::XMFLOAT4 dustColor(1, (rand() % 64 + 64) / 127.0f, (rand() % 64 + 64) / 127.0f, 1);
		pDustRender->GetPos()._color = dustColor;

		float dustScale = (rand() % 10 + 11) / 10.0f * 1.5f;
		pDustRender->GetPos()._scale = ff::PointFloat(dustScale * dir, dustScale);

		pDustEntity->TriggerEvent(ENTITY_EVENT_BORN);

		return true;
	}

	return false;
}

bool EntityFactoryService::CreatePlayerExplosion(IEntity *pPlayerEntity, IEntity *otherEntity)
{
	if (otherEntity->GetComponent<BulletComponent>())
	{
		assertRetVal(CreateBulletExplosion(otherEntity), false);
	}
	else if (otherEntity->GetComponent<InvaderComponent>())
	{
		assertRetVal(CreateInvaderExplosion(otherEntity), false);
	}
	else
	{
		return false;
	}

	return true;
}

bool EntityFactoryService::CreatePlayerDyingSmoke(IEntity *pPlayerEntity, PlayerHitSide hitSide)
{
	IPositionComponent* pPlayerPos = pPlayerEntity->GetComponent<IPositionComponent>();
	assertRetVal(pPlayerPos, false);

	ff::ComPtr<IEntity> pDustEntity;
	assertRetVal(pPlayerEntity->GetDomain()->GetEntityManager()->CreateEntity(&pDustEntity), false);

	IPositionComponent* pDustPos = pDustEntity->EnsureComponent<IPositionComponent>();
	ff::ISpriteAnimation* pAnim = _dustAnims[rand() % _countof(_dustAnims)].Flush();
	SpriteAnimationRender* pDustRender = SpriteAnimationRender::Create(pDustEntity, pAnim, ff::POSE_TWEEN_LINEAR_LOOP, LAYER_PRI_NORMAL + 1);
	SpriteAnimationAdvance* pDustAdvance = SpriteAnimationAdvance::Create(pDustEntity);

	ff::PointFloat playerPos = pPlayerPos->GetPos();
	ff::PointFloat dustPos = playerPos;
	float dustAlpha = (rand() % 101) / 100.0f;

	dustPos.x += -50 + (rand() % 101);
	dustPos.y -= 22 + 75 * (1 - dustAlpha);

	if ((hitSide == PLAYER_HIT_LEFT && dustPos.x > playerPos.x) ||
		(hitSide == PLAYER_HIT_RIGHT && dustPos.x < playerPos.x))
	{
		dustPos.x += (playerPos.x - dustPos.x) * 2;
	}

	pDustPos->SetPos(dustPos);

	pDustAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), (rand() % 8) + 16.0f);

	DirectX::XMFLOAT4 dustColor((rand() % 101) / 100.0f * .5f + .5f, .25f, .25f, dustAlpha);
	pDustRender->GetPos()._color = dustColor;

	float dustScale = (rand() % 10 + 11) / 10.0f * 3.0f;
	pDustRender->GetPos()._scale = ff::PointFloat(dustScale, dustScale);

	pDustEntity->TriggerEvent(ENTITY_EVENT_BORN);

	return true;
}

static float GetPlayerStartingX(bool isCoop, size_t index)
{
	float x = Globals::GetLevelGroundRect().Center().x;

	if (isCoop)
	{
		x += index ? -50.0f : 50.0f;
	}

	return x;
}

bool EntityFactoryService::CreatePlayerRebuild(IEntity *pPlayerEntity)
{
	ff::ComPtr<GlobalPlayerService> pGlobalPlayers;
	assertRetVal(GetService(_domain, &pGlobalPlayers), false);

	bool isCoop = (pGlobalPlayers->GetGameMode() == GAME_MODE_COOP);
	PlayerComponent* pPlayer = pPlayerEntity->GetComponent<PlayerComponent>();
	PlayerGlobals* pPlayerGlobals = pGlobalPlayers->GetPlayerGlobals(pPlayer->GetIndex(), isCoop);
	ThisApplication* pApp = ThisApplication::Get(_domain);

	DirectX::XMFLOAT4 wheelsColor = Globals::GetPlayerWheelsColor(pPlayer->GetIndex());
	DirectX::XMFLOAT4 bodyColor = Globals::GetPlayerBodyColor(pPlayer->GetIndex());
	ff::PointFloat startPos = Globals::GetPlayerLifePos(isCoop ? 0 : pPlayer->GetIndex(), pPlayerGlobals->GetLives());
	ff::PointFloat destPos1 = ff::PointFloat(GetPlayerStartingX(isCoop, pPlayer->GetIndex()), startPos.y);
	ff::PointFloat destPos2 = ff::PointFloat(destPos1.x, Globals::GetLevelGroundRect().top);

	pPlayerEntity->GetComponent<IPositionComponent>()->SetPos(destPos2);

	ff::ComPtr<IEntity> entity;
	ff::ComPtr<ff::ISpriteAnimation> pAnim;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);
	assertRetVal(IPositionComponent::Create(entity, ff::PointFloat(0, 0), ff::PointFloat(0, 0), nullptr, false), false);
	assertRetVal(ff::CreateSpriteAnimation(ff::MetroGlobals::Get()->GetGraph(), &pAnim), false);

	pAnim->SetLastFrame(3);
	pAnim->SetFPS(1);

	pAnim->SetSprite(0, 0, 0, _playerWheels);
	pAnim->SetSprite(0, 0, 1, _playerBody);
	pAnim->SetSprite(0, 0, 2, _playerTurret);

	pAnim->SetSprite(3, 0, 0, _playerWheels);
	pAnim->SetSprite(3, 0, 1, _playerBody);
	pAnim->SetSprite(3, 0, 2, _playerTurret);

	pAnim->SetColor(0, 0, 0, wheelsColor);
	pAnim->SetColor(0, 0, 1, bodyColor);
	pAnim->SetColor(0, 0, 2, bodyColor);

	pAnim->SetColor(3, 0, 0, wheelsColor);
	pAnim->SetColor(3, 0, 1, bodyColor);
	pAnim->SetColor(3, 0, 2, bodyColor);

	pAnim->SetOffset(0, 0, startPos);
	pAnim->SetOffset(2, 0, destPos1);
	pAnim->SetOffset(3, 0, destPos2);

	pAnim->SetScale(0, 0, ff::PointFloat(0.75f, 0.75f));
	pAnim->SetScale(3, 0, ff::PointFloat(1, 1));

	SpriteAnimationRender::Create(entity, pAnim, ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_HIGH);
	SpriteAnimationAdvance *pAdvance = SpriteAnimationAdvance::Create(entity);

	pAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), pAnim->GetFPS());

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	return true;
}

bool EntityFactoryService::CreateShieldEntity(ShieldEntityType type, ff::PointFloat pos, IEntity **pentity)
{
	assertRetVal(pentity, false);

	assertRetVal(InitSprites(), false);

	ff::ComPtr<IEntity> entity;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);

	// Init components
	{
		// Position
		ff::RectFloat posToBox(ff::PointFloat(0, 0), Globals::GetShieldSize());
		IPositionComponent::Create(entity, pos, ff::PointFloat(0, 0), &posToBox, true);

		ShieldComponent *pShield = entity->EnsureComponent<ShieldComponent>();
		pShield->SetType(type);
	}

	// Init sprite
	{
		Difficulty diff = Globals::GetDifficulty(_domain);
		size_t nInvaderLevel = Globals::IsEasyDifficulty(diff) ? 0 : Globals::GetInvaderLevel(Globals::GetCurrentLevel(_domain));
		size_t nShieldIndex = std::min(_countof(_shields[type]) - 1, nInvaderLevel / 2);
		ff::ISprite* pOriginalSprite = _shields[type][nShieldIndex];
		ff::IGraphTexture* pOriginalTexture = pOriginalSprite->GetSpriteData()._texture;
		ff::RectInt texRect = pOriginalSprite->GetSpriteData().GetTextureRect();

		ff::ComPtr<ff::IGraphTexture> pTexture;
		assertRetVal(ff::CreateGraphTexture(pOriginalTexture->GetDevice(), texRect.Size(), DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 0, &pTexture), false);

		assertRetVal(pOriginalSprite->GetSpriteData().CopyTo(pTexture, ff::PointInt(0, 0)), false);

		ff::ComPtr<ff::ISprite> pSprite;
		assertRetVal(ff::CreateSprite(pTexture, &pSprite), false);

		SpriteRender *pSpriteRender = SpriteRender::Create(entity, pSprite, LAYER_PRI_NORMAL - 3);
		assertRetVal(pSpriteRender, false);

		pSpriteRender->GetPos()._color = DirectX::XMFLOAT4(0.8706f, 0.8706f, 1, 1);

	}

	*pentity = ff::GetAddRef(entity.Interface());
	return true;
}

bool EntityFactoryService::CreateShieldExplosion(ShieldEntityType type, IEntity *pBulletEntity)
{
	BulletComponent* pBullet = pBulletEntity->GetComponent<BulletComponent>();
	IPositionComponent* pBulletPos = pBulletEntity->GetComponent<IPositionComponent>();
	assertRetVal(pBullet && pBulletPos, false);

	ff::ComPtr<IEntity> pHitEntity;
	assertRetVal(pBulletEntity->GetDomain()->GetEntityManager()->CreateEntity(&pHitEntity), false);

	IPositionComponent* pHitPos = pHitEntity->EnsureComponent<IPositionComponent>();
	ff::ISpriteAnimation* pAnim = _shieldHitAnims[pBullet->GetType()].Flush();
	SpriteAnimationRender* pHitRender = SpriteAnimationRender::Create(pHitEntity, pAnim, ff::POSE_TWEEN_LINEAR_LOOP, LAYER_PRI_NORMAL + 1);
	SpriteAnimationAdvance* pHitAdvance = SpriteAnimationAdvance::Create(pHitEntity);

	pHitPos->SetPos(pBulletPos->GetPos());
	pHitPos->SetRotate(pBulletPos->GetRotate());

	pHitAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), pAnim->GetFPS());

	pHitEntity->TriggerEvent(ENTITY_EVENT_BORN);

	return true;
}

bool EntityFactoryService::CreateSaucerEntity(SaucerEntityType type, ff::PointFloat pos, ff::PointFloat velocity, IEntity **pentity)
{
	assertRetVal(pentity, false);

	assertRetVal(InitSprites(), false);

	ff::ComPtr<IEntity> entity;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);

	// Init components
	{
		// Position
		IPositionComponent *pPos = IPositionComponent::Create(entity, pos, velocity, &ff::RectFloat(-75, -30, 75, 14), true);
		pPos->SetScale(pPos->GetScale() * ff::PointFloat(velocity.x < 0 ? -1.0f : 1.0f, 1));

		SaucerComponent *pSaucer = entity->EnsureComponent<SaucerComponent>();
		pSaucer->SetType(type);
	}

	// Init animation
	{
		ff::ISpriteAnimation* pAnim = _saucerAnims[type].Flush();
		SpriteAnimationRender* pAnimRender = SpriteAnimationRender::Create(entity, pAnim, ff::POSE_TWEEN_LINEAR_LOOP);
		SpriteAnimationAdvance* pAnimAdvance = SpriteAnimationAdvance::Create(entity);

		pAnimRender = SpriteAnimationRender::Create(entity, _pSaucerLightsAnim.Flush(), ff::POSE_TWEEN_LINEAR_LOOP, LAYER_PRI_NORMAL + 1);

		pAnimAdvance->SetInfiniteAnim(0, _pSaucerLightsAnim.Flush()->GetFPS());
	}

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	*pentity = ff::GetAddRef(entity.Interface());
	return true;
}

bool EntityFactoryService::CreateSaucerExplosion(IEntity *pSaucerEntity, IEntity *pBulletEntity)
{
	return CreateBulletExplosion(pBulletEntity);
}

bool EntityFactoryService::CreatePoints(
	IEntity* entity,
	size_t nPoints,
	float timeScale,
	ff::PointFloat offset,
	const DirectX::XMFLOAT4* pColor)
{
	ThisApplication *pApp = ThisApplication::Get(entity);
	ff::MetroGlobals *globals = ff::MetroGlobals::Get();

	ff::ComPtr<ff::ISprite> pSprite;
	ff::ComPtr<ff::ISpriteAnimation> pAnim;

	if (ff::CreateSpriteFromText(globals->Get2dRender(), globals->Get2dEffect(), _font.Flush(), ff::String::format_new(L"%lu", nPoints), &pSprite) &&
		ff::CreateSpriteAnimation(globals->GetGraph(), &pAnim))
	{
		pAnim->SetLastFrame(2);
		pAnim->SetFPS(std::min(std::max(0.0625f, timeScale), 10.0f));

		pAnim->SetSprite(0, 0, 0, pSprite);
		pAnim->SetSprite(2, 0, 0, pSprite);

		float xOffset = pSprite->GetSpriteData().GetTextureRect().Size().x / 2.0f;
		IPositionComponent* pPos = entity->GetComponent<IPositionComponent>();

		pAnim->SetOffset(0, 0, pPos->GetPos() + ff::PointFloat(-xOffset, -32.0f) + offset);
		pAnim->SetOffset(2, 0, pPos->GetPos() + ff::PointFloat(-xOffset, -64.0f) + offset);

		pAnim->SetColor(0, 0, 0, pColor ? *pColor : ff::GetColorWhite());
		pAnim->SetColor(1, 0, 0, pColor ? *pColor : ff::GetColorWhite());
		pAnim->SetColor(2, 0, 0, ff::GetColorNone());

		ff::ComPtr<IEntity> pPointsEntity;
		entity->GetDomain()->GetEntityManager()->CreateEntity(&pPointsEntity);

		SpriteAnimationRender::Create(pPointsEntity, pAnim, ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_NORMAL + 2);
		SpriteAnimationAdvance *pAdvance = SpriteAnimationAdvance::Create(pPointsEntity);

		pAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), pAnim->GetFPS());

		pPointsEntity->TriggerEvent(ENTITY_EVENT_BORN);

		return true;
	}

	return false;
}

bool EntityFactoryService::CreatePowerup(PoweruentityType type, size_t nPlayerIndex, ff::PointFloat pos, ff::PointFloat velocity, IEntity **pentity)
{
	assertRetVal(pentity, false);

	ff::ComPtr<IEntity> entity;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);

	// Init components
	{
		// Position
		IPositionComponent::Create(entity, pos, velocity, &ff::RectFloat(-25, -25, 25, 25), true);

		PowerupComponent *pPowerup = entity->EnsureComponent<PowerupComponent>();
		pPowerup->SetType(type);
		pPowerup->SetPlayerIndex(nPlayerIndex);
	}

	// Init animation
	{
		ff::ISpriteAnimation* pAnim = _powerupAnims[type].Flush();
		SpriteAnimationRender* pAnimRender = SpriteAnimationRender::Create(entity, pAnim, ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_NORMAL + 1);
		SpriteAnimationAdvance* pAnimAdvance = SpriteAnimationAdvance::Create(entity);

		pAnimAdvance->SetLoopingAnim(0, pAnim->GetLastFrame(), pAnim->GetFPS());
	}

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	*pentity = ff::GetAddRef(entity.Interface());
	return true;
}

bool EntityFactoryService::CreatePowerupCollect(IEntity *pPoweruentity, IEntity **pentity)
{
	assertRetVal(pentity, false);

	PowerupComponent* pPowerup = pPoweruentity->GetComponent<PowerupComponent>();
	IPositionComponent* pPos = pPoweruentity->GetComponent<IPositionComponent>();
	assertRetVal(pPowerup && pPos, false);

	ff::ComPtr<IEntity> entity;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);

	ff::String szText;
	switch (pPowerup->GetType())
	{
	case POWERUP_TYPE_MULTI_SHOT:
		szText = ff::GetThisModule().GetString(Globals::IDS_POWERUP_MULTI_SHOT);
		break;

	case POWERUP_TYPE_AMMO:
		szText = ff::GetThisModule().GetString(Globals::IDS_POWERUP_AMMO);
		break;

	case POWERUP_TYPE_HOMING_SHOT:
		szText = ff::GetThisModule().GetString(Globals::IDS_POWERUP_HOMING_SHOT);
		break;

	case POWERUP_TYPE_SHIELD:
		szText = ff::GetThisModule().GetString(Globals::IDS_POWERUP_SHIELD);
		break;

	case POWERUP_TYPE_SPEED_BOOST:
		szText = ff::GetThisModule().GetString(Globals::IDS_POWERUP_SPEED_BOOST);
		break;

	case POWERUP_TYPE_BONUS_POINTS:
		szText = L"1000";
		break;

	case POWERUP_TYPE_SPREAD_SHOT:
		szText = ff::GetThisModule().GetString(Globals::IDS_POWERUP_SPREAD_SHOT);
		break;

	case POWERUP_TYPE_PUSH_SHOT:
		szText = ff::GetThisModule().GetString(Globals::IDS_POWERUP_PUSH_SHOT);
		break;

	default:
		assertRetVal(false, false);
	}

	ThisApplication* pApp = ThisApplication::Get(_domain);
	ff::MetroGlobals *globals = ff::MetroGlobals::Get();

	ff::ComPtr<ff::ISprite> pSprite;
	ff::ComPtr<ff::ISpriteAnimation> pAnim;

	if (ff::CreateSpriteFromText(globals->Get2dRender(), globals->Get2dEffect(), _font.Flush(), szText, &pSprite) &&
		ff::CreateSpriteAnimation(globals->GetGraph(), &pAnim))
	{
		pAnim->SetLastFrame(2);
		pAnim->SetFPS(1);

		pAnim->SetSprite(0, 0, 0, pSprite);
		pAnim->SetSprite(2, 0, 0, pSprite);

		float xOffset = pSprite->GetSpriteData().GetTextureRect().Size().x / 2.0f;

		pAnim->SetOffset(0, 0, pPos->GetPos() + ff::PointFloat(-xOffset, -32.0f));
		pAnim->SetOffset(2, 0, pPos->GetPos() + ff::PointFloat(-xOffset, -64.0f));

		pAnim->SetColor(0, 0, 0, ff::GetColorWhite());
		pAnim->SetColor(1, 0, 0, ff::GetColorWhite());
		pAnim->SetColor(2, 0, 0, ff::GetColorNone());

		SpriteAnimationRender::Create(entity, pAnim, ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_NORMAL + 2);
		SpriteAnimationAdvance *pAdvance = SpriteAnimationAdvance::Create(entity);

		pAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), pAnim->GetFPS());
	}

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	*pentity = ff::GetAddRef(entity.Interface());
	return true;
}

bool EntityFactoryService::CreateFreeLifeIndicator(IEntityDomain *pDomain, ff::PointFloat pos)
{
	ff::ComPtr<IEntity> entity;
	assertRetVal(pDomain->GetEntityManager()->CreateEntity(&entity), false);

	IPositionComponent::Create(entity, pos, ff::PointFloat(0, 0), nullptr, false);

	ff::ISpriteAnimation* pAnim = _playerOrbAnim.Flush();
	SpriteAnimationRender* render = SpriteAnimationRender::Create(entity, pAnim, ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_HIGH + 1);
	SpriteAnimationAdvance* pAdvance = SpriteAnimationAdvance::Create(entity);

	pAdvance->SetOneTimeAnim(0, pAnim->GetLastFrame(), pAnim->GetFPS());

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	return true;
}

bool EntityFactoryService::CreateLevelEntity(Difficulty difficulty, size_t nLevel, IEntity **pentity)
{
	assertRetVal(pentity, false);

	ff::ComPtr<IEntity> entity;
	assertRetVal(_domain->GetEntityManager()->CreateEntity(&entity), false);

	// Advance and render level
	{
		ff::ComPtr<LevelAdvanceRender, IAdvanceComponent> pLevelAR;
		assertRetVal(_domain->GetComponentManager()->CreateComponent<LevelAdvanceRender>(nullptr, &pLevelAR), false);

		entity->AddComponent<IAdvanceComponent>(pLevelAR);
		entity->AddComponent<I2dRenderComponent>(pLevelAR);
	}

	if (Globals::IsSaucerBonusLevel(difficulty, nLevel))
	{
		assertRetVal(AddLevelBonusSaucer(nLevel), false);
	}
	else if (Globals::IsInvaderBonusLevel(difficulty, nLevel))
	{
		assertRetVal(AddLevelBonusInvaders(nLevel), false);
	}
	else
	{
		assertRetVal(AddLevelInvaders(nLevel), false);
		assertRetVal(AddLevelShields(nLevel), false);
	}

	entity->TriggerEvent(ENTITY_EVENT_BORN);

	*pentity = ff::GetAddRef(entity.Interface());
	return true;
}

bool EntityFactoryService::InitSprites()
{	
	if (_pShieldSprites.Flush())
	{
		ff::ISpriteList *sprites = _pShieldSprites.Flush();

		_shields[SHIELD_TYPE_NORMAL][0] = sprites->Get(ff::String(L"Shield Normal 0"));
		_shields[SHIELD_TYPE_NORMAL][1] = sprites->Get(ff::String(L"Shield Normal 1"));
		_shields[SHIELD_TYPE_NORMAL][2] = sprites->Get(ff::String(L"Shield Normal 2"));
		_shields[SHIELD_TYPE_NORMAL][3] = sprites->Get(ff::String(L"Shield Normal 3"));
		_shields[SHIELD_TYPE_NORMAL][4] = sprites->Get(ff::String(L"Shield Normal 4"));

		_shields[SHIELD_TYPE_HARD][0] = sprites->Get(ff::String(L"Shield Hard 0"));
		_shields[SHIELD_TYPE_HARD][1] = sprites->Get(ff::String(L"Shield Hard 1"));
		_shields[SHIELD_TYPE_HARD][2] = sprites->Get(ff::String(L"Shield Hard 2"));
		_shields[SHIELD_TYPE_HARD][3] = sprites->Get(ff::String(L"Shield Hard 3"));
		_shields[SHIELD_TYPE_HARD][4] = sprites->Get(ff::String(L"Shield Hard 4"));

		_pShieldSprites = ff::TypedResource<ff::ISpriteList>();
	}

	return true;
}

bool EntityFactoryService::AddLevelBonusSaucer(size_t nLevel)
{
	Difficulty diff = Globals::GetDifficulty(_domain);
	SaucerEntityType type = SAUCER_TYPE_BONUS_LEVEL;

	ff::PointFloat pos(0, Globals::GetSaucerRect(type).Center().y);
	ff::PointFloat vel = Globals::GetSaucerSpeed(type, diff);

	if (rand() % 2)
	{
		pos.x = Globals::GetSaucerRect(type).left;
	}
	else
	{
		pos.x = Globals::GetSaucerRect(type).right;
		vel.x = -vel.x;
	}

	ff::ComPtr<IEntity> pSaucerEntity;
	assertRetVal(CreateSaucerEntity(type, pos, vel, &pSaucerEntity), false);

	pSaucerEntity->TriggerEvent(ENTITY_EVENT_BORN);

	return true;
}

static ff::PointFloat GetHexGridPos(ff::PointInt cell)
{
	ff::PointFloat pos(-250.0f + cell.x * 200.0f, -200.0f + cell.y * 320.0f);

	if (cell.x % 2)
	{
		if (cell.y % 2)
		{
			pos.y -= 58;
		}
		else
		{
			pos.y += 58;
		}
	}
	else
	{
		if (cell.y % 2)
		{
			pos.y += 58;
		}
		else
		{
			pos.y -= 58;
		}
	}

	return pos;
}

enum HexMoveDir
{
	MOVE_DIR_NONE,
	MOVE_DIR_LEFT,
	MOVE_DIR_RIGHT,
	MOVE_DIR_VERT,
};

static ff::PointInt MoveHexCell(ff::PointInt cell, HexMoveDir dir)
{
	if (dir == MOVE_DIR_LEFT)
	{
		cell.x--;
	}
	else if (dir == MOVE_DIR_RIGHT)
	{
		cell.x++;
	}
	else if (dir == MOVE_DIR_VERT)
	{
		if (cell.x % 2)
		{
			if (cell.y % 2)
			{
				cell.y--;
			}
			else
			{
				cell.y++;
			}
		}
		else
		{
			if (cell.y % 2)
			{
				cell.y++;
			}
			else
			{
				cell.y--;
			}
		}
	}

	return cell;
}

static bool IsHexCellInLevel(ff::PointInt cell)
{
	return Globals::GetLevelRectF().PointInRect(GetHexGridPos(cell));
}

bool EntityFactoryService::AddLevelBonusInvaders(size_t nLevel)
{
	struct BonusInvader
	{
		ff::ComPtr<IEntity> _entity;
		PathComponent* _pPath;
		ff::Vector<PathComponent::Key> _keys;
		float _frameOffset;
	};

	// 1 frame = 230 pixels
	const float invaderFrameSpacing = Globals::GetInvaderSpacing().y / 230.0f;
	const float groupFrameSpacing = 4;

	for (int nGroup = 0; nGroup < 4; nGroup++)
	{
		BonusInvader invaders[8];
		ff::PointInt cell(nGroup * 2 + 3, 0);

		for (size_t i = 0; i < _countof(invaders); i++)
		{
			ff::PointFloat pos = GetHexGridPos(cell);
			invaders[i]._frameOffset = i * invaderFrameSpacing;

			PathComponent::Key initialKey = { pos, 0.0f };
			invaders[i]._keys.Push(initialKey);

			PathComponent::Key key = { pos, invaders[i]._frameOffset + groupFrameSpacing * nGroup };
			invaders[i]._keys.Push(key);

			assertRetVal(CreateInvaderEntity(
				(InvaderEntityType)((i / 2 % 3) + INVADER_TYPE_BONUS_0),
				ff::PointInt(nGroup, (int)i),
				pos, ff::PointFloat(0, 0),
				&invaders[i]._entity), false);

			InvaderComponent *pInvader = invaders[i]._entity->GetComponent<InvaderComponent>();
			pInvader->SetMoveType(INVADER_MOVE_PATH);

			invaders[i]._pPath = invaders[i]._entity->EnsureComponent<PathComponent>();
		}

		size_t nMoves = 10 + rand() % 6;
		HexMoveDir lastDir = MOVE_DIR_NONE;
		HexMoveDir nextDir = MOVE_DIR_VERT;
		float frame = groupFrameSpacing * nGroup + 1;

		for (size_t i = 0; i < nMoves; i++, frame++)
		{
			cell = MoveHexCell(cell, nextDir);

			for (size_t i = 0; i < _countof(invaders); i++)
			{
				PathComponent::Key key = { GetHexGridPos(cell), frame + invaders[i]._frameOffset };
				invaders[i]._keys.Push(key);
			}

			static HexMoveDir possibleMoves[4][2] =
			{
				{ MOVE_DIR_VERT, MOVE_DIR_VERT }, // from none
				{ MOVE_DIR_LEFT, MOVE_DIR_VERT }, // from left
				{ MOVE_DIR_RIGHT, MOVE_DIR_VERT }, // from right
				{ MOVE_DIR_LEFT, MOVE_DIR_RIGHT }, // from vertical
			};

			size_t nChoice = rand() % 2;
			{
				HexMoveDir possibleMove = possibleMoves[nextDir][nChoice];
				ff::PointInt nextCell = MoveHexCell(cell, possibleMove);

				if (nextCell.y >= 4 ||
					!IsHexCellInLevel(nextCell) ||
					!IsHexCellInLevel(MoveHexCell(nextCell, possibleMoves[possibleMove][0])) ||
					!IsHexCellInLevel(MoveHexCell(nextCell, possibleMoves[possibleMove][1])))
				{
					nChoice = !nChoice;
				}
			}

			nextDir = possibleMoves[nextDir][nChoice];
		}

		// move off the level
		for (; IsHexCellInLevel(cell); frame++)
		{
			cell = (GetHexGridPos(cell).x < Globals::GetLevelRectF().Center().x)
				? MoveHexCell(cell, MOVE_DIR_LEFT)
				: MoveHexCell(cell, MOVE_DIR_RIGHT);

			for (size_t i = 0; i < _countof(invaders); i++)
			{
				PathComponent::Key key = { GetHexGridPos(cell), frame + invaders[i]._frameOffset };
				invaders[i]._keys.Push(key);
			}
		}

		for (size_t i = 0; i < _countof(invaders); i++)
		{
			const ff::Vector<PathComponent::Key>& keys = invaders[i]._keys;

			invaders[i]._pPath->SetKeys(keys.Data(), keys.Size(), keys.GetLast()._frame, 1, ff::POSE_TWEEN_SPLINE_CLAMP);
		}
	}

	// Create powerups
	{
		ff::ComPtr<IEntity> powerups[4];
		ff::PointFloat speed(0, Globals::GetBonusDropSpeed());
		ff::PointFloat hexOffset(0, 110);

		CreatePowerup(POWERUP_TYPE_MULTI_SHOT, 0, GetHexGridPos(ff::PointInt(3, 0)) + hexOffset, speed, &powerups[0]);
		CreatePowerup(POWERUP_TYPE_AMMO, 1, GetHexGridPos(ff::PointInt(5, 0)) + hexOffset, speed, &powerups[1]);
		CreatePowerup(POWERUP_TYPE_HOMING_SHOT, 0, GetHexGridPos(ff::PointInt(7, 0)) + hexOffset, speed, &powerups[2]);
		CreatePowerup(POWERUP_TYPE_SPREAD_SHOT, 1, GetHexGridPos(ff::PointInt(9, 0)) + hexOffset, speed, &powerups[3]);
	}

	return true;
}

bool EntityFactoryService::AddLevelInvaders(size_t nLevel)
{
	bool bMoveLeft = !(rand() % 2);
	float initX = bMoveLeft
		? Globals::GetInvaderArea().right - (Globals::GetInvaderFieldSize().x - 1) * Globals::GetInvaderSpacing().x
		: Globals::GetInvaderArea().left;

	ff::PointFloat pos(initX, Globals::GetInvaderArea().top);
	ff::PointFloat vel(Globals::GetInvaderMoveDelta().x * (bMoveLeft ? -1 : 1), 0);
	size_t nInvaderLevel = Globals::GetInvaderLevel(nLevel);

	Difficulty diff = Globals::GetDifficulty(_domain);
	if (!Globals::IsEasyDifficulty(diff))
	{
		pos.y += std::min<size_t>(4, nInvaderLevel) * Globals::GetInvaderMoveDelta().y;
	}

	for (int nRow = 0; nRow < Globals::GetInvaderFieldSize().y; nRow++, pos.y += Globals::GetInvaderSpacing().y, pos.x = initX)
	{
		size_t nTypeIndex = (nInvaderLevel + Globals::GetInvaderFieldSize().y - nRow - 1);
		size_t nRepeat = nTypeIndex / INVADER_TYPE_NORMAL_COUNT;

		if (nTypeIndex < INVADER_TYPE_NORMAL_COUNT * 2)
		{
			nTypeIndex /= 2;
			nRepeat /= 2;
		}
		else
		{
			nTypeIndex -= INVADER_TYPE_NORMAL_COUNT;
			nRepeat--;
		}

		InvaderEntityType invaderType = (nRepeat <= 1)
			? (InvaderEntityType)(nTypeIndex % INVADER_TYPE_NORMAL_COUNT)
			: (InvaderEntityType)(rand() % INVADER_TYPE_NORMAL_COUNT);

		for (int nCol = 0; nCol < Globals::GetInvaderFieldSize().x; nCol++, pos.x += Globals::GetInvaderSpacing().x)
		{
			ff::ComPtr<IEntity> pInvaderEntity;
			assertRetVal(CreateInvaderEntity(invaderType, ff::PointInt(nCol, nRow), pos, vel, &pInvaderEntity), false);

			InvaderComponent* pInvader = pInvaderEntity->GetComponent<InvaderComponent>();
			pInvader->SetMoveType(INVADER_MOVE_TOGETHER);
			pInvader->SetRepeat(nRepeat);

			pInvaderEntity->TriggerEvent(ENTITY_EVENT_BORN);
		}
	}

	return true;
}

bool EntityFactoryService::AddLevelShields(size_t nLevel)
{
	Difficulty diff = Globals::GetDifficulty(_domain);

	for (size_t i = 0; i < Globals::GetShieldCount(); i++)
	{
		ShieldEntityType type = Globals::IsHardDifficulty(diff)
			? SHIELD_TYPE_HARD
			: SHIELD_TYPE_NORMAL;

		ff::ComPtr<IEntity> entity;
		assertRetVal(CreateShieldEntity(type, Globals::GetShieldRect(i).TopLeft(), &entity), false);

		entity->TriggerEvent(ENTITY_EVENT_BORN);
	}

	return true;
}
