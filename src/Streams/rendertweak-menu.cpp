#include "rendertweak.h"
#include <imgui.h>
#include <Helper/imgui.h>
#include <array>

void PropRenderTweak::OnMenu()
{
    ImGui::Checkbox("Hide props", &hide);
}

void EntityFilterTweak::OnMenu()
{
    static const char* ENTITY_POPUP = "##popup_add_class";
    static std::unordered_set<std::string>* new_class_destination = nullptr;
    
    // === Helper functions ===
    auto classes_getter = [](void* data, int index, const char** out_text) -> bool
    {
        auto set = (decltype(classes)*)data;
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
    auto classes_remove = [](decltype(classes)& set, size_t index) {
        if (index < 0 || index >= set.size())
            return;
        auto it = set.begin();
        for (size_t i = 0; i < index; ++i)
            ++it;
        set.erase(it);
    };

    if (ImGui::BeginCombo("Render effect", RenderEffectName(render_effect)))
    {
        for (int i = 0; i < (int)RenderEffect::_COUNT; ++i)
        {
            if (ImGui::Selectable(RenderEffectName((RenderEffect)i), i == (int)render_effect))
                render_effect = (RenderEffect)i;
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine(); Helper::ImGuiHelpMarker("Change how the models/textures are rendered");

    ImGui::ColorEdit4("Color multiply", color_multiply.data());
    ImGui::SameLine(); Helper::ImGuiHelpMarker("Use with the matte effect to get a solid color.");

    ImGui::TextUnformatted("Affected entities:");
    
    ImGui::RadioButton("All", (int*)&filter_choice, (int)FilterChoice::ALL);
    ImGui::RadioButton("Entities in the list", (int*)&filter_choice, (int)FilterChoice::WHITELIST);
    ImGui::RadioButton("Entities not in the list", (int*)&filter_choice, (int)FilterChoice::BLACKLIST);
    
    if (filter_choice == FilterChoice::ALL)
        ImGui::BeginDisabled();

    // === Include entities ===
    static int current_class = 0;
    
    ImGui::Text("Entity list:");
    ImGui::SameLine(); Helper::ImGuiHelpMarker("List of entity types to include/exclude");
    if (ImGui::Button("Add##include"))
    {
        new_class_destination = &classes;
        ImGui::OpenPopup(ENTITY_POPUP);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove##include"))
        classes_remove(classes, current_class);
    ImGui::SameLine();
    if (ImGui::Button("Remove all##include"))
        classes.clear();
    ImGui::ListBox("##included", &current_class, classes_getter, &classes, classes.size());
    
    if (filter_choice == FilterChoice::ALL)
        ImGui::EndDisabled();

    // === New entity popup ==
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