#include "AsmTools.h"
#include "Base.h"
#include <cstddef>
#include <vector>
#include <intrin.h>

#ifndef _DEBUG
#define ZYDIS_DISABLE_FORMATTER
#endif

#define Zydis_EXPORTS // Some hacky crap because lazey
#include <Zydis/Zydis.h>
#include <inttypes.h>

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

bool AsmTools::AnalyzeStackBeepBoop(StackSnapshot* Snap, const void* YourFunc)
{
	ZeroMemory(Snap, sizeof(*Snap));
	Snap->base = ((StackBase*)GetBP())->caller_base;

	ZyanI64 off = 0;
	ZyanStatus status;
	ZydisDecoder decoder;
	ZydisDecodedInstruction ins;
	ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

#ifdef _DEBUG
	ZydisFormatter formatter;
	ZydisFormatterInit(&formatter, ZydisFormatterStyle::ZYDIS_FORMATTER_STYLE_INTEL);
#endif

	if constexpr (Base::Win64)
		status = ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
	else
		status = ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_STACK_WIDTH_32);

	if (ZYAN_FAILED(status))
		return false;

	int bp_off = sizeof(void*);

	while (ZYAN_SUCCESS(status = ZydisDecoderDecodeFull(&decoder, (const char*)YourFunc + off, 0xFF, &ins, operands)))
	{
#ifdef _DEBUG
		// Print current instruction pointer.
		printf("%016" PRIX64 "  ", (ZyanU64)YourFunc + off);

		// Format & print the binary instruction structure to human readable format
		char buffer[256];
		ZydisFormatterFormatInstruction(&formatter, &ins, operands, ins.operand_count_visible, buffer, sizeof(buffer), 0, nullptr);
		puts(buffer);
#endif

		EStackRegIndex id = RegIndex_Count;
		int viscount = 0;
		for (int i = 0; i < ins.operand_count; i++)
			viscount += operands[i].visibility == ZYDIS_OPERAND_VISIBILITY_EXPLICIT;

		bool ret = false;
		switch (ins.mnemonic)
		{
		case ZYDIS_MNEMONIC_PUSH:
			if (viscount == 1)
			{
				auto& op = operands[0];
				if (op.type == ZYDIS_OPERAND_TYPE_REGISTER) // Pushing register
				{
					switch (op.reg.value)
					{
					case ZYDIS_REGISTER_RAX:
					case ZYDIS_REGISTER_EAX:
					case ZYDIS_REGISTER_AX:
					case ZYDIS_REGISTER_AH:
					case ZYDIS_REGISTER_AL:
						id = RegIndex_A;
						break;
					case ZYDIS_REGISTER_RBX:
					case ZYDIS_REGISTER_EBX:
					case ZYDIS_REGISTER_BX:
					case ZYDIS_REGISTER_BH:
					case ZYDIS_REGISTER_BL:
						id = RegIndex_B;
						break;
					case ZYDIS_REGISTER_RCX:
					case ZYDIS_REGISTER_ECX:
					case ZYDIS_REGISTER_CX:
					case ZYDIS_REGISTER_CH:
					case ZYDIS_REGISTER_CL:
						id = RegIndex_C;
						break;
					case ZYDIS_REGISTER_RDX:
					case ZYDIS_REGISTER_EDX:
					case ZYDIS_REGISTER_DX:
					case ZYDIS_REGISTER_DH:
					case ZYDIS_REGISTER_DL:
						id = RegIndex_D;
						break;
					case ZYDIS_REGISTER_RSI:
					case ZYDIS_REGISTER_ESI:
					case ZYDIS_REGISTER_SI:
					case ZYDIS_REGISTER_SIL:
						id = RegIndex_SI;
						break;
					case ZYDIS_REGISTER_RDI:
					case ZYDIS_REGISTER_EDI:
					case ZYDIS_REGISTER_DI:
					case ZYDIS_REGISTER_DIL:
						id = RegIndex_DI;
						break;
					}
				}
			}
		case ZYDIS_MNEMONIC_PUSHA:
		case ZYDIS_MNEMONIC_PUSHAD:
		case ZYDIS_MNEMONIC_PUSHF:
		case ZYDIS_MNEMONIC_PUSHFD:
			bp_off -= sizeof(int);
			break;
		case ZYDIS_MNEMONIC_POP:
		case ZYDIS_MNEMONIC_POPA:
		case ZYDIS_MNEMONIC_POPAD:
		case ZYDIS_MNEMONIC_POPCNT:
		case ZYDIS_MNEMONIC_POPF:
		case ZYDIS_MNEMONIC_POPFD:
		case ZYDIS_MNEMONIC_POPFQ:
			bp_off += sizeof(int);
			break;
		case ZYDIS_MNEMONIC_SUB:
		case ZYDIS_MNEMONIC_ADD:
			if (viscount == 2)
			{
				auto& op0 = operands[0], & op1 = operands[1];
				constexpr ZydisRegister sp_val = Base::Win64 ? ZYDIS_REGISTER_RSP : ZYDIS_REGISTER_ESP;
				if (op0.type == ZYDIS_OPERAND_TYPE_REGISTER && op0.reg.value == sp_val && op1.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
				{
					if (ins.mnemonic == ZYDIS_MNEMONIC_ADD)
						bp_off += op1.imm.value.u;
					else
						bp_off -= op1.imm.value.u;
				}
			}
			break;
		case ZYDIS_MNEMONIC_JMP:
		{
			ZyanU64 addr;
			if (ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&ins, operands, (ZyanU64)YourFunc + off, &addr)))
			{
				if (operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
				{
					off = (ZyanI64)addr - (ZyanI64)YourFunc;
					continue; // Skip the adding of instruction length at the loop's end
				}
			}
			// End loop, because code may flow anywhere following this jump
		}
		case ZYDIS_MNEMONIC_RET:
			ret = true;
			break;
		}

		if (ret)
			break;

		if (id != RegIndex_Count)
		{
			if (bp_off <= Snap->regs[id].off)
			{
				Snap->regs[id].off = bp_off;
				Snap->regs[id].value_old = (UINT_PTR*)((const char*)Snap->base + bp_off);
			}
		}

		off += ins.length;
		if ((const char*)YourFunc + off == _ReturnAddress())
			break;
	}

	return ZYAN_SUCCESS(status);
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
