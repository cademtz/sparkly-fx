#include "ClientHook.h"
#include "Base/Interfaces.h"
#include "Base/Sig.h"

CClientHook::CClientHook() : BASEHOOK(CClientHook)
{
	RegisterEvent(EVENT_HLCREATEMOVE);
	RegisterEvent(EVENT_FRAMESTAGENOTIFY);
}

void CClientHook::Hook()
{
	m_hlhook.Hook(Interfaces::hlclient->Inst());
	m_hlhook.Set(Interfaces::hlclient->GetOffset(Off_CreateMove), Hooked_HLCreateMove);
	m_hlhook.Set(Interfaces::hlclient->GetOffset(Off_FrameStageNotify), Hooked_FrameStageNotify);

	m_clhook.Hook(Interfaces::client->Inst());
	//m_clhook.Set(Interfaces::client->GetOffset(Off_CreateMove), Hooked_CreateMove);
}

void CClientHook::Unhook() {
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
	static auto original = m_clhook.Get<CreateMoveFn_t>(Interfaces::client->GetOffset(Off_CreateMove));
	return original(Interfaces::client->Inst(), flInputSampleTime, cmd);
}

void __stdcall CClientHook::Hooked_HLCreateMove(int sequence_number, float input_sample_frametime, bool active)
{
	static auto hook = GETHOOK(CClientHook);
	auto ctx = hook->Context();
	ctx->active = active, ctx->input_sample_frametime = input_sample_frametime;

	int flags = hook->PushEvent(EVENT_HLCREATEMOVE);
	if (flags & Return_NoOriginal)
		return;

	hook->HLCreateMove(sequence_number, input_sample_frametime, active);
}

void __stdcall CClientHook::Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	static auto hook = GETHOOK(CClientHook);

	auto ctx = hook->Context();
	ctx->curStage = curStage;

	int flags = hook->PushEvent(EVENT_FRAMESTAGENOTIFY);
	if (flags & Return_NoOriginal)
		return;

	hook->FrameStageNotify(curStage);
}

bool __fastcall CClientHook::Hooked_CreateMove(void* thisptr, void* edx, float flInputSampleTime, CUserCmd* cmd)
{
	static auto hook = GETHOOK(CClientHook);
	auto ctx = hook->Context();

	ctx->result = hook->CreateMove(flInputSampleTime, cmd);
	ctx->input_sample_frametime = flInputSampleTime;
	//ctx.cmd = CUserCmd_Wrap(cmd);

	hook->PushEvent(EVENT_CREATEMOVE);

    return ctx->result;
}
