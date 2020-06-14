#include "ClientHook.h"
#include "Base/Interfaces.h"

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
}

void CClientHook::Unhook() {
	m_hlhook.Unhook();
}

void CClientHook::HLCreateMove(int sequence_number, float input_sample_frametime, bool active)
{
	// TODO: Hideous
	static HLCreateMoveFn_t original = m_hlhook.Get<HLCreateMoveFn_t>(Interfaces::hlclient->GetOffset(Off_CreateMove));


	original(Interfaces::hlclient, sequence_number, input_sample_frametime, active);
}

void CClientHook::FrameStageNotify(ClientFrameStage_t curStage)
{
	static FrameStageNotifyFn_t original = m_hlhook.Get<FrameStageNotifyFn_t>(Interfaces::hlclient->GetOffset(Off_FrameStageNotify));

	original(Interfaces::hlclient, curStage);
}

void __stdcall CClientHook::Hooked_HLCreateMove(int sequence_number, float input_sample_frametime, bool active)
{
	static auto hook = GETHOOK(CClientHook);

	auto ctx = hook->Context();
	ctx.active = active, ctx.input_sample_frametime = input_sample_frametime;

	int flags = hook->PushEvent(EVENT_HLCREATEMOVE);
	if (flags & Return_NoOriginal)
		return;

	hook->HLCreateMove(sequence_number, input_sample_frametime, active);
}

void __stdcall CClientHook::Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	static auto hook = GETHOOK(CClientHook);

	auto ctx = hook->Context();
	ctx.curStage = curStage;

	int flags = hook->PushEvent(EVENT_FRAMESTAGENOTIFY);
	if (flags & Return_NoOriginal)
		return;

	hook->FrameStageNotify(curStage);
}
