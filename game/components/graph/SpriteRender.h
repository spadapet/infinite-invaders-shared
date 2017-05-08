#pragma once

#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "Graph\2D\SpritePos.h"

namespace ff
{
	class I2dRenderer;
	class ISprite;
}

class IEntity;

class __declspec(uuid("9a4e3847-790f-44ac-b55b-669fa884d08d"))
	SpriteRender : public ff::ComBase, public I2dRenderComponent
{
public:
	DECLARE_HEADER(SpriteRender);

	static SpriteRender *Create(
		IEntity* entity,
		ff::ISprite* pSprite,
		int priority = LAYER_PRI_NORMAL);

	const ff::SpritePos& GetPos() const;
	ff::SpritePos& GetPos();

	ff::ISprite* GetSprite() const;
	void SetSprite(ff::ISprite *pSprite);

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

private:
	bool Init(ff::ISprite *pSprite, int priority);

	ff::ComPtr<ff::ISprite> _sprite;
	ff::SpritePos _pos;
	int _priority;
};
