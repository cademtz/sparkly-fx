#include "RenderConfigEditor.h"
#include "ActiveRenderConfig.h"
#include "recorder.h"
#include <Modules/Menu.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <array>
#include <algorithm>

#define POPUP_STREAM_RENAMER "##popup_stream_renamer"
#define POPUP_STREAM_EDITOR "##popup_stream_editor"
#define POPUP_TWEAK_CREATOR "##popup_tweak_creator"
#define POPUP_TWEAK_EDITOR "##popup_tweak_editor"

void RenderConfigEditor::StartListening() {
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
}

int RenderConfigEditor::OnMenu()
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
        ShowConfigListEditor();
        ImGui::EndGroup();
        
        if (is_recording)
            ImGui::EndDisabled();
    }

    return 0;
}

void RenderConfigEditor::ShowConfigListEditor()
{
    static auto config_getter = [](void* data, int index, const char** out_text)
    {
        auto& config_vec = *(const std::vector<RenderConfig::Ptr>*)data;
        if (index >= config_vec.size())
            return false;
        *out_text = config_vec[index]->GetName().c_str();
        return true;
    };

    RenderConfig::Ptr selected = nullptr;
    if (m_config_index < m_render_configs.size())
        selected = m_render_configs[m_config_index];

    {
        ImGui::BeginGroup();

        ImGui::SeparatorText("Stream list");

        RenderConfig::Ptr prev_selected = selected;
        
        if (ImGui::Button("Add##stream"))
        {
            std::string new_name;
            for (size_t i = m_render_configs.size() + 1; true; ++i)
            {
                new_name = std::string("new stream ") + std::to_string(i);
                if (!IsDuplicateName(new_name))
                    break;
            }
            m_render_configs.push_back(std::make_shared<RenderConfig>(std::move(new_name)));
            m_config_index = m_render_configs.size() - 1;
            selected = m_render_configs.back();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove##stream"))
        {
            if (selected)
                m_render_configs.erase(m_render_configs.begin() + m_config_index);
            // Intentionally leave config_index unchanged
            selected = nullptr;
            if (m_config_index < m_render_configs.size())
                selected = m_render_configs[m_config_index];
        }
        ImGui::SameLine();
        if (ImGui::Button("Rename##stream"))
            ImGui::OpenPopup(POPUP_STREAM_RENAMER);

        int new_config_index = (int)m_config_index;
        bool changed_selection = ImGui::ListBox("##streams_list", &new_config_index, config_getter, &m_render_configs, m_render_configs.size());
        bool changed_preview = ImGui::Checkbox("Preview the selected stream", &m_preview);
        changed_selection |= prev_selected != selected;

        if (changed_selection || changed_preview)
        {
            selected = nullptr;
            m_config_index = (size_t)new_config_index;
            if (m_config_index < m_render_configs.size())
                selected = m_render_configs[m_config_index];
            g_active_rendercfg.Set(m_preview ? selected : nullptr);
        }

        ImGui::EndGroup();
    }

    if (selected)
    {
        ImGui::BeginGroup();
        ImGui::SeparatorText(selected->GetName().c_str());
        ShowConfigEditor(selected);
        ImGui::EndGroup();
    }

    PopupConfigRenamer(selected);
}

void RenderConfigEditor::PopupConfigRenamer(RenderConfig::Ptr render_config)
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

    if (render_config && ImGui::BeginPopup(POPUP_STREAM_RENAMER))
    {
        if (!input[0])
            strncpy_s(input.data(), input.size(), render_config->GetName().c_str(), input.size() - 1);

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
                else if (input.data() == render_config->GetName())
                    cancel |= true;
            }
        }

        if (accepted)
            render_config->GetName() = input.data();

        if (accepted || cancel)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    else
        input[0] = 0;
}

void RenderConfigEditor::ShowConfigEditor(RenderConfig::Ptr render_config)
{
    auto tweaks_list = &render_config->GetRenderTweaks();
    
    static auto tweaks_getter = [](void* data, int index, const char** out_text)
    {
        auto tweaks = (decltype(tweaks_list))data;
        if (index < 0 || index >= tweaks->size())
            return false;
        *out_text = tweaks->at(index)->GetName();
        return true;
    };

    const float child_height = 0;
    static int current_tweak = -1;
    {
        ImGui::BeginChild("##stream_editor", ImVec2(200, child_height));

        PopupTweakCreator(render_config);
        ImGui::Text("Active tweaks");
        if (ImGui::Button("Add##tweak"))
            ImGui::OpenPopup(POPUP_TWEAK_CREATOR);
        ImGui::SameLine();
        if (ImGui::Button("Remove##tweak"))
        {
            if (current_tweak >= 0 && current_tweak < tweaks_list->size())
                tweaks_list->erase(tweaks_list->begin() + current_tweak);
            // The removed tweak may have included semi-permanent effect.
            // Signal an update to ensure any semi-permanent effects are reset/recalculated.
            g_active_rendercfg.SignalUpdate(render_config);
        }

        ImGui::SetNextItemWidth(-1);
        ImGui::ListBox("##Active tweaks", &current_tweak, tweaks_getter, tweaks_list, tweaks_list->size());
        ImGui::EndChild();
    }

    if (current_tweak >= 0 && current_tweak < tweaks_list->size())
    {
        ImGui::SameLine();
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5);
        if (ImGui::BeginChild("##tweak_editor", ImVec2(0, child_height), true, ImGuiWindowFlags_MenuBar))
            ShowTweakEditor(tweaks_list->at(current_tweak));
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
}

void RenderConfigEditor::PopupTweakCreator(RenderConfig::Ptr render_config)
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
                render_config->GetRenderTweaks().emplace_back(RenderTweak::default_tweaks.at(choice)->Clone());
            }
        }

        if (ok || cancel)
                ImGui::CloseCurrentPopup();
        
        ImGui::EndPopup();
    }
}

void RenderConfigEditor::ShowTweakEditor(RenderTweak::Ptr render_tweak) {
    render_tweak->OnMenu();
}

bool RenderConfigEditor::IsDuplicateName(const std::string& name) const
{
    auto existing = std::find_if(m_render_configs.begin(), m_render_configs.end(), [&](auto& cfg) {
        return cfg->GetName() == name;
    });
    return existing != m_render_configs.end();
}
