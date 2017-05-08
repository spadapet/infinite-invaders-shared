#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "COM\ServiceCollection.h"
#include "components\bullet\BulletComponent.h"
#include "components\core\PathComponent.h"
#include "components\invader\InvaderAdvanceRender.h"
#include "components\invader\InvaderComponent.h"
#include "components\core\PositionComponent.h"
#include "coreEntity\component\standard\EntityEventListenersComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "coreEntity\entity\Entity.h"
#include "coreEntity\entity\EntityManager.h"
#include "entities\EntityEvents.h"
#include "Globals\MetroGlobals.h"
#include "Graph\Anim\AnimPos.h"
#include "Graph\2D\2dEffect.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Graph\GraphTexture.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Module\ModuleFactory.h"
#include "services\EntityFactoryService.h"
#include "services\GlobalPlayerService.h"
#include "services\PlayerService.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"InvaderAdvanceRender");
	module.RegisterClassT<InvaderAdvanceRender>(name);
});

BEGIN_INTERFACES(InvaderAdvanceRender)
	HAS_INTERFACE(IAdvanceComponent)
	HAS_INTERFACE(I2dRenderComponent)
	HAS_INTERFACE(IEntityEventListener)
	HAS_INTERFACE(IComponentListener)
	HAS_INTERFACE(IGameBeatService)
	HAS_INTERFACE(IInvaderService)
END_INTERFACES()

InvaderAdvanceRender::InvaderAdvanceRender()
	: _domain(nullptr)
	, _destroyBaseInvader(nullptr)
	, _frames(0)
	, _subBeat(0)
	, _beats(0)
	, _shots(0)
	, _cachedSpeed(ff::INVALID_SIZE)
	, _state(INVADER_STATE_INIT)
	, _stateCounter(0)
	, _stateSubCounter(0)
	, _difficulty(DIFFICULTY_NORMAL)
	, _invaderLevel(0)
	, _invaderBonus(false)
{
}

InvaderAdvanceRender::~InvaderAdvanceRender()
{
}

HRESULT InvaderAdvanceRender::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	_domain = pDomainProvider->GetDomain();
	ThisApplication* pApp = ThisApplication::Get(_domain);

	_difficulty = Globals::GetDifficulty(_domain);
	_invaderLevel = Globals::GetInvaderLevel(Globals::GetCurrentLevel(_domain));
	_invaderBonus = Globals::IsInvaderBonusLevel(_difficulty, Globals::GetCurrentLevel(_domain));

	_invaderListener.Init(_domain, this);
	_turnAroundListener.Init(_domain, ENTITY_EVENT_INVADERS_TURN_AROUND, this);

	_invaderAnims[INVADER_TYPE_0].Init(L"Invader 0");
	_invaderAnims[INVADER_TYPE_1].Init(L"Invader 1");
	_invaderAnims[INVADER_TYPE_2].Init(L"Invader 2");
	_invaderAnims[INVADER_TYPE_3].Init(L"Invader 3");
	_invaderAnims[INVADER_TYPE_4].Init(L"Invader 4");
	_invaderAnims[INVADER_TYPE_5].Init(L"Invader 5");
	_invaderAnims[INVADER_TYPE_6].Init(L"Invader 6");
	_invaderAnims[INVADER_TYPE_7].Init(L"Invader 7");

	_invaderAnims[INVADER_TYPE_BONUS_0].Init(L"Invader Bonus 0");
	_invaderAnims[INVADER_TYPE_BONUS_1].Init(L"Invader Bonus 1");
	_invaderAnims[INVADER_TYPE_BONUS_2].Init(L"Invader Bonus 2");

	_domain->GetServices()->AddService(__uuidof(IGameBeatService), static_cast<IGameBeatService*>(this));
	_domain->GetServices()->AddService(__uuidof(IInvaderService), static_cast<IInvaderService*>(this));

	return __super::_Construct(unkOuter);
}

int InvaderAdvanceRender::GetAdvancePriority() const
{
	return LAYER_PRI_NORMAL;
}

