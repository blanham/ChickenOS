/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/


/*
 * Copyright (c) 1996, by Sun Microsystems, Inc.
 * All rights reserved.
 */

/*
 * Portions Copyright (c) 2005 Gunnar Ritter, Freiburg i. Br., Germany
 *
 * Sccsid @(#)jobs.c	1.13 (gritter) 6/23/05
 */
/* from OpenSolaris "jobs.c	1.25	05/06/08 SMI" */
/*
 * Job control for UNIX Shell
 */

#include	<termios.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<fcntl.h>
#include	<errno.h>
#include	"defs.h"

/*
 * one of these for each active job
 */

struct job
{
	struct job *j_nxtp;	/* next job in job ID order */
	struct job *j_curp;	/* next job in job currency order */
	struct termios j_stty;	/* termio save area when job stops */
	pid_t  j_pid;		/* job leader's process ID */
	pid_t  j_pgid;		/* job's process group ID */
	pid_t  j_tgid;		/* job's foreground process group ID */
	unsigned int   j_jid;	/* job ID */
	unsigned short j_xval;	/* exit code, or exit or stop signal */
	unsigned short j_flag;	/* various status flags defined below */
	char  *j_pwd;		/* job's working directory */
	char  *j_cmd;		/* cmd used to invoke this job */
};

/* defines for j_flag */

#define	J_DUMPED	0001	/* job has core dumped */
#define	J_NOTIFY	0002	/* job has changed status */
#define	J_SAVETTY	0004	/* job was stopped in foreground, and its */
				/*   termio settings were saved */
#define	J_STOPPED	0010	/* job has been stopped */
#define	J_SIGNALED	0020	/* job has received signal; j_xval has it */
#define	J_DONE		0040	/* job has finished */
#define	J_RUNNING	0100	/* job is currently running */
#define	J_FOREGND	0200	/* job was put in foreground by shell */

/* options to the printjob() function defined below */

#define	PR_CUR		00001	/* print job currency ('+', '-', or ' ') */
#define	PR_JID		00002	/* print job ID				 */
#define	PR_PGID		00004	/* print job's process group ID		 */
#define	PR_STAT		00010	/* print status obtained from wait	 */
#define	PR_CMD		00020	/* print cmd that invoked job		 */
#define	PR_AMP		00040	/* print a '&' if in the background	 */
#define	PR_PWD		00100	/* print jobs present working directory	 */

#define	PR_DFL		(PR_CUR|PR_JID|PR_STAT|PR_CMD) /* default options */
#define	PR_LONG		(PR_DFL|PR_PGID|PR_PWD)	/* long options */

static struct termios 	mystty;	 /* default termio settings		 */
static int 		eofflg,
			jobcnt,	 /* number of active jobs		 */
			jobdone, /* number of active but finished jobs	 */
			jobnote; /* jobs requiring notification		 */
static pid_t		svpgid,	 /* saved process group ID		 */
			svtgid;	 /* saved foreground process group ID	 */
static struct job 	*jobcur, /* active jobs listed in currency order */
			**nextjob,
			*thisjob,
			*joblst; /* active jobs listed in job ID order	 */

static void	printjob(struct job *jp, int propts);

static struct job *
pgid2job(register pid_t pgid)
{
	register struct job *jp;

	for (jp = joblst; jp != 0 && jp->j_pid != pgid; jp = jp->j_nxtp)
		continue;

	return (jp);
}

