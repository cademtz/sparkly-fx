#pragma once
#include "BuildConfig.h"

#ifdef BUILDCFG_LUA

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#pragma comment(lib, "lua51.lib")

typedef struct {
	bool status;
	const char* error;
} RunResult_t;

class CLua
{
	static inline lua_State* m_state;
public:
	static void Init();

	static RunResult_t Execute(const char* Code);
};

#endif