#pragma once
#include "BuildConfig.h"

#ifdef BUILDCFG_LUA
#include "Modules/Module.h"
#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#pragma comment(lib, "lua51.lib")

typedef struct {
	bool status;
	const char* error;
} RunResult_t;

class CLua
{
public:
	static void Init();

	static RunResult_t Execute(const char* Code);

	static inline lua_State* m_state;
};

class CLuaEventListener : public CModule
{
public:
	static void Start(lua_State* LuaState);
};

#endif