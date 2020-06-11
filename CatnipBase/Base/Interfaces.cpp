#include "Interfaces.h"
#include "Base.h"
#include "SDK/interface.h"
#include "Wrappers/EngineClientWrappers.h"
#include "Wrappers/ClientDLLWrappers.h"

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

	if (void* clientdll7 = fn("VClient017", 0))
		clientdll = new IClientDLLWrapper017(clientdll7);

	if (!engine || !clientdll)
		FATAL("Unsupported game interfaces");
}

void Interfaces::DestroyInterfaces()
{
	delete engine;
	engine = nullptr;
}
