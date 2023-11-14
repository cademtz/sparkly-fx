#include "Base.h"
#include <Hooks/WindowHook.h>
#include <Hooks/OverlayHook.h>
#include <Modules/BaseModule.h>
#include <SDK/mathlib.h>
#include "Interfaces.h"
#include "Netvars.h"

// Stack trace code
#include <string>
#include <cstdint>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")
#define MAX_BACKTRACE_FRAMES 50

LONG WINAPI UnhandledFilter(struct _EXCEPTION_POINTERS* info);
LONG NTAPI MyVectoredHandler(struct _EXCEPTION_POINTERS *ExceptionInfo);
//

void Base::OnAttach(HMODULE Module)
{
	hInst = Module;
	CreateThread(0, 0, &Base::HookThread, 0, 0, 0);
}

DWORD WINAPI Base::HookThread(LPVOID Args)
{
	while (!(hWnd = FindWindowA("Valve001", 0)))
		Sleep(100);

	MathLib_Init();

#ifdef _DEBUG
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
#endif

	Interfaces::CreateInterfaces();
	//Logger_Hook();
	Netvars::GetNetvars();
	CBaseHook::HookAll();
	CModule::StartAll();

	return 0;
}

HMODULE Base::GetModule(const char* Module)
{
	HMODULE hMod = GetModuleHandleA(Module);
	if (!hMod)
		FATAL("Failed GetModuleHandle to '%s'", Module);
	return hMod;
}

FARPROC Base::GetProc(HMODULE Module, const char* Proc)
{
	FARPROC result = GetProcAddress(Module, Proc);
	if (!result)
	{
		char name[MAX_PATH];
		GetModuleFileNameA(Module, name, MAX_PATH);
		FATAL("Failed GetProcAddress to '%s' in '%s'", Proc, name);
	}
	return result;
}

void Base::Fatal(const char* Title, const char* Format, ...)
{
	char buf[512];

	va_list va;
	va_start(va, Format);
	vsprintf_s(buf, Format, va);
	va_end(va);

	MessageBoxA(0, buf, Title, MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), -1);
}

// === Stack trace code === //

// The following code was copied and modified from ClassiCube, which can be found here:
// https://github.com/UnknownShadow200/ClassiCube

static int GetFrames(CONTEXT* ctx, uintptr_t* addrs, int max)
{
	STACKFRAME frame = { 0 };
	uintptr_t spRegister;
	int count, type;
	HANDLE thread;

	frame.AddrPC.Mode    = AddrModeFlat;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Mode = AddrModeFlat;

#if defined _M_IX86
	type = IMAGE_FILE_MACHINE_I386;
	frame.AddrPC.Offset    = ctx->Eip;
	frame.AddrFrame.Offset = ctx->Ebp;
	frame.AddrStack.Offset = ctx->Esp;
	spRegister             = ctx->Esp;
#elif defined _M_X64
	type = IMAGE_FILE_MACHINE_AMD64;
	frame.AddrPC.Offset    = ctx->Rip;
	frame.AddrFrame.Offset = ctx->Rsp;
	frame.AddrStack.Offset = ctx->Rsp;
	spRegister             = ctx->Rsp;
#else
	/* Always available after XP, so use that */
	return RtlCaptureStackBackTrace(0, max, (void**)addrs, NULL);
#endif
	thread = GetCurrentThread();

	for (count = 0; count < max; count++) 
	{
		if (!StackWalk(type, GetCurrentProcess(), thread, &frame, ctx, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL)) break;
		if (!frame.AddrFrame.Offset) break;
		addrs[count] = frame.AddrPC.Offset;
	}
	return count;
}

static void PrintFrame(std::string& str, uintptr_t addr, uintptr_t symAddr, const char* symName, const char* modName) {
	std::string module;
	int offset;
	if (modName)
		module = std::string(modName) + ".dll";
	else
		module = "???";

	// This trims the full path down
	//Utils_UNSAFE_GetFilename(&module);
	char buf[128];
	sprintf_s(buf, sizeof(buf), "%p - %s", (void*)addr, module.c_str());
	str += buf;
	
	if (symName && symName[0]) {
		offset = (int)(addr - symAddr);
		sprintf_s(buf, sizeof(buf), "(%s+0x%x)", symName, offset);
		str += buf;
	}
	str += '\n';
}

struct SymbolAndName { IMAGEHLP_SYMBOL symbol; char name[256]; };
static void DumpFrame(std::string& str, HANDLE process, uintptr_t addr) {

	struct SymbolAndName s = { 0 };
	s.symbol.MaxNameLength = 255;
	s.symbol.SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
	if (!SymGetSymFromAddr(process, addr, NULL, &s.symbol))
		ZeroMemory(&s, sizeof(s));

	IMAGEHLP_MODULE m = { 0 };
	m.SizeOfStruct    = sizeof(IMAGEHLP_MODULE);
	SymGetModuleInfo(process, addr, &m);

	PrintFrame(str, addr, s.symbol.Address, s.symbol.Name, m.ModuleName);

	/* This function only works for .pdb debug info anyways */
	/* This function is also missing on Windows 9X */
#if _MSC_VER
	IMAGEHLP_LINE line = { 0 }; DWORD lineOffset;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
	if (SymGetLineFromAddr(process, addr, &lineOffset, &line)) {
		str += "  line " + std::to_string(line.LineNumber) + " in " + line.FileName + '\n';
	}
#endif
}

