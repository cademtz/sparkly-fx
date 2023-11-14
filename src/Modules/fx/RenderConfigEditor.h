#pragma once
#include <Modules/BaseModule.h>
#include <Streams/renderconfig.h>
#include <memory>

/**
 * @brief Add, remove, and edit render configs.
 * 
 * This will lock @ref ActiveRenderConfig's mutex when modifying collections (list, set, vector, ...).
 * 
 * While recording, no changes will be made.
 * Otherwise, it is unsafe to iterate or read render configs.
 */
class RenderConfigEditor : public CModule
{
public:
    void StartListening() override;
    
protected:
    int OnMenu();
    void ShowConfigListEditor();
    void PopupConfigRenamer(RenderConfig::Ptr render_config);
    void ShowConfigEditor(RenderConfig::Ptr render_config);
    void PopupTweakCreator(RenderConfig::Ptr render_config);
    void ShowTweakEditor(RenderTweak::Ptr render_tweak);
    /// @brief True if the name already exists 
    bool IsDuplicateName(const std::string& name) const;

    friend class CRecorder;
    /**
     * @brief The list of configs.
     * 
     * Do not read this list, except when recording. Thanks!
     */
    const std::vector<RenderConfig::Ptr>& GetConfigs() const { return m_render_configs; }

private:
    /// @brief A list of every stream. No duplicate names are allowed.
    std::vector<RenderConfig::Ptr> m_render_configs;
    /// @brief The selected config index, or a value >= `m_render_configs.size()`
    size_t m_config_index;
    bool m_preview = false;
};

inline RenderConfigEditor g_render_frame_editor;