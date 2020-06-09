#include "Event.h"

CEventCallback* CBaseEvent::AddCallback(const CallbackFunc_t& Func)
{
	m_listeners.push_back(new CEventCallback(this, Func));
	return m_listeners.back();
}

void CBaseEvent::Push(void* Data)
{
	for (auto& callback : m_listeners)
		callback->Func()(Data);
}