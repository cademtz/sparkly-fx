#include "imgui.h"
#include <array>
#include <cstdio>
#include <cassert>

namespace Helper
{

void KeyBind::OnMenu(const char* Label)
{
    const char* const popup_id = "popup";
    const int btn_id = 0;

    static ImGuiKey new_keys[MAX_KEYS];
    static uint8_t num_new_keys = 0;
    std::array<char, 64> textbuf;

    ImGui::PushID(Label);

    KeysToString(textbuf.data(), textbuf.size(), m_keys, m_num_keys);
    ImGui::PushID(btn_id);
    ImVec2 btn_size = ImVec2(0,0);
    if (strlen(textbuf.data()) < 3) // Set a good minimum size, that is easy to click
        btn_size = ImVec2(ImGui::GetTextLineHeight() * 3, 0);
    bool btn_pressed = ImGui::Button(textbuf.data(), btn_size);
    ImGui::PopID();

    if (btn_pressed)
    {
        num_new_keys = 0;
        ImGui::OpenPopup(popup_id);
    }

    ImGui::SameLine();
    ImGui::Text(Label);

    if (ImGui::BeginPopup(popup_id))
    {
        // The logic to update the `new_keys` must occur *after* all these controls run.
        // This way, we can avoid button presses as part of the new keybind.

        ImGui::Text("Press one or more keys...");

        KeysToString(textbuf.data(), textbuf.size(), new_keys, num_new_keys);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
        ImGui::Text(textbuf.data(), textbuf.size());
        ImGui::PopStyleColor();

        bool ok = ImGui::Button("Ok"); ImGui::SameLine();
        bool cancel = ImGui::Button("Cancel"); ImGui::SameLine();
        bool unbind = ImGui::Button("Unbind");

        if (ok)
        {
            memcpy(m_keys, new_keys, sizeof(new_keys));
            m_num_keys = num_new_keys;
            m_is_down = false;
            m_was_down = false;
        }
        if (unbind)
        {
            m_num_keys = 0;
            num_new_keys = 0;
        }

        if (ok || cancel || unbind)
            ImGui::CloseCurrentPopup();

        // Find up to `MAX_KEYS` currently-held keys.
        // Only update `new_keys` if a currently held key is not in the array.
        ImGuiKey cur_keys[MAX_KEYS];
        uint8_t num_cur_keys = 0;
        static uint8_t prev_num_cur_keys = 0;
        bool is_changed = false;
        for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END && num_cur_keys < MAX_KEYS; key = (ImGuiKey)(key + 1))
        {
            if (key == ImGuiKey_ReservedForModCtrl
                || key == ImGuiKey_ReservedForModShift
                || key == ImGuiKey_ReservedForModAlt
                || key == ImGuiKey_ReservedForModSuper
            ) {
                continue;
            }

            if (!ImGui::IsAnyItemActive() && ImGui::IsKeyDown(key))
            {
                // Was the current key not in `new_keys`?
                // If so, the bind changed.
                bool exists = false;
                for (uint8_t i = 0; i < num_new_keys; ++i)
                    exists |= new_keys[i] == key;
                is_changed |= !exists;
                cur_keys[num_cur_keys] = key;
                ++num_cur_keys;
            }
        }
        
        if (is_changed || num_cur_keys > prev_num_cur_keys)
        {
            memcpy(new_keys, cur_keys, sizeof(cur_keys));
            num_new_keys = num_cur_keys;
        }
        prev_num_cur_keys = num_cur_keys;

        ImGui::EndPopup();
    }
    
    ImGui::PopID();
}

bool KeyBind::Poll()
{
    uint8_t num_down = 0;
    for (uint8_t i = 0; i < m_num_keys; ++i)
    {
        if (ImGui::IsKeyDown(m_keys[i]))
            ++num_down;
    }

    m_was_down = m_is_down;
    m_is_down = num_down && num_down == m_num_keys;
    return m_is_down && !m_was_down;
}

void KeyBind::KeysToString(char* buffer, size_t buffer_len, const ImGuiKey* keys, uint8_t num_keys)
{
    const char* fmt;
    switch (num_keys) // Why? Because it's easy, fast, and heap-free.
    {
    case 0: fmt = "Not bound"; break;
    case 1: fmt = "%s"; break;
    case 2: fmt = "%s + %s"; break;
    case 3: fmt = "%s + %s + %s"; break;
    case 4: fmt = "%s + %s + %s + %s"; break;
    default:
        assert(0 && "Expected `0 <= num_keys <= 4`");
    }

    const char* names[MAX_KEYS];
    for (uint8_t i = 0; i < num_keys; ++i)
        names[i] = ImGui::GetKeyName(keys[i]);
    
    sprintf_s(buffer, buffer_len, fmt, names[0], names[1], names[2], names[3]);
}

void ImGuiHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

}