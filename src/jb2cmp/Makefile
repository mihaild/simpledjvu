#C- This program is free software; you can redistribute it and/or 
#C- modify it under the terms of the GNU General Public License, 
#C- either Version 2 of the License or (at your option) any later
#C- version.  The license should have accompanied the program 
#C- or you may obtain a copy of the license from the Free Software
#C- Foundation at http://www.fsf.org.
#C- 
#C- This program is distributed in the hope that it will be useful,
#C- but WITHOUT ANY WARRANTY; without even the implied warranty of
#C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#C- GNU General Public License for more details.


SHELL = /bin/bash

PACKAGE_NAME = djvulibre
PACKAGE_VERSION = 3.5.25

srcdir = .
top_srcdir = ../..
top_builddir = ../..

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
datarootdir = ${prefix}/share
datadir = ${datarootdir}
libdir = ${exec_prefix}/lib
mandir = ${datarootdir}/man

CC = gcc
CXX = g++
RM = /bin/rm -f
AR = /usr/bin/ar
RANLIB = ranlib
LN_S = ln -s
TOUCH = /usr/bin/touch
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
LIBTOOL = $(SHELL) $(top_builddir)/libtool

DEFS = -DHAVE_CONFIG_H
OPTS = -DNDEBUG -Wall -O3 -Wno-non-virtual-dtor
CXXRPOFLAGS = 
THREAD_LIBS =  -lpthread
THREAD_CFLAGS =  -pthread -DTHREADMODEL=POSIXTHREADS

INCS = -I${top_builddir} -I${top_srcdir} -I${srcdir}
FLAGS = ${DEFS} ${INCS} ${OPTS} ${THREAD_CFLAGS}
LIBS=  ${THREAD_LIBS} -lm 
CFLAGS =   ${FLAGS}
CXXFLAGS =   ${CXXRPOFLAGS} ${FLAGS}


OBJS = classify.o cuts.o frames.o patterns.o
JB2CMP_A = libjb2cmp.a

all: ${JB2CMP_A}

#${JB2CMP_A} : libjb2cmp-rpo
#libjb2cmp-rpo : ${OBJS}
#	@echo "[ performing library closure ... ]"
#	@echo "int main() { return 0; }" > $@.cpp
#	${CXX} ${CXXFLAGS} -c $@.cpp
#	${CXX} ${CXXFLAGS} -o $@ $@.o ${OBJS} ${LIBS}
install:

depend: FORCE
	${CXX} -MM ${CXXFLAGS} ${srcdir}/*.cpp > Makefile.dep

clean: FORCE
	-${RM} 2>/dev/null ${JB2CMP}
	-${RM} 2>/dev/null *.o *.a *.so *.stamp

distclean: clean 
	-${RM} -r 2>/dev/null Makefile *.rpo ii_files

# Link

${JB2CMP_A}: ${OBJS}
	-${RM} 2>/dev/null ${JB2CMP_A}
	${AR} cr ${JB2CMP_A} ${OBJS}
	${RANLIB} ${JB2CMP_A}

# Rules

FORCE:
.PHONY: FORCE
.SUFFIXES: .c .cpp .o

.c.o:
	${CC} ${CFLAGS} -c $<

.cpp.o:
	${CXX} ${CXXFLAGS} -c $<

.c.lo:
	${LIBTOOL} --mode=compile ${CC} ${CFLAGS} -c $<

.cpp.lo:
	${LIBTOOL} --mode=compile ${CXX} ${CXXFLAGS} -c $<

# Dependencies

Makefile.dep: ${srcdir}/Makefile.dep
	cp ${srcdir}/Makefile.dep $@

include Makefile.dep
