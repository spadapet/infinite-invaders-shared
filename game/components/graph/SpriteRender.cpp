#include "pch.h"
#include "ThisApplication.h"
#include "coreEntity\entity\Entity.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteRender.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"SpriteRender");
	module.RegisterClassT<SpriteRender>(name);
});

BEGIN_INTERFACES(SpriteRender)
	HAS_INTERFACE(I2dRenderComponent)
END_INTERFACES()

SpriteRender::SpriteRender()
	: _pos(ff::GetIdentitySpritePos())
	, _priority(LAYER_PRI_NORMAL)
{
}

SpriteRender::~SpriteRender()
{
}

// static
SpriteRender *SpriteRender::Create(
	IEntity* entity,
	ff::ISprite* pSprite,
	int priority)
{
	ff::ComPtr<SpriteRender, I2dRenderComponent> pComp;
	assertRetVal(entity->CreateComponent<SpriteRender>(&pComp), nullptr);
	assertRetVal(pComp->Init(pSprite, priority), false);

	entity->AddComponent<I2dRenderComponent>(pComp);

	return pComp;
}

bool SpriteRender::Init(ff::ISprite *pSprite, int priority)
{
	assertRetVal(pSprite, false);

	_sprite = pSprite;
	_priority = priority;

	return true;
}

const ff::SpritePos &SpriteRender::GetPos() const
{
	return _pos;
}

ff::SpritePos &SpriteRender::GetPos()
{
	return _pos;
}

ff::ISprite *SpriteRender::GetSprite() const
{
	return _sprite;
}

void SpriteRender::SetSprite(ff::ISprite *pSprite)
{
	assertRet(pSprite);

	_sprite = pSprite;
}

int SpriteRender::Get2dRenderPriority() const
{
	return _priority;
}

void SpriteRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	IPositionComponent* pPos = entity->GetComponent<IPositionComponent>();
	ff::PointFloat pos = _pos._translate;
	ff::PointFloat scale = _pos._scale;
	float rotate = _pos._rotate;

	if (pPos)
	{
		const PositionInfo &info = pPos->GetPosInfo();
		pos += info._translate;
		scale *= info._scale;
		rotate += info._rotate;
	}

	render->DrawSprite(_sprite, &pos, &scale, rotate, &_pos._color);
}
