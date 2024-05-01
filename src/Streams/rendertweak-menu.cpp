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

#include <algorithm>
#include <nlohmann/json.hpp>
#include "rendertweak.h"
#include "materials.h"
#include <Shaders/shaders.h>
#include <Modules/fx/ActiveStream.h>
#include <Hooks/fx/ModelRenderHook.h>
#include <SDK/client_class.h>
#include <Helper/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Helper/str.h>
#include <Base/Interfaces.h>
#include <imgui.h>
#include <array>
#include <utility>

static std::array<const char*, (size_t)FilterChoice::_COUNT> FILTER_CHOICE_NAME = {
    "All", "Whitelist", "Blacklist"
};

/**
 * @brief A string-insensitive conversion for `str` in `list`
 * @param out_value The output, which will be the index of `str` on success
 * @return `true` if `out_value` was assigned
 */
template <class TList, class TEnum>
static bool StringToEnum(const TList& list, std::string_view str, TEnum* out_value)
{
    auto it = Helper::FirstInsensitiveStr(list, str);
    if (it == list.end())
        return false;
    *out_value = (TEnum)(it - list.begin());
    return true;
}

/// @brief Read an RGBA object as created by @ref RgbaToJson
/// @param out_color May be partially-or-not modified, depending on which JSON values are present.
template <size_t TSize>
static void ColorFromJson(const nlohmann::json* j, std::array<float, TSize>* out_color)
{
    if (!j || !j->is_array() || j->size() < TSize)
        return;
    
    for (size_t i = 0; i < TSize; ++i)
    {
        auto& j_num = j->at(i);
        if (!j_num.is_number())
            break;
        float flt = j_num.get<float>();
        flt = max(0, flt);
        flt = min(1, flt);
        out_color->at(i) = flt;
    }
}
/// @brief Convert RGBA to a JSON object
template <size_t TSize>
static nlohmann::json ColorToJson(const std::array<float, TSize>& color) {
    return color;
}

nlohmann::json RenderTweak::ToJson() const
{
    nlohmann::json j = SubclassToJson();
    j.emplace("tweak_type", Helper::tolower(GetName()));
    return j;
}
void RenderTweak::FromJson(const nlohmann::json* json)
{
    std::string class_name;
    if (!Helper::FromJson(json, "tweak_type", class_name))
        return;
    
    bool is_correct_type = false;
    for (RenderTweak::ConstPtr tweak : RenderTweak::default_tweaks)
    {
        if (class_name == Helper::tolower(std::string(tweak->GetName())))
        {
            is_correct_type = true;
            break;
        }
    }
    if (!is_correct_type)
        return;
    
    SubclassFromJson(json);
}
RenderTweak::Ptr RenderTweak::CreateFromJson(const nlohmann::json* json)
{
    std::string class_name;
    if (!Helper::FromJson(json, "tweak_type", class_name))
        return nullptr;
    
    for (RenderTweak::ConstPtr tweak : RenderTweak::default_tweaks)
    {
        if (!Helper::stricmp(class_name, tweak->GetName()))
        {
            RenderTweak::Ptr new_tweak = tweak->Clone();
            new_tweak->SubclassFromJson(json);
            return new_tweak;
        }
    }

    return nullptr;
}

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

nlohmann::json CommandTweak::SubclassToJson() const
{
    nlohmann::json j;
    nlohmann::json j_commands = nlohmann::json::array();
    
    std::vector<Helper::ParsedCommand> parsed_cmds;
    GetCommandList(&parsed_cmds);
    for (auto& cmd : parsed_cmds)
    {
        if (cmd.args.empty())
            j_commands.push_back(std::string(cmd.name));
        else
            j_commands.push_back(std::string(cmd.name) + ' ' + std::string(cmd.args));
    }
    j.emplace("commands", std::move(j_commands));
    return j;
}
void CommandTweak::SubclassFromJson(const nlohmann::json* json)
{
    const nlohmann::json* j_commands = Helper::FromJson(json, "commands");
    if (!j_commands || !j_commands->is_array())
        return;

    commands.clear();
    
    for (auto& j_cmd : *j_commands)
    {
        Helper::ParsedCommand parsed_cmd;

        if (!Helper::ParseNextCommand(j_cmd.get_ref<const std::string&>().c_str(), &parsed_cmd))
            continue;
        
        std::string str_cmd = std::string(parsed_cmd.name);
        if (!parsed_cmd.args.empty())
        {
            str_cmd += ' ';
            str_cmd.append(parsed_cmd.args);
        }
        commands.append(std::move(str_cmd));
        commands += '\n';
    }
}

