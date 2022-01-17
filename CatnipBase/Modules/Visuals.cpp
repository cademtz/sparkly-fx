#include "Visuals.h"
#include <Base/Interfaces.h>
#include <Base/Entity.h>
#include "Draw.h"
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
		if (i == Interfaces::engine->GetLocalPlayer())
			continue;

		CBaseEntity* ent = Interfaces::entlist->GetClientEntity(i);
		if (!ent || ent->IsDormant())
			continue;

		Vector origin = ent->Origin();
		Vector maxs = ent->Maxs() + origin, mins = ent->Mins() + origin;
		float boxWidth = (maxs.x - mins.x) / 2.f;
		Vector corners[4] =
		{
			{ origin.x - boxWidth, origin.y - boxWidth, mins.z },
			{ origin.x + boxWidth, origin.y - boxWidth, mins.z },
			{ origin.x + boxWidth, origin.y + boxWidth, mins.z },
			{ origin.x - boxWidth, origin.y + boxWidth, mins.z },
		}, height(0, 0, maxs.z - mins.z);
		ImVec2 screen[8];

		bool visible = true;
		for (int i = 0; i < 4; i++) // Get all screen positions
		{
			if (!gDraw.WorldToScreen(corners[i], screen[i]) || !gDraw.WorldToScreen(corners[i] + height, screen[i + 4]))
			{
				visible = false;
				break;
			}
		}

		if (!visible)
			continue;

		for (int i = 0; i < 4; i++) // Another loop to connect them with lines
		{
			int next = i == 3 ? -3 : 1;
			gDraw.List()->AddLine(screen[i], screen[i + next], ImColor(255, 128, 0));
			gDraw.List()->AddLine(screen[i], screen[i + 4], ImColor(255, 128, 0));
			gDraw.List()->AddLine(screen[i + 4], screen[i + 4 + next], ImColor(255, 128, 0));
		}
	}

	return 0;
}
