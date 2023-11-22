/**
 * @file
 * @brief Implements the IMGUI menu for all @ref RenderTweak subclasses.
 * 
 * Thread safey
 * ------------
 * Acquire a write lock before modifying any dynamic collection (list, vector, set, string, ...):
 * ```cpp
 *      auto lock = g_active_stream.WriteLock();
 * ```
 * This avoids crashes where another thread is iterating the collection.
 * 
 * It's often unnecessary to synchronize primitives because most are read-once and only modified inside `OnMenu()`.
 * The nature of IMGUI also makes this difficult. However, it is still encouraged and sometimes necessary.
 * 
 * Updating the preview
 * --------------------
 * Some tweaks (like @ref MaterialTweak) are read by @ref ActiveStream only when necessary.
 * If such special tweaks/values are modified, call @ref ActiveStream::SignalUpdate to make it re-apply them.
 */

#include "rendertweak.h"
#include "materials.h"
#include <Modules/fx/ActiveStream.h>
#include <SDK/texture_group_names.h>
#include <SDK/client_class.h>
#include <Helper/imgui.h>
#include <Helper/str.h>
#include <Base/Interfaces.h>
#include <imgui.h>
#include <array>

//static std::vector<const char*> GetAllClassNames()
//{
//    std::vector<const char*> names;
//    ClientClass* client_class = Interfaces::hlclient->GetAllClasses();
//    for (; client_class; client_class = client_class->m_pNext)
//        names.emplace_back(client_class->m_pNetworkName);
//    return names;
//}

void MiscTweak::OnMenu()
{
    bool should_update = false;
    should_update |= ImGui::Checkbox("Enable viewmodel", &viewmodel_enabled);
    should_update |= ImGui::Checkbox("Enable HUD", &hud_enabled);
    should_update |= ImGui::Checkbox("Enable props", &props_enabled);
    should_update |= ImGui::Checkbox("Enable shadows", &shadows_enabled);
    should_update |= ImGui::Checkbox("Enable skybox", &skybox_enabled);
    should_update |= ImGui::Checkbox("Enable decals", &decals_enabled);
    should_update |= ImGui::Checkbox("Enable particles", &particles_enabled);
    should_update |= ImGui::Checkbox("Enable misc effects", &misc_effects_enabled);
    ImGui::SameLine(); Helper::ImGuiHelpMarker("In Team Fortress 2, this affects the glow outlines that appear on objectives");

    if (should_update)
        g_active_stream.SignalUpdate(nullptr, ActiveStream::UPDATE_WORLD);
}

