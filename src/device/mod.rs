use arch;

struct IOPort {
    address: u32
}

impl IOPort {
    unsafe fn out8(&self, value: u8) {
        unsafe {  arch::io::out8(self.address, value) }
    }

    unsafe fn out16(&self, value: u16) {
        unsafe {  arch::io::out16(self.address, value) }
    }
}