void InvaderAdvanceRender::Advance(IEntity *entity)
{
	_frames++;
	_stateCounter++;

	ff::ComPtr<IPlayerService> pPlayers;
	assertRet(GetService(entity, &pPlayers));

	switch (_state)
	{
	case INVADER_STATE_INIT:
		SetState(INVADER_STATE_MOVE);
		break;

	case INVADER_STATE_MOVE:
	case INVADER_STATE_MOVE_DOWN:
		if (pPlayers->AreAllPlayersOutOfLives())
		{
			SetState(INVADER_STATE_PRE_DESTROY_BASE);
		}
		else
		{
			MoveInvaders(pPlayers);
		}
		break;

	case INVADER_STATE_PRE_DESTROY_BASE:
		SetState(INVADER_STATE_DESTROY_BASE);
		break;

	case INVADER_STATE_DESTROY_BASE:
		if (!MoveDestroyBaseInvader())
		{
			SetState(INVADER_STATE_DANCING);
		}
		break;

	case INVADER_STATE_DANCING:
		break;
	}

	RenderFadedInvaders(true, INVADER_MOVE_NONE);

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		// Update the hit box for invaders that change size

		EntityInvader &ei = _entities[i];
		if (ei._component->HasCustomHitBox())
		{
			float invaderFrame = GetInvaderFrame();
			InvaderEntityType invaderType = ei._component->GetType();
			ff::ISpriteAnimation *anim = _invaderAnims[invaderType].GetObject();
			if (anim)
			{
				ff::RectFloat invaderBox = anim->GetHitBox(invaderFrame, 0, ff::POSE_TWEEN_LINEAR_LOOP);
				ei._pPosComp->SetPosToBox(invaderBox);
			}
		}
	}
}

int InvaderAdvanceRender::Get2dRenderPriority() const
{
	return LAYER_PRI_NORMAL;
}

static int s_nFadeScale = 4;
static ff::PointFloat s_ptFadeScale((float)s_nFadeScale, (float)s_nFadeScale);
static ff::PointFloat s_ptFadeScaleRender(1.0f / (float)s_nFadeScale, 1.0f / (float)s_nFadeScale);

void InvaderAdvanceRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	// Render old faded invaders
	if (_fadeSprite)
	{
		render->Flush();
		render->GetEffect()->PushDrawType(ff::DRAW_SAMPLE_MAG_LINEAR);

		render->DrawSprite(_fadeSprite, &ff::PointFloat(0, 0), &s_ptFadeScale, 0, nullptr);

		render->Flush();
		render->GetEffect()->PopDrawType();
	}

	RenderInvaders(render, INVADER_MOVE_NONE);
}

void InvaderAdvanceRender::RenderFadedInvaders(bool bFade, InvaderMoveType moveType)
{
	ff::MetroGlobals *app = ff::MetroGlobals::Get();
	ff::I2dRenderer* render = app->Get2dRender();
	ff::I2dEffect* pEffect = app->Get2dEffect();
	ff::PointInt fadeSize = Globals::GetLevelSize() / s_nFadeScale;

	if (!_fadeTarget)
	{
		ff::ComPtr<ff::IGraphTexture> pTexture;
		ff::CreateGraphTexture(render->GetDevice(), fadeSize, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, &pTexture);
		ff::CreateSprite(pTexture, &_fadeSprite);
		ff::CreateRenderTargetTexture(render->GetDevice(), pTexture, 0, 1, 0, &_fadeTarget);
		_fadeTarget->Clear(&ff::GetColorNone());
	}

	if (render->BeginRender(
		_fadeTarget,
		nullptr,
		ff::RectInt(ff::PointInt(0, 0), fadeSize).ToFloat(),
		ff::PointFloat(0, 0),
		s_ptFadeScaleRender,
		pEffect))
	{
		if (bFade)
		{
			pEffect->PushDrawType((ff::DrawType2d)(ff::DRAW_BLEND_MULTIPLY | ff::DRAW_DEPTH_DISABLE));
			DirectX::XMFLOAT4 color(1, 1, 1, 0.85f);
			render->DrawFilledRectangle(&Globals::GetLevelRectF(), &color, 1);

			if (!(_frames % 32))
			{
				// Alpha = (3/255)*.85 rounds back to (3/255), so to allow alpha to fade to zero,
				// I need to manually subtract a value once in a while

				render->Flush();
				pEffect->PopDrawType();

				pEffect->PushDrawType((ff::DrawType2d)(ff::DRAW_BLEND_SUB | ff::DRAW_DEPTH_DISABLE));
				DirectX::XMFLOAT4 color2(0, 0, 0, 1.0f / 255.0f);
				render->DrawFilledRectangle(&Globals::GetLevelRectF(), &color2, 1);
			}
		}
		else
		{
			pEffect->PushDrawType((ff::DrawType2d)(ff::DRAW_BLEND_ALPHA_MAX | ff::DRAW_DEPTH_DISABLE));
			RenderInvaders(render, moveType);
		}

		render->Flush();
		pEffect->PopDrawType();
		render->EndRender();
	}
}

