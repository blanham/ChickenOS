Architecture specific functions/hardware abstraction layer (architecture abstraction layer?)

Archs I want to support: i386 (would be cool to run on older hardware), x86_64, arm (BBB or raspi), m68k (either NexT or mac hardware), userspace? (not sure how to do threading for one)

entry: takes arch specific boot params and parses them into a common format (for some archs parts of this could be hardcoded, eg, memory map)
paging functions:
	init paging/turn on mmu/etc
	page maping/unmapping functions (will need to abstract read/write/execute permissions)
	page lookup
	page invalidate
	pagedir set/get


thread switch
basic console output: this could be directed to vga, serial, etc
low level serial interface
keyboard/mouse
port level i/o - GPIO (Might actually abstract this)
interrupts/exceptions
	page fault handling? 

time
syscall handler -> registers_t (renamed CPUContext) can have an impl that gives you the arguments, stack pointer, etc

module structure:

arch/
define high level stuff here

i386/
entry.s
mod.rs -> This contains entry(...)
io.rs -> could either define 
interrupt.rs
interrupt.s
paging.rs -> impls for PAddr/VAddr, paging functions, etc
thread.rs -> stackframe setup, thread_switch (handles arch specific operations that are not assembly, eg set TSS, handle signals)
switch.s -> thread switching code
console.rs
time.rs
serial.rs

platform/ -> defines platform/SoC specific stuff?

