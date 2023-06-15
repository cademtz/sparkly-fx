#include "Visuals.h"
#include <Base/Interfaces.h>
#include <Base/Entity.h>
#include <SDK/gametrace.h>
#include <SDK/IEngineTrace.h>
#include "Draw.h"
#include "Movement.h"
#include <string>

void CVisuals::StartListening() {
	Listen(EVENT_DRAW, [this] { return OnDraw(); });
}

int CVisuals::OnDraw()
{
	if (!Interfaces::engine->IsInGame())
		return 0;

	// Draw all players //
	for (int i = 1; i <= Interfaces::engine->GetMaxClients(); i++)
	{
		if (i == Interfaces::engine->GetLocalPlayer())
			continue;

		CBaseEntity* ent = Interfaces::entlist->GetClientEntity(i);
		if (!ent || ent->IsDormant())
			continue;

		Vector mins = ent->Mins() + ent->Origin();
		Vector maxs = ent->Maxs() + ent->Origin();
		gDraw.DrawBox3D(maxs, mins, ImColor(255, 128, 0));
	}

	// Draw raytrace info //
	CBasePlayer* local = Interfaces::entlist->GetClientEntity(Interfaces::engine->GetLocalPlayer())->ToPlayer();
	if (!local)
		return 0;

	Vector viewVec;
	AngleVectors(g_movement.GetCameraAng(), &viewVec);

	Ray_t ray;
	trace_t trace;
	CTraceFilterWorldAndPropsOnly filter;

	ray.Init(g_movement.GetCameraPos(), g_movement.GetCameraPos() + viewVec * 5000);

	Interfaces::trace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

	ImVec2 trace_screen;
	if (!gDraw.WorldToScreen(trace.endpos, trace_screen))
		return 0;

	gDraw.DrawBox3D_Radius(trace.endpos, 15, ImColor(0, 128, 255));
	gDraw.DrawText_Outline(trace_screen, ~0, ImColor(0, 0, 0), trace.surface.name);
	return 0;
}