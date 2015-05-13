#ifndef C_OS_BITMAP_H
#define C_OS_BITMAP_H
#include <stdint.h>
#include <limits.h>
#include <string.h>
//TODO: needs housekeeping

typedef struct bitmap {
	uint32_t *data;
	uint32_t size;
	uint32_t cache;
} bitmap_t;

#define BIT_HIGH (uint32_t)(1 << 31)
#define BITMAP_ERROR -1
#define BITMAP_BITS (sizeof(uint32_t)*CHAR_BIT)
#define BITMAP_MASK (BITMAP_BITS -1)
/*inline int bitmap_init(bitmap_t *bitmap, uint32_t size)
{


}*/

static inline void bitmap_init_phys(bitmap_t *bitmap, uint32_t size, uint32_t *ptr)
{
	bitmap->size = size;
	bitmap->data = ptr;
	size = size / BITMAP_BITS;

	memset(ptr, 0, size * sizeof(*ptr));
}

static inline int bitmap_test(bitmap_t *bitmap, uint32_t index)
{
	int array_offset = index / 32;
	int offset = index & BITMAP_MASK;
	
	if((bitmap->data[array_offset] & (BIT_HIGH >> offset)) != 0)
		return 1;
	else
		return -1;

}

static inline int bitmap_set(bitmap_t *bitmap, uint32_t index)
{
	int array = index / 32;
	int offset = index & BITMAP_MASK;
//	if(bitmap[array] & (BIT_HIGH >>  offset))
//		return -1;
//	else{
		bitmap->data[array] |= (BIT_HIGH >> offset);	
		return 1;

//	}

}

static inline int bitmap_set_multiple(bitmap_t *bitmap, uint32_t index, uint32_t count)
{
	for(uint32_t i = 0; i < count; i ++)
		bitmap_set(bitmap, index + i);

	return 0;
}

static inline int bitmap_clear(bitmap_t *bitmap, uint32_t index)
{
	int array = index / 32;
	int offset = index % 32;
//	if(bitmap[array] & (1 << ( 31 - offset)))
//		return -1;
//	else{
		bitmap->data[array] &= ~(BIT_HIGH >> offset);	
		return 1;

//	}

}

static inline int bitmap_clear_multiple(bitmap_t *bitmap, uint32_t index, uint32_t count)
{
	for(uint32_t i = 0; i < count; i ++)
		bitmap_clear(bitmap, index + i);

	return 0;
}

static inline uint32_t bitmap_find_first(bitmap_t *bitmap)
{
	uint32_t i;
	for(i = 0; i < bitmap->size/32; i++)
	{
		if(bitmap->data[i] == (uint32_t)~0)
			continue;
	
		for(int j = 0; j < 32; j++)
			if(bitmap_test(bitmap,i*32 + j) == -1)
				return i;
	}
	return BITMAP_ERROR;
}

static inline uint32_t bitmap_find_multiple(bitmap_t *bitmap, int count)
{
	uint32_t i;
	for(i = 0; i < bitmap->size; i++)
		if(bitmap_test(bitmap,i) == -1)
		{
			for(int j = 1; j < count - 1; j++)
			{	
				if(bitmap_test(bitmap,i+j) == 1)
					goto jump;//return BITMAP_ERROR;
			}
			return i;
			jump:
				;
		}
	return BITMAP_ERROR;
}

#endif
