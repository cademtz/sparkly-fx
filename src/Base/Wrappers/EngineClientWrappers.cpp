#include "EngineClientWrappers.h"
#include <SDK/InterfaceVersions/EngineClient013.h>
#include <SDK/InterfaceVersions/EngineClient014.h>
#include <cstdarg>

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
float IEngineClientWrapper013::GetLastTimeStamp() {
	return m_int->GetLastTimeStamp();
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
bool IEngineClientWrapper013::IsInGame() {
	return m_int->IsInGame();
}
bool IEngineClientWrapper013::IsConnected() {
	return m_int->IsConnected();
}
void IEngineClientWrapper013::Con_NPrintf(int pos, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	return m_int->Con_NPrintf(pos, fmt, va);
}
const VMatrix& IEngineClientWrapper013::WorldToScreenMatrix() {
	return m_int->WorldToScreenMatrix();
}
const VMatrix& IEngineClientWrapper013::WorldToViewMatrix() {
	return m_int->WorldToViewMatrix();
}
INetChannelInfo* IEngineClientWrapper013::GetNetChannelInfo() {
	return m_int->GetNetChannelInfo();
}
void IEngineClientWrapper013::DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color) {
	return m_int->DebugDrawPhysCollide(pCollide, pMaterial, transform, color);
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
int IEngineClientWrapper013::GetDemoRecordingTick(void) {
	return m_int->GetDemoRecordingTick();
}
int IEngineClientWrapper013::GetDemoPlaybackTick(void) {
	return m_int->GetDemoPlaybackTick();
}
int IEngineClientWrapper013::GetDemoPlaybackStartTick(void) {
	return m_int->GetDemoPlaybackStartTick();
}
float IEngineClientWrapper013::GetDemoPlaybackTimeScale(void) {
	return m_int->GetDemoPlaybackTimeScale();
}
int IEngineClientWrapper013::GetDemoPlaybackTotalTicks(void) {
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
const char* IEngineClientWrapper013::GetProductVersionString() {
	return m_int->GetProductVersionString();
}
void IEngineClientWrapper013::ExecuteClientCmd(const char* szCmdString) {
	return m_int->ExecuteClientCmd(szCmdString);
}
int IEngineClientWrapper013::GetAppID() {
	return m_int->GetAppID();
}
void IEngineClientWrapper013::ClientCmd_Unrestricted(const char* szCmdString) {
	return m_int->ClientCmd_Unrestricted(szCmdString);
}
IAchievementMgr* IEngineClientWrapper013::GetAchievementMgr() {
	return m_int->GetAchievementMgr();
}
CGamestatsData* IEngineClientWrapper013::GetGamestatsData() {
	return m_int->GetGamestatsData();
}

void IEngineClientWrapper014::GetScreenSize(int& width, int& height) {
	return m_int->GetScreenSize(width, height);
}
void IEngineClientWrapper014::ServerCmd(const char* szCmdString, bool bReliable) {
	return m_int->ServerCmd(szCmdString, bReliable);
}
void IEngineClientWrapper014::ClientCmd(const char* szCmdString) {
	return m_int->ClientCmd(szCmdString);
}
bool IEngineClientWrapper014::GetPlayerInfo(int ent_num, player_info_t* pinfo) {
	return m_int->GetPlayerInfo(ent_num, pinfo);
}
int IEngineClientWrapper014::GetPlayerForUserID(int userID) {
	return m_int->GetPlayerForUserID(userID);
}
client_textmessage_t* IEngineClientWrapper014::TextMessageGet(const char* pName) {
	return m_int->TextMessageGet(pName);
}
bool IEngineClientWrapper014::Con_IsVisible() {
	return m_int->Con_IsVisible();
}
int IEngineClientWrapper014::GetLocalPlayer() {
	return m_int->GetLocalPlayer();
}
const model_t* IEngineClientWrapper014::LoadModel(const char* pName, bool bProp) {
	return m_int->LoadModel(pName, bProp);
}
float IEngineClientWrapper014::GetLastTimeStamp() {
	return m_int->GetLastTimeStamp();
}
void IEngineClientWrapper014::GetViewAngles(QAngle& va) {
	return m_int->GetViewAngles(va);
}
void IEngineClientWrapper014::SetViewAngles(QAngle& va) {
	return m_int->SetViewAngles(va);
}
int IEngineClientWrapper014::GetMaxClients() {
	return m_int->GetMaxClients();
}
bool IEngineClientWrapper014::IsInGame() {
	return m_int->IsInGame();
}
bool IEngineClientWrapper014::IsConnected() {
	return m_int->IsConnected();
}
void IEngineClientWrapper014::Con_NPrintf(int pos, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	return m_int->Con_NPrintf(pos, fmt, va);
}
const VMatrix& IEngineClientWrapper014::WorldToScreenMatrix() {
	return m_int->WorldToScreenMatrix();
}
const VMatrix& IEngineClientWrapper014::WorldToViewMatrix() {
	return m_int->WorldToViewMatrix();
}
INetChannelInfo* IEngineClientWrapper014::GetNetChannelInfo() {
	return m_int->GetNetChannelInfo();
}
void IEngineClientWrapper014::DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color) {
	return m_int->DebugDrawPhysCollide(pCollide, pMaterial, transform, color);
}
bool IEngineClientWrapper014::IsPlayingDemo() {
	return m_int->IsPlayingDemo();
}
bool IEngineClientWrapper014::IsRecordingDemo() {
	return m_int->IsRecordingDemo();
}
bool IEngineClientWrapper014::IsPlayingTimeDemo() {
	return m_int->IsPlayingTimeDemo();
}
int IEngineClientWrapper014::GetDemoRecordingTick(void) {
	return m_int->GetDemoRecordingTick();
}
int IEngineClientWrapper014::GetDemoPlaybackTick(void) {
	return m_int->GetDemoPlaybackTick();
}
int IEngineClientWrapper014::GetDemoPlaybackStartTick(void) {
	return m_int->GetDemoPlaybackStartTick();
}
float IEngineClientWrapper014::GetDemoPlaybackTimeScale(void) {
	return m_int->GetDemoPlaybackTimeScale();
}
int IEngineClientWrapper014::GetDemoPlaybackTotalTicks(void) {
	return m_int->GetDemoPlaybackTotalTicks();
}
bool IEngineClientWrapper014::IsPaused() {
	return m_int->IsPaused();
}
bool IEngineClientWrapper014::IsTakingScreenshot() {
	return m_int->IsTakingScreenshot();
}
bool IEngineClientWrapper014::IsHLTV() {
	return m_int->IsHLTV();
}
const char* IEngineClientWrapper014::GetProductVersionString() {
	return m_int->GetProductVersionString();
}
void IEngineClientWrapper014::ExecuteClientCmd(const char* szCmdString) {
	return m_int->ExecuteClientCmd(szCmdString);
}
int IEngineClientWrapper014::GetAppID() {
	return m_int->GetAppID();
}
void IEngineClientWrapper014::ClientCmd_Unrestricted(const char* szCmdString) {
	return m_int->ClientCmd_Unrestricted(szCmdString);
}
IAchievementMgr* IEngineClientWrapper014::GetAchievementMgr() {
	return m_int->GetAchievementMgr();
}
CGamestatsData* IEngineClientWrapper014::GetGamestatsData() {
	return m_int->GetGamestatsData();
}