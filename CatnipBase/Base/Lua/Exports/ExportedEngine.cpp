#include "ExportedEngine.h"
#include <cstdarg>
#include <stdio.h>

ExportedEngine::ExportedEngine(IEngineClientWrapper* Engine) :
	m_engine{Engine}
{
}

void ExportedEngine::GetScreenSize(int& width, int& height) {
	return m_engine->GetScreenSize(width, height);
}
void ExportedEngine::ServerCmd(const char* szCmdString, bool bReliable) {
	return m_engine->ServerCmd(szCmdString, bReliable);
}
void ExportedEngine::ClientCmd(const char* szCmdString) {
	return m_engine->ClientCmd(szCmdString);
}
bool ExportedEngine::GetPlayerInfo(int ent_num, player_info_t* pinfo) {
	return m_engine->GetPlayerInfo(ent_num, pinfo);
}
int ExportedEngine::GetPlayerForUserID(int userID) {
	return m_engine->GetPlayerForUserID(userID);
}
client_textmessage_t* ExportedEngine::TextMessageGet(const char* pName) {
	return m_engine->TextMessageGet(pName);
}
bool ExportedEngine::Con_IsVisible() {
	return m_engine->Con_IsVisible();
}
int ExportedEngine::GetLocalPlayer() {
	return m_engine->GetLocalPlayer();
}
const model_t* ExportedEngine::LoadModel(const char* pName, bool bProp) {
	return m_engine->LoadModel(pName, bProp);
}
float ExportedEngine::GetLastTimeStamp() {
	return m_engine->GetLastTimeStamp();
}
void ExportedEngine::GetViewAngles(QAngle& va) {
	return m_engine->GetViewAngles(va);
}
void ExportedEngine::SetViewAngles(QAngle& va) {
	return m_engine->SetViewAngles(va);
}
int ExportedEngine::GetMaxClients() {
	return m_engine->GetMaxClients();
}
bool ExportedEngine::IsInGame() {
	return m_engine->IsInGame();
}
bool ExportedEngine::IsConnected() {
	return m_engine->IsConnected();
}
void ExportedEngine::Con_NPrintf(int pos, const char* fmt, ...) {
	char buf[512];

	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, fmt, va);
	va_end(va);

	return m_engine->Con_NPrintf(pos, buf);
}
const VMatrix& ExportedEngine::WorldToScreenMatrix() {
	return m_engine->WorldToScreenMatrix();
}
const VMatrix& ExportedEngine::WorldToViewMatrix() {
	return m_engine->WorldToViewMatrix();
}
INetChannelInfo* ExportedEngine::GetNetChannelInfo() {
	return m_engine->GetNetChannelInfo();
}
void ExportedEngine::DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, matrix3x4_t& transform, const color32& color) {
	return m_engine->DebugDrawPhysCollide(pCollide, pMaterial, transform, color);
}
bool ExportedEngine::IsPlayingDemo() {
	return m_engine->IsPlayingDemo();
}
bool ExportedEngine::IsRecordingDemo() {
	return m_engine->IsRecordingDemo();
}
bool ExportedEngine::IsPlayingTimeDemo() {
	return m_engine->IsPlayingTimeDemo();
}
bool ExportedEngine::IsPaused() {
	return m_engine->IsPaused();
}
bool ExportedEngine::IsTakingScreenshot() {
	return m_engine->IsTakingScreenshot();
}
bool ExportedEngine::IsHLTV() {
	return m_engine->IsHLTV();
}
const char* ExportedEngine::GetProductVersionString() {
	return m_engine->GetProductVersionString();
}
void ExportedEngine::ExecuteClientCmd(const char* szCmdString) {
	return m_engine->ExecuteClientCmd(szCmdString);
}
int ExportedEngine::GetAppID() {
	return m_engine->GetAppID();
}
void ExportedEngine::ClientCmd_Unrestricted(const char* szCmdString) {
	return m_engine->ClientCmd_Unrestricted(szCmdString);
}
IAchievementMgr* ExportedEngine::GetAchievementMgr() {
	return m_engine->GetAchievementMgr();
}
CGamestatsData* ExportedEngine::GetGamestatsData() {
	return m_engine->GetGamestatsData();
}