static struct job *
str2job(register char *cmd, register char *job, int mustbejob)
{
	register struct job *jp, *njp;
	register int i;

	if (*job != '%')
		jp = pgid2job(stoi(job));
	else if (*++job == 0 || *job == '+' || *job == '%' || *job == '-') {
		jp = jobcur;
		if (*job == '-' && jp)
			jp = jp->j_curp;
	} else if (*job >= '0' && *job <= '9') {
		i = stoi(job);
		for (jp = joblst; jp && jp->j_jid != i; jp = jp->j_nxtp)
			continue;
	} else if (*job == '?') {
		register int j;
		register char *p;
		i = strlen(++job);
		jp = 0;
		for (njp = jobcur; njp; njp = njp->j_curp) {
			if (njp->j_jid == 0)
				continue;
			for (p = njp->j_cmd, j = strlen(p); j >= i; p++, j--) {
				if (strncmp(job, p, i) == 0) {
					if (jp != 0)
						failed(cmd, ambiguous);
					jp = njp;
					break;
				}
			}
		}
	} else {
		i = strlen(job);
		jp = 0;
		for (njp = jobcur; njp; njp = njp->j_curp) {
			if (njp->j_jid == 0)
				continue;
			if (strncmp(job, njp->j_cmd, i) == 0) {
				if (jp != 0)
					failed(cmd, ambiguous);
				jp = njp;
			}
		}
	}

	if (mustbejob && (jp == 0 || jp->j_jid == 0))
		failed(cmd, nosuchjob);

	return (jp);
}

static void 
freejob(register struct job *jp)
{
	register struct job **njp;
	register struct job **cjp;

	for (njp = &joblst; *njp != jp; njp = &(*njp)->j_nxtp)
		continue;

	for (cjp = &jobcur; *cjp != jp; cjp = &(*cjp)->j_curp)
		continue;

	*njp = jp->j_nxtp;
	*cjp = jp->j_curp;
	free(jp);
	jobcnt--;
	jobdone--;
}

/*
 * Collect the foreground job.
 * Used in the case where the subshell wants
 * to exit, but needs to wait until the fg job
 * is done.
 */
void
collect_fg_job(void)
{
	register struct job *jp;
	register pid_t pid;
	int stat;

	for (jp = joblst; jp; jp = jp->j_nxtp)
		if (jp->j_flag & J_FOREGND)
			break;

	if (!jp)
		/* no foreground job */
		return;

	/*
	 * Wait on fg job until wait succeeds
	 * or it fails due to no waitable children.
	 */

	while (1) {
		errno = 0;
		pid = waitpid(jp->j_pid, &stat, 0);
		if (pid == jp->j_pid || (pid == -1 && errno == ECHILD))
			break;
	}
}

/*
 * analyze the status of a job
 */

static int
statjob(register struct job *jp, int stat, int fg, int rc)
{
	pid_t tgid;
	int done = 0;

#ifdef	notdef
	if (WIFCONTINUED(stat)) {
		if (jp->j_flag & J_STOPPED) {
			jp->j_flag &= ~(J_STOPPED|J_SIGNALED|J_SAVETTY);
			jp->j_flag |= J_RUNNING;
			if (!fg && jp->j_jid) {
				jp->j_flag |= J_NOTIFY;
				jobnote++;
			}
		}
	} else
#endif	/* notdef */
		if (WIFSTOPPED(stat)) {
		jp->j_xval = WSTOPSIG(stat);
		jp->j_flag &= ~J_RUNNING;
		jp->j_flag |= (J_SIGNALED|J_STOPPED);
		jp->j_pgid = getpgid(jp->j_pid);
		jp->j_tgid = jp->j_pgid;
		if (fg) {
			if (tgid = settgid(mypgid, jp->j_pgid))
				jp->j_tgid = tgid;
			else {
				jp->j_flag |= J_SAVETTY;
				tcgetattr(0, &jp->j_stty);
				tcsetattr(0, TCSANOW, &mystty);
			}
		}
		if (jp->j_jid) {
			jp->j_flag |= J_NOTIFY;
			jobnote++;
		}
	} else {
		jp->j_flag &= ~J_RUNNING;
		jp->j_flag |= J_DONE;
		done++;
		jobdone++;
		if (WIFSIGNALED(stat)) {
			jp->j_xval = WTERMSIG(stat);
			jp->j_flag |= J_SIGNALED;
#ifdef	WCOREDUMP
			if (WCOREDUMP(stat))
				jp->j_flag |= J_DUMPED;
#endif	/* WCOREDUMP */
			if (!fg || jp->j_xval != SIGINT) {
				jp->j_flag |= J_NOTIFY;
				jobnote++;
			}
		} else { /* WIFEXITED */
			jp->j_xval = WEXITSTATUS(stat);
			jp->j_flag &= ~J_SIGNALED;
			if (!fg && jp->j_jid) {
				jp->j_flag |= J_NOTIFY;
				jobnote++;
			}
		}
		if (fg) {
			if (!settgid(mypgid, jp->j_pgid) ||
			    !settgid(mypgid, getpgid(jp->j_pid)))
				tcgetattr(0, &mystty);
		}
	}
	if (rc) {
		exitval = jp->j_xval;
		if (jp->j_flag & J_SIGNALED)
			exitval |= SIGFLG;
		exitset();
	}
	if (done && !(jp->j_flag & J_NOTIFY))
		freejob(jp);
	return (done);
}

