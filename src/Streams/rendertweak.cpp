#include "rendertweak.h"
#include <SDK/texture_group_names.h>
#include <SDK/imaterial.h>
#include <Base/Entity.h>
#include <Helper/engine.h>
#include <Helper/str.h>

const std::array<const char*, 27> MaterialTweak::TEXTURE_GROUPS = {
    TEXTURE_GROUP_LIGHTMAP,
    TEXTURE_GROUP_WORLD,
    TEXTURE_GROUP_MODEL,
    TEXTURE_GROUP_VGUI,
    TEXTURE_GROUP_PARTICLE,
    TEXTURE_GROUP_DECAL,
    TEXTURE_GROUP_SKYBOX,
    TEXTURE_GROUP_CLIENT_EFFECTS,
    TEXTURE_GROUP_OTHER,
    TEXTURE_GROUP_PRECACHED,
    TEXTURE_GROUP_CUBE_MAP,
    TEXTURE_GROUP_RENDER_TARGET,
    TEXTURE_GROUP_UNACCOUNTED,
    TEXTURE_GROUP_STATIC_INDEX_BUFFER,
    TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP,
    TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR,
    TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD,
    TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS,
    TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER,
    TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER,
    TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER,
    TEXTURE_GROUP_DEPTH_BUFFER,
    TEXTURE_GROUP_VIEW_MODEL,
    TEXTURE_GROUP_PIXEL_SHADERS,
    TEXTURE_GROUP_VERTEX_SHADERS,
    TEXTURE_GROUP_RENDER_TARGET_SURFACE,
    TEXTURE_GROUP_MORPH_TARGETS,
};

void CommandTweak::GetCommandList(std::vector<Helper::ParsedCommand>* output) const
{
    size_t offset = 0;
    Helper::ParsedCommand cmd;
    while (size_t read = Helper::ParseNextCommand(commands.c_str() + offset, &cmd))
    {
        offset += read;
        output->emplace_back(cmd);
    }
}

bool ModelTweak::IsEntityAffected(CBaseEntity* entity) const
{
    if (filter_choice == FilterChoice::ALL)
        return true;
    
    bool is_specified = false;
    switch (Helper::GetEntityType(entity))
    {
    case Helper::EntityType::PLAYER: is_specified |= filter_player; break;
    case Helper::EntityType::WEAPON: is_specified |= filter_weapon; break;
    case Helper::EntityType::WEARABLE: is_specified |= filter_wearable; break;
    case Helper::EntityType::PROJECTILE: is_specified |= filter_projectile; break;
    }
    
    if (!is_specified)
        is_specified |= classes.find(entity->GetClientClass()) != classes.end();
    return filter_choice == FilterChoice::WHITELIST ? is_specified : !is_specified;
}

bool ModelTweak::HasEntityFilters() const
{
    return !classes.empty();
}

bool ModelTweak::IsModelAffected(const std::string& path) const
{
    if (filter_choice == FilterChoice::ALL)
        return true;

    for (auto& model_path : model_paths)
    {
		if (Helper::CaseInsensitivePathCompare(path.c_str(), model_path.c_str()) == 0)
			return filter_choice == FilterChoice::WHITELIST;
	}

    return filter_choice == FilterChoice::BLACKLIST;
}

bool ModelTweak::IsEffectInvisible() const {
    return render_effect == MaterialChoice::INVISIBLE || color_multiply[3] == 0;
}

bool MaterialTweak::IsMaterialAffected(class IMaterial* material) const
{
    if (filter_choice == FilterChoice::ALL)
        return true;
    
    bool is_match = false;
    
    // TODO: Make this a hash set
    for (size_t i = 0; i < groups.size(); ++i)
    {
        if (!groups[i])
            continue;
        if (!strcmp(material->GetTextureGroupName(), MaterialTweak::TEXTURE_GROUPS[i]))
        {
            is_match = true;
            break;
        }
    }

    return filter_choice == FilterChoice::WHITELIST ? is_match : !is_match;
}
