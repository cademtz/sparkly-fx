#pragma once
#include "ExportedEngine.h"
#include "Base/Interfaces.h"

class ExportedInterfaces
{
public:
	ExportedEngine GetExportedEngine()
	{
		static ExportedEngine engine(Interfaces::engine);
		return engine;
	}
};
inline ExportedInterfaces g_ExportedInterfaces;