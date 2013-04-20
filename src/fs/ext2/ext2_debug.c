

void inode_print(ext2_inode_t inode)
{

	printf("\n=========inode========\n");
	printf("inode mode %x\n",inode.i_mode);
	printf("inode size %i\n",inode.i_size);
	printf("inode atime %i\n",inode.i_atime);
	printf("inode mtime %i\n",inode.i_mtime);
	printf("inode ctime %i\n\n",inode.i_ctime);
	printf("inode block0 %x\n",inode.i_block[0]);
	printf("inode block1 %i\n",inode.i_block[1]);
	printf("inode block2 %i\n\n",inode.i_block[2]);
	if(inode.i_size == 0 && inode.i_block[0] != 0)
	{
		uint32_t dev = inode.i_block[0];
	  	unsigned major = (dev & 0xfff00) >> 8;
	  	unsigned minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);
		major = (dev >> 8 ) & 255;
		minor = dev & 255;
		printf("major %i minor %i\n",major,minor);
	}
}

void gd_print(ext2_group_descriptor_t gdt)
{
	printf("\n=========GDT========\n");
	printf("block bitmap\t%i\n", gdt.bg_block_bitmap);
	printf("inode bitmap\t%i\n", gdt.bg_inode_bitmap);
	printf("inode table\t%i\n", gdt.bg_inode_table);
	printf("free blocks\t%i\n", gdt.bg_free_blocks_count);
	printf("free inodes\t%i\n", gdt.bg_free_inodes_count);
	printf("dir inodes\t%i\n\n", gdt.bg_used_dirs_count);
}
//god damnit inode numbering starts with 1, not 0
void print_dir_entry(ext2_directory_t *dir)
{
	char buf[256];
	memset(buf, 0, 256);
	//strncpy(buf, dir[0].name, dir[0].name_len);	
	strcpy(buf, dir[0].name);
	printf("%s \t\tinode %i\n", buf, dir[0].inode);
}


