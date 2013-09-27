
global _loader                          ; Make entry point visible to linker.
extern kmain                            ; _main is defined elsewhere
extern end
extern _sbss 
; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0             ; align loaded modules on page boundaries
MEMINFO     equ  1<<1             ; provide memory map
VIDEOINFO	equ  1<<2 ;provide video info
FLAGS       equ  MODULEALIGN | MEMINFO; | VIDEOINFO  ; this is the Multiboot 'flag' field
MAGIC       equ    0x1BADB002     ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)  ; checksum required
 
; This is the virtual base address of kernel space. It must be used to convert virtual
; addresses into physical addresses until paging is enabled. Note that this is not
; the virtual address where the kernel image itself is loaded -- just the amount that must
; be subtracted from a virtual address to get a physical address.
KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)  ; Page directory index of kernel's 4MB PTE.
 
 
section .data
align 4096
BootPageDirectory:
    ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB.
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.
    ; This entry must be here -- otherwise the kernel will crash immediately after paging is
    ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0                 ; Pages before kernel space.
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0  ; Pages after the kernel image.
 
section .multiboot 
align 4
MultiBootHeader:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
;	dd 0
;	dd 0
;	dd 0
;	dd 0
;	dd 0
;	dd 0
;	dd 1
;	dd 1
;	dd 1
;	dd 1 
; reserve initial kernel stack space -- that's 16k.
STACKSIZE equ 0x4000
section .text
 
_loader:
    ; NOTE: Until paging is set up, the code must be position-independent and use physical
    ; addresses, not virtual ones!
    mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx                                        ; Load Page Directory Base Register.
 
    mov ecx, cr4
    or ecx, 0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx
 
    mov ecx, cr0
    or ecx, 0x80000000                          ; Set PG bit in CR0 to enable paging.
    mov cr0, ecx
 
    ; Start fetching instructions in kernel space.
    lea ecx, [StartInHigherHalf]
    jmp ecx                                                     ; NOTE: Must be absolute jump!
[global StartInHigherHalf] 
StartInHigherHalf:
    ; Unmap the identity-mapped first 4MB of physical address space. It should not be needed
    ; anymore.
    mov dword [BootPageDirectory], 0
    invlpg [0]
 
    ; NOTE: From now on, paging should be enabled. The first 4MB of physical address space is
    ; mapped starting at KERNEL_VIRTUAL_BASE. Everything is linked to this address, so no more
    ; position-independent code or funny business with virtual-to-physical address translation
    ; should be necessary. We now have a higher-half kernel.
    mov esp, stack+STACKSIZE           ; set up the stack
    push eax                           ; pass Multiboot magic number
 
    ; pass Multiboot info structure -- WARNING: This is a physical address and may not be
    ; in the first 4MB!
    ; 4/8/2013 - add KERNEL_VIRTUAL_BASE so it is a virtual address
	add ebx, KERNEL_VIRTUAL_BASE
	push ebx
 
    call  kmain                  ; call kernel proper
    hlt                          ; halt machine should kernel return
[global gdt_flush] ; make 'gdt_flush' accessible from C code
[extern gp]        ; tells the assembler to look at C code for 'gp'
 
; this function does the same thing of the 'start' one, this time with
; the real GDT
gdt_flush:
	mov eax, [esp+4]
	lgdt [eax]
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov ax, 0x2b
	ltr ax
	jmp 0x08:flush2
 
flush2:
	ret 

[global tss_flush]
tss_flush:
	mov ax, 0x28
	ltr ax
	ret
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

 
;align 4096
[global idt_table]
idt_table:
	resb 4096

section .bss
align 4096
[global stack]
stack:
    resb STACKSIZE      ; reserve 16k stack on a quadword boundary
