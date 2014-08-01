/*-
 * Copyright (c) 1982, 1986, 1991 The Regents of the University of California.
 * All rights reserved.
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
 *	@(#)tty_conf.c	7.8 (Berkeley) 2/15/92
 */

#include "param.h"
#include "systm.h"
#include "buf.h"
#include "ioctl.h"
#include "proc.h"
#include "tty.h"
#include "conf.h"

#define	ttynodisc ((int (*) __P((dev_t, struct tty *)))enodev)
#define	ttyerrclose ((int (*) __P((struct tty *, int flags)))enodev)
#define	ttyerrio ((int (*) __P((struct tty *, struct uio *, int)))enodev)
#define	ttyerrinput ((int (*) __P((int c, struct tty *)))enodev)
#define	ttyerrstart ((int (*) __P((struct tty *)))enodev)

int	ttyopen __P((dev_t dev, struct tty *tp));
int	ttylclose __P((struct tty *tp, int flags));
int	ttread __P((struct tty *, struct uio *, int flags));
int	ttwrite __P((struct tty *, struct uio *, int flags));
int	nullioctl __P((struct tty *tp, int cmd, caddr_t data,
			int flag, struct proc *p));
int	ttyinput __P((int c, struct tty *tp));
int	ttstart __P((struct tty *tp));
int	ttymodem __P((struct tty *tp, int flags));
int	nullmodem __P((struct tty *tp, int flags));

#include "tb.h"
#if NTB > 0
int	tbopen __P((dev_t dev, struct tty *tp));
int	tbclose __P((struct tty *tp, int flags));
int	tbread __P((struct tty *, struct uio *, int flags));
int	tbioctl __P((struct tty *tp, int cmd, caddr_t data,
			int flag, struct proc *p));
int	tbinput __P((int c, struct tty *tp));
#endif

#include "sl.h"
#if NSL > 0
int	slopen __P((dev_t dev, struct tty *tp));
int	slclose __P((struct tty *tp, int flags));
int	sltioctl __P((struct tty *tp, int cmd, caddr_t data,
			int flag, struct proc *p));
int	slinput __P((int c, struct tty *tp));
int	slstart __P((struct tty *tp));
#endif


struct	linesw linesw[] =
{
	ttyopen, ttylclose, ttread, ttwrite, nullioctl,
	ttyinput, ttstart, ttymodem,			/* 0- termios */

	ttynodisc, ttyerrclose, ttyerrio, ttyerrio, nullioctl,
	ttyerrinput, ttyerrstart, nullmodem,		/* 1- defunct */

	ttynodisc, ttyerrclose, ttyerrio, ttyerrio, nullioctl,
	ttyerrinput, ttyerrstart, nullmodem,		/* 2- defunct */

#if NTB > 0
	tbopen, tbclose, tbread, enodev, tbioctl,
	tbinput, ttstart, nullmodem,			/* 3- TABLDISC */
#else
	ttynodisc, ttyerrclose, ttyerrio, ttyerrio, nullioctl,
	ttyerrinput, ttyerrstart, nullmodem,
#endif

#if NSL > 0
	slopen, slclose, ttyerrio, ttyerrio, sltioctl,
	slinput, slstart, nullmodem,			/* 4- SLIPDISC */
#else
	ttynodisc, ttyerrclose, ttyerrio, ttyerrio, nullioctl,
	ttyerrinput, ttyerrstart, nullmodem,
#endif
};

int	nldisp = sizeof (linesw) / sizeof (linesw[0]);

/*
 * Do nothing specific version of line
 * discipline specific ioctl command.
 */
/*ARGSUSED*/
nullioctl(tp, cmd, data, flags, p)
	struct tty *tp;
	int cmd;
	char *data;
	int flags;
	struct proc *p;
{

#ifdef lint
	tp = tp; data = data; flags = flags; p = p;
#endif
	return (-1);
}