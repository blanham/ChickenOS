#include <common.h>
#include <stdio.h>

#include <dirent.h>



	//struct file *fp = thread_current()->files[fd];
struct __dirstream {
	//These are taken from Linux for reference
	//int fd;
	//lock?
	//allocation size
	//size
	//offset
	//
	//filepos
	//
	//data
};
/*typedef struct __dirstream DIR;

struct dirent
{
	ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[256];
};
*/
/*
int            closedir(DIR *);
DIR           *fdopendir(int);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
int            readdir_r(DIR *__restrict, struct dirent *__restrict, struct dirent **__restrict);
void           rewinddir(DIR *);
void           seekdir(DIR *, long);
long           telldir(DIR *);
int            dirfd(DIR *);

int alphasort(const struct dirent **, const struct dirent **);
int scandir(const char *, struct dirent ***, int (*)(const struct dirent *), int (*)(const struct dirent **, const struct dirent **));

int getdents(int, struct dirent *, size_t);

int versionsort(const struct dirent **, const struct dirent **);
*/
