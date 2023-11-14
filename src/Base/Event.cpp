#include "Event.h"

CEventCallback* CBaseEvent::AddCallback(const CallbackFunc_t& Func)
{
	m_listeners.push_back(new CEventCallback(this, Func));
	return m_listeners.back();
}

int CBaseEvent::Push()
{
	if (m_is_closed)
		return 0;
	
	int flags = 0;
	for (auto& callback : m_listeners)
	{
		flags |= callback->Func()();
		if (flags & Return_Skip)
			break;
	}
	return flags;
}

CEventManager::~CEventManager()
{
	for (auto event : m_events)
		delete event;
}
