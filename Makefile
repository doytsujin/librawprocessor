# Makefile for librawprocessor
# (C)2016 Raphael Kim / rageworx@gmail.com
#

#########################################################################
# About cross compiler, or other platform :
#
# To enable build for embedded linux, you may encomment next 2 lines.
# Or, may need to change your cross compiler path with environments.
# It is up to developer !

# CCPREPATH = ${ARM_LINUX_GCC_PATH}
# CCPREFIX  = arm-linux-

# To enable build for embedded linux, change following line.
# CCPATH    = ${CCPREPATH}/${CCPREFIX}
CCPATH =
#########################################################################

# Compiler configure.
GCC = ${CCPATH}gcc
GPP = ${CCPATH}g++
AR  = ${CCPATH}ar

# Sources and how it built
# Optimization issue: recommend to build with using -ffast-math option.
# Change if your CPU architecture supports more high technologies.
SOURCEDIR = ./src
OBJDIR    = ./obj/Release
OUTBIN    = librawprocessor.a
OUTDIR    = ./lib
DEFINEOPT =
OPTIMIZEOPT = -ffast-math -O3 -s
CPUARCHOPT    = -mavx

ifeq (64bit,$(firstword $(MAKECMDGOALS)))
	OBJDIR = ./obj64/Release
	OUTDIR = ./lib64
	CPUARCHOPT += -m64
endif

ifeq (openmp,$(firstword $(MAKECMDGOALS)))
	OPTIMIZEOPT += -fopenmp
endif

ifeq (debug,$(firstword $(MAKECMDGOALS)))
	DEFINEOPT += -DDEBUG
	OUTBIN = librawprocessor_d.a
endif

CFLAGS    = -I$(SOURCEDIR) $(DEFINEOPT) $(OPTIMIZEOPT) $(CPUARCHOPT) $(BITSOPT)

.PHONY: 64bit openmp

all: prepare clean ${OUTDIR}/${OUTBIN}

64bit: all
openmp: all
debug: all

prepare:
	@mkdir -p ${OBJDIR}
	@mkdir -p ${OUTDIR}

${OBJDIR}/stdunicode.o:
	@$(GPP) -c ${SOURCEDIR}/stdunicode.cpp ${CFLAGS} -o $@

${OBJDIR}/rawscale.o:
	@$(GPP) -c ${SOURCEDIR}/rawscale.cpp ${CFLAGS} -o $@

${OBJDIR}/rawprocessor.o:
	@$(GPP) -c ${SOURCEDIR}/rawprocessor.cpp ${CFLAGS} -o $@

${OBJDIR}/rawfilters.o:
	@$(GPP) -c ${SOURCEDIR}/rawfilters.cpp ${CFLAGS} -o $@
	
${OBJDIR}/rawimgtk.o:
	@$(GPP) -c ${SOURCEDIR}/rawimgtk.cpp ${CFLAGS} -o $@

${OBJDIR}/rawimgtk_tmodrago03.o:
	@$(GPP) -c ${SOURCEDIR}/rawimgtk_tmodrago03.cpp ${CFLAGS} -o $@

${OBJDIR}/rawimgtk_tmoreinhard05.o:
	@$(GPP) -c ${SOURCEDIR}/rawimgtk_tmoreinhard05.cpp ${CFLAGS} -o $@


${OUTDIR}/${OUTBIN}: ${OBJDIR}/rawscale.o \
					 ${OBJDIR}/stdunicode.o \
					 ${OBJDIR}/rawimgtk.o \
					 ${OBJDIR}/rawimgtk_tmodrago03.o \
					 ${OBJDIR}/rawimgtk_tmoreinhard05.o \
					 ${OBJDIR}/rawfilters.o \
					 ${OBJDIR}/rawprocessor.o
	@echo "Generating $@ ..."
	@$(AR) -q $@ ${OBJDIR}/*.o
	@cp -f ${SOURCEDIR}/rawprocessor.h ${OUTDIR}

clean:
	@echo "Cleaning built directories ..."
	@rm -rf ${OBJDIR}/*
	@rm -rf ${OUTDIR}/${OUTBIN}
