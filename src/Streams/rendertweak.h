#pragma once
#include <unordered_set>
#include <string_view>
#include <vector>
#include <memory>
#include <array>

enum class FilterChoice : int { ALL, WHITELIST, BLACKLIST, _COUNT };

/// @brief Configurable variables for one part of the rendering process.
/// Each subclass will typically provide settings for a specific, hooked render function.
class RenderTweak
{
public:
    using Ptr = std::shared_ptr<RenderTweak>;
    using ConstPtr = std::shared_ptr<const RenderTweak>;
    
    virtual ~RenderTweak() {}
    
    /// @return A user-friendly class name
    virtual const char* GetName() const = 0;
    virtual std::shared_ptr<RenderTweak> Clone() const = 0;
    /// @brief Display the controls to edit the tweak
    virtual void OnMenu() = 0;

    /// @brief An array of every RenderTweak subclass.
    /// Useful for displaying all types.
    static const std::vector<ConstPtr> default_tweaks;
};

class PropRenderTweak : public RenderTweak
{
public:
    const char* GetName() const override { return "Props"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<PropRenderTweak>(*this);
    }
    void OnMenu() override;

    /// @brief Hide all props
    bool hide = false;
};

class EntityFilterTweak : public RenderTweak
{
public:
    enum class MaterialChoice : int { NORMAL, INVISIBLE, CUSTOM, _COUNT = CUSTOM };

    const char* GetName() const override { return "Entities"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<EntityFilterTweak>(*this);
    }
    void OnMenu() override;
    
    /// @return `true` if an entity should be rendered differently than normal
    bool IsEntityAffected(const std::string& entity_class) const;
    /// @return `true` if the effect will make an entity invisible
    bool IsEffectInvisible() const;

    static const char* MaterialChoiceName(MaterialChoice value);

    /// @brief Chooses whether entities should be filtered, and which ones
    FilterChoice filter_choice = FilterChoice::ALL;
    /// @brief An effect given to each entity
    MaterialChoice render_effect = MaterialChoice::NORMAL;
    std::shared_ptr<class CustomMaterial> custom_material = nullptr;
    /// @brief A color multiply given to each entity
    std::array<float, 4> color_multiply = { 1,1,1,1 };

    /// @brief A list that decides which entity types are filtered.
    std::unordered_set<std::string> classes;
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
    std::array<bool, TEXTURE_GROUPS.size()> groups;
    FilterChoice filter_choice = FilterChoice::ALL;
};

class CameraTweak : public RenderTweak
{
public:
    const char* GetName() const override { return "Camera"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<CameraTweak>(*this);
    }
    void OnMenu() override;

    float fov = 90;
    bool fov_override = false;
};

inline const std::vector<RenderTweak::ConstPtr> RenderTweak::default_tweaks = {
    std::make_shared<PropRenderTweak>(),
    std::make_shared<EntityFilterTweak>(),
    std::make_shared<MaterialTweak>(),
    std::make_shared<CameraTweak>(),
};
