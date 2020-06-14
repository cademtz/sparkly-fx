#include "LuaCallbacks.h"
#ifdef BUILDCFG_LUA

#include "Hooks/ClientHook.h"

void CLuaCallbacks::AddCallback(const char* EventName, const char* Ident, luabridge::LuaRef Fn)
{
	uint32_t EventHash = fnv::calculate(EventName);
	uint32_t IdentHash = fnv::calculate(Ident);

	m_callbacks[EventHash].push_back(new CallbackInfo_t{ IdentHash, Fn });

#if defined(_DEBUG) and defined(BUILDCFG_VERBOSE_LOG)
	printf("Registered %s to %s\n", EventName, Ident);
#endif
}

void CLuaCallbacks::RemoveCallback(const char* EventName, const char* Ident)
{
	uint32_t EventHash = fnv::calculate(EventName);
	uint32_t IdentHash = fnv::calculate(Ident);

	if (m_callbacks.count(EventHash) > 0)
	{
		auto& callbacks = m_callbacks.at(EventHash);

		for (auto it = callbacks.rbegin(); it != callbacks.rend(); it++)
		{
			auto info = (*it);

			if (info->ident_hash != IdentHash)
				continue;

			callbacks.erase(it.base());
			delete info;
		}
	}
#if defined(_DEBUG) and defined(BUILDCFG_VERBOSE_LOG)
	else
		printf("Lua tried to remove invalid callback: %s\n", EventName);
#endif
}

void CLuaCallbacks::ExecuteCallbacks(const EventHandle Event, luabridge::LuaRef Arguments)
{
	auto& callbacks = m_callbacks.at(Event.hash);

	for (auto& info : callbacks)
		info->fn(Arguments);
}

#endif