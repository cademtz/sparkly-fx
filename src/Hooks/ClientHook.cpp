#include "ClientHook.h"
#include <Base/AsmTools.h>
#include <Base/Interfaces.h>
#include <Base/Sig.h>
#include <SDK/usercmd.h>
#include <intrin.h>

CClientHook::CClientHook() : BASEHOOK(CClientHook)
{
	RegisterEvent(EVENT_CREATEMOVE);
	RegisterEvent(EVENT_HLCREATEMOVE);
	RegisterEvent(EVENT_FRAMESTAGENOTIFY);
	RegisterEvent(EVENT_OVERRIDEVIEW);
}

void CClientHook::Hook()
{
	m_hlhook.Hook(Interfaces::hlclient->Inst());
	m_hlhook.Set(Interfaces::hlclient->GetOffset(Off_CreateMove), Hooked_HLCreateMove);
	m_hlhook.Set(Interfaces::hlclient->GetOffset(Off_FrameStageNotify), Hooked_FrameStageNotify);

	m_clhook.Hook(Interfaces::client->Inst());
	m_clhook.Set(Interfaces::client->GetOffset(Off_CreateMove), Hooked_CreateMove);
	m_clhook.Set(Interfaces::client->GetOffset(Off_OverrideView), Hooked_OverrideView);
}

void CClientHook::Unhook()
{
	m_hlhook.Unhook();
	m_clhook.Unhook();
}

void CClientHook::HLCreateMove(int sequence_number, float input_sample_frametime, bool active)
{
	// TODO: Hideous
	static auto original = m_hlhook.Get<HLCreateMoveFn_t>(Interfaces::hlclient->GetOffset(Off_CreateMove));
	original(Interfaces::hlclient->Inst(), sequence_number, input_sample_frametime, active);
}

void CClientHook::FrameStageNotify(ClientFrameStage_t curStage)
{
	static auto original = m_hlhook.Get<FrameStageNotifyFn_t>(Interfaces::hlclient->GetOffset(Off_FrameStageNotify));
	original(Interfaces::hlclient->Inst(), curStage);
}

bool CClientHook::CreateMove(float flInputSampleTime, CUserCmd* cmd)
{
	static auto original = m_clhook.Get<void*>(Interfaces::client->GetOffset(Off_CreateMove));
	return ((CreateMoveFn_t)original)(Interfaces::client->Inst(), flInputSampleTime, cmd);
}

bool CClientHook::OverrideView(CViewSetup* pSetup)
{
	static auto original = m_clhook.Get<OverrideViewFn_t>(Interfaces::client->GetOffset(Off_OverrideView));
	original(Interfaces::hlclient->Inst(), pSetup);
	return false;
}

void __stdcall CClientHook::Hooked_HLCreateMove(UNCRAP int sequence_number, float input_sample_frametime, bool active)
{
	bool bSendPacket = true;
	UINT_PTR* baseptr = (UINT_PTR*)_AddressOfReturnAddress() - 1;
	static int off = -1;

	if constexpr (Base::Win64)
		bSendPacket = AsmTools::GetR14();
	else if (Interfaces::engine->GetAppID() != AppId_CSGO)
		bSendPacket = *(*(bool**)baseptr - 1);

	auto ctx = &g_hk_client.Context()->hl_create_move;
	ctx->active = active, ctx->input_sample_frametime = input_sample_frametime, ctx->bSendPacket = bSendPacket;

	int flags = g_hk_client.PushEvent(EVENT_HLCREATEMOVE);

	if (!(flags & Return_NoOriginal))
		g_hk_client.HLCreateMove(sequence_number, input_sample_frametime, active);

	if constexpr (Base::Win64)
		AsmTools::SetR14((void*)ctx->bSendPacket);
	else if (Interfaces::engine->GetAppID() != AppId_CSGO)
		*(*(bool**)baseptr + off) = ctx->bSendPacket;
}

void __stdcall CClientHook::Hooked_FrameStageNotify(UNCRAP ClientFrameStage_t curStage)
{
	g_hk_client.Context()->curStage = curStage;

	int flags = g_hk_client.PushEvent(EVENT_FRAMESTAGENOTIFY);
	if (flags & Return_NoOriginal)
		return;

	g_hk_client.FrameStageNotify(curStage);
}

bool __stdcall CClientHook::Hooked_CreateMove(UNCRAP float flInputSampleTime, CUserCmd* cmd)
{
	auto ctx = g_hk_client.Context();

	ctx->create_move.result = g_hk_client.CreateMove(flInputSampleTime, cmd);
	ctx->create_move.flInputSampleTime = flInputSampleTime;

	switch (Interfaces::engine->GetAppID())
	{
	case AppID_GMod:
		ctx->create_move.cmd = (CUserCmd*)((void**)cmd - 1); // Offset missing VMT
		break;
	default:
		ctx->create_move.cmd = cmd;
	}

	g_hk_client.PushEvent(EVENT_CREATEMOVE);
    return ctx->create_move.result;
}

void __stdcall CClientHook::Hooked_OverrideView(UNCRAP CViewSetup* pSetup)
{
	auto ctx = g_hk_client.Context();
	
	ctx->pSetup = pSetup;

	if (g_hk_client.PushEvent(EVENT_OVERRIDEVIEW) & Return_NoOriginal)
		return;
	g_hk_client.OverrideView(pSetup);
}
