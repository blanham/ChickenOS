
pub const COM1: u16 = 0x3F8;
pub const COM2: u16 = 0x2F8;
pub const COM3: u16 = 0x3E8;
pub const COM4: u16 = 0x2E8;

pub const DLLB: u8 = 0;
pub const IER:  u8 = 1;
pub const DLHB: u8 = 1;
pub const IIR: u8 = 2;
pub const FCR: u8 = 2;
pub const LCR: u8 = 3;
pub const MCR: u8 = 4;
pub const LSR: u8 = 5;
pub const MSR: u8 = 6;
pub const SCRATCH: u8 = 7;

pub const DLAB: u8 = 0x80;
fn serial_init() {

}
