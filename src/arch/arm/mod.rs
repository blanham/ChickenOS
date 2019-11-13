

fn rust_entry() -> ! {
    let boot_info = get_bootinfo_from_soc()
    rust_kmain(boot_info);
}