/*
 * collect the status of jobs that have recently exited or stopped -
 * if wnohang == WNOHANG, wait until error, or all jobs are accounted for;
 *
 * called after each command is executed, with wnohang == 0, and as part
 * of "wait" builtin with wnohang == WNOHANG
 *
 * We do not need to call chktrap here if waitpid(2) is called with
 * wnohang == 0, because that only happens from syswait() which is called
 * from builtin() where chktrap() is already called.
 */

static void 
collectjobs(int wnohang)
{
	pid_t pid;
	register struct job *jp;
	int stat, n;
	int wflags;

	if ((flags & (monitorflg|jcflg|jcoff)) == (monitorflg|jcflg))
		wflags = WUNTRACED;
	else
		wflags = 0;

	for (n = jobcnt - jobdone; n > 0; n--) {
		if ((pid = waitpid(-1, &stat, wnohang|wflags)) <= 0)
			break;
		if (jp = pgid2job(pid))
			statjob(jp, stat, 0, 0);
	}

}

void 
freejobs(void)
{
	register struct job *jp;

	collectjobs(WNOHANG);

	if (jobnote) {
		register int savefd = setb(2);
		for (jp = joblst; jp; jp = jp->j_nxtp) {
			if (jp->j_flag & J_NOTIFY) {
				if (jp->j_jid)
					printjob(jp, PR_DFL);
				else if (jp->j_flag & J_FOREGND)
					printjob(jp, PR_STAT);
				else
					printjob(jp, PR_STAT|PR_PGID);
			}
		}
		setb(savefd);
	}

	if (jobdone) {
		for (jp = joblst; jp; jp = jp->j_nxtp) {
			if (jp->j_flag & J_DONE)
				freejob(jp);
		}
	}
}

static void 
waitjob(register struct job *jp)
{
	int stat;
	int done;
	pid_t pid = jp->j_pid;
	int wflags;
	int ret = 0;
	int	err = 0;

	if ((flags & (monitorflg|jcflg|jcoff)) == (monitorflg|jcflg))
		wflags = WUNTRACED;
	else
		wflags = 0;
	do {
		errno = 0;
		ret = waitpid(pid, &stat, wflags);
		err = errno;
		if (ret == -1 && err == ECHILD) {
			stat = 0;
			break;
		}
	} while (ret != pid);

	done = statjob(jp, stat, 1, 1);
	if (done && exitval && (flags & errflg))
		exitsh(exitval);
	flags |= eflag;
}

/*
 * modify the foreground process group to *new* only if the
 * current foreground process group is equal to *expected*
 * Some systems return the value of a non-existing process
 * group id with tcgetpgrp() after the last process of the
 * foreground process group has exited. In this case, behave
 * as if it had been the expected one.
 */

int
settgid(pid_t new, pid_t expected)
{
	register pid_t current = tcgetpgrp(0);

	if (current != expected && (kill(-current, 0) == 0 || errno == EPERM))
		return (current);

	if (new != current)
		tcsetpgrp(0, new);

	return (0);
}

static void 
restartjob(register struct job *jp, int fg)
{
	if (jp != jobcur) {
		register struct job *t;
		for (t = jobcur; t->j_curp != jp; t = t->j_curp);
		t->j_curp = jp->j_curp;
		jp->j_curp = jobcur;
		jobcur = jp;
	}
	if (fg) {
		if (jp->j_flag & J_SAVETTY) {
			jp->j_stty.c_lflag &= ~TOSTOP;
			jp->j_stty.c_lflag |= (mystty.c_lflag&TOSTOP);
			jp->j_stty.c_cc[VSUSP] = mystty.c_cc[VSUSP];
#ifdef	VDSUSP
			jp->j_stty.c_cc[VDSUSP] = mystty.c_cc[VDSUSP];
#endif	/* VDSUSP */
			tcsetattr(0, TCSADRAIN, &jp->j_stty);
		}
		settgid(jp->j_tgid, mypgid);
	}
	kill(-(jp->j_pgid), SIGCONT);
	if (jp->j_tgid != jp->j_pgid)
		kill(-(jp->j_tgid), SIGCONT);
	jp->j_flag &= ~(J_STOPPED|J_SIGNALED|J_SAVETTY);
	jp->j_flag |= J_RUNNING;
	if (fg)  {
		jp->j_flag |= J_FOREGND;
		printjob(jp, PR_JID|PR_CMD);
		waitjob(jp);
	} else {
		jp->j_flag &= ~J_FOREGND;
		printjob(jp, PR_JID|PR_CMD|PR_AMP);
	}
}

static void
printjob(register struct job *jp, int propts)
{
	int sp = 0;

	if (jp->j_flag & J_NOTIFY) {
		jobnote--;
		jp->j_flag &= ~J_NOTIFY;
	}

	if (propts & PR_JID) {
		prc_buff('[');
		prn_buff(jp->j_jid);
		prc_buff(']');
		sp = 1;
	}

	if (propts & PR_CUR) {
		while (sp-- > 0)
			prc_buff(SPACE);
		sp = 1;
		if (jobcur == jp)
			prc_buff('+');
		else if (jobcur != 0 && jobcur->j_curp == jp)
			prc_buff('-');
		else
			sp++;
	}

	if (propts & PR_PGID) {
		while (sp-- > 0)
			prc_buff(SPACE);
		prn_buff(jp->j_pid);
		sp = 1;
	}

	if (propts & PR_STAT) {
		while (sp-- > 0)
			prc_buff(SPACE);
		sp = 28;
		if (jp->j_flag & J_SIGNALED) {
			const char *sigstr;
			if ((sigstr = str_signal(jp->j_xval)) != NULL) {
				sp -= strlen(sigstr);
				prs_buff(sigstr);
			} else {
				itos(jp->j_xval);
				sp -= strlen(numbuf) + 7;
				prs_buff("Signal ");
				prs_buff(numbuf);
			}
			if (jp->j_flag & J_DUMPED) {
				sp -= strlen(coredump);
				prs_buff(coredump);
			}
		} else if (jp->j_flag & J_DONE) {
			itos(jp->j_xval);
			sp -= strlen(exited) + strlen(numbuf) + 2;
			prs_buff(exited);
			prc_buff('(');
			itos(jp->j_xval);
			prs_buff(numbuf);
			prc_buff(')');
		} else {
			sp -= strlen(running);
			prs_buff(running);
		}
		if (sp < 1)
			sp = 1;
	}

	if (propts & PR_CMD) {
		while (sp-- > 0)
			prc_buff(SPACE);
		prs_buff(jp->j_cmd);
		sp = 1;
	}

	if (propts & PR_AMP) {
		while (sp-- > 0)
			prc_buff(SPACE);
		prc_buff('&');
		sp = 1;
	}

	if (propts & PR_PWD) {
		while (sp-- > 0)
			prc_buff(SPACE);
		prs_buff("(wd: ");
		prs_buff(jp->j_pwd);
		prc_buff(')');
	}

	prc_buff(NL);
	flushb();

}


