//! # ChickenOS - x86 (32 and 64 bit)
//!

pub mod io;

mod multiboot;



use arch::boot::{BootParams, HardwareMemoryRegion};
use self::multiboot::{MultibootInfo, MultibootMmapEntry, MULTIBOOT_BOOTLOADER_MAGIC, MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT};
use ::rust_kmain;

//struct Book {
//    a: u32
//}
//fn booker(book: &Book) {
//   // println("Book {}", book)
//   unsafe { temp_print_u32(book.a) }
//}
#[no_mangle]
pub extern fn rust_entry(mb_info: &MultibootInfo, magic: u32)  {
    unsafe { serial_init(); };

    //booker(& {a: 5u32});

    if let Some(boot_info) = multiboot_parse(mb_info, magic) {
        rust_kmain(boot_info);
    } else {
        // If boot_info is None spin, dump message on serial line + vga
        loop { halt() };
    }

    unreachable!();
}

pub fn halt() {
    unsafe { asm!("hlt") }
}

extern {
    static end: usize;

    fn temp_print_u32(value: u32);
    fn print_memory_region(region: *const HardwareMemoryRegion);
    // remove this when I migrate serial code
    fn serial_init();
    // Remove this when I start using rlibc
    fn memcpy(dest: *mut u8, src: *const u8, n: usize) -> *mut u8;
    // Remove this when I start using rcstring
    fn strlen(string: *const u8) -> usize;
}

/// Parse the multiboot header and store the info in a BootParams.
/// TODO:
/// Modules should be moved to either the beginning or end of physical memory
pub fn multiboot_parse(mb_info: &MultibootInfo, magic: u32) -> Option<&BootParams> {
    if magic != MULTIBOOT_BOOTLOADER_MAGIC  { return None; }

    let mut kernel_end_address = unsafe {
        &end as *const usize as usize
    };

    kernel_end_address = (kernel_end_address & !PAGE_SIZE) + PAGE_SIZE;

    unsafe {
        let boot_params = boot_alloc(::core::mem::size_of::<BootParams>()) as *mut BootParams;

        // Ugly
        let cmdline = mb_info.cmdline.to_v().0 as *const u8;
        let cmdline_length = strlen(cmdline) + 1;

        (*boot_params).cmdline = boot_alloc(cmdline_length) as *mut u8;
        memcpy((*boot_params).cmdline, cmdline, cmdline_length);

        let mut mmap_addr = mb_info.mmap_addr.to_v().0 as usize;
        let mmap_end = mmap_addr + mb_info.mmap_length as usize;

        let mut mem_size = 0 as usize;

        while mmap_addr < mmap_end {
            let entry = mmap_addr as *const MultibootMmapEntry;
            let mut memory_map_entry = boot_alloc(::core::mem::size_of::<HardwareMemoryRegion>()) as *mut HardwareMemoryRegion;
            if (*boot_params).regions.is_null() {
                (*boot_params).regions = memory_map_entry as *mut HardwareMemoryRegion;
            }

            if (*entry).size == 0 { break; }

            (*memory_map_entry).addr = (*entry).addr;
            (*memory_map_entry).len = (*entry).len;
            // This should be an enum at some point
            (*memory_map_entry)._type = (*entry)._type;

            // (Stupidly) sets avialable memory size to the size of the largest free block
            if (*memory_map_entry).len as usize > mem_size { mem_size = (*memory_map_entry).len as usize; }

            mmap_addr += (*entry).size as usize + 4;
        }

        // Does not take into account memory occupied by kernel
        (*boot_params).mem_size = mem_size;

        (*boot_params).placement = kernel_end_address;

        if (*mb_info).framebuffer_is_ega() {
            (*boot_params).x_res = 80;
            (*boot_params).y_res = 25;
        } else {
            //handle VBE parsing later
        }

        boot_params.as_ref()
    }
}

/// Simple placement allocator. This is neccessary as Grub 2 places multiboot
/// structures and modules at high address instead of directly after the kernel.
/// `boot_scratchpad` is the address of a block of 4*PAGE_SIZE bytes, defined
/// in the entry assembly file
unsafe fn boot_alloc(size: usize) -> *mut usize {
    extern { static mut boot_scratchpad: usize; };

    let ret = boot_scratchpad;

    boot_scratchpad += size;

    ret as *mut usize
}


#[cfg(target_arch="x86")]
mod i386;
#[cfg(target_arch="x86")]
pub use self::i386::*;

#[cfg(target_arch="x86_64")]
mod x86_64;
#[cfg(target_arch="x86_64")]
pub use self::x86_64::*;