void InvaderAdvanceRender::RenderInvaders(ff::I2dRenderer *render, InvaderMoveType moveType)
{
	IEntity* pDestroyBaseInvader = GetDestroyBaseInvader();
	float invaderFrame = GetInvaderFrame();
	ff::PointFloat offset(0, 0);
	
	if (_state == INVADER_STATE_DANCING && _stateCounter >= 30)
	{
		bool bUp = _stateCounter % 40 >= 20;
		invaderFrame = bUp ? 0.5f : 0.0f;
		offset.y = bUp ? -24.0f : 0.0f;
	}

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityInvader& ei = _entities[i];

		if (moveType == INVADER_MOVE_NONE || ei._component->GetMoveType() == moveType)
		{
			float curInvaderFrame = invaderFrame;
			InvaderEntityType type = ei._component->GetType();
			ff::PointFloat scale(ei._pPosInfo->_velocity.x < 0 ? -1.0f : 1.0f, 1.0f);

			if (_state == INVADER_STATE_DESTROY_BASE && ei._entity == pDestroyBaseInvader)
			{
				curInvaderFrame = _stateCounter * 4 / Globals::GetAdvancesPerSecondF();
			}

			ff::ISpriteAnimation *anim = _invaderAnims[type].GetObject();
			if (anim)
			{
				anim->Render(render, ff::POSE_TWEEN_LINEAR_LOOP,
					curInvaderFrame, ei._pPosInfo->_translate + offset, &scale, 0, nullptr);
			}
		}
	}
}

void InvaderAdvanceRender::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_INVADERS_TURN_AROUND)
	{
		TurnAroundInvaders();
	}
	else if (eventName == ENTITY_EVENT_COLLISION)
	{
		const CollisionEventArgs &eventArgs2 = *(const CollisionEventArgs*)eventArgs;
		HandleCollision(entity, eventArgs2._pOther);
	}
}

void InvaderAdvanceRender::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<InvaderComponent> pInvader;
	assertRet(pInvader.QueryFrom(pComp));

	_entities.Push(EntityInvader(entity, pInvader, true));
	_cachedSpeed = ff::INVALID_SIZE;

	IEntityEventListeners::AddListener(entity, this);
}

void InvaderAdvanceRender::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<InvaderComponent> pInvader;
	assertRet(pInvader.QueryFrom(pComp));

	size_t i = _entities.Find(EntityInvader(entity, pInvader, false));
	assertRet(i != ff::INVALID_SIZE);

	_entities[i] = _entities.GetLast();
	_entities.Pop();

	_cachedSpeed = ff::INVALID_SIZE;

	IEntityEventListeners::RemoveListener(entity, this);
}

float InvaderAdvanceRender::GetInvaderFrame() const
{
	return (float)GetMeasureReal();
}

void InvaderAdvanceRender::SetState(InvaderAdvanceState state)
{
	if (state != _state)
	{
		InvaderAdvanceState oldState = _state;

		_state = state;
		_stateCounter = 0;
		_stateSubCounter = 0;

		OnStateChanged(oldState, _state);
	}
}

void InvaderAdvanceRender::OnStateChanged(InvaderAdvanceState oldState, InvaderAdvanceState newState)
{
	switch (newState)
	{
	case INVADER_STATE_PRE_DESTROY_BASE:
		_domain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_INVADERS_WIN);
		break;

	case INVADER_STATE_DANCING:
		_domain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_INVADERS_DANCING);
		break;
	}
}

