#include "EngineHook.h"

#include "Base/Interfaces.h"
#include "Base/Sig.h"

#include "Modules/fx/miscmodule.h"

#ifdef _M_AMD64
#define SIG_READCONSOLECMD "48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 48 8B 89 ? ? ? ? 48 85 C9"
#else
#define SIG_READCONSOLECMD "56 8B F1 8B 8E ? ? ? ? 85 C9 74 ? 80 79"
#endif

void CEngineHook::Hook()
{
    void* readConsoleCommand = reinterpret_cast<void*>(Sig::FindPattern("engine.dll", SIG_READCONSOLECMD));

    if (!readConsoleCommand)
    {
        const char* msg = "echo SparklyFX: " __FUNCTION__
                " failed to find signature. 'Demo Command Filtering' feature will be unavailable!!";

        printf("%s\n", msg + 5);

        Interfaces::engine->ClientCmd_Unrestricted(msg);
        return;
    }

    m_readconcmd_hook.Hook(readConsoleCommand, &CEngineHook::Hooked_CDemoFile_ReadConsoleCommand);
    m_orig_readconcmd = m_readconcmd_hook.Original<CDemoFile_ReadConsoleCommandFn>();

}

void CEngineHook::Unhook()
{
    if (m_orig_readconcmd)
        m_readconcmd_hook.Unhook();
}

const char* CEngineHook::Hooked_CDemoFile_ReadConsoleCommand(void* _this)
{
    const char* commandline = m_orig_readconcmd(_this);

    if (g_misc_module.ShouldIgnoreDemoCommand(commandline))
    {
        printf("Ignoring demo command: %s\n", commandline);
        return "";
    }

    return commandline;
}
