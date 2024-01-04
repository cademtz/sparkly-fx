#include "configmodule.h"
#include <cassert>
#include <fstream>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>
#include <Helper/json.h>
#include <Helper/defer.h>
#include <Helper/str.h>
#include <Helper/imgui.h>
#include <Helper/threading.h>
#include <Modules/Menu.h>
#include <Hooks/OverlayHook.h>

#define CURRENT_CONFIG "config.json"
#define CONFIG_VERSION_MAJOR 1
#define CONFIG_VERSION_MINOR 0

static ConfigModule g_config;
static std::optional<nlohmann::json> cfg_root_input;
static std::optional<nlohmann::json> cfg_root_output;
static std::string cfg_errors;
static std::mutex cfg_mutex;
static const COMDLG_FILTERSPEC COM_CONFIG_FILTER[] = {{L"JSON file", L"*.json"}, {L"All files", L"*.*"}, {0}};
static std::filesystem::path CONFIG_PATH;

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

void ConfigModule::StartListening()
{
    CONFIG_PATH = Base::GetModuleDir() / "sparklyfx" / CURRENT_CONFIG;

    Listen(EVENT_MENU, [this]() { return OnMenu(); });
    Listen(EVENT_DX9PRESENT, [this]() { return OnPresent(); });
}

int ConfigModule::OnMenu()
{
    if (!ImGui::CollapsingHeader("Config"))
        return 0;
    ImGui::PushID("Config");
    
    if (ImGui::Button("Reload")) {
        ConfigModule::Load(CONFIG_PATH);
    }
    ImGui::SetItemTooltip("Reload the '" CURRENT_CONFIG "' file");
    ImGui::SameLine();
    if (ImGui::Button("Open..."))
    {
        auto opt_path = Helper::OpenFileDialog(L"Select a config", nullptr, COM_CONFIG_FILTER);
        if (opt_path)
        {
            std::filesystem::copy_file(opt_path.value(), CONFIG_PATH, std::filesystem::copy_options::overwrite_existing);
            ConfigModule::Load(opt_path.value());
        }
    }
    ImGui::SetItemTooltip("Open a different config file.\nThis will overwrite " CURRENT_CONFIG);
    
    if (ImGui::Button("Save"))
        ConfigModule::Save(CONFIG_PATH);
    ImGui::SetItemTooltip("Save to the '" CURRENT_CONFIG "' file");
    ImGui::SameLine();
    if (ImGui::Button("Save as..."))
    {
        auto opt_path = Helper::SaveFileDialog(L"Save the config", nullptr, COM_CONFIG_FILTER);
        if (opt_path)
            ConfigModule::Save(opt_path.value());
    }

    ImGui::Checkbox("Autosave", &autosave); ImGui::SameLine();
    Helper::ImGuiHelpMarker("Make periodic saves of the config");
    int safe_autosave_mins = autosave_mins;
    if (ImGui::InputInt("Interval (minutes)", &safe_autosave_mins))
        autosave_mins = max(1, safe_autosave_mins);
    
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

int ConfigModule::OnPresent()
{
    if (!autosave)
        return;
    
    static auto prev_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - prev_time);
    if (duration.count() >= autosave_mins)
    {
        prev_time = std::chrono::steady_clock::now();
        ConfigModule::Save(CONFIG_PATH);
    }

    return 0;
}

bool ConfigModule::Save(std::ostream& output)
{
    GetError()->clear();
    cfg_root_output = nlohmann::json{};

    // Write our own config data first
    {
        nlohmann::json j = {
            {"autosave", autosave},
            {"autosave_mins", autosave_mins}
        };
        cfg_root_output->emplace("Config", std::move(j));

        // Also write some metadata
        j = {
            {"version_major", CONFIG_VERSION_MAJOR},
            {"version_minor", CONFIG_VERSION_MINOR},
            {"compile_date", __DATE__},
            {"compile_time", __TIME__},
            {
                "description",
                "A configuration file for SparklyFX, a Team Fortress 2 recording software. "
                "More info at https://github.com/cademtz/sparkly-fx/"
            }
        };
        cfg_root_output->emplace("metadata", std::move(j));
    }

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
        AppendError(Helper::sprintf(
            "Failed to create/open config file for writing: '%s'\n", path.u8string().c_str()
        ));
        return false;
    }
    if (!Save(file))
    {
        AppendError(Helper::sprintf(
            "Failed while writing JSON to file: '%s'\n", path.u8string().c_str()
        ));
        return false;
    }
    return true;
}

bool ConfigModule::Load(std::istream& input)
{
    GetError()->clear();
    defer { cfg_root_input = std::nullopt; };
    
    try {
        cfg_root_input = nlohmann::json::parse(input);
    }
    catch (nlohmann::json::parse_error e)
    {
        size_t line, col;
        GetTextPos(input, e.byte, &line, &col);
        AppendError(Helper::sprintf(
            "Failed to parse JSON: Bad syntax near line %zu, col %zu\n", line, col
        ));
        return false;
    }
    catch (std::exception e)
    {
        AppendError(Helper::sprintf("Failed to parse JSON: '%s'\n", e.what()));
        return false;
    }
    
    // Read our own config data first
    {
        // Check the version in metadata
        const nlohmann::json* j = Helper::FromJson(*cfg_root_input, "metadata");
        int version_major, version_minor;
        if (!j || !Helper::FromJson(j, "version", version_major) || !Helper::FromJson(j, "version", version_minor))
        {
            AppendError("Missing or invalid 'metadata' object in config\n");
            return false;
        }

        if (version_major != CONFIG_VERSION_MAJOR)
        {
            AppendError(Helper::sprintf(
                "Unsupported config version. Expected %d, but got %d\n", CONFIG_VERSION_MAJOR, version_major
            ));
            return false;
        }

        // Read the autosave settings, etc
        int safe_autosave_mins = autosave_mins;
        j = Helper::FromJson(*cfg_root_input, "Config");
        Helper::FromJson(j, "autosave", autosave);
        Helper::FromJson(j, "autosave_mins", safe_autosave_mins);
        autosave_mins = min(1, safe_autosave_mins);
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
        AppendError(Helper::sprintf(
            "Failed to open config file for reading: '%s'\n", path.u8string().c_str()
        ));
        return false;
    }
    if (!Load(file))
    {
        AppendError(Helper::sprintf(
            "Failed while parsing config file: '%s'\n", path.u8string().c_str()
        ));
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