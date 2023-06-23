#pragma once
#include <Modules/BaseModule.h>
#include <Streams/renderframe.h>
#include <memory>

class RenderFrameEditor : public CModule
{
public:
    void StartListening() override;
    std::vector<RenderFrame::Ptr>& GetRenderFrames() { return m_render_frames; }
    
protected:
    int OnMenu();
    void ShowFrameListEditor();
    void PopupFrameCreator();
    void ShowFrameEditor(RenderFrame::Ptr render_frame);
    void PopupTweakCreator(RenderFrame::Ptr render_frame);
    void ShowTweakEditor(RenderTweak::Ptr render_tweak);

private:
    std::vector<RenderFrame::Ptr> m_render_frames;
};

inline RenderFrameEditor g_render_frame_editor;