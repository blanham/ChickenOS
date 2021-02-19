// AArch64 mode

.set PAGE_SIZE, 4096
.set STACK_SIZE, PAGE_SIZE
.section ".text"
.global kernel_thread
.align 8
kernel_thread:
    .int kernel_thread_base

// To keep this in the first portion of the binary.
.section ".text.boot"

.global entry
.org 0x80000
// Entry point for the kernel. Registers:
// x0 -> 32 bit pointer to DTB in memory (primary core only) / 0 (secondary cores)
// x1 -> 0
// x2 -> 0
// x3 -> 0
// x4 -> 32 bit kernel entry point, _start location
entry:
    // set stack before our code
    ldr     x5, =entry
    mov     sp, x5

    // clear bss
    ldr     x5, =_sbss
    ldr     w6, =_ebss
3:  cbz     w6, 4f
    str     xzr, [x5], #8
    sub     w6, w6, #1
    cbnz    w6, 3b

    // jump to C code, should not return
4:  bl      aarch64_main
    // for failsafe, halt this core too
    b 4b

.section ".bss"
.balign 4096
kernel_thread_base:
	.space STACK_SIZE
kernel_thread_top:
