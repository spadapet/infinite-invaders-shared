#include "pch.h"
#include "ThisApplication.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteAnimationRender.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\entity\Entity.h"
#include "entities\EntityEvents.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\SpriteAnimation.h"
#include "Graph\Anim\AnimKeys.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"SpriteAnimationRender");
	module.RegisterClassT<SpriteAnimationRender>(name);
});

BEGIN_INTERFACES(SpriteAnimationRender)
	HAS_INTERFACE(I2dRenderComponent)
END_INTERFACES()

SpriteAnimationRender::SpriteAnimationRender()
	: _type(ff::POSE_TWEEN_LINEAR_LOOP)
	, _pos(ff::GetIdentitySpritePos())
	, _priority(LAYER_PRI_NORMAL)
	, _useTimeBank(false)
{
}

SpriteAnimationRender::~SpriteAnimationRender()
{
}

// static
SpriteAnimationRender *SpriteAnimationRender::Create(
	IEntity* entity,
	ff::ISpriteAnimation* pAnim,
	ff::AnimTweenType type,
	int priority)
{
	ff::ComPtr<SpriteAnimationRender, I2dRenderComponent> pComp;
	assertRetVal(entity->CreateComponent<SpriteAnimationRender>(&pComp), nullptr);
	assertRetVal(pComp->Init(pAnim, type, priority), false);

	entity->AddComponent<I2dRenderComponent>(pComp);

	return pComp;
}

bool SpriteAnimationRender::Init(ff::ISpriteAnimation *pAnim, ff::AnimTweenType type, int priority)
{
	assertRetVal(pAnim, false);

	_anim = pAnim;
	_type = type;
	_priority = priority;

	return true;
}

const ff::SpritePos &SpriteAnimationRender::GetPos() const
{
	return _pos;
}

ff::SpritePos &SpriteAnimationRender::GetPos()
{
	return _pos;
}

float SpriteAnimationRender::GetFrame() const
{
	return _pos._frame;
}

void SpriteAnimationRender::SetFrame(float frame)
{
	_pos._frame = frame;
}

ff::ISpriteAnimation *SpriteAnimationRender::GetAnim() const
{
	return _anim;
}

void SpriteAnimationRender::SetAnim(ff::ISpriteAnimation *pAnim, ff::AnimTweenType type)
{
	assertRet(pAnim);

	_anim = pAnim;
	_type = type;
}

ff::AnimTweenType SpriteAnimationRender::GetTweenType() const
{
	return _type;
}

void SpriteAnimationRender::SetTweenType(ff::AnimTweenType type)
{
	_type = type;
}

void SpriteAnimationRender::SetUseTimeBank(bool bUseTimeBank)
{
	_useTimeBank = bUseTimeBank;
}

int SpriteAnimationRender::Get2dRenderPriority() const
{
	return _priority;
}

void SpriteAnimationRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	IPositionComponent* pPos = entity->GetComponent<IPositionComponent>();
	ff::PointFloat pos = _pos._translate;
	ff::PointFloat scale = _pos._scale;
	float rotate = _pos._rotate;

	if (pPos)
	{
		if (_useTimeBank)
		{
			ff::MetroGlobals *app = ff::MetroGlobals::Get();
			pos += pPos->GetPos((float)app->GetGlobalTime()._bankScale);
		}
		else
		{
			pos += pPos->GetPos();
		}

		scale *= pPos->GetScale();
		rotate += pPos->GetRotate();
	}

	_anim->Render(render, _type, _pos._frame, pos, &scale, rotate, &_pos._color);
}
