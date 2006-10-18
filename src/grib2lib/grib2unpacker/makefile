#!/bin/make
SHELL = /bin/sh

#
# Purpose: To compile the grib2 UNPACK library using g77.
# 9/2002:  Arthur Taylor (MDL/RSIS):  Created
# Operating Systems Tested on:
#   CYGWIN & MINGW (MS-Windows)
# Notes:
#   CYGWIN & MINGW users: I think it needs -lg2c
#   also -mno-cygwin may be useful.
#

############################
#
# MACRO Definitions
#
############################
F77 = g77
FFLAGS = -O2 

LIB_SOURCES =\
            boundary.f\
            endomess.f\
            ierx.f\
            locpos.f\
            rdieee.f\
            unboustro.f\
            unpk_grib2.f\
            unpkbg.f\
            unpkbmap.f\
            unpk_temp40.f\
            unpk_temp41.f\
            unpk_temp42.f\
            unpk_temp48.f\
            unpk_temp49.f\
            unpk_temp420.f\
            unpk_temp430.f\
            unpk_trace.f\
            unpk_cylinder.f\
            unpk_equator.f\
            unpk_lambert.f\
            unpk_mercator.f\
            unpk_orthographic.f\
            unpk_polster.f\
            unpk_azimuth.f\
            unearth.f\
            unpkoo.f\
            unpkpo.f\
            unpkps.f\
            unpk_refer.f\
            unpksecdif.f\
            unpk_sect0.f\
            unpk_sect1.f\
            unpk_sect2.f\
            unpk_sect3.f\
            unpk_sect4.f\
            unpk_sect5.f\
            unpk_sect6.f\
            unpk_sect7.f\
            unpk_smple.f\
            unpkcmbm.f\
            unpk_cmplx.f\
            unpklxbm.f\
            unpklx.f\
            unpk_endian.f\
            timpr.f

LIB_OBJECTS = ${LIB_SOURCES:.f=.o}

LIB_UNPACKER = libunpk_grib2lib.a

############################
#
# TARGETS
#
############################

all: ${LIB_UNPACKER} clean

${LIB_UNPACKER}: ${LIB_OBJECTS}
	-rm -f $@
	ar -rvu $@ ${LIB_OBJECTS}
	ranlib $@

clean:
	rm -f *.o

############################
#
# SUFFIXES
#
############################
.f.o:
	${F77} -c ${FFLAGS} $<
