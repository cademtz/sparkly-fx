#include "Base.h"
#include <Hooks/WindowHook.h>
#include <Hooks/OverlayHook.h>
#include <Modules/BaseModule.h>
#include "Interfaces.h"
#include "Netvars.h"
#include <array>
#include <filesystem>
#include <cinttypes>

#include <shellapi.h> // CommandLineToArgvW


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

#define CRASHDUMP_NAME L"xsdk-minidump.dmp"

LONG NTAPI MyVectoredHandler(struct _EXCEPTION_POINTERS* ExceptionInfo);
static PVOID my_vectored_handler = nullptr;
//

enum
{
    DUMPMODE_NONE = 0,
    DUMPMODE_DEFAULT = 1,
    DUMPMODE_EXTENDED = 2
};
static int crash_minidump_mode = DUMPMODE_NONE;

#define MINIDUMP_MODE_FLAG L"-xsdk_minidump_mode"


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
    // Collect command line arguments
    CommandLineW = GetCommandLineW();

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(CommandLineW.c_str(), &argc);

    for (int i = 0; argv && i < argc; i++)
    {
        if (argv[i] && std::wcslen(argv[i]) > 0)
            CommandLineArgsW.emplace_back(argv[i]);
    }

    if (argv)
        LocalFree(argv);

    while (!(hWnd = FindWindowA("Valve001", 0)))
        Sleep(100);

    if (const std::wstring* dumpType = GetCommandLineParam(MINIDUMP_MODE_FLAG); dumpType && dumpType->size() == 1)
    {
        switch (*dumpType->data())
        {
        case L'1':
            crash_minidump_mode = DUMPMODE_DEFAULT;
            break;
        case L'2':
            crash_minidump_mode = DUMPMODE_EXTENDED;
            break;
        default:
            crash_minidump_mode = DUMPMODE_NONE;
            break;
        }

    }

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
        SymInitializeW(GetCurrentProcess(), module_dir.c_str(), TRUE);
    }
    else // We can't find our module's path, so we pass a NULL search path.
        SymInitializeW(GetCurrentProcess(), NULL, TRUE);

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

bool Base::CommandLineContains(std::wstring_view needle)
{
    return CommandLineW.find(needle) != std::wstring::npos;
}

const std::wstring* Base::GetCommandLineParam(std::wstring_view paramName)
{
    for (size_t i = 0; i < CommandLineArgsW.size(); i++)
    {
        const std::wstring* arg = &CommandLineArgsW[i];
        if (arg->compare(paramName) == 0 && i + 1 < CommandLineArgsW.size())
            return &CommandLineArgsW.at(i+1);
    }

    return nullptr;
}

// === Stack trace code === //

// Parts of the following code were copied and modified from ClassiCube, which can be found here:
// https://github.com/UnknownShadow200/ClassiCube