/*
 * called to initialize job control for each new input file to the shell,
 * and after the "exec" builtin
 */

void 
startjobs(void)
{
	svpgid = mypgid;

	if (tcgetattr(0, &mystty) == -1 || (svtgid = tcgetpgrp(0)) == -1) {
		flags &= ~jcflg;
		return;
	}

	flags |= jcflg;

	handle(SIGTTOU, SIG_IGN);
	handle(SIGTSTP, SIG_DFL);

	if (mysid != mypgid) {
		setpgid(0, 0);
		mypgid = mypid;
		settgid(mypgid, svpgid);
	}

}

int 
endjobs(int check_if)
{
	if ((flags & (jcoff|jcflg)) != jcflg)
		return (1);

	if (check_if && jobcnt && eofflg++ == 0) {
		register struct job *jp;
		if (check_if & JOB_STOPPED) {
			for (jp = joblst; jp; jp = jp->j_nxtp) {
				if (jp->j_jid && (jp->j_flag & J_STOPPED)) {
					prs(jobsstopped);
					prc(NL);
					return (0);
				}
			}
		}
		if (check_if & JOB_RUNNING) {
			for (jp = joblst; jp; jp = jp->j_nxtp) {
				if (jp->j_jid && (jp->j_flag & J_RUNNING)) {
					prs(jobsrunning);
					prc(NL);
					return (0);
				}
			}
		}
	}

	if (svpgid != mypgid) {
		settgid(svtgid, mypgid);
		setpgid(0, svpgid);
	}

	return (1);
}


/*
 * called by the shell to reserve a job slot for a job about to be spawned
 */

void 
deallocjob(void)
{
	free(thisjob);
	jobcnt--;
}

void
allocjob(register char *cmd, register unsigned char *cwd, int monitor)
{
	register struct job *jp, **jpp;
	register int jid, cmdlen, cwdlen;

	cmdlen = strlen(cmd) + 1;
	if (cmd[cmdlen-2] == '&') {
		cmd[cmdlen-3] = 0;
		cmdlen -= 2;
	}
	cwdlen = strlen(cwd) + 1;
	jp = (struct job *) alloc(sizeof (struct job) + cmdlen + cwdlen);
	if (jp == 0)
		error(nostack);
	jobcnt++;
	jp->j_cmd = ((char *)jp) + sizeof (struct job);
	strcpy(jp->j_cmd, cmd);
	jp->j_pwd = jp->j_cmd + cmdlen;
	strcpy(jp->j_pwd, cwd);

	jpp = &joblst;

	if (monitor) {
		for (; *jpp; jpp = &(*jpp)->j_nxtp)
			if ((*jpp)->j_jid != 0)
				break;
		for (jid = 1; *jpp; jpp = &(*jpp)->j_nxtp, jid++)
			if ((*jpp)->j_jid != jid)
				break;
	} else
		jid = 0;

	jp->j_jid = jid;
	nextjob = jpp;
	thisjob = jp;
}

void
clearjobs(void)
{
	register struct job *jp, *sjp;

	for (jp = joblst; jp; jp = sjp) {
		sjp = jp->j_nxtp;
		free(jp);
	}
	joblst = NULL;
	jobcnt = 0;
	jobnote = 0;
	jobdone = 0;

}

