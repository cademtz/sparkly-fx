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
#include <Hooks/fx/ModelRenderHook.h>
#include <SDK/texture_group_names.h>
#include <SDK/client_class.h>
#include <Helper/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Helper/str.h>
#include <Base/Interfaces.h>
#include <imgui.h>
#include <array>
#include <utility>

void CommandTweak::OnMenu()
{
    const char* const POPUP_PRESETS = "##presets";
    bool should_update = false;

    ImGui::Text("Console commands:");
    ImGui::SameLine();
    Helper::ImGuiHelpMarker(
        "These commands are executed before rendering the stream.\n"
        "After rendering, all variables are set to their original value."
    );
    if (ImGui::Button("Presets"))
        ImGui::OpenPopup(POPUP_PRESETS);
    ImGui::PushItemWidth(-1);
    should_update |= ImGui::InputTextMultiline("##commands", &commands, ImVec2(0, -1));
    ImGui::SameLine(); Helper::ImGuiHelpMarker("In Team Fortress 2, this affects the glow outlines that appear on objectives");

    if (ImGui::BeginPopup(POPUP_PRESETS))
    {
        const std::pair<const char*, const char*> presets[] = {
            {"Hide viewmodel", "r_drawviewmodel 0"},
            {"Hide props", "r_drawstaticprops 0"},
            {"Hide shadows", "r_shadows 0"},
            {"Hide skybox", "r_skybox 0\nr_3dsky 0"},
            {"Hide particles", "r_drawparticles 0"},
            {"Hide misc effects", "glow_outline_effect_enable 0"},
            {"Erase decals", "r_cleardecals"},
            {"Hide screen shake", "shake_show 0"},
            {"Erase overlays (like jarate)", "r_screenoverlay off"},
        };

        if (ImGui::BeginListBox("##presets_list"))
        {
            for (auto& preset : presets)
            {
                if (ImGui::Selectable(preset.first))
                {
                    should_update |= true;
                    if (commands.size() && commands.back() != '\n')
                        commands += '\n';
                    commands += preset.second;
                }
            }
            ImGui::EndListBox();
        }
        ImGui::EndPopup();
    }

    if (should_update)
        g_active_stream.SignalUpdate(nullptr, ActiveStream::UPDATE_CONVARS);
}

