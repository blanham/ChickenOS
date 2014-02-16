1	sys_exit	kernel/exit.c 	int	-	-	-	-
2	sys_fork	arch/i386/kernel/process.c 	struct pt_regs	-	-	-	-
3	sys_read	fs/read_write.c 	unsigned int	char *	size_t	-	-
4	sys_write	fs/read_write.c 	unsigned int	const char *	size_t	-	-
5	sys_open	fs/open.c 	const char *	int	int	-	-
6	sys_close	fs/open.c 	unsigned int	-	-	-	-
7	sys_waitpid	kernel/exit.c 	pid_t	unsigned int *	int	-	-
8	sys_creat	fs/open.c 	const char *	int	-	-	-
9	sys_link	fs/namei.c 	const char *	const char *	-	-	-
10	sys_unlink	fs/namei.c 	const char *	-	-	-	-
11	sys_execve	arch/i386/kernel/process.c 	struct pt_regs	-	-	-	-
12	sys_chdir	fs/open.c 	const char *	-	-	-	-
13	sys_time	kernel/time.c 	int *	-	-	-	-
14	sys_mknod	fs/namei.c 	const char *	int	dev_t	-	-
15	sys_chmod	fs/open.c 	const char *	mode_t	-	-	-
16	sys_lchown	fs/open.c 	const char *	uid_t	gid_t	-	-
18	sys_stat	fs/stat.c 	char *	struct __old_kernel_stat *	-	-	-
19	sys_lseek	fs/read_write.c 	unsigned int	off_t	unsigned int	-	-
20	sys_getpid	kernel/sched.c 	-	-	-	-	-
21	sys_mount	fs/super.c 	char *	char *	char *	-	-
22	sys_oldumount	fs/super.c 	char *	-	-	-	-
23	sys_setuid	kernel/sys.c 	uid_t	-	-	-	-
24	sys_getuid	kernel/sched.c 	-	-	-	-	-
25	sys_stime	kernel/time.c 	int *	-	-	-	-
26	sys_ptrace	arch/i386/kernel/ptrace.c 	long	long	long	long	-
27	sys_alarm	kernel/sched.c 	unsigned int	-	-	-	-
28	sys_fstat	fs/stat.c 	unsigned int	struct __old_kernel_stat *	-	-	-
29	sys_pause	arch/i386/kernel/sys_i386.c 	-	-	-	-	-
30	sys_utime	fs/open.c 	char *	struct utimbuf *	-	-	-
33	sys_access	fs/open.c 	const char *	int	-	-	-
34	sys_nice	kernel/sched.c 	int	-	-	-	-
36	sys_sync	fs/buffer.c 	-	-	-	-	-
37	sys_kill	kernel/signal.c 	int	int	-	-	-
38	sys_rename	fs/namei.c 	const char *	const char *	-	-	-
39	sys_mkdir	fs/namei.c 	const char *	int	-	-	-
40	sys_rmdir	fs/namei.c 	const char *	-	-	-	-
41	sys_dup	fs/fcntl.c 	unsigned int	-	-	-	-
42	sys_pipe	arch/i386/kernel/sys_i386.c 	unsigned long *	-	-	-	-
43	sys_times	kernel/sys.c 	struct tms *	-	-	-	-
45	sys_brk	mm/mmap.c 	unsigned long	-	-	-	-
46	sys_setgid	kernel/sys.c 	gid_t	-	-	-	-
47	sys_getgid	kernel/sched.c 	-	-	-	-	-
48	sys_signal	kernel/signal.c 	int	__sighandler_t	-	-	-
49	sys_geteuid	kernel/sched.c 	-	-	-	-	-
50	sys_getegid	kernel/sched.c 	-	-	-	-	-
51	sys_acct	kernel/acct.c 	const char *	-	-	-	-
52	sys_umount	fs/super.c 	char *	int	-	-	-
54	sys_ioctl	fs/ioctl.c 	unsigned int	unsigned int	unsigned long	-	-
55	sys_fcntl	fs/fcntl.c 	unsigned int	unsigned int	unsigned long	-	-
57	sys_setpgid	kernel/sys.c 	pid_t	pid_t	-	-	-
59	sys_olduname	arch/i386/kernel/sys_i386.c 	struct oldold_utsname *	-	-	-	-
60	sys_umask	kernel/sys.c 	int	-	-	-	-
61	sys_chroot	fs/open.c 	const char *	-	-	-	-
62	sys_ustat	fs/super.c 	dev_t	struct ustat *	-	-	-
63	sys_dup2	fs/fcntl.c 	unsigned int	unsigned int	-	-	-
64	sys_getppid	kernel/sched.c 	-	-	-	-	-
65	sys_getpgrp	kernel/sys.c 	-	-	-	-	-
66	sys_setsid	kernel/sys.c 	-	-	-	-	-
67	sys_sigaction	arch/i386/kernel/signal.c 	int	const struct old_sigaction *	struct old_sigaction *	-	-
68	sys_sgetmask	kernel/signal.c 	-	-	-	-	-
69	sys_ssetmask	kernel/signal.c 	int	-	-	-	-
70	sys_setreuid	kernel/sys.c 	uid_t	uid_t	-	-	-
71	sys_setregid	kernel/sys.c 	gid_t	gid_t	-	-	-
72	sys_sigsuspend	arch/i386/kernel/signal.c 	int	int	old_sigset_t	-	-
73	sys_sigpending	kernel/signal.c 	old_sigset_t *	-	-	-	-
74	sys_sethostname	kernel/sys.c 	char *	int	-	-	-
75	sys_setrlimit	kernel/sys.c 	unsigned int	struct rlimit *	-	-	-
76	sys_getrlimit	kernel/sys.c 	unsigned int	struct rlimit *	-	-	-
77	sys_getrusage	kernel/sys.c 	int	struct rusage *	-	-	-
78	sys_gettimeofday	kernel/time.c 	struct timeval *	struct timezone *	-	-	-
79	sys_settimeofday	kernel/time.c 	struct timeval *	struct timezone *	-	-	-
80	sys_getgroups	kernel/sys.c 	int	gid_t *	-	-	-
81	sys_setgroups	kernel/sys.c 	int	gid_t *	-	-	-
82	old_select	arch/i386/kernel/sys_i386.c 	struct sel_arg_struct *	-	-	-	-
83	sys_symlink	fs/namei.c 	const char *	const char *	-	-	-
84	sys_lstat	fs/stat.c 	char *	struct __old_kernel_stat *	-	-	-
85	sys_readlink	fs/stat.c 	const char *	char *	int	-	-
86	sys_uselib	fs/exec.c 	const char *	-	-	-	-
87	sys_swapon	mm/swapfile.c 	const char *	int	-	-	-
88	sys_reboot	kernel/sys.c 	int	int	int	void *	-
89	old_readdir	fs/readdir.c 	unsigned int	void *	unsigned int	-	-
90	old_mmap	arch/i386/kernel/sys_i386.c 	struct mmap_arg_struct *	-	-	-	-
91	sys_munmap	mm/mmap.c 	unsigned long	size_t	-	-	-
92	sys_truncate	fs/open.c 	const char *	unsigned long	-	-	-
93	sys_ftruncate	fs/open.c 	unsigned int	unsigned long	-	-	-
94	sys_fchmod	fs/open.c 	unsigned int	mode_t	-	-	-
95	sys_fchown	fs/open.c 	unsigned int	uid_t	gid_t	-	-
96	sys_getpriority	kernel/sys.c 	int	int	-	-	-
97	sys_setpriority	kernel/sys.c 	int	int	int	-	-
99	sys_statfs	fs/open.c 	const char *	struct statfs *	-	-	-
100	sys_fstatfs	fs/open.c 	unsigned int	struct statfs *	-	-	-
101	sys_ioperm	arch/i386/kernel/ioport.c 	unsigned long	unsigned long	int	-	-
102	sys_socketcall	net/socket.c 	int	unsigned long *	-	-	-
103	sys_syslog	kernel/printk.c 	int	char *	int	-	-
104	sys_setitimer	kernel/itimer.c 	int	struct itimerval *	struct itimerval *	-	-
105	sys_getitimer	kernel/itimer.c 	int	struct itimerval *	-	-	-
106	sys_newstat	fs/stat.c 	char *	struct stat *	-	-	-
107	sys_newlstat	fs/stat.c 	char *	struct stat *	-	-	-
108	sys_newfstat	fs/stat.c 	unsigned int	struct stat *	-	-	-
109	sys_uname	arch/i386/kernel/sys_i386.c 	struct old_utsname *	-	-	-	-
110	sys_iopl	arch/i386/kernel/ioport.c 	unsigned long	-	-	-	-
111	sys_vhangup	fs/open.c 	-	-	-	-	-
112	sys_idle	arch/i386/kernel/process.c 	-	-	-	-	-
113	sys_vm86old	arch/i386/kernel/vm86.c 	unsigned long	struct vm86plus_struct *	-	-	-
114	sys_wait4	kernel/exit.c 	pid_t	unsigned long *	int options	struct rusage *	-
115	sys_swapoff	mm/swapfile.c 	const char *	-	-	-	-
116	sys_sysinfo	kernel/info.c 	struct sysinfo *	-	-	-	-
117	sys_ipc (*Note)	arch/i386/kernel/sys_i386.c 	uint	int	int	int	void *
118	sys_fsync	fs/buffer.c 	unsigned int	-	-	-	-
119	sys_sigreturn	arch/i386/kernel/signal.c 	unsigned long	-	-	-	-
120	sys_clone	arch/i386/kernel/process.c 	struct pt_regs	-	-	-	-
121	sys_setdomainname	kernel/sys.c 	char *	int	-	-	-
122	sys_newuname	kernel/sys.c 	struct new_utsname *	-	-	-	-
123	sys_modify_ldt	arch/i386/kernel/ldt.c 	int	void *	unsigned long	-	-
124	sys_adjtimex	kernel/time.c 	struct timex *	-	-	-	-
125	sys_mprotect	mm/mprotect.c 	unsigned long	size_t	unsigned long	-	-
126	sys_sigprocmask	kernel/signal.c 	int	old_sigset_t *	old_sigset_t *	-	-
127	sys_create_module	kernel/module.c 	const char *	size_t	-	-	-
128	sys_init_module	kernel/module.c 	const char *	struct module *	-	-	-
129	sys_delete_module	kernel/module.c 	const char *	-	-	-	-
130	sys_get_kernel_syms	kernel/module.c 	struct kernel_sym *	-	-	-	-
131	sys_quotactl	fs/dquot.c 	int	const char *	int	caddr_t	-
132	sys_getpgid	kernel/sys.c 	pid_t	-	-	-	-
133	sys_fchdir	fs/open.c 	unsigned int	-	-	-	-
134	sys_bdflush	fs/buffer.c 	int	long	-	-	-
135	sys_sysfs	fs/super.c 	int	unsigned long	unsigned long	-	-
136	sys_personality	kernel/exec_domain.c 	unsigned long	-	-	-	-
138	sys_setfsuid	kernel/sys.c 	uid_t	-	-	-	-
139	sys_setfsgid	kernel/sys.c 	gid_t	-	-	-	-
140	sys_llseek	fs/read_write.c 	unsigned int	unsigned long	unsigned long	loff_t *	unsigned int
141	sys_getdents	fs/readdir.c 	unsigned int	void *	unsigned int	-	-
142	sys_select	fs/select.c 	int	fd_set *	fd_set *	fd_set *	struct timeval *
143	sys_flock	fs/locks.c 	unsigned int	unsigned int	-	-	-
144	sys_msync	mm/filemap.c 	unsigned long	size_t	int	-	-
145	sys_readv	fs/read_write.c 	unsigned long	const struct iovec *	unsigned long	-	-
146	sys_writev	fs/read_write.c 	unsigned long	const struct iovec *	unsigned long	-	-
147	sys_getsid	kernel/sys.c 	pid_t	-	-	-	-
148	sys_fdatasync	fs/buffer.c 	unsigned int	-	-	-	-
149	sys_sysctl	kernel/sysctl.c 	struct __sysctl_args *	-	-	-	-
150	sys_mlock	mm/mlock.c 	unsigned long	size_t	-	-	-
151	sys_munlock	mm/mlock.c 	unsigned long	size_t	-	-	-
152	sys_mlockall	mm/mlock.c 	int	-	-	-	-
153	sys_munlockall	mm/mlock.c 	-	-	-	-	-
154	sys_sched_setparam	kernel/sched.c 	pid_t	struct sched_param *	-	-	-
155	sys_sched_getparam	kernel/sched.c 	pid_t	struct sched_param *	-	-	-
156	sys_sched_setscheduler	kernel/sched.c 	pid_t	int	struct sched_param *	-	-
157	sys_sched_getscheduler	kernel/sched.c 	pid_t	-	-	-	-
158	sys_sched_yield	kernel/sched.c 	-	-	-	-	-
159	sys_sched_get_priority_max	kernel/sched.c 	int	-	-	-	-
160	sys_sched_get_priority_min	kernel/sched.c 	int	-	-	-	-
161	sys_sched_rr_get_interval	kernel/sched.c 	pid_t	struct timespec *	-	-	-
162	sys_nanosleep	kernel/sched.c 	struct timespec *	struct timespec *	-	-	-
163	sys_mremap	mm/mremap.c 	unsigned long	unsigned long	unsigned long	unsigned long	-
164	sys_setresuid	kernel/sys.c 	uid_t	uid_t	uid_t	-	-
165	sys_getresuid	kernel/sys.c 	uid_t *	uid_t *	uid_t *	-	-
166	sys_vm86	arch/i386/kernel/vm86.c 	struct vm86_struct *	-	-	-	-
167	sys_query_module	kernel/module.c 	const char *	int	char *	size_t	size_t *
168	sys_poll	fs/select.c 	struct pollfd *	unsigned int	long	-	-
169	sys_nfsservctl	fs/filesystems.c 	int	void *	void *	-	-
170	sys_setresgid	kernel/sys.c 	gid_t	gid_t	gid_t	-	-
171	sys_getresgid	kernel/sys.c 	gid_t *	gid_t *	gid_t *	-	-
172	sys_prctl	kernel/sys.c 	int	unsigned long	unsigned long	unsigned long	unsigned long
173	sys_rt_sigreturn	arch/i386/kernel/signal.c 	unsigned long	-	-	-	-
174	sys_rt_sigaction	kernel/signal.c 	int	const struct sigaction *	struct sigaction *	size_t	-
175	sys_rt_sigprocmask	kernel/signal.c 	int	sigset_t *	sigset_t *	size_t	-
176	sys_rt_sigpending	kernel/signal.c 	sigset_t *	size_t	-	-	-
177	sys_rt_sigtimedwait	kernel/signal.c 	const sigset_t *	siginfo_t *	const struct timespec *	size_t	-
178	sys_rt_sigqueueinfo	kernel/signal.c 	int	int	siginfo_t *	-	-
179	sys_rt_sigsuspend	arch/i386/kernel/signal.c 	sigset_t *	size_t	-	-	-
180	sys_pread	fs/read_write.c 	unsigned int	char *	size_t	loff_t	-
181	sys_pwrite	fs/read_write.c 	unsigned int	const char *	size_t	loff_t	-
182	sys_chown	fs/open.c 	const char *	uid_t	gid_t	-	-
183	sys_getcwd	fs/dcache.c 	char *	unsigned long	-	-	-
184	sys_capget	kernel/capability.c 	cap_user_header_t	cap_user_data_t	-	-	-
185	sys_capset	kernel/capability.c 	cap_user_header_t	const cap_user_data_t	-	-	-
186	sys_sigaltstack	arch/i386/kernel/signal.c 	const stack_t *	stack_t *	-	-	-
187	sys_sendfile	mm/filemap.c 	int	int	off_t *	size_t	-
190	sys_vfork
