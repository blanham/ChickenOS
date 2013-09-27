#ChickenOS ARM entry point
#Sets up stack and (basic mmu mapping?) and (sets up multiboot like commandline?)
.global _loader

.equ STACK_SIZE, 0x4000

.text
.align 4
.arm
_loader:
	#set up stack
	#either build fake multiboot struct or set the magic number to something special
	b kmain

.bss
.global stack
stack:
   .fill STACK_SIZE, 1, 0 
