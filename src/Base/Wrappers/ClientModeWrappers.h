#pragma once
#include "Wrappers.h"

class IClientMode;
class IClientModeCSGO;

class IClientModeWrapperSDK : public IClientModeWrapper
{
	IClientMode* m_int;

public:
	IClientModeWrapperSDK(void* ClientMode) : m_int((IClientMode*)ClientMode) { }

	void* Inst() override { return m_int; }
	int GetOffset(EOffsets Offset) override;

	bool	ShouldDrawDetailObjects() override;
	bool	ShouldDrawEntity(C_BaseEntity* pEnt) override;
	bool	ShouldDrawLocalPlayer(C_BasePlayer* pPlayer) override;
	bool	ShouldDrawParticles() override;
	bool	CreateMove(float flInputSampleTime, CUserCmd* cmd) override;
	bool	ShouldDrawViewModel(void) override;
	bool	ShouldDrawCrosshair(void) override;
	void	PreRender(CViewSetup* pSetup) override;
	void	PostRender(void) override;
	void	PostRenderVGui() override;
	void	OverrideView(CViewSetup* pSetup) override;
	vgui::Panel* GetViewport() override;
};

class IClientModeWrapperCSGO : public IClientModeWrapper
{
	IClientModeCSGO* m_int;

public:
	IClientModeWrapperCSGO(void* ClientMode) : m_int((IClientModeCSGO*)ClientMode) { }

	void* Inst() override { return m_int; }
	int GetOffset(EOffsets Offset) override;

	bool	ShouldDrawDetailObjects() override;
	bool	ShouldDrawEntity(C_BaseEntity* pEnt) override;
	bool	ShouldDrawLocalPlayer(C_BasePlayer* pPlayer) override;
	bool	ShouldDrawParticles() override;
	bool	CreateMove(float flInputSampleTime, CUserCmd* cmd) override;
	bool	ShouldDrawViewModel(void) override;
	bool	ShouldDrawCrosshair(void) override;
	void	PreRender(CViewSetup* pSetup) override;
	void	PostRender(void) override;
	void	PostRenderVGui() override;
	void	OverrideView(CViewSetup* pSetup) override;
	vgui::Panel* GetViewport() override;
};