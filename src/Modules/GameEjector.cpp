#include "GameEjector.h"
#include <Base/Base.h>
#include <Hooks/ClientHook.h>
#include <SDK/convar.h>
#include <atomic>
#include <Windows.h>

static GameEjector g_ejector;
static std::atomic<bool> is_ejecting = false;
DWORD WINAPI UnhookThread(LPVOID);

void GameEjector::StartListening() {
    CClientHook::OnFrameStageNotify.Listen(&GameEjector::OnFrameStageNotify, this);
}

void GameEjector::Eject() { is_ejecting = true; }

int GameEjector::OnFrameStageNotify(ClientFrameStage_t stage) {
    if (stage == FRAME_START && is_ejecting) {
        OnEject.DispatchEvent();
        ConVar_Unregister();
        CreateThread(0, 0, &UnhookThread, 0, 0, 0);
    }
	return 0;
}

static DWORD WINAPI UnhookThread(LPVOID)
{
	printf("Unhooking...\n");
	EventSource<void()>::Shutdown();
	CBaseHook::UnHookAll();

	printf("Waiting for any hooked calls to end...\n");
	Sleep(0x1000);

	printf("FreeLibraryAndExitThread\n");
	FreeLibraryAndExitThread(Base::hInst, 0);

	return 0;
}