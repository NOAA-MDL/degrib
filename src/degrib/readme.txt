This directory is broken up into the following files:

cstart.c     Beginning of degrib program
tclstart.c   Beginning of tkdegrib program

tcldegrib.c  Tcl/Tk interface to degrib code.

userparse.c  Routines used by cstart.c and tcldegrib.c to store user choices.

degrib2.c    Main set of driver procedures for GRIB2.

degrib1.c    Main set of driver procedures for GRIB1.
grib1tab.c   Table look up for GRIB1 data. 

inventory.c  Set of routines to inventory the files.
probe.c      Set of routines used to "probe" a given set of lat/lon values
             for their data in the GRIB2 file.

write.c      Main set of output procedures.  (.flt, .shp, etc)

interp.c     Set of routines used to interpolate to coverage grid before
             saving to .flt file.

metaparse.c  Handles parsing the different arrays passed back to degrib2.c
metaprint.c  Handles printing the meta data and looking up stuff in the GRIB2
             documentation files.

scan.c       Helper routines to handle the possible scan methods used in GRIB2
             but is not really needed since the ungrib library always returns
             scan = 0100.

myutil.c     Helper routines, currently contains code for getting lines of
             data from a file when you don't know how much to allocate for it.

endian.c     Helper routines to assist with endian'ness related issues.

myerror.c    Helper routines to buffer error messages
             Primarily for sending back to Tcl/Tk when no stdout is available
