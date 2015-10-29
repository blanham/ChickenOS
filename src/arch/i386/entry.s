; ChickenOS i386 entry code
; Patterned after the HigherHalf tutorial on wiki.osdev.org


global entry
extern kmain
extern multiboot_parse

PAGE_SIZE 	equ 4096
STACK_SIZE 	equ PAGE_SIZE
PHYS_BASE 	equ 0xC0000000

;Multiboot constants and flags
MULTIBOOT_MAGIC		equ 0x1BADB002
MULTIBOOT_ALIGN 	equ 0x00000001
MULTIBOOT_MEM		equ 0x00000002
MULTIBOOT_VIDEO		equ 0x00000004
MULTIBOOT_FLAGS 	equ MULTIBOOT_ALIGN + MULTIBOOT_MEM; + MULTIBOOT_VIDEO
MULTIBOOT_CHECKSUM 	equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
;Multiboot VBE Flags
MODE_TYPE 	equ 1	;(0 for linear fb, 1 for text)
MODE_WIDTH 	equ 80	;width, pixels for lfb, chars for text, 0 is default
MODE_HEIGHT equ 25	;height, as above
MODE_DEPTH 	equ 0	;

;Paging constants
PDE_P	equ 0x01
PDE_RW	equ 0x02
PDE_PS	equ 0x80
CR4_4MB equ 0x10
CR0_PGE equ 0x80000000

section .multiboot
align 4
multiboot_header:
	dd MULTIBOOT_MAGIC 		;magic
	dd MULTIBOOT_FLAGS 		;flags
	dd MULTIBOOT_CHECKSUM 	;checksum
	dd 0 ;header_addr
	dd 0 ;load_addr
	dd 0 ;load_end_addr
	dd 0 ;bss_end_addr
	dd 0 ;entry_addr
	dd MODE_TYPE ;mode_type: (0 for linear fb, 1 for ega text)
	dd MODE_WIDTH ;width, pixels for lfb, chars for text, 0 is default
	dd MODE_HEIGHT ;height, ass above
	dd MODE_DEPTH ;depth

section .text
align 4
entry:
	;set up identity paging
	;First put physical address of initial PD into cr3
	mov ecx, (initial_pagedirectory - PHYS_BASE)
	mov cr3, ecx
	;turn on 4MB paging by setting PSE bit in cr4
	mov ecx, cr4
	or ecx, CR4_4MB
	mov cr4, ecx
	;Enable paging :)
	mov ecx, cr0
	or ecx,  CR0_PGE
	mov cr0, ecx
	;Has to be an absolute jmp
	lea ecx, [higher_half]
	jmp ecx

higher_half:
	;unmap identity mapping
    mov dword [initial_pagedirectory], 0
    invlpg [0]
	;invalidate that page
	;Switch to known stack
	mov esp, initial_stack_start
	;push multiboot magic number
	push eax
	;ebx contains address of multiboot info
	;Convert to virtual address
	add ebx, PHYS_BASE
	push ebx
	;multiboot_parse takes info struct
	call multiboot_parse
	add esp, 4
	;multiboot_parse returns pointer to boot_info struct
	push eax
	call kmain
failure:
	;If we return
	hlt
	jmp failure

section .data
align 4096 
initial_pagedirectory:
	dd PDE_PS + PDE_RW + PDE_P	
	times (767) dd 0
	dd PDE_PS + PDE_RW + PDE_P	
	times (1024 - 767) dd 0

section .bss
initial_stack:
	resb STACK_SIZE
initial_stack_start:

; vim: set syntax=nasm: 
