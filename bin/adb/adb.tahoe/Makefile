#	@(#)Makefile	5.6 (Berkeley) 1/15/91

PROG=	adb
SRCS=	access.c command.c expr.c format.c kstack.c machdep.c main.c \
	opset.c optab.c output.c pcs.c print.c rodata.c setup.c \
	strings.c sym.c
.PATH: ${.CURDIR}/../common_source

CFLAGS+=-I${.CURDIR} -I${.CURDIR}/../common_source -I/sys

optab.o rodata.o strings.o:
	${CC} ${CFLAGS} -R -c ${.IMPSRC}

.include "../../Makefile.inc"
.include <bsd.prog.mk>
