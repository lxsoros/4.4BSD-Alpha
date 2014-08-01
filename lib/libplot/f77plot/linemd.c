/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 */

#ifndef lint
static char sccsid[] = "@(#)linemd.c	5.2 (Berkeley) 4/22/91";
#endif /* not lint */

linemd_(s, len)
register char *s;
long len;
{
	char buf[256];
	register char *cp, *cend;

	cp = buf;
	cend = cp + (len < 255 ? len : 255 );
	while ( cp < cend  && *s != ' ') 
		*cp++ = *s++;
	*cp = 0;
	linemod( buf );
}