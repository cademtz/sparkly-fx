#include <array>
#include <algorithm>
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <Helper/json.h>
#include "StreamEditor.h"
#include "ActiveStream.h"
#include "recorder.h"
#include "mainwindow.h"
#include "configmodule.h"
#include <Streams/materials.h>
#include <SDK/KeyValues.h>
#include <Base/Interfaces.h>
#include <Modules/InputModule.h>

#define POPUP_STREAM_RENAMER "##popup_stream_renamer"
#define POPUP_STREAM_PRESETS "##popup_stream_presets"
#define POPUP_STREAM_EDITOR "##popup_stream_editor"
#define POPUP_TWEAK_CREATOR "##popup_tweak_creator"
#define POPUP_TWEAK_EDITOR "##popup_tweak_editor"

void StreamEditor::StartListening()
{
    CustomMaterial::CreateDefaultMaterials();
    MainWindow::OnTabBar.Listen(&StreamEditor::OnTabBar, this);
    ConfigModule::OnConfigLoad.Listen(&StreamEditor::OnConfigLoad, this);
    ConfigModule::OnConfigSave.Listen(&StreamEditor::OnConfigSave, this);
    InputModule::OnPostImguiInput.Listen(&StreamEditor::OnPostImguiInput, this);
}

int StreamEditor::OnPostImguiInput() {
    if (ImGui::GetCurrentContext() == nullptr) {
        return 0;
    }
    
    if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
        m_stream_index = (size_t)-1;
        g_active_stream.Set(nullptr);
    }
    return 0;
}

void StreamEditor::OnEndMovie()
{
    Stream::Ptr stream = nullptr;
    if (m_preview && m_stream_index < m_streams.size())
        stream = m_streams[m_stream_index];
    g_active_stream.Set(stream);
}

int StreamEditor::OnTabBar()
{
    if (!ImGui::BeginTabItem("Streams"))
        return 0;

    bool is_recording = g_recorder.IsRecordingMovie();
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

    if (!g_active_stream.IsDepthAvailable())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
        ImGui::TextWrapped(
            "Depth buffer is unavailable. Your graphics driver probably lacks support for the 'INTZ' depth format."
        );
        ImGui::PopStyleColor();
    }
    
    ImGui::EndTabItem();
    return 0;
}

int StreamEditor::OnConfigSave()
{
    auto lock = g_active_stream.ReadLock();
    nlohmann::json j_streams = nlohmann::json::array();
    for (Stream::ConstPtr stream : m_streams)
       j_streams.emplace_back(stream->ToJson());
    
    nlohmann::json j = {
        {"m_streams", std::move(j_streams)}
    };
    ConfigModule::GetOutput().emplace("Streams", std::move(j));
    return 0;
}

int StreamEditor::OnConfigLoad()
{
    const nlohmann::json* j = Helper::FromJson(ConfigModule::GetInput(), "Streams");
    const nlohmann::json* j_streams = Helper::FromJson(j, "m_streams");

    if (!j_streams || !j_streams->is_array() || j_streams->empty())
        return 0;

    m_streams.clear();
    for (auto& j_stream : *j_streams)
    {
        Stream::Ptr stream = Stream::CreateFromJson(&j_stream);
        if (stream)
            m_streams.emplace_back(std::move(stream));
    }

    Stream::Ptr new_preview = nullptr;
    if (m_preview && m_stream_index < m_streams.size())
        new_preview = m_streams[m_stream_index];
    g_active_stream.Set(new_preview);
    return 0;
}

