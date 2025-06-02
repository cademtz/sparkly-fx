#include "miscmodule.h"

#include "configmodule.h"
#include "imgui.h"
#include "Helper/json.h"

#include "mainwindow.h"
#include "configmodule.h"

static std::vector<std::string> SuggestedIgnoredCommands =
{
    "+taunt",
    "cl_decline_first_notification",
    "cl_trigger_first_notification",
    "show_quest_log",
    "save_replay",
    "abuse_report_queue",
    "quit prompt",
};

std::string_view Trimmed(std::string_view str)
{
    if (str.empty())
        return str;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (!std::isspace(str[i]))
        {
            str = str.substr(i);
            break;
        }
    }

    for (size_t i = str.size() - 1; i > 0; i--)
    {
        if (!std::isspace(str[i]))
        {
            str = str.substr(0, i + 1);
            break;
        }
    }

    return str;
}

void MiscModule::StartListening()
{
    MainWindow::OnTabBar.Listen(&MiscModule::OnTabBar, this);
    ConfigModule::OnConfigLoad.Listen(&MiscModule::OnConfigLoad, this);
    ConfigModule::OnConfigSave.Listen(&MiscModule::OnConfigSave, this);
}

int MiscModule::OnTabBar()
{
    if (!ImGui::BeginTabItem("Misc"))
        return 0;

    std::scoped_lock _lock(m_mtx);

    static char inputbuf[256] = {};

    ImGui::Checkbox("Enable Demo Command Filtering", &m_should_ignore_demo_cmds);
    ImGui::TextDisabled("When enabled, the commands listed below will be ignored during demo playback.");

    if (!m_should_ignore_demo_cmds) ImGui::BeginDisabled();

    static size_t selectedIdx = -1;
    static std::string* selectedCmd = nullptr;

    const auto clearInput = [&]
    {
        selectedIdx = -1;
        selectedCmd = nullptr;
        inputbuf[0] = 0;
    };

    const auto select = [&](size_t idx)
    {
        selectedIdx = idx;
        selectedCmd = &m_ignored_demo_cmds[idx];
        std::strcpy(inputbuf, selectedCmd->c_str());
    };

    if (ImGui::Button("Add Suggested Commands"))
    {
        m_ignored_demo_cmds = SuggestedIgnoredCommands;
        clearInput();
    }

    if (ImGui::BeginListBox("Ignored Commands"))
    {
        for (size_t i = 0; i < m_ignored_demo_cmds.size(); i++)
        {
            std::string& item = m_ignored_demo_cmds[i];
            if (ImGui::Selectable(item.c_str(), item.compare(inputbuf) == 0))
                select(i);
        }

        ImGui::EndListBox();
    }

    bool submit = ImGui::InputText(
        "##command_input",
        inputbuf,
        sizeof(inputbuf),
        ImGuiInputTextFlags_EnterReturnsTrue
    );

    bool isEditing = selectedCmd && selectedCmd->compare(inputbuf) == 0;

    if (ImGui::Button(isEditing ? "Save" : "Add"))
        submit = true;

    ImGui::SameLine();

    bool remove = isEditing && ImGui::Button("Remove");

    ImGui::SameLine();

    if (submit && inputbuf[0] != '\0')
    {
        if (isEditing && selectedCmd)
            *selectedCmd = Trimmed(inputbuf);
        else
            m_ignored_demo_cmds.emplace_back(Trimmed(inputbuf));

        clearInput();
    }
    else if (remove && selectedIdx != -1)
    {
        auto it = m_ignored_demo_cmds.erase(m_ignored_demo_cmds.begin() + selectedIdx);
        if (it != m_ignored_demo_cmds.end())
            select(std::distance(m_ignored_demo_cmds.begin(), it));
        else
            clearInput();
    }

    if (!m_should_ignore_demo_cmds) ImGui::EndDisabled();

    ImGui::EndTabItem();
    return 0;
}

bool MiscModule::ShouldIgnoreDemoCommand(std::string_view commandline)
{
    if (!m_should_ignore_demo_cmds)
        return false;

    char buf[256] = {};
    if (commandline.size() > sizeof(buf))
    {
        printf("commandline.size() > %d\n", (int) sizeof(buf));
        return false;
    }

    // Copy lowercased commandline into buf
    for (int i = 0; i < commandline.size(); i++)
        buf[i] = std::tolower(commandline[i]);

    std::string_view haystack = Trimmed(std::string_view{ buf, commandline.size() });

    std::shared_lock _lock(m_mtx);
    for (const std::string& needle: m_ignored_demo_cmds)
    {
        if (haystack.starts_with(needle))
            return true;
    }

    return false;
}

int MiscModule::OnConfigLoad()
{
    const nlohmann::json* j = Helper::FromJson(ConfigModule::GetInput(), "Misc");
    if (!j)
        return 0;

    std::scoped_lock _lock(m_mtx);

    Helper::FromJson(j, "m_should_ignore_demo_cmds", m_should_ignore_demo_cmds);
    Helper::FromJson(j, "m_ignored_demo_cmds", m_ignored_demo_cmds);

    // Remove empty commands
    for (auto it = m_ignored_demo_cmds.begin(); it != m_ignored_demo_cmds.end();)
    {
        if ((*it).empty())
            it = m_ignored_demo_cmds.erase(it);
        else
            it++;
    }

    return 0;
}

int MiscModule::OnConfigSave()
{
    std::shared_lock _lock(m_mtx);

    nlohmann::json j = {
        { "m_should_ignore_demo_cmds", m_should_ignore_demo_cmds },
        { "m_ignored_demo_cmds", m_ignored_demo_cmds }
    };

    ConfigModule::GetOutput().emplace("Misc", std::move(j));
    return 0;
}

