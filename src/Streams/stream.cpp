#include "stream.h"
#include <nlohmann/json.hpp>
#include "materials.h"
#include <Shaders/shaders.h>
#include <Base/Interfaces.h>
#include <SDK/texture_group_names.h>

Stream::Ptr Stream::Clone(std::string&& new_name) const
{
    Stream::Ptr clone = std::make_shared<Stream>(std::move(new_name));
    for (auto& tweak : m_tweaks)
        clone->m_tweaks.emplace_back(std::move(tweak->Clone()));
    return clone;
}

nlohmann::json Stream::ToJson() const
{
    nlohmann::json j = {
        {"name", GetName()}
    };

    nlohmann::json j_tweak_arr = nlohmann::json::array();
    for (RenderTweak::ConstPtr tweak : m_tweaks)
        j_tweak_arr.emplace_back(tweak->ToJson());
    j.emplace("tweaks", std::move(j_tweak_arr));
    return j;
}
void Stream::FromJson(const nlohmann::json* j)
{
    if (!j)
        return;
    
    std::string safe_name;
    if (Helper::FromJson(j, "name", safe_name))
    {
        for (size_t i = 0; i < safe_name.size(); ++i)
        {
            if (safe_name[i] < 0x20 || safe_name[i] > 0x7E)
            {
                safe_name.erase(safe_name.begin() + i);
                --i;
            }
        }

        if (safe_name.empty())
            safe_name = "unnamed";
        if (safe_name.size() > MAX_PATH)
            safe_name.resize(MAX_PATH);
        m_name = std::move(safe_name);
    }

    const nlohmann::json* j_tweak_arr = Helper::FromJson(j, "tweaks");
    if (j_tweak_arr && j_tweak_arr->is_array())
    {
        for (auto& j_tweak : *j_tweak_arr)
        {
            RenderTweak::Ptr tweak = RenderTweak::CreateFromJson(&j_tweak);
            if (tweak)
                m_tweaks.emplace_back(std::move(tweak));
        }
    }
}
Stream::Ptr Stream::CreateFromJson(const nlohmann::json* json)
{
    if (!json)
        return nullptr;
    Stream::Ptr stream = std::make_shared<Stream>("");
    stream->FromJson(json);
    if (stream->m_name.empty())
        return nullptr;
    return stream;
}

const std::vector<std::shared_ptr<const Stream>>& Stream::GetPresets()
{
    static std::vector<Stream::ConstPtr> presets = MakePresets();
    return presets;
}

std::vector<Stream::ConstPtr> Stream::MakePresets()
{
    std::vector<Stream::ConstPtr> vec;
    
    // Player matte
    {
        Stream::Ptr matte = std::make_shared<Stream>("Player matte");

        auto entities = std::make_shared<ModelTweak>();
        entities->render_effect = ModelTweak::MaterialChoice::CUSTOM;
        entities->custom_material = CustomMaterial::GetMatte();
        entities->color_multiply = {0,1,0,1};
        entities->filter_choice = FilterChoice::WHITELIST;
        entities->filter_player = true;
        entities->filter_weapon = true;
        entities->filter_wearable = true;
        matte->m_tweaks.emplace_back(std::move(entities));

        auto fog = std::make_shared<FogTweak>();
        fog->fog_enabled = true;
        matte->m_tweaks.emplace_back(std::move(fog));

        auto cam = std::make_shared<CameraTweak>();
        cam->hud = CameraTweak::HUD_DISABLED;
        matte->m_tweaks.emplace_back(std::move(cam));

        auto misc = std::make_shared<CommandTweak>();
        misc->commands =
            "r_skybox 0\n"
            "r_3dsky 0\n"
            "glow_outline_effect_enable 0\n"
            "r_screenoverlay off";
        // Particles are intentionally left enabled, so they may obscure the player matte
        matte->m_tweaks.emplace_back(std::move(misc));

        // Get rid of the "client effects" materials by making them black
        auto materials = std::make_shared<MaterialTweak>();
        materials->filter_choice = FilterChoice::WHITELIST;
        materials->color_multiply = {0,0,0,1};
        materials->props = true;
        for (size_t i = 0; i < MaterialTweak::TEXTURE_GROUPS.size(); ++i)
        {
            const char* group = MaterialTweak::TEXTURE_GROUPS[i];
            materials->groups[i] = !strcmp(group, TEXTURE_GROUP_CLIENT_EFFECTS);
        }
        matte->m_tweaks.push_back(std::move(materials));

        vec.emplace_back(std::move(matte));
    }
    // Depth
    {
        Stream::Ptr depth = std::make_shared<Stream>("Depth");

        auto cam = std::make_shared<CameraTweak>();
        cam->pixel_shader = Shader::PixelShader::GetLoadedShader<Shader::DepthLinear>();
        depth->m_tweaks.emplace_back(std::move(cam));

        auto misc = std::make_shared<CommandTweak>();
        misc->commands = "r_drawvgui 0";
        depth->m_tweaks.push_back(std::move(misc));
        
        vec.emplace_back(std::move(depth));
    }

    return vec;
}