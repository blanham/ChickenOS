/*	$NetBSD: nodes.c.pat,v 1.8 1997/04/11 23:03:09 christos Exp $	*/

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)nodes.c.pat	8.2 (Berkeley) 5/4/95
 */

#include <stdlib.h>
/*
 * Routine for dealing with parsed shell commands.
 */

#include "shell.h"
#include "nodes.h"
#include "memalloc.h"
#include "machdep.h"
#include "mystring.h"


int     funcblocksize;		/* size of structures in function */
int     funcstringsize;		/* size of strings in node */
pointer funcblock;		/* block to allocate function from */
char   *funcstring;		/* block to allocate strings from */

%SIZES


STATIC void calcsize __P((union node *));
STATIC void sizenodelist __P((struct nodelist *));
STATIC union node *copynode __P((union node *));
STATIC struct nodelist *copynodelist __P((struct nodelist *));
STATIC char *nodesavestr __P((char *));



/*
 * Make a copy of a parse tree.
 */

union node *
copyfunc(n)
	union node *n;
{
	if (n == NULL)
		return NULL;
	funcblocksize = 0;
	funcstringsize = 0;
	calcsize(n);
	funcblock = ckmalloc(funcblocksize + funcstringsize);
	funcstring = (char *) funcblock + funcblocksize;
	return copynode(n);
}



STATIC void
calcsize(n)
	union node *n;
{
	%CALCSIZE
}



STATIC void
sizenodelist(lp)
	struct nodelist *lp;
{
	while (lp) {
		funcblocksize += ALIGN(sizeof(struct nodelist));
		calcsize(lp->n);
		lp = lp->next;
	}
}



STATIC union node *
copynode(n)
	union node *n;
{
	union node *new;

	%COPY
	return new;
}


STATIC struct nodelist *
copynodelist(lp)
	struct nodelist *lp;
{
	struct nodelist *start;
	struct nodelist **lpp;

	lpp = &start;
	while (lp) {
		*lpp = funcblock;
		funcblock = (char *) funcblock + ALIGN(sizeof(struct nodelist));
		(*lpp)->n = copynode(lp->n);
		lp = lp->next;
		lpp = &(*lpp)->next;
	}
	*lpp = NULL;
	return start;
}



STATIC char *
nodesavestr(s)
	char   *s;
{
	register char *p = s;
	register char *q = funcstring;
	char   *rtn = funcstring;

	while ((*q++ = *p++) != '\0')
		continue;
	funcstring = q;
	return rtn;
}



/*
 * Free a parse tree.
 */

void
freefunc(n)
	union node *n;
{
	if (n)
		ckfree(n);
}
