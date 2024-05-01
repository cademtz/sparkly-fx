#pragma once
#include <list>
#include <Base/Event.h>

class CModule
{
public:
    CModule() { GetModuleList().push_back(this); }
    bool IsActive() const { return !m_failed_requirements; }
    virtual ~CModule() { GetModuleList().remove(this); }

    static void StartAll()
    {
        for (auto mod : GetModuleList())
            mod->StartListening();
    }

protected:
    virtual void StartListening() = 0;

private:
    static std::list<CModule*>& GetModuleList();
    bool m_failed_requirements = false;
};
