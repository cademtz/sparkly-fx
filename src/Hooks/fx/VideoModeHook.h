#include <Hooks/Hooks.h>

DECL_EVENT(EVENT_WRITE_MOVIE_FRAME);

/**
 * @brief Hook the engine's private IVideoMode instance
 */
class VideoModeHook : public CBaseHook
{
public:
    VideoModeHook();

    void Hook() override;
    void Unhook() override;

private:
    void* m_videomode = nullptr;
    CVMTHook m_vmt;

    static void __stdcall Hooked_WriteMoveFrame(UNCRAP const void* info);
};

inline VideoModeHook g_hk_videomode;