#	@(#)Makefile	5.18 (Berkeley) 6/29/91

MAN3=	assert.0 bitstring.0 end.0 intro.0 moncontrol.0 stdarg.0
MLINKS+=end.3 edata.3 end.3 etext.3 moncontrol.3 monstartup.3
MLINKS+=stdarg.3 varargs.3 stdarg.3 va_arg.3 stdarg.3 va_end.3
MLINKS+=stdarg.3 va_start.3

all: ${MAN3}

clean depend lint tags:

cleandir:
	rm -f ${MAN3}

install: maninstall

.include <bsd.prog.mk>
