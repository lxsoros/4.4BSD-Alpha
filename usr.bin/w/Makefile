#	@(#)Makefile	5.8 (Berkeley) 4/13/92

PROG=	w
SRCS=	attime.c fmt.c w.c proc_compare.c
MAN1=	w.0 uptime.0
DPADD=	${LIBUTIL}
LDADD=	-lkvm -lutil
BINGRP=	kmem
BINMODE=2555
LINKS=	${BINDIR}/w ${BINDIR}/uptime

.PATH: ${.CURDIR}/../../bin/ps

.include <bsd.prog.mk>
