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
 * Copyright (c) 1996, 2001 by Sun Microsystems, Inc.
 * All rights reserved.
 */

/*
 * Portions Copyright (c) 2005 Gunnar Ritter, Freiburg i. Br., Germany
 *
 * Sccsid @(#)io.c	1.6 (gritter) 8/25/06
 */
/* from OpenSolaris "io.c	1.19	05/06/08 SMI"	 SVr4.0 1.10.2.1 */
/*
 * UNIX shell
 */

#include	"defs.h"
#include	"dup.h"
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>

short topfd;

/* ========	input output and file copying ======== */

void
initf(int fd)
{
	register struct fileblk *f = standin;

	f->fdes = fd;
	f->fsiz = ((flags & oneflg) == 0 ? BUFFERSIZE : 1);
	f->fnxt = f->fend = f->fbuf;
	f->nxtoff = f->endoff = 0;
	f->feval = 0;
	f->flin = 1;
	f->feof = FALSE;
}

int 
estabf(register unsigned char *s)
{
	register struct fileblk *f;

	(f = standin)->fdes = -1;
	f->fend = length(s) + (f->fnxt = s);
	f->nxtoff = 0;
	f->endoff = length(s);
	f->flin = 1;
	return (f->feof = (s == 0));
}

void
push(struct fileblk *af)
{
	register struct fileblk *f;

	(f = af)->fstak = standin;
	f->feof = 0;
	f->feval = 0;
	standin = f;
}

int 
pop(void)
{
	register struct fileblk *f;

	if ((f = standin)->fstak)
	{
		if (f->fdes >= 0)
			close(f->fdes);
		standin = f->fstak;
		return (TRUE);
	}else
		return (FALSE);
}

struct tempblk *tmpfptr;

void
pushtemp(int fd, struct tempblk *tb)
{
	tb->fdes = fd;
	tb->fstak = tmpfptr;
	tmpfptr = tb;
}

int 
poptemp(void)
{
	if (tmpfptr){
		close(tmpfptr->fdes);
		tmpfptr = tmpfptr->fstak;
		return (TRUE);
	}else
		return (FALSE);
}

void
chkpipe(int *pv)
{
	if (pipe(pv) < 0 || pv[INPIPE] < 0 || pv[OTPIPE] < 0)
		error(piperr);
}

int 
chkopen(const unsigned char *idf, int mode)
{
	register int	rc;

	if ((rc = open((const char *)idf, mode, 0666)) < 0)
		failed(idf, badopen);
	return (rc);
}

/*
 * Make f2 be a synonym (including the close-on-exec flag) for f1, which is
 * then closed.  If f2 is descriptor 0, modify the global ioset variable
 * accordingly.
 */
void
renamef(register int f1, register int f2)
{
#ifdef RES
	if (f1 != f2)
	{
		dup(f1 | DUPFLG, f2);
		close(f1);
		if (f2 == 0)
			ioset |= 1;
	}
#else
	int	fs;

	if (f1 != f2)
	{
		fs = fcntl(f2, F_GETFD, 0);
		close(f2);
		fcntl(f1, F_DUPFD, f2);
		close(f1);
		if (fs == 1)
			fcntl(f2, F_SETFD, FD_CLOEXEC);
		if (f2 == 0)
			ioset |= 1;
	}
#endif
}

int 
create(unsigned char *s)
{
	register int	rc;

	if ((rc = creat((char *)s, 0666)) < 0)
		failed(s, badcreate);
	return (rc);
}


int 
tmpfil(struct tempblk *tb)
{
	int fd;

	/* make sure tmp file does not already exist. */
	do {
		itos(serial++);
		movstr(numbuf, tmpname);
		fd = open((char *)tmpout, O_RDWR|O_CREAT|O_EXCL, 0600);
	} while ((fd == -1) && (errno == EEXIST));
	if (fd == -1)
		failed(tmpout, badcreate);
	pushtemp(fd, tb);
	return (fd);
}

