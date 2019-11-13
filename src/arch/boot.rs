
#[repr(C, packed)]
pub struct BootParams {
    pub cmdline: *mut u8,
    pub mem_size: usize,
    pub placement: usize,
    pub regions: *mut HardwareMemoryRegion,
    pub modules_count: u32,
    pub modules: *mut BootModule,
    pub framebuffer: *mut u32,
    pub x_res: u32,
    pub y_res: u32,
    pub bpp: u32,
    pub x_chars: u32,
    pub y_chars: u32,
    pub mode: u32,
    pub aux: *mut u32
}

#[repr(C, packed)]
pub struct HardwareMemoryRegion {
    pub size: u32,
    pub addr: u64,
    pub len: u64,
    pub _type: u32,
}

#[repr(C, packed)]
pub struct BootModule {
    pub start: usize,
    pub end: usize,
    pub cmdline: [u8; 256],
}

//TODO: add impls
