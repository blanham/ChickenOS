
use core::mem::size_of;

use ::arch::PAddr;
use arch::boot::{BootParams, HardwareMemoryRegion};

pub const MULTIBOOT_HEADER_MAGIC: u32 = 0x1BADB002;
pub const MULTIBOOT_BOOTLOADER_MAGIC: u32 = 0x2BADB002;
pub const MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED: u8 = 0;
pub const MULTIBOOT_FRAMEBUFFER_TYPE_RGB: u8 = 1;
pub const MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT: u8 = 2;

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootHeader {
    pub magic: u32,
    pub flags: u32,
    pub checksum: u32,
    pub header_addr: u32,
    pub load_addr: u32,
    pub load_end_addr: u32,
    pub bss_end_addr: u32,
    pub entry_addr: u32,
    pub mode_type: u32,
    pub width: u32,
    pub height: u32,
    pub depth: u32,
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootAoutSymbolTable {
    pub tabsize: u32,
    pub strsize: u32,
    pub addr: u32,
    pub reserved: u32,
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootElfSectionHeaderTable {
    pub num: u32,
    pub size: u32,
    pub addr: u32,
    pub shndx: u32,
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootInfo {
    pub flags: u32,
    pub mem_lower: u32,
    pub mem_upper: u32,
    pub boot_device: u32,
    pub cmdline: PAddr,
    pub mods_count: u32,
    pub mods_addr: u32,
    pub u: MultibootSymbolUnion,
    pub mmap_length: u32,
    pub mmap_addr: PAddr,
    pub drives_length: u32,
    pub drives_addr: u32,
    pub config_table: u32,
    pub boot_loader_name: u32,
    pub apm_table: u32,
    pub vbe_control_info: u32,
    pub vbe_mode_info: u32,
    pub vbe_mode: u16,
    pub vbe_interface_seg: u16,
    pub vbe_interface_off: u16,
    pub vbe_interface_len: u16,
    pub framebuffer_addr: u64,
    pub framebuffer_pitch: u32,
    pub framebuffer_width: u32,
    pub framebuffer_height: u32,
    pub framebuffer_bpp: u8,
    pub framebuffer_type: u8,
    pub _bindgen_data_1_: [u32; 2usize],
}

impl MultibootInfo {
    pub unsafe fn framebuffer_palette_addr(&mut self)
        -> *mut u32 {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_1_);
            ::core::mem::transmute(raw.offset(0))
        }
    pub unsafe fn framebuffer_palette_num_colors(&mut self)
        -> *mut u16 {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_1_);
            ::core::mem::transmute(raw.offset(4))
        }
    pub unsafe fn framebuffer_red_field_position(&mut self)
        -> *mut u8 {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_1_);
            ::core::mem::transmute(raw.offset(0))
        }
    pub unsafe fn framebuffer_red_mask_size(&mut self)
        -> *mut u8 {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_1_);
            ::core::mem::transmute(raw.offset(1))
        }
    pub unsafe fn framebuffer_green_field_position(&mut self)
        -> *mut u8 {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_1_);
            ::core::mem::transmute(raw.offset(2))
        }
    pub unsafe fn framebuffer_green_mask_size(&mut self)
        -> *mut u8 {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_1_);
            ::core::mem::transmute(raw.offset(3))
        }
    pub unsafe fn framebuffer_blue_field_position(&mut self)
        -> *mut u8 {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_1_);
            ::core::mem::transmute(raw.offset(4))
        }
    pub unsafe fn framebuffer_blue_mask_size(&mut self)
        -> *mut u8 {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_1_);
            ::core::mem::transmute(raw.offset(5))
        }

    pub fn framebuffer_is_ega(&self) -> bool {
        self.framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT
    }
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootSymbolUnion {
    pub _bindgen_data_: [u32; 4usize],
}

impl MultibootSymbolUnion {
    pub unsafe fn aout_sym(&mut self) -> *mut MultibootAoutSymbolTable {
        let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_);
        ::core::mem::transmute(raw.offset(0))
    }
    pub unsafe fn elf_sec(&mut self)
        -> *mut MultibootElfSectionHeaderTable {
            let raw: *mut u8 = ::core::mem::transmute(&self._bindgen_data_);
            ::core::mem::transmute(raw.offset(0))
        }
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootColor {
    pub red: u8,
    pub green: u8,
    pub blue: u8,
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootMmapEntry {
    pub size: u32,
    pub addr: u64,
    pub len: u64,
    pub _type: u32,
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootModList {
    pub mod_start: u32,
    pub mod_end: u32,
    pub cmdline: u32,
    pub pad: u32,
}

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct MultibootApmInfo {
    pub version: u16,
    pub cseg: u16,
    pub offset: u32,
    pub cseg_16: u16,
    pub dseg: u16,
    pub flags: u16,
    pub cseg_len: u16,
    pub cseg_16_len: u16,
    pub dseg_len: u16,
}

//pub unsafe fn multiboot_parse2(mb_info: &MultibootInfo, magic: u32, kernel_end: *mut u8) -> Option<&BootParams> {
//    if magic != MULTIBOOT_BOOTLOADER_MAGIC  { return None; }
//
//    let mut placement = kernel_end as usize;
//
//    let boot_params_ptr = placement as *mut BootParams;
//    let boot_params = boot_params_ptr.as_ref().unwrap();
//
//    placement += size_of::<BootParams>();
//
//    return None;
//}


