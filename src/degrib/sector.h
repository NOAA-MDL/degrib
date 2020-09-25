#ifndef SECTOR_H
#define SECTOR_H

#include "type.h"
#include "genprobe.h"
#include "meta.h"

typedef struct {
/*   int index;*/    /* was an index into which sector this was.*/
   sChar f_sector[NDFD_OCONUS_UNDEF]; /* Array of oconus sectors that the
                      * point falls in.  Initialized to values of UNDEF.
                      * This is a sorted list.  The first entry is the value
                      * of the 'primary' sector.  The second is the
                      * 'secondary' sector, trailing off to NDFD_OCONUS_UNDEF
                      * values */
   double X[NDFD_OCONUS_UNDEF]; /* Array of X grid cell locations for the
                      * points in the sector defined by f_sector[i],
                      * If f_sector[i] is NDFD_OCONUS_UNDEF, then -1. */
   double Y[NDFD_OCONUS_UNDEF]; /* Array of Y grid cell locations for the
                      * points in the sector defined by f_sector[i],
                      * If f_sector[i] is NDFD_OCONUS_UNDEF, then -1. */
   LatLon pnt1[NDFD_OCONUS_UNDEF]; /* Array of lat/lons for the floor(x,y)
                      * point in the sector defined by f_sector[i] */
   LatLon pnt2[NDFD_OCONUS_UNDEF]; /* Array of lat/lons for the floor(x),ceil(y)
                      * point in the sector defined by f_sector[i] */
   LatLon pnt3[NDFD_OCONUS_UNDEF]; /* Array of lat/lons for the ceil(x),floor(y)
                      * point in the sector defined by f_sector[i] */
   LatLon pnt4[NDFD_OCONUS_UNDEF]; /* Array of lat/lons for the ceil(x,y)
                      * point in the sector defined by f_sector[i] */
   sChar timeZone;   /* hours to add to local time to get UTC based on
                      * primary sector (aka f_sector[0]) */
   sChar f_dayLight; /* daylight flag (pnt observes daylight savings time)
                      * based on primary sector (aka f_sector[0]) */
   float elev;       /* Elevation for a cell based on 'primary' sector
                      * (aka f_sector[0]), or 9999 if it can't be
                      * determined. */
    /* Array of elevations for a cell in each
                      * sector it falls in, or 9999 if it can't be
                      * determined. */
   sChar numSector;  /* number of sectors point falls in */
   char *cwa;        /* The CWA the point fall in. */
   int startNum;     /* The first index in the match structure in which
                      * matches for this point can be found, as defined by
                      * sector. */
   int endNum;       /* The last index in the match structure in which matches
                      * for this point can be found, as defined by sector. */
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
