#pragma once
#include <unordered_set>
#include <vector>
#include <memory>

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

    bool IsActive() const { return m_is_active; }
    bool SetActive(bool active);

    bool m_is_active = false;

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

    bool m_hide = false;
};

class EntityRenderTweak : public RenderTweak
{
public:
    const char* GetName() const override { return "Entities"; }
    std::shared_ptr<RenderTweak> Clone() const override {
        return std::make_shared<EntityRenderTweak>(*this);
    }
    void OnMenu() override;

    /// @brief Hides all entities
    bool hide = false;

    /// @brief A set of entity classes to be included during rendering
    /// If any classes are included, then all others are excluded by default.
    std::unordered_set<std::string> include_classes;

    /// @brief A set of entity classes to be excluded during rendering
    std::unordered_set<std::string> exclude_classes;
};

inline const std::vector<std::shared_ptr<const RenderTweak>> RenderTweak::default_tweaks = {
    std::make_shared<PropRenderTweak>(),
    std::make_shared<EntityRenderTweak>(),
};
