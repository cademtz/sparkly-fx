#pragma once
#include "Hooks.h"
#include <Base/Interfaces.h>

typedef bool(__thiscall* CreateMoveFn_t)(void*, float, CUserCmd*);
typedef void(__thiscall* HLCreateMoveFn_t)(void*, int, float, bool);
typedef void(__thiscall* FrameStageNotifyFn_t)(void*, ClientFrameStage_t);
typedef void(__thiscall* OverrideViewFn_t)(void*, CViewSetup*);

class CClientHook : public CBaseHook
{
public:
	CClientHook() : BASEHOOK(CClientHook) {}

	void Hook() override;
	void Unhook() override;
	void HLCreateMove(int sequence_number, float input_sample_frametime, bool active);
	void FrameStageNotify(ClientFrameStage_t curStage);
	bool CreateMove(float flInputSampleTime, CUserCmd* cmd);
	bool OverrideView(CViewSetup* pSetup);

	using HLCreateMoveEvent = EventSource<void(int, float, bool, bool* bSendPacket)>;
	using FrameStageNotifyEvent = EventSource<void(ClientFrameStage_t)>;
	using CreateMoveEvent = EventSource<bool(float, CUserCmd*)>;
	using OverrideViewEvent = EventSource<void(CViewSetup*)>;

	static inline HLCreateMoveEvent OnHLCreateMove;
	static inline FrameStageNotifyEvent OnFrameStageNotify;
	static inline CreateMoveEvent OnCreateMove;
	static inline OverrideViewEvent OnOverrideView;
private:
	CVMTHook m_hlhook, m_clhook;

	static void __stdcall Hooked_HLCreateMove(UNCRAP int sequence_number, float input_sample_frametime, bool active);
	static void __stdcall Hooked_FrameStageNotify(UNCRAP ClientFrameStage_t curStage);
	static bool __stdcall Hooked_CreateMove(UNCRAP float flInputSampleTime, CUserCmd* cmd);
	static void __stdcall Hooked_OverrideView(UNCRAP CViewSetup* pSetup);
};

inline CClientHook g_hk_client;