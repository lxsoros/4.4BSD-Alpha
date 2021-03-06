#Copyright (C) 1990 Free Software Foundation, Inc.
#     Written by James Clark (jjc@jclark.uucp)
#
#This file is part of groff.
#
#groff is free software; you can redistribute it and/or modify it under
#the terms of the GNU General Public License as published by the Free
#Software Foundation; either version 1, or (at your option) any later
#version.
#
#groff is distributed in the hope that it will be useful, but WITHOUT ANY
#WARRANTY; without even the implied warranty of MERCHANTABILITY or
#FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
#for more details.
#
#You should have received a copy of the GNU General Public License along
#with groff; see the file LICENSE.  If not, write to the Free Software
#Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

MANROOT=/usr/share/man
# MAN1EXT is the man section for user commands
MAN1EXT=1
MAN1DIR=$(MANROOT)/cat$(MAN1EXT)
# MAN5EXT is the man section for file formats
MAN5EXT=5
MAN5DIR=$(MANROOT)/cat$(MAN5EXT)
# MAN7EXT is the man section for macros
MAN7EXT=7
MAN7DIR=$(MANROOT)/cat$(MAN7EXT)
# FONTDIR says where to install dev*/*
FONTDIR=/usr/share/groff_font
# FONTPATH says where to look for dev*/*
FONTPATH=.:$(FONTDIR):
MACRODIR=/usr/share/tmac
# MACROPATH says where to look for tmac.* macro files
MACROPATH=.:$(MACRODIR):
# DEVICE is the default device
DEVICE=ps
# HYPHENFILE is the file containing the hyphenation patterns
HYPHENFILE=/usr/share/misc/hyphen
TMAC_S=gs
SHELL=/bin/sh
MAN1PAGES= gsoelim.n \
	addftinfo.n grodvi.n tfmtodit.n afmtodit.n grog.n
MAN5PAGES=groff_font.n groff_out.n
MAN7PAGES=groff_me.n groff_ms.n
MANPAGES= $(MAN1PAGES) $(MAN5PAGES) $(MAN7PAGES)

.SUFFIXES: .man .n

.man.n:
	@echo Making $@ from $<
	@-rm -f $@
	@sed -e "s;@HYPHENFILE@;$(HYPHENFILE);" \
	-e "s;@FONTDIR@;$(FONTDIR);" \
	-e "s;@FONTPATH@;$(FONTPATH);" \
	-e "s;@MACRODIR@;$(MACRODIR);" \
	-e "s;@MACROPATH@;$(MACROPATH);" \
	-e "s;@DEVICE@;$(DEVICE);" \
	-e "s;@MAN1EXT@;$(MAN1EXT);" \
	-e "s;@MAN5EXT@;$(MAN5EXT);" \
	-e "s;@MAN7EXT@;$(MAN7EXT);" \
	-e "s;@TMAC_S@;$(TMAC_S);" \
	-e "s;@VERSION@;`cat ../VERSION`;" \
	$< >$@
	@chmod 444 $@

all: $(MANPAGES)

install.bin: $(MANPAGES)
	-[ -d $(MAN1DIR) ] || mkdir $(MAN1DIR)
	-[ -d $(MAN5DIR) ] || mkdir $(MAN5DIR)
	-[ -d $(MAN7DIR) ] || mkdir $(MAN7DIR)
	@for page in $(MAN1PAGES) ; do \
	target=$(MAN1DIR)/`basename $$page .n`.0; \
	rm -f $$target ; \
	echo nroff -man $$page ">" $$target ; \
	nroff -man $$page > $$target ; \
	done
	@for page in $(MAN5PAGES) ; do \
	target=$(MAN5DIR)/`basename $$page .n`.0; \
	rm -f $$target ; \
	echo nroff -man $$page ">" $$target ; \
	nroff -man $$page > $$target ; \
	done
	@for page in $(MAN7PAGES) ; do \
	target=$(MAN7DIR)/`basename $$page .n`.0; \
	rm -f $$target ; \
	echo nroff -man $$page ">" $$target ; \
	nroff -man $$page > $$target ; \
	done

$(MANPAGES): ../VERSION

install: install.bin 

clean:
	-rm -f $(MANPAGES)

distclean: clean

realclean: clean

TAGS:
