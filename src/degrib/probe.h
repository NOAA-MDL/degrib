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

int GRIB2Probe (userType * usr, IS_dataType * is, grib_MetaData * meta,
                int numPnts, Point *pnts, char **labels, char **pntFiles,
                sChar f_pntType);

#endif
