#ifndef C_OS_SYS_SIGINFO_H
#define C_OS_SYS_SIGINFO_H
union sigval {
	int sival_int;
	void *sival_ptr;
};

typedef struct {
	int si_signo;
	int si_code;
	union sigval si_value;
	int si_errno;
	pid_t si_pid;
	uid_t si_uid;
	void *si_addr;
	int si_status;
	int si_band;
} siginfo_t;
#endif
