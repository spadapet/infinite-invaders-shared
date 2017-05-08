#include "pch.h"
#include "ThisApplication.h"
#include "components\shield\ShieldComponent.h"
#include "components\core\PositionComponent.h"
#include "components\graph\SpriteRender.h"
#include "coreEntity\entity\Entity.h"
#include "Globals\MetroGlobals.h"
#include "Graph\2D\2dEffect.h"
#include "Graph\2D\2dRenderer.h"
#include "Graph\2D\Sprite.h"
#include "Graph\RenderTarget\RenderTarget.h"
#include "Graph\GraphDevice.h"
#include "Graph\GraphTexture.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"ShieldComponent");
	module.RegisterClassT<ShieldComponent>(name);
});

BEGIN_INTERFACES(ShieldComponent)
END_INTERFACES()

ShieldComponent::ShieldComponent()
	: _type(SHIELD_TYPE_NORMAL)
	, _bUpdateSolid(true)
{
	ff::ZeroObject(_solid);
}

ShieldComponent::~ShieldComponent()
{
}

ShieldEntityType ShieldComponent::GetType() const
{
	return _type;
}

void ShieldComponent::SetType(ShieldEntityType type)
{
	_type = type;
}

bool ShieldComponent::HitTest(IEntity *entity, IEntity *otherEntity)
{
	ff::RectFloat shieldRect = entity->GetComponent<IPositionComponent>()->GetBounds();

	IPositionComponent* pOtherPos = otherEntity->GetComponent<IPositionComponent>();
	const ff::PointFloat* pOtherPoints = pOtherPos->GetPoints();
	ff::RectFloat otherBounds = pOtherPos->GetBounds();

	otherBounds.Intersect(shieldRect);
	otherBounds.Offset(-shieldRect.left, -shieldRect.top);

	ff::RectInt iOtherRect((int)otherBounds.left, (int)otherBounds.top, (int)otherBounds.right, (int)otherBounds.bottom);
	bool bHit = false;

	UpdateSolid(entity);

	for (int y = iOtherRect.top; !bHit && y < iOtherRect.bottom && y < SHIELD_HEIGHT; y += 2)
	{
		for (int x = iOtherRect.left; !bHit && x < iOtherRect.right && x < SHIELD_WIDTH; x += 2)
		{
			if (_solid[y / 2][x / 2] > 2) // 3 or 4 pixels are set
			{
				bHit = pOtherPos->HitTest(ff::PointFloat(shieldRect.left + x, shieldRect.top + y));
			}
		}
	}

	return bHit;
}

void ShieldComponent::Erase(IEntity *entity, IEntity *otherEntity, ff::ISprite *pSprite, const ff::SpritePos &spritePos)
{
	assertRet(entity);

	if (!_renderTarget)
	{
		SpriteRender* pSpriteRender = entity->GetComponent<SpriteRender>();
		assertRet(pSpriteRender && pSpriteRender->GetSprite());

		ff::IGraphTexture* pTexture = pSpriteRender->GetSprite()->GetSpriteData()._texture;

		if (!_renderTarget)
		{
			assertRet(ff::CreateRenderTargetTexture(pTexture->GetDevice(), pTexture, 0, 1, 0, &_renderTarget));
		}
	}

	IPositionComponent* pPos = entity->GetComponent<IPositionComponent>();
	const ff::RectFloat& bounds = pPos->GetBounds();
	ff::PointInt targetSize = _renderTarget->GetRotatedSize();
	ff::MetroGlobals *pApp = ff::MetroGlobals::Get();

	assertRet(bounds.Width() && bounds.Height());

	// Draw the sprite to the shield texture
	if (pApp->Get2dRender()->BeginRender(
			_renderTarget, nullptr,
			ff::RectFloat(0, 0, bounds.Width(), bounds.Height()),
			pPos->GetPos(),
			ff::PointFloat(bounds.Width() / (float)targetSize.x, bounds.Height() / (float)targetSize.y),
			pApp->Get2dEffect()))
	{
		pApp->Get2dEffect()->PushDrawType((ff::DrawType2d)(ff::DRAW_BLEND_INV_MUL | ff::DRAW_DEPTH_DISABLE));

		if (pSprite)
		{
			pApp->Get2dRender()->DrawSprite(pSprite, &spritePos._translate, &spritePos._scale, spritePos._rotate, &spritePos._color);
		}
		else
		{
			ff::RectFloat otherBounds = otherEntity->GetComponent<IPositionComponent>()->GetBounds();
			otherBounds.Deflate(-1, -1); // prevents infinite collision

			pApp->Get2dRender()->DrawFilledRectangle(&otherBounds, nullptr, 0);
		}

		pApp->Get2dRender()->Flush();
		pApp->Get2dEffect()->PopDrawType();
		pApp->Get2dRender()->EndRender();
	}

	UpdateStagingTexture(entity);

	_bUpdateSolid = true;
}

bool ShieldComponent::UpdateStagingTexture(IEntity *entity)
{
	// Copy the current sprite to a staging texture (to read it on the CPU)
	SpriteRender* pSpriteRender = entity->GetComponent<SpriteRender>();
	assertRetVal(pSpriteRender && pSpriteRender->GetSprite(), false);

	if (!_stagingTexture)
	{
		ff::IGraphTexture* pTexture = pSpriteRender->GetSprite()->GetSpriteData()._texture;

		assertRetVal(CreateStagingTexture(
			pTexture->GetDevice(),
			pTexture->GetSize(),
			DXGI_FORMAT_R8G8B8A8_UNORM,
			true, false,
			&_stagingTexture), false);
	}

	assertRetVal(pSpriteRender->GetSprite()->GetSpriteData().CopyTo(_stagingTexture, ff::PointInt(0, 0)), false);

	return true;
}

void ShieldComponent::UpdateSolid(IEntity *entity)
{
	if (_bUpdateSolid)
	{
		_bUpdateSolid = false;
		ff::ZeroObject(_solid);

		assertRet(_stagingTexture || UpdateStagingTexture(entity));

		D3D11_MAPPED_SUBRESOURCE map;
		ID3D11DeviceContext *pContext = _stagingTexture->GetDevice()->GetContext();
		assertRet(SUCCEEDED(pContext->Map(_stagingTexture->GetTexture(), 0, D3D11_MAP_READ, 0, &map)));

		ff::PointInt size = _stagingTexture->GetSize();
		const BYTE *pData = (const BYTE *)map.pData;

		for (int y = 0; y < size.y && y < SHIELD_HEIGHT; y++)
		{
			const BYTE *pColor = &pData[y * map.RowPitch + 3]; // alpha is three bytes in

			for (int x = 0; x < size.x && x < SHIELD_WIDTH; x++, pColor += 4)
			{
				if (*pColor > 127) // more than half alpha
				{
					_solid[y / 2][x / 2]++;
				}
			}
		}

		pContext->Unmap(_stagingTexture->GetTexture(), 0);
	}
}