static void Backtrace(std::string& str, CONTEXT* ctx) {
	uintptr_t addrs[MAX_BACKTRACE_FRAMES];
	int i, frames;

	SymInitialize(GetCurrentProcess(), NULL, TRUE); /* TODO only in MSVC.. */
	frames  = GetFrames(ctx, addrs, MAX_BACKTRACE_FRAMES);

	for (i = 0; i < frames; i++) {
		DumpFrame(str, GetCurrentProcess(), addrs[i]);
	}
}

static const char* ExceptionDescribe(uint32_t code) {
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION: 			return "EXCEPTION_ACCESS_VIOLATION";
	case EXCEPTION_DATATYPE_MISALIGNMENT: 		return "EXCEPTION_DATATYPE_MISALIGNMENT";
	case EXCEPTION_BREAKPOINT: 					return "EXCEPTION_BREAKPOINT";
	case EXCEPTION_SINGLE_STEP: 				return "EXCEPTION_SINGLE_STEP";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: 		return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
	case EXCEPTION_FLT_DENORMAL_OPERAND: 		return "EXCEPTION_FLT_DENORMAL_OPERAND";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO: 			return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
	case EXCEPTION_FLT_INEXACT_RESULT: 			return "EXCEPTION_FLT_INEXACT_RESULT";
	case EXCEPTION_FLT_INVALID_OPERATION: 		return "EXCEPTION_FLT_INVALID_OPERATION";
	case EXCEPTION_FLT_OVERFLOW: 				return "EXCEPTION_FLT_OVERFLOW";
	case EXCEPTION_FLT_STACK_CHECK: 			return "EXCEPTION_FLT_STACK_CHECK";
	case EXCEPTION_FLT_UNDERFLOW: 				return "EXCEPTION_FLT_UNDERFLOW";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:			return "EXCEPTION_INT_DIVIDE_BY_ZERO";
	case EXCEPTION_INT_OVERFLOW: 				return "EXCEPTION_INT_OVERFLOW";
	case EXCEPTION_PRIV_INSTRUCTION: 			return "EXCEPTION_PRIV_INSTRUCTION";
	case EXCEPTION_IN_PAGE_ERROR: 				return "EXCEPTION_IN_PAGE_ERROR";
	case EXCEPTION_ILLEGAL_INSTRUCTION: 		return "EXCEPTION_ILLEGAL_INSTRUCTION";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:	return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
	case EXCEPTION_STACK_OVERFLOW: 				return "EXCEPTION_STACK_OVERFLOW";
	case EXCEPTION_INVALID_DISPOSITION: 		return "EXCEPTION_INVALID_DISPOSITION";
	case EXCEPTION_GUARD_PAGE: 					return "EXCEPTION_GUARD_PAGE";
	case EXCEPTION_INVALID_HANDLE: 				return "EXCEPTION_INVALID_HANDLE";
	//case EXCEPTION_POSSIBLE_DEADLOCK: 			return "EXCEPTION_POSSIBLE_DEADLOCK";
	case CONTROL_C_EXIT: 						return "CONTROL_C_EXIT";
	}
	return NULL;
}

static LONG WINAPI UnhandledFilter(struct _EXCEPTION_POINTERS* info) {
	std::string msg;
	const char* desc;
	uint32_t code;
	uintptr_t addr;
	DWORD i, numArgs;
	char buf[128];

	code = (uint32_t)info->ExceptionRecord->ExceptionCode;
	addr = (uintptr_t)info->ExceptionRecord->ExceptionAddress;
	desc = ExceptionDescribe(code);

	if (desc) {
		sprintf_s(buf, sizeof(buf), "Unhandled %s error at %p", desc, (void*)addr);
	} else {
		sprintf_s(buf, sizeof(buf), "Unhandled exception 0x%x at %p", code, (void*)addr);
	}
	msg += buf;

	numArgs = info->ExceptionRecord->NumberParameters;
	if (numArgs) {
		numArgs = min(numArgs, EXCEPTION_MAXIMUM_PARAMETERS);
		msg += '[';

		for (i = 0; i < numArgs; i++) {
			sprintf_s(buf, sizeof(buf), "%p,", (void*)info->ExceptionRecord->ExceptionInformation[i]);
			msg += buf;
		}
		msg += ']';
	}

	msg += "\n\n";
	
	Backtrace(msg, info->ContextRecord);
	MessageBoxA(NULL, msg.c_str(), "Unhandled exception", MB_OK);

	return EXCEPTION_CONTINUE_SEARCH; /* TODO: different flag */
}

static LONG NTAPI MyVectoredHandler(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	return UnhandledFilter(ExceptionInfo);
}