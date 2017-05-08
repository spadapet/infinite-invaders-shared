#pragma once

class IEntityDomain;

class __declspec(uuid("face281a-3cdf-41d5-9dca-e3f331933efd"))
	IEntityDomainProvider : public IUnknown
{
public:
	virtual IEntityDomain* GetDomain() const = 0;
};
