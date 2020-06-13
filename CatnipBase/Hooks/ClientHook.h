#pragma once
#include "Hooks.h"
#include "Base/Interfaces.h"

DECL_EVENT(EVENT_HLCREATEMOVE);
DECL_EVENT(EVENT_FRAMESTAGENOTIFY);

typedef void(__thiscall* HLCreateMoveFn_t)(void*, int, float, bool);
typedef void(__thiscall* FrameStageNotifyFn_t)(void*, ClientFrameStage_t);

struct CClientHook_Ctx
{
	int sequence_number;
	float input_sample_frametime;
	bool active;
	ClientFrameStage_t curStage;
};

class CClientHook : public CBaseHook
{
public:
	CClientHook();

	void Hook() override;
	void Unhook() override;
	void HLCreateMove(int sequence_number, float input_sample_frametime, bool active);
	void FrameStageNotify(ClientFrameStage_t curStage);
	CClientHook_Ctx& Context() { return m_ctx; }

private:
	CVMTHook m_hlhook;
	CClientHook_Ctx m_ctx;

	static void __stdcall Hooked_HLCreateMove(int sequence_number, float input_sample_frametime, bool active);
	static void __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
};

inline CClientHook _g_clienthook;