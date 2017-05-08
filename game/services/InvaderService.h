#pragma once

class IEntity;

class __declspec(uuid("8d546df0-6f35-4002-951c-f121904c2441"))
	IInvaderService : public IUnknown
{
public:
	virtual size_t GetInvaderCount() const = 0;
	virtual IEntity* GetInvader(size_t index) const = 0;
};
