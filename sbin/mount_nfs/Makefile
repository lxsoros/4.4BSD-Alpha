#	@(#)Makefile	5.1 (Berkeley) 1/6/92

PROG=	mount_nfs
CFLAGS+=-DNFS
MAN8=	mount_nfs.0
DPADD=	${LIBRPC}
LDADD=	-lrpc

.include <bsd.prog.mk>
