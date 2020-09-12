#include "Movement.h"
#include "Hooks/ClientHook.h"
#include "SDK/usercmd.h"

void CMovement::StartListening() {
	Listen(EVENT_CREATEMOVE, [this] {return OnCreateMove(); });
}

int CMovement::OnCreateMove()
{
	static auto hook = GETHOOK(CClientHook);
	auto ctx = hook->Context();

	return 0;
}
