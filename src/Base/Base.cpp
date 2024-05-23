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

#include <strsafe.h>

#define MAX_BACKTRACE_FRAMES 50
#define CRASHLOG_NAME L"xsdk-crashlog.txt"

LONG NTAPI MyVectoredHandler(struct _EXCEPTION_POINTERS* ExceptionInfo);
static PVOID my_vectored_handler = nullptr;
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
    RemoveVectoredExceptionHandler(my_vectored_handler);
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
    my_vectored_handler = AddVectoredExceptionHandler(TRUE, MyVectoredHandler);

    Netvars::GetNetvars();
    CModule::StartAll();
    CBaseHook::HookAll();

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

const std::filesystem::path& Base::GetModuleDir()
{
    return module_dir;
}


// === Stack trace code === //

// Parts of the following code were copied and modified from ClassiCube, which can be found here:
// https://github.com/UnknownShadow200/ClassiCube

static const char* ExceptionDescribe(const uint32_t code)
{
#define X(x) case (x): return #x
    switch (code)
    {
    X(EXCEPTION_ACCESS_VIOLATION);
    X(EXCEPTION_DATATYPE_MISALIGNMENT);
    X(EXCEPTION_BREAKPOINT);
    X(EXCEPTION_SINGLE_STEP);
    X(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
    X(EXCEPTION_FLT_DENORMAL_OPERAND);
    X(EXCEPTION_FLT_DIVIDE_BY_ZERO);
    X(EXCEPTION_FLT_INEXACT_RESULT);
    X(EXCEPTION_FLT_INVALID_OPERATION);
    X(EXCEPTION_FLT_OVERFLOW);
    X(EXCEPTION_FLT_STACK_CHECK);
    X(EXCEPTION_FLT_UNDERFLOW);
    X(EXCEPTION_INT_DIVIDE_BY_ZERO);
    X(EXCEPTION_INT_OVERFLOW);
    X(EXCEPTION_PRIV_INSTRUCTION);
    X(EXCEPTION_IN_PAGE_ERROR);
    X(EXCEPTION_ILLEGAL_INSTRUCTION);
    X(EXCEPTION_NONCONTINUABLE_EXCEPTION);
    X(EXCEPTION_STACK_OVERFLOW);
    X(EXCEPTION_INVALID_DISPOSITION);
    X(EXCEPTION_GUARD_PAGE);
    X(EXCEPTION_INVALID_HANDLE);
    X(CONTROL_C_EXIT);
    default: return nullptr;
    }
#undef X
}

struct CrashWriter
{
    bool Open()
    {
        wchar_t path[512];
        StringCchPrintfW(
            path,
            std::size(path),
            L"%s\\%s",
            Base::GetModuleDir().wstring().c_str(),
            CRASHLOG_NAME
        );

        HANDLE hFile = CreateFileW(
            path,
            GENERIC_READ | FILE_APPEND_DATA,
            0,
            nullptr,
            OPEN_ALWAYS, // Open or create
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (!hFile)
            return {};

        this->m_hFile = hFile;
        return true;
    }

    bool WriteException(_EXCEPTION_POINTERS* info)
    {
        constexpr auto header = R"(
--------------------------------------------------
                 xsdk crash log
--------------------------------------------------
)";

        if (!PutFile(header))
            return false;

        SYSTEMTIME time;
        GetSystemTime(&time);

        if (!PutFile(
            "Date (y-m-d h:m:s): %d-%d-%d %dh:%dm:%ds\n",
            time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond)
        ) {
            return false;
        }

        void* addr = info->ExceptionRecord->ExceptionAddress;
        DWORD code = info->ExceptionRecord->ExceptionCode;
        const char* desc = ExceptionDescribe(code);

        if (desc && !PutFile("Unhandled %s error at 0x%x", desc, addr))
            return false;
        if (!desc && !PutFile("Unhandled exception 0x%x at 0x%x", code, addr))
            return false;

        DWORD numArgs = info->ExceptionRecord->NumberParameters;
        if (numArgs)
        {
            numArgs = min(numArgs, EXCEPTION_MAXIMUM_PARAMETERS);
            if (!PutFile("["))
                return false;

            for (int i = 0; i < numArgs; i++)
            {
                ULONG_PTR exceptionInfo = info->ExceptionRecord->ExceptionInformation[i];
                if (!PutFile("0x%x,", exceptionInfo))
                    return false;
            }

            if (!PutFile("]"))
                return false;
        }

        if (!PutFile("\n\n"))
            return false;

        if (!WriteBacktrace(info->ContextRecord))
            return false;

        return PutFile("\n");
    }

    ~CrashWriter()
    {
        if (m_hFile && m_hFile != INVALID_HANDLE_VALUE)
            CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    explicit CrashWriter() : m_hFile(INVALID_HANDLE_VALUE)
    {

    }

private:
    static int GetFrames(CONTEXT* ctx, uintptr_t* addrs, int max)
    {
        return RtlCaptureStackBackTrace(3, max, (void**)addrs, nullptr);
    }

    bool WriteFrame(uintptr_t addr, uintptr_t symAddr, const char* symName,
                       const char* modName)
    {
        if (!PutFile("0x%x - ", addr))
            return false;

        bool validModName = modName && modName[0];

        if (validModName && !PutFile("%s.dll", modName))
            return false;
        if (!validModName && !PutFile("???.dll"))
            return false;

        if (symName && symName[0])
        {
            uintptr_t offset = addr - symAddr;
            if (!PutFile("(%s+0x%x)", symName, offset))
                return false;
        }

        return PutFile("\n");
    }

    bool DumpFrame(HANDLE process, uintptr_t addr)
    {
        struct SymbolAndName
        {
            IMAGEHLP_SYMBOL symbol;
            char name[256];
        };

        SymbolAndName s{};
        s.symbol.MaxNameLength = 255;
        s.symbol.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        if (!SymGetSymFromAddr(process, addr, nullptr, &s.symbol))
            ZeroMemory(&s, sizeof(s));

        IMAGEHLP_MODULE m{};
        m.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
        SymGetModuleInfo(process, addr, &m);

        const size_t module_name_len = std::size(m.ModuleName);
        char module_name_ascii[module_name_len]{};
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

        if (!WriteFrame(addr, s.symbol.Address, s.symbol.Name, module_name_ascii))
            return false;

        /* This function only works for .pdb debug info anyways */
        /* This function is also missing on Windows 9X */
#if _MSC_VER
        IMAGEHLP_LINE line{};
        DWORD lineOffset;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
        if (SymGetLineFromAddr(process, addr, &lineOffset, &line))
        {
            if (!PutFile("  line %d in ", line.LineNumber))
                return false;

            char filename[512]{};
            for (int i = 0; i < std::size(filename); i++)
            {
                auto ch = line.FileName[i];
                if (ch == 0)
                    break;
                filename[i] = (char) ch;
            }

            if (!PutFile("%s", filename))
                return false;

            return PutFile("\n");
        }
#endif

        return true;
    }

    bool WriteBacktrace(CONTEXT* ctx)
    {
        uintptr_t addrs[MAX_BACKTRACE_FRAMES];
        int numFrames = GetFrames(ctx, addrs, MAX_BACKTRACE_FRAMES);

        for (int i = 0; i < numFrames; i++)
        {
            if (!DumpFrame(GetCurrentProcess(), addrs[i]))
                return false;
        }

        return true;
    }

    bool PutFile(const char* fmt, ...)
    {
        char buffer[2048];
        va_list va;
        va_start(va, fmt);
        HRESULT result = StringCchVPrintfA(buffer, std::size(buffer), fmt, va);
        va_end(va);

        if (result != S_OK)
            return false;

        size_t len;
        if (StringCchLengthA(buffer, std::size(buffer), &len) != S_OK)
            return false;

        return WriteFile(m_hFile, buffer, len, nullptr, nullptr) != 0;
    }

    HANDLE m_hFile = INVALID_HANDLE_VALUE;
};


static LONG NTAPI MyVectoredHandler(_EXCEPTION_POINTERS* info)
{
    // Ignore informal exceptions. They are spammy and don't seem to cause any real crashes.
    // https://stackoverflow.com/questions/12298406/how-to-treat-0x40010006-exception-in-vectored-exception-handler
    if (info->ExceptionRecord->ExceptionCode < 0x80000000u)
        return EXCEPTION_CONTINUE_SEARCH;

    CrashWriter writer;
    if (!writer.Open() || !writer.WriteException(info))
    {
        char msg[64];
        StringCchPrintfA(msg, std::size(msg), "Failed to write crash log.\nGetLastError(): %d", GetLastError());

        MessageBoxA(
            nullptr,
            msg,
            "XSDK",
            MB_OK
        );
    }

    return EXCEPTION_CONTINUE_SEARCH;
}
