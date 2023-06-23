#include "renderframe-editor.h"
#include <Modules/Menu.h>
#include <array>

static const char* POPUP_FRAME_CREATOR = "##popup_frame_creator";
static const char* POPUP_FRAME_EDITOR = "##popup_frame_editor";
static const char* POPUP_TWEAK_CREATOR = "##popup_tweak_creator";
static const char* POPUP_TWEAK_EDITOR = "##popup_tweak_editor";

void RenderFrameEditor::StartListening() {
    Listen(EVENT_MENU, [this]{ return OnMenu(); });
}

int RenderFrameEditor::OnMenu()
{
    if (ImGui::CollapsingHeader("Frames"))
    {
        ImGui::BeginGroup();
        ShowFrameListEditor();
        ImGui::EndGroup();
    }

    return 0;
}

void RenderFrameEditor::ShowFrameListEditor()
{
    static auto frame_getter = [](void* data, int index, const char** out_text)
    {
        auto frames_vec = (const decltype(RenderFrameEditor::m_render_frames)*)data;
        if (index >= 0 && index < frames_vec->size())
        {
            *out_text = frames_vec->at(index)->GetName().c_str();
            return true;
        }
        return false;
    };

    static int current_frame = 0;

    {
        ImGui::BeginGroup();

        ImGui::SeparatorText("Frames list");
        
        if (ImGui::Button("New##frame"))
        ImGui::OpenPopup(POPUP_FRAME_CREATOR);
        ImGui::SameLine();
        if (ImGui::Button("Delete##frame"))
        {
            if (current_frame >= 0 && current_frame < m_render_frames.size())
                m_render_frames.erase(m_render_frames.begin() + current_frame);
        }

        ImGui::ListBox("##frames_list", &current_frame, frame_getter, &m_render_frames, m_render_frames.size());
        
        ImGui::EndGroup();
    }

    if (current_frame >= 0 && current_frame < m_render_frames.size())
    {
        ImGui::BeginGroup();

        RenderFrame::Ptr render_frame = m_render_frames.at(current_frame);
        ImGui::SeparatorText(render_frame->GetName().c_str());
        ShowFrameEditor(render_frame);

        ImGui::EndGroup();
    }

    PopupFrameCreator();
}

void RenderFrameEditor::PopupFrameCreator()
{
    if (ImGui::BeginPopup(POPUP_FRAME_CREATOR))
    {
        static std::array<char, 64> input_frame_name = { "my frame" };
        ImGui::Text("Choose a name");
        ImGui::PushItemWidth(-1);
        ImGui::InputText("##new_frame_name", input_frame_name.data(), input_frame_name.size());
        ImGui::PopItemWidth();
        if (ImGui::Button("Ok##new_frame")) {
            m_render_frames.emplace_back(std::make_shared<RenderFrame>(input_frame_name.data()));
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void RenderFrameEditor::ShowFrameEditor(RenderFrame::Ptr render_frame)
{
    auto frame_tweaks = &render_frame->GetRenderTweaks();
    
    static auto tweaks_getter = [](void* data, int index, const char** out_text)
    {
        auto tweaks = (decltype(frame_tweaks))data;
        if (index < 0 || index >= tweaks->size())
            return false;
        *out_text = tweaks->at(index)->GetName();
        return true;
    };

    const float child_height = 200;
    static int current_tweak = -1;
    static std::array<char, 64> input_name = {0};
    {
        ImGui::BeginChild("##frame_editor", ImVec2(150, child_height));

        PopupTweakCreator(render_frame);
        ImGui::InputText("Name##frame", input_name.data(), input_name.size());
        ImGui::Text("Active tweaks");
        if (ImGui::Button("Add##tweak"))
            ImGui::OpenPopup(POPUP_TWEAK_CREATOR);
        ImGui::SameLine();
        if (ImGui::Button("Remove##tweak"))
        {
            if (current_tweak >= 0 && current_tweak < frame_tweaks->size())
                frame_tweaks->erase(frame_tweaks->begin() + current_tweak);
        }

        ImGui::ListBox("##Active tweaks", &current_tweak, tweaks_getter, frame_tweaks, frame_tweaks->size());
        ImGui::EndChild();
    }

    if (current_tweak >= 0 && current_tweak < frame_tweaks->size())
    {
        ImGui::SameLine();
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5);
        ImGui::BeginChild("##tweak_editor", ImVec2(0, child_height), true);
        ShowTweakEditor(frame_tweaks->at(current_tweak));
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
}

void RenderFrameEditor::PopupTweakCreator(RenderFrame::Ptr render_frame)
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
                render_frame->GetRenderTweaks().emplace_back(RenderTweak::default_tweaks.at(choice)->Clone());
        }

        if (ok || cancel)
                ImGui::CloseCurrentPopup();
        
        ImGui::EndPopup();
    }
}

void RenderFrameEditor::ShowTweakEditor(RenderTweak::Ptr render_tweak)
{
    render_tweak->OnMenu();
}
