#pragma once
#include <list>
#include "Base/Event.h"

class CModule
{
public:
	CModule() { m_modules.push_back(this); }
	virtual ~CModule() { m_modules.remove(this); }

	static void StartAll()
	{
		for (auto mod : m_modules)
			mod->StartListening();
	}

protected:
	virtual void StartListening() = 0;

	inline void Listen(const EventHandle Event, const CallbackFunc_t& Func) {
		CBaseEvent::GetEvent(Event)->AddCallback(Func);
	}

private:
	static inline std::list<CModule*> m_modules;
	std::list<CEventCallback*> m_callbacks;
};
