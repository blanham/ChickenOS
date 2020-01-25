;  ChickenOS i386 thread switching code
;
;  According to the SysV i386 ABI:
; "Registers %ebp, %ebx, %edi, %esi, and %esp 'belong' to the calling function.
;  In other words, a called function must preserve these registers' values for
;  its caller."

extern thread_stack_offset

global switch_threads
align 4
;  switch_threads(thread_t *cur, thread_t *new)
switch_threads:
;push regs
	mov eax, [esp + 4]
	mov edx, [esp + 8]
	add eax, [thread_stack_offset]
	add edx, [thread_stack_offset]
	push ebp
	push ebx
	push edi
	push esi

	mov [eax], esp
	mov esp, [edx]

	pop esi
	pop edi
	pop ebx
	pop ebp

	ret

; vim: set syntax=nasm:
