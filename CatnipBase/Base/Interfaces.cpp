#include "Interfaces.h"
#include "Base.h"
#include "Sig.h"
#include "Wrappers/EngineClientWrappers.h"
#include "Wrappers/ClientDLLWrappers.h"
#include "Wrappers/ClientModeWrappers.h"
#include "Wrappers/EntityListWrappers.h"

#ifdef _WIN64
#define SIG_CLIENTMODE "8B 0D ? ? ? ? 48 8B 01"
#else
#define SIG_CLIENTMODE "8B 0D ? ? ? ? 8B 01"
#endif

CreateInterfaceFn GetFactory(const char* Module)
{
	HMODULE hmod = Base::GetModule(Module);
	return (CreateInterfaceFn)Base::GetProc(hmod, "CreateInterface");
}

void Interfaces::CreateInterfaces()
{
	CreateInterfaceFn fn = GetFactory("engine.dll");

	if (void* engine13 = fn("VEngineClient013", 0))
		engine = new IEngineClientWrapper013(engine13);
	else if (void* engine14 = fn("VEngineClient014", 0))
		engine = new IEngineClientWrapper014(engine14);

	fn = GetFactory("client.dll");

	if (void* clientdll17 = fn("VClient017", 0))
		hlclient = new IClientDLLWrapper017(clientdll17);
	else if (void* clientdll18 = fn("VClient018", 0))
		hlclient = new IClientDLLWrapper018(clientdll18);
	if (void* entitylist = fn(VCLIENTENTITYLIST_INTERFACE_VERSION, 0))
		entlist = new IClientEntityListWrapper003(entitylist);

	fn = GetFactory("vgui2.dll");

	if (void* vguipanel = fn(VGUI_PANEL_INTERFACE_VERSION, 0))
		panels = (vgui::IPanel*)vguipanel;

	if (!engine || !hlclient || !entlist || !panels)
		FATAL("Unsupported game interfaces");

	void* hudprocinput = (*(void***)hlclient->Inst())[10];
	void* clientref = Sig::FindPattern<void*>(hudprocinput, 0x10, SIG_CLIENTMODE);
	if (!clientref)
		FATAL("Failed signature to g_pClientModeShared");

	void* clientmode = *AsmTools::Relative<void**>(clientref, 2);
	switch (engine->GetAppID())
	{
	case AppId_CSGO:
		client = new IClientModeWrapperCSGO(clientmode);
		break;
	default:
		client = new IClientModeWrapperSDK(clientmode);
	}
}

void Interfaces::DestroyInterfaces()
{
	delete engine;
	delete hlclient;
	delete entlist;
	delete client;
	engine = nullptr;
	hlclient = nullptr;
	entlist = nullptr;
	client = nullptr;
}