void
makejob(int monitor, int fg)
{
	if (monitor) {
		mypgid = mypid;
		setpgid(0, 0);
		if (fg)
			tcsetpgrp(0, mypid);
		handle(SIGTTOU, SIG_DFL);
		handle(SIGTSTP, SIG_DFL);
	} else if (!fg) {
#ifdef NICE
		nice(NICE);
#endif
		handle(SIGTTIN, SIG_IGN);
		handle(SIGINT,  SIG_IGN);
		handle(SIGQUIT, SIG_IGN);
		if (!ioset)
			renamef(chkopen(devnull, 0), 0);
	}
}

/*
 * called by the shell after job has been spawned, to fill in the
 * job slot, and wait for the job if in the foreground
 */

void
postjob(pid_t pid, int fg)
{

	register int propts;

	thisjob->j_nxtp = *nextjob;
	*nextjob = thisjob;
	thisjob->j_curp = jobcur;
	jobcur = thisjob;

	if (thisjob->j_jid) {
		thisjob->j_pgid = pid;
		propts = PR_JID|PR_PGID;
	} else {
		thisjob->j_pgid = mypgid;
		propts = PR_PGID;
	}

	thisjob->j_flag = J_RUNNING;
	thisjob->j_tgid = thisjob->j_pgid;
	thisjob->j_pid = pid;
	eofflg = 0;

	if (fg) {
		thisjob->j_flag |= J_FOREGND;
		waitjob(thisjob);
	} else  {
		if (flags & ttyflg)
			printjob(thisjob, propts);
		assnum(&pcsadr, (long)pid);
	}
}

/*
 * the builtin "jobs" command
 */

void 
sysjobs(int argc, char *argv[])
{
	register char *cmd = *argv;
	register struct job *jp;
	register int propts, c;
	int savoptind = optind;
	int loptind = -1;
	int savopterr = opterr;
	int savsp = getopt_sp;
	char *savoptarg = optarg;
	optind = 1;
	opterr = 0;
	getopt_sp = 1;
	propts = 0;

	if ((flags & jcflg) == 0)
		failed(cmd, nojc);

	while ((c = getopt(argc, argv, "lpx")) != -1) {
		if (propts) {
			failure(usage, jobsuse);
			goto err;
		}
		switch (c) {
			case 'x':
				propts = -1;
				break;
			case 'p':
				propts = PR_PGID;
				break;
			case 'l':
				propts = PR_LONG;
				break;
			case '?':
				failure(usage, jobsuse);
				goto err;
		}
	}

	loptind = optind;
err:
	optind = savoptind;
	optarg = savoptarg;
	opterr = savopterr;
	getopt_sp = savsp;
	if (loptind == -1)
		return;

	if (propts == -1) {
		register unsigned char *bp;
		register char *cp;
		unsigned char *savebp;
		for (savebp = bp = locstak(); loptind < argc; loptind++) {
			cp = argv[loptind];
			if (*cp == '%') {
				jp = str2job(cmd, cp, 1);
				itos(jp->j_pid);
				cp = (char *)numbuf;
			}
			while (*cp) {
				if (bp >= brkend)
					growstak(bp);
				*bp++ = *cp++;
			}
			if (bp >= brkend)
				growstak(bp);
			*bp++ = SPACE;
		}
		endstak(bp);
		execexp(savebp, 0);
		return;
	}

	collectjobs(WNOHANG);

	if (propts == 0)
		propts = PR_DFL;

	if (loptind == argc) {
		for (jp = joblst; jp; jp = jp->j_nxtp) {
			if (jp->j_jid)
				printjob(jp, propts);
		}
	} else do
		printjob(str2job(cmd, argv[loptind++], 1), propts);
	while (loptind < argc);

}

/*
 * the builtin "fg" and "bg" commands
 */

void
sysfgbg(int argc, char *argv[])
{
	register char *cmd = *argv;
	register int fg;

	if ((flags & jcflg) == 0)
		failed(cmd, nojc);

	fg = eq("fg", cmd);

	if (*++argv == 0) {
		struct job *jp;
		for (jp = jobcur; ; jp = jp->j_curp) {
			if (jp == 0)
				failed(cmd, nocurjob);
			if (jp->j_jid)
				break;
		}
		restartjob(jp, fg);
	}

	else do
		restartjob(str2job(cmd, *argv, 1), fg);
	while (*++argv);

}

