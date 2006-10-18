#ifndef CUBE_H
#define CUBE_H

#include "degrib2.h"
#include "userparse.h"

int Grib2Database (userType * usr, IS_dataType * is, grib_MetaData * meta);

int Grib2DataConvert(userType * usr);

/* Moved to genprobe.c 
int Grib2DataProbe (userType * usr, int numPnts, Point *pnts, char **labels,
                    char **pntFiles);
*/

#endif
