#include "pch.h"
#include "ThisApplication.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteAnimationAdvance.h"
#include "components\graph\SpriteAnimationRender.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\entity\Entity.h"
#include "entities\EntityEvents.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"SpriteAnimationAdvance");
	module.RegisterClassT<SpriteAnimationAdvance>(name);
});

BEGIN_INTERFACES(SpriteAnimationAdvance)
	HAS_INTERFACE(IAdvanceComponent)
END_INTERFACES()

SpriteAnimationAdvance::SpriteAnimationAdvance()
	: _priority(LAYER_PRI_NORMAL)
	, _loops(0)
	, _start(0)
	, _end(0)
	, _frame(0)
	, _frameAdvance(0)
	, _done(true)
{
}

SpriteAnimationAdvance::~SpriteAnimationAdvance()
{
}

// static
SpriteAnimationAdvance *SpriteAnimationAdvance::Create(IEntity *entity, int priority)
{
	ff::ComPtr<SpriteAnimationAdvance, IAdvanceComponent> pComp;
	assertRetVal(entity->CreateComponent<SpriteAnimationAdvance>(&pComp), nullptr);
	assertRetVal(pComp->Init(priority), false);

	entity->AddComponent<IAdvanceComponent>(pComp);
	pComp->UpdateAnims(entity);

	return pComp;
}

bool SpriteAnimationAdvance::Init(int priority)
{
	_priority = priority;

	return true;
}

void SpriteAnimationAdvance::SetLoopingAnim(float start, float end, float fps, int loops)
{
	_loops = loops;
	_start = start;
	_end = end;
	_frame = start;
	_frameAdvance = fabs(fps) * (start > end ? -1 : 1) / Globals::GetAdvancesPerSecondF();
	_done = _frameAdvance == 0 || !_loops;
}

void SpriteAnimationAdvance::SetOneTimeAnim(float start, float end, float fps)
{
	SetLoopingAnim(start, end, fps, 1);
}

void SpriteAnimationAdvance::SetInfiniteAnim(float start, float fps)
{
	SetLoopingAnim(start, start, fps, -1);
}

bool SpriteAnimationAdvance::IsDone() const
{
	return _done;
}

bool SpriteAnimationAdvance::DidLoop() const
{
	if (_loops > 0)
	{
		return
			(_frameAdvance < 0 && _frame <= _end) ||
			(_frameAdvance > 0 && _frame >= _end);
	}

	return false;
}

int SpriteAnimationAdvance::GetAdvancePriority() const
{
	return _priority;
}

void SpriteAnimationAdvance::Advance(IEntity *entity)
{
	if (!_done)
	{
		bool bLooped = false;

		if (_loops)
		{
			_frame += _frameAdvance;

			while (DidLoop())
			{
				_frame += (_start - _end);
				_loops = (_loops > 0) ? _loops - 1 : _loops;
				bLooped = true;
			}
		}

		UpdateAnims(entity);

		_done = !_loops;

		if (bLooped)
		{
			AnimLoopedEventArgs eventArgs;
			eventArgs._pAdvance = this;
			eventArgs._loopsLeft = _loops;

			entity->TriggerEvent(ENTITY_EVENT_ANIM_LOOPED, &eventArgs);
		}

		if (_done)
		{
			entity->TriggerEvent(ENTITY_EVENT_DIED);
		}
	}
}

void SpriteAnimationAdvance::UpdateAnims(IEntity *entity)
{
	SpriteAnimationRender *pAnim;

	for (size_t i = 0; pAnim = entity->GetComponent<SpriteAnimationRender>(i); i++)
	{
		pAnim->SetFrame(_frame);
	}
}
