#!/bin/make
SHELL = /bin/sh

#
# Purpose: To compile the grib2 PACK library using g77.
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
CC = gcc
FFLAGS = -O2 -mno-cygwin -Wall -ff90
CFLAGS = -O2 -Wall -ansii -mno-cygwin

LIB_PACKER_F_SOURCES = \
            boustro_int.f\
            boustro_flt.f\
            check_flt.f\
            check_int.f\
            earth.f\
            flr.f\
            flt_map.f\
            fmkieee.f\
            int_map.f\
            length.f\
            pack_gp.f\
            pack_opt.f\
            pk_azimuth.f\
            pk_bmap.f\
            pk_c7.f\
            pk_cmplx.f\
            pk_cylinder.f\
            pk_equator.f\
            pk_grib2.f\
            pk_lambert.f\
            pk_mercator.f\
            pk_missp.f\
            pk_nomiss.f\
            pk_orthographic.f\
            pk_polster.f\
            pk_sect0.f\
            pk_sect1.f\
            pk_sect2.f\
            pk_sect3.f\
            pk_sect4.f\
            pk_sect5.f\
            pk_sect6.f\
            pk_sect7.f\
            pk_sect8.f\
            pk_smple.f\
            pk_s7.f\
            pk_temp40.f\
            pk_temp41.f\
            pk_temp42.f\
            pk_temp48.f\
            pk_temp49.f\
            pk_temp420.f\
            pk_temp430.f\
            pk_trace.f\
            pkbg.f\
            prepr.f\
            prep_flt.f\
            prep_int.f\
            prep_noval.f\
            prep_sect2_int.f\
            prep_sect2_real.f\
            pk_endian.f\
            reduce.f\
            timpr.f

LIB_PACKER_C_SOURCES = pk_swap.c

LIB_PACKER_O_FILES = ${LIB_PACKER_F_SOURCES:.f=.o} \
                     ${LIB_PACKER_C_SOURCES:.c=.o}

LIB_PACKER = libpk_grib2lib.a

############################
#
# TARGETS
#
############################

all: ${LIB_PACKER}

${LIB_PACKER}: ${LIB_PACKER_O_FILES}
	-rm -f $@
	ar -rvu $@ ${LIB_PACKER_O_FILES}

clean:
	rm -f *.o ${LIB_PACKER} *.ln

############################
#
# SUFFIXES
#
############################
.f.o:
	    ${F77} -c ${FFLAGS} $<
.c.o:
	    ${CC} -c ${CFLAGS} $<

