#pragma once
#include <nlohmann/json.hpp>

namespace Helper
{
    /// @brief Get a value pointer from JSON if the key is present
    /// @return `nullptr` if no value was obtained
    template <class TKey>
    const nlohmann::json* FromJson(const nlohmann::json& json, const TKey& key)
    {
        auto it = json.find(key);
        if (it == json.end())
            return nullptr;
        return &(*it);
    }
    /// @brief Get a value pointer from JSON if the key is present
    /// @param json A JSON pointer. `nullptr` is allowed.
    /// @return `nullptr` if no value was obtained
    template <class TKey>
    const nlohmann::json* FromJson(const nlohmann::json* json, const TKey& key)
    {
        if (!json)
            return nullptr;
        return FromJson(json, key);
    }
    
    /// @brief Get a value from JSON if the key is present
    /// @return `true` if a value was obtained
    template <class TValue, class TKey>
    bool FromJson(const nlohmann::json& json, const TKey& key, TValue& value)
    {
        if (const nlohmann::json* found = FromJson(json, key))
        {
            found->get_to(value);
            return true;
        }
        return false;
    }
    /// @brief Get a value from JSON if the key is present
    /// @param json A JSON pointer. `nullptr` is allowed.
    /// @return `true` if a value was obtained
    template <class TValue, class TKey>
    bool FromJson(const nlohmann::json* json, const TKey& key, TValue& value)
    {
        if (!json)
            return false;
        return FromJson(*json, key, value);
    }
}