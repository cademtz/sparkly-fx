#include "Interfaces.h"
#include "Base.h"
#include "SDK/interface.h"
#include "Wrappers/EngineClientWrappers.h"

void Interfaces::CreateInterfaces()
{
	HMODULE engine_dll = Base::GetModule("engine.dll");
	CreateInterfaceFn fn = (CreateInterfaceFn)Base::GetProc(engine_dll, "CreateInterface");
	
	if (void* engine13 = fn("VEngineClient013", 0))
		engine = new IEngineClientWrapper013(engine13);
	else if (void* engine14 = fn("VEngineClient014", 0))
		engine = new IEngineClientWrapper014(engine14);

	if (!engine)
		FATAL("Unsupported game interfaces");
}

void Interfaces::DestroyInterfaces()
{
	delete engine;
	engine = nullptr;
}
