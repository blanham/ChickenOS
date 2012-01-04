ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
   . = 0x80000000;

   .text  :{
       *(.text)
       *(.rodata*)
   }

   .data ALIGN (0x1000): {
       *(.data)
   }

   .bss : {
       _sbss = .;
       *(COMMON)
       *(.bss)
       _ebss = .;
   }
}
