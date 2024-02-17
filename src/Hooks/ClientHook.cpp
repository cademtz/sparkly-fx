#include "ClientHook.h"
#include <Base/AsmTools.h>
#include <Base/Interfaces.h>
#include <Base/Sig.h>
#include <SDK/usercmd.h>
#include <intrin.h>

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
	uintptr_t* baseptr = static_cast<uintptr_t*>(_AddressOfReturnAddress()) - 1;
	static int off = -1;

	if constexpr (Base::Win64)
		bSendPacket = AsmTools::GetR14();
	else if (Interfaces::engine->GetAppID() != static_cast<int>(EAppID::CSGO))
		bSendPacket = *(*reinterpret_cast<bool**>(baseptr) - 1);

	int flags = OnHLCreateMove.DispatchEvent(sequence_number, input_sample_frametime, active, &bSendPacket);

	if (!(flags & EventReturnFlags::NoOriginal))
		g_hk_client.HLCreateMove(sequence_number, input_sample_frametime, active);

	if constexpr (Base::Win64)
		AsmTools::SetR14(reinterpret_cast<void*>(bSendPacket));
	else if (Interfaces::engine->GetAppID() != static_cast<int>(EAppID::CSGO))
		*(*reinterpret_cast<bool**>(baseptr) + off) = bSendPacket;
}

void __stdcall CClientHook::Hooked_FrameStageNotify(UNCRAP ClientFrameStage_t curStage)
{
	int flags = OnFrameStageNotify.DispatchEvent(curStage);
	if (flags & EventReturnFlags::NoOriginal)
		return;

	g_hk_client.FrameStageNotify(curStage);
}

bool __stdcall CClientHook::Hooked_CreateMove(UNCRAP float flInputSampleTime, CUserCmd* cmd)
{
	CUserCmd* eventcmd = cmd;
	if (Interfaces::engine->GetAppID() == static_cast<int>(EAppID::GMOD))
	{
		eventcmd = reinterpret_cast<CUserCmd*>(reinterpret_cast<void**>(cmd) - 1); // Offset missing VMT
	}

	bool result = true;
	int flags = OnCreateMove.DispatchEvent(result, flInputSampleTime, eventcmd);

	if (flags & EventReturnFlags::NoOriginal)
		return result;

    const bool original_return_value = g_hk_client.CreateMove(flInputSampleTime, cmd);
	if (result)
		return result;

	return original_return_value;
}

void __stdcall CClientHook::Hooked_OverrideView(UNCRAP CViewSetup* pSetup)
{
	int flags = OnOverrideView.DispatchEvent(pSetup);
	if (flags & EventReturnFlags::NoOriginal)
		return;
	g_hk_client.OverrideView(pSetup);
}
