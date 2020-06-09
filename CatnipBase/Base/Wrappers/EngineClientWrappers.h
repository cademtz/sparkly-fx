#pragma once
#include "Base\Interfaces.h"

class IVEngineClient013;

class IEngineClientWrapper013
{
	IVEngineClient013* m_int;

public:
	IEngineClientWrapper013(void* EngineClient) : m_int((IVEngineClient013*)EngineClient) { }

	virtual int					GetIntersectingSurfaces(
		const model_t* model,
		const Vector& vCenter,
		const float radius,
		const bool bOnlyVisibleSurfaces,
		SurfInfo* pInfos,
		const int nMaxInfos);

	virtual Vector				GetLightForPoint(const Vector& pos, bool bClamp);

	virtual IMaterial* TraceLineMaterialAndLighting(const Vector& start, const Vector& end,
		Vector& diffuseLightColor, Vector& baseColor);

	virtual const char* ParseFile(const char* data, char* token, int maxlen);
	virtual bool				CopyLocalFile(const char* source, const char* destination);

	// Gets the dimensions of the game window
	virtual void				GetScreenSize(int& width, int& height);

	virtual void				ServerCmd(const char* szCmdString, bool bReliable = true);
	virtual void				ClientCmd(const char* szCmdString);

	virtual bool				GetPlayerInfo(int ent_num, player_info_t* pinfo);

	virtual int					GetPlayerForUserID(int userID);

	virtual client_textmessage_t* TextMessageGet(const char* pName);

	virtual bool				Con_IsVisible(void);

	virtual int					GetLocalPlayer(void);

	virtual const model_t* LoadModel(const char* pName, bool bProp = false);

	virtual float				Time(void);

	virtual float				GetLastTimeStamp(void);

	virtual CSentence* GetSentence(CAudioSource* pAudioSource);
	virtual float				GetSentenceLength(CAudioSource* pAudioSource);
	virtual bool				IsStreaming(CAudioSource* pAudioSource) const;

	virtual void				GetViewAngles(QAngle& va);
	virtual void				SetViewAngles(QAngle& va);

	virtual int					GetMaxClients(void);

	virtual	const char* Key_LookupBinding(const char* pBinding);

	virtual const char* Key_BindingForKey(ButtonCode_t code);

	virtual void				StartKeyTrapMode(void);
	virtual bool				CheckDoneKeyTrapping(ButtonCode_t& code);

	virtual bool				IsInGame(void);
	virtual bool				IsConnected(void);
	virtual bool				IsDrawingLoadingImage(void);

	virtual void				Con_NPrintf(int pos, const char* fmt, ...);
	virtual void				Con_NXPrintf(const struct con_nprint_s* info, const char* fmt, ...);

	virtual int					IsBoxVisible(const Vector& mins, const Vector& maxs);

	virtual int					IsBoxInViewCluster(const Vector& mins, const Vector& maxs);

	virtual bool				CullBox(const Vector& mins, const Vector& maxs);

	virtual void				Sound_ExtraUpdate(void);

	virtual const char* GetGameDirectory(void);

	virtual const VMatrix& WorldToScreenMatrix();

	virtual const VMatrix& WorldToViewMatrix();

	virtual int					GameLumpVersion(int lumpId) const;
	virtual int					GameLumpSize(int lumpId) const;
	virtual bool				LoadGameLump(int lumpId, void* pBuffer, int size);

	virtual int					LevelLeafCount() const;

	virtual ISpatialQuery* GetBSPTreeQuery();

	virtual void		LinearToGamma(float* linear, float* gamma);

	virtual float		LightStyleValue(int style);

	virtual void		ComputeDynamicLighting(const Vector& pt, const Vector* pNormal, Vector& color);

	virtual void		GetAmbientLightColor(Vector& color);

	virtual int			GetDXSupportLevel();

	virtual bool        SupportsHDR();

	virtual void		Mat_Stub(IMaterialSystem* pMatSys);

	virtual void GetChapterName(char* pchBuff, int iMaxLength);
	virtual char const* GetLevelName(void);
	virtual int	GetLevelVersion(void);
#if !defined( NO_VOICE )
	virtual struct IVoiceTweak_s* GetVoiceTweakAPI(void);
#endif
	virtual void		EngineStats_BeginFrame(void);
	virtual void		EngineStats_EndFrame(void);

	virtual void		FireEvents();

	virtual int			GetLeavesArea(int* pLeaves, int nLeaves);

	virtual bool		DoesBoxTouchAreaFrustum(const Vector& mins, const Vector& maxs, int iArea);

	virtual void		SetAudioState(const AudioState_t& state);

