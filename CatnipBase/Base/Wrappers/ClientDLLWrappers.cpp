#include "ClientDLLWrappers.h"
#include "SDK/InterfaceVersions/ClientDLL017.h"

int IClientDLLWrapper017::GetOffset(EOffsets Offset)
{
	switch (Offset)
	{
	case Off_CreateMove:
		return 21;
	}
	return -1;
}

ClientClass* IClientDLLWrapper017::GetAllClasses(void) {
    return m_int->GetAllClasses();
}
void IClientDLLWrapper017::CreateMove(int sequence_number, float input_sample_frametime, bool active) {
	return m_int->CreateMove(sequence_number, input_sample_frametime, active);
}
bool IClientDLLWrapper017::WriteUsercmdDeltaToBuffer(bf_write* buf, int from, int to, bool isnewcommand) {
	return m_int->WriteUsercmdDeltaToBuffer(buf, from, to, isnewcommand);
}
void IClientDLLWrapper017::View_Render(vrect_t* rect) {
	return m_int->View_Render(rect);
}
void IClientDLLWrapper017::RenderView(const CViewSetup& view, int nClearFlags, int whatToDraw) {
	return m_int->RenderView(view, nClearFlags, whatToDraw);
}
void IClientDLLWrapper017::SetCrosshairAngle(const QAngle& angle) {
	return m_int->SetCrosshairAngle(angle);
}
void IClientDLLWrapper017::InstallStringTableCallback(char const* tableName) {
	return m_int->InstallStringTableCallback(tableName);
}
void IClientDLLWrapper017::FrameStageNotify(ClientFrameStage_t curStage) {
	return m_int->FrameStageNotify(curStage);
}
bool IClientDLLWrapper017::DispatchUserMessage(int msg_type, bf_read& msg_data) {
	return m_int->DispatchUserMessage(msg_type, msg_data);
}
CStandardRecvProxies* IClientDLLWrapper017::GetStandardRecvProxies() {
	return m_int->GetStandardRecvProxies();
}
int IClientDLLWrapper017::GetScreenWidth() {
	return m_int->GetScreenWidth();
}
int IClientDLLWrapper017::GetScreenHeight() {
	return m_int->GetScreenHeight();
}
bool IClientDLLWrapper017::GetPlayerView(CViewSetup& playerView) {
	return m_int->GetPlayerView(playerView);
}
CRenamedRecvTableInfo* IClientDLLWrapper017::GetRenamedRecvTableInfos() {
	return m_int->GetRenamedRecvTableInfos();
}
