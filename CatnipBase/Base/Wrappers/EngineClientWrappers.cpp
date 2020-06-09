#include "EngineClientWrappers.h"
#include "SDK/cdll_int.h"

int IEngineClientWrapper013::GetIntersectingSurfaces(const model_t* model, const Vector& vCenter, const float radius, const bool bOnlyVisibleSurfaces, SurfInfo* pInfos, const int nMaxInfos) {
	return m_int->GetIntersectingSurfaces(model, vCenter, radius, bOnlyVisibleSurfaces, pInfos, nMaxInfos);
}
Vector IEngineClientWrapper013::GetLightForPoint(const Vector& pos, bool bClamp) {
	return m_int->GetLightForPoint(pos, bClamp);
}
IMaterial* IEngineClientWrapper013::TraceLineMaterialAndLighting(const Vector& start, const Vector& end, Vector& diffuseLightColor, Vector& baseColor) {
	return m_int->TraceLineMaterialAndLighting(start, end, diffuseLightColor, baseColor);
}
const char* IEngineClientWrapper013::ParseFile(const char* data, char* token, int maxlen) {
	return m_int->ParseFile(data, token, maxlen);
}
bool IEngineClientWrapper013::CopyLocalFile(const char* source, const char* destination) {
	return m_int->CopyLocalFile(source, destination);
}
void IEngineClientWrapper013::GetScreenSize(int& width, int& height) {
	return m_int->GetScreenSize(width, height);
}
void IEngineClientWrapper013::ServerCmd(const char* szCmdString, bool bReliable) {
	return m_int->ServerCmd(szCmdString, bReliable);
}
void IEngineClientWrapper013::ClientCmd(const char* szCmdString) {
	return m_int->ClientCmd(szCmdString);
}
bool IEngineClientWrapper013::GetPlayerInfo(int ent_num, player_info_t* pinfo) {
	return m_int->GetPlayerInfo(ent_num, pinfo);
}
int IEngineClientWrapper013::GetPlayerForUserID(int userID) {
	return m_int->GetPlayerForUserID(userID);
}
client_textmessage_t* IEngineClientWrapper013::TextMessageGet(const char* pName) {
	return m_int->TextMessageGet(pName);
}
bool IEngineClientWrapper013::Con_IsVisible() {
	return m_int->Con_IsVisible();
}
int IEngineClientWrapper013::GetLocalPlayer() {
	return m_int->GetLocalPlayer();
}
const model_t* IEngineClientWrapper013::LoadModel(const char* pName, bool bProp) {
	return m_int->LoadModel(pName, bProp);
}
float IEngineClientWrapper013::Time() {
	return m_int->Time();
}
float IEngineClientWrapper013::GetLastTimeStamp() {
	return m_int->GetLastTimeStamp();
}
CSentence* IEngineClientWrapper013::GetSentence(CAudioSource* pAudioSource) {
	return m_int->GetSentence(pAudioSource);
}
float IEngineClientWrapper013::GetSentenceLength(CAudioSource* pAudioSource) {
	return m_int->GetSentenceLength(pAudioSource);
}
bool IEngineClientWrapper013::IsStreaming(CAudioSource* pAudioSource) const {
	return m_int->IsStreaming(pAudioSource);
}
void IEngineClientWrapper013::GetViewAngles(QAngle& va) {
	return m_int->GetViewAngles(va);
}
void IEngineClientWrapper013::SetViewAngles(QAngle& va) {
	return m_int->SetViewAngles(va);
}
int IEngineClientWrapper013::GetMaxClients() {
	return m_int->GetMaxClients();
}
const char* IEngineClientWrapper013::Key_LookupBinding(const char* pBinding) {
	return m_int->Key_LookupBinding(pBinding);
}
const char* IEngineClientWrapper013::Key_BindingForKey(ButtonCode_t code) {
	return m_int->Key_BindingForKey(code);
}
void IEngineClientWrapper013::StartKeyTrapMode() {
	return m_int->StartKeyTrapMode();
}
bool IEngineClientWrapper013::CheckDoneKeyTrapping(ButtonCode_t& code) {
	return m_int->CheckDoneKeyTrapping(code);
}
bool IEngineClientWrapper013::IsInGame() {
	return m_int->IsInGame();
}
bool IEngineClientWrapper013::IsConnected() {
	return m_int->IsConnected();
}
bool IEngineClientWrapper013::IsDrawingLoadingImage() {
	return m_int->IsDrawingLoadingImage();
}
void IEngineClientWrapper013::Con_NPrintf(int pos, const char* fmt, ...) {
	//return m_int->Con_NPrintf(pos, fmt, ...);
}
void IEngineClientWrapper013::Con_NXPrintf(const con_nprint_s* info, const char* fmt, ...) {
	//return m_int->Con_NXPrintf(info, fmt, ...);
}
int IEngineClientWrapper013::IsBoxVisible(const Vector& mins, const Vector& maxs) {
	return m_int->IsBoxVisible(mins, maxs);
}
int IEngineClientWrapper013::IsBoxInViewCluster(const Vector& mins, const Vector& maxs) {
	return m_int->IsBoxInViewCluster(mins, maxs);
}
bool IEngineClientWrapper013::CullBox(const Vector& mins, const Vector& maxs) {
	return m_int->CullBox(mins, maxs);
}
void IEngineClientWrapper013::Sound_ExtraUpdate() {
	return m_int->Sound_ExtraUpdate();
}
const char* IEngineClientWrapper013::GetGameDirectory() {
	return m_int->GetGameDirectory();
}
const VMatrix& IEngineClientWrapper013::WorldToScreenMatrix() {
	return m_int->WorldToScreenMatrix();
}
const VMatrix& IEngineClientWrapper013::WorldToViewMatrix() {
	return m_int->WorldToViewMatrix();
}
int IEngineClientWrapper013::GameLumpVersion(int lumpId) const {
	return m_int->GameLumpVersion(lumpId);
}
int IEngineClientWrapper013::GameLumpSize(int lumpId) const {
	return m_int->GameLumpSize(lumpId);
}
bool IEngineClientWrapper013::LoadGameLump(int lumpId, void* pBuffer, int size) {
	return m_int->LoadGameLump(lumpId, pBuffer, size);
}
int IEngineClientWrapper013::LevelLeafCount() const {
	return m_int->LevelLeafCount();
}
ISpatialQuery* IEngineClientWrapper013::GetBSPTreeQuery() {
	return m_int->GetBSPTreeQuery();
}
void IEngineClientWrapper013::LinearToGamma(float* linear, float* gamma) {
	return m_int->LinearToGamma(linear, gamma);
}
float IEngineClientWrapper013::LightStyleValue(int style) {
	return m_int->LightStyleValue(style);
}
void IEngineClientWrapper013::ComputeDynamicLighting(const Vector& pt, const Vector* pNormal, Vector& color) {
	return m_int->ComputeDynamicLighting(pt, pNormal, color);
}
void IEngineClientWrapper013::GetAmbientLightColor(Vector& color) {
	return m_int->GetAmbientLightColor(color);
}
int IEngineClientWrapper013::GetDXSupportLevel() {
	return m_int->GetDXSupportLevel();
}
bool IEngineClientWrapper013::SupportsHDR() {
	return m_int->SupportsHDR();
}
void IEngineClientWrapper013::Mat_Stub(IMaterialSystem* pMatSys) {
	return m_int->Mat_Stub(pMatSys);
}
void IEngineClientWrapper013::GetChapterName(char* pchBuff, int iMaxLength) {
	return m_int->GetChapterName(pchBuff, iMaxLength);
}
char const* IEngineClientWrapper013::GetLevelName() {
	return m_int->GetLevelName();
}
int IEngineClientWrapper013::GetLevelVersion() {
	return m_int->GetLevelVersion();
}
IVoiceTweak_s* IEngineClientWrapper013::GetVoiceTweakAPI() {
	return m_int->GetVoiceTweakAPI();
}
void IEngineClientWrapper013::EngineStats_BeginFrame() {
	return m_int->EngineStats_BeginFrame();
}
void IEngineClientWrapper013::EngineStats_EndFrame() {
	return m_int->EngineStats_EndFrame();
}
void IEngineClientWrapper013::FireEvents() {
	return m_int->FireEvents();
}
int IEngineClientWrapper013::GetLeavesArea(int* pLeaves, int nLeaves) {
	return m_int->GetLeavesArea(pLeaves, nLeaves);
}
bool IEngineClientWrapper013::DoesBoxTouchAreaFrustum(const Vector& mins, const Vector& maxs, int iArea) {
	return m_int->DoesBoxTouchAreaFrustum(mins, maxs, iArea);
}
void IEngineClientWrapper013::SetAudioState(const AudioState_t& state) {
	return m_int->SetAudioState(state);
}
int IEngineClientWrapper013::SentenceGroupPick(int groupIndex, char* name, int nameBufLen) {
	return m_int->SentenceGroupPick(groupIndex, name, nameBufLen);
}
int IEngineClientWrapper013::SentenceGroupPickSequential(int groupIndex, char* name, int nameBufLen, int sentenceIndex, int reset) {
	return m_int->SentenceGroupPickSequential(groupIndex, name, nameBufLen, sentenceIndex, reset);
}
int IEngineClientWrapper013::SentenceIndexFromName(const char* pSentenceName) {
	return m_int->SentenceIndexFromName(pSentenceName);
}
const char* IEngineClientWrapper013::SentenceNameFromIndex(int sentenceIndex) {
	return m_int->SentenceNameFromIndex(sentenceIndex);
}
int IEngineClientWrapper013::SentenceGroupIndexFromName(const char* pGroupName) {
	return m_int->SentenceGroupIndexFromName(pGroupName);
}
const char* IEngineClientWrapper013::SentenceGroupNameFromIndex(int groupIndex) {
	return m_int->SentenceGroupNameFromIndex(groupIndex);
}
float IEngineClientWrapper013::SentenceLength(int sentenceIndex) {
	return m_int->SentenceLength(sentenceIndex);
}
void IEngineClientWrapper013::ComputeLighting(const Vector& pt, const Vector* pNormal, bool bClamp, Vector& color, Vector* pBoxColors) {
	return m_int->ComputeLighting(pt, pNormal, bClamp, color, pBoxColors);
}
void IEngineClientWrapper013::ActivateOccluder(int nOccluderIndex, bool bActive) {
	return m_int->ActivateOccluder(nOccluderIndex, bActive);
}
bool IEngineClientWrapper013::IsOccluded(const Vector& vecAbsMins, const Vector& vecAbsMaxs) {
	return m_int->IsOccluded(vecAbsMins, vecAbsMaxs);
}
void* IEngineClientWrapper013::SaveAllocMemory(size_t num, size_t size) {
	return m_int->SaveAllocMemory(num, size);
}
void IEngineClientWrapper013::SaveFreeMemory(void* pSaveMem) {
	return m_int->SaveFreeMemory(pSaveMem);
}
INetChannelInfo* IEngineClientWrapper013::GetNetChannelInfo() {
	return m_int->GetNetChannelInfo();
}
void IEngineClientWrapper013::DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color) {
	return m_int->DebugDrawPhysCollide(pCollide, pMaterial, transform, color);
}
void IEngineClientWrapper013::CheckPoint(const char* pName) {
	return m_int->CheckPoint(pName);
}
void IEngineClientWrapper013::DrawPortals() {
	return m_int->DrawPortals();
}
bool IEngineClientWrapper013::IsPlayingDemo() {
	return m_int->IsPlayingDemo();
}
bool IEngineClientWrapper013::IsRecordingDemo() {
	return m_int->IsRecordingDemo();
}
bool IEngineClientWrapper013::IsPlayingTimeDemo() {
	return m_int->IsPlayingTimeDemo();
}
int IEngineClientWrapper013::GetDemoRecordingTick() {
	return m_int->GetDemoRecordingTick();
}
int IEngineClientWrapper013::GetDemoPlaybackTick() {
	return m_int->GetDemoPlaybackTick();
}
int IEngineClientWrapper013::GetDemoPlaybackStartTick() {
	return m_int->GetDemoPlaybackStartTick();
}
float IEngineClientWrapper013::GetDemoPlaybackTimeScale() {
	return m_int->GetDemoPlaybackTimeScale();
}
int IEngineClientWrapper013::GetDemoPlaybackTotalTicks() {
	return m_int->GetDemoPlaybackTotalTicks();
}
bool IEngineClientWrapper013::IsPaused() {
	return m_int->IsPaused();
}
bool IEngineClientWrapper013::IsTakingScreenshot() {
	return m_int->IsTakingScreenshot();
}
bool IEngineClientWrapper013::IsHLTV() {
	return m_int->IsHLTV();
}
bool IEngineClientWrapper013::IsLevelMainMenuBackground() {
	return m_int->IsLevelMainMenuBackground();
}
void IEngineClientWrapper013::GetMainMenuBackgroundName(char* dest, int destlen) {
	return m_int->GetMainMenuBackgroundName(dest, destlen);
}
void IEngineClientWrapper013::GetVideoModes(int& nCount, void*& pModes) {
	return m_int->GetVideoModes(nCount, pModes);
}
void IEngineClientWrapper013::SetOcclusionParameters(const OcclusionParams_t& params) {
	return m_int->SetOcclusionParameters(params);
}
void IEngineClientWrapper013::GetUILanguage(char* dest, int destlen) {
	return m_int->GetUILanguage(dest, destlen);
}
SkyboxVisibility_t IEngineClientWrapper013::IsSkyboxVisibleFromPoint(const Vector& vecPoint) {
	return m_int->IsSkyboxVisibleFromPoint(vecPoint);
}
const char* IEngineClientWrapper013::GetMapEntitiesString() {
	return m_int->GetMapEntitiesString();
}
bool IEngineClientWrapper013::IsInEditMode() {
	return m_int->IsInEditMode();
}
float IEngineClientWrapper013::GetScreenAspectRatio() {
	return m_int->GetScreenAspectRatio();
}
bool IEngineClientWrapper013::REMOVED_SteamRefreshLogin(const char* password, bool isSecure) {
	return m_int->REMOVED_SteamRefreshLogin(password, isSecure);
}
bool IEngineClientWrapper013::REMOVED_SteamProcessCall(bool& finished) {
	return m_int->REMOVED_SteamProcessCall(finished);
}
unsigned int IEngineClientWrapper013::GetEngineBuildNumber() {
	return m_int->GetEngineBuildNumber();
}
const char* IEngineClientWrapper013::GetProductVersionString() {
	return m_int->GetProductVersionString();
}
void IEngineClientWrapper013::GrabPreColorCorrectedFrame(int x, int y, int width, int height) {
	return m_int->GrabPreColorCorrectedFrame(x, y, width, height);
}
bool IEngineClientWrapper013::IsHammerRunning() const {
	return m_int->IsHammerRunning();
}
void IEngineClientWrapper013::ExecuteClientCmd(const char* szCmdString) {
	return m_int->ExecuteClientCmd(szCmdString);
}
bool IEngineClientWrapper013::MapHasHDRLighting() {
	return m_int->MapHasHDRLighting();
}
int IEngineClientWrapper013::GetAppID() {
	return m_int->GetAppID();
}
Vector IEngineClientWrapper013::GetLightForPointFast(const Vector& pos, bool bClamp) {
	return m_int->GetLightForPointFast(pos, bClamp);
}
void IEngineClientWrapper013::ClientCmd_Unrestricted(const char* szCmdString) {
	return m_int->ClientCmd_Unrestricted(szCmdString);
}
void IEngineClientWrapper013::SetRestrictServerCommands(bool bRestrict) {
	return m_int->SetRestrictServerCommands(bRestrict);
}
void IEngineClientWrapper013::SetRestrictClientCommands(bool bRestrict) {
	return m_int->SetRestrictClientCommands(bRestrict);
}
void IEngineClientWrapper013::SetOverlayBindProxy(int iOverlayID, void* pBindProxy) {
	return m_int->SetOverlayBindProxy(iOverlayID, pBindProxy);
}
bool IEngineClientWrapper013::CopyFrameBufferToMaterial(const char* pMaterialName) {
	return m_int->CopyFrameBufferToMaterial(pMaterialName);
}
void IEngineClientWrapper013::ChangeTeam(const char* pTeamName) {
	return m_int->ChangeTeam(pTeamName);
}
void IEngineClientWrapper013::ReadConfiguration(const bool readDefault) {
	return m_int->ReadConfiguration(readDefault);
}
void IEngineClientWrapper013::SetAchievementMgr(IAchievementMgr* pAchievementMgr) {
	return m_int->SetAchievementMgr(pAchievementMgr);
}
IAchievementMgr* IEngineClientWrapper013::GetAchievementMgr() {
	return m_int->GetAchievementMgr();
}
bool IEngineClientWrapper013::MapLoadFailed() {
	return m_int->MapLoadFailed();
}
void IEngineClientWrapper013::SetMapLoadFailed(bool bState) {
	return m_int->SetMapLoadFailed(bState);
}
bool IEngineClientWrapper013::IsLowViolence() {
	return m_int->IsLowViolence();
}
const char* IEngineClientWrapper013::GetMostRecentSaveGame() {
	return m_int->GetMostRecentSaveGame();
}
void IEngineClientWrapper013::SetMostRecentSaveGame(const char* lpszFilename) {
	return m_int->SetMostRecentSaveGame(lpszFilename);
}
void IEngineClientWrapper013::StartXboxExitingProcess() {
	return m_int->StartXboxExitingProcess();
}
bool IEngineClientWrapper013::IsSaveInProgress() {
	return m_int->IsSaveInProgress();
}
uint IEngineClientWrapper013::OnStorageDeviceAttached() {
	return m_int->OnStorageDeviceAttached();
}
void IEngineClientWrapper013::OnStorageDeviceDetached() {
	return m_int->OnStorageDeviceDetached();
}
void IEngineClientWrapper013::ResetDemoInterpolation() {
	return m_int->ResetDemoInterpolation();
}
void IEngineClientWrapper013::SetGamestatsData(CGamestatsData* pGamestatsData) {
	return m_int->SetGamestatsData(pGamestatsData);
}
CGamestatsData* IEngineClientWrapper013::GetGamestatsData() {
	return m_int->GetGamestatsData();
}
#if defined(USE_SDL)
void IEngineClientWrapper013::GetMouseDelta(int& x, int& y, bool bIgnoreNextMouseDelta) {
	return m_int->GetMouseDelta(x, y, bIgnoreNextMouseDelta);
}
#endif
void IEngineClientWrapper013::ServerCmdKeyValues(KeyValues* pKeyValues) {
	return m_int->ServerCmdKeyValues(pKeyValues);
}
bool IEngineClientWrapper013::IsSkippingPlayback() {
	return m_int->IsSkippingPlayback();
}
bool IEngineClientWrapper013::IsLoadingDemo() {
	return m_int->IsLoadingDemo();
}
bool IEngineClientWrapper013::IsPlayingDemoALocallyRecordedDemo() {
	return m_int->IsPlayingDemoALocallyRecordedDemo();
}
const char* IEngineClientWrapper013::Key_LookupBindingExact(const char* pBinding) {
	return m_int->Key_LookupBindingExact(pBinding);
}
void IEngineClientWrapper013::AddPhonemeFile(const char* pszPhonemeFile) {
	return m_int->AddPhonemeFile(pszPhonemeFile);
}
