#include "pch.h"
#include "Globals.h"
#include "ThisApplication.h"
#include "components\core\LoadingComponent.h"
#include "coreEntity\component\standard\LayerGroupComponent.h"
#include "coreEntity\entity\Entity.h"
#include "Globals\ProcessGlobals.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"LoadingComponent");
	module.RegisterClassT<LoadingComponent>(name);
});

BEGIN_INTERFACES(LoadingComponent)
	HAS_INTERFACE(IAdvanceComponent)
END_INTERFACES()

LoadingComponent::LoadingComponent()
	: _state(LS_LOADING)
{
}

LoadingComponent::~LoadingComponent()
{
}

bool LoadingComponent::IsLoading() const
{
	return _state != LS_DONE;
}

int LoadingComponent::GetAdvancePriority() const
{
	return LAYER_PRI_OVERLAY_NORMAL;
}

void LoadingComponent::Advance(IEntity*) // doesn't need a parent entity
{
	if (_state != LS_DONE)
	{
		bool bLoading = ff::ProcessGlobals::Get()->GetModules().AreResourcesLoading();

		switch (_state)
		{
		case LS_LOADING:
			_state = bLoading ? LS_LOADING : LS_ALMOST_DONE;
			break;

		case LS_ALMOST_DONE:
			_state = bLoading ? LS_LOADING : LS_DONE;
			break;
		}
	}
}
