#pragma once
#include "Base.h"
#include "fnv1a.h"
#include <unordered_map>
#include <functional>

#define DECL_EVENT(name) constexpr EventHandle name = { #name##_hash }

class CEventCallback;
typedef std::function<int()> CallbackFunc_t;

struct EventHandle {
	const uint32_t hash;
};

enum EEventReturnFlags
{
	Return_Normal = 0,
	Return_Skip = (1 << 0), // - Skip calling other modules/listeners after the current function returns
	Return_NoOriginal = (1 << 1), // - For hooks. Prevents calling the original function
};

class CBaseEvent
{
public:
	CBaseEvent(const EventHandle Event) : m_hash(Event.hash) { m_events[m_hash] = this; }
	~CBaseEvent() { m_events.erase(m_hash); }

	CEventCallback* AddCallback(const CallbackFunc_t& Func);
	inline const uint32_t hash() const { return m_hash; }
	int Push();

	static CBaseEvent* GetEvent(const EventHandle Event) { return m_events[Event.hash]; }
	/// @brief Permanently stop all (currently-registered) events.
	/// Part of a hack to help "eject" the cheat.
	static void ShutdownAll() {
		for (auto entry : m_events)
			entry.second->m_is_closed = true;
	}

protected:
	friend CEventCallback;
	inline void RemoveCallback(CEventCallback* Callback) { m_listeners.remove(Callback); }

private:
	CBaseEvent(CBaseEvent&&);

	uint32_t m_hash;
	bool m_is_closed = false;
	std::list<CEventCallback*> m_listeners;
	inline static std::unordered_map<uint32_t, CBaseEvent*> m_events;
};

class CEventManager
{
public:
	~CEventManager();

	inline void RegisterEvent(const EventHandle Event) { m_events.push_back(new CBaseEvent(Event)); }

	inline int PushEvent(const EventHandle Event) { return CBaseEvent::GetEvent(Event)->Push(); }

private:
	unsigned m_evenstate = 0;
	std::list<CBaseEvent*> m_events;
};

class CEventCallback
{
public:
	~CEventCallback() { m_event->RemoveCallback(this); }

	const CallbackFunc_t& Func() const { return m_func; }

protected:
	friend CBaseEvent;
	CEventCallback(CBaseEvent* Event, const CallbackFunc_t& Function)
		: m_event(Event), m_func(Function) { }

private:
	CBaseEvent* m_event;
	CallbackFunc_t m_func;
};