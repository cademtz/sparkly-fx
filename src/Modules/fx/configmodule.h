#pragma once
#include <filesystem>
#include <Modules/BaseModule.h>
#include <iosfwd>
#include <optional>
#include <nlohmann/json_fwd.hpp>

/**
 * @brief The global load/save-config feature
 */
class ConfigModule : public CModule
{
public:
    static bool Save(std::ostream& output);
    static bool Save(const std::filesystem::path& path);
    static bool Load(std::istream& input);
    static bool Load(const std::filesystem::path& path);

    /// @brief The config input. Call during @ref EVENT_CONFIG_LOAD.
    static const nlohmann::json& GetInput();
    /// @brief The config output. Call during @ref EVENT_CONFIG_SAVE.
    static nlohmann::json& GetOutput();

    static inline EventSource<void()> OnConfigSave;
    static inline EventSource<void()> OnConfigLoad;

protected:
    void StartListening() override;

private:
    int OnMenu();
    /// @brief Autosave happen in here to keep everything in the UI thread
    int OnPresent();
    static inline int autosave_mins = 1;
    static inline bool autosave = true;
};
