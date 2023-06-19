#pragma once
#include <list>
#include <Base/Event.h>

class CModule
{
public:
	CModule() { m_modules.push_back(this); }
	bool IsActive() { return !m_failed_requirements; }
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

	void Require();

private:
	static inline std::list<CModule*> m_modules;
	bool m_failed_requirements = false;
	std::list<CEventCallback*> m_callbacks;
};
