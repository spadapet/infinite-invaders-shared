#include "pch.h"
#include "Globals.h"
#include "components\core\PathComponent.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"PathComponent");
	module.RegisterClassT<PathComponent>(name);
});

BEGIN_INTERFACES(PathComponent)
END_INTERFACES()

PathComponent::PathComponent()
	: _type(ff::POSE_TWEEN_LINEAR_LOOP)
	, _fps(1)
	, _frame(0)
{
}

PathComponent::~PathComponent()
{
}

void PathComponent::SetKeys(const Key *pKeys, size_t nKeys, float lastFrame, float fps, ff::AnimTweenType type)
{
	_keys.Clear();

	for (size_t i = 0; i < nKeys; i++)
	{
		_keys.Set(pKeys[i]._frame, DirectX::XMFLOAT4(pKeys[i]._point.x, pKeys[i]._point.y, 0, 0));
	}

	_keys.SetLastFrame(lastFrame);

	_fps = fps;
	_type = type;
	_frame = 0;
}

float PathComponent::GetFPS() const
{
	return _fps;
}

void PathComponent::SetFPS(float fps)
{
	_fps = fps;
}

float PathComponent::GetFrame() const
{
	return _frame;
}

void PathComponent::SetFrame(float frame)
{
	_frame = frame;
}

float PathComponent::GetLastFrame() const
{
	return _keys.GetLastFrame();
}

ff::PointFloat PathComponent::GetPos()
{
	DirectX::XMFLOAT4 pos;
	_keys.Get(_frame, _type, pos);

	return ff::PointFloat(pos.x, pos.y);
}

void PathComponent::AdvanceFrame()
{
	AdvanceFrame(_fps);
}

void PathComponent::AdvanceFrame(float fps)
{
	_frame += fps / Globals::GetAdvancesPerSecondF();

	if ((_type == ff::POSE_TWEEN_LINEAR_LOOP || _type == ff::POSE_TWEEN_SPLINE_LOOP) &&
		GetLastFrame() > 0)
	{
		while (_frame > GetLastFrame())
		{
			_frame -= GetLastFrame();
		}

		while (_frame < 0)
		{
			_frame += GetLastFrame();
		}
	}
}
