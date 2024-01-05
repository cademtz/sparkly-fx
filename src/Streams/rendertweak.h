#pragma once
#include <unordered_set>
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <Helper/json.h>

namespace Helper { class ParsedCommand; }
class CBaseEntity;
class ClientClass;
enum class FilterChoice : int { ALL, WHITELIST, BLACKLIST, _COUNT };

/// @brief Configurable variables for one part of the rendering process.
/// @details Each subclass will typically provide settings for a specific, hooked render function.
class RenderTweak : public Helper::JsonConfigurable
{
public:
    using Ptr = std::shared_ptr<RenderTweak>;
    using ConstPtr = std::shared_ptr<const RenderTweak>;
    
    virtual ~RenderTweak() {}
    
    /// @return A class-specific display name. Must be unique.
    virtual const char* GetName() const = 0;
    virtual std::shared_ptr<RenderTweak> Clone() const = 0;
    /// @brief Display the controls to edit the tweak
    virtual void OnMenu() = 0;
    nlohmann::json ToJson() const override;
    void FromJson(const nlohmann::json* json) override;
    /// @brief Create a new instance from JSON
    /// @return `nullptr` if the json object does not specify the stream type and name
    static Ptr CreateFromJson(const nlohmann::json* json);

    /// @brief An array of every RenderTweak subclass.
    /// Useful for displaying all types.
    static const std::vector<ConstPtr> default_tweaks;

protected:
    /// @brief Convert properties to a JSON object without a `tweak_type` key
    virtual nlohmann::json SubclassToJson() const = 0;
    /// @brief Read properties from a JSON object
    virtual void SubclassFromJson(const nlohmann::json* json) = 0;
};

class CommandTweak : public RenderTweak
{
public:
    const char* GetName() const override { return "Commands"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<CommandTweak>(*this);
    }
    void OnMenu() override;
    /// @brief Split @ref commands into individual commands and add them to a vector
    void GetCommandList(std::vector<Helper::ParsedCommand>* output) const;

    /// @brief Console commands separated by newlines or semicolons
    std::string commands;

protected:
    nlohmann::json SubclassToJson() const override;
    void SubclassFromJson(const nlohmann::json* json) override;
};

class ModelTweak : public RenderTweak
{
public:
    enum class MaterialChoice : int { NORMAL, INVISIBLE, CUSTOM, _COUNT };
    static inline const std::array<const char*, 3> MATERIAL_CHOICE_NAME = {
        "Normal", "Invisible", "Custom"
    };

    const char* GetName() const override { return "Models"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<ModelTweak>(*this);
    }
    void OnMenu() override;
    
    /// @return `true` if an entity should be rendered differently than normal
    bool IsEntityAffected(CBaseEntity* entity) const;
    bool IsModelAffected(const std::string& path) const;
    /// @return `true` if the effect will make an entity invisible
    bool IsEffectInvisible() const;

    /// @brief Chooses whether entities should be filtered, and which ones
    FilterChoice filter_choice = FilterChoice::ALL;
    /// @brief An effect given to each entity
    MaterialChoice render_effect = MaterialChoice::NORMAL;
    std::shared_ptr<class CustomMaterial> custom_material = nullptr;
    /// @brief A color multiply given to each entity
    std::array<float, 4> color_multiply = { 1,1,1,1 };

    /// @brief A list of entity classes to filter
    std::unordered_set<ClientClass*> classes;
    /// @brief A list of model paths to filter. Paths must be lowercase with forward-slashes.
    std::unordered_set<std::string> model_paths;
    bool filter_player = false;
    bool filter_weapon = false;
    bool filter_wearable = false;
    bool filter_projectile = false;

protected:
    nlohmann::json SubclassToJson() const override;
    void SubclassFromJson(const nlohmann::json* json) override;
};

class MaterialTweak : public RenderTweak
{
public:
    static const std::array<const char*, 27> TEXTURE_GROUPS;

    const char* GetName() const override { return "Materials"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<MaterialTweak>(*this);
    }
    void OnMenu() override;

    bool IsMaterialAffected(class IMaterial* material) const;

    /// @brief A color multiply given to each material
    std::array<float, 4> color_multiply = { 1,1,1,1 };
    /// @brief A fixed array containing `true` or `false` for each texture group
    std::array<bool, TEXTURE_GROUPS.size()> groups = {0};
    /// @brief This will affect props specially, since typical material overrides doesn't work on them.
    bool props = false;
    FilterChoice filter_choice = FilterChoice::ALL;

protected:
    nlohmann::json SubclassToJson() const override;
    void SubclassFromJson(const nlohmann::json* json) override;
};

class CameraTweak : public RenderTweak
{
public:
    enum HudChoice : int {
        HUD_DEFAULT, HUD_DISABLED, HUD_ENABLED
    };
    static inline const std::array<const char*, 3> HUD_CHOICE_NAME = {
        "Default", "Disabled", "Enabled"
    };

    const char* GetName() const override { return "Camera"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<CameraTweak>(*this);
    }
    void OnMenu() override;

    float fov = 90;
    bool fov_override = false;
    bool hide_fade = false;
    HudChoice hud = HudChoice::HUD_DEFAULT;

protected:
    nlohmann::json SubclassToJson() const override;
    void SubclassFromJson(const nlohmann::json* json) override;
};

class FogTweak : public RenderTweak
{
public:
    const char* GetName() const override { return "Fog"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<FogTweak>(*this);
    }
    void OnMenu() override;

    bool fog_enabled;
    int fog_start;
    int fog_end;
    std::array<float, 3> fog_color;

    bool skyfog_enabled;
    int skyfog_start;
    int skyfog_end;
    std::array<float, 3> skyfog_color;

protected:
    nlohmann::json SubclassToJson() const override;
    void SubclassFromJson(const nlohmann::json* json) override;
};

inline const std::vector<RenderTweak::ConstPtr> RenderTweak::default_tweaks = {
    std::make_shared<CommandTweak>(),
    std::make_shared<ModelTweak>(),
    std::make_shared<MaterialTweak>(),
    std::make_shared<CameraTweak>(),
    std::make_shared<FogTweak>(),
};
