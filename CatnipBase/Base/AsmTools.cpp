#include "AsmTools.h"
#include <cstddef>

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
