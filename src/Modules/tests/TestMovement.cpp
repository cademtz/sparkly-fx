#include "TestMovement.h"
#include <SDK/usercmd.h>
#include <SDK/in_buttons.h>
#include <SDK/view_shared.h>
#include <SDK/gametrace.h>
#include <SDK/IEngineTrace.h>
#include <Hooks/ClientHook.h>
#include <Base/Interfaces.h>
#include <Base/Entity.h>
#include <stdio.h>
#include <stack>
#include <Modules/Menu.h>
#include <Modules/Draw.h>

#define MAX_TRACE 10000

static CTraceFilterWorldOnly worldTraceFilter;

void TestMovement::StartListening()
{
	CMenu::OnMenu.ListenNoArgs(&TestMovement::OnMenu, this);
	CDraw::OnDraw.ListenNoArgs(&TestMovement::OnDraw, this);
	CClientHook::OnCreateMove.Listen(&TestMovement::OnCreateMove, this);
	CClientHook::OnOverrideView.Listen(&TestMovement::OnOverrideView, this);
}

int TestMovement::OnMenu()
{
	if (ImGui::CollapsingHeader("Movement"))
	{
		ImGui::BeginGroup();

		ImGui::Checkbox("Fakelag", &m_fakelag);
		ImGui::Checkbox("Freecam", &m_freecam);
		ImGui::SliderFloat("Freecam speed", &m_freecam_speed, 0, 50);

		static const char* previews[] = { "None", "Start", "End" };
		assert(sizeof(previews) / sizeof(previews[0]) == _PathEdit_EnumEnd);

		ImGui::Checkbox("Show path", &m_pathDraw);

		if (ImGui::BeginCombo("Edit path", previews[m_pathEdit]))
		{
			for (int i = PathEdit_None; i < _PathEdit_EnumEnd; ++i)
			{
				bool is_selected = i == m_pathEdit;

				if (ImGui::Selectable(previews[i], is_selected))
					m_pathEdit = (EPathEdit)i;
			}
			ImGui::EndCombo();
		}

		ImGui::EndGroup();
	}
	return 0;
}

int TestMovement::OnDraw()
{
	if (!ShouldDrawPaths())
		return 0;

	ImVec2 screen;

	gDraw.DrawBox3D_Radius(m_pathStart, 25, ImColor(230, 80, 80), 2);
	if (gDraw.WorldToScreen(m_pathStart, screen))
		gDraw.DrawText_Outline(screen, ~0, ImColor(0, 0, 0), "[Path]\nstart");

	gDraw.DrawBox3D_Radius(m_pathEnd, 25, ImColor(80, 210, 80), 2);
	if (gDraw.WorldToScreen(m_pathEnd, screen))
		gDraw.DrawText_Outline(screen, ~0, ImColor(0, 0, 0), "[Path]\nend");

	return 0;
}

int TestMovement::OnCreateMove(bool& result, float, CUserCmd* cmd)
{
	CBasePlayer* local = Interfaces::entlist->GetClientEntity(Interfaces::engine->GetLocalPlayer())->ToPlayer();

	if (m_fakelag)
	{
		//if (cmd->tick_count() % 14)
		//	ctx->hl_create_move.bSendPacket = false;
	}

	if (m_pathEdit != PathEdit_None && cmd->buttons() & IN_ATTACK)
	{
		cmd->buttons() &= ~IN_ATTACK;

		trace_t trace;
		if (SurfaceUnderCrosshair(&trace, true))
		{
			trace.endpos += trace.plane.normal * 25;

			switch (m_pathEdit)
			{
			case PathEdit_Start: m_pathStart = trace.endpos; break;
			case PathEdit_End: m_pathEnd = trace.endpos; break;
			}
		}
	}

	if (IsInFreecam())
	{
		// TODO: Stuck if looking at ground. Cmd viewangles won't budge.
		// For now, just don't bother hiding freelook from usercmd.
		//m_freecamAng += cmd->viewangles() - m_lastView;
		m_freecamAng = cmd->viewangles();

		Vector vForward, vRight, vUp;
		AngleVectors(m_freecamAng, &vForward, &vRight, &vUp);

		vForward *= m_freecam_speed;
		vRight *= m_freecam_speed;
		vUp *= m_freecam_speed;

		if (cmd->buttons() & IN_FORWARD)
			m_freecamVec += vForward;
		if (cmd->buttons() & IN_BACK)
			m_freecamVec -= vForward;
		if (cmd->buttons() & IN_MOVERIGHT)
			m_freecamVec += vRight;
		if (cmd->buttons() & IN_MOVELEFT)
			m_freecamVec -= vRight;

		cmd->forwardmove() = 0.f;
		cmd->sidemove() = 0.f;
		cmd->upmove() = 0.f;
		cmd->buttons() = 0;
		cmd->impulse() = 0;
	}
	else
	{
		m_freecamVec = local->Origin() + local->ViewOffset();
		m_freecamAng = cmd->viewangles();
		m_lastView = cmd->viewangles();
	}

	return 0;
}

int TestMovement::OnOverrideView(CViewSetup* pSetup)
{
	if (m_freecam)
	{
		pSetup->angles = m_freecamAng;
		pSetup->origin = m_freecamVec;
	}

	return 0;
}

bool TestMovement::ShouldDrawPaths() {
	return m_pathDraw && Interfaces::engine->IsInGame();
}


bool TestMovement::SurfaceUnderCrosshair(trace_t* out_Trace, bool GoInsideMap, ITraceFilter* Filter)
{
	if (!Filter)
		Filter = &worldTraceFilter;

	Vector viewVec;
	AngleVectors(GetCameraAng(), &viewVec);

	Ray_t ray;
	ray.Init(GetCameraPos(), GetCameraPos() + viewVec * MAX_TRACE);
	Interfaces::trace->TraceRay(ray, MASK_PLAYERSOLID, Filter, out_Trace);

	bool insideMap = !Interfaces::trace->PointOutsideWorld(out_Trace->endpos);

	if (!insideMap && out_Trace->DidHit() && GoInsideMap) // Re-trace past that wall
	{
		ray.m_Start = out_Trace->endpos + viewVec;
		Interfaces::trace->TraceRay(ray, MASK_PLAYERSOLID, Filter, out_Trace);
		insideMap = !Interfaces::trace->PointOutsideWorld(out_Trace->endpos);
	}

	return insideMap;
}
