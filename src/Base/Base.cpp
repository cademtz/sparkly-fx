#include "Base.h"
#include <Hooks/WindowHook.h>
#include <Hooks/OverlayHook.h>
#include <Modules/BaseModule.h>
#include "Interfaces.h"
#include "Netvars.h"
#include <array>
#include <filesystem>

// Stack trace code
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <cstdint>
#define DBGHELP_TRANSLATE_TCHAR
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

#define MAX_BACKTRACE_FRAMES 50
#define CRASHLOG_NAME "xsdk-crashlog.txt"

LONG NTAPI MyVectoredHandler(struct _EXCEPTION_POINTERS *ExceptionInfo);
//static PVOID my_vectored_handler = nullptr;
//

static std::filesystem::path module_dir;

void Base::OnAttach(HMODULE Module)
{
	hInst = Module;
	CreateThread(0, 0, &Base::HookThread, 0, 0, 0);
}

void Base::OnDetach()
{
	// Call this to free the handle on our PDB file.
	// This allows re-compilation after ejecting xsdk.
	SymCleanup(GetCurrentProcess());
	// TODO: Make interfaces and other memory use smart pointers, which will self-destruct automatically.
	Interfaces::DestroyInterfaces();
	//RemoveVectoredExceptionHandler(my_vectored_handler);
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
	
	// At this point, the game engine is fully initialized.
	// Now we can insert our exception handlers and hooks.

	// Load symbols for nicer stack traces
	LoadLibraryA("IMAGEHLP.DLL");
	std::array<TCHAR, MAX_PATH + 1> module_filename = {0};
	if (GetModuleFileName(hInst, module_filename.data(), module_filename.size()))
	{
		// We found our module's path. Load symbols with its folder as a search path.
		module_dir = std::filesystem::path(module_filename.data()).parent_path();
		SymInitialize(GetCurrentProcess(), module_dir.c_str(), TRUE);
	}
	else // We can't find our module's path, so we pass a NULL search path.
		SymInitialize(GetCurrentProcess(), NULL, TRUE);
	
	// This won't have an effect, because Source-Engine/Steam catches all crashes.
	//SetUnhandledExceptionFilter(UnhandledFilter);
	
	// This works, because we add a new "first" handler after all others have been set up.
	//my_vectored_handler = AddVectoredExceptionHandler(TRUE, MyVectoredHandler);

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

const std::filesystem::path& Base::GetModuleDir() {
	return module_dir;
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
//#elif defined _M_X64
#elif 0 // x64 trace crashes when recording more than a couple frames. Fall back to RtlCaptureStackBackTrace
	type = IMAGE_FILE_MACHINE_AMD64;
	frame.AddrPC.Offset    = ctx->Rip;
	frame.AddrFrame.Offset = ctx->Rsp;
	frame.AddrStack.Offset = ctx->Rsp;
	spRegister             = ctx->Rsp;
#else
	/* Always available after XP, so use that */
	// Skip 3 frames (GetFrames <- Backtrace <- MyVectoredHandler)
	return RtlCaptureStackBackTrace(3, max, (void**)addrs, NULL);
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

static void PrintFrame(std::stringstream& str, uintptr_t addr, uintptr_t symAddr, const char* symName, const char* modName) {
	str << (void*)addr << " - ";
	if (modName && modName[0])
		str << modName << ".dll";
	else
		str << "???";
	
	if (symName && symName[0]) {
		uintptr_t offset = addr - symAddr;
		str << '(' << symName << '+' << (void*)offset << ')';
	}
	str << std::endl;
}

struct SymbolAndName { IMAGEHLP_SYMBOL symbol; char name[256]; };
static void DumpFrame(std::stringstream& str, HANDLE process, uintptr_t addr) {

	struct SymbolAndName s = { 0 };
	s.symbol.MaxNameLength = 255;
	s.symbol.SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
	if (!SymGetSymFromAddr(process, addr, NULL, &s.symbol))
		ZeroMemory(&s, sizeof(s));

	IMAGEHLP_MODULE m = { 0 };
	m.SizeOfStruct    = sizeof(IMAGEHLP_MODULE);
	SymGetModuleInfo(process, addr, &m);

	const size_t module_name_len = sizeof(m.ModuleName) / sizeof(m.ModuleName[0]);
	char module_name_ascii[module_name_len] = {0};
	for (size_t i = 0; i < module_name_len; ++i)
	{
		auto ch = m.ModuleName[i];
		if (ch == 0)
		{
			module_name_ascii[i] = 0;
			break;
		}

		if (ch < 32 || ch > 126)
			ch = '?';
		module_name_ascii[i] = (char)ch;
	}
	PrintFrame(str, addr, s.symbol.Address, s.symbol.Name, module_name_ascii);

	/* This function only works for .pdb debug info anyways */
	/* This function is also missing on Windows 9X */
#if _MSC_VER
	IMAGEHLP_LINE line = { 0 }; DWORD lineOffset;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
	if (SymGetLineFromAddr(process, addr, &lineOffset, &line)) {
		str << "  line " << line.LineNumber << " in " << line.FileName << std::endl;
	}
#endif
}

static void Backtrace(std::stringstream& str, CONTEXT* ctx) {
	uintptr_t addrs[MAX_BACKTRACE_FRAMES];
	int i, frames;

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

static LONG NTAPI MyVectoredHandler(struct _EXCEPTION_POINTERS* info) {
	// FIXME(Cade): The code in here can absolutely cause crashes, even during non-fatal exceptions.
	// So ironically, this causes more crashes.
	// It should be rewritten to avoid using any C/C++ runtime libraries.
	std::stringstream trace;
	const char* desc;
	uint32_t code;
	uintptr_t addr;
	DWORD i, numArgs;

	{
		auto time_now = std::chrono::system_clock::now();
		std::time_t rawtime = std::chrono::system_clock::to_time_t(time_now);
		trace << "--------------------------------------------------" << std::endl;
		trace << std::put_time(std::localtime(&rawtime), "%c %Z") << std::endl;
		trace << "--------------------------------------------------" << std::endl;
	}

	code = (uint32_t)info->ExceptionRecord->ExceptionCode;
	addr = (uintptr_t)info->ExceptionRecord->ExceptionAddress;
	desc = ExceptionDescribe(code);

	if (desc)
		trace << "Unhandled " << desc << " error at " << (void*)addr;
	else
		trace << "Unhandled exception 0x" << std::hex << code << std::dec << " at " << (void*)addr;

	numArgs = info->ExceptionRecord->NumberParameters;
	if (numArgs) {
		numArgs = min(numArgs, EXCEPTION_MAXIMUM_PARAMETERS);
		trace << '[';

		for (i = 0; i < numArgs; i++)
			trace << (void*)info->ExceptionRecord->ExceptionInformation[i] << ',';
		trace << ']';
	}

	trace << std::endl << std::endl;
	Backtrace(trace, info->ContextRecord);
	trace << std::endl;

	std::filesystem::path log_path = Base::GetModuleDir() / CRASHLOG_NAME;
	std::fstream log_file = std::fstream(log_path, std::ios::out | std::ios::app);
	if (log_file)
	{
		log_file << trace.rdbuf();
		log_file.close();
	}

	// This exception nag is really annoying and unnecessary.
	// I'll leave the commented code here incase it's useful later.

	/*static bool is_first_time = true;
	if (is_first_time)
	{
		std::basic_stringstream<TCHAR> msg;
		is_first_time = false;

		std::error_code err;
		auto absolute_log_path = std::filesystem::absolute(log_path, err);
		if (err)
			absolute_log_path = log_path;

#ifdef UNICODE
		auto log_path_string = absolute_log_path.wstring();
#else
		auto log_path_string = absolute_log_path.string();
#endif

		msg << "An error occured. The program may crash." << std::endl;
		msg << "Details were written to:" << std::endl;
		msg << log_path_string << std::endl;

		MessageBox(
			NULL, 
			msg.str().c_str(),
			TEXT("Unhandled exception"),
			MB_OK | MB_ICONWARNING
		);
	}*/

	return EXCEPTION_CONTINUE_SEARCH;
}
