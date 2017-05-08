#pragma once

#include "coreEntity\component\standard\RenderComponent.h"
#include "Graph\2D\SpritePos.h"

namespace ff
{
	class I2dRenderer;
	class ISprite;
}

class IEntity;

class __declspec(uuid("9f0216f8-f93b-4c8f-a996-a9b5c49935a7"))
	MultiSpriteRender : public ff::ComBase, public I2dRenderComponent
{
public:
	DECLARE_HEADER(MultiSpriteRender);

	static MultiSpriteRender *Create(
		IEntity* entity,
		ff::ISprite** ppSprites,
		size_t nSprites,
		const DirectX::XMFLOAT4* pColors,
		size_t nColors,
		int priority);

	static bool Create(
		IEntityDomain* pDomain,
		ff::ISprite** ppSprites,
		size_t nSprites,
		const DirectX::XMFLOAT4* pColors,
		size_t nColors,
		int priority,
		MultiSpriteRender** ppComp);

	const ff::SpritePos& GetPos() const;
	ff::SpritePos& GetPos();

	void SetSprites(ff::ISprite** ppSprites, size_t nSprites);
	void SetColors(const DirectX::XMFLOAT4* pColors, size_t nColors);

	ff::ISprite* GetSprite(size_t index);

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

private:
	bool Init(ff::ISprite** ppSprites, size_t nSprites, const DirectX::XMFLOAT4* pColors, size_t nColors, int priority);

	ff::Vector<ff::ComPtr<ff::ISprite>, 4> _sprites;
	ff::Vector<DirectX::XMFLOAT4, 4> _colors;
	ff::SpritePos _pos;
	int _priority;
};
