#	@(#)Makefile	5.1 (Berkeley) 7/13/92

#CFLAGS+=	-g -DDEBUG
PROG=	mount_portal
PTAL=	pt_conf.c pt_exec.c pt_file.c pt_tcp.c
SRCS=	mount_portal.c conf.c activate.c ${PTAL}
NOMAN=	noman
#MAN8=	mount_portal.0
LDADD+=	-lutil

.include <bsd.prog.mk>
