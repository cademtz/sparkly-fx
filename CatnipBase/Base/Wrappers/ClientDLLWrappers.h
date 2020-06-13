#pragma once
#include "Wrappers.h"

class IBaseClientDLL;
class IBaseClientDLL018;

class IClientDLLWrapper017 : public IClientDLLWrapper
{
	IBaseClientDLL* m_int;

public:
	IClientDLLWrapper017(void* ClientDLL) : m_int((IBaseClientDLL*)ClientDLL) { }

	virtual int GetOffset(EOffsets Offset);

	virtual ClientClass* GetAllClasses(void);
	virtual void CreateMove(int sequence_number, float input_sample_frametime, bool active);
	virtual bool			WriteUsercmdDeltaToBuffer(bf_write* buf, int from, int to, bool isnewcommand);
	virtual void			View_Render(vrect_t* rect);
	virtual void			RenderView(const CViewSetup& view, int nClearFlags, int whatToDraw);
	virtual void			InstallStringTableCallback(char const* tableName);
	virtual void			FrameStageNotify(ClientFrameStage_t curStage);
	virtual bool			DispatchUserMessage(int msg_type, bf_read& msg_data);
	virtual CStandardRecvProxies* GetStandardRecvProxies();
	virtual bool			GetPlayerView(CViewSetup& playerView);
};

class IClientDLLWrapper018 : public IClientDLLWrapper
{
	IBaseClientDLL018* m_int;

public:
	IClientDLLWrapper018(void* ClientDLL) : m_int((IBaseClientDLL018*)ClientDLL) { }

	virtual int GetOffset(EOffsets Offset);

	virtual ClientClass* GetAllClasses(void);
	virtual void CreateMove(int sequence_number, float input_sample_frametime, bool active);
	virtual bool			WriteUsercmdDeltaToBuffer(bf_write* buf, int from, int to, bool isnewcommand);
	virtual void			View_Render(vrect_t* rect);
	virtual void			RenderView(const CViewSetup& view, int nClearFlags, int whatToDraw);
	virtual void			InstallStringTableCallback(char const* tableName);
	virtual void			FrameStageNotify(ClientFrameStage_t curStage);
	virtual bool			DispatchUserMessage(int msg_type, bf_read& msg_data);
	virtual CStandardRecvProxies* GetStandardRecvProxies();
	virtual bool			GetPlayerView(CViewSetup& playerView);
};