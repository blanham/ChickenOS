#![feature(asm)]

struct IOPort {
    port: u16
}

impl IOPort {
}


#[no_mangle]
pub unsafe fn outb(port: u16, value: u8) {
	asm!("outb $0, $1" : : "{al}"(value), "{dx}N"(port));
}

#[no_mangle]
pub unsafe fn outw(port: u16, value: u16) {
	asm!("outw $0, $1" : : "{ax}"(value), "{dx}N"(port));
}

#[no_mangle]
pub unsafe fn outl(port: u16, value: u32) {
	asm!("outl $0, $1" : : "{eax}"(value), "{dx}N"(port));
}

#[no_mangle]
pub unsafe fn inb(port: u16) -> u8 {
	let ret: u8;
	asm!("inb $1, $0" : "={al}"(ret) : "{dx}N"(port));
	ret
}

#[no_mangle]
pub unsafe fn inw(port: u16) -> u16 {
	let ret: u16;
	asm!("inw $1, $0" : "={ax}"(ret) : "{dx}N"(port));
	ret
}

#[no_mangle]
pub unsafe fn inl(port: u16) -> u32 {
	let ret: u32;
	asm!("inl $1, $0" : "={eax}"(ret) : "{dx}N"(port));
	ret
}

pub unsafe fn out8(port: u16, value: u8) {
	outb(port as u16, value)
}

pub unsafe fn out16(port: u16, value: u16) {
	outw(port as u16, value)
}

pub unsafe fn out32(port: u16, value: u32) {
	outl(port as u16, value)
}

pub unsafe fn in8(port: u16) -> u8 {
	inb(port)
}
