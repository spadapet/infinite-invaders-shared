#include "pch.h"
#include "coreEntity\domain\EntityDomainProvider.h"
#include "Module\ModuleFactory.h"
#include "systems\AdvanceSystem.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"AdvanceSystem");
	module.RegisterClassT<AdvanceSystem>(name);
});

BEGIN_INTERFACES(AdvanceSystem)
	HAS_INTERFACE(ISystem)
	HAS_INTERFACE(IComponentListener)
END_INTERFACES()

AdvanceSystem::AdvanceSystem()
{
}

AdvanceSystem::~AdvanceSystem()
{
}

HRESULT AdvanceSystem::_Construct(IUnknown *unkOuter)
{
	ff::ComPtr<IEntityDomainProvider> pDomainProvider;
	assertRetVal(pDomainProvider.QueryFrom(unkOuter), E_INVALIDARG);

	_advanceListener.Init(pDomainProvider->GetDomain(), this);

	return __super::_Construct(unkOuter);
}

int AdvanceSystem::GetSystemPriority() const
{
	return SYS_PRI_ADVANCE_NORMAL;
}

PingResult AdvanceSystem::Ping(IEntityDomain *pDomain)
{
	return PING_RESULT_RUNNING;
}

bool IsValidEntity(IEntity *entity);

void AdvanceSystem::Advance(IEntityDomain *pDomain)
{
	if (_pendingAdd.Size())
	{
		// Flush new entities

		for (size_t i = 0; i < _pendingAdd.Size(); i++)
		{
			_entities.SortInsert(_pendingAdd[i]);
		}

		_pendingAdd.Clear();
	}

	if (_pendingRemove.Size())
	{
		// Flush dead entities

		for (size_t i = 0; i < _pendingRemove.Size(); i++)
		{
			size_t index;
			if (_entities.SortFind(_pendingRemove[i], &index))
			{
				assert(_pendingRemove[i] == _entities[index]);
				_entities.Delete(index);
			}
			else
			{
				assert(false);
			}
		}

		_pendingRemove.Clear();
	}

	// Advance all living entities

	for (size_t i = 0; i < _entities.Size(); i++)
	{
		const EntityAdvanceComp &ea = _entities[i];
		if (_pendingRemove.Find(ea) == ff::INVALID_SIZE)
		{
			ea._component->Advance(ea._entity);
		}
	}
}

void AdvanceSystem::Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget)
{
}

void AdvanceSystem::OnAddComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<IAdvanceComponent> pAdvance;
	assertRet(pAdvance.QueryFrom(pComp));

	_pendingAdd.Push(EntityAdvanceComp(entity, pAdvance, true));
}

void AdvanceSystem::OnRemoveComponent(IEntity *entity, REFGUID compId, IUnknown *pComp)
{
	ff::ComPtr<IAdvanceComponent> pAdvance;
	assertRet(pAdvance.QueryFrom(pComp));

	_pendingRemove.Push(EntityAdvanceComp(entity, pAdvance, true));
}
