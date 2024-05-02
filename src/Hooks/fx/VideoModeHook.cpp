#include "VideoModeHook.h"
#include <Base/Sig.h>
#include <Base/AsmTools.h>
#include <cstdio>

#define WRITE_MOVIE_FRAME_INDEX 26

VideoModeHook::VideoModeHook() : BASEHOOK(VideoModeHook) {}

void VideoModeHook::Hook()
{
    UINT_PTR videomode_ref = 0;
    size_t videomode_offset = 0;
#ifdef _M_AMD64 
    videomode_ref = Sig::FindPattern("engine.dll", "48 8B 0D ? ? ? ? 48 8D 54 24 ? 44 8B 05 ? ? ? ? ");
    videomode_offset = 3;
#elif defined(_M_IX86)
    videomode_ref = Sig::FindPattern("engine.dll", "8B 0D ? ? ? ? 8B 01 FF 50 20 8B 4D 0C ");
    videomode_offset = 2;
#endif

    if (!videomode_ref)
    {
        printf("IVideoMode could not be found (it may be outdated). Recording will perform slower than normal.\n");
        return;
    }
    
    m_videomode = *AsmTools::Relative<void**>(videomode_ref, videomode_offset);
    m_vmt.Hook(m_videomode);
    m_vmt.Set(WRITE_MOVIE_FRAME_INDEX, &Hooked_WriteMoveFrame);
}

void VideoModeHook::Unhook() {
    m_vmt.Unhook();
}

void __stdcall VideoModeHook::Hooked_WriteMoveFrame(UNCRAP const void* info)
{
    int result = OnWriteMoveFrame.DispatchEvent();
    if (result & EventReturnFlags::NoOriginal)
        return;
    
    typedef void (__thiscall* OriginalFn)(void*, const void*);
    g_hk_videomode.m_vmt.Get<OriginalFn>(WRITE_MOVIE_FRAME_INDEX)(g_hk_videomode.m_videomode, info);
}