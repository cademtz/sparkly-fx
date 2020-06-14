#include "ClientModeWrappers.h"
#include "SDK/InterfaceVersions/ClientModeSDK.h"

int IClientModeWrapper001::GetOffset(EOffsets Offset)
{
	switch (Offset)
	{
	case Off_ShouldDrawDetailObjects:
		return 11;
	case Off_ShouldDrawEntity:
		return 12;
	case Off_ShouldDrawLocalPlayer:
		return 13;
	case Off_ShouldDrawParticles:
		return 14;
	case Off_CreateMove:
		return 21;
	case Off_ShouldDrawViewModel:
		return 24;
	case Off_ShouldDrawCrosshair:
		return 25;
	}
	return -1;
}

bool IClientModeWrapper001::ShouldDrawDetailObjects() {
    return m_int->ShouldDrawDetailObjects();
}
bool IClientModeWrapper001::ShouldDrawEntity(C_BaseEntity* pEnt) {
    return m_int->ShouldDrawEntity(pEnt);
}
bool IClientModeWrapper001::ShouldDrawLocalPlayer(C_BasePlayer* pPlayer) {
    return m_int->ShouldDrawLocalPlayer(pPlayer);
}
bool IClientModeWrapper001::ShouldDrawParticles() {
	return m_int->ShouldDrawParticles();
}
bool IClientModeWrapper001::CreateMove(float flInputSampleTime, CUserCmd* cmd) {
	return m_int->CreateMove(flInputSampleTime, cmd);
}
bool IClientModeWrapper001::ShouldDrawViewModel(void) {
	return m_int->ShouldDrawViewModel();
}
bool IClientModeWrapper001::ShouldDrawCrosshair(void) {
	return m_int->ShouldDrawCrosshair();
}
void IClientModeWrapper001::PreRender(CViewSetup* pSetup) {
	return m_int->PreRender(pSetup);
}
void IClientModeWrapper001::PostRender(void) {
	return m_int->PostRender();
}
wchar_t* IClientModeWrapper001::GetServerName() {
	return m_int->GetServerName();
}
wchar_t* IClientModeWrapper001::GetMapName() {
	return m_int->GetMapName();
}