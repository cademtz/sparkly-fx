#include <Hooks/Hooks.h>

/**
 * @brief Hook the engine's private IVideoMode instance to stop unnecessary ReadPixels calls
 */
class VideoModeHook : public CBaseHook
{
public:
    VideoModeHook();

    void Hook() override;
    void Unhook() override;

    static inline EventSource<void()> WriteMoveFrameEvent;

private:
    void* m_videomode = nullptr;
    CVMTHook m_vmt;

    static void __stdcall Hooked_WriteMoveFrame(UNCRAP const void* info);
};

inline VideoModeHook g_hk_videomode;