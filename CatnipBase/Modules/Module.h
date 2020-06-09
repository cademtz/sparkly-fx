#pragma once
#include <list>
#include "Base/Event.h"

class CModule
{
public:
	CModule() { m_modules.push_back(this); }
	virtual ~CModule() { m_modules.remove(this); }

protected:
	template<size_t N>
	inline void Listen(const char(&Name)[N], const CallbackFunc_t& Func) {
		CBaseEvent::GetEvent(Name)->AddCallback(Func);
	}

private:
	static inline std::list<CModule*> m_modules;
	std::list<CEventCallback*> m_callbacks;
};
