ENTRY(entry)
SECTIONS {
	/*
	 *
	 *
	 *
	 */
	. = 0x80000;
	.text : AT(ADDR(.text)) {
		*(.text)
		*(.rodata*)
	}
	.user ALIGN(0x1000) : AT(ADDR(.user)) {
		*(.user)
		. = ALIGN(0x1000);
	}

	.data ALIGN(0x1000) : AT(ADDR(.data)) {
		*(.data)
		_init_start = .;
		*(.data.init)
		_init_end = .;
	}

	.bss : AT(ADDR(.bss)) {
		_sbss = .;
		*(COMMON)
		*(.bss)
		_ebss = .;
	}

	end = .;
}
