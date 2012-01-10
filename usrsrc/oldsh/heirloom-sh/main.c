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
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*
 * Portions Copyright (c) 2005 Gunnar Ritter, Freiburg i. Br., Germany
 *
 * Sccsid @(#)main.c	1.12 (gritter) 2/26/07
 */


/* from OpenSolaris "main.c	1.34	05/06/08 SMI" */

/*
 * UNIX shell
 */

#include	"defs.h"
#include	"sym.h"
#include	"hash.h"
#include	"timeout.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<fcntl.h>
#include	<time.h>
#include	"dup.h"

#ifdef RES
#include	<sgtty.h>
#endif

#define	setmode	sh_setmode

pid_t mypid, mypgid, mysid;

static BOOL	beenhere = FALSE;
unsigned char		tmpout[20] = "/tmp/sh-";
struct fileblk	stdfile;
struct fileblk *standin = &stdfile;
int mailchk = 0;

static unsigned char	*mailp;
static long	*mod_time = 0;
static BOOL login_shell = FALSE;

#if vax
char **execargs = (char **)(0x7ffffffc);
#endif

#if pdp11
char **execargs = (char **)(-2);
#endif


static void exfile(int);


int 
main(int c, char *v[], char *e[])
{
	int		rflag = ttyflg;
	int		rsflag = 1;	/* local restricted flag */
	unsigned char *flagc = flagadr;
	struct namnod	*n;

	(void) &rflag;
	(void) &rsflag;
	(void) &flagc;

	init_sigval();
	mypid = getpid();
	mypgid = getpgid(mypid);
	mysid = getsid(mypid);

	/*
	 * initialize storage allocation
	 */

	if (stakbot == 0) {
	addblok((unsigned)0);
	}

	/*
	 * If the first character of the last path element of v[0] is "-"
	 * (ex. -sh, or /bin/-sh), this is a login shell
	 */
	if (*simple(v[0]) == '-') {
		signal(SIGXCPU, SIG_DFL);
		signal(SIGXFSZ, SIG_DFL);

		/*
		 * As the previous comment states, this is a login shell.
		 * Therefore, we set the login_shell flag to explicitly
		 * indicate this condition.
		 */
		login_shell = TRUE;
	}

	stdsigs();

	/*
	 * set names from userenv
	 */

	setup_env();

	/*
	 * Do locale processing.
	 */
	setlocale(LC_CTYPE, "");

	/*
	 * 'rsflag' is zero if SHELL variable is
	 *  set in environment and
	 *  the simple file part of the value.
	 *  is rsh
	 */
	if (n = findnam("SHELL"))
	{
		if (eq("rsh", simple(n->namval)))
			rsflag = 0;
	}

	/*
	 * a shell is also restricted if the simple name of argv(0) is
	 * rsh or -rsh in its simple name
	 */

#ifndef RES

	if (c > 0 && (eq("rsh", simple(*v)) || eq("-rsh", simple(*v))))
		rflag = 0;

#endif

	if (eq("jsh", simple(*v)) || eq("-jsh", simple(*v)))
		flags |= monitorflg;

	hcreate();
	set_dotpath();


	/*
	 * look for options
	 * dolc is $#
	 */
	dolc = options(c, (unsigned char **)v);

	if (dolc < 2)
	{
		flags |= stdflg;
		{

			while (*flagc)
				flagc++;
			*flagc++ = STDFLG;
			*flagc = 0;
		}
	}
	if ((flags & stdflg) == 0)
		dolc--;

	if ((flags & privflg) == 0) {
		register uid_t euid;
		register gid_t egid;
		register uid_t ruid;
		register gid_t rgid;

		/*
		 * Determine all of the user's id #'s for this process and
		 * then decide if this shell is being entered as a result
		 * of a fork/exec.
		 * If the effective uid/gid do NOT match and the euid/egid
		 * is < 100 and the egid is NOT 1, reset the uid and gid to
		 * the user originally calling this process.
		 */
		euid = geteuid();
		ruid = getuid();
		egid = getegid();
		rgid = getgid();
		if ((euid != ruid) && (euid < 100))
			setuid(ruid);   /* reset the uid to the orig user */
		if ((egid != rgid) && ((egid < 100) && (egid != 1)))
			setgid(rgid);   /* reset the gid to the orig user */
	}

	dolv = (unsigned char **)v + c - dolc;
	dolc--;

	/*
	 * return here for shell file execution
	 * but not for parenthesis subshells
	 */
	if (setjmp(subshell)) {
		freejobs();
		flags |= subsh;
	}

	/*
	 * number of positional parameters
	 */
	replace(&cmdadr, dolv[0]);	/* cmdadr is $0 */

	/*
	 * set pidname '$$'
	 */
	assnum(&pidadr, (long)mypid);

	/*
	 * set up temp file names
	 */
	settmp();

	/*
	 * default internal field separators
	 * Do not allow importing of IFS from parent shell.
	 * setup_env() may have set anything from parent shell to IFS.
	 * Always set the default ifs to IFS.
	 */
	assign(&ifsnod, sptbnl);

	dfault(&timeoutnod, "0");
	timeoutnod.namflg |= N_RDONLY;

	dfault(&mchknod, MAILCHECK);
	mailchk = stoi(mchknod.namval);

	/* initialize OPTIND for getopt */

	n = lookup("OPTIND");
	assign(n, "1");
	/*
	 * make sure that option parsing starts
	 * at first character
	 */
	getopt_sp = 1;

	/* initialize multibyte information */
	setwidth();

	if ((beenhere++) == FALSE)	/* ? profile */
	{
		if ((login_shell == TRUE) && (flags & privflg) == 0) {

			/* system profile */

#ifndef RES

			if ((input = pathopen(nullstr, sysprofile)) >= 0)
				exfile(rflag);		/* file exists */

#endif
			/* user profile */

			if ((input = pathopen(homenod.namval, profile)) >= 0)
			{
				exfile(rflag);
				flags &= ~ttyflg;
			}
		}
		if (rsflag == 0 || rflag == 0) {
			if ((flags & rshflg) == 0) {
				while (*flagc)
					flagc++;
				*flagc++ = 'r';
				*flagc = '\0';
			}
			flags |= rshflg;
		}

		/*
		 * open input file if specified
		 */
		if (comdiv)
		{
			estabf(comdiv);
			input = -1;
		}
		else
		{
			if (flags & stdflg) {
				input = 0;
			} else {
			/*
			 * If the command file specified by 'cmdadr'
			 * doesn't exist, chkopen() will fail calling
			 * exitsh(). If this is a login shell and
			 * the $HOME/.profile file does not exist, the
			 * above statement "flags &= ~ttyflg" does not
			 * get executed and this makes exitsh() call
			 * longjmp() instead of exiting. longjmp() will
			 * return to the location specified by the last
			 * active jmpbuffer, which is the one set up in
			 * the function exfile() called after the system
			 * profile file is executed (see lines above).
			 * This would cause an infinite loop, because
			 * chkopen() will continue to fail and exitsh()
			 * to call longjmp(). To make exitsh() exit instead
			 * of calling longjmp(), we then set the flag forcexit
			 * at this stage.
			 */

				flags |= forcexit;
				input = chkopen(cmdadr, 0);
				flags &= ~forcexit;
			}

#ifdef ACCT
			if (input != 0)
				preacct(cmdadr);
#endif
			comdiv--;
		}
	}
#ifdef pdp11
	else
		*execargs = (char *)dolv;	/* for `ps' cmd */
#endif


	exfile(0);
	done(0);
	/*NOTREACHED*/
	return 0;
}

static void
exfile(prof)
BOOL	prof;
{
	time_t	mailtime = 0;	/* Must not be a register variable */
	time_t 	curtime = 0;
	long	timeout = 0;

	(void) &mailtime;
	(void) &timeout;
	/*
	 * move input
	 */
	if (input > 0)
	{
		Ldup(input, INIO);
		input = INIO;
	}


	setmode(prof);

	if (setjmp(errshell) && prof)
	{
		close(input);
		endjobs(0);
		return;
	}
	/*
	 * error return here
	 */

	loopcnt = peekc = peekn = 0;
	fndef = 0;
	nohash = 0;
	iopend = 0;

	if (input >= 0)
		initf(input);
	/*
	 * command loop
	 */
	for (;;)
	{
		tdystak(0);
		stakchk();	/* may reduce sbrk */
		exitset();

		if ((flags & prompt) && standin->fstak == 0 && !eof)
		{

			if (mailp)
			{
				time(&curtime);

				if ((curtime - mailtime) >= mailchk)
				{
					chkmail();
					mailtime = curtime;
				}
			}

			/* necessary to print jobs in a timely manner */
			if (trapnote & TRAPSET)
				chktrap();

			prs(ps1nod.namval);

			if ((timeout = atol(timeoutnod.namval)) > 0)
				alarm(timeout);

#ifdef TIME_OUT
			alarm(TIMEOUT);
#endif
			flags |= waiting;

		}

		trapnote = 0;
		peekc = readwc();
		if (eof) {
			if (endjobs(JOB_STOPPED))
				return;
			eof = 0;
		}

		if (timeout > 0) {
			alarm(0);
			timeout = 0;
		}
#ifdef TIME_OUT
		alarm(0);
#endif
		flags &= ~waiting;

		{
			register struct trenod *t;
			t = cmd(NL, MTFLG);
			if (t == NULL && flags & ttyflg)
				freejobs();
			else
				execute(t, 0, eflag, NULL, NULL);
		}

		eof |= (flags & oneflg);

	}
}

void
chkpr(void)
{
	if ((flags & prompt) && standin->fstak == 0)
		prs(ps2nod.namval);
}

void
settmp(void)
{
	int i;
	i = ltos(mypid);
	serial = 0;
	tmpname = movstr(numbuf + i, &tmpout[TMPNAM]);
}

void
Ldup(register int fa, register int fb)
{
#ifdef RES

	dup(fa | DUPFLG, fb);
	close(fa);
	ioctl(fb, FIOCLEX, 0);

#else

	if (fa >= 0) {
		if (fa != fb)
		{
			close(fb);
			fcntl(fa, F_DUPFD, fb); /* normal dup */
			close(fa);
		}
		fcntl(fb, F_SETFD, FD_CLOEXEC);	/* autoclose for fb */
	}

#endif
}


void
chkmail(void)
{
	register unsigned char 	*s = mailp;
	register unsigned char	*save;

	long	*ptr = mod_time;
	unsigned char	*start;
	BOOL	flg;
	struct stat	statb;

	while (*s) {
		start = s;
		save = 0;
		flg = 0;

		while (*s) {
			if (*s != COLON) {
				if (*s == '%' && save == 0)
					save = s;

				s++;
			} else {
				flg = 1;
				*s = 0;
			}
		}

		if (save)
			*save = 0;

		if (*start && stat((const char *)start, &statb) >= 0) {
			if (statb.st_size && *ptr &&
			    statb.st_mtime != *ptr) {
				if (save) {
					prs(save+1);
					newline();
				}
				else
					prs(mailmsg);
			}
			*ptr = statb.st_mtime;
		} else if (*ptr == 0)
			*ptr = 1;

		if (save)
			*save = '%';

		if (flg)
			*s++ = COLON;

		ptr++;
	}
}


void
setmail(unsigned char *mailpath)
{
	register unsigned char	*s = mailpath;
	register int 	cnt = 1;

	long	*ptr;

	free(mod_time);
	if (mailp = mailpath)
	{
		while (*s)
		{
			if (*s == COLON)
				cnt += 1;

			s++;
		}

		ptr = mod_time = (long *)alloc(sizeof (long) * cnt);

		while (cnt)
		{
			*ptr = 0;
			ptr++;
			cnt--;
		}
	}
}

void 
setwidth(void)
{
	unsigned char *name = lookup("LC_ALL")->namval;
	if (!name || !*name)
		name = lookup("LC_CTYPE")->namval;
	if (!name || !*name)
		name = lookup("LANG")->namval;
	if (!name || !*name)
		setlocale(LC_CTYPE, "C");
	else
		setlocale(LC_CTYPE, (const char *)name);
	mb_cur_max = MB_CUR_MAX;
}

void
setmode(int prof)
{
	/*
	 * decide whether interactive
	 */

	if ((flags & intflg) ||
	    ((flags&oneflg) == 0 &&
	    isatty(output) &&
	    isatty(input)))

	{
		dfault(&ps1nod, (geteuid() ? stdprompt : supprompt));
		dfault(&ps2nod, readmsg);
		flags |= ttyflg | prompt;
		if (mailpnod.namflg != N_DEFAULT)
			setmail(mailpnod.namval);
		else
			setmail(mailnod.namval);
		startjobs();
	}
	else
	{
		flags |= prof;
		flags &= ~prompt;
	}
}
