#pragma once
#include <Modules/BaseModule.h>
#include <iosfwd>
#include <optional>
#include <nlohmann/json_fwd.hpp>

DECL_EVENT(EVENT_CONFIG_SAVE);
DECL_EVENT(EVENT_CONFIG_LOAD);

/**
 * @brief The global load/save-config feature
 */
class ConfigModule : public CModule, CEventManager
{
public:
    ConfigModule();

    static bool Save(std::ostream& output);
    static bool Save(const std::filesystem::path& path);
    static bool Load(std::istream& input);
    static bool Load(const std::filesystem::path& path);

    // TODO: Rewrite xsdk-base's event system to accept arguments instead of relying on the following...

    /// @brief The config input. Call during @ref EVENT_CONFIG_LOAD.
    static const nlohmann::json& GetInput();
    /// @brief The config output. Call during @ref EVENT_CONFIG_SAVE.
    static nlohmann::json& GetOutput();

protected:
    void StartListening() override;

private:
    int OnMenu();
    /// @brief Autosave happen in here to keep everything in the UI thread
    int OnPresent();
    static inline int autosave_mins = 1;
    static inline bool autosave = true;
};
