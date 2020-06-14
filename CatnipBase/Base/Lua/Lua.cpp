#include "Lua.h"
#include "Base/Interfaces.h"
#include "Exports/ExportedInterfaces.h"

void CLua::Init()
{
	m_state = lua_open();
	luaL_openlibs(m_state);

	auto L = m_state;

	luabridge::getGlobalNamespace(L)
		.beginNamespace("Game")
		.beginClass<ExportedEngine>("Engine")
		.addFunction("ClientCmd", &ExportedEngine::ClientCmd_Unrestricted)
		.addFunction("GetAppID", &ExportedEngine::GetAppID)
		.endClass()
		.beginClass<ExportedInterfaces>("InterfaceClass")
		.addFunction("GetEngine", &ExportedInterfaces::GetExportedEngine)
		.endClass()
		.addVariable("Interfaces", &g_ExportedInterfaces, false)
		.endNamespace();
}

RunResult_t CLua::Execute(const char* Code)
{
	if (luaL_dostring(m_state, Code) != 0)
	{
		const char* error = luaL_checkstring(m_state, -1);

#ifdef _DEBUG
		printf("Lua Error: %s\n", error);
#endif

		return { false, error };
	}
	return { true, NULL };
}
