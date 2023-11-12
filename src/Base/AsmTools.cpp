#include "AsmTools.h"
#include "Base.h"
#include <cstddef>
#include <vector>
#include <intrin.h>

#ifndef _DEBUG
#define ZYDIS_DISABLE_FORMATTER
#endif

void* AsmTools::Relative(void* PtrToRel, int Offset)
{
	UINT8* ptr = (UINT8*)PtrToRel + Offset;
#ifdef _WIN64
	int offset = *(int*)ptr;
	return ptr + offset + sizeof(offset);
#else
	return *(void**)ptr;
#endif
}

// ONLY 32-bit code follows. Corresponding 64-bit code can be found in "AsmTools64.asm"
#ifndef _WIN64

__declspec(naked) void* AsmTools::GetB()
{
	__asm
	{
		mov eax, ebx
		ret
	}
}
__declspec(naked) void AsmTools::SetB(void* Value)
{
	__asm
	{
		mov ebx, [esp - 4]
		ret
	}
}

__declspec(naked) void* AsmTools::GetBP()
{
	__asm
	{
		mov eax, ebp
		ret
	}
}
__declspec(naked) void AsmTools::SetBP(void* Value)
{
	__asm
	{
		mov ebp, [esp - 4]
		ret
	}
}

#endif
