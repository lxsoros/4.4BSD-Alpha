/*-
 * Copyright (c) 1982, 1986 Regents of the University of California.
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
 *	@(#)subr_prof.c	7.17 (Berkeley) 7/10/92
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <machine/cpu.h>

#ifdef GPROF
#include <sys/malloc.h>
#include <sys/gmon.h>

/*
 * Froms is actually a bunch of unsigned shorts indexing tos
 */
struct gmonparam _gmonparam = { GMON_PROF_OFF };

u_short	*kcount;
extern char etext[];

kmstartup()
{
	char *cp;
	int fsize, tsize, ksize;
	struct gmonparam *p = &_gmonparam;
	/*
	 * Round lowpc and highpc to multiples of the density we're using
	 * so the rest of the scaling (here and in gprof) stays in ints.
	 */
	p->lowpc = ROUNDDOWN(KERNBASE, HISTFRACTION * sizeof(HISTCOUNTER));
	p->highpc = ROUNDUP((u_long)etext, HISTFRACTION * sizeof(HISTCOUNTER));
	p->textsize = p->highpc - p->lowpc;
	printf("Profiling kernel, textsize=%d [%x..%x]\n",
	       p->textsize, p->lowpc, p->highpc);
	ksize = p->textsize / HISTFRACTION;
	fsize = p->textsize / HASHFRACTION;
	p->tolimit = p->textsize * ARCDENSITY / 100;
	if (p->tolimit < MINARCS)
		p->tolimit = MINARCS;
	else if (p->tolimit > MAXARCS)
		p->tolimit = MAXARCS;
	tsize = p->tolimit * sizeof(struct tostruct);
	cp = (char *)malloc(ksize + fsize + tsize, M_GPROF, M_NOWAIT);
	if (cp == 0) {
		printf("No memory for profiling.\n");
		return;
	}
	bzero(cp, ksize + tsize + fsize);
	p->tos = (struct tostruct *)cp;
	cp += tsize;
	kcount = (u_short *)cp;
	cp += ksize;
	p->froms = (u_short *)cp;
	startprofclock(&proc0);
}
#endif

/*
 * Profiling system call.
 *
 * The scale factor is a fixed point number with 16 bits of fraction, so that
 * 1.0 is represented as 0x10000.  A scale factor of 0 turns off profiling.
 */
struct profil_args {
	caddr_t	buf;
	u_int	bufsize;
	u_int	offset;
	u_int	scale;
};
/* ARGSUSED */
profil(p, uap, retval)
	struct proc *p;
	register struct profil_args *uap;
	int *retval;
{
	register struct uprof *upp;
	int s;

	if (uap->scale > (1 << 16))
		return (EINVAL);
	if (uap->scale == 0) {
		stopprofclock(p);
		return (0);
	}
	upp = &p->p_stats->p_prof;
	s = splstatclock(); /* block profile interrupts while changing state */
	upp->pr_base = uap->buf;
	upp->pr_size = uap->bufsize;
	upp->pr_off = uap->offset;
	upp->pr_scale = uap->scale;
	startprofclock(p);
	splx(s);
	return (0);
}

/*
 * Scale is a fixed-point number with the binary point 16 bits
 * into the value, and is <= 1.0.  pc is at most 32 bits, so the
 * intermediate result is at most 48 bits.
 */
#define	PC_TO_INDEX(pc, prof) \
	((int)(((u_quad_t)((pc) - (prof)->pr_off) * \
	    (u_quad_t)((prof)->pr_scale)) >> 16) & ~1)

/*
 * Collect user-level profiling statistics; called on a profiling tick,
 * when a process is running in user-mode.  This routine may be called
 * from an interrupt context.  We try to update the user profiling buffers
 * cheaply with fuswintr() and suswintr().  If that fails, we revert to
 * an AST that will vector us to trap() with a context in which copyin
 * and copyout will work.  Trap will then call addupc_task().
 *
 * Note that we may (rarely) not get around to the AST soon enough, and
 * lose profile ticks when the next tick overwrites this one, but in this
 * case the system is overloaded and the profile is probably already
 * inaccurate.
 */
void
addupc_intr(p, pc, ticks)
	register struct proc *p;
	register u_long pc;
	u_int ticks;
{
	register struct uprof *prof;
	register caddr_t addr;
	register u_int i;
	register int v;

	if (ticks == 0)
		return;
	prof = &p->p_stats->p_prof;
	if (pc < prof->pr_off ||
	    (i = PC_TO_INDEX(pc, prof)) >= prof->pr_size)
		return;			/* out of range; ignore */

	addr = prof->pr_base + i;
	if ((v = fuswintr(addr)) == -1 || suswintr(addr, v + ticks) == -1) {
		prof->pr_addr = pc;
		prof->pr_ticks = ticks;
		need_proftick(p);
	}
}

/*
 * Much like before, but we can afford to take faults here.  If the
 * update fails, we simply turn off profiling.
 */
void
addupc_task(p, pc, ticks)
	register struct proc *p;
	register u_long pc;
	u_int ticks;
{
	register struct uprof *prof;
	register caddr_t addr;
	register u_int i;
	u_short v;

	/* testing SPROFIL may be unnecessary, but is certainly safe */
	if ((p->p_flag & SPROFIL) == 0 || ticks == 0)
		return;

	prof = &p->p_stats->p_prof;
	if (pc < prof->pr_off ||
	    (i = PC_TO_INDEX(pc, prof)) >= prof->pr_size)
		return;

	addr = prof->pr_base + i;
	if (copyin(addr, (caddr_t)&v, sizeof(v)) == 0) {
		v += ticks;
		if (copyout((caddr_t)&v, addr, sizeof(v)) == 0)
			return;
	}
	stopprofclock(p);
}
