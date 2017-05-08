#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteAnimationAdvance.h"
#include "components\graph\SpriteAnimationRender.h"
#include "components\graph\SpriteRender.h"
#include "components\invader\InvaderComponent.h"
#include "components\level\LevelAdvanceRender.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Graph\2D\SpriteList.h"
#include "Graph\Anim\AnimKeys.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\Font\SpriteFont.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"LevelAdvanceRender");
	module.RegisterClassT<LevelAdvanceRender>(name);
});

BEGIN_INTERFACES(LevelAdvanceRender)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(I2dRenderComponent)
END_INTERFACES()

LevelAdvanceRender::LevelAdvanceRender()
	: _domain(nullptr)
	, _levelType(LEVEL_TYPE_INVADERS)
	, _spaceFrame(0)
{
}

LevelAdvanceRender::~LevelAdvanceRender()
{
	if (_listener)
	{
		_listener->SetOwner(nullptr);
	}
}

HRESULT LevelAdvanceRender::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);
	ThisApplication *pApp = ThisApplication::Get(pDomainProvider);

	_domain = pDomainProvider->GetDomain();

	size_t nLevel = Globals::GetCurrentLevel(_domain);
	Difficulty diff = Globals::GetDifficulty(_domain);

	if (Globals::IsSaucerBonusLevel(diff, nLevel))
	{
		_levelType = LEVEL_TYPE_BONUS_SAUCER;
	}
	else if (Globals::IsInvaderBonusLevel(diff, nLevel))
	{
		_levelType = LEVEL_TYPE_BONUS_INVADERS;
	}

	ff::String szBgAnim;
	ff::String szGroundSprite;
	
	switch (_levelType)
	{
	case LEVEL_TYPE_INVADERS:
		szBgAnim = ff::String::format_new(L"Space %lu", Globals::GetInvaderLevel(nLevel) % 12);
		szGroundSprite = L"Sprites.Ground";
		break;

	case LEVEL_TYPE_BONUS_SAUCER:
		szBgAnim = ff::String(L"Bonus Saucer BG");
		szGroundSprite = L"Sprites.Bonus Saucer Ground";
		break;

	case LEVEL_TYPE_BONUS_INVADERS:
		szBgAnim = ff::String(L"Bonus Invaders BG");
		szGroundSprite = L"Sprites.Bonus Invader Ground";
		break;
	}

	assertRetVal(CreateProxyEntityEventListener(this, &_listener), E_FAIL);
	_invadersWinListener.Init(_domain, ENTITY_EVENT_INVADERS_WIN, this);

	_font.Init(L"Classic");
	_bgAnim.Init(szBgAnim);
	_groundSprite.Init(szGroundSprite);

	// Ground sprite render
	{
		assertRetVal(_domain->GetEntityManager()->CreateEntity(&_groundEntity), E_FAIL);
		ff::ComPtr<ff::ISprite> spriteWrapper;
		assertRetVal(ff::CreateSpriteResource(_groundSprite.GetResourceValue(), &spriteWrapper), E_FAIL);
		SpriteRender *pGroundRender = SpriteRender::Create(_groundEntity, spriteWrapper, LAYER_PRI_NORMAL - 3);

		pGroundRender->GetPos()._translate = Globals::GetLevelGroundRect().TopLeft();
	}

	return __super::_Construct(unkOuter);
}

int LevelAdvanceRender::GetAdvancePriority() const
{
	return LAYER_PRI_LOW;
}

void LevelAdvanceRender::Advance(IEntity *entity)
{
}

int LevelAdvanceRender::Get2dRenderPriority() const
{
	return LAYER_PRI_LOW;
}

void LevelAdvanceRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	ff::ISpriteAnimation *bgAnim = _bgAnim.GetObject();
	if (bgAnim)
	{
		bgAnim->Render(render, ff::POSE_TWEEN_LINEAR_CLAMP, _spaceFrame, ff::PointFloat(0, 0), nullptr, 0, nullptr);
		render->Flush();
	}

	DirectX::XMFLOAT4 color(1, 1, 1, 0.75);
	render->DrawFilledRectangle(&ff::RectFloat(0, 0, _spaceFrame / 4 * 1900, 8), &color, 1);
}

void LevelAdvanceRender::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_INVADERS_WIN)
	{
		OnInvadersWin();
	}
	else if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;
		HandleCollision(entity, eventArgs2._pOther);
	}
}

