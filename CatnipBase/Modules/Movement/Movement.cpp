#include "Movement.h"
#include "Hooks/ClientHook.h"
#include "SDK/usercmd.h"
#include "Modules/Menu/Menu.h"

void CMovement::StartListening()
{
	Listen(EVENT_MENU, [this] { return OnMenu(); });
	Listen(EVENT_CREATEMOVE, [this] { return OnCreateMove(); });
}

int CMovement::OnMenu()
{
	ImGui::Checkbox("Fakelag", &m_fakelag);
	return 0;
}

int CMovement::OnCreateMove()
{
	static auto hook = GETHOOK(CClientHook);
	auto ctx = hook->Context();

	if (m_fakelag)
	{
		if (ctx->cmd->tick_count % 14)
			ctx->bSendPacket = false;
	}

	return 0;
}
