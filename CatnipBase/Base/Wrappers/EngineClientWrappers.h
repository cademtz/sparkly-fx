#pragma once
#include "Wrappers.h"

class IVEngineClient013;
class IVEngineClient014;

class IEngineClientWrapper013 : public IEngineClientWrapper
{
	IVEngineClient013* m_int;

public:
	IEngineClientWrapper013(void* EngineClient) : m_int((IVEngineClient013*)EngineClient) { }

	void* Inst() override { return m_int; }

	virtual void				GetScreenSize(int& width, int& height);
	virtual void				ServerCmd(const char* szCmdString, bool bReliable = true);
	virtual void				ClientCmd(const char* szCmdString);
	virtual bool				GetPlayerInfo(int ent_num, player_info_t* pinfo);
	virtual int					GetPlayerForUserID(int userID);
	virtual client_textmessage_t* TextMessageGet(const char* pName);
	virtual bool				Con_IsVisible(void);
	virtual int					GetLocalPlayer(void);
	virtual const model_t* LoadModel(const char* pName, bool bProp = false);
	virtual float				GetLastTimeStamp(void);
	virtual void				GetViewAngles(QAngle& va);
	virtual void				SetViewAngles(QAngle& va);
	virtual int					GetMaxClients(void);
	virtual bool				IsInGame(void);
	virtual bool				IsConnected(void);
	virtual void				Con_NPrintf(int pos, const char* fmt, ...);
	virtual const VMatrix& WorldToScreenMatrix();
	virtual const VMatrix& WorldToViewMatrix();
	virtual INetChannelInfo* GetNetChannelInfo(void);
	virtual void		DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color);
	virtual bool		IsPlayingDemo(void);
	virtual bool		IsRecordingDemo(void);
	virtual bool		IsPlayingTimeDemo(void);
	virtual bool		IsPaused(void);
	virtual bool		IsTakingScreenshot(void);
	virtual bool		IsHLTV(void);
	virtual const char* GetProductVersionString();
	virtual void			ExecuteClientCmd(const char* szCmdString);
	virtual int	GetAppID();
	virtual void			ClientCmd_Unrestricted(const char* szCmdString);
	virtual IAchievementMgr* GetAchievementMgr();
	virtual CGamestatsData* GetGamestatsData();
};

class IEngineClientWrapper014 : public IEngineClientWrapper
{
	IVEngineClient014* m_int;

public:
	IEngineClientWrapper014(void* EngineClient14) : m_int((IVEngineClient014*)m_int) { }

	void* Inst() override { return m_int; }

	virtual void				GetScreenSize(int& width, int& height);
	virtual void				ServerCmd(const char* szCmdString, bool bReliable = true);
	virtual void				ClientCmd(const char* szCmdString);
	virtual bool				GetPlayerInfo(int ent_num, player_info_t* pinfo);
	virtual int					GetPlayerForUserID(int userID);
	virtual client_textmessage_t* TextMessageGet(const char* pName);
	virtual bool				Con_IsVisible(void);
	virtual int					GetLocalPlayer(void);
	virtual const model_t* LoadModel(const char* pName, bool bProp = false);
	virtual float				GetLastTimeStamp(void);
	virtual void				GetViewAngles(QAngle& va);
	virtual void				SetViewAngles(QAngle& va);
	virtual int					GetMaxClients(void);
	virtual bool				IsInGame(void);
	virtual bool				IsConnected(void);
	virtual void				Con_NPrintf(int pos, const char* fmt, ...);
	virtual const VMatrix& WorldToScreenMatrix();
	virtual const VMatrix& WorldToViewMatrix();
	virtual INetChannelInfo* GetNetChannelInfo(void);
	virtual void		DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color);
	virtual bool		IsPlayingDemo(void);
	virtual bool		IsRecordingDemo(void);
	virtual bool		IsPlayingTimeDemo(void);
	virtual bool		IsPaused(void);
	virtual bool		IsTakingScreenshot(void);
	virtual bool		IsHLTV(void);
	virtual const char* GetProductVersionString();
	virtual void			ExecuteClientCmd(const char* szCmdString);
	virtual int	GetAppID();
	virtual void			ClientCmd_Unrestricted(const char* szCmdString);
	virtual IAchievementMgr* GetAchievementMgr();
	virtual CGamestatsData* GetGamestatsData();
};