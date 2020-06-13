#pragma once
#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

class CLua
{
	lua_State* m_state;
public:
	void Init();
};
inline CLua g_Lua;