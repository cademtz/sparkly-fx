#include "ClientModeWrappers.h"
#include "SDK/InterfaceVersions/ClientModeSDK.h"
#include "SDK/InterfaceVersions/ClientModeCSGO.h"

int IClientModeWrapperSDK::GetOffset(EOffsets Offset)
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
	case Off_PostRender:
		return 28;
	case Off_PostRenderVGui:
		return 29;
	}
	return -1;
}

int IClientModeWrapperCSGO::GetOffset(EOffsets Offset)
{
	switch (Offset)
	{
	case Off_ShouldDrawDetailObjects:
		return 13;
	case Off_ShouldDrawEntity:
		return 14;
	case Off_ShouldDrawLocalPlayer:
		return 15;
	case Off_ShouldDrawParticles:
		return 16;
	case Off_CreateMove:
		return 24;
	case Off_ShouldDrawViewModel:
		return 27;
	case Off_ShouldDrawCrosshair:
		return 28;
	case Off_PostRender:
		return 31;
	case Off_PostRenderVGui:
		return 32;
	}
	return -1;
}

bool IClientModeWrapperSDK::ShouldDrawDetailObjects() {
    return m_int->ShouldDrawDetailObjects();
}
bool IClientModeWrapperSDK::ShouldDrawEntity(C_BaseEntity* pEnt) {
    return m_int->ShouldDrawEntity(pEnt);
}
bool IClientModeWrapperSDK::ShouldDrawLocalPlayer(C_BasePlayer* pPlayer) {
    return m_int->ShouldDrawLocalPlayer(pPlayer);
}
bool IClientModeWrapperSDK::ShouldDrawParticles() {
	return m_int->ShouldDrawParticles();
}
bool IClientModeWrapperSDK::CreateMove(float flInputSampleTime, CUserCmd* cmd) {
	return m_int->CreateMove(flInputSampleTime, cmd);
}
bool IClientModeWrapperSDK::ShouldDrawViewModel(void) {
	return m_int->ShouldDrawViewModel();
}
bool IClientModeWrapperSDK::ShouldDrawCrosshair(void) {
	return m_int->ShouldDrawCrosshair();
}
void IClientModeWrapperSDK::PreRender(CViewSetup* pSetup) {
	return m_int->PreRender(pSetup);
}
void IClientModeWrapperSDK::PostRender(void) {
	return m_int->PostRender();
}
void IClientModeWrapperSDK::PostRenderVGui() {
	return m_int->PostRenderVGui();
}

bool IClientModeWrapperCSGO::ShouldDrawDetailObjects() {
	return m_int->ShouldDrawDetailObjects();
}
bool IClientModeWrapperCSGO::ShouldDrawEntity(C_BaseEntity* pEnt) {
	return m_int->ShouldDrawEntity(pEnt);
}
bool IClientModeWrapperCSGO::ShouldDrawLocalPlayer(C_BasePlayer* pPlayer) {
	return m_int->ShouldDrawLocalPlayer(pPlayer);
}
bool IClientModeWrapperCSGO::ShouldDrawParticles() {
	return m_int->ShouldDrawParticles();
}
bool IClientModeWrapperCSGO::CreateMove(float flInputSampleTime, CUserCmd* cmd) {
	return m_int->CreateMove(flInputSampleTime, cmd);
}
bool IClientModeWrapperCSGO::ShouldDrawViewModel(void) {
	return m_int->ShouldDrawViewModel();
}
bool IClientModeWrapperCSGO::ShouldDrawCrosshair(void) {
	return m_int->ShouldDrawCrosshair();
}
void IClientModeWrapperCSGO::PreRender(CViewSetup* pSetup) {
	return m_int->PreRender(pSetup);
}
void IClientModeWrapperCSGO::PostRender(void) {
	return m_int->PostRender();
}
void IClientModeWrapperCSGO::PostRenderVGui() {
	return m_int->PostRenderVGui();
}
