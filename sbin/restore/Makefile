#	@(#)Makefile	5.13 (Berkeley) 3/16/92

PROG=	restore
LINKS=	${BINDIR}/restore ${BINDIR}/rrestore
CFLAGS+=-DRRESTORE
SRCS=	main.c interactive.c restore.c dirs.c symtab.c tape.c utilities.c \
	dumprmt.c
BINOWN=	root
BINGRP=	tty
BINMODE=6555
MAN8=	restore.0
MLINKS+=restore.8 rrestore.8
.PATH:	${.CURDIR}/../dump

all: ${PROG} ${MAN8}

.include <bsd.prog.mk>
