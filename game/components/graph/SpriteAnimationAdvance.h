#pragma once

#include "coreEntity\component\standard\AdvanceComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"

class IEntity;

class __declspec(uuid("efd43e7d-0276-4e81-944e-2bfe0148dd65"))
	SpriteAnimationAdvance : public ff::ComBase, public IAdvanceComponent
{
public:
	DECLARE_HEADER(SpriteAnimationAdvance);

	static SpriteAnimationAdvance *Create(IEntity *entity, int priority = LAYER_PRI_NORMAL);

	void SetLoopingAnim(float start, float end, float fps, int loops = -1);
	void SetOneTimeAnim(float start, float end, float fps);
	void SetInfiniteAnim(float start, float fps);

	bool IsDone() const;

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

private:
	bool Init(int priority);
	bool DidLoop() const;
	void UpdateAnims(IEntity *entity);

	int _priority;
	int _loops;
	float _start;
	float _end;
	float _frame;
	float _frameAdvance;
	bool _done;
};
