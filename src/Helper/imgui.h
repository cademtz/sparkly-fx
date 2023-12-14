#pragma once
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <shtypes.h>

namespace Helper
{
    namespace stdfs = std::filesystem;

    class KeyBind
    {
    public:
        void OnMenu(const char* Label);
        /// @brief Read the IMGUI key states and update the "IsDown" state.
        /// @return `true` if the key was pressed down for the first time.
        bool Poll();
        bool IsDown() const { return m_is_down; }
        bool IsBound() const { return m_num_keys; }
    
    private:
        static const int MAX_KEYS = 4;

        /// @brief Format up to 4 keys in the form 'a + b + c', or a default string when `num_keys == 0`
        /// @param num_keys 0 <= num_keys <= MAX_KEYS
        static void KeysToString(char* buffer, size_t buffer_len, const ImGuiKey* keys, uint8_t num_keys);

        /// @brief IMGUI key codes
        ImGuiKey m_keys[MAX_KEYS];
        uint8_t m_num_keys = 0;
        bool m_was_down = false;
        bool m_is_down = false;
    };
    
    void ImGuiHelpMarker(const char* desc);
    ImVec2 CalcListBoxSize(size_t num_items);
    static void PushStyleCompact()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
    }
    static void PopStyleCompact() {
        ImGui::PopStyleVar(2);
    }

    /// @brief Run the system's file dialog. Either open/save and either file/folder.
    /// @param filter (optional) A null-terminated array of whitelisted file types
    std::optional<stdfs::path> OpenFileDialog(
        const wchar_t* title = nullptr, const stdfs::path* initial_path = nullptr, const COMDLG_FILTERSPEC* filter = nullptr,
        bool folder_dialog = false, bool save_dialog = false
    );

    /// @brief Run the system's file dialog. Either open/save and either file/folder.
    /// @param filter (optional) A null-terminated array of whitelisted file types
    static inline std::optional<stdfs::path> OpenFolderDialog(
        const wchar_t* title = nullptr, const stdfs::path* initial_path = nullptr, const COMDLG_FILTERSPEC* filter = nullptr,
        bool save_dialog = false
    ) {
        return OpenFileDialog(title, initial_path, filter, true, false);
    }

    /// @brief Run the system's "Save file" dialog
    /// @param filter (optional) A null-terminated array of  whitelisted file types
    static inline std::optional<stdfs::path> SaveFileDialog(
        const wchar_t* title = nullptr, const stdfs::path* initial_path = nullptr, const COMDLG_FILTERSPEC* filter = nullptr,
        bool folder_dialog = false
    ) {
        return OpenFileDialog(title, initial_path, filter, folder_dialog, true);
    }
}