static const wchar_t* ExceptionDescribe(const uint32_t code)
{
#define X(x) case (x): return L#x
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
    struct SymbolAndName
    {
        SYMBOL_INFOW Symbol;
        wchar_t Name[256];
    };

    ~CrashWriter()
    {
        if (m_hFile && m_hFile != INVALID_HANDLE_VALUE)
            CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    bool Open()
    {
        wchar_t path[512];
        if (!GetModuleRelPath(path, std::size(path), CRASHLOG_NAME))
            return false;

        HANDLE hFile = CreateFileW(
            path,
            GENERIC_READ | FILE_APPEND_DATA,
            0,
            nullptr,
            OPEN_ALWAYS, // Open or create
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hFile == nullptr || hFile == INVALID_HANDLE_VALUE)
            return false;

        this->m_hFile = hFile;
        return true;
    }


    bool WriteException(_EXCEPTION_POINTERS* info)
    {
        constexpr auto header = LR"(
--------------------------------------------------
                 xsdk crash log
--------------------------------------------------
)";

        if (!PutFile(header))
            return false;

        SYSTEMTIME time;
        GetSystemTime(&time);

        if (!PutFileFmt(
            L"Date (y-m-d h:m:s): %d-%d-%d %dh:%dm:%ds\n",
            time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond)
        ) {
            return false;
        }

        if (!PutFileFmt(L"Command line: %s\n\n", Base::CommandLineW.c_str()))
            return false;

        wchar_t wtmpbuf[512];

        if (crash_minidump_mode != DUMPMODE_NONE)
        {
            bool status = GetModuleRelPath(
                wtmpbuf,
                std::size(wtmpbuf),
                CRASHDUMP_NAME
            );

            if (!status)
                return PutFile(L"Failed to get minidump path\n");

            if (WriteMiniDump(wtmpbuf, info, crash_minidump_mode == DUMPMODE_EXTENDED))
            {
                const wchar_t* fmt = L"Minidump: %s\n"
                    "IMPORTANT: the minidump may contain sensitive information, only share with people you trust!!\n";

                if (!PutFileFmt(fmt, wtmpbuf))
                    return false;

            }
            else if (!PutFileFmt(L"Minidump: error creating minidump. GetLastError() = %d\n", GetLastError()))
                return false;
        }
        else if (!PutFileFmt(
            L"Minidump: disabled\nLaunch the game with '%s 1' to enable minidumps, and '%s 2' to enable extended minidumps\n",
            MINIDUMP_MODE_FLAG,
            MINIDUMP_MODE_FLAG
        ))
            return false;

        if (!PutFile(L"\n"))
            return false;

        if (!DumpThreadContext(info->ContextRecord))
            return false;

        if (!PutFile(L"\n"))
            return false;

        void* addr = info->ExceptionRecord->ExceptionAddress;
        m_FaultingAddress = reinterpret_cast<uintptr_t>(addr);

        DWORD code = info->ExceptionRecord->ExceptionCode;
        const wchar_t* desc = ExceptionDescribe(code);

        if (m_FaultingModule = ModuleFromAddress(reinterpret_cast<uintptr_t>(addr)))
        {
            if (!PutFileFmt(L"Faulting module: %s\n", GetModuleName(wtmpbuf, std::size(wtmpbuf), m_FaultingModule)))
                return false;
        }

        if (desc && !PutFileFmt(L"Unhandled %s error at 0x%p\n", desc, addr))
            return false;
        if (!desc && !PutFileFmt(L"Unhandled exception 0x%x at 0x%p\n", code, addr))
            return false;

        if (!PutFileFmt(L"Exception information:\n"))
            return false;

        if (!PutFileFmt(L"\tcode = 0x%" PRIx32 "\n", info->ExceptionRecord->ExceptionCode))
            return false;

        if (!PutFileFmt(L"\tflags = 0x%" PRIx32 "\n", info->ExceptionRecord->ExceptionFlags))
            return false;

        DWORD numArgs = info->ExceptionRecord->NumberParameters;
        if (numArgs)
        {
            numArgs = min(numArgs, EXCEPTION_MAXIMUM_PARAMETERS);
            for (int i = 0; i < numArgs; i++)
            {
                ULONG_PTR exceptionInfo = info->ExceptionRecord->ExceptionInformation[i];
                if (!PutFileFmt(L"\tp%d = 0x%p\n", i, (void*)exceptionInfo))
                    return false;
            }
        }

        if (!PutFile(L"\n\nStack trace:\n"))
            return false;

        if (!WriteBacktrace(info->ContextRecord))
            return false;

        if (!PutFile(L"\n\nLoaded modules:\n"))
            return false;

        if (!EnumerateLoadedModulesEx(
            GetCurrentProcess(),
            LoadedModuleCallback,
            this
        ))
        {
            if (!PutFileFmt(L"Failed to enumerate modules. GetLastError() = %d\n", GetLastError()))
                return false;
        }

        return PutFile(L"\n");
    }

    const wchar_t* GetModuleName(wchar_t* wbuf, int wbuflen, HMODULE module, bool extractFileName = false)
    {
        DWORD status = GetModuleFileNameW(module, wbuf, wbuflen);
        if (status && extractFileName)
        {
            int len = std::wcslen(wbuf);
            // Walk backward until we find a path separator
            for (int i = len; i > 0; i--)
            {
                if (wbuf[i] == L'/' || wbuf[i] == L'\\')
                {
                    // Path ends with separator
                    if (i == len)
                        return L"???";

                    return &wbuf[i + 1];
                }
            }
        }
        return status ? wbuf : L"???";
    }

    bool GetModuleRelPath(wchar_t* buf, size_t bufLen, const wchar_t* filename)
    {
        HRESULT res = StringCchPrintfW(
            buf,
            bufLen,
            L"%s\\%s",
            Base::GetModuleDir().wstring().c_str(),
            filename
        );

        return res == S_OK;
    }

    bool DumpThreadContext(CONTEXT* context)
    {
        if (!PutFile(L"Thread context:\n"))
            return false;

#define PRINT_REGISTER(name, spec) if (!PutFileFmt(L#name L" = 0x%" spec L"\n", context->##name)) return false;

#ifdef _M_AMD64
        PRINT_REGISTER(Rax, PRIx64);
        PRINT_REGISTER(Rcx, PRIx64);
        PRINT_REGISTER(Rdx, PRIx64);
        PRINT_REGISTER(Rbx, PRIx64);
        PRINT_REGISTER(Rsp, PRIx64);
        PRINT_REGISTER(Rbp, PRIx64);
        PRINT_REGISTER(Rsi, PRIx64);
        PRINT_REGISTER(Rdi, PRIx64);
        PRINT_REGISTER(R8, PRIx64);
        PRINT_REGISTER(R9, PRIx64);
        PRINT_REGISTER(R10, PRIx64);
        PRINT_REGISTER(R11, PRIx64);
        PRINT_REGISTER(R12, PRIx64);
        PRINT_REGISTER(R13, PRIx64);
        PRINT_REGISTER(R14, PRIx64);
        PRINT_REGISTER(R15, PRIx64);

        PRINT_REGISTER(Rip, PRIx64);
#else
        PRINT_REGISTER(Eax, PRIx32);
        PRINT_REGISTER(Ecx, PRIx32);
        PRINT_REGISTER(Edx, PRIx32);
        PRINT_REGISTER(Ebx, PRIx32);
        PRINT_REGISTER(Esp, PRIx32);
        PRINT_REGISTER(Ebp, PRIx32);
        PRINT_REGISTER(Esi, PRIx32);
        PRINT_REGISTER(Edi, PRIx32);

        PRINT_REGISTER(Eip, PRIx32);
#endif

#undef PRINT_REGISTER

        return true;
    }

    bool WriteMiniDump(const wchar_t* path, _EXCEPTION_POINTERS* exceptionInfo, bool fat)
    {
        HANDLE hDumpFile = CreateFileW(
            path,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hDumpFile == nullptr || hDumpFile == INVALID_HANDLE_VALUE)
            return false;

        int type = 
            MiniDumpNormal 
            | MiniDumpWithModuleHeaders
            | MiniDumpWithThreadInfo
            | MiniDumpWithHandleData
            | MiniDumpWithCodeSegs
            | MiniDumpWithDataSegs;

        if (fat)
        {
            type |= 
                MiniDumpWithFullMemory;
        }

        MINIDUMP_EXCEPTION_INFORMATION dumpExInfo;
        dumpExInfo.ThreadId = GetCurrentThreadId();
        dumpExInfo.ExceptionPointers = exceptionInfo;
        dumpExInfo.ClientPointers = TRUE;

        BOOL status = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hDumpFile,
            (MINIDUMP_TYPE) type,
            &dumpExInfo,
            NULL,
            NULL
        );

        CloseHandle(hDumpFile);
        return status;
    }

    static BOOL WINAPI LoadedModuleCallback(PCWSTR moduleName, DWORD64 moduleBase, ULONG moduleSize, PVOID userContext)
    {
        CrashWriter* w = (CrashWriter*)userContext;

        bool status = true;

        if (status && w->m_FaultingModule == (PVOID)moduleBase)
            status = w->PutFileFmt(L"---> ");

        if (status) status = w->PutFileFmt(L"%s @ 0x%p\n", moduleName, (PVOID)moduleBase);

        return status;
    }

    static int GetFrames(CONTEXT* ctx, uintptr_t* addrs, int max)
    {
        return RtlCaptureStackBackTrace(3, max, (void**)addrs, nullptr);
    }

    void FmtAddressWithSymbol(wchar_t* buf, size_t bufLen, const wchar_t* optModName, uintptr_t addr, const SymbolAndName* san)
    {
        optModName = optModName ? optModName : L"???";
        if (san->Symbol.Address)
        {
            ptrdiff_t offset = addr - san->Symbol.Address;
            if (offset >= 0)
                swprintf(buf, bufLen, L"%s.dll!%s+0x%" PRIxPTR, optModName, san->Symbol.Name, offset);
            else
                swprintf(buf, bufLen, L"%s.dll!%s-0x%" PRIxPTR, optModName, san->Symbol.Name, -offset);
        }
        else
            swprintf(buf, bufLen, L"%s.dll!0x%p", optModName, (void*) addr);
    }

    bool WriteFrame(uintptr_t modAddr, uintptr_t addr, const wchar_t* maybeModName, const SymbolAndName* san)
    {
        const wchar_t* modName = maybeModName ? maybeModName : L"???";

        wchar_t buf[128] = {};

        if (modAddr)
            swprintf_s(buf, L"%s.dll+0x%" PRIxPTR, modName, addr - modAddr);
        else
            swprintf_s(buf, L"0x%" PRIxPTR " (%s.dll)", addr, modName);

        bool status = PutFileFmt(L"%s%-48s", addr == m_FaultingAddress ? L"---> " : L"", buf);

        if (status)
        {
            FmtAddressWithSymbol(buf, std::size(buf), modName, addr, san);
            status = PutFileFmt(L"%s", buf);
        }

        return status;
    }


    bool GetSymbolForAddress(HANDLE process, uintptr_t addr, SymbolAndName* outSAN)
    {
        ZeroMemory(outSAN, sizeof(*outSAN));

        SYMBOL_INFOW* symbol = &outSAN->Symbol;
        symbol->SizeOfStruct = sizeof(*symbol);
        symbol->MaxNameLen = 256;

        return SymFromAddrW(process, addr, nullptr, symbol);
    }

    bool DumpFrame(HANDLE process, uintptr_t addr)
    {
        SymbolAndName san;
        if (!GetSymbolForAddress(process, addr, &san))
            ZeroMemory(&san, sizeof(san));

        IMAGEHLP_MODULEW m{};
        m.SizeOfStruct = sizeof(m);

        if (!SymGetModuleInfoW(process, addr, &m))
            ZeroMemory(&m, sizeof(m));

        HMODULE moduleBase = san.Symbol.ModBase ? 
            reinterpret_cast<HMODULE>(san.Symbol.ModBase) 
            : ModuleFromAddress(addr);

        if (!WriteFrame(reinterpret_cast<uintptr_t>(moduleBase), addr, m.ModuleName, &san))
            return false;

        /* This function only works for .pdb debug info anyways */
        /* This function is also missing on Windows 9X */
#if _MSC_VER
        IMAGEHLP_LINE line{};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
        DWORD lineOffset;
        if (SymGetLineFromAddr(process, addr, &lineOffset, &line))
        {
            // TODO
#ifndef _M_AMD64
            wchar_t wfilename[512] = { };

            int len = std::strlen(line.FileName);

            if (len + 1 < std::size(wfilename))
            {
                for (int i = 0; i < len + 1; i++)
                    wfilename[i] = line.FileName[i];
            }

            if (!PutFileFmt(L"  %s:%d", wfilename, line.LineNumber))
                return false;
#else
            if (!PutFileFmt(L"  %s:%d", line.FileName, line.LineNumber))
                return false;
#endif
        }
#endif

        return PutFile(L"\n");
    }

    HMODULE ModuleFromAddress(uintptr_t address)
    {
        HMODULE module = NULL;

        BOOL status = GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            reinterpret_cast<LPCSTR>(address),
            &module
        );

        return status ? module : NULL;
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

    bool PutFileFmt(PRINTF_FORMAT_STRING const wchar_t* fmt, ...)
    {
        wchar_t wbuf[512];

        va_list va;
        va_start(va, fmt);
        HRESULT result = StringCchVPrintfW(wbuf, std::size(wbuf), fmt, va);
        va_end(va);

        if (result != S_OK)
            return false;

        size_t wlen = 0;
        if (StringCchLengthW(wbuf, std::size(wbuf), &wlen) != S_OK)
            return false;

        int utf8Len = 0;
        const char* utf8 = ToUTF8(wbuf, &utf8Len);

        return utf8 && WriteFile(m_hFile, utf8, utf8Len, nullptr, nullptr) != 0;
    }

    bool PutFile(const wchar_t* str)
    {
        int utf8Len = 0;
        const char* utf8 = ToUTF8(str, &utf8Len);

        return utf8 && WriteFile(m_hFile, utf8, utf8Len, nullptr, nullptr) != 0;
    }

    const char* ToUTF8(const wchar_t* wc, int* outLen)
    {
        int bytesRequired = WideCharToMultiByte(CP_UTF8, 0, wc, -1, NULL, 0, NULL, NULL);

        if (bytesRequired <= 0 || bytesRequired > std::size(m_UTF8Buf))
        {
            assert(false);
            return nullptr;
        }

        int bytesConverted = WideCharToMultiByte(
            CP_UTF8,
            0,
            wc,
            -1,
            m_UTF8Buf,
            bytesRequired,
            NULL,
            NULL
        );

        assert(bytesRequired == bytesConverted);

        *outLen = bytesConverted - 1;
        return m_UTF8Buf;
    }

    HMODULE m_FaultingModule = NULL;

    HANDLE m_hFile = INVALID_HANDLE_VALUE;
    char m_UTF8Buf[512] = {};
    uintptr_t m_FaultingAddress = NULL;
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
