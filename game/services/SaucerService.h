#pragma once

class IEntity;

class __declspec(uuid("4f293d3e-84df-4b04-ae93-f1b2883dd59a"))
	ISaucerService : public IUnknown
{
public:
	virtual size_t GetSaucerCount() const = 0;
	virtual IEntity* GetSaucer(size_t index) const = 0;
};
