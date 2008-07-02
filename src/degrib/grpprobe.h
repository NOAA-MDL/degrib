#ifndef GRPPROBE_H
#define GRPPROBE_H

#include "type.h"
#include "sector.h"

#include "genprobe.h"

int matchCompare2 (const void *A, const void *B);

typedef struct {
   double validTime;
   int allElem[NDFD_MATCHALL + 1];
} collateType;

/*
void PrintSameDay1 (genMatchType *match, size_t pntIndex,
                    collateType *collate, size_t numCollate,
                    sChar pntTimeZone, sChar f_dayCheck);
*/

int MOTDProbe (uChar f_MOTD, size_t numPnts, Point * pnts,
               PntSectInfo *pntInfo, sChar f_pntType, char **labels,
               size_t numInFiles, char **inFiles, uChar f_fileType,
               sChar f_interp, sChar f_unit, double majEarth,
               double minEarth, sChar f_WxParse, sChar f_SimpleVer, sChar f_SimpleWWA,
               sChar f_valTime, double startTime, double endTime,
               size_t numNdfdVars, uChar *ndfdVars, char *f_inTypes,
               char *gribFilter, size_t numSector, char ** sector,
               sChar f_ndfdConven, uChar f_XML, sChar f_avgInterp);

int GraphProbe (uChar f_XML, size_t numPnts, Point * pnts,
                PntSectInfo *pntInfo, sChar f_pntType, char **labels,
                size_t numInFiles, char **inFiles, uChar f_fileType,
                sChar f_interp, sChar f_unit, double majEarth,
                double minEarth, sChar f_WxParse, sChar f_SimpleVer, sChar f_SimpleWWA,
                sChar f_valTime, double startTime, double endTime,
                size_t numNdfdVars, uChar *ndfdVars, char *f_inTypes,
                char *gribFilter, size_t numSector, char ** sector,
                sChar f_ndfdConven, sChar f_avgInterp);

#endif
