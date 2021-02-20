cmake_minimum_required(VERSION 3.1.0)

enable_language(ASM)
set(CMAKE_CROSSCOMPILING TRUE)
SET(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

SET(TARGET aarch64-elf)

SET(CMAKE_C_COMPILER_TARGET ${TARGET})
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER_TARGET ${TARGET})
SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_ASM_COMPILER_TARGET ${TARGET})
SET(CMAKE_ASM_COMPILER clang)

#set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
#set(CMAKE_NASM_LINK_EXECUTABLE "clang <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
#set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
#set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS ${CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS} s S)

set(CMAKE_C_FLAGS
    "-m64 -target aarch64-elf -mgeneral-regs-only -g -Wall\
    -std=gnu11\
    -ffreestanding\
    -fno-stack-protector\
    -Wextra\
    -fno-builtin -DARCH_AARCH64\
    -Wextra"
)
file(GLOB LINKER_SCRIPT "src/arch/aarch64/linker.ld")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -T ${LINKER_SCRIPT}")

file(GLOB_RECURSE ASM_FILES "src/arch/aarch64/*.s")
#set_source_files_properties(${ASM_FILES} PROPERTIES LANGUAGE ASM_NASM)

file(GLOB_RECURSE ARCH_FILES "src/arch/aarch64/*.c")

set(CMAKE_ASM_LINK_EXECUTABLE"ld <FLAGS> <CMAKE_ASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_LINK_EXECUTABLE"ld <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE"ld <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
#cmake_policy(SET CMP0037 OLD) # Enable using "test" as target name
#add_custom_target(test cd .. && ./new_update.sh && DISPLAY=:0 qemu-system-i386 -vga std -no-reboot  -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso -soundhw pcspk -serial file:log && cd src)
#add_dependencies(test kernel.bin)
#add_custom_target(debug cd .. && ./new_update.sh && DISPLAY=:0 qemu-system-i386 -vga std   -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso -soundhw pcspk -serial file:log -gdb tcp::1212 -S  && cd src)
#add_dependencies(debug kernel.bin)
#add_custom_target(monitor  cd .. && ./new_update.sh &&        qemu-system-i386 -nographic -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso && cd src USES_TERMINAL)
#add_dependencies(monitor kernel.bin)
#add_custom_target(terminal cd .. && ./new_update.sh &&        qemu-system-i386 -curses    -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso && cd src USES_TERMINAL)
#add_dependencies(terminal kernel.bin)
#
#set_target_properties(kernel.bin PROPERTIES C_STANDARD 11)
#target_link_libraries(kernel.bin ${CMAKE_SOURCE_DIR}/src/clangrt.a) # FIXME: This should be removed heh
