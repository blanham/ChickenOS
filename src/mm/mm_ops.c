/*	ChickenOS - mm/mm_ops.c
 */
#include <common.h>
#include <stdio.h>
#include <mm/vm.h>
#include <chicken/thread.h>
#include <kernel/hw.h>

//Put sys_mmap, and any other system calls that affect memory mapping
int sys_mprotect(void *addr UNUSED, size_t len UNUSED, int prot UNUSED)
{
    serial_printf("mprotect not implemented yet\n");
    return 0;
}

