#include "pch.h"
#include "Module/ModuleFactory.h"
#include "Module/Modules.h"
#include "Resource/Resources.h"

ff::Modules::Modules()
{
}

static ff::Module *FindModule(const ff::Vector<std::unique_ptr<ff::Module>> &modules, ff::StringRef name)
{
	for (auto &module: modules)
	{
		if (module->GetName() == name)
		{
			return module.get();
		}
	}

	return nullptr;
}

static ff::Module *FindModule(const ff::Vector<std::unique_ptr<ff::Module>> &modules, REFGUID id)
{
	for (auto &module: modules)
	{
		if (module->GetId() == id)
		{
			return module.get();
		}
	}

	return nullptr;
}

static ff::Module *FindModule(const ff::Vector<std::unique_ptr<ff::Module>> &modules, HINSTANCE instance)
{
	for (auto &module: modules)
	{
		if (module->GetInstance() == instance)
		{
			return module.get();
		}
	}

	return nullptr;
}

const ff::Module *ff::Modules::Get(StringRef name)
{
	// Try and create it
	LockMutex lock(_mutex);

	Module *module = FindModule(_modules, name);
	if (module == nullptr)
	{
		std::unique_ptr<Module> newModule = ModuleFactory::Create(name);
		assertRetVal(newModule != nullptr, nullptr);

		module = newModule.get();
		_modules.Push(std::move(newModule));
		module->FinishInit();
	}

	assert(module);
	return module;
}

const ff::Module *ff::Modules::Get(REFGUID id)
{
	// Try and create it
	LockMutex lock(_mutex);

	Module *module = FindModule(_modules, id);
	if (module == nullptr)
	{
		std::unique_ptr<Module> newModule = ModuleFactory::Create(id);
		assertRetVal(newModule != nullptr, nullptr);

		module = newModule.get();
		_modules.Push(std::move(newModule));
		module->FinishInit();
	}

	assert(module);
	return module;
}

const ff::Module *ff::Modules::Get(HINSTANCE instance)
{
	// Try and create it
	LockMutex lock(_mutex);

	Module *module = FindModule(_modules, instance);
	if (module == nullptr)
	{
		std::unique_ptr<Module> newModule = ModuleFactory::Create(instance);
		assertRetVal(newModule != nullptr, nullptr);

		module = newModule.get();
		_modules.Push(std::move(newModule));
		module->FinishInit();
	}

	assert(module);
	return module;
}

const ff::Module *ff::Modules::GetMain()
{
	HINSTANCE instance = ff::GetMainModuleInstance();
#if !METRO_APP
	if (!instance)
	{
		instance = GetModuleHandle(nullptr);
	}
#endif
	return Get(instance);
}

ff::Vector<HINSTANCE> ff::Modules::GetAllInstances() const
{
	return ModuleFactory::GetAllInstances();
}

bool ff::Modules::AreResourcesLoading() const
{
	LockMutex lock(_mutex);

	for (auto &module : _modules)
	{
		if (module->GetResources()->IsLoading())
		{
			return true;
		}
	}

	return false;

}

const ff::ModuleClassInfo *ff::Modules::FindClassInfo(ff::StringRef name)
{
	LockMutex lock(_mutex);

	// First try modules that were already created
	for (size_t i = 0; i < 2; i++)
	{
		if (i == 1)
		{
			CreateAllModules();
		}

		for (auto &module : _modules)
		{
			const ModuleClassInfo *info = module->GetClassInfo(name);
			if (info != nullptr)
			{
				return info;
			}
		}
	}

	return nullptr;
}

const ff::ModuleClassInfo *ff::Modules::FindClassInfo(REFGUID classId)
{
	LockMutex lock(_mutex);

	// First try modules that were already created
	for (size_t i = 0; i < 2; i++)
	{
		if (i == 1)
		{
			CreateAllModules();
		}

		for (auto &module: _modules)
		{
			const ModuleClassInfo *info = module->GetClassInfo(classId);
			if (info != nullptr)
			{
				return info;
			}
		}
	}

	return nullptr;
}

