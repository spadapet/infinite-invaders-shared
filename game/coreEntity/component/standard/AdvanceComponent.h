#pragma once

class IEntity;

class __declspec(uuid("c768e20d-6a59-42c7-9741-44b174258490")) __declspec(novtable)
	IAdvanceComponent : public IUnknown
{
public:
	virtual int GetAdvancePriority() const = 0;
	virtual void Advance(IEntity *entity) = 0;
};
