#pragma once
#include "Base/Interfaces.h"

class ExportedEngine
{
	IEngineClientWrapper* m_engine;
public:
	ExportedEngine(IEngineClientWrapper* Engine);

	void				GetScreenSize(int& width, int& height);
	void				ServerCmd(const char* szCmdString, bool bReliable = true);
	void				ClientCmd(const char* szCmdString);
	bool				GetPlayerInfo(int ent_num, player_info_t* pinfo);
	int					GetPlayerForUserID(int userID);
	client_textmessage_t* TextMessageGet(const char* pName);
	bool				Con_IsVisible(void);
	int					GetLocalPlayer(void);
	const model_t* LoadModel(const char* pName, bool bProp = false);
	float				GetLastTimeStamp(void);
	void				GetViewAngles(QAngle& va);
	void				SetViewAngles(QAngle& va);
	int					GetMaxClients(void);
	bool				IsInGame(void);
	bool				IsConnected(void);
	void				Con_NPrintf(int pos, const char* fmt, ...);
	const VMatrix& WorldToScreenMatrix();
	const VMatrix& WorldToViewMatrix();
	INetChannelInfo* GetNetChannelInfo(void);
	void		DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color);
	bool		IsPlayingDemo(void);
	bool		IsRecordingDemo(void);
	bool		IsPlayingTimeDemo(void);
	bool		IsPaused(void);
	bool		IsTakingScreenshot(void);
	bool		IsHLTV(void);
	const char* GetProductVersionString();
	void			ExecuteClientCmd(const char* szCmdString);
	int	GetAppID();
	void			ClientCmd_Unrestricted(const char* szCmdString);
	IAchievementMgr* GetAchievementMgr();
	CGamestatsData* GetGamestatsData();
};
