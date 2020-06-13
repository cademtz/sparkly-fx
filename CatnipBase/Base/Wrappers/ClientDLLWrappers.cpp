#include "ClientDLLWrappers.h"
#include "SDK/InterfaceVersions/ClientDLL017.h"
#include "SDK/InterfaceVersions/ClientDLL018.h"

int IClientDLLWrapper017::GetOffset(EOffsets Offset)
{
	switch (Offset)
	{
	case Off_CreateMove:
		return 21;
	case Off_FrameStageNotify:
		return 35;
	}
	return -1;
}

int IClientDLLWrapper018::GetOffset(EOffsets Offset)
{
	switch (Offset)
	{
	case Off_CreateMove:
		return 22;
	case Off_FrameStageNotify:
		return 37;
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
bool IClientDLLWrapper017::GetPlayerView(CViewSetup& playerView) {
	return m_int->GetPlayerView(playerView);
}

ClientClass* IClientDLLWrapper018::GetAllClasses(void) {
	return m_int->GetAllClasses();
}
void IClientDLLWrapper018::CreateMove(int sequence_number, float input_sample_frametime, bool active) {
	return m_int->CreateMove(sequence_number, input_sample_frametime, active);
}
bool IClientDLLWrapper018::WriteUsercmdDeltaToBuffer(bf_write* buf, int from, int to, bool isnewcommand) {
	return m_int->WriteUsercmdDeltaToBuffer(buf, from, to, isnewcommand);
}
void IClientDLLWrapper018::View_Render(vrect_t* rect) {
	return m_int->View_Render(rect);
}
void IClientDLLWrapper018::RenderView(const CViewSetup& view, int nClearFlags, int whatToDraw) {
	return m_int->RenderView(view, nClearFlags, whatToDraw);
}
void IClientDLLWrapper018::InstallStringTableCallback(char const* tableName) {
	return m_int->InstallStringTableCallback(tableName);
}
void IClientDLLWrapper018::FrameStageNotify(ClientFrameStage_t curStage) {
	return m_int->FrameStageNotify(curStage);
}
bool IClientDLLWrapper018::DispatchUserMessage(int msg_type, bf_read& msg_data) {
	return m_int->DispatchUserMessage(msg_type, msg_data);
}
CStandardRecvProxies* IClientDLLWrapper018::GetStandardRecvProxies() {
	return m_int->GetStandardRecvProxies();
}
bool IClientDLLWrapper018::GetPlayerView(CViewSetup& playerView) {
	return m_int->GetPlayerView(playerView);
}
