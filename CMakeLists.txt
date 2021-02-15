cmake_minimum_required(VERSION 3.1.0)

project(ChickenOS C)

#ASM_NASM)

set(CAN_USE_ASSEMBLER TRUE)

set(CMAKE_C_COMPILER "clang")
#set(CMAKE_LINKER "clang")

set(CMAKE_C_LINK_EXECUTABLE "ld <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
include(src/arch/aarch64/CMakeLists.txt)
include_directories(src/arch/aarch64/include src/include src/fs src/include/kernel)


file(GLOB_RECURSE DEV_FILES src/device/*.c)
file(GLOB_RECURSE FS_FILES src/fs/*.c)
file(GLOB_RECURSE INIT_FILES src/init/*.c)
file(GLOB_RECURSE LIB_FILES src/lib/*.c)
file(GLOB_RECURSE MISC_FILES src/misc/*.c)
file(GLOB_RECURSE MM_FILES src/mm/*.c)
file(GLOB_RECURSE NET_FILES src/net/*.c)
file(GLOB_RECURSE THREAD_FILES src/thread/*.c)

add_executable(kernel.bin
    ${ARCH_FILES}
    ${DEV_FILES}
    ${FS_FILES}
    ${INIT_FILES}
    ${LIB_FILES}
    ${MISC_FILES}
    ${MM_FILES}
    ${NET_FILES}
    ${THREAD_FILES}
    ${ASM_FILES}
)

cmake_policy(SET CMP0037 OLD) # Enable using "test" as target name
add_custom_target(test cd .. && ./new_update.sh && DISPLAY=:0 qemu-system-i386 -vga std -no-reboot  -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso -soundhw pcspk -serial file:log && cd src)
add_dependencies(test kernel.bin)
add_custom_target(debug cd .. && ./new_update.sh && DISPLAY=:0 qemu-system-i386 -vga std   -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso -soundhw pcspk -serial file:log -gdb tcp::1212 -S  && cd src)
add_dependencies(debug kernel.bin)
add_custom_target(monitor  cd .. && ./new_update.sh &&        qemu-system-i386 -nographic -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso && cd src USES_TERMINAL)
add_dependencies(monitor kernel.bin)
add_custom_target(terminal cd .. && ./new_update.sh &&        qemu-system-i386 -curses    -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso && cd src USES_TERMINAL)
add_dependencies(terminal kernel.bin)

set_target_properties(kernel.bin PROPERTIES C_STANDARD 11)
#target_link_libraries(kernel.bin ${CMAKE_SOURCE_DIR}/src/clangrt.a) # FIXME: This should be removed heh
