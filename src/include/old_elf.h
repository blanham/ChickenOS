typedef struct elf_header {
	uint8_t magic[4];
	uint8_t class;
	uint8_t byteorder;
	uint8_t hversion;
	uint8_t pad[9];
	uint16_t filetype;
	uint16_t archtype;
	uint32_t fversion;
	uint32_t entry;
	uint32_t phdrpos;
	uint32_t shdrpos;
	uint32_t flags;
	uint16_t hdrsize;
	uint16_t phdrent;
	uint16_t phdrcnt;
	uint16_t shdrent;
	uint16_t shdrcnt;
	uint16_t strsec;
} __attribute__((packed)) elf_header_t;
typedef struct elf_section {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_align;
	uint32_t sh_entsize;
}  __attribute__((packed)) elf_section_t;
typedef struct elf_program_header {
	uint32_t ph_type;
	uint32_t ph_offset;
	uint32_t ph_virtaddr;
	uint32_t ph_physaddr;
	uint32_t ph_filesize;
	uint32_t ph_memsize;
	uint32_t ph_flags;
	uint32_t ph_align;
} __attribute__((packed)) elf_program_header_t;

