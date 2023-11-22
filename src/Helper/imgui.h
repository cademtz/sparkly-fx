#pragma once
#include <imgui.h>
#include <cstdint>

namespace Helper
{
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
}