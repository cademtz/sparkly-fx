#pragma once
#include "Wrappers/Wrappers.h"

namespace Interfaces
{
	void CreateInterfaces();
	void DestroyInterfaces();

	inline IEngineClientWrapper* engine = nullptr;
}