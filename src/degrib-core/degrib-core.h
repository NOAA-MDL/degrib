#ifndef MDL_G2C_H
#define MDL_G2C_H

/* Pays attention to the following compile time #defines:
 * 1a) __64BIT__ => use 64 bit compilation as opposed to 32 bit
 * 1b) SIZEOF_LONG_INT =8 vs =4 => use 64 bit compilation as opposed to 32 bit
 * 2) WORDS_BIGENDIAN => files are bigendian
 * 3) DEBUG => Pay attention to assert statements.
 * 4) MEMWATCH => Track memory.
 */

#include <stdarg.h>
#include <stdio.h>

#include "libaat_type.h"

/* From scan.h */
#ifndef GRIB2BIT_ENUM
#define GRIB2BIT_ENUM
/* See rule (8) bit 1 is most significant, bit 8 least significant. */
enum {GRIB2BIT_1=128, GRIB2BIT_2=64, GRIB2BIT_3=32, GRIB2BIT_4=16,
      GRIB2BIT_5=8, GRIB2BIT_6=4, GRIB2BIT_7=2, GRIB2BIT_8=1};
#endif

void XY2ScanIndex (sInt4 *Row, sInt4 x, sInt4 y, uChar scan, sInt4 Nx,
                   sInt4 Ny);
void ScanIndex2XY (sInt4 row, sInt4 *X, sInt4 *Y, uChar scan, sInt4 Nx,
                   sInt4 Ny);

/* From grib2api.h */
void unpk_grib2 (sInt4 *kfildo, float *ain, sInt4 *iain, sInt4 *nd2x3,
                 sInt4 *idat, sInt4 *nidat, float *rdat, sInt4 *nrdat,
                 sInt4 *is0, sInt4 *ns0, sInt4 *is1, sInt4 *ns1, sInt4 *is2,
                 sInt4 *ns2, sInt4 *is3, sInt4 *ns3, sInt4 *is4, sInt4 *ns4,
                 sInt4 *is5, sInt4 *ns5, sInt4 *is6, sInt4 *ns6, sInt4 *is7,
                 sInt4 *ns7, sInt4 *ib, sInt4 *ibitmap, sInt4 *ipack,
                 sInt4 *nd5, float *xmissp, float *xmisss, sInt4 *inew,
                 sInt4 *iclean, sInt4 *l3264b, sInt4 *iendpk, sInt4 *jer,
                 sInt4 *ndjer, sInt4 *kjer);
void unpk_g2ncep(sInt4 *kfildo, float *ain, sInt4 *iain, sInt4 *nd2x3,
                 sInt4 *idat, sInt4 *nidat, float *rdat, sInt4 *nrdat,
                 sInt4 *is0, sInt4 *ns0, sInt4 *is1, sInt4 *ns1,
                 sInt4 *is2, sInt4 *ns2, sInt4 *is3, sInt4 *ns3,
                 sInt4 *is4, sInt4 *ns4, sInt4 *is5, sInt4 *ns5,
                 sInt4 *is6, sInt4 *ns6, sInt4 *is7, sInt4 *ns7,
                 sInt4 *ib, sInt4 *ibitmap, unsigned char *c_ipack,
                 sInt4 *nd5, float *xmissp, float *xmisss,
                 sInt4 *inew, sInt4 *iclean, sInt4 *l3264b,
                 sInt4 *iendpk, sInt4 *jer, sInt4 *ndjer, sInt4 *kjer);
int C_pkGrib2 (unsigned char *cgrib, sInt4 *sec0, sInt4 *sec1,
               unsigned char *csec2, sInt4 lcsec2,
               sInt4 *igds, sInt4 *igdstmpl, sInt4 *ideflist,
               sInt4 idefnum, sInt4 ipdsnum, sInt4 *ipdstmpl,
               float *coordlist, sInt4 numcoord, sInt4 idrsnum,
               sInt4 *idrstmpl, float *fld, sInt4 ngrdpts,
               sInt4 ibmap, sInt4 *bmap);

/* From engribapi.h */
typedef struct {
   sInt4 sec0[2];  /* info for section 0 */
   sInt4 sec1[13]; /* info for section 1 */
   uChar *sec2;   /* section 2 free form info */
   sInt4 lenSec2;  /* length of section 2 free form info */
   sInt4 gds[5];
   sInt4 *gdsTmpl; /* grid definition template (mapgrid) */
   sInt4 lenGdsTmpl; /* length of grid definition template (mapgrid) */
   sInt4 *idefList;
   sInt4 idefnum;

   sInt4 ipdsnum; /* Product Definition Template Number (Code Table 4.0) */
   sInt4 *pdsTmpl; /* Contains the data values for the specified Product
                    * Definition Template (N=ipdsnum).  Each element of this
                    * integer array contains an entry (in the order
                    * specified) of Product Definition Template 4.N */
   sInt4 lenPdsTmpl; /* length of product definition template*/
   float *coordlist; /* Array containing floating point values intended to
                      * document the vertical discretization associated to
                      * model data on hybrid coordinate vertical levels. */
   sInt4 numcoord; /* number of values in array coordlist. */

   sInt4 idrsnum; /* Data Representation Template Number (Code Table 5.0) */
   sInt4 *drsTmpl; /* Contains the data values for the specified Data
                    * Representation Template (N=idrsnum).  Each element of
                    * this integer array contains an entry (in the order
                    * specified) of Data Representation Template 5.N. Note
                    * that some values in this template (eg. reference
                    * values, number of bits, etc...) may be changed by the
                    * data packing algorithms.  Use this to specify scaling
                    * factors and order of spatial differencing, if desired. */
   sInt4 lenDrsTmpl; /* length of data representation template*/

   float *fld;    /* Array of data points to pack. */
   sInt4 ngrdpts; /* Number of data points in grid. i.e. size of fld and bmap. */
   sInt4 ibmap;   /* Bitmap indicator ( see Code Table 6.0 )
                   * 0 = bitmap applies and is included in Section 6.
                   * 1-253 = Predefined bitmap applies
                   * 254 = Previously defined bitmap applies to this field
                   * 255 = Bit map does not apply to this product. */
   sInt4 *bmap;   /* Integer array containing bitmap to be added. (if ibmap=0) */
} enGribMeta;

