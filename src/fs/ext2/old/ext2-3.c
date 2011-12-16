#include "ext2fs_defs.h"
#include "ext2fs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define BLOCK_SIZE 512
#define EXT2_SB_SIZE 1024
#define EXT2_SB_BLOCK 2
struct block_dev {
	uint16_t dev;
	char name[255];
	int fd;

};
void inode_print2(ext2_inode_t inode)
{

	printf("\n=========inode========\n");
	printf("inode mode %x\n",inode.i_mode);
	printf("inode size %i\n",inode.i_size);
	printf("inode atime %i\n",inode.i_atime);
	printf("inode ctime %i\n\n",inode.i_ctime);
	printf("inode block0 %x\n",inode.i_block[0]);
	printf("inode block1 %i\n",inode.i_block[1]);
	printf("inode block2 %i\n\n",inode.i_block[2]);
	uint32_t dev = inode.i_block[0];
//	uint32_t dev = ((dev2 >> 8) & 0xff) | ((dev2 &0xff) << 8);
  unsigned major = (dev & 0xfff00) >> 8;
  unsigned minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);
	major = (dev >> 8 ) & 255;
	minor = dev & 255;
	printf("major %i minor %i\n",major,minor);
}

struct block_dev loop;
int block_read(struct block_dev *dev, void * buf, int block)
{
	lseek64(dev->fd, (int64_t)block*(int64_t)BLOCK_SIZE, SEEK_SET);
	int ret= read(dev->fd, buf, BLOCK_SIZE);
	lseek64(dev->fd, 0, SEEK_SET);
	return ret;
}
int block_readn(struct block_dev *dev,void *buf, int block, int size)
{
	int count = 0;
	while(size > 0)
	{
		block_read(dev, buf, block);

		size -=BLOCK_SIZE;
		buf = (void*)((uint32_t)buf + BLOCK_SIZE);
		count +=BLOCK_SIZE;
		block++;
	}
	return count;
}
int block_read2(struct block_dev *dev,void *buf, int block, int block_size)
{
	int count = 0;
	int size = block_size;
//	while(size > 0)
	{
		block_readn(dev, buf, block, block_size);

//		size -= block_size;
//		buf = (void*)((uint32_t)buf + block_size);
//		count +=block_size;
//		block += block_size/BLOCK_SIZE;
	}
	return count;
}

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
	printf("offset %i size%i block_size%i\n", offset, size,block_size);
	printf("block = %i\n",offset/4096);
	int buf_ofs;
	while(size > 0)
	{
		int block = offset / block_size;
		int block_ofs = offset % block_size;
		int cur_size = block_size - block_ofs;
	//	printf("reading block %i\n",block);	
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
//#define ext2_block_read2(dev,buf,block,blocksize) read_block_generic(buf,blocksize, 0,blocksize, dev,block_read)

#define ext2_block_read3(dev,buf,block,size,blocksize) read_block_generic(buf,blocksize, block,blocksize, (void *)dev,block_read)


//shouldn't need all of this
int ext2_block_read(ext2_fs_t *fs,struct block_dev *dev,void *buf, int block)
{
	int block_size = 1024 << fs->superblock->s_log_block_size;
	int multiplier = block_size / BLOCK_SIZE;
	int count = block_readn(dev,buf, (block)*multiplier, block_size);

	return count;
}

int ext2_block_read2(struct block_dev *dev,void *buf, int block)
{
	int block_size = 4096;//1024 << fs->superblock->s_log_block_size;
	int multiplier = block_size / BLOCK_SIZE;
	int count = block_readn(dev,buf, (block)*multiplier, block_size);

	return count;
}

int ext2_block_readn(ext2_fs_t *fs,struct block_dev *dev,void *buf, int block, int size)
{
	int block_size = 1024 << fs->superblock->s_log_block_size;
	int multiplier = block_size / BLOCK_SIZE;
	int count = 0;
	while(size > 0)
	{
		ext2_block_read(fs, dev, buf, block);
	//	ext2_block_read2(dev,buf,block,block_size);
		size -= block_size;
		buf = (void*)((uint32_t)buf + block_size);
		count += block_size;
		block++;
	}
	abort();
	return count;

}


ext2_fs_t *ext2_new()
{
	ext2_fs_t *ext2 = malloc(sizeof(ext2_fs_t));
	ext2->superblock = malloc(sizeof(ext2_superblock_t));
	ext2->aux = malloc(sizeof(ext2_aux_t));

	return ext2;
}

void gd_print2(ext2_group_descriptor_t gdt)
{
	printf("\n=========GDT========\n");
	printf("block bitmap\t%i\n", gdt.bg_block_bitmap);
	printf("inode bitmap\t%i\n", gdt.bg_inode_bitmap);
	printf("inode table\t%i\n", gdt.bg_inode_table);
	printf("free blocks\t%i\n", gdt.bg_free_blocks_count);
	printf("free inodes\t%i\n", gdt.bg_free_inodes_count);
	printf("dir inodes\t%i\n\n", gdt.bg_used_dirs_count);
}

/*god damnit inode numbering starts with 1, not 0*/
void print_dir_entry(ext2_directory_t *dir)
{
	char buf[256];
	memset(buf, 0, 256);
	strncpy(buf, dir[0].name, dir[0].name_len);	
	printf("%-20s \t\tinode %i\n", buf, dir[0].inode);
}

int ext2_read_superblock(ext2_fs_t *fs, struct block_dev *dev, void *buf)
{
	ext2_superblock_t *sb = buf;
	if(block_readn(dev, buf, EXT2_SB_BLOCK,EXT2_SB_SIZE) != EXT2_SB_SIZE)
		return -1;

	if(sb->s_magic != EXT2_MAGIC)
		return -1;
		
	fs->aux->gd_block = (sb->s_log_block_size == 0 ? 3 : 2);

	int number_of_groups = fs->superblock->s_blocks_count/fs->superblock->s_blocks_per_group;


	int size = sizeof(ext2_group_descriptor_t)*number_of_groups*16;

	fs->aux->gd_table = malloc(size*8);
	fs->block_size = 1024 << sb->s_log_block_size;
 int block_size = fs->block_size;
read_block_generic(fs->aux->gd_table, size, (fs->aux->gd_block -1)*block_size, block_size, &loop, (block_access_fn)ext2_block_read2);
	gd_print2(fs->aux->gd_table[361]);
	return 0;
}

//read_superblock
//read_inode
ext2_inode_t * ext2_read_inode(ext2_fs_t *fs, int inode)
{
	int per_group = fs->superblock->s_inodes_per_group;
	int group = 0;//((inode-1) /per_group);
	int block_size = fs->block_size;
	ext2_group_descriptor_t * gd = &fs->aux->gd_table[group];
	
	inode = inode - group*per_group;
	printf("per %i\n",per_group);
	gd_print2(*gd);	
	int size= per_group*sizeof(ext2_inode_t)*8;
	ext2_inode_t *table = malloc(size);
	//FIXME: fixed sizes

	int64_t block = (int64_t)gd->bg_inode_table*(int64_t)block_size;
	
//	read_block_generic(table, size, gd->bg_inode_table, block_size, &loop, (block_access_fn)ext2_block_read2);
//	read_block_generic(table, sizeof(ext2_inode_t), 11829250*4096, block_size, &loop, (block_access_fn)ext2_block_read2);
	int fd = loop.fd;
	lseek64(fd, (int64_t)gd->bg_inode_table*(int64_t)block_size,SEEK_SET);	
	read(fd, table, size);
	printf("block %lli\n",(block + sizeof(ext2_inode_t)*inode) / 4096);
//	ext2_block_read2(&loop, table, gd->bg_inode_table);
	printf("inode %i group %i block %i\n",inode,group,gd->bg_inode_table);
	inode_print2(table[16]);
	abort();	
	ext2_inode_t *ret = malloc(sizeof(*ret));
	memcpy(ret, &table[inode-1], sizeof(*ret));
	free(table);	
	return ret;
}
//read_dir
struct file {
	void *fs;
	ino_t inode;
};


int ext2_namei(ext2_fs_t *fs, char *file, ino_t directory)
{
	ext2_inode_t *dir_ino = ext2_read_inode(fs, directory);
	int len = 0;	
	ext2_directory_t *dir = malloc(1024*8*4);
	void *fdir = dir;
	int count =256;
	int block_size = 4096;
	ext2_block_read(fs, &loop, dir, dir_ino->i_block[0]);
//	printf("fdir%i test %i\n",directory, dir_ino->i_block[0]);
//read_block_generic(dir, 1024*8*4, dir_ino->i_block[0]*block_size, block_size, &loop, ext2_block_read2);
//	struct file test;
//	test.fs = fs;
//	test.inode = directory;

//	ext2_read(&test, dir, 4096, 0);
	while(count)
	{
		print_dir_entry(dir);
	//	printf("%s %s\n",dir->name, file);
		if(!strncmp(dir->name,file,strlen(file)))
		{
			int result = dir->inode;
			free(fdir);
			return result; 
		}
		if((len = dir[0].rec_len) == 0)
			break;	
		dir = (void *)((uint32_t)dir + dir->rec_len);
		
		count -= len;

	}
	return -1;
}

#define EXT2_NDIR_BLOCKS 12
int byte_to_block(ext2_fs_t *fs, ext2_inode_t *inode, int offset)
{
	int block_offset = offset / fs->block_size;
	int block_size = fs->block_size;
	
	if(offset > inode->i_size)
		return -1;
	
	if(offset < EXT2_NDIR_BLOCKS*block_size)
	{
		return inode->i_block[block_offset];

	}else if(offset < (EXT2_NDIR_BLOCKS + (block_size/sizeof(uint32_t)))){
		uint32_t *indirect = malloc(block_size*sizeof(uint32_t));
		if(ext2_block_read(fs,&loop,indirect, inode->i_block[12]) != block_size)
			return -1;	
		int ret = indirect[(offset - EXT2_NDIR_BLOCKS*block_size)/block_size];
		free(indirect);
		return ret;

	}else{
		printf("doubly indirect blocks not supported yet!\n");
		abort();

	}
}

int ext2_read(struct file *file, void *buffer, int size, int offset)
{
	ext2_fs_t *fs = file->fs;
	ext2_inode_t *inode = ext2_read_inode(fs, file->inode);
	inode_print2(*inode);
	
	int block = byte_to_block(fs, inode, 0);
	ext2_block_read(fs, &loop, buffer, block);

	return 0;
}

void done()
{
	close(loop.fd);
}

int lookup(ext2_fs_t *fs, char *name_)
{
//FIXME: handle other cases	
	int inode = 2;

	char *name = malloc(strlen(name_));
	strcpy(name, name_);

	char *tok = strtok(name, "/");

	if((inode = ext2_namei(fs, tok, inode)) == -1)
		return -1;
	printf("inode lookup%i\n",inode);	
	while((tok = strtok(NULL,"/")) != NULL)
	{
		printf("inode lookup%i %s\n",inode,tok);	
		if((inode = ext2_namei(fs, tok, inode)) == -1)
			break;
	}
	printf("inode result %i\n",inode);	
	return inode;
}

int main(int argc,char**argv)
{
	strcpy(loop.name, "loop");
	loop.fd = open("./old/ext2.img", O_RDONLY);
	ext2_fs_t *new = ext2_new();
	new->tmp = &loop;
	ext2_read_superblock(new, &loop, new->superblock);
		
//	gd_print2(new->aux->gd_table[0]);
//	int ino = ext2_namei(new, "dv",2);
//	int ino = lookup(new, "dev");
//	printf("ino %i\n",ino);
//	return 0;
//	ino = ext2_namei(new, "hello.c",ino);
	
	ext2_inode_t *elem = ext2_read_inode(new, 16);
	inode_print2(*elem);


		
//	int ino = lookup(new, "/home/blanham/hello.c");
//	int ino2 = 0;//lookup(new, "/test/hsdfasdf/ello.c");
//	printf("ino %i ino2 %i\n", ino,ino2);
 //ext2_namei(new, "blanham", 2951537);
//	struct file test;
//	test.fs = new;
//	test.inode = ino;
	exit(0);
	char *buf = malloc(1024);
//	ext2_read(&test, buf, 1024, 0);
	printf("%s\n",buf);
	atexit(done);

}

