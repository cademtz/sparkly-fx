#include "Visuals.h"
#include "Base/Interfaces.h"
#include "Base/Entity.h"
#include "Modules/Draw/Draw.h"
#include <string>

void CVisuals::StartListening() {
	Listen(EVENT_DRAW, [this] { return OnDraw(); });
}

int CVisuals::OnDraw()
{
	if (!Interfaces::engine->IsInGame())
		return 0;

	for (int i = 1; i <= Interfaces::engine->GetMaxClients(); i++)
	{
		CBaseEntity ent = Interfaces::entlist->GetClientEntity(i);
		if (!ent.Inst() || ent.IsDormant())
			continue;

		ImVec2 scr;
		if (gDraw.WorldToScreen(ent.Origin(), scr))
		{
			std::string str = "Index: " + std::to_string(i);
			gDraw.List()->AddText(scr, ImColor(255, 128, 0), str.c_str());
		}
	}

	return 0;
}