typedef struct {
   uChar processID;     /* Statistical process method used. */
   uChar incrType;      /* Type of time increment between intervals */
   uChar timeRangeUnit; /* Time range unit. [Code Table 4.4] */
   sInt4 lenTime;       /* Range or length of time interval. */
   uChar incrUnit;      /* Unit of time increment. [Code Table 4.4] */
   sInt4 timeIncr;      /* Time increment between intervals. */
} sect4IntervalType;

void initEnGribMeta (enGribMeta *en);
void freeEnGribMeta (enGribMeta *en);
void fillSect0 (enGribMeta *en, uChar prodType);
void fillSect1 (enGribMeta *en, uShort2 center, uShort2 subCenter,
                uChar mstrVer, uChar lclVer, uChar refCode, sInt4 refYear,
                int refMonth, int refDay, int refHour, int refMin, int refSec,
                uChar prodStat, uChar typeData);
void fillSect2 (enGribMeta *en, uChar *sec2, sInt4 lenSec2);
int fillSect3 (enGribMeta *en, uShort2 tmplNum, double majEarth,
               double minEarth, sInt4 Nx, sInt4 Ny, double lat1, double lon1,
               double lat2, double lon2, double Dx, double Dy, uChar resFlag,
               uChar scanFlag, uChar centerFlag, sInt4 angle, sInt4 subDivis,
               double meshLat, double orientLon, double scaleLat1,
               double scaleLat2, double southLat, double southLon);
int fillSect4_0 (enGribMeta *en, uShort2 tmplNum, uChar cat, uChar subCat,
                 uChar genProcess, uChar bgGenID, uChar genID,
                 uChar f_valCutOff, sInt4 cutOff, uChar timeCode,
                 double foreSec, uChar surfType1, sChar surfScale1,
                 double dSurfVal1, uChar surfType2, sChar surfScale2,
                 double dSurfVal2);
int fillSect4_1 (enGribMeta *en, uShort2 tmplNum, uChar typeEnsemble,
                 uChar perturbNum, uChar numFcsts);
int fillSect4_2 (enGribMeta *en, uShort2 tmplNum, uChar numFcsts,
                 uChar derivedFcst);
int fillSect4_5 (enGribMeta *en, uShort2 tmplNum, uChar numFcsts,
                 uChar foreProbNum, uChar probType, sChar lowScale,
                 double dlowVal, sChar upScale, double dupVal);
int fillSect4_8 (enGribMeta *en, uShort2 tmplNum, sInt4 endYear, int endMonth,
                 int endDay, int endHour, int endMin, int endSec,
                 uChar numInterval, sInt4 numMissing,
                 sect4IntervalType * interval);
int fillSect4_9 (enGribMeta *en, uShort2 tmplNum, uChar numFcsts,
                 uChar foreProbNum, uChar probType, sChar lowScale,
                 double dlowVal, sChar upScale, double dupVal, sInt4 endYear,
                 int endMonth, int endDay, int endHour, int endMin,
                 int endSec, uChar numInterval, sInt4 numMissing,
                 sect4IntervalType * interval);
int fillSect4_10 (enGribMeta *en, uShort2 tmplNum, int percentile,
                  sInt4 endYear, int endMonth, int endDay, int endHour,
                  int endMin, int endSec, uChar numInterval, sInt4 numMissing,
                  sect4IntervalType * interval);
int fillSect4_12 (enGribMeta *en, uShort2 tmplNum, uChar numFcsts,
                  uChar derivedFcst, sInt4 endYear, int endMonth, int endDay,
                  int endHour, int endMin, int endSec, uChar numInterval,
                  sInt4 numMissing, sect4IntervalType * interval);
int fillSect5 (enGribMeta *en, uShort2 tmplNum, sShort2 BSF, sShort2 DSF,
               uChar fieldType, uChar f_miss, float missPri, float missSec,
               uChar orderOfDiff);
int fillGrid (enGribMeta *en, double *data, sInt4 lenData, sInt4 Nx, sInt4 Ny,
              sInt4 ibmap, sChar f_boustify, uChar f_miss, float missPri,
              float missSec);

#endif