void StreamEditor::ShowStreamListEditor()
{
    Stream::Ptr selected = nullptr;
    if (m_stream_index < m_streams.size())
        selected = m_streams[m_stream_index];
    static Stream::Ptr prev_selected = selected;
    
    if (ImGui::Button("Add##stream"))
    {
        m_stream_index = m_streams.size();
        ImGui::OpenPopup(POPUP_STREAM_RENAMER);
    } ImGui::SameLine();
    if (ImGui::Button("Presets##stream"))
    {
        m_stream_index = m_streams.size();
        ImGui::OpenPopup(POPUP_STREAM_PRESETS);
    } ImGui::SameLine();
    if (ImGui::Button("Remove##stream"))
    {
        if (m_stream_index < m_streams.size())
            m_streams.erase(m_streams.begin() + m_stream_index);
    } ImGui::SameLine();
    if (ImGui::Button("Rename##stream")) {
        ImGui::OpenPopup(POPUP_STREAM_RENAMER);
    } ImGui::SameLine();
    if (ImGui::Button("/\\") && m_stream_index < m_streams.size() && m_stream_index > 0) {
        m_streams.erase(m_streams.begin() + m_stream_index);
        --m_stream_index;
        m_streams.insert(m_streams.begin() + m_stream_index, selected);
    } ImGui::SameLine();
    if (ImGui::Button("\\/") && m_stream_index < m_streams.size() - 1 && m_streams.size() > 0) {
        m_streams.erase(m_streams.begin() + m_stream_index);
        ++m_stream_index;
        m_streams.insert(m_streams.begin() + m_stream_index, selected);
    }

    size_t prev_stream_index = m_stream_index;
    bool changed_preview = ImGui::Checkbox("Preview the selected stream", &m_preview);
    
    // Draw the stream list and its controls:
    // The top buttons (add, remove, edit, preview, ...) must remain fixed at the top.
    // To do this, wrap the table in a window so it has an independent scroll.
    ImGui::BeginChild("##table_window");
    // Remove vertical padding
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ImGui::GetStyle().CellPadding.x,0});
    if (ImGui::BeginTable("##table", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("##visible", ImGuiTableColumnFlags_WidthFixed, 24.0f);

        for (size_t i = 0; i < m_streams.size(); ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Selectable(m_streams[i]->GetName().c_str(), i == m_stream_index))
                m_stream_index = i;
            
            ImGui::TableNextColumn();
            char temp_label[16];
            std::snprintf(temp_label, sizeof(temp_label), "##%zu", i);
            bool enabled = m_streams[i]->IsEnabled();
            if (ImGui::Checkbox(temp_label, &enabled))
                m_streams[i]->SetEnabled(enabled);
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();

    selected = m_stream_index < m_streams.size() ? m_streams[m_stream_index] : nullptr;
    bool changed_selection = prev_selected != selected;
    prev_selected = selected;
    if (changed_selection || changed_preview)
        g_active_stream.Set(m_preview ? selected : nullptr);

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
    PopupStreamPresets();
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
    
    if (!input[0])
    {
        if (stream)
            strncpy_s(input.data(), input.size(), stream->GetName().c_str(), input.size() - 1);
        else // Use a default name
            strncpy_s(input.data(), input.size(), MakeUniqueName("new stream").c_str(), input.size() - 1);
    }

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
            if (!IsDuplicateName(std::string_view(input.data())))
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

void StreamEditor::PopupStreamPresets()
{
    if (!ImGui::BeginPopup(POPUP_STREAM_PRESETS))
        return;
    
    Stream::ConstPtr selection = nullptr;

    if (ImGui::BeginListBox("Presets"))
    {
        for (const Stream::ConstPtr& preset : Stream::GetPresets())
        {
            if (ImGui::Selectable(preset->GetName().c_str()))
                selection = preset;
        }
        ImGui::EndListBox();
    }
    
    if (selection)
    {
        Stream::Ptr clone = selection->Clone(MakeUniqueName(selection->GetName()));
        m_streams.emplace_back(std::move(clone));
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void StreamEditor::ShowStreamEditor(const Stream::Ptr& stream)
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
        g_active_stream.SignalUpdate(stream);
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

void StreamEditor::PopupTweakCreator(const Stream::Ptr& stream)
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
                auto lock = g_active_stream.WriteLock();
                stream->GetRenderTweaks().emplace_back(RenderTweak::default_tweaks.at(choice)->Clone());
            }
        }

        if (ok || cancel)
                ImGui::CloseCurrentPopup();
        
        ImGui::EndPopup();
    }
}

void StreamEditor::ShowTweakEditor(const RenderTweak::Ptr& render_tweak) {
    render_tweak->OnMenu();
}

bool StreamEditor::IsDuplicateName(std::string_view name) const
{
    auto existing = std::find_if(m_streams.begin(), m_streams.end(), [&](auto& cfg) {
        return cfg->GetName() == name;
    });
    return existing != m_streams.end();
}

std::string StreamEditor::MakeUniqueName(const std::string_view base_name) const
{
    std::string new_name = std::string(base_name);
    if (!IsDuplicateName(new_name))
        return new_name;
    
    for (size_t i = m_streams.size(); i != m_streams.size() - 1; ++i)
    {
        new_name = std::string(base_name) + ' ' + std::to_string(i);
        if (!IsDuplicateName(new_name))
            return new_name;
    }

    assert(0 && "This should never happen");
    return ":)";
}