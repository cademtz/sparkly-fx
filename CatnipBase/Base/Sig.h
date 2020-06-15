#pragma once
#include "Base.h"

namespace Sig
{
	// - Searches for 'Count' duplicate bytes within region
	UINT_PTR FindDup(UINT_PTR Start, size_t Len, BYTE Val, size_t Count);
	UINT_PTR FindPattern(UINT_PTR Start, UINT_PTR End, const char* Pattern);
	UINT_PTR FindPattern(const char* Module, const char* Pattern);
	UINT_PTR FindCave(const char* Module, size_t Len);
	UINT_PTR GetBaseOfCode(HMODULE Module);
	DWORD GetSizeOfCode(HMODULE Module);

	// - Searches for 'Count' padding bytes (0xCC or 0x00) from given start within 'Len' bytes
	// ** UNSAFE: If count is too low, it may return important code and memory
	template <class T = UINT_PTR, class TStart>
	inline T FindPad(TStart Start, size_t Len, size_t Count) {
		UINT_PTR result = FindDup((UINT_PTR)Start, Len, 0xCC, Count);
		if (!result)
			result = FindDup((UINT_PTR)Start, Len, 0x00, Count);
		return (T)result;
	}
	template <class T = UINT_PTR, class TStart>
	inline T FindPattern(TStart Start, size_t Len, const char* Pattern) {
		return (T)FindPattern((UINT_PTR)Start, (UINT_PTR)Start + Len, Pattern);
	}
	template <class T = UINT_PTR>
	inline T FindPattern(const char* Module, const char* Pattern) {
		return (T)FindPattern(Module, Pattern);
	}
	template <class T = UINT_PTR, class TStart>
	inline T FindDup(TStart Start, size_t Len, BYTE Val, size_t Count) {
		return (T)FindDup((UINT_PTR)Start, Len, Val, Count);
	}

	// - Returns the start of Module's code region. Optionally stores the length in 'Len'
	inline UINT_PTR GetCodeRegion(const char* Module, size_t* Len = nullptr)
	{
		HMODULE hmod = Base::GetModule(Module);
		if (Len) *Len = GetSizeOfCode(hmod);
		return GetBaseOfCode(hmod);
	}
}