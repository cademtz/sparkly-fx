#pragma once
#include <unordered_set>
#include <string_view>
#include <vector>
#include <memory>
#include <array>

/// @brief Configurable variables for one part of the rendering process.
/// Each subclass will typically provide settings for a specific, hooked render function.
class RenderTweak
{
public:
    using Ptr = std::shared_ptr<RenderTweak>;
    
    virtual ~RenderTweak() {}
    
    /// @return A user-friendly class name
    virtual const char* GetName() const = 0;
    virtual std::shared_ptr<RenderTweak> Clone() const = 0;
    /// @brief Display the controls to edit the tweak
    virtual void OnMenu() = 0;

    /// @brief An array of every RenderTweak subclass.
    /// Useful for displaying all types.
    static const std::vector<std::shared_ptr<const RenderTweak>> default_tweaks;
};

class PropRenderTweak : public RenderTweak
{
public:
    const char* GetName() const override { return "Props"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<PropRenderTweak>(*this);
    }
    void OnMenu() override;

    bool hide = false;
};

class EntityFilterTweak : public RenderTweak
{
public:
    enum class FilterChoice : int { ALL, WHITELIST, BLACKLIST, _COUNT };
    enum class RenderEffect : int { NORMAL, MATTE, INVISIBLE, _COUNT };

    const char* GetName() const override { return "Entity effects"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<EntityFilterTweak>(*this);
    }
    void OnMenu() override;
    
    /// @return `true` if an entity should be rendered differently than normal
    bool IsEntityAffected(const std::string& entity_class) const;
    /// @return `true` if the effect will make an entity invisible
    bool IsEffectInvisible() const;

    static const char* FilterChoiceName(FilterChoice value);
    static const char* RenderEffectName(RenderEffect value);

    /// @brief Chooses whether entities should be filtered, and which ones
    FilterChoice filter_choice = FilterChoice::ALL;
    /// @brief An effect given to each entity
    RenderEffect render_effect = RenderEffect::MATTE;
    /// @brief A color multiply given to each entity
    std::array<float, 4> color_multiply = { 1,1,1,1 };

    /// @brief A list that decides which entity types are filtered.
    std::unordered_set<std::string> classes;
};

inline const std::vector<std::shared_ptr<const RenderTweak>> RenderTweak::default_tweaks = {
    std::make_shared<PropRenderTweak>(),
    std::make_shared<EntityFilterTweak>(),
};
