#ifndef C_OS_MEMORY_H
#define C_OS_MEMORY_H
#include <types.h>
size_t kstrlen(uint8_t *str);
uint8_t *kmemcpy(void *dest, void *src, size_t count);
uint16_t *kmemcpyw(uint16_t *dest, uint16_t *src, size_t count);
uint8_t *kmemset(void *dest, uint8_t val, size_t count);
uint16_t *kmemsetw(uint16_t *dest, uint16_t val, size_t count);
uint32_t *kmemsetl(uint32_t *dest, uint32_t val, size_t count);
#endif

