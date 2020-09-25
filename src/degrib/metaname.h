#ifndef METANAME_H
#define METANAME_H

#include "type.h"
#include "meta.h"

char * centerLookup (unsigned short int center);

char * subCenterLookup (unsigned short int center,
                        unsigned short int subcenter);

char * processLookup (unsigned short int center, unsigned char process);

void ParseElemName (uChar mstrVersion, uShort2 center, uShort2 subcenter, int prodType,
                    int templat, int cat, int subcat, sInt4 lenTime,
                    uChar timeRangeUnit, uChar statProcessID,
                    uChar timeIncrType, uChar genID, uChar probType,
                    double lowerProb, double upperProb, char **name,
                    char **comment, char **unit, int *convert,
                    sChar percentile, uChar genProcess,
                    sChar f_fstValue, double fstSurfValue,
                    sChar f_sndValue, double sndSurfValue);

int ComputeUnit (int convert, char * origName, sChar f_unit, double *unitM,
                 double *unitB, char *name);
/*
int ComputeUnit (int prodType, int templat, int cat, int subcat, sChar f_unit,
                 double *unitM, double *unitB, char *name);
*/
typedef struct {
   char *name, *comment, *unit;
} GRIB2SurfTable;

GRIB2SurfTable Table45Index (int i, int *f_reserved, uShort2 center,
                             uShort2 subcenter);
/*
GRIB2SurfTable Table45Index (int i, int *f_reserved);
int Table45Index (int i);
*/

int IsData_NDFD (unsigned short int center, unsigned short int subcenter);

int IsData_MOS (unsigned short int center, unsigned short int subcenter);

void ParseLevelName (unsigned short int center, unsigned short int subcenter,
                     uChar surfType, double value, sChar f_sndValue,
                     double sndValue, char **shortLevelName,
                     char **longLevelName);

#endif
