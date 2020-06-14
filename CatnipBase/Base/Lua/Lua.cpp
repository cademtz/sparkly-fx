#include "Lua.h"
#ifdef BUILDCFG_LUA
#include "Base/Interfaces.h"
#include "Hooks/ClientHook.h"

#include "Exports/ExportedInterfaces.h"

#include "Callbacks/LuaCallbacks.h"

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

void CLuaEventListener::Start(lua_State* LuaState)
{
	// TODO listen for events

	/*
		- Run callbacks like this
		- Table should be automatically deleted once out of scope 

		auto arguments = luabridge::newTable(LuaState);
		arguments["example"] = 123;

		CLuaCallbacks::ExecuteCallbacks(EVENT_*, arguments);
	*/
}

#endif