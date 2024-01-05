#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Helper
{
    /**
     * @brief The interface to save/load instance properties with JSON data.
     */
    class JsonConfigurable
    {
    public:
        virtual ~JsonConfigurable() {}
        /**
         * @brief Read properties from JSON
         * @param json A JSON pointer. May be `nullptr`.
         * 
         * Use the JSON helpers to safely read values which may not be present.
         * @see Helper::FromJson
         */
        virtual void FromJson(const nlohmann::json* json) = 0;
        /// @brief Write properties to JSON
        virtual nlohmann::json ToJson() const = 0;
    };

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
        return FromJson(*json, key);
    }
    
    /// @brief Get a value from JSON if the key is present
    /// @return `true` if a value was obtained
    template <class TValue, class TKey>
    bool FromJson(const nlohmann::json& json, const TKey& key, TValue& value)
    {
        if (const nlohmann::json* found = FromJson(json, key))
        {
            try {
                found->get_to(value);
            } catch (nlohmann::json::exception e) {
                return false;
            }
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