#pragma once
#include "Wrappers.h"

class IClientMode;
class IClientModeCSGO;

class IClientModeWrapper001
{
	IClientMode* m_int;

public:
	IClientModeWrapper001(void* ClientMode) : m_int((IClientMode*)ClientMode) { }

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
	virtual wchar_t* GetServerName();
	virtual wchar_t* GetMapName();
};