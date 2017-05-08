#pragma once

namespace ff
{
	class I2dRenderer;
}

class I2dLayerGroup;
class IEntity;

class __declspec(uuid("dcfb4fe3-2852-4052-b065-84af945b0ae3")) __declspec(novtable)
	I2dRenderComponent : public IUnknown
{
public:
	virtual int Get2dRenderPriority() const = 0;
	virtual void Render(IEntity *entity, I2dLayerGroup *group, ff::I2dRenderer *render) = 0;
};
