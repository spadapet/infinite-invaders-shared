#include "pch.h"
#include "ThisApplication.h"
#include "components\core\PositionComponent.h"
#include "components\graph\MultiSpriteRender.h"
#include "coreEntity\component\ComponentManager.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "coreEntity\domain\EntityDomain.h"
#include "coreEntity\entity\Entity.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"MultiSpriteRender");
	module.RegisterClassT<MultiSpriteRender>(name);
});

BEGIN_INTERFACES(MultiSpriteRender)
	HAS_INTERFACE(I2dRenderComponent)
END_INTERFACES()

MultiSpriteRender::MultiSpriteRender()
	: _pos(ff::GetIdentitySpritePos())
	, _priority(LAYER_PRI_NORMAL)
{
}

MultiSpriteRender::~MultiSpriteRender()
{
}

// static
MultiSpriteRender *MultiSpriteRender::Create(
	IEntity* entity,
	ff::ISprite** ppSprites,
	size_t nSprites,
	const DirectX::XMFLOAT4* pColors,
	size_t nColors,
	int priority)
{
	assertRetVal(entity, nullptr);

	ff::ComPtr<MultiSpriteRender, I2dRenderComponent> pComp;
	assertRetVal(entity->CreateComponent<MultiSpriteRender>(&pComp), nullptr);
	assertRetVal(pComp->Init(ppSprites, nSprites, pColors, nColors, priority), nullptr);

	entity->AddComponent<I2dRenderComponent>(pComp);

	return pComp;
}

// static
bool MultiSpriteRender::Create(
	IEntityDomain* pDomain,
	ff::ISprite** ppSprites,
	size_t nSprites,
	const DirectX::XMFLOAT4* pColors,
	size_t nColors,
	int priority,
	MultiSpriteRender** ppComp)
{
	assertRetVal(pDomain && ppComp, false);

	ff::ComPtr<MultiSpriteRender, I2dRenderComponent> pComp;
	assertRetVal(pDomain->GetComponentManager()->CreateComponent<MultiSpriteRender>(nullptr, &pComp), false);
	assertRetVal(pComp->Init(ppSprites, nSprites, pColors, nColors, priority), false);

	*ppComp = ff::GetAddRef(pComp.Object());
	return true;
}

bool MultiSpriteRender::Init(ff::ISprite** ppSprites, size_t nSprites, const DirectX::XMFLOAT4* pColors, size_t nColors, int priority)
{
	assertRetVal(ppSprites && nSprites > 0 && nSprites < 4, false);
	assertRetVal((pColors || !nColors) && nColors <= 4, false);

	for (size_t i = 0; i < nSprites; i++)
	{
		assertRetVal(ppSprites[i], false);
		_sprites.Push(ppSprites[i]);
	}

	for (size_t i = 0; i < nColors; i++)
	{
		_colors.Push(pColors[i]);
	}

	_priority = priority;

	return true;
}

const ff::SpritePos &MultiSpriteRender::GetPos() const
{
	return _pos;
}

ff::SpritePos &MultiSpriteRender::GetPos()
{
	return _pos;
}

void MultiSpriteRender::SetSprites(ff::ISprite** ppSprites, size_t nSprites)
{
	assertRet(ppSprites && nSprites > 0 && nSprites < 4);

	for (size_t i = 0; i < nSprites; i++)
	{
		assertRet(ppSprites[i]);
	}

	_sprites.Clear();

	for (size_t i = 0; i < nSprites; i++)
	{
		_sprites.Push(ppSprites[i]);
	}
}

void MultiSpriteRender::SetColors(const DirectX::XMFLOAT4* pColors, size_t nColors)
{
	assertRet(nColors >= 0 && nColors < 4);

	_colors.Clear();

	if (pColors && nColors)
	{
		_colors.Push(pColors, nColors);
	}
}

ff::ISprite *MultiSpriteRender::GetSprite(size_t index)
{
	if (index < _sprites.Size())
	{
		return _sprites[index];
	}

	return nullptr;
}

int MultiSpriteRender::Get2dRenderPriority() const
{
	return _priority;
}

void MultiSpriteRender::Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render)
{
	IPositionComponent* pPos = entity->GetComponent<IPositionComponent>();
	ff::PointFloat pos = _pos._translate;

	if (pPos)
	{
		pos += pPos->GetPos();
	}

	render->DrawMultiSprite(
		_sprites[0].Address(), _sprites.Size(),
		&pos, &_pos._scale, _pos._rotate,
		_colors.Size() ? _colors.Data() : nullptr, _colors.Size());
}
