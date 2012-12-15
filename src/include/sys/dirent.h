#ifndef C_OS_DIRENT_H
#define C_OS_DIRENT_H
#include <sys/types.h>
#define PATH_MAX 255


struct dirent {
	ino_t d_ino;
	char d_name[];


};

typedef struct dirstream {
	int d_fd;

} DIR;
int            closedir(DIR *);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
int            readdir_r(DIR *, struct dirent *, struct dirent **);
void           rewinddir(DIR *);
void           seekdir(DIR *, long int);
long int       telldir(DIR *);

#endif
