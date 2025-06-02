#pragma once
#include <Modules/BaseModule.h>
#include <unordered_set>
#include <string>
#include <shared_mutex>

class MiscModule : public CModule
{
    std::shared_mutex m_mtx;
    std::vector<std::string> m_ignored_demo_cmds;

public:
    bool m_should_ignore_demo_cmds = false;

    bool ShouldIgnoreDemoCommand(std::string_view commandline);

protected:
    void StartListening() override;

    int OnTabBar();

    int OnConfigLoad();
    int OnConfigSave();
};

inline MiscModule g_misc_module;