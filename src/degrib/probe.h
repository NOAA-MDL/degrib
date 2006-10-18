/*****************************************************************************
 * probe.h
 *
 * DESCRIPTION
 *    This file contains the code that is called by cstart.c and tcldegrib.c
 * to handle the "Probe" or "-P" command.
 *
 * HISTORY
 *   12/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#ifndef PROBE_H
#define PROBE_H

#include "userparse.h"
#include "meta.h"
#include "degrib2.h"

void GRIB2ProbeLabel0 (FILE **pnt_fps, char *f_firstFps,
                       char *separator, int numPnts, char **labels,
                       sChar f_surface);

void GRIB2ProbeLabel1 (FILE **pnt_fps, char *f_firstFps,
                       char *separator, uInt4 numPnts,
                       char **labels, sChar f_surface, sChar f_cells);

int GRIB2ProbeOpenOutFile (userType *usr, int numPnts, char **pntFiles,
                           FILE *** pnt_fps, char ** f_firstFps);

void GRIB2ProbeCloseOutFile (userType *usr, int numPnts, FILE ** pnt_fps,
                             char * f_firstFps);

int GRIB2Probe (userType * usr, int numPnts, Point *pnts, char **labels,
                char **pntFiles);

#endif
