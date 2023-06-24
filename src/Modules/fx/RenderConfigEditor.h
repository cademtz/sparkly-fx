#pragma once
#include <Modules/BaseModule.h>
#include <Streams/renderconfig.h>
#include <memory>

class RenderConfigEditor : public CModule
{
public:
    void StartListening() override;
    std::vector<RenderConfig::Ptr>& GetRenderConfigs() { return m_render_configs; }
    
protected:
    int OnMenu();
    void ShowConfigListEditor();
    void PopupConfigCreator();
    void ShowConfigEditor(RenderConfig::Ptr render_config);
    void PopupTweakCreator(RenderConfig::Ptr render_config);
    void ShowTweakEditor(RenderTweak::Ptr render_tweak);

private:
    std::vector<RenderConfig::Ptr> m_render_configs;
    bool m_preview = false;
};

inline RenderConfigEditor g_render_frame_editor;