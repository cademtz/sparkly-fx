#include "engine.h"
#include <Base/Entity.h>
#include <SDK/client_class.h>
#include <SDK/convar.h>
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

void RestoringConVar::SetValue(const char* value)
{
    StoreOldValue();
    m_var->SetValue(value);
}
void RestoringConVar::SetValue(float value)
{
    StoreOldValue();
    m_var->SetValue(value);
}
void RestoringConVar::SetValue(int value)
{
    StoreOldValue();
    m_var->SetValue(value);
}

void RestoringConVar::Restore()
{
    std::lock_guard lock{m_mutex};
    if (!m_is_stored)
        return;
    m_is_stored = false;

    if (m_var->IsFlagSet(FCVAR_NEVER_AS_STRING))
        m_var->SetValue(m_float_value);
    else
        m_var->SetValue(m_str_value.c_str());
}

void RestoringConVar::StoreOldValue()
{
    std::lock_guard lock{m_mutex};
    if (m_is_stored)
        return;
    m_is_stored = true;

    if (m_var->IsFlagSet(FCVAR_NEVER_AS_STRING))
        m_float_value = m_var->GetFloat();
    else
    {
        const char* str = m_var->GetString();
        m_str_value.resize(std::strlen(str));
        memcpy(m_str_value.data(), str, m_str_value.length());
    }
}

}