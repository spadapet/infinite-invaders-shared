#pragma once

#include "Graph\Anim\AnimKeys.h"
#include "Graph\Anim\KeyFrames.h"

class __declspec(uuid("402695db-6603-4dc8-8cf8-7b83ed874344"))
	PathComponent : public ff::ComBase, public IUnknown
{
public:
	DECLARE_HEADER(PathComponent);

	struct Key
	{
		ff::PointFloat _point;
		float _frame;
	};

	void SetKeys(const Key *pKeys, size_t nKeys, float lastFrame, float fps, ff::AnimTweenType type);

	float GetFPS() const;
	void SetFPS(float fps);

	float GetFrame() const;
	void SetFrame(float frame);
	float GetLastFrame() const;
	ff::PointFloat GetPos();

	void AdvanceFrame();
	void AdvanceFrame(float fps);

private:
	ff::KeyFrames<ff::VectorKey> _keys;
	ff::AnimTweenType _type;
	float _fps;
	float _frame;
};
