#include "rendertweak.h"
#include <imgui.h>
#include <array>

void PropRenderTweak::OnMenu()
{
    ImGui::Checkbox("Hide props", &m_hide);
}

void EntityRenderTweak::OnMenu()
{
    static const char* ENTITY_POPUP = "##popup_add_class";
    static std::unordered_set<std::string>* new_class_destination = nullptr;
    
    // === Helper functions ===
    auto classes_getter = [](void* data, int index, const char** out_text) -> bool
    {
        auto set = (decltype(include_classes)*)data;
        size_t i = 0;
        for (auto it = set->begin(); it != set->end(); ++it, ++i)
        {
            if (i == index)
            {
                *out_text = it->c_str();
                return true;
            }
        }
        return false;
    };
    auto classes_remove = [](decltype(include_classes)& set, size_t index) {
        if (index < 0 || index >= set.size())
            return;
        auto it = set.begin();
        for (size_t i = 0; i < index; ++i)
            ++it;
        set.erase(it);
    };

    // === Include entities ===
    static int current_include = 0;
    ImGui::Text("Included entity classes");
    if (ImGui::Button("Add##include"))
    {
        new_class_destination = &include_classes;
        ImGui::OpenPopup(ENTITY_POPUP);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove##include"))
        classes_remove(include_classes, current_include);
    ImGui::ListBox("##Included entity classes", &current_include, classes_getter, &include_classes, include_classes.size());

    // === Exclude entities ===
    static int current_exclude = 0;
    ImGui::Text("Excluded entity classes");
    if (ImGui::Button("Add##exclude"))
    {
        new_class_destination = &exclude_classes;
        ImGui::OpenPopup(ENTITY_POPUP);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove##exclude"))
        classes_remove(exclude_classes, current_exclude);
    ImGui::ListBox("##Excluded entity classes", &current_exclude, classes_getter, &exclude_classes, exclude_classes.size());

    // === New entity name popup ==

    if (ImGui::BeginPopup(ENTITY_POPUP))
    {
        static std::array<char, 64> input_entity_class;
        ImGui::InputText("Class name##input_class", input_entity_class.data(), input_entity_class.size());
        bool ok = ImGui::Button("Ok##input_class");
        ImGui::SameLine();
        bool cancel = ImGui::Button("Cancel##input_class");

        if (ok)
            new_class_destination->emplace(input_entity_class.data());
        if (ok || cancel)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}