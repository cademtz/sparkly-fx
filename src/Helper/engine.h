#pragma once
#include <optional>
#include <variant>
#include <string>
#include <mutex>

class CBaseEntity;
class ConVar;

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

    /// @brief A ConVar wrapper that stores and restores its original value.
    class RestoringConVar
    {
    public:
        RestoringConVar(ConVar* var) : m_var(var) {}
        RestoringConVar(RestoringConVar&& other)
        {
            m_var = other.m_var;
            m_is_stored = other.m_is_stored;
            m_float_value = other.m_float_value;
            m_str_value = std::move(other.m_str_value);
            
            other.m_var = nullptr;
            other.m_is_stored = false;
        }
        void SetValue(const char* value);
        void SetValue(float value);
        void SetValue(int value);
        /// @brief Restore the ConVar to its original value
        void Restore();
        /// @brief Whether the ConVar has been overrided 
        bool IsSet() const { return m_is_stored; }

    private:
        void StoreOldValue();

        ConVar* m_var;
        bool m_is_stored = false;
        std::string m_str_value;
        float m_float_value;
        std::mutex m_mutex;
    };
}