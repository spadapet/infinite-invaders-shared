#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "components\invader\InvaderComponent.h"
#include "components\invader\InvaderTitleAdvanceRender.h"
#include "components\core\PositionComponent.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"
#include "Graph\Anim\AnimKeys.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"InvaderTitleAdvanceRender");
	module.RegisterClassT<InvaderTitleAdvanceRender>(name);
});

BEGIN_INTERFACES(InvaderTitleAdvanceRender)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(I2dRenderComponent)
	HAS_INTERFACE(IEntityEventListener)
	HAS_INTERFACE(IComponentListener)
END_INTERFACES()

InvaderTitleAdvanceRender::InvaderTitleAdvanceRender()
	: _frames(0)
{
}

InvaderTitleAdvanceRender::~InvaderTitleAdvanceRender()
{
}

HRESULT InvaderTitleAdvanceRender::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	ThisApplication* pApp = ThisApplication::Get(pDomainProvider);
	IEntityDomain* pDomain = pDomainProvider->GetDomain();

	_invaderListener.Init(pDomain, this);

	_invaderAnims[INVADER_TYPE_0].Init(L"Invader 0");
	_invaderAnims[INVADER_TYPE_1].Init(L"Invader 1");
	_invaderAnims[INVADER_TYPE_2].Init(L"Invader 2");

	return __super::_Construct(unkOuter);
}

int InvaderTitleAdvanceRender::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void InvaderTitleAdvanceRender::Advance(IEntity *entity)
{
	_frames++;

	for (size_t i = ff::PreviousSize(_invaders.Size()); i != ff::INVALID_SIZE; i = ff::PreviousSize(i))
	{
		const EntityInvader& ei = _invaders[i];

		ei._pPosComp->SetPos(ei._pPosInfo->_translate + ei._pPosInfo->_velocity);

		if (!Globals::GetLevelRectF().DoesTouch(ei._pPosInfo->_bounds))
		{
			ei._entity->TriggerEvent(ENTITY_EVENT_DIED);
		}
	}

	if (!(_frames % 30))
	{
		ff::ComPtr<EntityFactoryService> pFactory;
		if (GetService(entity, &pFactory))
		{
			// Invader
			{
				ff::PointFloat pos(0, (rand() % 450) + 400.0f);
				InvaderEntityType type = (InvaderEntityType)(INVADER_TYPE_0 + rand() % 3);
				ff::ComPtr<IEntity> pInvaderEntity;

				pFactory->CreateInvaderEntity(type, ff::PointInt(0, 0), pos, ff::PointFloat((rand() % 4) + 3.0f, 0), &pInvaderEntity);
			}

			// Bullet
			{
				ff::PointFloat pos((float)(rand() % Globals::GetLevelSize().x), Globals::GetLevelSizeF().y);
				BulletEntityType type = BULLET_TYPE_PLAYER_0;
				ff::ComPtr<IEntity> pBulletEntity;

				pFactory->CreateBulletEntity(type, pos, ff::PointFloat(0, ((rand() % 6) + 6) * -1.0f), &pBulletEntity);
			}
		}
	}
}

int InvaderTitleAdvanceRender::Get2dRenderPriority() const
{
	return LAYER_PRI_NORMAL;
}

void InvaderTitleAdvanceRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	float invaderFrame = _frames / 30.0f;

	for (size_t i = 0; i < _invaders.Size(); i++)
	{
		const EntityInvader& ei = _invaders[i];
		InvaderEntityType type = ei._component->GetType();

		if (type >= 0 && type < _countof(_invaderAnims))
		{
			ff::ISpriteAnimation *anim = _invaderAnims[type].GetObject();
			if (anim)
			{
				anim->Render(render, ff::POSE_TWEEN_LINEAR_LOOP, invaderFrame, ei._pPosInfo->_translate, nullptr, 0, nullptr);
			}
			else
			{
				assert(false);
			}
		}
	}
}

void InvaderTitleAdvanceRender::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;
		HandleCollision(entity, eventArgs2._pOther);
	}
}

void InvaderTitleAdvanceRender::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<InvaderComponent> pInvader;
	if (pInvader.QueryFrom(pComp))
	{
		_invaders.Push(EntityInvader(entity, pInvader, true));
		IEntityEventListeners::AddListener(entity, this);
	}
}

void InvaderTitleAdvanceRender::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<InvaderComponent> pInvader;
	if (pInvader.QueryFrom(pComp))
	{
		size_t i = _invaders.Find(EntityInvader(entity, pInvader, false));
		assertRet(i != ff::INVALID_SIZE);

		_invaders[i] = _invaders.GetLast();
		_invaders.Pop();

		IEntityEventListeners::RemoveListener(entity, this);
	}
}

void InvaderTitleAdvanceRender::HandleCollision(IEntity *pInvaderEntity, IEntity *otherEntity)
{
	InvaderComponent* pInvader = pInvaderEntity->GetComponent<InvaderComponent>();
	BulletComponent* pBullet = otherEntity->GetComponent<BulletComponent>();

	if (pInvader && pBullet && !pBullet->IsInvader())
	{
		ff::ComPtr<EntityFactoryService> pFactory;
		if (GetService(pInvaderEntity, &pFactory))
		{
			pFactory->CreateInvaderExplosion(pInvaderEntity);
		}

		BulletEventArgs eventArgs;
		eventArgs._pBullet = otherEntity;
		eventArgs._pSource = pInvaderEntity;

		pInvaderEntity->TriggerEvent(ENTITY_EVENT_BULLET_HIT, &eventArgs);
		pInvaderEntity->TriggerEvent(ENTITY_EVENT_DIED);
		otherEntity->TriggerEvent(ENTITY_EVENT_DIED);
	}
}