const ff::ModuleClassInfo *ff::Modules::FindClassInfoForInterface(REFGUID interfaceId)
{
	LockMutex lock(_mutex);

	// First try modules that were already created
	for (size_t i = 0; i < 2; i++)
	{
		if (i == 1)
		{
			CreateAllModules();
		}

		for (auto &module: _modules)
		{
			const ModuleClassInfo *info = module->GetClassInfoForInterface(interfaceId);
			if (info != nullptr)
			{
				return info;
			}
		}
	}

	return nullptr;
}

bool ff::Modules::FindClassFactory(REFGUID classId, IClassFactory **factory)
{
	LockMutex lock(_mutex);

	// First try modules that were already created
	for (size_t i = 0; i < 2; i++)
	{
		if (i == 1)
		{
			CreateAllModules();
		}

		for (auto &module : _modules)
		{
			if (module->GetClassFactory(classId, factory))
			{
				return true;
			}
		}
	}

	return false;
}

const ff::ModuleInterfaceInfo *ff::Modules::FindInterfaceInfo(REFGUID interfaceId)
{
	LockMutex lock(_mutex);

	// First try modules that were already created
	for (size_t i = 0; i < 2; i++)
	{
		if (i == 1)
		{
			CreateAllModules();
		}

		for (auto &module: _modules)
		{
			const ModuleInterfaceInfo *info = module->GetInterfaceInfo(interfaceId);
			if (info != nullptr)
			{
				return info;
			}
		}
	}

	return nullptr;
}

const ff::ModuleCategoryInfo *ff::Modules::FindCategoryInfo(REFGUID categoryId)
{
	LockMutex lock(_mutex);

	// First try modules that were already created
	for (size_t i = 0; i < 2; i++)
	{
		if (i == 1)
		{
			CreateAllModules();
		}

		for (auto &module: _modules)
		{
			const ModuleCategoryInfo *info = module->GetCategoryInfo(categoryId);
			if (info != nullptr)
			{
				return info;
			}
		}
	}

	return nullptr;
}

ff::ComPtr<IUnknown> ff::Modules::CreateParentForCategory(REFGUID categoryId, AppGlobals *context)
{
	const ModuleCategoryInfo *info = FindCategoryInfo(categoryId);

	if (info != nullptr && info->_parentObjectFactory != nullptr)
	{
		ComPtr<IUnknown> parent;
		if (SUCCEEDED(info->_parentObjectFactory(context, &parent)))
		{
			return parent;
		}
	}

	return nullptr;
}

ff::ComPtr<IUnknown> ff::Modules::CreateParentForObject(IUnknown *obj, AppGlobals *context)
{
	ff::ComPtr<ff::IComObject> comObj;
	noAssertRetVal(comObj.QueryFrom(obj), nullptr);
	return CreateParentForCategory(comObj->GetComCategoryID(), context);
}

ff::ComPtr<IUnknown> ff::Modules::CreateParentForClass(ff::StringRef name, AppGlobals *context)
{
	const ModuleClassInfo *info = FindClassInfo(name);
	noAssertRetVal(info, nullptr);
	return CreateParentForCategory(info->_categoryId, context);
}

ff::ComPtr<IUnknown> ff::Modules::CreateClass(ff::StringRef name, AppGlobals *context)
{
	const ff::ModuleClassInfo *info = FindClassInfo(name);
	assertRetVal(info && info->_factory, nullptr);

	ff::ComPtr<IUnknown> obj;
	ff::ComPtr<IUnknown> parent = CreateParentForCategory(info->_categoryId, context);
	assertHrRetVal(info->_factory(parent, info->_classId, __uuidof(IUnknown), (void**)&obj), nullptr);

	return obj;
}

void ff::Modules::Clear()
{
	LockMutex lock(_mutex);

	_modules.ClearAndReduce();
}

void ff::Modules::CreateAllModules()
{
	Vector<HINSTANCE> instances = ModuleFactory::GetAllInstances();
	for (HINSTANCE inst: instances)
	{
		Get(inst);
	}
}