void InvaderAdvanceRender::HandleCollision(IEntity *pInvaderEntity, IEntity *otherEntity)
{
	InvaderComponent* pInvader = pInvaderEntity->GetComponent<InvaderComponent>();
	BulletComponent* pBullet = otherEntity->GetComponent<BulletComponent>();

	if (pInvader && pBullet && pBullet->CanHitInvader(otherEntity))
	{
		ff::ComPtr<EntityFactoryService> pFactory;
		if (GetService(pInvaderEntity, &pFactory))
		{
			pFactory->CreateInvaderExplosion(pInvaderEntity);
		}

		// Add score
		if (pBullet->IsPlayer())
		{
			ScoreEventArgs eventArgs;
			eventArgs._nPlayer = pBullet->GetPlayer();
			eventArgs._nAddScore = Globals::GetInvaderPoints(pInvader, _difficulty);

			pInvaderEntity->GetDomain()->GetEntityManager()->TriggerEvent(ENTITY_EVENT_ADD_SCORE, &eventArgs);

			DirectX::XMFLOAT4 color(1, 1, 1, 0.375f);
			pFactory->CreatePoints(pInvaderEntity, eventArgs._nAddScore, 4, ff::PointFloat(0, -25), &color);
		}

		// Push column up
		if (pBullet->GetType() == BULLET_TYPE_PUSH_PLAYER_0 ||
			pBullet->GetType() == BULLET_TYPE_PUSH_PLAYER_1)
		{
			ff::PointFloat pos = pInvaderEntity->GetComponent<IPositionComponent>()->GetPos();
			float moveUp = Globals::GetInvaderMoveDelta().y * 2;
			bool bTooHigh = false;

			for (size_t i = 0; i < _entities.Size(); i++)
			{
				const EntityInvader &ei = _entities[i];
				ff::PointFloat otherPos = ei._pPosInfo->_translate;

				if (otherPos.y - moveUp < Globals::GetSaucerRect(SAUCER_TYPE_NONE).bottom + Globals::GetLevelGridSizeF().y / 2 &&
					otherPos.y < pos.y &&
					fabs(otherPos.x - pos.x) < 8)
				{
					bTooHigh = true;
					break;
				}
			}

			if (!bTooHigh)
			{
				for (size_t i = 0; i < _entities.Size(); i++)
				{
					const EntityInvader &ei = _entities[i];
					ff::PointFloat otherPos = ei._pPosInfo->_translate;

					if (otherPos.y < pos.y && fabs(otherPos.x - pos.x) < 8)
					{
						ei._pPosComp->SetPos(otherPos - ff::PointFloat(0, moveUp));
					}
				}
			}
		}

		// Kill invader and bullet
		{
			BulletEventArgs eventArgs;
			eventArgs._pBullet = otherEntity;
			eventArgs._pSource = pInvaderEntity;

			pInvaderEntity->TriggerEvent(ENTITY_EVENT_BULLET_HIT, &eventArgs);
			pInvaderEntity->TriggerEvent(ENTITY_EVENT_DIED);
			otherEntity->TriggerEvent(ENTITY_EVENT_DIED);
		}
	}
}

size_t InvaderAdvanceRender::GetFrameCount() const
{
	return _frames;
}

size_t InvaderAdvanceRender::GetFramesPerBeat() const
{
	if (_cachedSpeed == ff::INVALID_SIZE && _invaderBonus)
	{
		size_t nCount = _entities.Size();
		size_t nDiff = Globals::IsEasyDifficulty(_difficulty) ? 2 : 1;

		if (nCount > 24)
		{
			_cachedSpeed = 12 + nDiff * 2;
		}
		else if (nCount > 16)
		{
			_cachedSpeed = 8 + nDiff * 2;
		}
		else if (nCount > 8)
		{
			_cachedSpeed = 4 + nDiff * 2;
		}
		else if (nCount > 4)
		{
			_cachedSpeed = 2 + nDiff * 2;
		}
		else
		{
			_cachedSpeed = 1 + nDiff * 2;
		}
	}
	else if (_cachedSpeed == ff::INVALID_SIZE)
	{
		size_t nCount = _entities.Size();
		size_t nDiff = Globals::IsHardDifficulty(_difficulty) ? 0 : (Globals::IsEasyDifficulty(_difficulty) ? 2 : 1);
		size_t nMin = 0;

		if (nCount > 46)
		{
			nMin = 12;
			_cachedSpeed = 12 + nDiff * 2;
		}
		else if (nCount > 32)
		{
			nMin = 10;
			_cachedSpeed = 10 + nDiff * 2;
		}
		else if (nCount > 18)
		{
			nMin = 8;
			_cachedSpeed = 8 + nDiff * 2;
		}
		else if (nCount > 8)
		{
			nMin = 6;
			_cachedSpeed = 6 + nDiff * 2;
		}
		else if (nCount > 4)
		{
			nMin = 4;
			_cachedSpeed = 4 + (nDiff ? (nDiff - 1) : 0);
		}
		else if (nCount > 1)
		{
			nMin = 2;
			_cachedSpeed = 2 + (nDiff ? (nDiff - 1) : 0);
		}
		else
		{
			nMin = 1;
			_cachedSpeed = 1 + (nDiff ? (nDiff - 1) : 0);
		}

		assert(_cachedSpeed >= nMin);

		if (!Globals::IsEasyDifficulty(_difficulty))
		{
			if (_invaderLevel / 2 >= _cachedSpeed - nMin)
			{
				_cachedSpeed = nMin;
			}
			else
			{
				_cachedSpeed -= _invaderLevel / 2;
			}
		}
	}

	return _cachedSpeed;
}

