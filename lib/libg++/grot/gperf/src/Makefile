# Copyright (C) 1989 Free Software Foundation, Inc.
# written by Douglas C. Schmidt (schmidt@ics.uci.edu)
# 
# This file is part of GNU GPERF.
# 
# GNU GPERF is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
# 
# GNU GPERF is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU GPERF; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 

GXX   = g++
DFLAGS= -Wall -g -DLO_CAL -DUNLIMIT_STACK # -DTRACE
OFLAGS= -O -fstrength-reduce -fdelayed-branch -fsave-memoized \
        -finline-functions -felide-constructors
GXXFLAGS= $(DFLAGS) $(OFLAGS)
prefix=/usr/local
BINDIR=$(prefix)/bin
LIBDIR=$(prefix)/lib
MANDIR=$(prefix)/man
INSTALL=install
OBJS  = new.o options.o iterator.o main.o gen-perf.o key-list.o list-node.o \
        hash-table.o bool-array.o read-line.o std-err.o version.o 
.SUFFIXES: .cc
.cc.o:
	$(GXX) $(GXXFLAGS) -c  $<

all: gperf

gperf: $(OBJS) 
	$(GXX) $(GXXFLAGS) -o gperf $(OBJS) -L../../src $(LIBS)

install: all
	$(INSTALL) gperf $(BINDIR)/gperf

clean: 
	rm -f *.o core *~ #*# mon.out

realclean: clean
	rm -f gperf

depend:
	g++dep *.cc
# dependencies
# DO NOT DELETE THIS LINE -- g++dep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

bool-array.o : bool-array.cc options.h std-err.h trace.h bool-array.h 
gen-perf.o : gen-perf.cc options.h std-err.h trace.h gen-perf.h key-list.h \
  list-node.h vectors.h read-line.h bool-array.h 
hash-table.o : hash-table.cc hash-table.h list-node.h std-err.h vectors.h \
  options.h trace.h 
iterator.o : iterator.cc iterator.h trace.h 
key-list.o : key-list.cc options.h std-err.h trace.h read-line.h \
  hash-table.h list-node.h vectors.h key-list.h 
list-node.o : list-node.cc options.h std-err.h trace.h list-node.h \
  vectors.h 
main.o : main.cc std-err.h options.h trace.h gen-perf.h key-list.h \
  list-node.h vectors.h read-line.h bool-array.h 
new.o : new.cc std-err.h trace.h 
options.o : options.cc options.h std-err.h trace.h iterator.h 
read-line.o : read-line.cc std-err.h read-line.h trace.h 
std-err.o : std-err.cc std-err.h trace.h 
version.o : version.cc 

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
