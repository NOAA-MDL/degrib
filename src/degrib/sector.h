#ifndef SECTOR_H
#define SECTOR_H

#include "type.h"
#include "genprobe.h"
#include "meta.h"

typedef struct {
   int index;
   sChar timeZone;   /* hours to add to local time to get UTC */
   sChar f_dayLight; /* daylight flag (pnt observes daylight savings time) */
} PntSectInfo;


int SectorFindGDS (gdsType *gds);

int isPntInASector (Point pnt);

int WhichSector (char *sectFile, Point pnt, sChar f_cells);

int GetSectorList (char *sectFile, size_t numPnts, Point *pnts, sChar f_cells,
                   const char *geoDataDir, PntSectInfo *pntInfo,
                   size_t *NumSect, char ***Sect);

#ifdef OLD_CODE
void sectExpandInName (size_t *NumInNames, char ***InNames, char **F_inTypes,
                       const char *filter, size_t numSect, char **sect,
                       sChar f_ndfdConven, size_t numNdfdVars,
                       uChar *ndfdVars);
#endif

void expandInName (size_t numInNames, char **inNames, char *f_inTypes,
                   char *filter, size_t numSect, char **sect,
                   sChar f_ndfdConven, size_t numElem, genElemDescript * elem,
                   size_t *NumOutNames, char ***OutNames);

#endif
