#pragma once

namespace ff
{
	class IGraphTexture;
	class IRenderTarget;
	class ISprite;
	class ISpriteFont;
	struct SpritePos;
}

class IEntity;

enum ShieldEntityType
{
	SHIELD_TYPE_NORMAL,
	SHIELD_TYPE_HARD,

	SHIELD_TYPE_COUNT
};

class __declspec(uuid("8cb2c04b-1243-466c-b93a-00be2606dcd3"))
	ShieldComponent : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(ShieldComponent);

	ShieldEntityType GetType() const;
	void SetType(ShieldEntityType type);

	bool HitTest(IEntity *entity, IEntity *otherEntity);
	void Erase(IEntity *entity, IEntity *otherEntity, ff::ISprite *pSprite, const ff::SpritePos &spritePos);

private:
	bool UpdateStagingTexture(IEntity *entity);
	void UpdateSolid(IEntity *entity);

	static const size_t SHIELD_WIDTH = 200;
	static const size_t SHIELD_HEIGHT = 100;

	ShieldEntityType _type;
	bool _bUpdateSolid;
	BYTE _solid[SHIELD_HEIGHT / 2][SHIELD_WIDTH / 2];

	ff::ComPtr<ff::IGraphTexture> _stagingTexture;
	ff::ComPtr<ff::IRenderTarget> _renderTarget;
};
