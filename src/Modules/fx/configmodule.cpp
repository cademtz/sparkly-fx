#include "configmodule.h"
#include <cassert>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <Helper/str.h>
#include <Helper/imgui.h>
#include <Helper/threading.h>
#include <Modules/Menu.h>
#include <Base/Interfaces.h>

static ConfigModule g_config;
static std::optional<nlohmann::json> cfg_root_input;
static std::optional<nlohmann::json> cfg_root_output;
static std::string cfg_errors;
static std::mutex cfg_mutex;
static const COMDLG_FILTERSPEC COM_CONFIG_FILTER[] = {{L"JSON file", L"*.json"}, {L"All files", L"*.*"}, {0}};

#define CURRENT_CONFIG "config.json"

/**
 * @brief Get text position based on offset
 * @param istream The input stream. Its current position will be changed. 
 * @param pos Absolute text position, starting from 0
 * @param out_line Receives the line number, starting from 1
 * @param out_column Receives the column number, starting from 1
 */
static void GetTextPos(std::istream& stream, size_t pos, size_t* out_line, size_t* out_column)
{
    stream.seekg(0, std::ios::beg);
    size_t line = 1;
    size_t col = 1;
    size_t count = 0;
    int ch;
    while (count < pos && (ch = stream.get()) != std::char_traits<char>::eof())
    {
        ++col;
        if (ch == '\n')
        {
            ++line;
            col = 1;
        }
    }
    *out_line = line;
    *out_column = col;
}

static Helper::LockedRef<std::string> GetError() {
    return {cfg_errors, cfg_mutex};
}
static void AppendError(std::string_view text) {
    *GetError() += text;
}

ConfigModule::ConfigModule()
{
    RegisterEvent(EVENT_CONFIG_SAVE);
    RegisterEvent(EVENT_CONFIG_LOAD);
}

void ConfigModule::StartListening() {
    Listen(EVENT_MENU, [this]() { return OnMenu(); });
}

int ConfigModule::OnMenu()
{
    if (!ImGui::CollapsingHeader("Config"))
        return 0;
    ImGui::PushID("Config");

    static const std::filesystem::path config_path = Base::GetModuleDir() / "sparklyfx" / CURRENT_CONFIG;
    
    if (ImGui::Button("Reload")) {
        ConfigModule::Load(config_path);
    }
    ImGui::SetItemTooltip("Reload the '" CURRENT_CONFIG "' file");
    ImGui::SameLine();
    if (ImGui::Button("Open..."))
    {
        auto opt_path = Helper::OpenFileDialog(L"Select a config", nullptr, COM_CONFIG_FILTER);
        if (opt_path)
        {
            std::filesystem::copy_file(opt_path.value(), config_path, std::filesystem::copy_options::overwrite_existing);
            ConfigModule::Load(opt_path.value());
        }
    }
    ImGui::SetItemTooltip("Open a different config file.\nThis will overwrite " CURRENT_CONFIG);
    
    if (ImGui::Button("Save"))
        ConfigModule::Save(config_path);
    ImGui::SetItemTooltip("Save to the '" CURRENT_CONFIG "' file");
    ImGui::SameLine();
    if (ImGui::Button("Save as..."))
    {
        auto opt_path = Helper::SaveFileDialog(L"Save the config", nullptr, COM_CONFIG_FILTER);
        if (opt_path)
            ConfigModule::Save(opt_path.value());
    }
    
    if (!GetError()->empty())
    {
        auto& err = GetError();
        ImGui::TextUnformatted("Errors:");
        if (ImGui::Button("Clear##errors"))
            err->clear();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
        ImGui::InputTextMultiline("##errors", err->data(), err->size(), ImVec2(0,0), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
    }

    ImGui::PopID();
    return 0;
}

bool ConfigModule::Save(std::ostream& output)
{
    GetError()->clear();
    cfg_root_output = nlohmann::json{};
    g_config.PushEvent(EVENT_CONFIG_SAVE);
    output << cfg_root_output->dump(2);
    if (!output)
    {
        AppendError("Failed to write config data\n");
        return false;
    }
    cfg_root_output = std::nullopt;
    return true;
}
bool ConfigModule::Save(const std::filesystem::path& path)
{
    std::fstream file{ path, std::ios::out };
    if (!file)
    {
        AppendError(
            Helper::sprintf("Failed to create/open config file for writing: '%s'\n", path.u8string().c_str()).c_str()
        );
        return false;
    }
    if (!Save(file))
    {
        AppendError(
            Helper::sprintf("Failed while writing JSON to file: '%s'\n", path.u8string().c_str()).c_str()
        );
        return false;
    }
    return true;
}

bool ConfigModule::Load(std::istream& input)
{
    GetError()->clear();
    bool failed = true;
    try
    {
        cfg_root_input = nlohmann::json::parse(input);
        failed = false;
    }
    catch (nlohmann::json::parse_error e)
    {
        size_t line, col;
        GetTextPos(input, e.byte, &line, &col);
        AppendError(
            Helper::sprintf("Failed to parse JSON: Bad syntax near line %zu, col %zu\n", line, col).c_str()
        );
    }
    catch (std::exception e)
    {
        AppendError(
            Helper::sprintf("Failed to parse JSON: '%s'\n", e.what()).c_str()
        );
    }

    if (failed)
    {
        cfg_root_input = std::nullopt;
        return false;
    }

    g_config.PushEvent(EVENT_CONFIG_LOAD);
    cfg_root_input = std::nullopt;
    return true;
}
bool ConfigModule::Load(const std::filesystem::path& path)
{
    std::fstream file{ path, std::ios::in };
    if (!file)
    {
        AppendError(
            Helper::sprintf("Failed to open config file for reading: '%s'\n", path.u8string().c_str()).c_str()
        );
        return false;
    }
    if (!Load(file))
    {
        AppendError(
            Helper::sprintf("Failed while parsing config file: '%s'\n", path.u8string().c_str()).c_str()
        );
        return false;
    }
    return true;
}

const nlohmann::json& ConfigModule::GetInput()
{
    assert(cfg_root_input && "Only call this during EVENT_CONFIG_LOAD");
    return *cfg_root_input;
}
nlohmann::json& ConfigModule::GetOutput()
{
    assert(cfg_root_output && "Only call this during EVENT_CONFIG_SAVE");
    return *cfg_root_output;
}