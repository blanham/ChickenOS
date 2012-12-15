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
 * Portions Copyright (c) 2005 Gunnar Ritter, Freiburg i. Br., Germany
 *
 * Sccsid @(#)pwd.c	1.6 (gritter) 6/15/05
 */

/* from OpenSolaris "pwd.c	1.10	05/06/08 SMI"	 SVr4.0 1.14.6.1 */
/* 
 *	UNIX shell
 */

#include	"mac.h"
#include	"defs.h"
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<limits.h>

#define	DOT		'.'
#define	SLASH	'/'
#define PARTLY	2

static void rmslash(unsigned char *);
extern const char	longpwd[];

unsigned char cwdname[PATH_MAX+1];

static int 	didpwd = FALSE;

void
cwd(register unsigned char *dir)
{
	register unsigned char *pcwd;
	register unsigned char *pdir;

	/* First remove extra /'s */

	rmslash(dir);

	/* Now remove any .'s */

	pdir = dir;
	if(*dir == SLASH)
		pdir++;
	while(*pdir) 			/* remove /./ by itself */
	{
		if((*pdir==DOT) && (*(pdir+1)==SLASH))
		{
			movstr(pdir+2, pdir);
			continue;
		}
		pdir++;
		while ((*pdir) && (*pdir != SLASH)) 
			pdir++;
		if (*pdir) 
			pdir++;
	}
	/* take care of trailing /. */
	if(*(--pdir)==DOT && pdir > dir && *(--pdir)==SLASH) {
		if(pdir > dir) {
			*pdir = '\0';
		} else {
			*(pdir+1) = '\0';
		}

	}

	/* Remove extra /'s */

	rmslash(dir);

	/* Now that the dir is canonicalized, process it */

	if(*dir==DOT && *(dir+1)=='\0')
	{
		return;
	}


	if(*dir==SLASH)
	{
		/* Absolute path */

		pcwd = cwdname;
		*pcwd++ = *dir++;
		didpwd = PARTLY;
	}
	else
	{
		/* Relative path */

		if (didpwd == FALSE) 
			return;
		didpwd = PARTLY;	
		pcwd = cwdname + length(cwdname) - 1;
		if(pcwd != cwdname+1)
			*pcwd++ = SLASH;
	}
	while(*dir)
	{
		if(*dir==DOT && 
		   *(dir+1)==DOT &&
		   (*(dir+2)==SLASH || *(dir+2)=='\0'))
		{
			/* Parent directory, so backup one */

			if( pcwd > cwdname+2 )
				--pcwd;
			while(*(--pcwd) != SLASH)
				;
			pcwd++;
			dir += 2;
			if(*dir==SLASH)
			{
				dir++;
			}
			continue;
		}
	 	if (pcwd >= &cwdname[PATH_MAX+1])
		{
			didpwd=FALSE;
			return;
		}
		*pcwd++ = *dir++;
		while((*dir) && (*dir != SLASH))
		{  
	 		if (pcwd >= &cwdname[PATH_MAX+1])
			{
				didpwd=FALSE;
				return;
			}
			*pcwd++ = *dir++;
		} 
		if (*dir) 
		{
	 		if (pcwd >= &cwdname[PATH_MAX+1])
			{
				didpwd=FALSE;
				return;
			}
			*pcwd++ = *dir++;
		}
	}
	if (pcwd >= &cwdname[PATH_MAX+1])
	{
		didpwd=FALSE;
		return;
	}
	*pcwd = '\0';

	--pcwd;
	if(pcwd>cwdname && *pcwd==SLASH)
	{
		/* Remove trailing / */

		*pcwd = '\0';
	}
	return;
}

void 
cwd2(void)
{
	struct stat stat1, stat2;
	unsigned char *pcwd;
	/* check if there are any symbolic links in pathname */

	if(didpwd == FALSE)
		return;
	pcwd = cwdname + 1;
	if(didpwd == PARTLY) {
		while (*pcwd)
		{
			char c;
			while((c = *pcwd++) != SLASH && c != '\0');
			*--pcwd = '\0';
			if (lstat((char *)cwdname, &stat1) == -1
		    	|| (stat1.st_mode & S_IFMT) == S_IFLNK) {
				didpwd = FALSE;
				*pcwd = c;
				return;
			}
			*pcwd = c;
			if(c)
				pcwd++;
		}
		didpwd = TRUE;
	} else 
		if (stat((char *)cwdname, &stat1) == -1) {
			didpwd = FALSE;
			return;
		}
	/*
	 * check if ino's and dev's match; pathname could
	 * consist of symbolic links with ".."
	 */

	if (stat(".", &stat2) == -1
	    || stat1.st_dev != stat2.st_dev
	    || stat1.st_ino != stat2.st_ino)
		didpwd = FALSE;
	return;
}

unsigned char *
cwdget(void)
{
	cwd2();
	if (didpwd == FALSE) {
		if(getcwd(cwdname, PATH_MAX+1) == (char *)0)
			*cwdname = 0;
		didpwd = TRUE;
	} 
	return (cwdname);
}

/*
 *	Print the current working directory.
 */

void
cwdprint(void)
{
	register unsigned char *cp;

	cwd2();
	if (didpwd == FALSE) {
		if(getcwd(cwdname, PATH_MAX+1) == (char *)0) {
			if(errno && errno != ERANGE)
				error("cannot determine current directory");
			else
				error(longpwd);
		}
		didpwd = TRUE;
	}

	for (cp = cwdname; *cp; cp++) {
	  prc_buff(*cp);
	}

	prc_buff(NL);
	return;
}

/*
 *	This routine will remove repeated slashes from string.
 */

static void 
rmslash(unsigned char *string)
{
	register unsigned char *pstring;

	pstring = string;
	while(*pstring)
	{
		if(*pstring==SLASH && *(pstring+1)==SLASH)
		{
			/* Remove repeated SLASH's */

			movstr(pstring+1, pstring);
			continue;
		}
		pstring++;
	}

	--pstring;
	if(pstring>string && *pstring==SLASH)
	{
		/* Remove trailing / */

		*pstring = '\0';
	}
	return;
}
