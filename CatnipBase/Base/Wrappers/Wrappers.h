#pragma once
#include "SDK/basetypes.h"
#include "SDK/mathlib.h"

struct model_t;
class Vector;
class SurfInfo;
class IMaterial;
struct client_textmessage_t;
typedef struct player_info_s player_info_t;
class CSentence;
class CAudioSource;
class QAngle;
enum ButtonCode_t;
class VMatrix;
class ISpatialQuery;
class IMaterialSystem;
struct AudioState_t;
class INetChannelInfo;
class CPhysCollide;
struct OcclusionParams_t;
enum SkyboxVisibility_t;
class IAchievementMgr;
class CGamestatsData;
class KeyValues;

class ClientClass;
class bf_write;
class bf_read;
class CViewSetup;
enum ClientFrameStage_t;
class CStandardRecvProxies;
class CRenamedRecvTableInfo;

class IEngineClientWrapper
{
public:
	virtual ~IEngineClientWrapper() { }

	virtual void				GetScreenSize(int& width, int& height) = 0;
	virtual void				ServerCmd(const char* szCmdString, bool bReliable = true) = 0;
	virtual void				ClientCmd(const char* szCmdString) = 0;
	virtual bool				GetPlayerInfo(int ent_num, player_info_t* pinfo) = 0;
	virtual int					GetPlayerForUserID(int userID) = 0;
	virtual client_textmessage_t* TextMessageGet(const char* pName) = 0;
	virtual bool				Con_IsVisible(void) = 0;
	virtual int					GetLocalPlayer(void) = 0;
	virtual const model_t* LoadModel(const char* pName, bool bProp = false) = 0;
	virtual float				GetLastTimeStamp(void) = 0;
	virtual void				GetViewAngles(QAngle& va) = 0;
	virtual void				SetViewAngles(QAngle& va) = 0;
	virtual int					GetMaxClients(void) = 0;
	virtual bool				IsInGame(void) = 0;
	virtual bool				IsConnected(void) = 0;
	virtual void				Con_NPrintf(int pos, const char* fmt, ...) = 0;
	virtual const VMatrix& WorldToScreenMatrix() = 0;
	virtual const VMatrix& WorldToViewMatrix() = 0;
	virtual INetChannelInfo* GetNetChannelInfo(void) = 0;
	virtual void		DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color) = 0;
	virtual bool		IsPlayingDemo(void) = 0;
	virtual bool		IsRecordingDemo(void) = 0;
	virtual bool		IsPlayingTimeDemo(void) = 0;
	virtual bool		IsPaused(void) = 0;
	virtual bool		IsTakingScreenshot(void) = 0;
	virtual bool		IsHLTV(void) = 0;
	virtual const char* GetProductVersionString() = 0;
	virtual void			ExecuteClientCmd(const char* szCmdString) = 0;
	virtual int	GetAppID() = 0;
	virtual void			ClientCmd_Unrestricted(const char* szCmdString) = 0;
	virtual IAchievementMgr* GetAchievementMgr() = 0;
	virtual CGamestatsData* GetGamestatsData() = 0;
};

class IClientDLLWrapper
{
public:
	enum EOffsets {
		Off_CreateMove,
		Off_FrameStageNotify
	};

	virtual ~IClientDLLWrapper() { }

	virtual int GetOffset(EOffsets Offset) = 0;

	virtual ClientClass* GetAllClasses(void) = 0;
	virtual void CreateMove(int sequence_number, float input_sample_frametime, bool active) = 0;
	virtual bool			WriteUsercmdDeltaToBuffer(bf_write* buf, int from, int to, bool isnewcommand) = 0;
	virtual void			View_Render(vrect_t* rect) = 0;
	virtual void			RenderView(const CViewSetup& view, int nClearFlags, int whatToDraw) = 0;
	virtual void			InstallStringTableCallback(char const* tableName) = 0;
	virtual void			FrameStageNotify(ClientFrameStage_t curStage) = 0;
	virtual bool			DispatchUserMessage(int msg_type, bf_read& msg_data) = 0;
	virtual CStandardRecvProxies* GetStandardRecvProxies() = 0;
	virtual bool			GetPlayerView(CViewSetup& playerView) = 0;
};