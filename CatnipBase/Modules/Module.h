#pragma once

#include <vector>
#include <cassert>

#include <CatnipBase\Hooks\Hooks.h>

class CModule;
class CModuleHandler
{
public:
	void RegisterModule(CModule* Module)
	{
		ModuleList.push_back(Module);
	}

	void ExecuteModule(HookTypes Type)
	{
		for (auto Module : ModuleList)
			Module->Callback(Type);
	}

protected:
	std::vector<CModule*> ModuleList;
};

inline CModuleHandler gModuleHandler;

class CModule
{
public:
	CModule()
	{
		gModuleHandler.RegisterModule(this);
	}

	~CModule() {}

	virtual void Callback(HookTypes Type)
	{
		assert("Unimplemented module callback! Type: %i", Type);
	}
};


