#pragma once
#include "Base.h"
#include <list>
#include <functional>

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
	template<size_t N>
	CBaseEvent(const char(& Name)[N]) : m_name(Name) { m_events.push_back(this); }
	~CBaseEvent() { m_events.remove(this); }

	CEventCallback* AddCallback(const CallbackFunc_t& Func);
	inline const char* Name() const { return m_name; }
	int Push();

	template<size_t N>
	static CBaseEvent* GetEvent(const char(&Name)[N])
	{
		for (auto event : m_events)
			if (event->m_name == Name)
				return event;
		FATAL("'%s' was not a registered event", Name);
		return nullptr;
	}

protected:
	friend CEventCallback;
	inline void RemoveCallback(CEventCallback* Callback) { m_listeners.remove(Callback); }

private:
	CBaseEvent(CBaseEvent&&);

	const char* m_name;
	std::list<CEventCallback*> m_listeners;
	inline static std::list<CBaseEvent*> m_events;
};

class CEventManager
{
public:
	~CEventManager();

	template<size_t N>
	inline void RegisterEvent(const char(&Name)[N]) { m_events.push_back(new CBaseEvent(Name)); }

	template<size_t N>
	inline int PushEvent(const char(&Name)[N]) { return CBaseEvent::GetEvent(Name)->Push(); }

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