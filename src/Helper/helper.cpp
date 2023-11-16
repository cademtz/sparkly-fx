#include "imgui.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
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

const char* VirtualKeyString(uint8_t vkey)
{
    switch (vkey)
    {
    case VK_LBUTTON: return "LeftMouse";
    case VK_RBUTTON: return "RightMouse";
    case VK_CANCEL: return "Cancel";
    case VK_MBUTTON: return "MiddleMouse";
    case VK_XBUTTON1: return "X1Mouse";
    case VK_XBUTTON2: return "X2Mouse";
    case VK_BACK: return "Back";
    case VK_TAB: return "Tab";
    case VK_CLEAR: return "Clear";
    case VK_RETURN: return "Enter";
    case VK_SHIFT: return "Shift";
    case VK_CONTROL: return "Ctrl";
    case VK_MENU: return "Menu";
    case VK_PAUSE: return "Pause";
    case VK_CAPITAL: return "CapsLock";
    //case VK_KANA: /* case VK_HANGEUL: */ /* case VK_HANGUL: */
    //case VK_IME_ON:
    //case VK_JUNJA:
    //case VK_FINAL:
    //case VK_HANJA: /* case VK_KANJI: */
    //case VK_IME_OFF:
    case VK_ESCAPE: return "Esc";
    //case VK_CONVERT:
    //case VK_NONCONVERT:
    //case VK_ACCEPT:
    //case VK_MODECHANGE:
    case VK_SPACE: return "Space";
    case VK_PRIOR: return "PageUp";
    case VK_NEXT: return "PageDown";
    case VK_END: return "End";
    case VK_HOME: return "Home";
    case VK_LEFT: return "Left";
    case VK_UP: return "Up";
    case VK_RIGHT: return "Right";
    case VK_DOWN: return "Down";
    case VK_SELECT: return "Select";
    case VK_PRINT: return "Print";
    case VK_EXECUTE: return "Execute";
    case VK_SNAPSHOT: return "Snapshot";
    case VK_INSERT: return "Insert";
    case VK_DELETE: return "Delete";
    case VK_HELP: return "Help";
    case '0': return "0";
    case '1': return "1";
    case '2': return "2";
    case '3': return "3";
    case '4': return "4";
    case '5': return "5";
    case '6': return "6";
    case '7': return "7";
    case '8': return "8";
    case '9': return "9";
    case 'A': return "A";
    case 'B': return "B";
    case 'C': return "C";
    case 'D': return "D";
    case 'E': return "E";
    case 'F': return "F";
    case 'G': return "G";
    case 'H': return "H";
    case 'I': return "I";
    case 'J': return "J";
    case 'K': return "K";
    case 'L': return "L";
    case 'M': return "M";
    case 'N': return "N";
    case 'O': return "O";
    case 'P': return "P";
    case 'Q': return "Q";
    case 'R': return "R";
    case 'S': return "S";
    case 'T': return "T";
    case 'U': return "U";
    case 'V': return "V";
    case 'W': return "W";
    case 'X': return "X";
    case 'Y': return "Y";
    case 'Z': return "Z";
    case VK_LWIN: return "LeftWindow";
    case VK_RWIN: return "RightWindow";
    case VK_APPS: return "Apps";
    case VK_SLEEP: return "Sleep";
    case VK_NUMPAD0: return "Numpad-0";
    case VK_NUMPAD1: return "Numpad-1";
    case VK_NUMPAD2: return "Numpad-2";
    case VK_NUMPAD3: return "Numpad-3";
    case VK_NUMPAD4: return "Numpad-4";
    case VK_NUMPAD5: return "Numpad-5";
    case VK_NUMPAD6: return "Numpad-6";
    case VK_NUMPAD7: return "Numpad-7";
    case VK_NUMPAD8: return "Numpad-8";
    case VK_NUMPAD9: return "Numpad-9";
    case VK_MULTIPLY: return "*";
    case VK_ADD: return "+";
    case VK_SEPARATOR: return "Separator";
    case VK_SUBTRACT: return "-";
    case VK_DECIMAL: return ".";
    case VK_DIVIDE: return "/";
    case VK_F1: return "F1";
    case VK_F2: return "F2";
    case VK_F3: return "F3";
    case VK_F4: return "F4";
    case VK_F5: return "F5";
    case VK_F6: return "F6";
    case VK_F7: return "F7";
    case VK_F8: return "F8";
    case VK_F9: return "F9";
    case VK_F10: return "F10";
    case VK_F11: return "F11";
    case VK_F12: return "F12";
    case VK_F13: return "F13";
    case VK_F14: return "F14";
    case VK_F15: return "F15";
    case VK_F16: return "F16";
    case VK_F17: return "F17";
    case VK_F18: return "F18";
    case VK_F19: return "F19";
    case VK_F20: return "F20";
    case VK_F21: return "F21";
    case VK_F22: return "F22";
    case VK_F23: return "F23";
    case VK_F24: return "F24";
    case VK_NUMLOCK: return "NumLock";
    case VK_SCROLL: return "ScrollLock";
    case VK_LSHIFT: return "LeftShift";
    case VK_RSHIFT: return "RightShift";
    case VK_LCONTROL: return "LeftCtrl";
    case VK_RCONTROL: return "RightCtrl";
    case VK_LMENU: return "LeftMenu";
    case VK_RMENU: return "RightMenu";
    case VK_BROWSER_BACK: return "BrowserBack";
    case VK_BROWSER_FORWARD: return "BrowserForward";
    case VK_BROWSER_REFRESH: return "BrowserRefresh";
    case VK_BROWSER_STOP: return "BrowserStop";
    case VK_BROWSER_SEARCH: return "BrowserSearch";
    case VK_BROWSER_FAVORITES: return "BrowserFavorites";
    case VK_BROWSER_HOME: return "BrowserHome";
    case VK_VOLUME_MUTE: return "VolumeMute";
    case VK_VOLUME_DOWN: return "VolumeDown";
    case VK_VOLUME_UP: return "VolumeUp";
    case VK_MEDIA_NEXT_TRACK: return "MediaNextTrack";
    case VK_MEDIA_PREV_TRACK: return "MediaPrevTrack";
    case VK_MEDIA_STOP: return "MediaStop";
    case VK_MEDIA_PLAY_PAUSE: return "MediaPlayPause";
    case VK_LAUNCH_MAIL: return "LaunchMail";
    case VK_LAUNCH_MEDIA_SELECT: return "LaunchMediaSelect";
    case VK_LAUNCH_APP1: return "LaunchApp1";
    case VK_LAUNCH_APP2: return "LaunchApp2";
    case VK_OEM_1: return "OEM1";
    case VK_OEM_PLUS: return "+";
    case VK_OEM_COMMA: return ",";
    case VK_OEM_MINUS: return "-";
    case VK_OEM_PERIOD: return ".";
    case VK_OEM_2: return "OEM2";
    case VK_OEM_3: return "OEM3";
    case VK_OEM_4: return "OEM4";
    case VK_OEM_5: return "OEM5";
    case VK_OEM_6: return "OEM6";
    case VK_OEM_7: return "OEM7";
    case VK_OEM_8: return "OEM8";
    case VK_OEM_102: return "OEM102";
    case VK_PROCESSKEY: return "Process";
    //case VK_PACKET:
    case VK_ATTN: return "Attn";
    case VK_CRSEL: return "CrSel";
    case VK_EXSEL: return "ExSel";
    case VK_EREOF: return "EraseEOF";
    case VK_PLAY: return "Play";
    case VK_ZOOM: return "Zoom";
    //case VK_NONAME:
    case VK_PA1: return "PA1";
    case VK_OEM_CLEAR: return "Clear";
    }
    return "?";
}

}