#pragma once
#include "Hooks.h"
#include <Base/Interfaces.h>

DECL_EVENT(EVENT_CREATEMOVE);
DECL_EVENT(EVENT_HLCREATEMOVE);
DECL_EVENT(EVENT_FRAMESTAGENOTIFY);
DECL_EVENT(EVENT_OVERRIDEVIEW);

typedef bool(__thiscall* CreateMoveFn_t)(void*, float, CUserCmd*);
typedef void(__thiscall* HLCreateMoveFn_t)(void*, int, float, bool);
typedef void(__thiscall* FrameStageNotifyFn_t)(void*, ClientFrameStage_t);
typedef void(__thiscall* OverrideViewFn_t)(void*, CViewSetup*);

struct CClientHook_Ctx
{
	struct
	{
		float flInputSampleTime;
		CUserCmd* cmd;
		bool result;
	} create_move;

	struct
	{
		float input_sample_frametime;
		int sequence_number;
		bool active;
		bool bSendPacket;
	} hl_create_move;

	ClientFrameStage_t curStage;
	CViewSetup* pSetup;
};

class CClientHook : public CBaseHook
{
public:
	CClientHook();

	void Hook() override;
	void Unhook() override;
	void HLCreateMove(int sequence_number, float input_sample_frametime, bool active);
	void FrameStageNotify(ClientFrameStage_t curStage);
	bool CreateMove(float flInputSampleTime, CUserCmd* cmd);
	bool OverrideView(CViewSetup* pSetup);
	CClientHook_Ctx* Context() { return &m_ctx; }

private:
	CVMTHook m_hlhook, m_clhook;
	CClientHook_Ctx m_ctx;

	static void __stdcall Hooked_HLCreateMove(UNCRAP int sequence_number, float input_sample_frametime, bool active);
	static void __stdcall Hooked_FrameStageNotify(UNCRAP ClientFrameStage_t curStage);
	static bool __stdcall Hooked_CreateMove(UNCRAP float flInputSampleTime, CUserCmd* cmd);
	static void __stdcall Hooked_OverrideView(UNCRAP CViewSetup* pSetup);
};

inline CClientHook g_hk_client;