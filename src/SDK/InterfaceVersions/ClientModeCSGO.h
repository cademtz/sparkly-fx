#pragma once
#include "SDK/iclientmode.h"

// idk what im supposed to call these or whatever.

class IClientModeCSGO
{
public:
	virtual			~IClientModeCSGO() {}
	virtual void	InitViewport() = 0;
	virtual void	Init() = 0;
	virtual void	VGui_Shutdown() = 0;
	virtual void	Shutdown() = 0;
	virtual void _unk5() = 0;
	virtual void	Enable() = 0;
	virtual void	Disable() = 0;
	virtual void	Layout() = 0;
	virtual vgui::Panel* GetViewport() = 0;
	virtual void _unk10() = 0;
	virtual vgui::AnimationController* GetViewportAnimationController() = 0;
	virtual void	ProcessInput(bool bActive) = 0;
	virtual bool	ShouldDrawDetailObjects() = 0;
	virtual bool	ShouldDrawEntity(C_BaseEntity* pEnt) = 0;
	virtual bool	ShouldDrawLocalPlayer(C_BasePlayer* pPlayer) = 0;
	virtual bool	ShouldDrawParticles() = 0;
	virtual bool	ShouldDrawFog(void) = 0;
	virtual void	OverrideView(CViewSetup* pSetup) = 0;
	virtual void _unk19() = 0;
	virtual int		KeyInput(int down, ButtonCode_t keynum, const char* pszCurrentBinding) = 0;
	virtual vgui::Panel* GetMessagePanel() = 0;
	virtual void _unk22() = 0;
	virtual void	OverrideMouseInput(float* x, float* y) = 0;
	virtual bool	CreateMove(float flInputSampleTime, CUserCmd* cmd) = 0;
	virtual void	LevelInit(const char* newmap) = 0;
	virtual void	LevelShutdown(void) = 0;
	virtual bool	ShouldDrawViewModel(void) = 0;
	virtual bool	ShouldDrawCrosshair(void) = 0;
	virtual void	AdjustEngineViewport(int& x, int& y, int& width, int& height) = 0;
	virtual void	PreRender(CViewSetup* pSetup) = 0;
	virtual void	PostRender(void) = 0;
	virtual void	PostRenderVGui() = 0;
	virtual void	ActivateInGameVGuiContext(vgui::Panel* pPanel) = 0;
	virtual void	DeactivateInGameVGuiContext() = 0;
	virtual float	GetViewModelFOV(void) = 0;
	virtual bool	CanRecordDemo(char* errorMsg, int length) const = 0;
	virtual wchar_t* GetServerName() = 0;
	virtual void SetServerName(wchar_t* name) = 0;
	virtual wchar_t* GetMapName() = 0;
	virtual void SetMapName(wchar_t* name) = 0;
};