size_t InvaderAdvanceRender::GetBeatsPerShot() const
{
	size_t nCount = _entities.Size();
	size_t nBPS = (nCount > 12) ? 2 : 3;

	switch (_difficulty)
	{
	case DIFFICULTY_HARD:
		if (rand() % 8 == 0)
		{
			nBPS--;
		}
		break;

	case DIFFICULTY_EASY:
		if (nBPS >= 3)
		{
			nBPS += (rand() % 1) + 1;
		}
		break;

	case DIFFICULTY_BABY:
		nBPS = 32 + rand() % 16;

	default:
		if (_invaderLevel < 4 && nBPS == 3)
		{
			nBPS++;
		}

		if (rand() % 12 == 0)
		{
			nBPS--;
		}
		break;
	}

	return nBPS;
}

size_t InvaderAdvanceRender::GetBeatsPerMeasure() const
{
	return BEATS_PER_MEASURE;
}

size_t InvaderAdvanceRender::GetMeasureCount() const
{
	return _beats / BEATS_PER_MEASURE;
}

size_t InvaderAdvanceRender::GetBeatCount() const
{
	return _beats;
}

double InvaderAdvanceRender::GetMeasureReal() const
{
	return GetBeatReal() / BEATS_PER_MEASURE;
}

double InvaderAdvanceRender::GetBeatReal() const
{
	assert(GetFramesPerBeat() > 0);

	return _beats + _subBeat / (double)GetFramesPerBeat();
}

size_t InvaderAdvanceRender::GetInvaderCount() const
{
	return _entities.Size();
}

IEntity *InvaderAdvanceRender::GetInvader(size_t index) const
{
	assertRetVal(index >= 0 && index < _entities.Size(), nullptr);

	return _entities[index]._entity;
}

void InvaderAdvanceRender::MoveInvaders(IPlayerService *pPlayers)
{
	if (_entities.Size() && pPlayers->IsAnyPlayerAlive())
	{
		size_t nSpeed = GetFramesPerBeat();

		if (MoveInvadersAlongPath())
		{
			RenderFadedInvaders(false, INVADER_MOVE_PATH);
		}

		if (++_subBeat >= nSpeed)
		{
			_subBeat %= nSpeed;
			_beats++;

			if (_state == INVADER_STATE_MOVE)
			{
				MoveInvadersSideways(pPlayers);
				_domain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_INVADERS_MOVED_SIDEWAYS);

				ShootBullet();
				RenderFadedInvaders(false, INVADER_MOVE_TOGETHER);
			}
			else if (_state == INVADER_STATE_MOVE_DOWN)
			{
				if (MoveInvadersDown())
				{
					_domain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_INVADERS_MOVED_DOWN);

					if (++_stateSubCounter == 2)
					{
						SetState(INVADER_STATE_MOVE);
					}
				}
				else
				{
					SetState(INVADER_STATE_MOVE);
				}

				RenderFadedInvaders(false, INVADER_MOVE_TOGETHER);
			}
		}
	}
}

