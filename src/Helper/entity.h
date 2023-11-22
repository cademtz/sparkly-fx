#pragma once

class CBaseEntity;

namespace Helper
{
    enum class EntityType
    {
        UNKNOWN,
        PLAYER,
        WEAPON,
        WEARABLE,
        PROJECTILE,
        BUILDING, // Team Fortress 2 buildings
    };

    EntityType GetEntityType(CBaseEntity* entity);
}