/**
 * @file
 * @brief Implements the IMGUI menu for all @ref RenderTweak subclasses.
 * 
 * Thread safey
 * ------------
 * Acquire a write lock before modifying any dynamic collection (list, vector, set, string, ...):
 * ```cpp
 *      auto lock = g_active_rendercfg.WriteLock();
 * ```
 * This avoids crashes where another thread is iterating the collection.
 * 
 * It's often unnecessary to synchronize primitives because most are read-once and only modified inside `OnMenu()`.
 * The nature of IMGUI also makes this difficult. However, it is still encouraged and sometimes necessary.
 * 
 * Updating the preview
 * --------------------
 * Some tweaks (like @ref MaterialTweak) are read by @ref ActiveRenderConfig only when necessary.
 * If such special tweaks/values are modified, call @ref ActiveRenderConfig::SignalUpdate to make it re-apply them.
 */

#include "rendertweak.h"
#include <Modules/fx/ActiveRenderConfig.h>
#include <SDK/texture_group_names.h>
#include <imgui.h>
#include <Helper/imgui.h>
#include <array>

void PropRenderTweak::OnMenu()
{
    ImGui::Checkbox("Hide props", &hide);
}

void EntityFilterTweak::OnMenu()
{
    const char* const ENTITY_POPUP = "##popup_add_class";
    
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
    auto classes_remove = [](decltype(classes)& set, size_t index)
    {
        std::unique_lock lock = g_active_rendercfg.WriteLock();
        
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

    // === Include entities ===
    static int current_class = 0;

    if (filter_choice == FilterChoice::ALL)
        ImGui::BeginDisabled();
    
    ImGui::Text("Entity list:");
    ImGui::SameLine(); Helper::ImGuiHelpMarker("List of entity types to include/exclude");
    if (ImGui::Button("Add##include"))
        ImGui::OpenPopup(ENTITY_POPUP);
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
        {
            auto lock = g_active_rendercfg.WriteLock();
            classes.emplace(input_entity_class.data());
        }
        if (ok || cancel)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void MaterialTweak::OnMenu()
{
    bool set_is_updated = false; // Acquire a read lock and set `is_updated = true`

    set_is_updated |= ImGui::ColorEdit4("Color multiply", color_multiply.data());
    ImGui::SameLine(); Helper::ImGuiHelpMarker("Use with the matte effect to get a solid color.");

    ImGui::TextUnformatted("Affected material groups:");
    
    FilterChoice past_filter_choice = filter_choice;
    ImGui::RadioButton("All", (int*)&filter_choice, (int)FilterChoice::ALL);
    ImGui::RadioButton("Materials in the list", (int*)&filter_choice, (int)FilterChoice::WHITELIST);
    ImGui::RadioButton("Materials not in the list", (int*)&filter_choice, (int)FilterChoice::BLACKLIST);
    set_is_updated |= past_filter_choice != filter_choice;

    // === Include material groups ===
    if (filter_choice == FilterChoice::ALL)
        ImGui::BeginDisabled();
    
    ImGui::Text("Material group list:");
    ImGui::SameLine(); Helper::ImGuiHelpMarker("List of material groups to include/exclude");
    
    const int max_lines = 10;
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginChild("ConstrainedChild", ImVec2(0, 0), true, 0))
    {
        for (size_t i = 0; i < groups.size(); ++i)
            set_is_updated |= ImGui::Checkbox(MaterialTweak::TEXTURE_GROUPS[i], &groups[i]);
    }
    ImGui::EndChild();

    if (filter_choice == FilterChoice::ALL)
        ImGui::EndDisabled();

    if (set_is_updated)
        g_active_rendercfg.SignalUpdate();
}

void CameraTweak::OnMenu()
{
    ImGui::Checkbox("FOV override", &fov_override);

    if (!fov_override)
        ImGui::BeginDisabled();

    ImGui::SliderFloat("FOV", &fov, 0, 180);

    if (!fov_override)
        ImGui::EndDisabled();
}