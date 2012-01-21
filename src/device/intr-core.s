;ChickenOS Interrupt Core ASM
;based off of Bran and JamesM's tutorials
;and the pintos teaching os
%macro ISR_NO_ARGS 1
	[GLOBAL isr%1]
	isr%1:
		;cli
		push byte 0
		push %1
		jmp _isr_handler
%endmacro

%macro ISR_ARGS 1
	[GLOBAL isr%1]
	isr%1:
		;cli
		push %1
		jmp _isr_handler
%endmacro

%macro IRQ 1
	[GLOBAL irq%1]
	irq%1:
		;cli
		push byte 0
		push (%1+32)
		jmp _isr_handler
%endmacro

ISR_NO_ARGS 0
ISR_NO_ARGS 1
ISR_NO_ARGS 2
ISR_NO_ARGS 3
ISR_NO_ARGS 4
ISR_NO_ARGS 5
ISR_NO_ARGS 6
ISR_NO_ARGS 7
ISR_ARGS 	8
ISR_NO_ARGS 9
ISR_ARGS 	10
ISR_ARGS 	11
ISR_ARGS 	12
ISR_ARGS 	13
ISR_ARGS 	14
ISR_NO_ARGS	15
ISR_NO_ARGS 16
ISR_NO_ARGS 17
ISR_NO_ARGS 18
ISR_NO_ARGS 19
ISR_NO_ARGS 20
ISR_NO_ARGS 21
ISR_NO_ARGS 22
ISR_NO_ARGS 23
ISR_NO_ARGS 24
ISR_NO_ARGS 25
ISR_NO_ARGS 26
ISR_NO_ARGS 27
ISR_NO_ARGS 28
ISR_NO_ARGS 29
ISR_NO_ARGS 30
ISR_NO_ARGS 31
;IRQS:
IRQ 0
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQ 8
IRQ 9
IRQ 10
IRQ 11
IRQ 12
IRQ 13
IRQ 14
IRQ 15


[GLOBAL sysc]
sysc:
	;cli
	push byte 0
	push 0x80
	jmp _isr_handler


[GLOBAL intr_return]
[extern interrupt_handler]
_isr_handler:
	sti	;reenable interrupts
	push gs
	push fs
	push es
	push ds
	pusha
	;mov ax, ds
	;push eax
	;have to setup kernel environment
	cld
	mov eax, 0x10
	mov ds, eax
	mov es, eax
	lea ebp, [esp+52]	

	mov eax, esp
	sub eax, 8
	push eax

;	mov eax, esp
;	push eax
;	push format
;	call printf
;	add esp, 8
	

	push esp
	

	call interrupt_handler 
	pop eax
	
	add esp,4
	popa
	pop ds
	pop es
	pop fs
	pop gs
	add esp,8;jump ahead of error code/interrupt number
	;sti
	iret	
[extern dump_regs]

format db "out %X",10,0
[extern printf]

intr_return:
	
	add esp,4	

	add esp,4

	
	popa
	pop ds
	pop es
	pop fs
	pop gs
	add esp,8;jump ahead of error code/interrupt number
	;
;	mov eax, esp
;	push eax
;	push format
;	call printf
;	add esp, 8
	;


	iretd	


[GLOBAL get_eip]
get_eip:
	;pop eax

	;jmp eax
	mov eax, dword[esp]
	ret


[GLOBAL idt_flush]    ; Allows the C code to call idt_flush().
idt_flush:
   mov eax, [esp+4]  ; Get the pointer to the IDT, passed as a parameter.
   lidt [eax]        ; Load the IDT pointer.
   ret

;this stuff is old
[extern syscall_handler]
[GLOBAL syscall_isr]
syscall_isr:
	push eax
	call syscall_handler
	pop eax	
	iret	

