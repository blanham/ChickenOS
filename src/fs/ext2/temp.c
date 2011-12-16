



typedef int(*block_access_fn)(void *aux, void *buf, int block);
int read_block_generic(void * _buf, int size, int offset, int block_size, void *aux, block_access_fn f)
{
	uint8_t *buf = _buf;
	uint8_t *bounce;
	off_t count = 0;
	if(block_size <= 0)
		return -1;
	
	if((bounce = malloc(block_size)) == NULL)
	{
		printf("memory allocation error in readblock\n");
		return -1;

	}
	
	int buf_ofs;
	while(size > 0)
	{
		int block = offset / block_size;
		int block_ofs = offset % block_size;
		int cur_size = block_size - block_ofs;
		
		if(block_ofs == 0 && cur_size == block_size)
		{

			if(f(aux, buf + count, block) == -1)
			{
				count = -1;
				goto end;
			}

		}
		else
		{
			if(f(aux, bounce, block) == -1)
			{
				count = -1;
				goto end;
			}

			memcpy(buf + count, bounce + block_ofs, cur_size);
		} 

		count  += cur_size;
		offset += cur_size;
		size   -= cur_size;
	}

end:
	free(bounce);
	return count;
}
