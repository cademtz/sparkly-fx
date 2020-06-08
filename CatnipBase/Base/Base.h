#pragma once

#define FATAL(Msg) __FUNCTION__ ": " Msg

namespace Base
{
	void OnAttach(HMODULE Module);

	void Fatal(const char* Title, const char* Format, ...);

	inline HMODULE hInst;
};