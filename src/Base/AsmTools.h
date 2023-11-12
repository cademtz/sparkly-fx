#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct StackBase;
struct StackReg;
struct StackSnapshot;

enum EStackRegIndex
{
	RegIndex_A,
	RegIndex_B,
	RegIndex_C,
	RegIndex_D,
	RegIndex_SI,
	RegIndex_DI,
	RegIndex_Count,
};

struct StackBase {
	StackBase* caller_base; // Base of caller, [BP]
};

struct StackReg {
	//UINT_PTR value;		// Current value
	INT_PTR off;			// Offset from base
	UINT_PTR* value_old;	// Pointer to old value, nullptr if none.
};

struct StackSnapshot
{
	StackBase* base;				// Current base, BP
	StackReg regs[RegIndex_Count];	// All register values
};

namespace AsmTools
{
	void* Relative(void* PtrToRel, int Offset = 0);

	template<class Ret = UINT_PTR, class Ptr>
	inline Ret Relative(Ptr PtrToRel, int Offset = 0) {
		return (Ret)Relative((void*)PtrToRel, Offset);
	}

	extern "C"
	{
		void* GetR14();
		void SetR14(void* Value);

		void* GetB();
		void SetB(void* Value);
		void* GetBP();
		void SetBP(void* Value);
	}
}