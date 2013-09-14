#include <types.h>
void initrd_init(uintptr_t start, uintptr_t end);
void initrd_tester(char *filename);
int initrd_read_block(void *_disk, void *dst, uint32_t blocknum);

