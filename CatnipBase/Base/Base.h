#pragma once

#include <Windows.h>

#define FATAL(Msg) __FUNCTION__ ": " Msg

namespace Base
{
	void OnAttach(HMODULE Module);

	void Fatal(const char* Title, const char* Format, ...);

	inline HMODULE hInst;
};