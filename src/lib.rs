//! # ChickenOS - A kernel in C and Rust

#![no_std]

#![feature(lang_items, compiler_builtins_lib, core_intrinsics)]
#![feature(libc)]
#![feature(asm)]

//XXX: Remove this
#![allow(dead_code)]

// Used for types
//
//extern crate libc;
extern crate compiler_builtins;

use core::intrinsics;

pub mod arch;
//pub mod device;
pub mod external;


use arch::boot::BootParams;

//u386 assembly entry.s calls i386_boot() rust function
//which initiliazes the serial console (and maybe the screen?), parses multiboot into kernel boot
//into and then calls rust_kmain(boot_info)

extern {
    //fn multiboot_parse(mb: *const i32, info: *const i32) -> *const i32;
    fn kmain(info: *const BootParams);
    //vm_init
    //thread_init
    //interrupt_init
    //if cfg!(arch = x86) {
    //pci_init
    //}
    //
    //video_init
    //
    //tty_init
    //
    //println!(boot_msg)
    //
    //kbd_init
    //
    //time_init
    //
    //interrupt_enable?
    //
    //ata_init -> block_init?
    //
    //vfs_init
    //
    //network_init -> this should happen before the vfs?
    //
    //mount_root
    //
    //thread_create
    //
    //kernel_halt
}


#[no_mangle]
pub extern fn rust_kmain(info: &BootParams) {

    unsafe { kmain(info); }

    kernel_halt();
}

pub fn kernel_halt() -> ! {
    loop {
        arch::halt()
    }
}


//Maybe more like this:
//#[repr(C)]
//pub struct arch_boot_info;
//#[no_mangle]
//pub extern fn rust_entry(boot_info: *const arch_boot_info) {
//    //let kernel_boot_info = arch::boot::boot(arch_boot_info);
//
//    //kmain(kernel_boot_info);
//}
//
////Move this to multiboot module
//#[no_mangle]
//pub extern fn rust_multiboot(mb: *const i32, info: *const i32) -> *const i32 {
//
//    arch::boot::boot(mb, info);
//
//    unsafe {
//    return multiboot_parse(mb, info);
//    }
//}
//

// These functions are used by the compiler, but not
// for a bare-bones hello world. These are normally
// provided by libstd.
#[lang = "eh_personality"]
#[no_mangle]
pub extern fn rust_eh_personality() {
}

// This function may be needed based on the compilation target.
#[lang = "eh_unwind_resume"]
#[no_mangle]
pub extern fn rust_eh_unwind_resume() {
}

#[lang = "panic_fmt"]
#[no_mangle]
pub extern fn rust_begin_panic(_msg: core::fmt::Arguments,
                               _file: &'static str,
                               _line: u32,
                               _column: u32)
                               -> !
{
    unsafe { intrinsics::abort() }
}
