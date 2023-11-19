#include "StreamEditor.h"
#include "ActiveStream.h"
#include "recorder.h"
#include <Modules/Menu.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <array>
#include <algorithm>

#define POPUP_STREAM_RENAMER "##popup_stream_renamer"
#define POPUP_STREAM_EDITOR "##popup_stream_editor"
#define POPUP_TWEAK_CREATOR "##popup_tweak_creator"
#define POPUP_TWEAK_EDITOR "##popup_tweak_editor"

void StreamEditor::StartListening() {
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
}

int StreamEditor::OnMenu()
{
    bool is_recording = g_recorder.IsRecordingMovie();
    if (ImGui::CollapsingHeader("Streams"))
    {
        if (is_recording)
        {
            ImGui::BeginDisabled();
            m_preview = false;
        }
        
        ImGui::BeginGroup();
        ShowStreamListEditor();
        ImGui::EndGroup();
        
        if (is_recording)
            ImGui::EndDisabled();
    }

    return 0;
}

void StreamEditor::ShowStreamListEditor()
{
    static auto stream_getter = [](void* data, int index, const char** out_text)
    {
        auto& stream_vec = *(const std::vector<Stream::Ptr>*)data;
        if (index >= stream_vec.size())
            return false;
        *out_text = stream_vec[index]->GetName().c_str();
        return true;
    };

    Stream::Ptr selected = nullptr;
    if (m_stream_index < m_streams.size())
        selected = m_streams[m_stream_index];
    Stream::Ptr prev_selected = selected;
    
    if (ImGui::Button("Add##stream"))
    {
        m_stream_index = m_streams.size();
        ImGui::OpenPopup(POPUP_STREAM_RENAMER);
    } ImGui::SameLine();
    if (ImGui::Button("Remove##stream"))
    {
        if (m_stream_index < m_streams.size())
            m_streams.erase(m_streams.begin() + m_stream_index);
    } ImGui::SameLine();
    if (ImGui::Button("Rename##stream"))
        ImGui::OpenPopup(POPUP_STREAM_RENAMER);

    bool changed_selection = ImGui::ListBox("##streams_list", &m_stream_index, stream_getter, &m_streams, m_streams.size());
    bool changed_preview = ImGui::Checkbox("Preview the selected stream", &m_preview);

    selected = m_stream_index < m_streams.size() ? m_streams[m_stream_index] : nullptr;
    changed_selection |= prev_selected != selected;
    if (changed_selection || changed_preview)
        g_active_rendercfg.Set(m_preview ? selected : nullptr);

    if (selected)
    {
        ImVec2 win_pos = ImGui::GetWindowPos();
        win_pos.x += ImGui::GetWindowSize().x;

        ImGui::SetNextWindowPos(win_pos);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Once);
        
        bool opened = true;
        if (ImGui::Begin("Stream editor##stream_editor", &opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
        {
            // While resizing from the top/left handles, the window will infinitely resize to reach the cursor.
            // Prevent this by subtracting the change in position from its size.
            if (ImGui::IsWindowFocused()) // The window is focused. It may be resizing.
            {
                ImVec2 delta_pos = ImGui::GetWindowPos();
                delta_pos = { win_pos.x - delta_pos.x, win_pos.y - delta_pos.y };

                if (delta_pos[0] || delta_pos[1]) // Position has changed, which only happens during resize.
                {
                    ImVec2 new_size = ImGui::GetWindowSize();
                    new_size = { new_size.x - delta_pos.x, new_size.y - delta_pos.y };
                    ImGui::SetWindowSize(new_size);
                }
            }
            ShowStreamEditor(selected);
        }
        if (!opened)
            m_stream_index = m_streams.size();
        ImGui::End();
    }

    PopupStreamRenamer(selected);
}

void StreamEditor::PopupStreamRenamer(Stream::Ptr stream)
{
    static std::array<char, 64> input = {0};
    static auto input_filter = [](ImGuiInputTextCallbackData* data) -> int
    {
        // Filter reserved filename characters
        switch (data->EventChar)
        {
        case '<': case '>': case ':': case '"':
        case '/': case '\\': case '|': case '?':
        case '*':
            return 1;
        }
        return 0;
    };

    if (!ImGui::BeginPopup(POPUP_STREAM_RENAMER))
    {
        input[0] = 0;
        return;
    }
    
    if (stream && !input[0])
        strncpy_s(input.data(), input.size(), stream->GetName().c_str(), input.size() - 1);

    // Focus on the following text input
    if ((ImGui::IsWindowFocused() || !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) && !ImGui::IsAnyItemActive())
        ImGui::SetKeyboardFocusHere(0);
    
    bool ok = ImGui::InputText("New name", input.data(), input.size(), ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_EnterReturnsTrue, input_filter);
    ok |= ImGui::Button("Ok"); ImGui::SameLine();
    bool cancel = ImGui::Button("Cancel");
    bool accepted = false;

    if (ok) // Validate the final name before accepting it
    {
        if (input[0])
        {
            if (!IsDuplicateName(input.data()))
                accepted = true;
            else if (stream && input.data() == stream->GetName())
                cancel |= true;
        }
    }

    if (accepted)
    {
        if (!stream)
        {
            m_streams.push_back(std::make_shared<Stream>(input.data()));
            stream = m_streams.back();
        }
        stream->GetName() = input.data();
    }

    if (accepted || cancel)
        ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}

void StreamEditor::ShowStreamEditor(Stream::Ptr stream)
{
    auto tweaks_list = &stream->GetRenderTweaks();
    
    static auto tweaks_getter = [](void* data, int index, const char** out_text)
    {
        auto tweaks = (decltype(tweaks_list))data;
        if (index < 0 || index >= tweaks->size())
            return false;
        *out_text = tweaks->at(index)->GetName();
        return true;
    };

    const float child_height = 0;
    static int current_tweak = 0;
    
    PopupTweakCreator(stream);
    ImGui::Text("Active tweaks");
    if (ImGui::Button("Add##tweak"))
    {
        ImGui::OpenPopup(POPUP_TWEAK_CREATOR);
        // If the user adds a tweak, then this will focus it.
        // Otherwise, it will deselect the current tweak. Overall, it's less disorienting.
        current_tweak = stream->GetRenderTweaks().size();
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove##tweak"))
    {
        if (current_tweak < tweaks_list->size())
            tweaks_list->erase(tweaks_list->begin() + current_tweak);
        // The removed tweak may have included semi-permanent effect.
        // Signal an update to ensure any semi-permanent effects are reset/recalculated.
        g_active_rendercfg.SignalUpdate(stream);
    }

    ImGui::SetNextItemWidth(-1);
    ImGui::ListBox("##Active tweaks", &current_tweak, tweaks_getter, tweaks_list, tweaks_list->size());

    if (current_tweak < tweaks_list->size())
    {;
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5);
        if (ImGui::BeginChild("##tweak_editor", ImVec2(0, child_height), true))
            ShowTweakEditor(tweaks_list->at(current_tweak));
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
}

void StreamEditor::PopupTweakCreator(Stream::Ptr stream)
{
    static auto default_tweaks_getter = [](void* data, int index, const char** out_text)
    {
        if (index < 0 || index >= RenderTweak::default_tweaks.size())
            return false;
        *out_text = RenderTweak::default_tweaks.at(index)->GetName();
        return true;
    };

    if (ImGui::BeginPopup(POPUP_TWEAK_CREATOR))
    {
        ImGui::Text("Choose a tweak");
        // TODO: Manually use Begin/End List, because it will be faster and storing the last selected state is useless
        int choice = -1;
        bool ok = ImGui::ListBox("##default_tweaks_list", &choice, default_tweaks_getter, nullptr, RenderTweak::default_tweaks.size());
        bool cancel = ImGui::Button("Cancel##new_tweak");

        if (ok)
        {
            if (choice >= 0 && choice < RenderTweak::default_tweaks.size())
            {
                auto lock = g_active_rendercfg.WriteLock();
                stream->GetRenderTweaks().emplace_back(RenderTweak::default_tweaks.at(choice)->Clone());
            }
        }

        if (ok || cancel)
                ImGui::CloseCurrentPopup();
        
        ImGui::EndPopup();
    }
}

void StreamEditor::ShowTweakEditor(RenderTweak::Ptr render_tweak) {
    render_tweak->OnMenu();
}

bool StreamEditor::IsDuplicateName(const std::string& name) const
{
    auto existing = std::find_if(m_streams.begin(), m_streams.end(), [&](auto& cfg) {
        return cfg->GetName() == name;
    });
    return existing != m_streams.end();
}