/*
 * set by trim
 */
extern BOOL		nosubst;
#define			CPYSIZ		512

void
copy(struct ionod *ioparg)
{
	register unsigned char	*cline;
	register unsigned char	*clinep;
	register struct ionod	*iop;
	unsigned int	c;
	unsigned char	*ends;
	unsigned char	*start;
	int		fd;
	int		i;
	int		stripflg;
	unsigned char	*pc;


	if (iop = ioparg)
	{
		struct tempblk tb;
		copy(iop->iolst);
		ends = mactrim(iop->ioname);
		stripflg = iop->iofile & IOSTRIP;
		if (nosubst)
			iop->iofile &= ~IODOC;
		fd = tmpfil(&tb);

		if (fndef)
			iop->ioname = (char *) make(tmpout);
		else
			iop->ioname = (char *) cpystak(tmpout);

		iop->iolst = iotemp;
		iotemp = iop;

		cline = clinep = start = locstak();
		if (stripflg)
		{
			iop->iofile &= ~IOSTRIP;
			while (*ends == '\t')
				ends++;
		}
		for (;;)
		{
			chkpr();
			if (nosubst)
			{
				c = readwc();
				if (stripflg)
					while (c == '\t')
						c = readwc();

				while (!eolchar(c))
				{
					pc = readw(c);
					while (*pc) {
						if (clinep >= brkend)
							growstak(clinep);
						*clinep++ = *pc++;
					}
					c = readwc();
				}
			}else{
				c = nextwc();
				if (stripflg)
					while (c == '\t')
						c = nextwc();

				while (!eolchar(c))
				{
					pc = readw(c);
					while (*pc) {
						if (clinep >= brkend)
							growstak(clinep);
						*clinep++ = *pc++;
					}
					if (c == '\\')
					{
						pc = readw(readwc());
						/* *pc might be NULL */
						if (*pc) {
							while (*pc) {
								if (clinep >= brkend)
									growstak(clinep);
								*clinep++ = *pc++;
							}
						} else {
							if (clinep >= brkend)
								growstak(clinep);
							*clinep++ = *pc;
						}
					}
					c = nextwc();
				}
			}

			if (clinep >= brkend)
				growstak(clinep);
			*clinep = 0;
			if (eof || eq(cline, ends))
			{
				if ((i = cline - start) > 0)
					write(fd, start, i);
				break;
			}else{
				if (clinep >= brkend)
					growstak(clinep);
				*clinep++ = NL;
			}

			if ((i = clinep - start) < CPYSIZ)
				cline = clinep;
			else
			{
				write(fd, start, i);
				cline = clinep = start;
			}
		}

		poptemp();	/*
				 * pushed in tmpfil -- bug fix for problem
				 * deleting in-line scripts
				 */
	}
}


void
link_iodocs(struct ionod *i)
{
	int r;

	while (i)
	{
		free(i->iolink);

		/* make sure tmp file does not already exist. */
		do {
			itos(serial++);
			movstr(numbuf, tmpname);
			r = link(i->ioname, (char *)tmpout);
		} while (r == -1 && errno == EEXIST);

		if (r != -1) {
			i->iolink = (char *)make(tmpout);
			i = i->iolst;
		} else
			failed(tmpout, badcreate);

	}
}


void
swap_iodoc_nm(struct ionod *i)
{
	while (i)
	{
		free(i->ioname);
		i->ioname = i->iolink;
		i->iolink = 0;

		i = i->iolst;
	}
}


int 
savefd(int fd)
{
	register int	f;

	f = fcntl(fd, F_DUPFD, 10);
	return (f);
}


void
restore(register int last)
{
	register int 	i;
	register int	dupfd;

	for (i = topfd - 1; i >= last; i--)
	{
		if ((dupfd = fdmap[i].dup_fd) > 0)
			renamef(dupfd, fdmap[i].org_fd);
		else
			close(fdmap[i].org_fd);
	}
	topfd = last;
}
