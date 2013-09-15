#ifndef C_OS_SYS_STAT_H
#define C_OS_SYS_STAT_H
#include <sys/types.h>
struct stat {
	dev_t	st_dev; 	//ID of device containing file
	ino_t	st_ino;		//file serial number
	mode_t	st_mode; 	//mode of file (see below)
	nlink_t	st_nlink;	//number of links to the file
	uid_t	st_uid;		//user ID of file
	gid_t	st_gid;		//group ID of file
	dev_t	st_rdev;	//device ID (if file is character or block special)
	off_t	st_size;   //file size in bytes (if file is a regular file)
	time_t	st_atime;   //time of last access
	time_t	st_mtime;   //time of last data modification
	time_t	st_ctime;   //time of last status change
	blksize_t st_blksize; //a filesystem-specific preferred I/O block size for
	                     //this object.  In some filesystem types, this may
	                     //vary from file to file
	blkcnt_t  st_blocks;//  number of blocks allocated for this object
};

#define S_IFMT		0170001	
#define S_IFBLK 	0010000
#define S_IFCHR 	0020000
#define S_IFIFO 	0040000
#define S_IFREG 	0060000
#define S_IFDIR 	0100000
#define S_IFLNK 	0120000

#define S_IRUSR		0000400
#define S_IWUSR		0000200
#define S_IXUSR		0000100
#define S_IRGRP		0000040
#define S_IWGRP		0000020
#define S_IXGRP		0000010
#define S_IROTH		0000004
#define S_IWOTH		0000002
#define S_IXOTH		0000001
#define S_ISUID		0004000
#define S_ISGID		0002000
#define S_ISVTX		0001000


#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH) 

#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)	
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)	
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)	
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)	
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)	
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)	

#define S_TYPEISMQ(buf)		(0)
#define S_TYPEISSEM(buf)	(0)
#define S_TYPEISSHM(buf)	(0)

int    chmod(const char *, mode_t);
int    fchmod(int, mode_t);
int    fstat(int, struct stat *);
int    lstat(const char *, struct stat *);
int    mkdir(const char *, mode_t);
int    mkfifo(const char *, mode_t);
int    mknod(const char *, mode_t, dev_t);
int    stat(const char *, struct stat *);
mode_t umask(mode_t);
#endif