void InvaderAdvanceRender::MoveInvadersSideways(IPlayerService *pPlayers)
{
	IEntity* pTriggerTurn = nullptr;
	float bottom = Globals::GetInvaderArea().bottom;

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityInvader& ei = _entities[i];

		if (ei._component->GetMoveType() == INVADER_MOVE_TOGETHER)
		{
			ff::PointFloat velocity = ei._pPosInfo->_velocity;
			ff::PointFloat pos = ei._pPosInfo->_translate;

			if (pos.y >= bottom)
			{
				for (size_t h = 0; h < pPlayers->GetPlayerCount(); h++)
				{
					if (pPlayers->IsPlayerAlive(h))
					{
						IEntity *pPlayerEntity = pPlayers->GetPlayer(h);
						ff::PointFloat playerPos = pPlayerEntity->GetComponent<IPositionComponent>()->GetPos();

						if ((playerPos.x < pos.x && velocity.x > 0) ||
							(playerPos.x > pos.x && velocity.x < 0))
						{
							velocity.x = -velocity.x;
						}
					}
				}
			}

			pos.x += velocity.x;
			ei._pPosComp->SetPos(pos);

			if (!pTriggerTurn)
			{
				if ((pos.x <= Globals::GetInvaderArea().left && velocity.x < 0) ||
					(pos.x >= Globals::GetInvaderArea().right && velocity.x > 0))
				{
					pTriggerTurn = ei._entity;
				}
			}
		}
	}

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityInvader& ei = _entities[i];

		if (ei._component->GetMoveType() == INVADER_MOVE_TOGETHER)
		{
			ei._entity->TriggerEvent(ENTITY_EVENT_INVADER_MOVED_SIDEWAYS);
		}
	}

	if (pTriggerTurn)
	{
		_turnAroundListener.GetDomain()->GetEntityManager()->TriggerEvent(ENTITY_EVENT_INVADERS_TURN_AROUND);
	}
}

bool InvaderAdvanceRender::MoveInvadersDown()
{
	bool bHitBottom = false;
	float bottom = Globals::GetInvaderArea().bottom;

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityInvader& ei = _entities[i];

		if (ei._component->GetMoveType() == INVADER_MOVE_TOGETHER)
		{
			ff::PointFloat pos = ei._pPosInfo->_translate;

			if (pos.y >= bottom)
			{
				bHitBottom = true;
				break;
			}
		}
	}

	if (!bHitBottom)
	{
		for (size_t i = 0; i < _entities.Size(); i++)
		{
			const EntityInvader& ei = _entities[i];

			if (ei._component->GetMoveType() == INVADER_MOVE_TOGETHER)
			{
				ff::PointFloat pos = ei._pPosInfo->_translate;

				pos.y += Globals::GetInvaderMoveDelta().y;
				pos.y = std::min(pos.y, bottom);

				ei._pPosComp->SetPos(pos);
			}
		}
	}

	return !bHitBottom;
}

bool InvaderAdvanceRender::MoveInvadersAlongPath()
{
	bool bFoundAny = false;
	ff::Vector<ff::ComPtr<IEntity>, 32> died;

	float timeScale = 1;
	
	if (GetFramesPerBeat() < 16)
	{
		timeScale = (16 - GetFramesPerBeat()) / 16.0f + 1.0f;
	}

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityInvader& ei = _entities[i];

		if (ei._component->GetMoveType() == INVADER_MOVE_PATH)
		{
			PathComponent* pPath = ei._entity->GetComponent<PathComponent>();

			if (pPath)
			{
				bFoundAny = true;
				pPath->AdvanceFrame(pPath->GetFPS() * timeScale);

				ff::PointFloat pos = pPath->GetPos();
				ei._pPosComp->SetPos(pos);

				if (pPath->GetFrame() >= pPath->GetLastFrame() &&
					!Globals::GetLevelRectF().DoesTouch(ei._pPosInfo->_bounds))
				{
					died.Push(ei._entity);
				}
			}
			else
			{
				assertSz(false, L"Path movement invader doesn't have a path, killing it");
				died.Push(ei._entity);
			}
		}
	}

	for (size_t i = 0; i < died.Size(); i++)
	{
		died[i]->TriggerEvent(ENTITY_EVENT_DIED);
	}

	return bFoundAny;
}

