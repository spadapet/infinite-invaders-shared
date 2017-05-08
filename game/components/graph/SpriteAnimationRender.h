#pragma once

#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\component\standard\RenderComponent.h"
#include "Graph\2D\SpritePos.h"

namespace ff
{
	class I2dRenderer;
	class ISpriteAnimation;
	enum AnimTweenType;
}

class IEntity;

class __declspec(uuid("4c567b9a-0cab-439a-ba9f-3acb299042a0"))
	SpriteAnimationRender : public ff::ComBase, public I2dRenderComponent
{
public:
	DECLARE_HEADER(SpriteAnimationRender);

	static SpriteAnimationRender *Create(
		IEntity* entity,
		ff::ISpriteAnimation* pAnim,
		ff::AnimTweenType type,
		int priority = LAYER_PRI_NORMAL);

	const ff::SpritePos& GetPos() const;
	ff::SpritePos& GetPos();

	float GetFrame() const;
	void SetFrame(float frame);

	ff::ISpriteAnimation* GetAnim() const;
	void SetAnim(ff::ISpriteAnimation *pAnim, ff::AnimTweenType type);

	ff::AnimTweenType GetTweenType() const;
	void SetTweenType(ff::AnimTweenType type);

	void SetUseTimeBank(bool bUseTimeBank);

	// I2dRenderComponent
	virtual int Get2dRenderPriority() const override;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) override;

private:
	bool Init(ff::ISpriteAnimation *pAnim, ff::AnimTweenType type, int priority);

	ff::ComPtr<ff::ISpriteAnimation> _anim;
	ff::AnimTweenType _type;
	ff::SpritePos _pos;
	int _priority;
	bool _useTimeBank;
};