void ModelTweak::OnMenu()
{
    const char* const ENTITY_POPUP = "##popup_add_class";
    const char* const MODEL_POPUP = "##popup_add_model";

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
    ImGui::SameLine(); Helper::ImGuiHelpMarker("Use this with the matte material to get a solid color.");

    ImGui::TextUnformatted("Affected models:");
    
    ImGui::RadioButton("All models", (int*)&filter_choice, (int)FilterChoice::ALL);
    ImGui::RadioButton("Models in the list", (int*)&filter_choice, (int)FilterChoice::WHITELIST);
    ImGui::RadioButton("Models not in the list", (int*)&filter_choice, (int)FilterChoice::BLACKLIST);

    // === Include entities === //
    static ClientClass* selected_class = nullptr;

    if (filter_choice == FilterChoice::ALL)
        ImGui::BeginDisabled();

    ImGui::BeginGroup();

    ImGui::Checkbox("Players", &filter_player);
    ImGui::Checkbox("Weapons", &filter_weapon);

    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::Checkbox("Wearables", &filter_wearable);
    ImGui::Checkbox("Projectiles", &filter_projectile);

    ImGui::EndGroup();
    
    ImGui::Text("Entity classes:");
    ImGui::SameLine(); Helper::ImGuiHelpMarker("List of entity classes to include/exclude");
    if (ImGui::Button("Add##class"))
        ImGui::OpenPopup(ENTITY_POPUP);
    ImGui::SameLine();
    if (ImGui::Button("Remove##class"))
    {
        auto lock = g_active_stream.WriteLock();
        classes.erase(selected_class);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove all##class"))
    {
        auto lock = g_active_stream.WriteLock();
        classes.clear();
    }

    if (ImGui::BeginListBox("##class_list", Helper::CalcListBoxSize(classes.size())))
    {
        for (ClientClass* client_class : classes)
        {
            if (ImGui::Selectable(client_class->GetName(), client_class == selected_class))
                selected_class = client_class;
        }
        ImGui::EndListBox();
    }

    // === Include models === //
    
    static size_t selected_model = 0;

    ImGui::Text("Models:");
    ImGui::SameLine(); Helper::ImGuiHelpMarker("List of models to include/exclude");
    if (ImGui::Button("Add##model"))
        ImGui::OpenPopup(MODEL_POPUP);
    ImGui::SameLine();
    if (ImGui::Button("Remove##model"))
    {
        size_t i = 0;
        for (auto it = model_paths.begin(); it != model_paths.end(); ++it, ++i)
        {
            if (i == selected_model)
            {
                auto lock = g_active_stream.WriteLock();
                model_paths.erase(it);
                break;
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove all##model"))
    {
        auto lock = g_active_stream.WriteLock();
        model_paths.clear();
    }

    if (ImGui::BeginListBox("##model_list", Helper::CalcListBoxSize(model_paths.size())))
    {
        size_t i = 0;
        for (const std::string& path : model_paths)
        {
            if (ImGui::Selectable(path.c_str(), i == selected_model))
                selected_model = i;
            ++i;
        }
        ImGui::EndListBox();
    }
    
    if (filter_choice == FilterChoice::ALL)
        ImGui::EndDisabled();

    // === New entity popup == //
    if (ImGui::BeginPopup(ENTITY_POPUP))
    {
        static std::array<char, 64> search_input = {0};
        ImGui::InputText("Search", search_input.data(), search_input.size() - 1);

        if (ImGui::BeginListBox("Class names"))
        {
            ClientClass* next_class = Interfaces::hlclient->GetAllClasses();
            for (; next_class; next_class = next_class->m_pNext)
            {
                if (!search_input[0] || Helper::strcasestr(next_class->GetName(), search_input.data()))
                {
                    if (ImGui::Selectable(next_class->GetName(), false, ImGuiSelectableFlags_DontClosePopups))
                    {
                        auto lock = g_active_stream.WriteLock();
                        classes.emplace(next_class);
                    }
                }
            }
            ImGui::EndListBox();
        }

        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup(MODEL_POPUP))
    {
        static std::array<char, 64> search_input = {0};
        ImGui::Text("Search:");
        ImGui::InputText("##search", search_input.data(), search_input.size() - 1); ImGui::SameLine();
        if (ImGui::Button("Add"))
        {
            auto lock = g_active_stream.WriteLock();
            model_paths.emplace(search_input.data());
        }

        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginListBox("##models"))
        {
            CModelRenderHook::LockedModelNames model_list = g_hk_model_render.GetDrawnModelList();
            for (const std::string& name : *model_list)
            {
                if (!search_input[0] || Helper::CaseInsensitivePathSubstr(name.c_str(), search_input.data()))
                {
                    if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_DontClosePopups))
                    {
                        auto lock = g_active_stream.WriteLock();
                        model_paths.emplace(name);
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
                        ImGui::SetTooltip("%s", name.c_str());
                }
            }
            ImGui::EndListBox();
        }

        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();

        ImGui::BeginDisabled();
        ImGui::Text( // Don't use TextWrapped. Popups don't like it.
            "This list only contains model that have been used.\n"
            "If you can't find a model, then make it appear in-game."
        );
        ImGui::EndDisabled();
        
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
        Helper::PushStyleCompact();
        should_update |= ImGui::Checkbox("Props", &props);
        for (size_t i = 0; i < groups.size(); ++i)
            should_update |= ImGui::Checkbox(MaterialTweak::TEXTURE_GROUPS[i], &groups[i]);
        Helper::PopStyleCompact();
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

    ImGui::Checkbox("Hide fade effects", &hide_fade); ImGui::SameLine();
    Helper::ImGuiHelpMarker("This hides the effects of flashbangs, teleporters, and other mechanics that normally cover the screen.");

    const std::array<const char*, 3> HUDCHOICE_STRINGS = { "Default", "Disabled", "Enabled" };
    if (ImGui::BeginCombo("HUD visiblity", HUDCHOICE_STRINGS[hud]))
    {
        for (int i = 0; i < HUDCHOICE_STRINGS.size(); ++i)
        {
            if (ImGui::Selectable(HUDCHOICE_STRINGS[i], i == hud))
                hud = (HudChoice)i;
        }
        ImGui::EndCombo();
    }
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