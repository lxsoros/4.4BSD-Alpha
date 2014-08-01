/*-
 * Copyright (c) 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 *
 *	@(#)gmon.h	5.1 (Berkeley) 4/12/91
 */

/* @(#)gmon.h	2.0 (Tahoe) 3/20/85 */

struct phdr {
    char	*lpc;
    char	*hpc;
    int		ncnt;
};

    /*
     *	histogram counters are unsigned shorts (according to the kernel).
     */
#define	HISTCOUNTER	unsigned short

    /*
     *	fraction of text space to allocate for histogram counters
     *	here, 1/2
     */
#define	HISTFRACTION	2

    /*
     *	Fraction of text space to allocate for from hash buckets.
     *	The value of HASHFRACTION is based on the minimum number of bytes
     *	of separation between two subroutine call points in the object code.
     *	Given MIN_SUBR_SEPARATION bytes of separation the value of
     *	HASHFRACTION is calculated as:
     *
     *		HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof(short) - 1);
     *
     *	For the Tahoe, the shortest two call sequence is:
     *
     *		calls	$0,(r0)
     *		calls	$0,(r0)
     *
     *	which is separated by only three bytes, thus HASHFRACTION is 
     *	calculated as:
     *
     *		HASHFRACTION = 3 / (2 * 2 - 1) = 1
     *
     *	Note that the division above rounds down, thus if MIN_SUBR_FRACTION
     *	is less than three, this algorithm will not work!
     */
#define	HASHFRACTION	1

    /*
     *	percent of text space to allocate for tostructs
     *	with a minimum.
     */
#define ARCDENSITY	2
#define MINARCS		50

struct tostruct {
    char		*selfpc;
    long		count;
    unsigned short	link;
};

    /*
     *	a raw arc,
     *	    with pointers to the calling site and the called site
     *	    and a count.
     */
struct rawarc {
    unsigned long	raw_frompc;
    unsigned long	raw_selfpc;
    long		raw_count;
};

    /*
     *	general rounding functions.
     */
#define ROUNDDOWN(x,y)	(((x)/(y))*(y))
#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y))