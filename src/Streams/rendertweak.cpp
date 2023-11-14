#include "rendertweak.h"
#include "SDK/texture_group_names.h"
#include "SDK/imaterial.h"

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

bool EntityFilterTweak::IsEntityAffected(const std::string& class_name) const
{
    if (filter_choice == FilterChoice::ALL)
        return true;
    
    bool is_in_list = classes.find(class_name) != classes.end();
    return filter_choice == FilterChoice::WHITELIST ? is_in_list : !is_in_list;
}

bool EntityFilterTweak::IsEffectInvisible() const {
    return render_effect == RenderEffect::INVISIBLE || color_multiply[3] == 0;
}

const char* EntityFilterTweak::RenderEffectName(RenderEffect value)
{
    switch (value)
    {
    case RenderEffect::NORMAL: return "Normal";
    case RenderEffect::MATTE: return "Matte";
    case RenderEffect::INVISIBLE: return "Invisible";
    }
    return nullptr;
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
