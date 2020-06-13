#pragma once
#include "Base.h"
#include <unordered_map>
#include <functional>

#define DECL_EVENT(name) constexpr uint32_t name = #name##_hash

class CEventCallback;
typedef std::function<int()> CallbackFunc_t;

enum EEventReturnFlags
{
	Return_Normal = 0,
	Return_Skip = (1 << 1), // - Skip calling other modules/listeners after the current function returns
	Return_NoOriginal = (1 << 1), // - For hooks. Prevents calling the original function
};

class CBaseEvent
{
public:
	CBaseEvent(const uint32_t Hash) : m_hash(Hash) { m_events[Hash] = this; }
	~CBaseEvent() { m_events.erase(m_hash); }

	CEventCallback* AddCallback(const CallbackFunc_t& Func);
	inline const uint32_t hash() const { return m_hash; }
	int Push();

	static CBaseEvent* GetEvent(const uint32_t Hash)
	{
		return m_events[Hash];
	}

protected:
	friend CEventCallback;
	inline void RemoveCallback(CEventCallback* Callback) { m_listeners.remove(Callback); }

private:
	CBaseEvent(CBaseEvent&&);

	uint32_t m_hash;
	std::list<CEventCallback*> m_listeners;
	inline static std::unordered_map<uint32_t, CBaseEvent*> m_events;
};

class CEventManager
{
public:
	~CEventManager();

	inline void RegisterEvent(const uint32_t Hash) { m_events.push_back(new CBaseEvent(Hash)); }

	inline int PushEvent(const uint32_t Hash) { return CBaseEvent::GetEvent(Hash)->Push(); }

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