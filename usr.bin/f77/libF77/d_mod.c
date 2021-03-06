/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 */

#ifndef lint
static char sccsid[] = "@(#)d_mod.c	5.5 (Berkeley) 4/12/91";
#endif /* not lint */

#ifdef tahoe
#include <tahoe/math/FP.h>
#endif

double d_mod(x,y)
double *x, *y;
{
double floor(), quotient = *x / *y;
if (quotient >= 0.0)
	quotient = floor(quotient);
else {
#ifndef tahoe
	quotient = -floor(-quotient);
#else tahoe
	*(unsigned long *)&quotient ^= SIGN_BIT;
	quotient = floor(quotient);
	if (quotient !=0)
		*(unsigned long *)&quotient ^= SIGN_BIT;
#endif tahoe
}
return(*x - (*y) * quotient );
}
