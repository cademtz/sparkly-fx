#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace AsmTools
{
	void* Relative(void* PtrToRel, int Offset = 0);

	template<class Ret = UINT_PTR, class Ptr>
	inline Ret Relative(Ptr PtrToRel, int Offset = 0) {
		return (Ret)Relative((void*)PtrToRel, Offset);
	}
}