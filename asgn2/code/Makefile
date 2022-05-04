#*************************************************
# Justin Satriano    (jsatrian)
# Sriharsha Madala   (srmadala)
#*************************************************

# $Id: Makefile,v 1.48 2022-04-07 15:59:12-07 - - $

MKFILE      = Makefile
DEPSFILE    = ${MKFILE}.deps
NOINCL      = check lint ci clean spotless 
NEEDINCL    = ${filter ${NOINCL}, ${MAKECMDGOALS}}
GMAKE       = ${MAKE} --no-print-directory
GPPOPTS     = -std=gnu++20 -fdiagnostics-color=never
GPPWARN     = -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast
GPP         = g++ ${GPPOPTS} ${GPPWARN}
COMPILECPP  = ${GPP} -g -O0 ${GPPOPTS}
MAKEDEPSCPP = ${GPP} -MM ${GPPOPTS}

MODULES     = commands debug file_sys util
CPPHEADER   = ${MODULES:=.h}
CPPSOURCE   = ${MODULES:=.cpp} main.cpp
EXECBIN     = yshell
OBJECTS     = ${CPPSOURCE:.cpp=.o}
MODULESRC   = ${foreach MOD, ${MODULES}, ${MOD}.h ${MOD}.cpp}
OTHERSRC    = ${filter-out ${MODULESRC}, ${CPPHEADER} ${CPPSOURCE}}
ALLSOURCES  = ${MODULESRC} ${OTHERSRC} ${MKFILE}

export PATH := ${PATH}:/afs/cats.ucsc.edu/courses/cse110a-wm/bin

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${COMPILECPP} -o $@ ${OBJECTS}

%.o : %.cpp
	- checksource $<
	- cpplint.py.perl $<
	${COMPILECPP} -c $<

ci : check
	- cid -is ${ALLSOURCES}

check : ${ALLSOURCES}
	- checksource ${ALLSOURCES}
	- cpplint.py.perl ${CPPSOURCE}

clean :
	- rm ${OBJECTS} ${DEPSFILE} core ${EXECBIN}.errs

spotless : clean
	- rm ${EXECBIN}


deps : ${CPPSOURCE} ${CPPHEADER}
	@ echo "# ${DEPSFILE} created $$(LC_TIME=C date)" >${DEPSFILE}
	${MAKEDEPSCPP} ${CPPSOURCE} >>${DEPSFILE}

${DEPSFILE} :
	@ touch ${DEPSFILE}
	${GMAKE} deps

again :
	${GMAKE} spotless deps ci all

ifeq (${NEEDINCL}, )
include ${DEPSFILE}
endif

