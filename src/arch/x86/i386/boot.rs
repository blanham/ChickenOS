use ::arch::i386::multiboot_parser;

pub fn boot(mb: *const i32, magic: u32) -> *const usize {
    let ret = multiboot_parser(mb, magic);

    return ret;

}