bool InvaderAdvanceRender::MoveDestroyBaseInvader()
{
	IEntity* entity = GetDestroyBaseInvader();
	assertRetVal(entity, false);

	IPositionComponent* pPos = entity->GetComponent<IPositionComponent>();
	ff::PointFloat pos = pPos->GetPos();
	float speed = Globals::GetInvaderMoveDelta().x;
	ff::RectFloat area = Globals::GetInvaderArea();

	if (pos.y < 1055)
	{
		if (pos.x > area.left)
		{
			pos.x = std::max(area.left, pos.x - speed);
		}
		else
		{
			pos.y = std::min(1055.0f, pos.y + speed);
		}
	}
	else if (pos.y < 1105)
	{
		if (pos.x < area.right)
		{
			pos.x = std::min(area.right, pos.x + speed);
		}
		else
		{
			pos.y = std::max(1105.0f, pos.y + speed);
		}
	}
	else if (pos.y < 1155)
	{
		if (pos.x > area.left)
		{
			pos.x = std::max(area.left, pos.x - speed);
		}
		else
		{
			pos.y = std::min(1155.0f, pos.y + speed);
		}
	}
	else if (pos.x < area.right)
	{
		pos.x = std::min(area.right, pos.x + speed);
	}
	else
	{
		return false;
	}

	pPos->SetVelocity(pos - pPos->GetPos());
	pPos->SetPos(pos);

	RenderFadedInvaders(false, INVADER_MOVE_NONE);

	return true;
}

void InvaderAdvanceRender::TurnAroundInvaders()
{
	SetState(INVADER_STATE_MOVE_DOWN);

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityInvader& ei = _entities[i];

		if (ei._component->GetMoveType() == INVADER_MOVE_TOGETHER)
		{
			ff::PointFloat velocity = ei._pPosInfo->_velocity;

			velocity.x = -velocity.x;
			ei._pPosComp->SetVelocity(velocity);
		}
	}

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityInvader& ei = _entities[i];

		if (ei._component->GetMoveType() == INVADER_MOVE_TOGETHER)
		{
			ei._entity->TriggerEvent(ENTITY_EVENT_INVADER_MOVED_DOWN);
		}
	}
}

