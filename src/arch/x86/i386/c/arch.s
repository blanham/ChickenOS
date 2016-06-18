[global gdt_flush]
[global tss_flush]
[extern gp]

section .text
align 4
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

tss_flush:
	mov ax, 0x2b
	ltr ax
	ret