void EntityFilterTweak::OnMenu()
{
    const char* const ENTITY_POPUP = "##popup_add_class";
    
    // === Helper functions === //
    auto classes_getter = [](void* data, int index, const char** out_text) -> bool
    {
        const auto* set = (decltype(classes)*)data;
        size_t i = 0;
        for (auto it = set->begin(); it != set->end(); ++it, ++i)
        {
            if (i == index)
            {
                *out_text = (*it)->GetName();
                return true;
            }
        }
        return false;
    };
    auto classes_remove = [](decltype(classes)& set, size_t index)
    {
        std::unique_lock lock = g_active_stream.WriteLock();
        
        if (index < 0 || index >= set.size())
            return;
        auto it = set.begin();
        for (size_t i = 0; i < index; ++i)
            ++it;
        set.erase(it);
    };

    if (ImGui::BeginCombo("Material", custom_material ? custom_material->GetName().c_str() : MaterialChoiceName(render_effect)))
    {
        for (int i = 0; i < (int)MaterialChoice::_COUNT; ++i)
        {
            if (ImGui::Selectable(MaterialChoiceName((MaterialChoice)i), i == (int)render_effect))
            {
                render_effect = (MaterialChoice)i;
                custom_material = nullptr;
            }
        }

        size_t i = 0;
        for (auto mat : CustomMaterial::GetAll())
        {
            ImGui::PushID(i);
            if (ImGui::Selectable(mat->GetName().c_str(), custom_material == mat))
            {
                render_effect = MaterialChoice::CUSTOM;
                custom_material = mat;
            }
            ImGui::PopID();
            ++i;
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

    // === Include entities === //
    static int current_class = 0;

    if (filter_choice == FilterChoice::ALL)
        ImGui::BeginDisabled();

    ImGui::Checkbox("Players", &filter_player);
    ImGui::Checkbox("Weapons", &filter_weapon);
    ImGui::Checkbox("Wearables", &filter_wearable);
    ImGui::Checkbox("Projectiles", &filter_projectile);
    
    ImGui::Text("Entity classes:");
    ImGui::SameLine(); Helper::ImGuiHelpMarker("List of entity classes to include/exclude");
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

    // === New entity popup == //
    if (ImGui::BeginPopup(ENTITY_POPUP))
    {
        static std::array<char, 64> search_input = {0};
        ImGui::InputText("Search", search_input.data(), search_input.size() - 1);

        ClientClass* selected = nullptr;
        if (ImGui::BeginListBox("Class names"))
        {
            ClientClass* next_class = Interfaces::hlclient->GetAllClasses();
            for (; next_class; next_class = next_class->m_pNext)
            {
                if (!search_input[0] || Helper::strcasestr(next_class->GetName(), search_input.data()))
                {
                    if (ImGui::Selectable(next_class->GetName(), false, ImGuiSelectableFlags_DontClosePopups))
                        selected = next_class;
                }
            }
            ImGui::EndListBox();
        }

        bool close = ImGui::Button("Close");

        if (selected)
        {
            auto lock = g_active_stream.WriteLock();
            classes.emplace(selected);
        }
        if (close)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void MaterialTweak::OnMenu()
{
    bool should_update = false;

    should_update |= ImGui::ColorEdit4("Color multiply", color_multiply.data());
    ImGui::SameLine(); Helper::ImGuiHelpMarker("\"Tints\" the color of a material.");

    ImGui::TextUnformatted("Affected material groups:");
    
    FilterChoice past_filter_choice = filter_choice;
    ImGui::RadioButton("All", (int*)&filter_choice, (int)FilterChoice::ALL);
    ImGui::RadioButton("Materials in the list", (int*)&filter_choice, (int)FilterChoice::WHITELIST);
    ImGui::RadioButton("Materials not in the list", (int*)&filter_choice, (int)FilterChoice::BLACKLIST);
    should_update |= past_filter_choice != filter_choice;

    // === Include material groups === //
    if (filter_choice == FilterChoice::ALL)
        ImGui::BeginDisabled();
    
    ImGui::Text("Material group list:");
    ImGui::SameLine(); Helper::ImGuiHelpMarker("List of material groups to include/exclude");
    
    const int max_lines = 10;
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::BeginChild("ConstrainedChild", ImVec2(0, 0), true, 0))
    {
        should_update |= ImGui::Checkbox("Props", &props);
        for (size_t i = 0; i < groups.size(); ++i)
            should_update |= ImGui::Checkbox(MaterialTweak::TEXTURE_GROUPS[i], &groups[i]);
    }
    ImGui::EndChild();

    if (filter_choice == FilterChoice::ALL)
        ImGui::EndDisabled();

    if (should_update)
        g_active_stream.SignalUpdate(nullptr, ActiveStream::UPDATE_MATERIALS);
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

void FogTweak::OnMenu()
{
    const int FOG_STEP = 50;
    bool should_update = false;
    should_update |= ImGui::Checkbox("Enable fog", &fog_enabled);
    should_update |= ImGui::InputInt("Fog start", &fog_start, FOG_STEP);
    should_update |= ImGui::InputInt("Fog end", &fog_end, FOG_STEP);
    should_update |= ImGui::ColorEdit3("Fog color", fog_color.data(), ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8);

    should_update |= ImGui::Checkbox("Enable sky fog", &skyfog_enabled);
    should_update |= ImGui::InputInt("Sky fog start", &skyfog_start, FOG_STEP);
    should_update |= ImGui::InputInt("Sky fog end", &skyfog_end, FOG_STEP);
    should_update |= ImGui::ColorEdit3("Sky fog color", skyfog_color.data(), ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Uint8);

    if (should_update)
        g_active_stream.SignalUpdate(nullptr, ActiveStream::UPDATE_FOG);
}