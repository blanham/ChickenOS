#include <kernel/common.h>
#include <kernel/fs/vfs.h>
#include <mm/liballoc.h>
#include "ext2fs_defs.h"
#include "ext2fs.h"
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 512
#define EXT2_SB_SIZE 1024
#define EXT2_SB_BLOCK 2


//moved debug functions to seperate file
#include "ext2_debug.c"

int ext2_read_superblock(vfs_fs_t *fs, uint16_t dev);
struct inode * ext2_read_inode(ext2_fs_t *fs, int ino);

struct inode * ext2_namei(struct inode *dir, char *file);
//FIXME
vfs_ops_t ext2_ops = {
	NULL,//vfs_read_inode_t read;
	NULL,//vfs_write_inode_t write;
	ext2_read_superblock,//vfs_read_sb_t read_sb;
	NULL//vfs_namei_t namei;
};



void ext2_inode_to_vfs(ext2_fs_t *fs,struct inode *vfs,ext2_inode_t *ext2,uint32_t inode)
{
	vfs->size = ext2->i_size;
	vfs->inode_num = inode;
	vfs->mode = ext2->i_mode;
	vfs->size = ext2->i_size;
	vfs->time = ext2->i_ctime;
	//if part of mount point,keep in cache
	vfs->flags = 0;
	vfs->storage = ext2;
	vfs->fs = (void *)fs;

}

extern int read_block(uint16_t dev, void * _buf, int block, int block_size);

size_t ext2_read_block(ext2_fs_t *fs, void *buf, int block)
{
	int block_size = fs->aux->block_size;
	return	read_block(fs->dev, buf, block, block_size);
//block_device_read(fs->dev, buf, 
//		((block)*block_size)/512);
}
int ext2_read_superblock(vfs_fs_t *fs, uint16_t dev)
{
	ext2_fs_t *ext2 = (ext2_fs_t *)fs;
	fs->dev = dev;

	struct inode *root;
	fs->superblock->sb = kmalloc(sizeof(ext2_superblock_t));
	ext2_superblock_t *sb = fs->superblock->sb;
		
	int count = read_block_generic(sb, 1024, 2*512, 512, 
		(void *)(uint32_t)dev, NULL);
	if(count != 1024)
	{
		printf("reading superblock failed\n");
		goto fail;
	}
	
	if(sb->s_magic != EXT2_MAGIC)
	{
		printf("bad magic number\n");
		goto fail;
	}

	ext2_aux_t *aux = fs->aux;
	ext2->aux->gd_block = (sb->s_log_block_size == 0 ? 3 : 2);

	int num_groups = (sb->s_blocks_count - 1)/sb->s_blocks_per_group + 1;

	
	int size = sizeof(ext2_group_descriptor_t)*num_groups;
	
	printf("EXT2 groups %i blocks %i gd_size %i\n", 
		num_groups,sb->s_blocks_count, size);
	
	ext2->aux->gd_table = 
		(ext2_group_descriptor_t *)kmalloc(size);

	ext2->aux->block_size = 1024 << sb->s_log_block_size;
	
	ext2_read_block(ext2, aux->gd_table, aux->gd_block - 1);
	
	//now that we can read inodes, we cache the root inode(2)
	//in the vfs superblock struct
	if((root  = kmalloc(sizeof(*root))) == NULL)
		goto fail;

	if((root = ext2_read_inode((ext2_fs_t *)fs, EXT2_ROOT_INO)) == NULL)
		goto fail;
	
	fs->superblock->root = root;
	
struct inode * test2;// = ext2_read_inode((ext2_fs_t *)fs, 13);
	struct inode * test = ext2_namei(root, "dev");
	test2 = ext2_namei(test, "tty");
	inode_print(*(ext2_inode_t *)test2->storage);
//	test = test;
	return 0;
fail:
	if(root != NULL)
		kfree(root);	
	return -1;
}




//read_superblock
//read_inode

struct inode * ext2_read_inode(ext2_fs_t *fs, int inode)
{
	struct inode *read = kmalloc(sizeof(*read));
	ext2_inode_t *ext2_ino = kmalloc(sizeof(*ext2_ino));
	ext2_superblock_t *sb = fs->superblock->sb;
	
	int group = ((inode - 1)/sb->s_inodes_per_group) + 1;
	
	ext2_group_descriptor_t * gd = 
		&fs->aux->gd_table[group -1];
	printf("group =%i\n",group -1);	
	ext2_inode_t *table = 
		kmalloc(sizeof(*ext2_ino) * sb->s_inodes_per_group);
	
	//FIXME: fixed size (1 block)
	read_block(fs->dev, table, gd->bg_inode_table, fs->aux->block_size);
	read_block(fs->dev, (void*)((uint32_t)table + 1024), gd->bg_inode_table + 1, fs->aux->block_size);
	read_block(fs->dev, (void*)((uint32_t)table + 2048), gd->bg_inode_table + 2, fs->aux->block_size);

//	ext2_read_block(fs, (void*)((uint32_t)table + 1024), gd->bg_inode_table+1);
//	ext2_read_block(fs, (void*)((uint32_t)table + 1024*2), gd->bg_inode_table+2);
//	ext2_read_block(fs, (void*)((uint32_t)table + 1024*3), gd->bg_inode_table+3);
	//if(this fails) free(read,ext_ino, table) and return NULL;
	memcpy(ext2_ino, &table[inode-1], sizeof(*ext2_ino));
	kfree(table);
	ext2_inode_to_vfs(fs, read,ext2_ino,inode);
	return read;
}
//read_dir

typedef uint32_t ino_t;
struct inode * ext2_namei(struct inode *dir, char *file)
{
	ext2_fs_t *fs = (ext2_fs_t *)dir->fs;
	printf("num %i\n", dir->inode_num);
	struct inode *inode = ext2_read_inode((ext2_fs_t *)dir->fs, dir->inode_num);
	ext2_inode_t * in = (ext2_inode_t *)(inode->storage);
	int len = 0;	
	ext2_directory_t *ext2_dir = kmalloc(inode->size);
	int count =inode->size;
	void *fdir = ext2_dir;
	
//	inode_print(*in);
	ext2_read_block(fs, ext2_dir, in->i_block[0]);

	printf("block %x\n",(((ext2_inode_t *)(inode->storage))->i_block[0]));
	printf("count %i\n",count); 
//	ext2_read(&test, dir, 4096, 0);
	file = file;
	while(count)
	{
		print_dir_entry(ext2_dir);
		if(!strcmp(ext2_dir->name,file))//,strlen(file)))
		{
			printf("%s%s\n", ext2_dir->name, file);			
			kfree(fdir);
			return ext2_read_inode(fs,ext2_dir->inode); 
		}
		if((len = ext2_dir[0].rec_len) == 0)
			break;	
		ext2_dir = (ext2_directory_t *)(uint32_t)((uint32_t)ext2_dir + (uint32_t)ext2_dir->rec_len);
		count -= len;

	}
	return 0;
}
/*
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

}*/
int ext2_init()
{
	printf("Initializing EXT2 FS\n");
	ext2_fs_t * new = (ext2_fs_t *)vfs_alloc();
	new->aux = kmalloc(sizeof(ext2_aux_t));
	new->ops = &ext2_ops;
	//FIXME:should use strncpy
	strcpy(new->name, "ext2");
	return vfs_register_fs((vfs_fs_t *)new);	
}

