/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 */

#ifndef lint
static char sccsid[] = "@(#)ovprintf.c	1.4 (Berkeley) 4/17/91";
#endif /* not lint */

/*
 * This version of printf calls doprnt, and as such is not portable,
 * since doprnt is written in pdp-11 assembly language.  (There is a
 * vax doprnt which has the first 2 arguments reversed.  We don't use it.)
 * This version is used because it is about 900 bytes smaller than the
 * portable version, which is also included in case it is needed.
 */
#ifdef TRACE
#include	<stdio.h>
#undef putchar
#endif

printf(fmt, args)
char *fmt;
{
	_doprnt(fmt, &args, 0);
}

_strout(string, count, adjust, file, fillch)
register char *string;
register count;
int adjust;
register struct _iobuf *file;
{
	while (adjust < 0) {
		if (*string=='-' && fillch=='0') {
			putchar(*string++);
			count--;
		}
		putchar(fillch);
		adjust++;
	}
	while (--count>=0)
		putchar(*string++);
	while (adjust) {
		putchar(fillch);
		adjust--;
	}
}
