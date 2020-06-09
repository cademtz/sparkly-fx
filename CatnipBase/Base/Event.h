#pragma once
#include <list>
#include <functional>

class CEventCallback;
typedef std::function<void(void*)> CallbackFunc_t;

class CBaseEvent
{
public:
	template<size_t N>
	CBaseEvent(const char(& Name)[N]) : m_name(Name) { m_events.push_back(this); }
	~CBaseEvent() { m_events.remove(this); }

	CEventCallback* AddCallback(const CallbackFunc_t& Func);
	inline const char* Name() const { return m_name; }
	void Push(void* Data);

	template<size_t N>
	static CBaseEvent* GetEvent(const char(&Name)[N])
	{
		for (auto event : m_events)
			if (event->m_name == Name)
				return event;
		return nullptr;
	}

protected:
	friend CEventCallback;
	inline void RemoveCallback(CEventCallback* Callback) { m_listeners.remove(Callback); }

private:
	const char* m_name;
	std::list<CEventCallback*> m_listeners;
	inline static std::list<CBaseEvent*> m_events;
};

class CEventManager
{
public:
	template<size_t N>
	inline void RegisterEvent(const char(&Name)[N]) { m_events.push_back(CBaseEvent(Name)); }

	template<size_t N>
	inline void PushEvent(const char(&Name)[N], void* Data) { CBaseEvent::GetEvent(Name)->Push(Data); }

private:
	std::list<CBaseEvent> m_events;
};

class CEventCallback
{
public:
	~CEventCallback() { m_event->RemoveCallback(this); }

	const std::function<void(void*)>& Func() const { return m_func; }

protected:
	friend CBaseEvent;
	CEventCallback(CBaseEvent* Event, const CallbackFunc_t& Function)
		: m_event(Event), m_func(Function) { }

private:
	CBaseEvent* m_event;
	CallbackFunc_t m_func;
};