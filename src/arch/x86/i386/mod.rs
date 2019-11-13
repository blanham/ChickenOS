use ::arch::{PAddr, VAddr};

pub const PHYS_BASE: usize = 0xC0000000;
//pub mod boot;
//pub mod io;

pub const PAGE_SIZE: usize = 4096;

impl PAddr {
    pub fn to_v(&self) -> VAddr {
        let PAddr(addr) = *self;
        VAddr(addr + PHYS_BASE)
    }
}

impl VAddr {
    pub fn to_p(&self) -> PAddr {
        let VAddr(addr) = *self;
        PAddr(addr - PHYS_BASE)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use ::arch::{PAddr, VAddr};

    #[test]
    fn virtual_to_physical() {
        let virt = VAddr(0xB00B8000);
        assert_eq!(PAddr(0xB8000), virt.to_p());
    }

    #[test]
    fn physical_to_virtual() {
        let phys = PAddr(0xB8000);
        assert_eq!(VAddr(0xB00B8000), phys.to_v());
    }

}

