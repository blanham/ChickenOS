cmake_minimum_required(VERSION 3.1.0)

project(ChickenOS C)

#set(CAN_USE_ASSEMBLER TRUE)
#set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
#set(CMAKE_NASM_LINK_EXECUTABLE "/usr/bin/clang <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
#set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")
#set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS ${CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS} s S)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_C_FLAGS
    "-m64 -target x86_64-unknown-windows -mno-sse -mno-mmx -g -Wall -Werror\
				-std=gnu11\
				-ffreestanding\
                -fshort-wchar\
				-fno-stack-protector\
                -mno-red-zone\
                -nostdinc\
                -Wextra\
				-fno-builtin -DARCH_X86_64\
                -Wextra"
)
#set(CMAKE_LINKER "/usr/bin/clang")
#file(GLOB LINKER_SCRIPT "src/arch/x86_64/linker.ld")
#set(CMAKE_EXE_LINKER_FLAGS "-m64 -target x86_64-unknown-windows -nostdlib -T ${LINKER_SCRIPT}")

#file(GLOB_RECURSE ASM_FILES "src/arch/i386/*.s")
#set_source_files_properties(${ASM_FILES} PROPERTIES LANGUAGE ASM_NASM)

set(PREFIX_PATH "../../..")

include_directories(${PREFIX_PATH}/src/arch/x86_64/include ${PREFIX_PATH}/src/arch/generic/include ${PREFIX_PATH}/src/include ${PREFIX_PATH}/src/fs)
file(GLOB_RECURSE ARCH_FILES ${PREFIX_PATH}/src/arch/x86_64/*.c)
file(GLOB_RECURSE DEV_FILES ${PREFIX_PATH}/src/device/*.c)
file(GLOB_RECURSE FS_FILES ${PREFIX_PATH}/src/fs/*.c)
file(GLOB_RECURSE INIT_FILES ${PREFIX_PATH}/src/init/*.c)
file(GLOB_RECURSE LIB_FILES ${PREFIX_PATH}/src/lib/*.c)
file(GLOB_RECURSE MISC_FILES ${PREFIX_PATH}/src/misc/*.c)
file(GLOB_RECURSE MM_FILES ${PREFIX_PATH}/src/mm/*.c)
file(GLOB_RECURSE NET_FILES ${PREFIX_PATH}/src/net/*.c)
file(GLOB_RECURSE THREAD_FILES ${PREFIX_PATH}/src/thread/*.c)

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
add_custom_target(test cd ${PREFIX_PATH}/.. && ./new_update.sh && DISPLAY=:0 qemu-system-x86_64 -vga std -no-reboot  -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso -soundhw pcspk -serial file:log && cd src)
add_dependencies(test kernel.bin)
#add_custom_target(debug cd .. && ./new_update.sh && DISPLAY=:0 qemu-system-i386 -vga std   -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso -soundhw pcspk -serial file:log -gdb tcp::1212 -S  && cd src)
#add_dependencies(debug kernel.bin)
#add_custom_target(monitor  cd .. && ./new_update.sh &&        qemu-system-i386 -nographic -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso && cd src USES_TERMINAL)
#add_dependencies(monitor kernel.bin)
#add_custom_target(terminal cd .. && ./new_update.sh &&        qemu-system-i386 -curses    -rtc base=localtime -m 128 -usb -hda disk.img -cdrom ~/disk_images/alpine-standard-3.11.2-x86.iso && cd src USES_TERMINAL)
#add_dependencies(terminal kernel.bin)

set_target_properties(kernel.bin PROPERTIES C_STANDARD 11)
#target_link_libraries(kernel.bin ${CMAKE_SOURCE_DIR}/src/clangrt.a) # FIXME: This should be removed heh

