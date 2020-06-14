#pragma once
#include "BuildConfig.h"
#ifdef BUILDCFG_LUA
#include "Base/fnv1a.h"
#include "Base/Event.h"
#include "Base/Lua/Lua.h"
#include <unordered_map>
#include <list>


struct CallbackInfo_t
{
	uint32_t ident_hash;
	luabridge::LuaRef fn;
};

class CLuaCallbacks
{
public:
	static void AddCallback(const char* EventName, const char* Ident, luabridge::LuaRef Fn);
	static void RemoveCallback(const char* EventName, const char* Ident);

	static void ExecuteCallbacks(const EventHandle Event, luabridge::LuaRef Arguments);

	static inline std::unordered_map<uint32_t, std::list<CallbackInfo_t*>> m_callbacks;
};
#endif