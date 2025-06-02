#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <filesystem>
#undef DrawText // Thanks Windows.h

#define FATAL(...) Base::Fatal("Fatal error in " __FUNCTION__, __VA_ARGS__)

enum class EAppID
{
	HL2 = 220,
	CSS = 240,
	HL2DM = 320,
	Portal = 400,
	TF2 = 440,
	L4D = 500,
	L4D2 = 550,
	Portal2 = 620,
	CSGO = 730,
	GMOD = 4000,
};

inline bool operator==(const int Value, EAppID AppID)
{
	return static_cast<int>(AppID) == Value;
}

namespace Base
{
	void OnAttach(HMODULE Module);
	void OnDetach();
	DWORD WINAPI HookThread(LPVOID Args);

	// Use these if failure to get a handle is not an option
	HMODULE GetModule(const char* Module);
	FARPROC GetProc(HMODULE Module, const char* Proc);
	void Fatal(const char* Title, const char* Format, ...);
	/**
	 * @brief The directory containing our module.
	 * 
	 * It's encouraged to store configs, logs, and other files here.
	 * The path may be blank if depending on injection method.
	 */
	const std::filesystem::path& GetModuleDir();

	inline HMODULE hInst;
	inline HWND hWnd;

	inline std::wstring CommandLineW;
	inline std::vector<std::wstring> CommandLineArgsW;

	bool CommandLineContains(std::wstring_view needle);
	const std::wstring* GetCommandLineParam(std::wstring_view paramName);

#ifdef _WIN64
	constexpr bool Win64 = true;
#else
	constexpr bool Win64 = false;
#endif
};