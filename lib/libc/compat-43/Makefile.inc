#	@(#)Makefile.inc	5.6 (Berkeley) 7/9/92

# compat-43 sources
.PATH: ${.CURDIR}/${MACHINE}/compat-43 ${.CURDIR}/compat-43

SRCS+=	creat.c getwd.c killpg.c setpgrp.c setreuid.c sigcompat.c

MAN2+=	creat.0 killpg.0 setreuid.0 sigblock.0 sigpause.0 sigsetmask.0 sigvec.0