void InvaderAdvanceRender::ShootBullet()
{
	const int nFirstBeat = 6;
	const int nGroupSize = 2;
	const int nColCount = Globals::GetInvaderFieldSize().x;
	const int nMaxGroup = nColCount / nGroupSize;

	if (_beats >= nFirstBeat && !((_beats - nFirstBeat) % GetBeatsPerShot()))
	{
		const size_t nShots = _shots++;

		if (rand() % nMaxGroup)
		{
			ff::Vector<EntityInvader> choices;

			int nGroup = nShots % nMaxGroup;

			// Adjust the group randomly
			{
				int nGroupAdjust = (rand() % 100);

				if (nGroupAdjust < 5)
				{
					nGroup = (nGroup - 1) % nMaxGroup;
				}
				else if (nGroupAdjust < 10)
				{
					nGroup = (nGroup + 1) % nMaxGroup;
				}
			}

			int nAdjustChance = Globals::IsHardDifficulty(_difficulty) ? 6 : 12;
			int nMinShootCol = nGroup * 2 - ((nAdjustChance && rand() % nAdjustChance == 0) ? 1 : 0);
			int nShootColCount = 2 +((nAdjustChance && rand() % nAdjustChance == 0) ? 1 : 0);

			nMinShootCol = std::max(0, nMinShootCol);
			nShootColCount = std::min(nColCount - nMinShootCol, nShootColCount);

			for (size_t i = 0; i < _entities.Size(); i++)
			{
				InvaderComponent* pInvader = _entities[i]._component;
				ff::PointInt cell = pInvader->GetCell();

				if (cell.x >= nMinShootCol && cell.x < nMinShootCol + nShootColCount && pInvader->CanShoot())
				{
					bool bFound = false;

					for (size_t h = 0; h < choices.Size() && !bFound; h++)
					{
						ff::PointInt otherCell = choices[h]._component->GetCell();

						if (cell.x == otherCell.x)
						{
							if (cell.y > otherCell.y)
							{
								choices[h] = _entities[i];
							}

							bFound = true;
						}
					}

					if (!bFound)
					{
						choices.Push(_entities[i]);
					}
				}
			}

			if (choices.Size())
			{
				ff::ComPtr<EntityFactoryService> pFactory;
				ff::ComPtr<IPlayerService> pPlayers;

				assertRet(GetService(_domain, &pFactory));
				assertRet(GetService(_domain, &pPlayers));

				size_t nChoice = rand() % choices.Size();
				bool bTargetPlayer = !Globals::IsEasyDifficulty(_difficulty);

				if (_difficulty == DIFFICULTY_NORMAL && _invaderLevel >= 4)
				{
					if (_invaderLevel < 8)
					{
						bTargetPlayer = !(rand() % 4);
					}
					else if (_invaderLevel < 12)
					{
						bTargetPlayer = !(rand() % 2);
					}
				}

				if (bTargetPlayer && pPlayers->GetPlayerCount())
				{
					float closestPlayerDist = FLT_MAX;

					for (size_t i = 0; i < choices.Size(); i++)
					{
						ff::PointFloat choicePos = choices[i]._pPosInfo->_translate;

						for (size_t h = 0; h < pPlayers->GetPlayerCount(); h++)
						{
							ff::PointFloat playerPos = pPlayers->GetPlayer(h)->GetComponent<IPositionComponent>()->GetPos();

							if (fabs(choicePos.x - playerPos.x) < closestPlayerDist)
							{
								closestPlayerDist = fabs(choicePos.x - playerPos.x);
								nChoice = i;
							}
						}
					}
				}

				const EntityInvader& choice = choices[nChoice];
				IPositionComponent* pInvaderPos = choice._entity->GetComponent<IPositionComponent>();

				if (pInvaderPos->GetPos().y < Globals::GetInvaderNoShootRow())
				{
					BulletEntityType type = (rand() % 16) ? BULLET_TYPE_INVADER_SMALL : BULLET_TYPE_INVADER_LARGE;
					ff::PointFloat pos(pInvaderPos->GetPos() + Globals::GetInvaderBulletOffset() + ff::PointFloat(rand() % 13 - 6.0f, 0));

					switch (choice._component->GetType())
					{
					case INVADER_TYPE_3:
						type = (type == BULLET_TYPE_INVADER_SMALL) ? BULLET_TYPE_INVADER_LARGE : BULLET_TYPE_INVADER_SMALL;
						break;

					case INVADER_TYPE_5:
					case INVADER_TYPE_7:
						type = BULLET_TYPE_INVADER_SMALL;
						pos.x -= 12;
						break;
					}

					ff::PointFloat vel(0, Globals::GetBulletSpeed(type, _difficulty, _invaderLevel));

					switch (choice._component->GetType())
					{
					case INVADER_TYPE_7:
						vel.x = cos(8 * ff::PI_F / 6) * vel.y;
						vel.y = sin(8 * ff::PI_F / 6) * -vel.y;
						break;
					}

					// Shoot one bullet
					{
						ff::ComPtr<IEntity> pBulletEntity;
						pFactory->CreateBulletEntity(type, pos, vel, &pBulletEntity);

						BulletEventArgs eventArgs;
						eventArgs._pSource = choice._entity;
						eventArgs._pBullet = pBulletEntity;

						_domain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_BULLET_SHOOT, &eventArgs);
					}

					// Shoot another bullet
					switch (choice._component->GetType())
					{
					case INVADER_TYPE_5:
					case INVADER_TYPE_7:
						{
							pos.x += 24;
							vel.x = -vel.x;

							ff::ComPtr<IEntity> pBulletEntity;
							pFactory->CreateBulletEntity(type, pos, vel, &pBulletEntity);

							BulletEventArgs eventArgs;
							eventArgs._pSource = choice._entity;
							eventArgs._pBullet = pBulletEntity;

							_domain->GetEntityManager()->TriggerEvent(ENTITY_EVENT_BULLET_SHOOT, &eventArgs);
						}
						break;
					}
				}
			}
		}
	}
}

IEntity *InvaderAdvanceRender::GetDestroyBaseInvader()
{
	IEntity* pBestInvader = nullptr;

	if (_state == INVADER_STATE_DESTROY_BASE)
	{
		ff::PointFloat bestPos(0, 0);

		for (size_t i = 0; i < _entities.Size(); i++)
		{
			const EntityInvader& ei = _entities[i];
			const ff::PointFloat pos = ei._pPosInfo->_translate;

			if (!pBestInvader || (pos.y == bestPos.y && pos.x < bestPos.x) || (pos.y > bestPos.y))
			{
				bestPos = pos;
				pBestInvader = ei._entity;
			}
		}

		if (!pBestInvader)
		{
			ff::ComPtr<EntityFactoryService> pFactory;
			if (GetService(_domain, &pFactory))
			{
				ff::ComPtr<IEntity> entity;
				ff::PointFloat pos(Globals::GetInvaderArea().left, 0);
				pFactory->CreateInvaderEntity(INVADER_TYPE_0, ff::PointInt(0, 0), pos, Globals::GetInvaderMoveDelta(), &entity);

				pBestInvader = entity;
			}
		}
	}

	return pBestInvader;
}
