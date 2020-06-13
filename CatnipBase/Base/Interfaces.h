#pragma once
#include "Wrappers/Wrappers.h"
#include "SDK/icliententitylist.h"

namespace Interfaces
{
	void CreateInterfaces();
	void DestroyInterfaces();

	inline IEngineClientWrapper* engine = nullptr;
	inline IClientDLLWrapper* hlclient = nullptr;
	inline IClientEntityList* entlist = nullptr;
}