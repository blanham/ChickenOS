#include <kernel/types.h>
void initrd_init(uintptr_t start, uintptr_t end);
void initrd_tester(char *filename);
int initrd_read_block(/*initrd_t *disk,*/ uint8_t *dst, uint32_t blocknum);

