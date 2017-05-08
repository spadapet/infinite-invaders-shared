#pragma once

#include "coreEntity\component\standard\AdvanceComponent.h"

class IEntity;

class __declspec(uuid("9a7fbd33-89c9-4121-8a81-5827330ddd5a"))
	LoadingComponent : public ff::ComBase, public IAdvanceComponent
{
public:
	DECLARE_HEADER(LoadingComponent);

	bool IsLoading() const;

	// IAdvanceComponent
	virtual int GetAdvancePriority() const override;
	virtual void Advance(IEntity *entity) override;

private:
	enum ELoadingState
	{
		LS_LOADING,
		LS_ALMOST_DONE,
		LS_DONE,
	};

	ELoadingState _state;
};
