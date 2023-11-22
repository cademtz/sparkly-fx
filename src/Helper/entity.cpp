#include "entity.h"
#include <Base/Entity.h>
#include <SDK/client_class.h>
#include <unordered_map>
#include <cstring>

namespace Helper
{

static EntityType RecurseRecvTable(RecvTable* table)
{
    if (!strcmp(table->GetName(), "DT_BasePlayer"))
        return EntityType::PLAYER;
    if (!strcmp(table->GetName(), "DT_BaseCombatWeapon"))
        return EntityType::WEAPON;
    if (!strcmp(table->GetName(), "DT_WearableItem"))
        return EntityType::WEARABLE;
    if (!strcmp(table->GetName(), "DT_BaseProjectile"))
        return EntityType::PROJECTILE;
    if (!strcmp(table->GetName(), "DT_BaseObject"))
        return EntityType::BUILDING;
    
    for (size_t i = 0; i < table->m_nProps; ++i)
    {
        RecvProp* prop = &table->m_pProps[i];
        if (prop->GetType() == DPT_DataTable)
        {
            EntityType type = RecurseRecvTable(prop->GetDataTable());
            if (type != EntityType::UNKNOWN)
                return type;
        }
    }

    return EntityType::UNKNOWN;
}

EntityType GetEntityType(CBaseEntity* entity)
{
    static std::unordered_map<ClientClass*, EntityType> type_map;

    ClientClass* cl_class = entity->GetClientClass();
    auto insertion = type_map.emplace(std::make_pair(cl_class, EntityType::UNKNOWN));
    auto& type = insertion.first->second;
    
    if (insertion.second)
        type = RecurseRecvTable(cl_class->m_pRecvTable);
    return type;
}

}