	virtual int			SentenceGroupPick(int groupIndex, char* name, int nameBufLen);
	virtual int			SentenceGroupPickSequential(int groupIndex, char* name, int nameBufLen, int sentenceIndex, int reset);
	virtual int			SentenceIndexFromName(const char* pSentenceName);
	virtual const char* SentenceNameFromIndex(int sentenceIndex);
	virtual int			SentenceGroupIndexFromName(const char* pGroupName);
	virtual const char* SentenceGroupNameFromIndex(int groupIndex);
	virtual float		SentenceLength(int sentenceIndex);

	virtual void		ComputeLighting(const Vector& pt, const Vector* pNormal, bool bClamp, Vector& color, Vector* pBoxColors = nullptr);

	virtual void		ActivateOccluder(int nOccluderIndex, bool bActive);
	virtual bool		IsOccluded(const Vector& vecAbsMins, const Vector& vecAbsMaxs);

	virtual void* SaveAllocMemory(size_t num, size_t size);
	virtual void		SaveFreeMemory(void* pSaveMem);

	virtual INetChannelInfo* GetNetChannelInfo(void);

	virtual void		DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color);
	virtual void		CheckPoint(const char* pName);
	virtual void		DrawPortals();
	virtual bool		IsPlayingDemo(void);
	virtual bool		IsRecordingDemo(void);
	virtual bool		IsPlayingTimeDemo(void);
	virtual int			GetDemoRecordingTick(void);
	virtual int			GetDemoPlaybackTick(void);
	virtual int			GetDemoPlaybackStartTick(void);
	virtual float		GetDemoPlaybackTimeScale(void);
	virtual int			GetDemoPlaybackTotalTicks(void);
	virtual bool		IsPaused(void);
	virtual bool		IsTakingScreenshot(void);
	virtual bool		IsHLTV(void);
	virtual bool		IsLevelMainMenuBackground(void);
	virtual void		GetMainMenuBackgroundName(char* dest, int destlen);

	virtual void		GetVideoModes(int& nCount, void*& pModes);

	virtual void		SetOcclusionParameters(const OcclusionParams_t& params);

	virtual void		GetUILanguage(char* dest, int destlen);

	virtual SkyboxVisibility_t IsSkyboxVisibleFromPoint(const Vector& vecPoint);

	virtual const char* GetMapEntitiesString();

	virtual bool		IsInEditMode(void);

	virtual float		GetScreenAspectRatio();

	virtual bool		REMOVED_SteamRefreshLogin(const char* password, bool isSecure);
	virtual bool		REMOVED_SteamProcessCall(bool& finished);

	virtual unsigned int	GetEngineBuildNumber(); // engines build
	virtual const char* GetProductVersionString(); // mods version number (steam.inf)

	virtual void			GrabPreColorCorrectedFrame(int x, int y, int width, int height);

	virtual bool			IsHammerRunning() const;

	virtual void			ExecuteClientCmd(const char* szCmdString);

	virtual bool MapHasHDRLighting(void);

	virtual int	GetAppID();

	virtual Vector			GetLightForPointFast(const Vector& pos, bool bClamp);

	virtual void			ClientCmd_Unrestricted(const char* szCmdString);

	virtual void			SetRestrictServerCommands(bool bRestrict);

	virtual void			SetRestrictClientCommands(bool bRestrict);

	virtual void			SetOverlayBindProxy(int iOverlayID, void* pBindProxy);

	virtual bool			CopyFrameBufferToMaterial(const char* pMaterialName);

	virtual void			ChangeTeam(const char* pTeamName);

	virtual void			ReadConfiguration(const bool readDefault = false);

	virtual void SetAchievementMgr(IAchievementMgr* pAchievementMgr);
	virtual IAchievementMgr* GetAchievementMgr();

	virtual bool			MapLoadFailed(void);
	virtual void			SetMapLoadFailed(bool bState);

	virtual bool			IsLowViolence();
	virtual const char* GetMostRecentSaveGame(void);
	virtual void			SetMostRecentSaveGame(const char* lpszFilename);

	virtual void			StartXboxExitingProcess();
	virtual bool			IsSaveInProgress();
	virtual uint			OnStorageDeviceAttached(void);
	virtual void			OnStorageDeviceDetached(void);

	virtual void			ResetDemoInterpolation(void);

	virtual void SetGamestatsData(CGamestatsData* pGamestatsData);
	virtual CGamestatsData* GetGamestatsData();

#if defined( USE_SDL )
	virtual void GetMouseDelta(int& x, int& y, bool bIgnoreNextMouseDelta = false);
#endif

	virtual void ServerCmdKeyValues(KeyValues* pKeyValues);
	virtual bool IsSkippingPlayback(void);
	virtual bool IsLoadingDemo(void);
	virtual bool IsPlayingDemoALocallyRecordedDemo();
	virtual	const char* Key_LookupBindingExact(const char* pBinding);
	virtual void				AddPhonemeFile(const char* pszPhonemeFile);
};