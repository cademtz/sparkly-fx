#pragma once
#include <SDK/mathlib.h>
#include <SDK/basehandle.h>
#include <SDK/ienginevgui.h>

enum EOffsets {
	Off_CreateMove,
	Off_FrameStageNotify,
	Off_ShouldDrawDetailObjects,
	Off_ShouldDrawEntity,
	Off_ShouldDrawLocalPlayer,
	Off_ShouldDrawParticles,
	Off_ShouldDrawViewModel,
	Off_ShouldDrawCrosshair,
	Off_PostRender,
	Off_PostRenderVGui,
	Off_Paint,
	Off_View_Render,
	Off_OverrideView,
	Off_DrawModelExecute,
	Off_DrawModelExStaticProp,
	Off_DrawStaticPropArrayFast,
};

struct model_t;
class Vector;
class IMaterial;
struct client_textmessage_t;
typedef struct player_info_s player_info_t;
class QAngle;
enum ButtonCode_t;
class VMatrix;
class IMaterialSystem;
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
class C_BaseEntity;
class C_BasePlayer;
class CUserCmd;
class CBaseEntity;
struct Ray_t;
typedef class CGameTrace trace_t;
class ITraceFilter;
class IMatRenderContext;
enum OverrideType_t;
struct DrawModelState_t;
struct ModelRenderInfo_t;

class IEngineClientWrapper
{
public:
	virtual ~IEngineClientWrapper() { }
	virtual void* Inst() = 0;

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
	virtual ~IClientDLLWrapper() { }
	virtual void* Inst() = 0;
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

class IClientModeWrapper
{
public:
	virtual ~IClientModeWrapper() { }
	virtual void* Inst() = 0;
	virtual int GetOffset(EOffsets Offset) = 0;

	virtual bool	ShouldDrawDetailObjects() = 0;
	virtual bool	ShouldDrawEntity(C_BaseEntity* pEnt) = 0;
	virtual bool	ShouldDrawLocalPlayer(C_BasePlayer* pPlayer) = 0;
	virtual bool	ShouldDrawParticles() = 0;
	virtual bool	CreateMove(float flInputSampleTime, CUserCmd* cmd) = 0;
	virtual bool	ShouldDrawViewModel(void) = 0;
	virtual bool	ShouldDrawCrosshair(void) = 0;
	virtual void	PreRender(CViewSetup* pSetup) = 0;
	virtual void	PostRender(void) = 0;
	virtual void	PostRenderVGui() = 0;
	virtual void	OverrideView(CViewSetup* pSetup) = 0;
};

class IClientEntityListWrapper
{
public:
	virtual ~IClientEntityListWrapper() { }
	virtual void* Inst() = 0;

	virtual CBaseEntity* GetClientEntity(int entnum) = 0;
	virtual CBaseEntity* GetClientEntityFromHandle(CBaseHandle hEnt) = 0;
	virtual int					NumberOfEntities(bool bIncludeNonNetworkable) = 0;
	virtual int					GetHighestEntityIndex(void) = 0;
	virtual int					GetMaxEntities() = 0;
};

class CEngineVGUIWrapper
{
public:
	virtual ~CEngineVGUIWrapper() { }
	virtual void* Inst() = 0;
	virtual int GetOffset(EOffsets Offset) = 0;

	virtual bool IsGameUIVisible() = 0;
	virtual void Paint(PaintMode_t mode) = 0;
};

class IEngineTraceWrapper
{
public:
	virtual ~IEngineTraceWrapper() { }
	virtual void* Inst() = 0;

	virtual int		GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity = NULL) = 0;
	virtual void	ClipRayToEntity(const Ray_t& ray, unsigned int fMask, IHandleEntity* pEnt, trace_t* pTrace) = 0;
	virtual void	TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual bool PointOutsideWorld(const Vector& ptTest) = 0;
};

class IVModelRenderWrapper
{
public:
	virtual ~IVModelRenderWrapper() { }
	virtual void* Inst() = 0;
	virtual int GetOffset(EOffsets Offset) = 0;

	virtual void ForcedMaterialOverride(IMaterial* newMaterial, OverrideType_t nOverrideType = (OverrideType_t)0) = 0;
	virtual void DrawModelExecute(const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld = NULL) = 0;
};

class IMaterialSystemWrapper
{
public:
	virtual ~IMaterialSystemWrapper() { }
	virtual void* Inst() = 0;

	virtual IMatRenderContext* GetRenderContext() = 0;
	virtual IMaterial* CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) = 0;
};
