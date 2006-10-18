/*****************************************************************************
 * tcldegrib.h
 *
 * DESCRIPTION
 *    This file contains the Tcl extension library wrapper around the degrib
 * options.  What that means is, it contains all the interface code needed to
 * call the degrib routines from Tcl/Tk.
 *
 * HISTORY
 *    9/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#ifndef TCLDEGRIB_H
#define TCLDEGRIB_H

int Grib2_Init (Tcl_Interp *interp);

#endif