void LevelAdvanceRender::HandleCollision(IEntity *entity, IEntity *otherEntity)
{
	InvaderComponent* pOtherInvader = otherEntity->GetComponent<InvaderComponent>();
	SpriteRender* pSpriteRender = entity->GetComponent<SpriteRender>();

	if (pOtherInvader && pSpriteRender)
	{
		float dir = (otherEntity->GetComponent<IPositionComponent>()->GetVelocity().x > 0) ? 1.0f : -1.0f;
		ff::PointFloat basePos = entity->GetComponent<IPositionComponent>()->GetPos();

		ff::ComPtr<ff::ISpriteAnimation> pSpriteAnim;
		ff::ComPtr<IEntity> pNewEntity;

		if (_domain->GetEntityManager()->CreateEntity(&pNewEntity) &&
			CreateSpriteAnimation(ff::MetroGlobals::Get()->GetGraph(), &pSpriteAnim))
		{
			pSpriteAnim->SetFPS(1);
			pSpriteAnim->SetLastFrame(1);

			pSpriteAnim->SetColor(0, 0, 0, ff::GetColorWhite());
			pSpriteAnim->SetColor(1, 0, 0, ff::GetColorNone());

			pSpriteAnim->SetOffset(0, 0, basePos);
			pSpriteAnim->SetOffset(1, 0, basePos + ff::PointFloat(dir * (rand() % 50 + 200), -(rand() % 50 + 200.0f)));

			pSpriteAnim->SetRotate(0, 0, 0);
			pSpriteAnim->SetRotate(1, 0, rand() % 200 / 50.0f * ff::PI_F);

			float newScale = rand() % 50 / 100.0f + 0.5f;
			pSpriteAnim->SetScale(0, 0, ff::PointFloat(1, 1));
			pSpriteAnim->SetScale(1, 0, ff::PointFloat(newScale, newScale));

			pSpriteAnim->SetSprite(0, 0, 0, pSpriteRender->GetSprite());
			pSpriteAnim->SetSprite(1, 0, 0, pSpriteRender->GetSprite());

			SpriteAnimationRender* pAnimRender = SpriteAnimationRender::Create(pNewEntity, pSpriteAnim, ff::POSE_TWEEN_LINEAR_CLAMP, LAYER_PRI_NORMAL - 3);
			SpriteAnimationAdvance* pAnimAdvance = SpriteAnimationAdvance::Create(pNewEntity);
			pAnimAdvance->SetOneTimeAnim(0, pSpriteAnim->GetLastFrame(), pSpriteAnim->GetFPS());

			pNewEntity->TriggerEvent(ENTITY_EVENT_BORN);
		}

		entity->TriggerEvent(ENTITY_EVENT_DIED);

		_domain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_LEVEL_BASE_HIT);
	}
}

void LevelAdvanceRender::OnInvadersWin()
{
	// Replace the ground sprite with a bunch of breakable blocks

	_groundEntity->RemoveAllComponents();
	_groundEntity = nullptr;

	for (int y = 0; y < (int)GROUND_ROWS; y++)
	{
		for (int x = 0; x < (int)GROUND_COLS; x++)
		{
			ff::ComPtr<IEntity> entity;
			if (_domain->GetEntityManager()->CreateEntity(&entity))
			{
				ff::PointInt tl(x * 50, y * 50);
				ff::RectInt rect(tl, tl + ff::PointInt(50, 50));

				ff::ComPtr<ff::ISprite> pSprite;
				ff::ISprite *groundSprite = _groundSprite.Flush();
				assertRet(ff::CreateSprite(groundSprite, rect.ToFloat(), ff::PointFloat(25, 25), ff::PointFloat(1, 1), ff::SpriteType::Unknown, &pSprite));

				ff::PointFloat pos = Globals::GetLevelGroundRect().TopLeft() + ff::PointFloat(x * 50 + 25.0f, y * 50 + 25.0f);
				IPositionComponent::Create(entity, pos, ff::PointFloat(0, 0), &ff::RectFloat(-25, -25, 25, 25), true);
				SpriteRender::Create(entity, pSprite, LAYER_PRI_NORMAL - 3);

				IEntityEventListeners::AddListener(entity, _listener);
				entity->TriggerEvent(ENTITY_EVENT_BORN);
			}
		}
	}

	// Game over text
	{
		ff::ComPtr<ff::ISprite> pSprite;
		ff::MetroGlobals *app = ff::MetroGlobals::Get();
		ff::ISpriteFont *font = _font.GetObject();

		if (font &&
			ff::CreateSpriteFromText(
				app->Get2dRender(),
				app->Get2dEffect(),
				font, ff::String(L"GAME OVER"), &pSprite))
		{
			ff::PointInt size = pSprite->GetSpriteData().GetTextureRect().Size();
			ff::PointFloat fSize((float)size.x, (float)size.y);
			ff::PointFloat pos = Globals::GetLevelGroundRect().Center() - fSize;
			pos += ff::PointFloat(0, 15);
			ff::ComPtr<IEntity> entity;
			if (_domain->GetEntityManager()->CreateEntity(&entity))
			{
				IPositionComponent* pPos = IPositionComponent::Create(entity, pos, ff::PointFloat(0, 0), nullptr, false);
				SpriteRender::Create(entity, pSprite, LAYER_PRI_NORMAL - 4);

				pPos->SetScale(ff::PointFloat(2, 2));
				entity->TriggerEvent(ENTITY_EVENT_BORN);
			}
		}
	}
}
