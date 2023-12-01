#pragma once
#include <Modules/BaseModule.h>
#include <optional>
#include <filesystem>

class ConVar;

/**
 * @brief Control "Half-Life Advanced Effects" commands from the GUI
 */
class HlaeUi : public CModule
{
protected:
    void StartListening();

private:
    bool m_is_hlae_loaded = false;
    bool m_campaths_draw = false;
    bool m_autosave = true;
    int m_autosave_mins = 1;
    std::optional<std::filesystem::path> m_prev_campath_save;

    int OnMenu();
    int OnFrameStageNotify();
    /// @brief Find HLAE's commands
    void FindCommands();
    void SaveCamPath(bool save_as = false);
    /// @brief Load a campath
    /// @param path If `nullptr`, a file dialog is opened
    void LoadCamPath(const std::filesystem::path* path = nullptr);
    void WriteNewAutosave();
};

inline HlaeUi g_hlae_ui;