/*
 * the builtin "wait" commands
 */

void 
syswait(int argc, char *argv[])
{
	register char *cmd = *argv;
	register struct job *jp;
	int stat;
	int wflags;

	if ((flags & (monitorflg|jcflg|jcoff)) == (monitorflg|jcflg))
		wflags = WUNTRACED;
	else
		wflags = 0;

	if (argc == 1)
		collectjobs(0);
	else while (--argc) {
		if ((jp = str2job(cmd, *++argv, 0)) == 0)
			continue;
		if (!(jp->j_flag & J_RUNNING))
			continue;
		if (waitpid(jp->j_pid, &stat, wflags) <= 0)
			break;
		statjob(jp, stat, 0, 1);
	}
}

static void
sigv(char *cmd, int sig, char *args)
{
	int pgrp = 0;
	int stopme = 0;
	pid_t id;

	if (*args == '%') {
		register struct job *jp;
		jp = str2job(cmd, args, 1);
		id = jp->j_pgid;
		pgrp++;
	} else {
		if (*args == '-') {
			pgrp++;
			args++;
		}
		id = 0;
		do {
			if (*args < '0' || *args > '9') {
				failure(cmd, badid);
				return;
			}
			id = (id * 10) + (*args - '0');
		} while (*++args);
		if (id == 0) {
			id = mypgid;
			pgrp++;
		}
	}

	if (sig == SIGSTOP) {
		if (id == mysid || id == mypid && mypgid == mysid) {
			failure(cmd, loginsh);
			return;
		}
		if (id == mypgid && mypgid != svpgid) {
			settgid(svtgid, mypgid);
			setpgid(0, svpgid);
			stopme++;
		}
	}

	if (pgrp)
		id = -id;

	if (kill(id, sig) < 0) {

		switch (errno) {
			case EPERM:
				failure(cmd, eacces);
				break;

			case EINVAL:
				failure(cmd, badsig);
				break;

			default:
				if (pgrp)
					failure(cmd, nosuchpgid);
				else
					failure(cmd, nosuchpid);
				break;
		}

	} else if (sig == SIGTERM && pgrp)
		kill(id, SIGCONT);

	if (stopme) {
		setpgid(0, mypgid);
		settgid(mypgid, svpgid);
	}

}

void
sysstop(int argc, char *argv[])
{
	char *cmd = *argv;
	if (argc <= 1)
		failed(usage, stopuse);
	while (*++argv)
		sigv(cmd, SIGSTOP, *argv);
}

void
syskill(int argc, char *argv[])
{
	char *cmd = *argv;
	int sig = SIGTERM;

	if (argc == 1) {
		failure(usage, killuse);
		return;
	}

	if (argv[1][0] == '-') {

		if (argc == 2) {

			register int i;
			register int cnt = 0;
			register char sep = 0;
			char buf[12];

			if (!eq(argv[1], "-l")) {
				failure(usage, killuse);
				return;
			}

			for (i = 1; i < MAXTRAP; i++) {
				if (sig_2_str(i, buf) < 0)
					continue;
				if (sep)
					prc_buff(sep);
				prs_buff(buf);
				if ((flags & ttyflg) && (++cnt % 10))
					sep = TAB;
				else
					sep = NL;
			}
			prc_buff(NL);
			return;
		}

		if (str_2_sig(&argv[1][1], &sig)) {
			failure(cmd, badsig);
			return;
		}
		argv++;
	}

	while (*++argv)
		sigv(cmd, sig, *argv);

}

void
syssusp(int argc, char *argv[])
{
	if (argc != 1)
		failed(argv[0], badopt);
	sigv(argv[0], SIGSTOP, "0");
}
