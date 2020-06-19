#pragma once
#include "Wrappers.h"

class IClientMode;
class IClientModeCSGO;

class IClientModeWrapperSDK : public IClientModeWrapper
{
	IClientMode* m_int;

public:
	IClientModeWrapperSDK(void* ClientMode) : m_int((IClientMode*)ClientMode) { }

	virtual void* Inst() { return m_int; }
	virtual int GetOffset(EOffsets Offset);

	virtual bool	ShouldDrawDetailObjects();
	virtual bool	ShouldDrawEntity(C_BaseEntity* pEnt);
	virtual bool	ShouldDrawLocalPlayer(C_BasePlayer* pPlayer);
	virtual bool	ShouldDrawParticles();
	virtual bool	CreateMove(float flInputSampleTime, CUserCmd* cmd);
	virtual bool	ShouldDrawViewModel(void);
	virtual bool	ShouldDrawCrosshair(void);
	virtual void	PreRender(CViewSetup* pSetup);
	virtual void	PostRender(void);
	virtual void	PostRenderVGui();
};

class IClientModeWrapperCSGO : public IClientModeWrapper
{
	IClientModeCSGO* m_int;

public:
	IClientModeWrapperCSGO(void* ClientMode) : m_int((IClientModeCSGO*)ClientMode) { }

	virtual void* Inst() { return m_int; }
	virtual int GetOffset(EOffsets Offset);

	virtual bool	ShouldDrawDetailObjects();
	virtual bool	ShouldDrawEntity(C_BaseEntity* pEnt);
	virtual bool	ShouldDrawLocalPlayer(C_BasePlayer* pPlayer);
	virtual bool	ShouldDrawParticles();
	virtual bool	CreateMove(float flInputSampleTime, CUserCmd* cmd);
	virtual bool	ShouldDrawViewModel(void);
	virtual bool	ShouldDrawCrosshair(void);
	virtual void	PreRender(CViewSetup* pSetup);
	virtual void	PostRender(void);
	virtual void	PostRenderVGui();
};