void ModelTweak::OnMenu()
{
    const char* const ENTITY_POPUP = "##popup_add_class";
    const char* const MODEL_POPUP = "##popup_add_model";

    if (ImGui::BeginCombo("Material", custom_material ? custom_material->GetName().c_str() : MATERIAL_CHOICE_NAME[(int)render_effect]))
    {
        // Purposely don't display the fake CUSTOM value. It only exists to indicate that `custom_material` is used instead.
        for (int i = 0; i < (int)MaterialChoice::CUSTOM; ++i)
        {
            if (ImGui::Selectable(MATERIAL_CHOICE_NAME[i], i == (int)render_effect))
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

nlohmann::json ModelTweak::SubclassToJson() const
{
    nlohmann::json j = {
        {"filter_choice", Helper::tolower(FILTER_CHOICE_NAME[(size_t)filter_choice])},
        {"render_effect", Helper::tolower(MATERIAL_CHOICE_NAME[(int)render_effect])},
        {"color_multiply", ColorToJson(color_multiply)},
        {"filter_player", filter_player},
        {"filter_weapon", filter_weapon},
        {"filter_wearable", filter_wearable},
        {"filter_projectile", filter_projectile},
    };

    if (custom_material)
        j.emplace("custom_material", Helper::tolower(custom_material->GetName()));
    else
        j.emplace("custom_material", nullptr);

    nlohmann::json j_classes = nlohmann::json::array();
    for (ClientClass* cl_class : classes)
        j_classes.emplace_back(cl_class->GetName());
    j.emplace("classes", std::move(j_classes));

    nlohmann::json j_model_paths = nlohmann::json::array();
    for (const std::string& path : model_paths)
        j_model_paths.push_back(path);
    j.emplace("model_paths", std::move(j_model_paths));
    return j;
}
void ModelTweak::SubclassFromJson(const nlohmann::json* json)
{
    std::string filter_choice_name;
    std::string render_effect_name;
    const nlohmann::json* j_color;
    std::string material_name;
    const nlohmann::json* j_classes;
    const nlohmann::json* j_model_paths;

    Helper::FromJson(json, "filter_choice", filter_choice_name);
    Helper::FromJson(json, "render_effect", render_effect_name);
    j_color = Helper::FromJson(json, "color_multiply");
    Helper::FromJson(json, "filter_player", filter_player);
    Helper::FromJson(json, "filter_weapon", filter_weapon);
    Helper::FromJson(json, "filter_wearable", filter_wearable);
    Helper::FromJson(json, "filter_projectile", filter_projectile);
    Helper::FromJson(json, "custom_material", material_name);
    j_classes = Helper::FromJson(json, "classes");
    j_model_paths = Helper::FromJson(json, "model_paths");

    ColorFromJson(j_color, &color_multiply);

    if (!filter_choice_name.empty())
        StringToEnum(FILTER_CHOICE_NAME, filter_choice_name, &filter_choice);

    if (!render_effect_name.empty())
        StringToEnum(MATERIAL_CHOICE_NAME, render_effect_name, &render_effect);
    
    if (!material_name.empty())
    {
        auto it = std::find_if(CustomMaterial::GetAll().begin(), CustomMaterial::GetAll().end(),
            [&](auto& mat){ return Helper::stricmp(material_name, mat->GetName()) == 0; }
        );
        if (it != CustomMaterial::GetAll().end())
            custom_material = *it;
    }
    if (j_classes && j_classes->is_array())
    {
        for (auto& j_name : *j_classes)
        {
            std::string str_name = j_name.get<std::string>();
            ClientClass* next_class = Interfaces::hlclient->GetAllClasses();
            for (; next_class; next_class = next_class->m_pNext)
            {
                if (!Helper::stricmp(str_name, next_class->GetName()))
                {
                    classes.emplace(next_class);
                    break;
                }
            }
        }
    }
    if (j_model_paths && j_model_paths->is_array())
    {
        for (auto& j_path : *j_model_paths)
            model_paths.emplace(j_path.get<std::string>());
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

nlohmann::json MaterialTweak::SubclassToJson() const
{
    nlohmann::json j = {
        {"color_multiply", ColorToJson(color_multiply)},
        {"props", props},
        {"filter_choice", Helper::tolower(FILTER_CHOICE_NAME[(int)filter_choice])},
    };

    nlohmann::json j_groups = nlohmann::json::array();
    for (size_t i = 0; i < TEXTURE_GROUPS.size(); ++i)
    {
        if (groups[i])
            j_groups.emplace_back(Helper::tolower(TEXTURE_GROUPS[i]));
    }
    j.emplace("groups", std::move(j_groups));
    return j;
}
void MaterialTweak::SubclassFromJson(const nlohmann::json* json)
{
    const nlohmann::json* j_color;
    const nlohmann::json* j_groups;
    std::string filter_choice_name;

    j_color = Helper::FromJson(json, "color_multiply");
    j_groups = Helper::FromJson(json, "groups");
    Helper::FromJson(json, "props", props);
    Helper::FromJson(json, "filter_choice", filter_choice_name);

    ColorFromJson(j_color, &color_multiply);
    
    if (j_groups && j_groups->is_array())
    {
        groups.fill(false);
        for (auto& j_str : *j_groups)
        {
            size_t index;
            if (StringToEnum(TEXTURE_GROUPS, j_str.get_ref<const nlohmann::json::string_t&>(), &index))
                groups[index] = true;
        }
    }

    if (!filter_choice_name.empty())
        StringToEnum(FILTER_CHOICE_NAME, filter_choice_name, &filter_choice);
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

    if (ImGui::BeginCombo("HUD visiblity", HUD_CHOICE_NAME[hud]))
    {
        for (int i = 0; i < HUD_CHOICE_NAME.size(); ++i)
        {
            if (ImGui::Selectable(HUD_CHOICE_NAME[i], i == hud))
                hud = (HudChoice)i;
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Pixel shader", pixel_shader ? pixel_shader->GetDisplayName() : "None"))
    {
        if (ImGui::Selectable("None", false))
            pixel_shader = nullptr;
        for (const Shader::PixelShader::ConstPtr& shader : *Shader::PixelShader::GetLoadedShaders())
        {
            if (ImGui::Selectable(shader->GetDisplayName(), false))
                pixel_shader = shader->NewInstance();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
                ImGui::SetTooltip("%s", shader->GetDesc());
        }
        ImGui::EndCombo();
    }

    if (pixel_shader)
    {
        ImGui::PushID("pixel_shader");
        pixel_shader->OnMenu();
        ImGui::PopID();
    }
}

nlohmann::json CameraTweak::SubclassToJson() const
{
    return {
        {"fov", fov},
        {"fov_override", fov_override},
        {"hide_fade", hide_fade},
        {"hud", Helper::tolower(HUD_CHOICE_NAME[hud])},
        {"pixel_shader", pixel_shader ? pixel_shader->ToJson() : nullptr},
    };
}
void CameraTweak::SubclassFromJson(const nlohmann::json* json)
{
    std::string hud_choice_name;

    Helper::FromJson(json, "fov", fov);
    Helper::FromJson(json, "fov_override", fov_override);
    Helper::FromJson(json, "hide_fade", hide_fade);
    Helper::FromJson(json, "hud", hud_choice_name);
    pixel_shader = Shader::PixelShader::CreateFromJson(Helper::FromJson(json, "pixel_shader"));

    if (!hud_choice_name.empty())
        StringToEnum(HUD_CHOICE_NAME, hud_choice_name, &hud);
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

nlohmann::json FogTweak::SubclassToJson() const
{
    return {
        {"fog_enabled", fog_enabled},
        {"fog_start", fog_start},
        {"fog_end", fog_end},
        {"fog_color", ColorToJson(fog_color)},
        {"skyfog_enabled", skyfog_enabled},
        {"skyfog_start", skyfog_start},
        {"skyfog_end", skyfog_end},
        {"skyfog_color", ColorToJson(skyfog_color)},
    };
}
void FogTweak::SubclassFromJson(const nlohmann::json* json)
{
    const nlohmann::json* j_fog_color;
    const nlohmann::json* j_skyfog_color;

    Helper::FromJson(json, "fog_enabled", fog_enabled);
    Helper::FromJson(json, "fog_start", fog_start);
    Helper::FromJson(json, "fog_end", fog_end);
    j_fog_color = Helper::FromJson(json, "fog_color");
    Helper::FromJson(json, "skyfog_enabled", skyfog_enabled);
    Helper::FromJson(json, "skyfog_start", skyfog_start);
    Helper::FromJson(json, "skyfog_end", skyfog_end);
    j_skyfog_color = Helper::FromJson(json, "skyfog_color");

    ColorFromJson(j_fog_color, &fog_color);
    ColorFromJson(j_skyfog_color, &skyfog_color);
}