#ifndef C_OS_SYS_TYPES_H
#define C_OS_SYS_TYPES_H
typedef unsigned int mode_t;
typedef unsigned int time_t;
typedef unsigned int dev_t;
typedef unsigned int ino_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef signed int pid_t;
typedef long long blkcnt_t;
typedef signed int blksize_t;
typedef unsigned short nlink_t;
typedef long long off_t;

#ifndef _SIZE_T
typedef unsigned int size_t;
#define _SIZE_T
#endif
typedef int ssize_t;
typedef signed int suseconds_t;
typedef unsigned int useconds_t;
#endif
