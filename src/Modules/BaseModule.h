#pragma once
#include <list>
#include <Base/Event.h>

class CModule
{
public:
	CModule() { m_modules.push_back(this); }
	bool IsActive() const { return !m_failed_requirements; }
	virtual ~CModule() { m_modules.remove(this); }

	static void StartAll()
	{
		for (auto mod : m_modules)
			mod->StartListening();
	}

protected:
	virtual void StartListening() = 0;

	void Require();

private:
	static inline std::list<CModule*> m_modules;
	bool m_failed_requirements = false;
};
