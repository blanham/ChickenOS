ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
   . = 0x8048000;

   .text  ALIGN (0x1000):{
       *(.text)
       *(.rodata*)
   }

   .data ALIGN (0x1000): {
       *(.data)
   }

   .bss ALIGN (0x1000): {
       _sbss = .;
       *(COMMON)
       *(.bss)
       _ebss = .;
   }
}
