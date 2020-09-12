; https://stackoverflow.com/questions/12659972/distinguish-between-x64-and-x86-mode-in-masm
; Whatever works for now

IFDEF RAX
    END_IF_NOT_X64 equ <>
ELSE
    END_IF_NOT_X64 equ end  ; stop the assembler if we're not compiling for x64
ENDIF

END_IF_NOT_X64

_text segment

GetR14 proc
	mov rax, r14
	ret
GetR14 endp

SetR14 proc
	mov r14, rcx
	ret
SetR14 endp

GetB proc
	mov rax, rbx
	ret
GetB endp

SetB proc
	mov rbx, rcx
	ret
SetB endp

GetBP proc
	mov rax, rbp
	ret
GetBP endp

SetBP proc
	mov rbp, rcx
	ret
SetBP endp

_text ends

end