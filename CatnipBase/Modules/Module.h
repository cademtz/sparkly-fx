#pragma once
#include <list>
#include "Base/Event.h"

class CModule
{
public:
	CModule() { m_modules.push_back(this); }
	virtual ~CModule() { m_modules.remove(this); }

protected:
	inline void Listen(const uint32_t Hash, const CallbackFunc_t& Func) {
		CBaseEvent::GetEvent(Hash)->AddCallback(Func);
	}

private:
	static inline std::list<CModule*> m_modules;
	std::list<CEventCallback*> m_callbacks;
};
