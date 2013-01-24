/*****************************************************************************
 * genprobe.c
 *
 * DESCRIPTION
 *    This file contains the generic probing routines.
 *
 * HISTORY
 *   12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myerror.h"
#include "myutil.h"
#include "genprobe.h"
#include "probe.h"
#include "cube.h"
#include "myassert.h"
#include "mymapf.h"
#include "scan.h"
#include "interp.h"
#include "database.h"
#include "tendian.h"
#include "weather.h"
#include "hazard.h"
#include "sector.h"
#include "grpprobe.h"
#ifdef _DWML_
#include "xmlparse.h"
#endif
#ifdef MEMWATCH
#include "memwatch.h"
#endif
#include "hazard.h"

/* *INDENT-OFF* */
/* Problems using MISSING to denote all possible, since subcenter = Missing
 * is defined for NDFD. */
/* A good way to get these numbers is "-C -IS0 -stdout" */
/* Also look for a document called genelem.txt in /degrib/src/degrib" */
static const genElemDescript NdfdElements[] = {
/* 0 */   {NDFD_MAX,2, 8,MISSING_2,2,0, 0,8,0,4,-1,12, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 1 */   {NDFD_MIN,2, 8,MISSING_2,2,0, 0,8,0,5,-1,12, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 2 */   {NDFD_POP,2, 8,MISSING_2,2,0, 0,9,1,8,-1,12, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,254,3},
/* 3 */   {NDFD_TEMP,2, 8,MISSING_2,2,0, 0,0,0,0,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 4 */   {NDFD_WD,2, 8,MISSING_2,2,0, 0,0,2,0,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 5 */   {NDFD_WS,2, 8,MISSING_2,2,0, 0,0,2,1,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 6 */   {NDFD_TD,2, 8,MISSING_2,2,0, 0,0,0,6,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 7 */   {NDFD_SKY,2, 8,MISSING_2,2,0, 0,0,6,1,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 8 */   {NDFD_QPF,2, 8,MISSING_2,2,0, 0,8,1,8,-1,6, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 9 */   {NDFD_SNOW,2, 8,MISSING_2,2,0, 0,8,1,29,-1,6, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 10 */  {NDFD_ICEACC,2, 8,MISSING_2,2,0, 0,8,1,227,-1,6, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 11 */  {NDFD_WX,2, 8,MISSING_2,2,0, 0,0,1,192,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 12 */  {NDFD_WH,2, 8,MISSING_2,2,0, 10,0,0,5,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 13 */  {NDFD_AT,2, 8,MISSING_2,2,0, 0,0,0,193,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 14 */  {NDFD_RH,2, 8,MISSING_2,2,0, 0,0,1,1,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 15 */  {NDFD_WG,2, 8,MISSING_2,2,0, 0,0,2,22,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 16 */  {NDFD_WWA,2, 8,MISSING_2,2,0, 0,0,19,217,-1,0, 1,0.0,0.0 ,0,-1,-1,-1,-1},

/* 17 */  {NDFD_INC34,2, 8,MISSING_2,2,0, 0,9,2,1,-1,6, 103,10.0,0.0 ,1,0,0,17491,3},
/* 18 */  {NDFD_INC50,2, 8,MISSING_2,2,0, 0,9,2,1,-1,6, 103,10.0,0.0 ,1,0,0,25722,3},
/* 19 */  {NDFD_INC64,2, 8,MISSING_2,2,0, 0,9,2,1,-1,6, 103,10.0,0.0 ,1,0,0,32924,3},
/* 20 */  {NDFD_CUM34,2, 8,MISSING_2,2,0, 0,9,2,1,0,0, 103,10.0,0.0 ,1,0,0,17491,3},
/* 21 */  {NDFD_CUM50,2, 8,MISSING_2,2,0, 0,9,2,1,0,0, 103,10.0,0.0 ,1,0,0,25722,3},
/* 22 */  {NDFD_CUM64,2, 8,MISSING_2,2,0, 0,9,2,1,0,0, 103,10.0,0.0 ,1,0,0,32924,3},

/* 23 */  {NDFD_FWXWINDRH,2, 8,MISSING_2,2,0, 0,9,192,192,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},
/* 24 */  {NDFD_FWXTSTORM,2, 8,MISSING_2,2,0, 0,9,192,194,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},

/* 25 */  {NDFD_CONHAZ,2, 8,MISSING_2,2,0, 0,8,19,194,-1,24, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 26 */  {NDFD_PTORN,2, 8,MISSING_2,2,0, 0,9,19,197,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},
/* 27 */  {NDFD_PHAIL,2, 8,MISSING_2,2,0, 0,9,19,198,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},
/* 28 */  {NDFD_PTSTMWIND,2, 8,MISSING_2,2,0, 0,9,19,199,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},
/* 29 */  {NDFD_PXTORN,2, 8,MISSING_2,2,0, 0,9,19,200,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},
/* 30 */  {NDFD_PXHAIL,2, 8,MISSING_2,2,0, 0,9,19,201,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},
/* 31 */  {NDFD_PXTSTMWIND,2, 8,MISSING_2,2,0, 0,9,19,202,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},
/* Following two lines changed from 203->215 and 204->216 9/19/2007 */
/* 32 */  {NDFD_PSTORM,2, 8,MISSING_2,2,0, 0,9,19,215,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},
/* 33 */  {NDFD_PXSTORM,2, 8,MISSING_2,2,0, 0,9,19,216,-1,24, 1,0.0,0.0 ,1,GRIB2MISSING_s4,-1,0,0},

/* If we have to change the 0,0,0,0 for lower/upper Value/Factor, also update
 * cube.c :: line 634, 655 */
   /* the 6 in the length of time column stands for 6 days... */
/* 34 */   {NDFD_TMPABV14D,2, 8,MISSING_2,2,0, 0,9,0,0,-1,6, 1,0.0,0.0, 3,0,0,0,0},
/* 35 */   {NDFD_TMPBLW14D,2, 8,MISSING_2,2,0, 0,9,0,0,-1,6, 1,0.0,0.0, 0,0,0,0,0},
/* 36 */   {NDFD_PRCPABV14D,2, 8,MISSING_2,2,0, 0,9,1,8,-1,6, 1,0.0,0.0, 3,0,0,0,0},
/* 37 */   {NDFD_PRCPBLW14D,2, 8,MISSING_2,2,0, 0,9,1,8,-1,6, 1,0.0,0.0, 0,0,0,0,0},
   /* the 1 in the length of time column stands for 1 month... */
/* 38 */   {NDFD_TMPABV30D,2, 8,MISSING_2,2,0, 0,9,0,0,-1,1, 1,0.0,0.0, 3,0,0,0,0},
/* 39 */   {NDFD_TMPBLW30D,2, 8,MISSING_2,2,0, 0,9,0,0,-1,1, 1,0.0,0.0, 0,0,0,0,0},
/* 40 */   {NDFD_PRCPABV30D,2, 8,MISSING_2,2,0, 0,9,1,8,-1,1, 1,0.0,0.0, 3,0,0,0,0},
/* 41 */   {NDFD_PRCPBLW30D,2, 8,MISSING_2,2,0, 0,9,1,8,-1,1, 1,0.0,0.0, 0,0,0,0,0},
   /* the 3 in the length of time column stands for 3 months... */
/* 42 */   {NDFD_TMPABV90D,2, 8,MISSING_2,2,0, 0,9,0,0,-1,3, 1,0.0,0.0, 3,0,0,0,0},
/* 43 */   {NDFD_TMPBLW90D,2, 8,MISSING_2,2,0, 0,9,0,0,-1,3, 1,0.0,0.0, 0,0,0,0,0},
/* 44 */   {NDFD_PRCPABV90D,2, 8,MISSING_2,2,0, 0,9,1,8,-1,3, 1,0.0,0.0, 3,0,0,0,0},
/* 45 */   {NDFD_PRCPBLW90D,2, 8,MISSING_2,2,0, 0,9,1,8,-1,3, 1,0.0,0.0, 0,0,0,0,0},
/* 46 */   {NDFD_MAXRH,2, 8,MISSING_2,2,0, 0,8,1,27,-1,12, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 47 */   {NDFD_MINRH,2, 8,MISSING_2,2,0, 0,8,1,198,-1,12, 1,0.0,0.0 ,0,-1,-1,-1,-1},
/* 48 */   {LAMP_TSTMPRB,2, 7,14,5,108, 0,8,19,2,-1,2, 1,0.0,0.0, 0,-1,-1,-1,-1},

/* 49 */  {RTMA_PRECIPA,2, 7,0,2,109, 0,8,1,8,-1,1, 1,0.0,0.0, 0,-1,-1,-1,-1}, /* genProcess = 2 forecast? */
/* 50 */  {RTMA_SKY,2, 7,0,8,109, 0,0,6,1,-1,0, 200,0.0,0.0, 0,-1,-1,-1,-1},  /* genProcess = 8 observation */
/* 51 */  {RTMA_TD,2,  7,4,0,109, 0,0,0,6,-1,0, 103,2.0,0.0, 0,-1,-1,-1,-1},
/* 52 */  {RTMA_TEMP,2, 7,4,0,109, 0,0,0,0,-1,0, 103,2.0,0.0, 0,-1,-1,-1,-1},
/* 53 */  {RTMA_UTD,2, 7,4,7,109, 0,0,0,6,-1,0, 103,2.0,0.0, 0,-1,-1,-1,-1},
/* 54 */  {RTMA_UTEMP,2, 7,4,7,109, 0,0,0,0,-1,0, 103,2.0,0.0, 0,-1,-1,-1,-1},
/* 55 */  {RTMA_UWDIR,2, 7,4,7,109, 0,0,2,0,-1,0, 103,10.0,0.0, 0,-1,-1,-1,-1},
/* 56 */  {RTMA_UWSPD,2, 7,4,7,109, 0,0,2,1,-1,0, 103,10.0,0.0, 0,-1,-1,-1,-1},
/* 57 */  {RTMA_WDIR,2, 7,4,0,109, 0,0,2,0,-1,0, 103,10.0,0.0, 0,-1,-1,-1,-1},
/* 58 */  {RTMA_WSPD,2, 7,4,0,109, 0,0,2,1,-1,0, 103,10.0,0.0, 0,-1,-1,-1,-1},
/* 59 */  {NDFD_CANL,2, 8,MISSING_2,2,0, 2,8,1,192,-1,6, 1,0.0,0.0 ,0,-1,-1,-1,-1},

#ifdef GFSEKD
/* 60 */  {GFSEKDMOS_MAXT,2, 7,14,2,96,   0,10,0,4,-1,-1, 103,2,0,  0,-1,-1,-1,-1},
/* 61 */  {GFSEKDMOS_MINT,2, 7,14,2,96,   0,10,0,4,-1,-1, 103,2,0,  0,-1,-1,-1,-1},
/* 62 */  {GFSEKDMOS_TEMP,2, 7,14,2,96,   0,10,0,4,-1,-1, 103,2,0,  0,-1,-1,-1,-1},
/* 63 */  {GFSEKDMOS_TD,2, 7,14,2,96,   0,10,0,4,-1,-1, 103,2,0,  0,-1,-1,-1,-1},
/* 64 */  {GFSEKDMOS_QPF,2, 7,14,2,96,   0,10,0,4,-1,-1, 103,2,0,  0,-1,-1,-1,-1},
#endif

   {NDFD_UNDEF,2, MISSING_2,MISSING_2,MISSING_1,MISSING_1,
                  MISSING_1,MISSING_2,MISSING_1,MISSING_1,-1,0,
                  MISSING_1,0.0,0.0, 0,-1,-1,-1,-1},
   {NDFD_MATCHALL,0, MISSING_2,MISSING_2,MISSING_1,MISSING_1,
                     MISSING_1,MISSING_2,MISSING_1,MISSING_1,-1,0,
                     MISSING_1,0.0,0.0, 0,-1,-1,-1,-1}
};

static const uChar NdfdElementsLen = (sizeof (NdfdElements) /
                                      sizeof (NdfdElements[0]));

/* *INDENT-ON* */

/*****************************************************************************
 * gen_NDFD_NDGD_Lookup() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Return the NDFD enumeration of the given character string (or UNDEF).
 *
 * ARGUMENTS
 *          str = A pointer to the char string to look up. (Input)
 *    f_toLower = Perform strToLower in this procedure (Input)
 * f_ndfdConven = 0 => use short name conventions (see metaname.c)
 *                1 => use standard NDFD file naming convention
 *                2 => use verification NDFD file naming convention (Input)
 *
 * RETURNS: int
 *   The desired NDFD enumeration value.
 *
 *  1/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *   see meta.h for the following:
 *   enum { NDFD_MAX, NDFD_MIN, NDFD_POP, NDFD_TEMP, NDFD_WD, NDFD_WS,
 *          NDFD_TD, NDFD_SKY, NDFD_QPF, NDFD_SNOW, NDFD_WX, NDFD_WH,
 *          NDFD_AT, NDFD_RH, NDFD_WG, NDFD_INC34, NDFD_INC50, NDFD_INC64,
 *          NDFD_CUM34, NDFD_CUM50, NDFD_CUM64, NDFD_CONHAZ, NDFD_PTORN,
 *          NDFD_PHAIL, NDFD_PTSTMWIND, NDFD_PXTORN, NDFD_PXHAIL, 
 *          NDFD_TMPBLW14D, NDFD_PRCPABV14D, NDFD_PRCPBLW14D, NDFD_TMPABV30D, 
 *          NDFD_TMPBLW30D, NDFD_PRCPABV30D, NDFD_PRCPBLW30D, NDFD_TMPABV90D, 
 *          NDFD_TMPBLW90D, NDFD_PRCPABV90D, NDFD_PRCPBLW90D, RTMA_PRECIPA, 
 *          RTMA_SKY, RTMA_TD, RTMA_TEMP, RTMA_UTD, RTMA_UTEMP, RTMA_UWDIR,
 *          RTMA_UWSPD, RTMA_WDIR, RTMA_WSPD, NDFD_UNDEF, NDFD_MATCHALL };
 *
 *****************************************************************************
 */
/* these are a string tolower on the short name of each NDFD element */
static char *NDFD_Type[] = { "maxt", "mint", "pop12", "t", "winddir",
   "windspd", "td", "sky", "qpf", "snowamt", "iceaccum", "wx", "waveheight",
   "apparentt", "rh", "windgust", "wwa", "probwindspd34i", "probwindspd50i",
   "probwindspd64i", "probwindspd34c", "probwindspd50c", "probwindspd64c",
   "probfirewx24", "probdrylightning24", "convoutlook", "tornadoprob", "hailprob",
   "windprob", "xtrmtornprob", "xtrmhailprob", "xtrmwindprob", "totalsvrprob",
   "totalxtrmprob", "probtmpabv144", "probtmpblw144", "probprcpabv144",
   "probprcpblw144", "probtmpabv01m", "probtmpblw01m", "probprcpabv01m",
   "probprcpblw01m", "probtmpabv03m", "probtmpblw03m", "probprcpabv03m",
   "probprcpblw03m", "maxrh", "minrh", "tstm02", "apcp01", "tcdc", "dpt",
   "tmp", "dpterr", "tmperr", "wdirerr", "winderr", "wdir", "wind", "canl", NULL
};
/* These match the convention on official download pages (cube names). */
static char *NDFD_File[] = { "maxt", "mint", "pop12", "temp", "wdir",
   "wspd", "td", "sky", "qpf", "snow", "iceaccum", "wx", "waveh", "apt", "rhm",
   "wgust", "wwa", "tcwspdabv34i", "tcwspdabv50i", "tcwspdabv64i",
   "tcwspdabv34c", "tcwspdabv50c", "tcwspdabv64c", "critfireo",
   "dryfireo", "conhazo", "ptornado", "phail", "ptstmwinds", "pxtornado",
   "pxhail", "pxtstmwinds", "ptotsvrtstm", "ptotxsvrtstm", "tmpabv14d",
   "tmpblw14d", "prcpabv14d", "prcpblw14d", "tmpabv30d", "tmpblw30d",
   "prcpabv30d", "prcpblw30d", "tmpabv90d", "tmpblw90d", "prcpabv90d",
   "prcpblw90d", "maxrh", "minrh", "tstmprb", "precipa_r", "sky_r", "td_r",
   "temp_r", "utd", "utemp", "uwdir", "uwspd", "wdir_r", "wspd_r", "canl", NULL
};
/* A (mostly) 2 letter abreviation scheme created with/for the verification
   group */
static char *NDFD_File2[] = { "mx", "mn", "po", "tt", "wd",
   "ws", "dp", "cl", "qp", "sn", "icea", "wx", "wh", "at", "rh", "wg", "wwa",
   "i3", "i5", "i6", "c3", "c5", "c6", "fwxwdrh", "fwxdry", "ch", "pt", "ph",
   "pw", "xt", "xh", "xw", "ps", "xs", "ta6d", "tb6d", "pa6d", "pb6d", "ta1m",
   "tb1m", "pa1m", "pb1m", "ta3m", "tb3m", "pa3m", "pb3m", "mxrh", "mnrh",
   "tstmprb", "apcp01", "tcdc", "dpt", "tmp", "dpterr", "tmperr", "wdirerr",
   "winderr", "wdir", "wind","canl", NULL
};

uChar gen_NDFD_NDGD_Lookup (char *str, char f_toLower, char f_ndfdConven)
{
   int index;
   uChar elemNum;       /* The index into the table that matches str. */

   if (f_toLower)
      strToLower (str);
   if (f_ndfdConven == 0) {
      if (GetIndexFromStr (str, NDFD_Type, &index) < 0) {
         elemNum = NDFD_UNDEF;
      } else {
         elemNum = (uChar) index;
      }
   } else if (f_ndfdConven == 1) {
      if (GetIndexFromStr (str, NDFD_File, &index) < 0) {
         elemNum = NDFD_UNDEF;
      } else {
         elemNum = (uChar) index;
      }
   } else if (f_ndfdConven == 2) {
      if (GetIndexFromStr (str, NDFD_File2, &index) < 0) {
         elemNum = NDFD_UNDEF;
      } else {
         elemNum = (uChar) index;
      }
   } else {
      elemNum = NDFD_UNDEF;
   }
   return elemNum;
}

/*****************************************************************************
 * validMatch
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Determines whether or not an element's NDFD or RTMA match is to be returned 
 *  by the grid probe. Detemined by comparing the both the element's starting and 
 *  ending times with the user supplied startTime and endTime. 
 *
 * ARGUMENTS
 *
 * elemRefTime = Reference time of this element (Input).
 * elemEndTime = The end time of the duration the element is valid for (Input).
 * elemEnum    = Enum # (see meta.h) of element (Input).
 * f_valTime   = 0 false, 1 f_validStartTime, 2 f_validEndTime,
 *               3 both f_validStartTime, and f_validEndTime (Input)
 *   startTime = User supplied start time (Input).
 *     endTime = User supplied end time (Input).
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: int
 
 * HISTORY:
 * 11/2007 Paul Hershberg (MDL): Created.
 * 11/2007 Paul Hershberg (MDL): Added RTMA elements.
 *  6/2008 Paul Hershberg (MDL): Added Hazard element.
 *  8/2009 Paul Hershberg (MDL): Added Lamp Tstm element. 
 *  3/2012 Paul Hershberg (MDL): Added MaxRH and MinRH elements.
 *
 * NOTES:
 *****************************************************************************
 */
int validMatch(double elemEndTime, double elemRefTime, int elemEnum, 
               sChar f_valTime, double startTime, double endTime)
{
   double elemStartTime = 0.0; /* Element's starting time. Calculated by
                                * subtracting period from element's end time. */

   /* MAXT, MINT, the nine SPC convective hazard elements, and the two Fire Wx
    * elements. All have 24-hour periods.
    */
   if ((elemEnum == NDFD_MAX) || (elemEnum == NDFD_MIN) ||
       (elemEnum == NDFD_FWXWINDRH) || (elemEnum == NDFD_FWXTSTORM) ||
       (elemEnum == NDFD_CONHAZ) || (elemEnum == NDFD_PTORN) ||
       (elemEnum == NDFD_PHAIL) || (elemEnum == NDFD_PTSTMWIND) ||
       (elemEnum == NDFD_PXTORN) || (elemEnum == NDFD_PXHAIL) ||
       (elemEnum == NDFD_PXTSTMWIND) || (elemEnum == NDFD_PSTORM) ||
       (elemEnum == NDFD_PXSTORM) || (elemEnum == NDFD_MAXRH) ||
       (elemEnum == NDFD_MINRH)) {
      elemStartTime = elemEndTime - (24*3600);
      if (((f_valTime & 1) && (elemEndTime <= startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }

   /* POP12 and WaveHeight and EKDMOS. 12-hour period. */
   if ((elemEnum == NDFD_POP) || (elemEnum == NDFD_WH)) {
      elemStartTime = elemEndTime - (12*3600);
      if (((f_valTime & 1) && (elemEndTime <= startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }

   /* TEMP, WDIR, WSPD, TD, SKY, WX, AT, RH, and WG. 3-hour period.
    * Deal with the change to a 6-hour period after day 3 (72 hrs).
    */
   if ((elemEnum == NDFD_TEMP) || (elemEnum == NDFD_WD) || (elemEnum == NDFD_WS) ||
       (elemEnum == NDFD_TD) || (elemEnum == NDFD_SKY) || (elemEnum == NDFD_WX) ||
       (elemEnum == NDFD_AT) || (elemEnum == NDFD_RH) || (elemEnum == NDFD_WG)) {
      if ((elemEndTime - elemRefTime > (72*3600)) && (elemEnum != NDFD_WG)) {
         elemStartTime = elemEndTime - (6*3600);
      } else {
         elemStartTime = elemEndTime - (3*3600);
      }
      if (((f_valTime & 1) && (elemEndTime <= startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }

   /* QPF, SNOW, Ice Accumulation, and the six Tropical Wind Thresholds. 6-hour
    * period.
    */
   if ((elemEnum == NDFD_QPF) || (elemEnum == NDFD_SNOW) ||
       (elemEnum == NDFD_INC34) || (elemEnum == NDFD_INC50) ||
       (elemEnum == NDFD_INC64) || (elemEnum == NDFD_CUM34) ||
       (elemEnum == NDFD_CUM50) || (elemEnum == NDFD_CUM64) ||
       (elemEnum == NDFD_ICEACC) || (elemEnum == NDFD_CANL)) {
      elemStartTime = elemEndTime - (6*3600);
      if (((f_valTime & 1) && (elemEndTime <= startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }

   /* The four climate outlooks 8-14 days out. 6-day (144 hr)
    * period.
    */
   if ((elemEnum == NDFD_TMPABV14D) || (elemEnum == NDFD_TMPBLW14D) ||
       (elemEnum == NDFD_PRCPABV14D) || (elemEnum == NDFD_PRCPBLW14D)) {
      elemStartTime = elemEndTime - (6*24*3600);
      if (((f_valTime & 1) && (elemEndTime <= startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }

   /* The four climate outlooks 30 days out. 30-day (720 hr)
    * period.
    */
   if ((elemEnum == NDFD_TMPABV30D) || (elemEnum == NDFD_TMPBLW30D) ||
       (elemEnum == NDFD_PRCPABV30D) || (elemEnum == NDFD_PRCPBLW30D)) {
      elemStartTime = elemEndTime - (30*24*3600);
      if (((f_valTime & 1) && (elemEndTime <= startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }

   /* The four climate outlooks 90 days out. 90-day (2160 hr)
    * period.
    */
   if ((elemEnum == NDFD_TMPABV90D) || (elemEnum == NDFD_TMPBLW90D) ||
       (elemEnum ==  NDFD_PRCPABV90D) || (elemEnum == NDFD_PRCPBLW90D)) {
      elemStartTime = elemEndTime - (90*24*3600);
      if (((f_valTime & 1) && (elemEndTime <= startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }

   /* 10 RTMA elements and the instantaneous Hazard element. 1-hour period. */
   if ((elemEnum == RTMA_PRECIPA) || (elemEnum == RTMA_SKY) ||
       (elemEnum == RTMA_TD) || (elemEnum == RTMA_TEMP) ||
       (elemEnum == RTMA_UTD) || (elemEnum == RTMA_UTEMP) ||
       (elemEnum == RTMA_UWDIR) || (elemEnum == RTMA_UWSPD) ||
       (elemEnum == RTMA_WDIR) || (elemEnum == RTMA_WSPD) ||
       (elemEnum == NDFD_WWA)) {
      elemStartTime = elemEndTime - 3600;

      /* Notice no "=" sign in first part of if statement below, which differs
       * from NDFD element. This allows top of hour times to be chosen.
       */
      if (((f_valTime & 1) && (elemEndTime < startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }

   /* LAMP Tstm Prob. 2-hour period. */
   if (elemEnum == LAMP_TSTMPRB) {
      elemStartTime = elemEndTime - (2*3600);
      if (((f_valTime & 1) && (elemEndTime <= startTime)) ||
          ((f_valTime & 2) && (elemStartTime >= endTime))) {
         return 0;
      }
   }
   return 1;
}

/*****************************************************************************
 * genNdfdEnumToStr() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Return a pointer to the string that matches the NDFD enumeration, or
 * NULL for NDFD_MATCHALL, NDFD_UNDEF, or error.
 *
 * ARGUMENTS
 *     ndfdEnum = The NDFD Enumeration to look up.
 * f_ndfdConven = 0 => use short name conventions (see metaname.c)
 *                1 => use standard NDFD file naming convention
 *                2 => use verification NDFD file naming convention (Input)
 *
 * RETURNS: const char *
 *   The desired string associated with the given NDFD enumeration.
 *
 *  1/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *   see meta.h for the following:
 *   enum { NDFD_MAX, NDFD_MIN, NDFD_POP, NDFD_TEMP, NDFD_WD, NDFD_WS,
 *          NDFD_TD, NDFD_SKY, NDFD_QPF, NDFD_SNOW, NDFD_WX, NDFD_WH,
 *          NDFD_AT, NDFD_RH, NDFD_UNDEF, NDFD_MATCHALL };
 *****************************************************************************
 */
const char *genNdfdEnumToStr (uChar ndfdEnum, char f_ndfdConven)
{
   if (ndfdEnum >= NDFD_UNDEF)
      return NULL;
   if (f_ndfdConven == 0) {
      return (NDFD_Type[ndfdEnum]);
   } else if (f_ndfdConven == 1) {
      return (NDFD_File[ndfdEnum]);
   } else if (f_ndfdConven == 2) {
      return (NDFD_File2[ndfdEnum]);
   } else {
      return NULL;
   }
}

/*****************************************************************************
 * genElemInit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Initialize an element structure to the NDFD_UNDEF value.
 *
 * ARGUMENTS
 * elem = A pointer to the element to init. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
void genElemInit (genElemDescript * elem)
{
   memset (elem, 0, sizeof (genElemDescript));
   elem->ndfdEnum = NDFD_UNDEF;
}

/*****************************************************************************
 * genElemFree() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Free the data assocoated with an element.  This procedure currently is
 * just a place holder for future developments.
 *
 * ARGUMENTS
 * elem = A pointer to the element to free. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
void genElemFree (genElemDescript * elem)
{
   return;
}

/*****************************************************************************
 * genElemListInit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Make sure that the elem data structure is completed correctly. If
 * f_validNDFD is true, then it inits it based on the value in ndfdEnum,
 * otherwise it sets ndfdEnum to NDFD_UNDEF, and inits the values to
 * Missing.  Presumably the user would then init the other values themself.
 * If they didn't then they'd get all matches.
 *
 * ARGUMENTS
 *     numElem = Number of elements to init. (Input)
 *        elem = The element array to init. (Input/Output)
 * f_validNDFD = 1 ndfdEnum is already value, 0 ndfdEnum is not (Input)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *   Being phased out...
 *****************************************************************************
 */
#ifdef OLD_176
void genElemListInit (size_t numElem, genElemDescript * elem,
                      uChar f_validNDFD)
{
   size_t i;            /* Loop variable over number of elements. */
   uShort2 j;           /* Loop variable over number of NdfdElements */

   if (!f_validNDFD) {
      for (i = 0; i < numElem; i++) {
         genElemInit (elem + i);
      }
      return;
   }
   for (i = 0; i < numElem; i++) {
      /* Try guessing where it is in the table. */
      if ((elem[i].ndfdEnum < NdfdElementsLen) &&
          (NdfdElements[elem[i].ndfdEnum].ndfdEnum == elem[i].ndfdEnum)) {
         memcpy (&(elem[i]), &(NdfdElements[elem[i].ndfdEnum]),
                 sizeof (genElemDescript));
      } else {
         /* Guess Failed, search for it. */
         for (j = 0; j < NdfdElementsLen; j++) {
            if (NdfdElements[j].ndfdEnum == elem[i].ndfdEnum) {
               memcpy (&(elem[i]), &(NdfdElements[j]),
                       sizeof (genElemDescript));
               break;
            }
         }
         /* Couldn't find it. */
         if (j == NdfdElementsLen) {
#ifdef DEBUG
            printf ("Couldn't find %d\n", elem->ndfdEnum);
#endif
            genElemInit (elem + i);
         }
      }
   }
}
#endif

/*****************************************************************************
 * genElemListInit2() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Takes the set of flags for variables that the procedure cared about,
 * adjusts them based on the user choices, allocates memory for elem, and
 * copies the NDFD var structure to elem, for variables that scored at least
 * a "2" (1 interest from procedure + 1 interest from user), or (2 vital from
 * procedure + 0/1 from user).
 *
 * ARGUMENTS
 *   varFilter = A set of flags for each NDFD variable already set to calling
 *               procedure's interest level.  (1 means interested but ok if
 *               user wants to drop, 2 means vital, 0 means don't care)
 *               (Input/Output)
 * numNdfdVars = Number of user selected ndfdVars (Input)
 *    ndfdVars = The user selected NDFD variables. (Input)
 *     numElem = length of elem. (Input)
 *        elem = The element array to init. (Output)
 *
 * RETURNS: void
 *
 *  1/2006 Arthur Taylor (MDL): Created.
 *  5/2006 AAT: Modified to choose all only if calling procedure didn't select
 *              something.
 *
 * NOTES:
 *****************************************************************************
 */
void genElemListInit2 (uChar varFilter[NDFD_MATCHALL + 1],
                       size_t numNdfdVars, const uChar *ndfdVars,
                       size_t *numElem, genElemDescript ** elem)
{
   size_t i;            /* Loop variable over number of elements. */
/*   size_t valElem;*/  /* The current location in elem to copy to. */
   sChar f_force;       /* Flag whether the caller forced an element. */

   myAssert (NDFD_MATCHALL + 1 == NdfdElementsLen);
   myAssert (*elem == NULL);
   myAssert (*numElem == 0);

   for (i = 0; i < numNdfdVars; i++) {
      if (ndfdVars[i] < NDFD_MATCHALL + 1) {
         varFilter[ndfdVars[i]]++;
      }
   }
   /* If the user provided no elements, then treat it as if user had set all
    * of them. */
   if (numNdfdVars == 0) {
      /* Check if program forced a choice. */
      f_force = 0;
      for (i = 0; i < NDFD_MATCHALL + 1; i++) {
         if (varFilter[i] > 1) {
            f_force = 1;
         }
      }
      if (!f_force) {
         for (i = 0; i < NDFD_MATCHALL + 1; i++) {
            varFilter[i]++;
         }
      }
   }

   /* Walk through varFilter, when it is >= 2, copy it to valElem location. */
   *numElem = 0;
   for (i = 0; i < NDFD_MATCHALL + 1; i++) {
      if (varFilter[i] >= 2) {
         (*elem) = realloc((*elem), (*numElem + 1) * sizeof (genElemDescript));
         memcpy (&((*elem)[*numElem]), &(NdfdElements[i]),
                 sizeof (genElemDescript));
         *numElem = *numElem + 1;
      }
   }
   /* Reduce numElem to correct valElem */
/*   *numElem = valElem; */
}

/*****************************************************************************
 * genMatchInit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Initialize a match structure.
 *
 * ARGUMENTS
 * match = A pointer to the match to init. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
void genMatchInit (genMatchType * match)
{
   genElemInit (&(match->elem));
   match->value = NULL;
   match->numValue = 0;
   match->unit = NULL;
}

/*****************************************************************************
 * genValueFree() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Free the data assocoated with a value.
 *
 * ARGUMENTS
 * value = A pointer to the value to free. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void genValueFree (genValueType * value)
{
   if ((value->valueType == 1) || (value->valueType == 2)) {
      if (value->str != NULL) {
         free (value->str);
         value->str = NULL;
      }
   }
}

/*****************************************************************************
 * genMatchFree() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Free the data assocoated with a match.
 *
 * ARGUMENTS
 * match = A pointer to the match to free. (Output)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
void genMatchFree (genMatchType * match)
{
   size_t i;            /* Loop variable over number of values. */

   genElemFree (&(match->elem));
   if (match->value != NULL) {
      for (i = 0; i < match->numValue; i++) {
         genValueFree (match->value + i);
      }
      free (match->value);
      match->value = NULL;
   }
   if (match->unit != NULL) {
      free (match->unit);
   }
   match->numValue = 0;
   return;
}

/*****************************************************************************
 * genElemMatchMeta() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Determine if the meta data matches what the element is looking for.
 *
 * ARGUMENTS
 * elem = An element description structure to try to match. (Input)
 * meta = The meta data associated with the current grid. (Input)
 *
 * RETURNS: int
 *   1 if they match, 0 if they don't match.
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static int genElemMatchMeta (const genElemDescript * elem,
                             const grib_MetaData *meta)
{
#ifdef DEBUG
/*
   if (((meta->pds2.sect4.templat == 8) ||
        (meta->pds2.sect4.templat == 9)) &&
       (meta->pds2.sect4.numInterval == 1)) {
   printf ("PI1 %d %d,%d,%d,%d %d,%d,%d,%d,%f,%ld %d,%f,%f %d,%ld,%d,%ld,%d\n", meta->GribVersion,
           meta->center, meta->subcenter, meta->pds2.sect4.genProcess, meta->pds2.sect4.genID, meta->pds2.prodType,
           meta->pds2.sect4.templat, meta->pds2.sect4.cat, meta->pds2.sect4.subcat, meta->pds2.sect4.foreSec,
           meta->pds2.sect4.Interval[0].lenTime,
           meta->pds2.sect4.fstSurfType, meta->pds2.sect4.fstSurfValue, meta->pds2.sect4.sndSurfValue,
           meta->pds2.sect4.probType, meta->pds2.sect4.lowerLimit.value, meta->pds2.sect4.lowerLimit.factor,
           meta->pds2.sect4.upperLimit.value, meta->pds2.sect4.upperLimit.factor);
   } else {
   printf ("PI2 %d %d,%d,%d,%d %d,%d,%d,%d,-1,%ld %d,%f,%f %d,-1,-1,-1,-1\n", meta->GribVersion,
           meta->center, meta->subcenter, meta->pds2.sect4.genProcess, meta->pds2.sect4.genID, meta->pds2.prodType,
           meta->pds2.sect4.templat, meta->pds2.sect4.cat, meta->pds2.sect4.subcat, (sInt4) 0,
           meta->pds2.sect4.fstSurfType, meta->pds2.sect4.fstSurfValue, meta->pds2.sect4.sndSurfValue,
           meta->pds2.sect4.probType);
   }
*/
#endif
   if ((elem->center != MISSING_2) && (elem->center != meta->center))
      return 0;
   if ((elem->subcenter != MISSING_2) && (elem->subcenter != meta->subcenter))
      return 0;
   if ((elem->version != 0) && (elem->version != meta->GribVersion))
      return 0;
   /* Those are all the current checks for non-GRIB2 data. */
   if (meta->GribVersion != 2)
      return 1;

   if ((elem->genProcess != MISSING_1) && (elem->genProcess != meta->pds2.sect4.genProcess))
      return 0;
   if ((elem->genID != MISSING_1) && (elem->genID != meta->pds2.sect4.genID))
      return 0;
   if ((elem->prodType != MISSING_1) &&
       (elem->prodType != meta->pds2.prodType))
      return 0;
   if ((elem->templat != MISSING_2) &&
       (elem->templat != meta->pds2.sect4.templat))
      return 0;
   if ((elem->cat != MISSING_1) && (elem->cat != meta->pds2.sect4.cat))
      return 0;
   if ((elem->subcat != MISSING_1) &&
       (elem->subcat != meta->pds2.sect4.subcat))
      return 0;
   if (((meta->pds2.sect4.templat == 8) ||
        (meta->pds2.sect4.templat == 9)) &&
       (meta->pds2.sect4.numInterval == 1)) {
      if ((elem->lenTime != 0) &&
          (elem->lenTime != meta->pds2.sect4.Interval[0].lenTime)) {
         return 0;
      }
      if ((elem->foreSec != -1) &&
          (elem->foreSec != meta->pds2.sect4.foreSec)) {
         return 0;
      }
   }

   if (elem->surfType != MISSING_1) {
      if (elem->surfType != meta->pds2.sect4.fstSurfType)
         return 0;
      if (elem->value != meta->pds2.sect4.fstSurfValue)
         return 0;
      if (elem->sndValue != meta->pds2.sect4.sndSurfValue)
         return 0;
   }

   if (meta->pds2.sect4.templat == 9) {
      if (elem->probType != meta->pds2.sect4.probType) {
         return 0;
      }

      if ((elem->lowerVal != meta->pds2.sect4.lowerLimit.value) ||
          (elem->lowerFact != meta->pds2.sect4.lowerLimit.factor)) {
         return 0;
      }
      if ((elem->upperVal != meta->pds2.sect4.upperLimit.value) ||
          (elem->upperFact != meta->pds2.sect4.upperLimit.factor)) {
         return 0;
      }
   }
   return 1;
}
#endif

#ifndef DP_ONLY
uChar genNdfdEnum_fromMeta (const grib_MetaData *meta) {
   uShort2 i;           /* Used to help find the ndfdEnum value. */
   sInt4 lenTime;
   sInt4 lcl_lenTime;
   double foreSec;
   double lcl_foreSec;
   uChar probType;
   sInt4 lowerVal;
   sChar lowerFact;
   sInt4 upperVal;
   sChar upperFact;

   if (meta->GribVersion != 2)
      return NDFD_UNDEF;

   if (((meta->pds2.sect4.templat == 8) ||
        (meta->pds2.sect4.templat == 9)) &&
       (meta->pds2.sect4.numInterval == 1)) {
      lenTime = meta->pds2.sect4.Interval[0].lenTime;
      foreSec = meta->pds2.sect4.foreSec;
   } else {
      lenTime = 0;
      foreSec = -1;
   }
   if ((meta->pds2.sect4.templat == 9)) {
      probType = meta->pds2.sect4.probType;
      lowerVal = meta->pds2.sect4.lowerLimit.value;
      lowerFact = meta->pds2.sect4.lowerLimit.factor;
      upperVal = meta->pds2.sect4.upperLimit.value;
      upperFact = meta->pds2.sect4.upperLimit.factor;
   } else {
      probType = 0;
      lowerVal = -1;
      lowerFact = -1;
      upperVal = -1;
      upperFact = -1;
   }

   /* Determine the ndfdEnum for this element. Note, ndfdEnum is already
    * initialized to NDFD_UNDEF. */
   for (i = 0; i < NdfdElementsLen; i++) {
      if (NdfdElements[i].foreSec == -1) {
         lcl_foreSec = -1;
      } else {
         lcl_foreSec = foreSec;
      }
      if (NdfdElements[i].lenTime == 0) {
         lcl_lenTime = 0;
      } else {
         lcl_lenTime = lenTime;
      }
      if ((NdfdElements[i].version == meta->GribVersion) &&
          (NdfdElements[i].center == meta->center) &&
          (NdfdElements[i].subcenter == meta->subcenter) &&
          (NdfdElements[i].genProcess == meta->pds2.sect4.genProcess) &&
          (NdfdElements[i].genID == meta->pds2.sect4.genID) &&
          (NdfdElements[i].prodType == meta->pds2.prodType) &&
          (NdfdElements[i].templat == meta->pds2.sect4.templat) &&
          (NdfdElements[i].cat == meta->pds2.sect4.cat) &&
          (NdfdElements[i].subcat == meta->pds2.sect4.subcat) &&
          (NdfdElements[i].lenTime == lcl_lenTime) && (NdfdElements[i].foreSec == lcl_foreSec) &&
          (NdfdElements[i].surfType == meta->pds2.sect4.fstSurfType) &&
          (NdfdElements[i].value == meta->pds2.sect4.fstSurfValue) &&
          (NdfdElements[i].sndValue == meta->pds2.sect4.sndSurfValue) &&
          (NdfdElements[i].probType == probType) &&
          (NdfdElements[i].lowerVal == lowerVal) && (NdfdElements[i].lowerFact == lowerFact) &&
          (NdfdElements[i].upperVal == upperVal) && (NdfdElements[i].upperFact == upperFact)) {
         return NdfdElements[i].ndfdEnum;
      }
   }
#ifdef DEBUG
   printf ("PI3 %d %d,%d,%d,%d %d,%d,%d,%d,-1,%ld %d,%f,%f %d,-1,-1,-1,-1\n", meta->GribVersion,
           meta->center, meta->subcenter, meta->pds2.sect4.genProcess, meta->pds2.sect4.genID, meta->pds2.prodType,
           meta->pds2.sect4.templat, meta->pds2.sect4.cat, meta->pds2.sect4.subcat, lenTime,
           meta->pds2.sect4.fstSurfType, meta->pds2.sect4.fstSurfValue, meta->pds2.sect4.sndSurfValue,
           meta->pds2.sect4.probType);
#endif
   return NDFD_UNDEF;
}
#endif

/*****************************************************************************
 * setGenElem() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Set an element descriptor based on what is in the meta data from the
 * current grid.
 *
 * ARGUMENTS
 * elem = The element description structure to set. (Output)
 * meta = The meta data associated with the current grid. (Input)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static void setGenElem (genElemDescript * elem, const grib_MetaData *meta)
{
   elem->center = meta->center;
   elem->subcenter = meta->subcenter;
   elem->version = meta->GribVersion;
   elem->ndfdEnum = genNdfdEnum_fromMeta (meta);
   /* Those are all the current variables to set for non-GRIB2 data. */
   if (meta->GribVersion != 2)
      return;

   elem->genProcess = meta->pds2.sect4.genProcess;
   elem->genID = meta->pds2.sect4.genID;
   elem->prodType = meta->pds2.prodType;
   elem->templat = meta->pds2.sect4.templat;
   elem->cat = meta->pds2.sect4.cat;
   elem->subcat = meta->pds2.sect4.subcat;
   if (((meta->pds2.sect4.templat == 8) ||
        (meta->pds2.sect4.templat == 9)) &&
       (meta->pds2.sect4.numInterval == 1)) {
      elem->lenTime = meta->pds2.sect4.Interval[0].lenTime;
      if (NdfdElements[elem->ndfdEnum].foreSec != -1) {
         elem->foreSec = meta->pds2.sect4.foreSec;
      } else {
         elem->foreSec = -1;
      }
   } else {
      elem->lenTime = 0;
      elem->foreSec = -1;
   }
   if ((meta->pds2.sect4.templat == 9)) {
      elem->probType = meta->pds2.sect4.probType;
      elem->lowerVal = meta->pds2.sect4.lowerLimit.value;
      elem->lowerFact = meta->pds2.sect4.lowerLimit.factor;
      elem->upperVal = meta->pds2.sect4.upperLimit.value;
      elem->upperFact = meta->pds2.sect4.upperLimit.factor;
   } else {
      elem->probType = 0;
      elem->lowerVal = -1;
      elem->lowerFact = -1;
      elem->upperVal = -1;
      elem->upperFact = -1;
   }
   elem->surfType = meta->pds2.sect4.fstSurfType;
   elem->value = meta->pds2.sect4.fstSurfValue;
   elem->sndValue = meta->pds2.sect4.sndSurfValue;
}
#endif

/*****************************************************************************
 * getValAtPnt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a grid and a point, determine the value at that point.
 *
 * ARGUMENTS
 * gribDataLen = Length of gribData. (Input)
 *    gribData = The current grid. (Input)
 *         map = The current map transformation (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y (Input)
 *        pntX = The point in question (X component) (Input)
 *        pntY = The point in question (Y component) (Input)
 *          Nx = Number of X values in the grid (Input)
 *          Ny = Number of Y values in the grid (Input)
 *      f_miss = Missing management: 0 none, 1 pri, 2 pri & sec. (Input)
 *     missPri = Primary missing value. (Input)
 *     missSec = Secondary missing value. (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *         ans = The desired value.
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static void getValAtPnt (sInt4 gribDataLen, const double *gribData,
                         myMaparam *map, sChar f_pntType, double pntX,
                         double pntY, sInt4 Nx, sInt4 Ny, uChar f_miss,
                         double missPri, double missSec, uChar f_interp,
                         double *ans, sChar f_avgInterp)
{
   double newX, newY;   /* The location of point on the grid. */
   sInt4 x1, y1;        /* f_interp=0, The nearest grid point, Otherwise
                         * corners of bounding box around point */
   sInt4 x2, y2;        /* Corners of bounding box around point. */
   sInt4 row;           /* The index into gribData for a given x,y pair *
                         * using scan-mode = 0100 = GRIB2BIT_2 */
   double d11, d12, d21, d22; /* values of bounding box corners. */
   double d_temp1, d_temp2; /* Temp storage during interpolation. */

   myAssert (Nx * Ny >= gribDataLen);

   /* Get point on the grid. */
   myAssert ((f_pntType == 0) || (f_pntType == 1));
   if (f_pntType == 0) {
      myCll2xy (map, pntY, pntX, &newX, &newY);
   } else {
      newX = pntX;
      newY = pntY;
   }

   /* Find value at the nearest grid cell. */
   if (!f_interp) {
      /* make sure x1 and y1 are in bounds. */
      x1 = (sInt4) (newX + .5);
      if ((x1 < 1) || (x1 > Nx)) {
         *ans = missPri;
         return;
      }
      y1 = (sInt4) (newY + .5);
      if ((y1 < 1) || (y1 > Ny)) {
         *ans = missPri;
         return;
      }

      /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
      row = (x1 - 1) + (y1 - 1) * Nx;
      myAssert ((row >= 0) && (row < gribDataLen));
      *ans = gribData[row];
      return;
   }

   /* Perform bi-linear interpolation. */
   x1 = (sInt4) newX;
   x2 = x1 + 1;
   y1 = (sInt4) newY;
   y2 = y1 + 1;
   if ((x1 < 1) || (x2 > Nx) || (y1 < 1) || (y2 > Ny)) {
      if (map->f_latlon) {
         /* Find out if we can do a border interpolation. */
         *ans = BiLinearBorder (gribData, map, newX, newY, Nx, Ny, f_miss,
                                missPri, missSec, f_avgInterp);
      } else {
         *ans = missPri;
      }
      return;
   }

   /* Get the first (1,1) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   row = (x1 - 1) + (y1 - 1) * Nx;
   myAssert ((row >= 0) && (row < gribDataLen));
   d11 = gribData[row];
   if ((d11 == missPri) || ((f_miss == 2) && (d11 == missSec))) {
      *ans = missPri;
      return;
   }

   /* Get the second (1,2) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   row = (x1 - 1) + (y2 - 1) * Nx;
   myAssert ((row >= 0) && (row < gribDataLen));
   d12 = gribData[row];
   if ((d12 == missPri) || ((f_miss == 2) && (d12 == missSec))) {
      *ans = missPri;
      return;
   }

   /* Get the third (2,1) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   row = (x2 - 1) + (y1 - 1) * Nx;
   myAssert ((row >= 0) && (row < gribDataLen));
   d21 = gribData[row];
   if ((d21 == missPri) || ((f_miss == 2) && (d21 == missSec))) {
      *ans = missPri;
      return;
   }

   /* Get the fourth (2,2) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   row = (x2 - 1) + (y2 - 1) * Nx;
   myAssert ((row >= 0) && (row < gribDataLen));
   d22 = gribData[row];
   if ((d22 == missPri) || ((f_miss == 2) && (d22 == missSec))) {
      *ans = missPri;
      return;
   }

   /* Do Bi-linear interpolation to get value. */
   /* Corrected 1/24/2007 due to email from jeff.sharkey */
   /* Was (d11 - d12) and (d21 - d22), but d12 is x1,y2 and d21 is x2,y1.
    * d_temp1 is holding y1 constant so should be dealing with d?1 */
   d_temp1 = d11 + (newX - x1) * (d11 - d21) / (x1 - x2);
   d_temp2 = d12 + (newX - x1) * (d12 - d22) / (x1 - x2);
   *ans = (d_temp1 + (newY - y1) * (d_temp1 - d_temp2) / (y1 - y2));
}
#endif

/*****************************************************************************
 * getCubeValAtPnt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a cube file and a point, determine the value at that point.
 *
 * ARGUMENTS
 *        data = The opened data cube to read from. (Input)
 *  dataOffset = The starting offset in the data cube file. (Input)
 *        scan = The scan mode of the data cube file (0 or 64) (Input)
 * f_bigEndian = Endian'ness of the data cube file (1=Big, 0=Lit) (Input)
 *         map = The current map transformation (Input)
 *        pntX = The point in question (in grid cell space) (Input)
 *        pntY = The point in question (in grid cell space) (Input)
 *          Nx = Number of X values in the grid (Input)
 *          Ny = Number of Y values in the grid (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *         ans = The desired value. (Output)
 *
 * RETURNS: void
 *
 *  2/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 * Doesn't handle border interpolation exception for lat/lon grids.
 *****************************************************************************
 */
static void getCubeValAtPnt (FILE *data, sInt4 dataOffset, uChar scan,
                             uChar f_bigEndian, myMaparam *map, double pntX,
                             double pntY, sInt4 Nx, sInt4 Ny, uChar f_interp,
                             float *ans)
{
   sInt4 offset;        /* Where the current data is in the data file. */
   sInt4 x1, y1;        /* f_interp=0, The nearest grid point, Otherwise
                         * corners of bounding box around point */
   sInt4 x2, y2;        /* Corners of bounding box around point. */
   float missPri = 9999; /* Missing value to use with cube's is always 9999 */
   float d11, d12, d21, d22; /* values of bounding box corners. */
   double d_temp1, d_temp2; /* Temp storage during interpolation. */

   /* Find value at the nearest grid cell. */
   if (!f_interp) {
      /* make sure newX and newY are in bounds. */
      x1 = (sInt4) (pntX + .5);
      if ((x1 < 1) || (x1 > Nx)) {
         *ans = missPri;
         return;
      }
      y1 = (sInt4) (pntY + .5);
      if ((y1 < 1) || (y1 > Ny)) {
         *ans = missPri;
         return;
      }

      if (scan == 0) {
         offset = dataOffset + (((x1 - 1) + ((Ny - 1) - (y1 - 1)) * Nx) *
                                sizeof (float));
      } else {
         offset = dataOffset + ((x1 - 1) + (y1 - 1) * Nx) * sizeof (float);
      }
      fseek (data, offset, SEEK_SET);
      if (f_bigEndian) {
         FREAD_BIG (ans, sizeof (float), 1, data);
      } else {
         FREAD_LIT (ans, sizeof (float), 1, data);
      }
      return;
   }

   /* Perform bi-linear interpolation. */
   x1 = (sInt4) pntX;
   x2 = x1 + 1;
   y1 = (sInt4) pntY;
   y2 = y1 + 1;
   if ((x1 < 1) || (x2 > Nx) || (y1 < 1) || (y2 > Ny)) {
      *ans = missPri;
      myAssert (!map->f_latlon);
      /* For latlon grids, would try BiLinearBorder here.  We assume that
       * since this is a cube routine, that the cubes would not contain
       * lat/lon grids where the deltaX is such that 1 cell from the right
       * edge of the grid is on the left edge of the grid.  If they did,
       * then_ handling the probes that are outside the grid wasn't worth the
       * effort, and one could just return missing. Technically,
       * BiLinearBorder could be rewritten so that it doesn't need a map, as
       * we already have x1, y1, Nx, Ny, and from those could calcualate the
       * appropriate points. */
      return;
   }

   /* Get the (1,1) corner value. */
   if (scan == 0) {
      offset = dataOffset + (((x1 - 1) + ((Ny - 1) - (y1 - 1)) * Nx) *
                             sizeof (float));
   } else {
      offset = dataOffset + ((x1 - 1) + (y1 - 1) * Nx) * sizeof (float);
   }
   fseek (data, offset, SEEK_SET);
   if (f_bigEndian) {
      FREAD_BIG (&d11, sizeof (float), 1, data);
   } else {
      FREAD_LIT (&d11, sizeof (float), 1, data);
   }
   if (d11 == missPri) {
      *ans = missPri;
      return;
   }

   /* Get the (1,2) corner value. */
   if (scan == 0) {
      offset = dataOffset + (((x1 - 1) + ((Ny - 1) - (y2 - 1)) * Nx) *
                             sizeof (float));
   } else {
      offset = dataOffset + ((x1 - 1) + (y2 - 1) * Nx) * sizeof (float);
   }
   fseek (data, offset, SEEK_SET);
   if (f_bigEndian) {
      FREAD_BIG (&d12, sizeof (float), 1, data);
   } else {
      FREAD_LIT (&d12, sizeof (float), 1, data);
   }
   if (d12 == missPri) {
      *ans = missPri;
      return;
   }

   /* Get the (2,1) corner value. */
   if (scan == 0) {
      offset = dataOffset + (((x2 - 1) + ((Ny - 1) - (y1 - 1)) * Nx) *
                             sizeof (float));
   } else {
      offset = dataOffset + ((x2 - 1) + (y1 - 1) * Nx) * sizeof (float);
   }
   fseek (data, offset, SEEK_SET);
   if (f_bigEndian) {
      FREAD_BIG (&d21, sizeof (float), 1, data);
   } else {
      FREAD_LIT (&d21, sizeof (float), 1, data);
   }
   if (d21 == missPri) {
      *ans = missPri;
      return;
   }

   /* Get the (2,2) corner value. */
   if (scan == 0) {
      offset = dataOffset + (((x2 - 1) + ((Ny - 1) - (y2 - 1)) * Nx) *
                             sizeof (float));
   } else {
      offset = dataOffset + ((x2 - 1) + (y2 - 1) * Nx) * sizeof (float);
   }
   fseek (data, offset, SEEK_SET);
   if (f_bigEndian) {
      FREAD_BIG (&d22, sizeof (float), 1, data);
   } else {
      FREAD_LIT (&d22, sizeof (float), 1, data);
   }
   if (d21 == missPri) {
      *ans = missPri;
      return;
   }

   /* Do Bi-linear interpolation to get value. */
   /* Corrected 1/24/2007 due to email from jeff.sharkey */
   /* Was (d11 - d12) and (d21 - d22), but d12 is x1,y2 and d21 is x2,y1.
    * d_temp1 is holding y1 constant so should be dealing with d?1 */
   d_temp1 = d11 + (pntX - x1) * (d11 - d21) / (x1 - x2);
   d_temp2 = d12 + (pntX - x1) * (d12 - d22) / (x1 - x2);
   *ans = (d_temp1 + (pntY - y1) * (d_temp1 - d_temp2) / (y1 - y2));
}

/*****************************************************************************
 * getWxString() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a value at a point, look up the weather information in the weather
 * table, and store it in str.
 *
 * ARGUMENTS
 *       str = Where to store the answer (Output)
 *   wxIndex = The index into the wx table (as a double) (Input)
 *        wx = The weather table associated with this grid (Input)
 * f_wxParse = 0 => store ugly string, 1 => store English Translation,
 *             2 => store -SimpleWx code. (Input)
 *
 * RETURNS: void
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static void getWxString (char **str, sInt4 wxIndex, const sect2_WxType *wx,
                         sChar f_WxParse)
{
   int j;               /* loop counter over the weather keys. */

   if ((wxIndex < 0) || (wxIndex >= (sInt4) wx->dataLen)) {
      mallocSprintf (str, "%ld", wxIndex);
      return;
   }
   /* Print out the weather string according to f_WxParse. */
   switch (f_WxParse) {
      case 0:
         *str = (char *) malloc (strlen (wx->data[wxIndex]) + 1);
         strcpy ((*str), wx->data[wxIndex]);
         return;
      case 1:
         *str = NULL;
         for (j = 0; j < NUM_UGLY_WORD; j++) {
            if (wx->ugly[wxIndex].english[j] == NULL) {
               if (j == 0) {
                  reallocSprintf (str, "No Weather");
               }
               return;
            }
            if (j != 0) {
               if (j == wx->ugly[wxIndex].numValid - 1) {
                  reallocSprintf (str, " and ");
               } else {
                  reallocSprintf (str, ", ");
               }
            }
            reallocSprintf (str, "%s", wx->ugly[wxIndex].english[j]);
         }
         return;
      case 2:
         mallocSprintf (str, "%d", wx->ugly[wxIndex].SimpleCode);
         return;
   }
}

static void getWWAString (char **str, sInt4 wwaIndex, const sect2_HazardType *haz,
                          sChar f_WxParse)
{
   int j;               /* loop counter over the weather keys. */

   if ((wwaIndex < 0) || (wwaIndex >= (sInt4) haz->dataLen)) {
      mallocSprintf (str, "%ld", wwaIndex);
      return;
   }
   /* Print out the weather string according to f_WxParse. */
   switch (f_WxParse) {
      case 0:
         *str = (char *) malloc (strlen (haz->data[wwaIndex]) + 1);
         strcpy ((*str), haz->data[wwaIndex]);
         return;
      case 1:
         *str = NULL;
         for (j = 0; j < NUM_HAZARD_WORD; j++) {
            if (haz->haz[wwaIndex].english[j] == NULL) {
               if (j == 0) {
                  reallocSprintf (str, "No Weather");
               }
               return;
            }
            if (j != 0) {
               if (j == haz->haz[wwaIndex].numValid - 1) {
                  reallocSprintf (str, " and ");
               } else {
                  reallocSprintf (str, ", ");
               }
            }
            reallocSprintf (str, "%s", haz->haz[wwaIndex].english[j]);
         }
         return;
      case 2:
         mallocSprintf (str, "%d", haz->haz[wwaIndex].SimpleCode);
         return;
   }
}
#endif

/*****************************************************************************
 * genFillValue() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a grid and a set of points, determine the values for those points.
 *
 * ARGUMENTS
 * gribDataLen = Length of gribData. (Input)
 *    gribData = The current grid. (Input)
 *      grdAtt = Attributes about the current grid (ie missing values). (In)
 *         map = Map projection to use to convert from lat/lon to x/y (In)
 *          Nx = Number of X values in the grid (Input)
 *          Ny = Number of Y values in the grid (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *          wx = The weather table associated with this grid (Input)
 *   f_wxParse = 0 => store ugly string, 1 => store English Translation,
 *               2 => store -SimpleWx code. (Input)
 *     numPnts = Number of points (Input)
 *        pnts = The points to probe. (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y (Input)
 *       value = The values at the points (already alloced to numPnts. (Out)
 *
 * RETURNS: void
 *
 *  1/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static void genFillValue (sInt4 gribDataLen, const double *gribData,
                          const gridAttribType *grdAtt, myMaparam *map,
                          sInt4 Nx, sInt4 Ny, uChar f_interp,
                          const sect2_WxType *wx, const sect2_HazardType *haz,
                          sChar f_WxParse, size_t numPnts,
                          const Point * pnts, sChar f_pntType,
                          genValueType * value, sChar f_avgInterp)
{
   double missing;      /* Missing value to use. */
   size_t i;            /* loop counter over number of points. */
   double ans;          /* The grid value at the current point. */

   /* getValAtPnt does not allow f_pntType == 2 */
   myAssert (f_pntType != 2);

   /* Figure out a missing value, to assist with interpolation. */
   if (grdAtt->f_miss == 0) {
      missing = 9999;
      if (grdAtt->f_maxmin) {
         if ((missing <= grdAtt->max) && (missing >= grdAtt->min)) {
            missing = grdAtt->max + 1;
         }
      }
   } else {
      missing = grdAtt->missPri;
   }

   /* Need an adjustment for QPF (and RTMA_PRECIP) and IceAccum in the 
    * case of f_unit = 2.  The reason is that Paul defined metric to be 
    * cm instead of kg/m^2.  The code by default should have converted
    * from GRIB units to kg/m^2 (ie did nothing in this case).  To convert
    * kg/m^2 to cm, we multipy with a MQ = 1/10. */            
    
   /* Need an adjustment for SNOW in the case of f_unit = 2.  The reason is 
    * that Paul defined metric to be cm instead of m.  The code by default 
    * should have converted from GRIB units to m (ie did nothing in this case).
    * To convert m to cm, we multipy with a MS = 100. */ 
    
   /* Note the difference between MQ and MS is a factor of 1000 which is the
    * density of water (1000 kg/m^3). */                    

   /* Loop over the points. */
   for (i = 0; i < numPnts; i++) {
      getValAtPnt (gribDataLen, gribData, map, f_pntType, pnts[i].X,
                   pnts[i].Y, Nx, Ny, grdAtt->f_miss, missing,
                   grdAtt->missSec, f_interp, &ans, f_avgInterp);
      if (ans == missing) {
         value[i].valueType = 2;
         value[i].data = ans;
         if ((wx == NULL) && (haz == NULL)) {
            value[i].str = NULL;
         } else {
            mallocSprintf (&(value[i].str), "%.0f", ans);
         }
      } else {
         if ((wx == NULL) && (haz == NULL)) {
            value[i].valueType = 0;
            value[i].data = ans;
            value[i].str = NULL;
         } else if (haz == NULL) {
            value[i].valueType = 1;
            value[i].data = 0;
            getWxString (&(value[i].str), (sInt4) ans, wx, f_WxParse);
         } else {
            value[i].valueType = 1;
            value[i].data = 0;
            getWWAString (&(value[i].str), (sInt4) ans, haz, f_WxParse);
         }
      }
   }
}
#endif

/*****************************************************************************
 * InverseComputUnit() -- (See metaname.c::ComputeUnit()
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   NDFD typically stores the units in the cube using f_unit 1.  However 
 * for DWML, there was a desire to be able to change the unit based on the
 * f_unit.  So we needed a "InverseComputeUnit" method to potentially go
 * from f_unit = 1 to f_unit = 0 or f_unit = 2.             
 *            
 *   This code assumes that the data is in f_unit 1, and reverses it.       
 *    
 *   This code sets m, and b for equation y = mx + b, where x is in the unit
 * of f_unit 1 (english), and y is the one specified by f_unit.  The default
 * is m = 1, b = 0.
 *    
 * Currently:
 *   For f_unit = 1 (english) we return Fahrenheit, knots, and inches for
 * temperature, wind speed, and amount of snow or rain.  The original units
 * are Kelvin, m/s, kg/m**2.
 *   For f_unit = 2 (metric) we return Celsius instead of Kelvin.
 *   For f_unit = 0 (SI) we return Kelvin instead of  
 *
 * ARGUMENTS
 *  convert = The enumerated type describing the type of conversion. (Input)
 * origName = Original unit name (needed for log10 option) (Input)
 *   f_unit = What type of unit to return (see above) (Input).
 *    unitM = M in equation y = m x + b (Output)
 *    unitB = B in equation y = m x + b (Output)
 *     name = Where to store the result (assumed already allocated to at
 *           least 15 bytes) (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   0 if we set M and B, 1 if we used defaults.
 *
 * HISTORY
 *   10/2011 Arthur Taylor (MDL/RSIS): Re-Created.
 *
 * NOTES
 *****************************************************************************
 */
static int InverseComputeUnit (int convert, const char *origName, sChar f_unit, 
                               double *unitM, double *unitB, char *name)
{
   switch (convert) {
      case UC_NONE:
         break;
      case UC_K2F:
         if (f_unit == 2) {
            /* F to C here. */
            strcpy (name, "[C]");         
            *unitM = 5. / 9.;
            *unitB = - 160 / 9.;
            return 0;
         } else if (f_unit == 0) {
            /* F to K here. */
            strcpy (name, "[K]");         
            *unitM = 5. / 9.;
            *unitB = 273.15 - (160 / 9.);
            return 0;
         }
         break;
      case UC_InchWater: /* Convert from kg/(m^2) to inches water. */
         if (f_unit != 1) {
            strcpy (name, "[kg/m^2]");
            /* 
             * kg/m**2 / density of water (1000 kg/m**3)
             * 1/1000 m * 1/2.54 in/cm * 100 cm/m = 1/25.4 inches
             * Reversing we multiply by 25.4 to get kg/m^2             
             */
            *unitM = 25.4;
            *unitB = 0;
            return 0;
         }
         break;
      case UC_M2Feet:  /* Convert from meters to feet. */
         if (f_unit != 1) {
            /* 1 (m) * (100cm/m) * (inch/2.54cm) * (ft/12inch) = X (ft) */
            /* ft to m here. */
            strcpy (name, "[m]");
            *unitM = .3048;
            *unitB = 0;
            return 0;
         }
         break;
      case UC_M2Inch:  /* Convert from meters to inches. */
         if (f_unit != 1) {
            /* inch to m here. */
            strcpy (name, "[m]");
            *unitM = .0254; /* inch / m */
            *unitB = 0;
            return 0;
         }
         break;
      case UC_M2StatuteMile: /* Convert from meters to statute miles. */
         if (f_unit != 1) {
            /* mile to m here. */
            strcpy (name, "[m]");
            *unitM = 1609.344; /* mile / m */
            *unitB = 0;
            return 0;
         }
         break;
         /* NCEP goes with a convention of 1 nm = 1853.248 m.
          * http://www.sizes.com/units/mile_USnautical.htm Shows that on
          * 7/1/1954 US Department of Commerce switched to 1 nm = 1852 m
          * (International standard.) */
      case UC_MS2Knots: /* Convert from m/s to knots. */
         if (f_unit != 1) {
            /* knots to m/s */
            strcpy (name, "[m/s]");
            *unitM = 1852. / 3600.; /* knot / m s**-1 */
            *unitB = 0;
            return 0;
         }
         break;
      case UC_UVIndex: /* multiply by Watts/ m**2 by 40 for the UV index. */
         if (f_unit != 1) {
            strcpy (name, "[Watts/m^2]");
            *unitM = 1/40.;
            *unitB = 0;
            return 0;
         }
         break;
/*
      case UC_LOG10:   * convert from log10 (x) to x *
         if ((f_unit == 1) || (f_unit == 2)) {
            origName[strlen (origName) - 2] = '\0';
            if (strlen (origName) > 21)
               origName[21] = '\0';
            sprintf (name, "[%s]", origName + 7);
            *unitM = -10; 
                       * M = -10 => take 10^(x) *
            *unitB = 0;
            return 0;
         }
         break;
*/
   }
   strncpy (name, origName, 14);
   name[14] = '\0';
   *unitM = 1;
   *unitB = 0;
   return 1;
}

/*****************************************************************************
 * genCubeFillValue() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Given a cube file and a set of points, determine the values for those
 * points.
 *
 * ARGUMENTS
 *        data = The opened data cube to read from. (Input)
 *  dataOffset = The starting offset in the data cube file. (Input)
 *        scan = The scan mode of the data cube file (0 or 64) (Input)
 * f_bigEndian = Endian'ness of the data cube file (1=Big, 0=Lit) (Input)
 *         map = The current map transformation (Input)
 *     numPnts = Number of points (Input)
 *        pnts = The points to probe (at this point in grid cell units). (In)
 *          Nx = Number of X values in the grid (Input)
 *          Ny = Number of Y values in the grid (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *    elemEnum = The NDFD element enumeration for this grid (Input)
 *    numTable = Number of strings in the table (Input)
 *       table = Table of strings associated with this PDS (Input)
 *   f_wxParse = 0 => store ugly string, 1 => store English Translation,
 *               2 => store -SimpleWx code. (Input)
 * f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *       value = The values at the points (already alloced to numPnts. (Out)
 *
 * RETURNS: void
 *
 *  2/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void genCubeFillValue (FILE *data, sInt4 dataOffset, uChar scan,
                              uChar f_bigEndian, myMaparam *map,
                              size_t numPnts, const Point * pnts, sInt4 Nx,
                              sInt4 Ny, uChar f_interp, uChar elemEnum,
                              uShort2 numTable, char **table,
                              sChar f_WxParse, sChar f_SimpleVer, 
                              sChar f_SimpleWWA, char *unitReadFromBufr, 
                              sChar f_unit, char **convertedUnit,
                              genValueType *value)
{
   size_t i;            /* loop counter over number of points. */
   float ans;           /* The current cell value. */
   uShort2 wxIndex;     /* 'value' cast to an integer for table lookup. */
   size_t j;            /* Counter used to print "english" weather. */
   UglyStringType ugly; /* Used to 'translate' the weather keys. */
   HazardStringType haz; /* Used to 'translate' the hazard keys. */
   double unitM, unitB;

   myAssert ((scan == 0) || (scan == 64));
   myAssert (sizeof (float) == 4);
/*
   myAssert (((elemEnum == NDFD_WX) && (numTable != 0)) ||
             ((elemEnum != NDFD_WX) && (numTable == 0)));
*/

   /* Check if the user is trying to bi-linear interpolate weather. */
   if ((elemEnum == NDFD_WX) || (elemEnum == NDFD_WWA)) {
      f_interp = 0;
   }

   /* Determine the units.  Call metaname.c :: InverseCompute() */
   if (f_unit == 1) {
      *convertedUnit = (char *) malloc (strlen (unitReadFromBufr) + 1);
      unitM = 1;
      unitB = 0;
      strcpy (*convertedUnit, unitReadFromBufr);
   }  else {
      *convertedUnit = (char *) malloc (15 + 1);
      /* Would prefer something better than a big if/else statement here. */      
      if ((elemEnum == NDFD_MAX) || (elemEnum == NDFD_MIN) || 
          (elemEnum == NDFD_TEMP) || (elemEnum == NDFD_TD) || 
          (elemEnum == NDFD_AT) || (elemEnum == RTMA_TD) || 
          (elemEnum == RTMA_TEMP)) {
         /* Reverse the K to F conversion... */ 
         InverseComputeUnit (UC_K2F, unitReadFromBufr, f_unit, &unitM, &unitB,
                             *convertedUnit);
      } else if ((elemEnum == RTMA_UTD || elemEnum == RTMA_UTEMP) ||
                 (elemEnum == RTMA_UWSPD)) {
         /* No conversion took place. */
         InverseComputeUnit (UC_NONE, unitReadFromBufr, f_unit, &unitM, &unitB,
                             *convertedUnit);
      } else if ((elemEnum == NDFD_WS) || (elemEnum == NDFD_WG) || 
                 (elemEnum == RTMA_WSPD)) {
         /* Reverse m/s to knots... */ 
         InverseComputeUnit (UC_MS2Knots, unitReadFromBufr, f_unit, &unitM, 
                             &unitB, *convertedUnit);
      } else if ((elemEnum == NDFD_QPF) || (elemEnum == RTMA_PRECIPA) || 
                 (elemEnum == NDFD_ICEACC)) {
         if (f_unit == 0) {
            /* Reverse kg/m^2 to inches... */ 
            InverseComputeUnit (UC_InchWater, unitReadFromBufr, f_unit, &unitM, 
                                &unitB, *convertedUnit);
         } else {
            /* Have them in inches, want them in cm */
            strcpy (*convertedUnit, "[cm]");
            unitM = 2.54;
            unitB = 0;
         } 
      } else if (elemEnum == NDFD_SNOW) {
         if (f_unit == 0) {
            /* Reverse m to inch... */ 
            InverseComputeUnit (UC_M2Inch, unitReadFromBufr, f_unit, &unitM, 
                                &unitB, *convertedUnit);
         } else {
            /* Have them in inches, want them in cm */
            strcpy (*convertedUnit, "[cm]");
            unitM = 2.54;
            unitB = 0;
         }                             
      } else if ((elemEnum == NDFD_WH)) {
         /* Reverse m to feet... */ 
         InverseComputeUnit (UC_M2Feet, unitReadFromBufr, f_unit, &unitM, 
                             &unitB, *convertedUnit);
      } else {
         /* Unrecoginzed... Assume no conversion took place. */
         InverseComputeUnit (UC_NONE, unitReadFromBufr, f_unit, &unitM, &unitB,
                             *convertedUnit);
      }
   } 

   for (i = 0; i < numPnts; i++) {
      getCubeValAtPnt (data, dataOffset, scan, f_bigEndian, map, pnts[i].X,
                       pnts[i].Y, Nx, Ny, f_interp, &ans);

      /* 9999 is the missing value for data cubes */
      if (ans == 9999) {
         value[i].valueType = 2;
         value[i].data = ans;
         if ((elemEnum != NDFD_WX) && (elemEnum != NDFD_WWA)) {
            value[i].str = NULL;
         } else {
            value[i].str = (char *) malloc (4 + 1);
            strcpy (value[i].str, "9999");
         }
      } else {
         if ((elemEnum != NDFD_WX) && (elemEnum != NDFD_WWA)) {
            value[i].valueType = 0;
            value[i].data = unitM * ans + unitB;
            value[i].str = NULL;
         } else {
            wxIndex = (uShort2) ans;
            if ((numTable == 0) || (wxIndex >= numTable)) {
               value[i].valueType = 2;
               value[i].data = wxIndex;
               mallocSprintf (&(value[i].str), "%ld", wxIndex);
            } else {
               value[i].valueType = 1;
               value[i].data = 0;
               switch (f_WxParse) {
                  case 0:
                     value[i].str =
                           (char *) malloc (strlen (table[wxIndex]) + 1);
                     strcpy (value[i].str, table[wxIndex]);
                     break;
                  case 1:
                     if (elemEnum == NDFD_WX) {
                        ParseUglyString (&ugly, table[wxIndex], f_SimpleVer);
                        value[i].str = NULL;
                        for (j = 0; j < NUM_UGLY_WORD; j++) {
                           if (ugly.english[j] == NULL) {
                              if (j == 0) {
                                 reallocSprintf (&(value[i].str), "No Weather");
                              }
                              break;
                           }
                           if (j != 0) {
                              if (j + 1 == ugly.numValid) {
                                 reallocSprintf (&(value[i].str), " and ");
                              } else {
                                 reallocSprintf (&(value[i].str), ", ");
                              }
                           }
                           reallocSprintf (&(value[i].str), "%s",
                                           ugly.english[j]);
                        }
                        FreeUglyString (&ugly);
                     } else {
                        ParseHazardString (&haz, table[wxIndex], f_SimpleWWA);
                        value[i].str = NULL;
                        for (j = 0; j < NUM_HAZARD_WORD; j++) {
                           if (haz.english[j] == NULL) {
                              if (j == 0) {
                                 reallocSprintf (&(value[i].str), "No Hazard");
                              }
                              break;
                           }
                           if (j != 0) {
                              if (j + 1 == haz.numValid) {
                                 reallocSprintf (&(value[i].str), " and ");
                              } else {
                                 reallocSprintf (&(value[i].str), ", ");
                              }
                           }
                           reallocSprintf (&(value[i].str), "%s",
                                           haz.english[j]);
                        }
                        FreeHazardString (&haz);
                     }
                     break;
                  case 2:
                     if (elemEnum == NDFD_WX) {
                        ParseUglyString (&ugly, table[wxIndex], f_SimpleVer);
                        mallocSprintf (&(value[i].str), "%d", ugly.SimpleCode);
                        FreeUglyString (&ugly);
                     } else {
                        ParseHazardString (&haz, table[wxIndex], f_SimpleWWA);
                        mallocSprintf (&(value[i].str), "%d", haz.SimpleCode);
                        FreeHazardString (&haz);
                     }
                     break;
               }
            }
         }
      }
   }
}

/*****************************************************************************
 * genProbeGrib() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Probe a given GRIB file for all messages that match the element criteria,
 * and return the values of the given set of points inside the match
 * structure.
 *
 * ARGUMENTS
 *          fp = Opened GRIB file ready to be read. (Input)
 *                      POINT FILTERING INFO
 *     numPnts = Number of points (Input)
 *        pnts = The points to probe. (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y (Input)
 *                      ELEMENT FILTERING INFO
 *     numElem = Number of elements in element filter list. (Input)
 *        elem = Return only data found in this list.
 *               Use NDFD_MATCHALL, to get all the elements. (Input)
 *                      TIME FILTERING INFO
 *   f_valTime = 0 false, 1 f_validStartTime, 2 f_validEndTime,
 *               3 both f_validStartTime, and f_validEndTime (Input)
 *   startTime = first valid time that we are interested in. (Input)
 *     endTime = last valid time that we are interested in. (Input)
 *                      SPECIAL FLAGS
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *      f_unit = 0 -Unit n || 1 -Unit e || 2 -Unit m (Input)
 *    majEarth = Use this to override the majEarth (< 6000 ignored) (Input)
 *    minEarth = Use this to override the minEarth (< 6000 ignored) (Input)
 *   f_wxParse = 0 => ugly string, 1 => English Translation,
 *               2 => -SimpleWx code. (Input)
 * f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *                      OUTPUT
 *    numMatch = Number of matches found. (Output)
 *       match = Matches. (Output)
 *
 * RETURNS: int
 *   -1 = problems reading a GRIB message
 *   -2 = problems with the Grid Definition Section.
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#ifndef DP_ONLY
static int genProbeGrib (FILE *fp, size_t numPnts, const Point * pnts,
                         sChar f_pntType, size_t numElem,
                         const genElemDescript * elem, sChar f_valTime,
                         double startTime, double endTime, uChar f_interp,
                         sChar f_unit, double majEarth, double minEarth,
                         sChar f_WxParse, sChar f_SimpleVer, sChar f_SimpleWWA,
                         size_t *numMatch, genMatchType ** match,
                         sChar f_avgInterp)
{
   IS_dataType is;      /* Un-parsed meta data for this GRIB2 message. As
                         * well as some memory used by the unpacker. */
   grib_MetaData meta;  /* The meta structure for this GRIB2 message. */
   int subgNum;         /* Subgrid in the msg that we are interested in. */
   uInt4 gribDataLen;   /* Current length of gribData. */
   double *gribData;    /* Holds the grid retrieved from a GRIB2 message. */
   int c;               /* Determine if end of the file without fileLen. */
   sInt4 f_lstSubGrd;   /* 1 if we read the last subGrid in a message */
   LatLon lwlf;         /* ReadGrib2Record allows subgrids.  We want entire
                         * grid, so set the lat to -100. */
   LatLon uprt;         /* ReadGrib2Record allows subgrids.  We want entire
                         * grid, so set the lat to -100. */
   genMatchType *curMatch; /* The current match */
   size_t i;            /* Loop counter while searching for a matching elem */
   double validTime;    /* The current grid's valid time. */
   double refTime;      /* The current grid's ref time. */
   myMaparam map;       /* The current grid's map parameter. */
   char f_sector;       /* Enumerated Sector associated with this file */
   int k;               /* Loop counter. */
   char f_interest;     /* used to help determine if we've already found
                         * this match so we don't need to do it again. */
   int elemEnum;        /* The NDFD element enumeration for the read grid */

   /* getValAtPnt does not currently allow f_pntType == 2 */
   myAssert (f_pntType != 2);

   /* Initialize data and structures used when unpacking a message */
   IS_Init (&is);
   MetaInit (&meta);
   f_lstSubGrd = 1;
   subgNum = 0;
   lwlf.lat = -100;
   uprt.lat = -100;
   gribDataLen = 0;
   gribData = NULL;

   /* Start loop for all messages. */
   while ((c = fgetc (fp)) != EOF) {
      ungetc (c, fp);

      /* Improve this by reading the message into memory, parsing key peices
       * out of it, and returning them to the caller.  Then if caller still
       * interested call ReadGrib2Record to break up the message. */

      /* Read the GRIB message. */
      if (ReadGrib2Record (fp, f_unit, &gribData, &gribDataLen, &meta,
                           &is, subgNum, majEarth, minEarth, f_SimpleVer, f_SimpleWWA,
                           &f_lstSubGrd, &(lwlf), &(uprt)) != 0) {
         preErrSprintf ("ERROR: In call to ReadGrib2Record.\n");
         free (gribData);
         IS_Free (&is);
         MetaFree (&meta);
         return -1;
      }
      if (!f_lstSubGrd) {
         subgNum++;
      } else {
         subgNum = 0;
      }

      /* Check if we're interested in this data based on validTime. */
      if (meta.GribVersion == 2) {
         validTime = meta.pds2.sect4.validTime;
         refTime = meta.pds2.refTime;
      } else if (meta.GribVersion == 1) {
         validTime = meta.pds1.validTime;
         refTime = meta.pds1.refTime;
      } else if (meta.GribVersion == -1) {
         validTime = meta.pdsTdlp.refTime + meta.pdsTdlp.project;
         refTime = meta.pdsTdlp.refTime;
      } else {
         MetaFree (&meta);
         continue;
      }

      if ((f_valTime & 1) && (validTime < startTime)) {
         MetaFree (&meta);
         continue;
      }
      if ((f_valTime & 2) && (validTime > endTime)) {
         MetaFree (&meta);
         continue;
      }

#ifdef DEBUG
/*
      elemEnum = genNdfdEnum_fromMeta (&meta);
      printf ("Element is entry %d\n", elemEnum);
*/
#endif

      /* Check if we're interested in this data based on an element match. */
      for (i = 0; i < numElem; i++) {
         if (genElemMatchMeta (&(elem[i]), &meta) == 1) {
            break;
         }
      }
      if (i == numElem) {
         MetaFree (&meta);
         continue;
      }
      elemEnum = genNdfdEnum_fromMeta (&meta);

      /* Check that gds is valid before setting up map projection. */
      if ((GDSValid (&(meta.gds)) != 0) ||
          (meta.gds.Nx * meta.gds.Ny < gribDataLen)) {
         preErrSprintf ("ERROR: Sect3 was not Valid.\n");
         free (gribData);
         IS_Free (&is);
         MetaFree (&meta);
         return -2;
      }
      SetMapParamGDS (&map, &(meta.gds));
      f_sector = SectorFindGDS (&(meta.gds));
      if (f_sector == -1) {
         f_sector = NDFD_OCONUS_UNDEF;
      }

      /* Check if this f_sector, refTime, validTime, element has already
       * been checked. */
      f_interest = 1;
      if (elemEnum != NDFD_UNDEF) {
         for (k = 0; k < *numMatch; k++) {
            if (((*match)[k].refTime == refTime) &&
                ((*match)[k].validTime == validTime) &&
                ((*match)[k].f_sector == f_sector) &&
                ((*match)[k].elem.ndfdEnum == elemEnum)) {
               f_interest = 0;
               break;
            }
         }
      }
      if (f_interest == 0) {
         MetaFree (&meta);
         continue;
      }
      /* Have determined that this is a good match, allocate memory */
      *numMatch = *numMatch + 1;
      *match = (genMatchType *) realloc (*match,
                                         (*numMatch) * sizeof (genMatchType));
      curMatch = &((*match)[*numMatch - 1]);

      /* Might try to use genElemMatchMeta info to help with the enum type.
       * Note: Can't just init the elem type since the data could be
       * NDFD_UNDEF, so we need to call setGenElem. */
      setGenElem (&(curMatch->elem), &meta);
#ifdef DEBUG
      if (curMatch->elem.ndfdEnum != elem[i].ndfdEnum) {
         printf ("%d %d\n", curMatch->elem.ndfdEnum, elem[i].ndfdEnum);
      }
      myAssert (curMatch->elem.ndfdEnum == elem[i].ndfdEnum);
#endif

      /* Set other meta info about the match. */
      curMatch->refTime = refTime;
      curMatch->validTime = validTime;
      curMatch->f_sector = f_sector;
      curMatch->unit = (char *) malloc (strlen (meta.unitName) + 1);
      strcpy (curMatch->unit, meta.unitName);

      /* fill in the value structure. */
      curMatch->numValue = numPnts;
      curMatch->value = (genValueType *) malloc (numPnts * sizeof (genValueType));
      if ((meta.GribVersion == 2) && (strcmp (meta.element, "Wx") == 0)) {
         genFillValue (gribDataLen, gribData, &(meta.gridAttrib), &map,
                       meta.gds.Nx, meta.gds.Ny, f_interp, &(meta.pds2.sect2.wx), NULL, f_WxParse,
                       numPnts, pnts, f_pntType, curMatch->value,
                       f_avgInterp);

      } else if ((meta.GribVersion == 2) && (strcmp (meta.element, "WWA") == 0)) {
         genFillValue (gribDataLen, gribData, &(meta.gridAttrib), &map,
                       meta.gds.Nx, meta.gds.Ny, f_interp, NULL, &(meta.pds2.sect2.hazard), f_WxParse,
                       numPnts, pnts, f_pntType, curMatch->value,
                       f_avgInterp);

      } else {
         genFillValue (gribDataLen, gribData, &(meta.gridAttrib), &map,
                       meta.gds.Nx, meta.gds.Ny, f_interp, NULL, NULL, f_WxParse,
                       numPnts, pnts, f_pntType, curMatch->value,
                       f_avgInterp);

      }
      MetaFree (&meta);
   }
   IS_Free (&is);
   free (gribData);
   return 0;
}
#endif

/*****************************************************************************
 * genProbeGrib() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Probe a given Cube file for all messages that match the element criteria,
 * and return the values of the given set of points inside the match
 * structure.
 *
 * ARGUMENTS
 *    filename = The name of the cube index file. (Input)
 *                      POINT FILTERING INFO
 *     numPnts = Number of points (Input)
 *        pnts = The points to probe. (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y (Input)
 *                      ELEMENT FILTERING INFO
 *     numElem = Number of elements in element filter list. (Input)
 *        elem = Return only data found in this list.
 *               Use NDFD_MATCHALL, to get all the elements. (Input)
 *                      TIME FILTERING INFO
 *   f_valTime = 0 false, 1 f_validStartTime, 2 f_validEndTime,
 *               3 both f_validStartTime, and f_validEndTime (Input)
 *   startTime = first valid time that we are interested in. (Input)
 *     endTime = last valid time that we are interested in. (Input)
 *                      SPECIAL FLAGS
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *      f_unit = 0 -Unit n || 1 -Unit e || 2 -Unit m (Input)
 *    majEarth = Use this to override the majEarth (< 6000 ignored) (Input)
 *    minEarth = Use this to override the minEarth (< 6000 ignored) (Input)
 *   f_wxParse = 0 => ugly string, 1 => English Translation,
 *               2 => -SimpleWx code. (Input)
 * f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *                      OUTPUT
 *    numMatch = Number of matches found. (Output)
 *       match = Matches. (Output)
 *
 * RETURNS: int
 *   -1 = problems reading a GRIB message
 *   -2 = problems with the Grid Definition Section.
 *
 *  2/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static int genProbeCube (const char *filename, size_t numPnts,
                         const Point * pnts, sChar f_pntType, size_t numElem,
                         const genElemDescript * elem, sChar f_valTime,
                         double startTime, double endTime, uChar f_interp,
                         sChar f_unit, double majEarth, double minEarth,
                         sChar f_WxParse, uChar f_XML, sChar f_SimpleVer, sChar f_SimpleWWA,
                         size_t *numMatch, genMatchType ** match)
{
   char *flxArray = NULL; /* The index file in a char buffer. */
   int flxArrayLen;     /* The length of the flxArray buffer. */
   char *ptr;           /* A pointer to where we are in the array. */
   uShort2 numGDS;      /* # of GDS Sections. */
   uShort2 numSupPDS;   /* # of Super PDS Sections. */
   char *sPtr;          /* A pointer to the current SuperPDS. */
   sInt4 lenTotPds;     /* Length of the total PDS record */
   size_t i;            /* Loop counter over SuperPDS. */
   size_t jj;           /* Loop over the desired elements */
   char elemName[256];  /* A holder for element from meta data. */
   double refTime;      /* Reference time of this data set. */
   char unit[256];      /* A holder for unit for this data. */
   char comment[256];   /* A holder for comment from meta data. */
   uShort2 gdsNum;      /* Which GDS is associated with this data. */
   uShort2 center;      /* The center that created this data */
   uShort2 subCenter;   /* The subCenter that created this data */
   uShort2 numPDS;      /* number of PDS Sections. */
   char *pdsPtr;        /* A pointer to the current PDS in the PDS array. */
   int j;               /* Loop counter over PDS Array. */
   double validTime;    /* Valid time of this PDS. */
   char dataFile[256];  /* A holder for the Data file for this record. */
   char curFile[256];   /* A holder for the Current Data file. */
   sInt4 dataOffset;    /* An offset into dataFile for this record. */
   uChar f_bigEndian;   /* Endian'ness of the data grid. */
   uChar scan;          /* Scan mode for the data grid. */
   uShort2 numTable = 0; /* Number of strings in the table */
   char **table = NULL; /* Table of strings associated with this PDS. */
   int k;               /* Loop counter over table entries. */
   int elemEnum;        /* The NDFD element enumeration for the read grid */
   int curGdsNum;       /* Which gdsNum currently in gds. */
   gdsType gds;         /* The current grid definition section. */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   Point *gridPnts = NULL; /* Holds the converted to grid point points. */
   size_t ii;           /* Loop counter over number of points. */
   genMatchType *curMatch; /* The current match */
   char *dataName = NULL; /* The name of the current opened data file. */
   char *lastSlash;     /* A pointer to last slash in the index file. */
   FILE *data = NULL;   /* A pointer to the data file. */
   char f_sector = NDFD_OCONUS_UNDEF; /* Enumerated Sector associated with
                         * this file */
   char f_interest;     /* used to help determine if we've already found
                         * this match so we don't need to do it again. */
   int f_chooseMatch = 0; /* Flag used to determine if element's data is to be
                          * returned as a match as determined by element's
                          * starting and ending valid times. */

   if (ReadFLX (filename, &flxArray, &flxArrayLen) != 0) {
      errSprintf ("Problems Reading %s\n", filename);
      return -1;
   }

   /* Start walking through the flx array. */
   ptr = flxArray + HEADLEN;
   MEMCPY_LIT (&numGDS, ptr, sizeof (uShort2));
   ptr += 2 + numGDS * GDSLEN;
   MEMCPY_LIT (&numSupPDS, ptr, sizeof (uShort2));
   ptr += 2;
   sPtr = ptr;

   curGdsNum = -1;
   if (f_pntType == 0) {
      gridPnts = (Point *) malloc (numPnts * sizeof (Point));
   }
   curFile[0] = '\0';
   if ((lastSlash = strrchr (filename, '/')) == NULL) {
      lastSlash = strrchr (filename, '\\');
   }

   for (i = 0; i < numSupPDS; i++) {
      ReadSupPDSBuff (sPtr, elemName, &refTime, unit, comment, &gdsNum,
                      &center, &subCenter, &numPDS, &pdsPtr, &lenTotPds);
/*
      if (center != 8) {
         sPtr += lenTotPds;
         continue;
      }
*/
      elemEnum = gen_NDFD_NDGD_Lookup (elemName, 1, 0);
      if (elemEnum == NDFD_UNDEF) {
         sPtr += lenTotPds;
         continue;
      }
#ifdef DEBUG
/*
printf ("element is %d\n", elemEnum);
*/
#endif
      /* Check if we're interested in this data based on an element match. */
      for (jj = 0; jj < numElem; jj++) {
         if (elem[jj].ndfdEnum == elemEnum) {
            break;
         }
      }
      if (jj == numElem) {
         sPtr += lenTotPds;
         continue;
      }

      for (j = 0; j < numPDS; j++) {
         ReadPDSBuff (pdsPtr, &validTime, dataFile, &dataOffset,
                      &f_bigEndian, &scan, &numTable, &table, &pdsPtr);

         f_chooseMatch = 1;
         /* Do we return data? */
         if ((f_XML == 1) || (f_XML == 2) || (f_XML == 5) || (f_XML == 6)) {
            /* Check if we're interested in this data based on validTime and
             * user supplied startTime and/or endTime. If f_chooseMatch flag
             * remains true, return match.
             */
            f_chooseMatch = validMatch(validTime, refTime, elemEnum, f_valTime,
                                       startTime, endTime);
         } else { /* If product is summary type (f_XML = 3 or 4). */
            /* Check if we're interested in this data based on validTime. */
            if (((f_valTime & 1) && (validTime < startTime)) ||
                ((f_valTime & 2) && (validTime > endTime))) {
               f_chooseMatch = 0;
            }
         }

         /* Check flag to see if interested in data. */
         if (!f_chooseMatch) {
            /* Not interested in data. Free some things. */
            if (numTable != 0) {
               for (k = 0; k < numTable; k++) {
                  free (table[k]);
               }
               free (table);
               numTable = 0;
               table = NULL;
            }
            continue /* To next "j" value (next validTime) */;
         } else {
            /* Interested in data. */
            /* Set up gds. */
            if (curGdsNum != gdsNum) {
               ReadGDSBuffer (flxArray + HEADLEN + 2 + (gdsNum - 1) * GDSLEN,
                              &gds);
               /* Check that gds is valid before setting up map projection. */
               if (GDSValid (&gds) != 0) {
                  errSprintf ("ERROR: Sect3 was not Valid.\n");
                  goto error;
               }
               SetMapParamGDS (&map, &gds);
               f_sector = SectorFindGDS (&gds);
               if (f_sector == -1) {
                  f_sector = NDFD_OCONUS_UNDEF;
               }

               /* Get points on the grid. */
               myAssert ((f_pntType == 0) || (f_pntType == 1));
               if (f_pntType == 0) {
                  for (ii = 0; ii < numPnts; ii++) {
                     myCll2xy (&map, pnts[ii].Y, pnts[ii].X, &(gridPnts[ii].X),
                               &(gridPnts[ii].Y));
                  }
               }
            }

            /* Check if this f_sector, refTime, validTime, element has already
             * been checked. */
            f_interest = 1;
            if (elemEnum != NDFD_UNDEF) {
               for (k = 0; k < *numMatch; k++) {
                  if (((*match)[k].refTime == refTime) &&
                      ((*match)[k].validTime == validTime) &&
                      ((*match)[k].f_sector == f_sector) &&
                      ((*match)[k].elem.ndfdEnum == elemEnum)) {
                     f_interest = 0;
                     break;
                  }
               }
            }
            if (f_interest == 0) {
               if (numTable != 0) {
                  for (k = 0; k < numTable; k++) {
                     free (table[k]);
                  }
                  free (table);
                  numTable = 0;
                  table = NULL;
               }
               continue;
            }

            if (strcmp (curFile, dataFile) != 0) {
               if (lastSlash == NULL) {
                  dataName = (char *) realloc (dataName, strlen (dataFile) + 1);
                  strcpy (dataName, dataFile);
               } else {
                  dataName = (char *) realloc (dataName,
                                               (lastSlash - filename) + 1 +
                                               strlen (dataFile) + 1);
                  memcpy (dataName, filename, (lastSlash - filename) + 1);
                  dataName[(lastSlash - filename) + 1] = '\0';
                  strcat (dataName, dataFile);
               }
               strcpy (curFile, dataFile);
               if (data != NULL) {
                  fclose (data);
               }
               if ((data = fopen (dataName, "rb")) == NULL) {
                  errSprintf ("Problems opening %s\n", dataName);
                  goto error;
               }
            }

            /* Have determined that this is a good match, allocate memory */
            *numMatch = *numMatch + 1;
            *match = (genMatchType *) realloc (*match,
                                               (*numMatch) *
                                               sizeof (genMatchType));
            curMatch = &((*match)[*numMatch - 1]);

            /* Set Element info about the match */
            memcpy (&(curMatch->elem), &(NdfdElements[elemEnum]),
                    sizeof (genElemDescript));
            /* Set other meta info about the match. */
            curMatch->refTime = refTime;
            curMatch->validTime = validTime;
            curMatch->f_sector = f_sector;
            /*  Moving the following into genCubeFillValue()
            curMatch->unit = (char *) malloc (strlen (unit) + 1);
            strcpy (curMatch->unit, unit);
            */
            
            /* Fill the value structure. */
            curMatch->numValue = numPnts;
            curMatch->value = (genValueType *) malloc (numPnts *
                                                       sizeof (genValueType));
            /* Read from data, and fill in the value. */
            if (f_pntType == 0) {
               genCubeFillValue (data, dataOffset, scan, f_bigEndian, &map,
                                 numPnts, gridPnts, gds.Nx, gds.Ny, f_interp,
                                 elemEnum, numTable, table, f_WxParse,
                                 f_SimpleVer, f_SimpleWWA, unit, f_unit, 
                                 &curMatch->unit, curMatch->value);
            } else {
               genCubeFillValue (data, dataOffset, scan, f_bigEndian, &map,
                                 numPnts, pnts, gds.Nx, gds.Ny, f_interp,
                                 elemEnum, numTable, table, f_WxParse,
                                 f_SimpleVer, f_SimpleWWA, unit, f_unit, 
                                 &curMatch->unit, curMatch->value);
            }

            if (numTable != 0) {
               for (k = 0; k < numTable; k++) {
                  free (table[k]);
               }
               free (table);
               numTable = 0;
               table = NULL;
            }
         }
      }
      sPtr += lenTotPds;
   }

   if (data != NULL) {
      fclose (data);
   }
   if (dataName != NULL) {
      free (dataName);
   }
   if (gridPnts != NULL) {
      free (gridPnts);
   }
   free (flxArray);
   return 0;
 error:
   if (numTable != 0) {
      for (k = 0; k < numTable; k++) {
         free (table[k]);
      }
      free (table);
      numTable = 0;
      table = NULL;
   }

   if (data != NULL) {
      fclose (data);
   }
   if (dataName != NULL) {
      free (dataName);
   }
   if (gridPnts != NULL) {
      free (gridPnts);
   }
   free (flxArray);
   return -2;
}

/*****************************************************************************
 * genProbe() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Probes the given files for data that matches the given elements.
 * Returns the values at the given points.
 *
 * ARGUMENTS
 *      numPnts = Number of points to look at. (Input)
 *         pnts = Points to look at. (Input)
 *    f_pntType = 0 => lat/lon pnts, 1 => cell X/Y pnts, 2 => all Cells. (In)
 *   numInFiles = Number of input files. (Input)
 *      inFiles = Input file names. (Input)
 *   f_fileType = Type of input files. (0=GRIB, 1=Data Cube index file) (Input)
 *     f_interp = 1 => interpolate to points, 0 => nearest neighbor. (Input)
 *       f_unit = 0 -Unit n || 1 -Unit e || 2 -Unit m (Input)
 *     majEarth = Use this to override the majEarth (< 6000 ignored) (Input)
 *     minEarth = Use this to override the minEarth (< 6000 ignored) (Input)
 *    f_wxParse = 0 => ugly string, 1 => English Translation,
 *                2 => -SimpleWx code. (Input)
 *  f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *      numElem = Number of elements in element filter list. (Input)
 *         elem = Return only data found in this list.
 *                Use NDFD_MATCHALL, to get all the elements. (Input)
 *    f_valTime = 0 false, 1 f_validStartTime, 2 f_validEndTime,
 *                3 both f_validStartTime, and f_validEndTime (Input)
 *    startTime = first valid time that we are interested in. (Input)
 *      endTime = last valid time that we are interested in. (Input)
 *        f_XML = Flag denoting type of XML product (1 = DWMLgen's
 *                "time-series"product, 2 = DWMLgen's "glance" product, 3
 *                = DWMLgenByDay's "12 hourly" product, 4 = DWMLgenByDay's
 *                "24 hourly" product, 5 = DWMLgen's "RTMA time-series"
 *                product, 6 = DWMLgen's mix of "RTMA & NDFD time-series"
 *                product. (Input)
 *     numMatch = Number of matches found. (Output)
 *        match = Matches. (Output)
 *    f_inTypes = File types of InFiles array. (Input)
 *   gribFilter = File filter used to find GRIB or data cube files. (Input)
 *    numSector = Number of sectors that points were found in. (Input)
 *       sector = Names of sectors that points were found in. (Input)
 * f_ndfdConven = NDFD Naming convention to use. (Input)
 *
 * RETURNS: int
 *   -1 = If numMatch or match is not 0 or NULL respectively to begin with.
 *   -2 = numInfiles < 1. (Should probably change to numInFiles = 0 => stdin)
 *   -3 = Problems opening an input file.
 *
 * 12/2005 Arthur Taylor (MDL): Created.
 *  1/2006 AAT: Modified so some matches will return values, and it will
 *         ignore bad files.
 *
 * NOTES:
 *   1) May want to add a valid time list to also match.
 *   2) Assumes that inFiles[0] = NULL implies use stdin.
 *
 *   3) with f_pntType = 1 or 2. may want to return to caller the lat/lon.
 *      Problem... lat/lon would theoretically change based on GDS.
 *****************************************************************************
 */
int genProbe (size_t numPnts, Point * pnts, sChar f_pntType,
              size_t numInFiles, char **inFiles, uChar f_fileType,
              uChar f_interp, sChar f_unit, double majEarth, double minEarth,
              sChar f_WxParse, sChar f_SimpleVer, sChar f_SimpleWWA, size_t numElem,
              genElemDescript * elem, sChar f_valTime, double startTime,
              double endTime, uChar f_XML, size_t *numMatch, genMatchType ** match,
              char *f_inTypes, char *gribFilter, size_t numSector,
              char **sector, sChar f_ndfdConven, sChar f_avgInterp)
{
#ifndef DP_ONLY
   FILE *fp;
#endif
   char f_stdin;
   size_t i;
#ifdef DEBUG
   char *msg;
#endif
   size_t numOutNames;
   char **outNames;
   char f_conus2_5;     /* whether 2.5km res conus was seen in sector list. */
   char f_conus5;       /* whether 5km res conus was seen in sector list. */
   char f_nhemi;        /* whether nhemi was seen in sector list. */
   char f_pr;           /* whether pr was seen in sector list. */
   char f_npacocn;      /* whether N Pacific was seen in sector list. */
   char f_hawaii;       /* whether Hawaii was seen in sector list. */
   char f_guam;         /* whether Guam was seen in sector list. */
   char **sect2;        /* used to temporarily expand sector list. */
   int numAddSect;      /* Number of additional sectors needed.  Handles the
                         * case when we have either (Puerto Rico or Conus,
                         * but not nhemi) or (Guam or Hawaii, but not npacocn)
                         * The reason we need npacocn and nhemi in that case
                         * is because Tropical Wind Threshold data is only
                         * found in Nhemi/npacocn sectors. */

   myAssert (*numMatch == 0);
   myAssert (*match == NULL);
   /* Check input state. */
   if ((*numMatch != 0) || (*match != NULL))
      return -1;
   if (numInFiles < 1)
      return -2;

#ifdef DP_ONLY
   if (f_fileType == 0) {
      printf ("DP only executable doesn't handle -P option\n");
      myAssert (1 == 0);
      return -3;
   }
#endif

   myAssert (numInFiles > 0);
   f_stdin = (inFiles[0] == NULL);

   /* Assert that f_stdin does not apply to f_fileType == 1 */
   /* Actually it can, by assuming that datdName is in cur dir. */
   myAssert ((f_fileType != 1) || (!f_stdin));

   myAssert (numElem != 0);

   f_conus5 = 0;
   f_conus2_5 = 0;
   f_nhemi = 0;
   f_pr = 0;
   f_npacocn = 0;
   f_hawaii = 0;
   f_guam = 0;
   for (i = 0; i < numSector; i++) {
      if (strcmp (sector[i], "conus5") == 0) {
         f_conus5 = 1;
      } else if (strcmp (sector[i], "conus2_5") == 0) {
         f_conus2_5 = 1;
      } else if (strcmp (sector[i], "nhemi") == 0) {
         f_nhemi = 1;
      } else if (strcmp (sector[i], "puertori") == 0) {
         f_pr = 1;
      } else if (strcmp (sector[i], "npacocn") == 0) {
         f_npacocn = 1;
      } else if (strcmp (sector[i], "hawaii") == 0) {
         f_hawaii = 1;
      } else if (strcmp (sector[i], "guam") == 0) {
         f_guam = 1;
      }
   }
   if ((!f_nhemi && (f_conus2_5 || f_conus5 || f_pr)) ||
       (!f_npacocn && (f_hawaii || f_guam))) {
      numAddSect = 0;
      if (!f_nhemi && (f_conus2_5 || f_conus5 || f_pr)) {
         numAddSect++;
      }
      if (!f_npacocn && (f_hawaii || f_guam)) {
         numAddSect++;
      }
      sect2 = (char **) malloc ((numSector + numAddSect) * sizeof (char *));
      for (i = 0; i < numSector; i++) {
         sect2[i] = sector[i];
      }
      if (!f_nhemi && (f_conus2_5 || f_conus5 || f_pr)) {
         sect2[numSector] = (char *) malloc (6 * sizeof (char *));
         strcpy (sect2[numSector], "nhemi");
      }
      if (!f_npacocn && (f_hawaii || f_guam)) {
         if (numAddSect == 1) {
            sect2[numSector] = (char *) malloc (8 * sizeof (char *));
            strcpy (sect2[numSector], "npacocn");
         } else {
            sect2[numSector + 1] = (char *) malloc (8 * sizeof (char *));
            strcpy (sect2[numSector + 1], "npacocn");
         }
      }

      /* Expand the input files... */
      expandInName (numInFiles, inFiles, f_inTypes, gribFilter,
                    numSector + numAddSect, sect2, f_ndfdConven, numElem,
                    elem, &numOutNames, &outNames);
      free (sect2[numSector]);
      if (numAddSect == 2) {
         free (sect2[numSector + 1]);
      }
      free (sect2);
   } else {
      /* Expand the input files... */
      expandInName (numInFiles, inFiles, f_inTypes, gribFilter, numSector,
                    sector, f_ndfdConven, numElem, elem, &numOutNames,
                    &outNames);
   }
/*
#ifdef DEBUG
   for (i = 0; i < numOutNames; i++) {
      printf ("outnames[%d] = %s\n", i, outNames[i]);
   }
#endif
*/
   for (i = 0; i < numOutNames; i++) {
#ifndef DP_ONLY
      if (f_fileType == 0) {
         if ((i == 0) && f_stdin) {
            fp = stdin;
         } else {
            if ((fp = fopen (outNames[i], "rb")) == NULL) {
               continue;
            }
         }
         if (genProbeGrib (fp, numPnts, pnts, f_pntType, numElem, elem,
                           f_valTime, startTime, endTime, f_interp, f_unit,
                           majEarth, minEarth, f_WxParse, f_SimpleVer, f_SimpleWWA,
                           numMatch, match, f_avgInterp) != 0) {
#ifdef DEBUG
            msg = errSprintf (NULL);
            printf ("Error message was: '%s'\n", msg);
            free (msg);
#endif
            if (!f_stdin) {
#ifdef DEBUG
               printf ("\nProblems with GRIB file '%s'\n", outNames[i]);
#endif
               fclose (fp);
            }
            continue;
            /* return -3; */
         }
         if (!f_stdin) {
            fclose (fp);
         }
      } else {
#endif
         if (genProbeCube (outNames[i], numPnts, pnts, f_pntType, numElem,
                           elem, f_valTime, startTime, endTime, f_interp,
                           f_unit, majEarth, minEarth, f_WxParse, f_XML,
                           f_SimpleVer, f_SimpleWWA, numMatch, match) != 0) {
#ifdef DEBUG
            msg = errSprintf (NULL);
            printf ("Error message was: '%s'\n", msg);
            free (msg);
            printf ("\nProblems with Index file '%s'\n", outNames[i]);
#endif
         }
         continue;
#ifndef DP_ONLY
      }
#endif
   }

#ifdef DEBUG
/*
   for (i=0; i < *numMatch; i++) {
      if ((*match)[i].value->valueType == 2) {
         printf ("%d : elem %d : sector %d refTime %f valTime %f missing\n", i,
                 (*match)[i].elem.ndfdEnum, (*match)[i].f_sector,
                 (*match)[i].refTime, (*match)[i].validTime);
      } else {
         printf ("%d : elem %d : sector %d refTime %f valTime %f val %s\n", i,
                 (*match)[i].elem.ndfdEnum, (*match)[i].f_sector,
                 (*match)[i].refTime, (*match)[i].validTime, (*match)[i].value->str);
      }
   }
*/
#endif

   for (i = 0; i < numOutNames; i++) {
      free (outNames[i]);
   }
   free (outNames);
   return 0;
}

/*
Following is what fortran programmers wanted...
 * filename = The GRIB file to probe. (Input)
 *  numPnts = The number of points to probe. (Input)
 *      lat = The latitudes of the points to probe. (Input)
 *      lon = The longitudes of the points to probe. (Input)
 * f_interp = true (1) if we want to perform bi-linear interp
 *            false (0) if we want nearest neighbor (Input)
 *  lenTime = The number of messages (or validTimes) in the file (Output)
 *  valTime = The array of valid times (as strings). (Output)
 *     data = The values at the various points. (Output)
int GenericProbe (char *filename, int numPnts, double *lat, double *lon,
                  int f_interp, int *lenTime, char ***valTime,
                  double ***data)

GRIB2Probe (usr, &is, &meta, numPnts, pnts, labels, pntFiles,
                           f_pntType);
Grib2DataProbe (usr, numPnts, pnts, labels, pntFiles);
*/

/* Want to join together relevant parts of:
probe.c :: int GRIB2Probe (userType * usr, IS_dataType * is, grib_MetaData * meta)
and
genprobe.c :: int Grib2DataProbe (userType * usr)
*/

/*****************************************************************************
 * ReadPntFile() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Read in a set of points from pntFile, for use with the probe command.
 *
 * ARGUMENTS
 * pntFile = File to read the points in from. (Input)
 *    pnts = The points read in. (Input/Output)
 * NumPnts = The number of points read in (Input/Output)
 *  labels = The Labels for those points (Input/Output)
 *   files = The Output file for each point (Input/Output)
 *
 * FILES/DATABASES:
 *   A comma delimited file with (place, lat, lon) per line.
 *   A '#' at beginnig of line denotes line is commented out.
 *   Don't really need commas, just spaces.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = Ok.
 * -1 = Problems opening file for read.
 *
 * HISTORY
 *  12/2002 Arthur Taylor (MDL/RSIS): Created.
 *   8/2003 AAT: Found that it didn't handle "lat,lon" correctly.
 *   1/2005 AAT: Due to frequent requests no longer allow the space separator
 *               so that people's stations can have spaces.
 *   1/2005 AAT: Added an optional forth element which is what file to save a
 *               point to.
 *
 * NOTES
 *****************************************************************************
 */
static int ReadPntFile (char *pntFile, Point ** pnts, size_t *NumPnts,
                        char ***labels, char ***files)
{
   FILE *fp;            /* Ptr to point file. */
   char *buffer = NULL; /* Holds a line from the file. */
   size_t buffLen = 0;  /* Current length of buffer. */
   char *first;         /* The first phrase in buffer. */
   char *second;        /* The second phrase in buffer. */
   char *third;         /* The third phrase in buffer. */
   char *forth;         /* The forth phrase in buffer. */
   size_t numPnts;      /* Local count of number of points. */

   if ((fp = fopen (pntFile, "rt")) == NULL) {
      errSprintf ("ERROR: opening file %s for read", pntFile);
      return -1;
   }
   numPnts = *NumPnts;
   while (reallocFGets (&buffer, &buffLen, fp) > 0) {
/*      first = strtok (buffer, " ,\n"); */
      first = strtok (buffer, ",\n");
      if ((first != NULL) && (*first != '#')) {
/*         second = strtok (NULL, " ,\n"); */
         second = strtok (NULL, ",\n");
         if (second != NULL) {
            numPnts++;
            *pnts = (Point *) realloc ((void *) *pnts,
                                       numPnts * sizeof (Point));
            *labels = (char **) realloc ((void *) *labels,
                                         numPnts * sizeof (char *));
            *files = (char **) realloc ((void *) *files,
                                        numPnts * sizeof (char *));
/*            third = strtok (NULL, " ,\n"); */
            third = strtok (NULL, ",\n");
            if (third != NULL) {
               /* Assume: Name, lat, lon */
               (*pnts)[numPnts - 1].Y = atof (second);
               (*pnts)[numPnts - 1].X = atof (third);
               (*labels)[numPnts - 1] = (char *) malloc (strlen (first) + 1);
               strcpy ((*labels)[numPnts - 1], first);
               forth = strtok (NULL, ",\n");
               if (forth != NULL) {
                  strTrim (forth);
                  (*files)[numPnts - 1] =
                        (char *) malloc (strlen (forth) + 1);
                  strcpy ((*files)[numPnts - 1], forth);
               } else {
                  (*files)[numPnts - 1] = NULL;
               }
            } else {
               /* Assume: lat, lon */
               (*pnts)[numPnts - 1].Y = atof (first);
               (*pnts)[numPnts - 1].X = atof (second);
               mallocSprintf (&((*labels)[numPnts - 1]), "(%f,%f)",
                              (*pnts)[numPnts - 1].Y, (*pnts)[numPnts - 1].X);
            }
         } else {
            *NumPnts = numPnts;
            errSprintf ("ERROR: problems parsing '%s'", buffer);
            free (buffer);
            fclose (fp);
            return -1;
         }
      }
   }
   free (buffer);
   fclose (fp);
   *NumPnts = numPnts;
   return 0;
}

/*****************************************************************************
 * Grib2DataProbe() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Probe an index file in a similar manner to how we probed a GRIB file.
 *
 * ARGUMENTS
 * usr = The user option structure to use while 'Probing'. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 *  1 = Error.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 * May want to move some of this to a ReadPDS in database.c
 * May want to combine some of this with the probe.c stuff.
 *****************************************************************************
 */
int Grib2DataProbe (userType *usr, int numPnts, Point * pnts, char **labels,
                    char **pntFiles)
{
   char *flxArray = NULL; /* The index file in a char buffer. */
   int flxArrayLen;     /* The length of the flxArray buffer. */
   sInt4 *grid_X = NULL; /* The nearest grid point (x coord) */
   sInt4 *grid_Y = NULL; /* The nearest grid point (x coord) */
   int grid_gdsIndex;   /* Which gdsIndex is correct for X, Y. */
   char format[20];     /* Format (# of decimals) to print the data with. */
   char *ptr;           /* A pointer to where we are in the array. */
   uShort2 numGDS;      /* # of GDS Sections. */
   uShort2 numSupPDS;   /* # of Super PDS Sections. */
   char *sPtr;          /* A pointer to the current SuperPDS. */
   int i;               /* Loop counter over SuperPDS. */
   sInt4 lenTotPDS;     /* Length of total PDS section. */
   uChar numBytes;      /* number of bytes in following string. */
   char elem[256];      /* A holder for element from meta data. */
   double refTime;      /* Reference time of this data set. */
   char unit[256];      /* A holder for unit for this data. */
   /* char comment[256]; *//* A holder for comment from meta data. */
   uShort2 gdsIndex;    /* Which GDS is associated with this data. */
   uShort2 numPDS;      /* number of PDS Sections. */
   char *PDSptr;        /* A pointer to the current PDS in the PDS array. */
   int j;               /* Loop counter over PDS Array. */
   uShort2 lenPDS;      /* The length of the current PDS. */
   double validTime;    /* Valid time of this PDS. */
   char dataFile[256];  /* A holder for the Data file for this record. */
   sInt4 dataOffset;    /* An offset into dataFile for this record. */
   uChar endian;        /* Endian'ness of the data grid. */
   uChar scan;          /* Scan mode for the data grid. */
   char *gdsPtr;        /* The location of the current GDS data. */
   gdsType gds;         /* The current grid definition section. */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   int k;               /* Loop counter over number of probed points. */
   double newX, newY;   /* The grid (1..n)(1..m) value of a given lat/lon. */
   time_t tempTime;     /* Used when printing out a "double" time_t value. */
   char refBuff[21];    /* The reference time in ASCII form. */
   char validBuff[21];  /* The valid time in ASCII form. */
   FILE *data = NULL;   /* A pointer to the data file. */
   char *curDataName = NULL; /* The name of the current opened data file. */
   sInt4 offset;        /* Where the current data is in the data file. */
   float value;         /* The current cell value. */
   uShort2 numTable = 0; /* Number of strings in the table */
   uShort2 sNumBytes;   /* number of bytes in an "ugly" string. */
   char **table = NULL; /* Table of strings associated with this PDS. */
   int tableIndex;      /* 'value' cast to an integer for table lookup. */
   int jj;              /* Counter used to print "english" weather. */
   UglyStringType ugly; /* Used to 'translate' the weather keys. */
   char *lastSlash;     /* A pointer to last slash in the index file. */
   HazardStringType haz;

   if (usr->Asc2Flx_File != NULL) {
      Asc2Flx (usr->Asc2Flx_File, usr->inNames[0]);
      return 0;
   }
   if (ReadFLX (usr->inNames[0], &flxArray, &flxArrayLen) != 0) {
      errSprintf ("Problems Reading %s\n", usr->inNames[0]);
      goto error;
   }
   if (usr->f_Print) {
      PrintFLXBuffer (flxArray, flxArrayLen);
      goto done;
   }

   /* Allocate space for grid pnts */
   grid_X = (sInt4 *) malloc (numPnts * sizeof (sInt4));
   grid_Y = (sInt4 *) malloc (numPnts * sizeof (sInt4));
   grid_gdsIndex = -1;

   /* Print labels */
   if (usr->f_pntStyle == 0) {
      printf ("element%sunit%srefTime%svalidTime%s", usr->separator,
              usr->separator, usr->separator, usr->separator);
      for (i = 0; i < numPnts; i++) {
         if (i != numPnts - 1) {
            printf ("%s%s", labels[i], usr->separator);
         } else {
            printf ("%s", labels[i]);
         }
      }
      printf ("\n");
   } else {
      printf ("Location%sElement[Unit]%srefTime%svalidTime%sValue\n",
              usr->separator, usr->separator, usr->separator, usr->separator);
   }

   /* Set up output format. */
   sprintf (format, "%%.%df", usr->decimal);

   /* Start walking through the flx array. */
   ptr = flxArray + HEADLEN;
   MEMCPY_LIT (&numGDS, ptr, sizeof (uShort2));
   ptr += 2 + numGDS * GDSLEN;
   MEMCPY_LIT (&numSupPDS, ptr, sizeof (uShort2));
   ptr += 2;
   sPtr = ptr;
   for (i = 0; i < numSupPDS; i++) {
      MEMCPY_LIT (&lenTotPDS, sPtr, sizeof (sInt4));
      ptr = sPtr + 4;
      /* Skip sizeof super PDS. */
      ptr += 2;
      numBytes = *ptr;
      ptr++;
      strncpy (elem, ptr, numBytes);
      elem[numBytes] = '\0';
      ptr += numBytes;
      /* Compare matchElem to elem. */
/*
      if (usr->matchElem != NULL) {
         if (strcmp (elem, usr->matchElem) != 0) {
            sPtr += lenTotPDS;
            break;
         }
      }
*/
      MEMCPY_LIT (&refTime, ptr, sizeof (double));
      ptr += 8;
/*
      if (usr->matchRefTime != -1) {
         if (refTime != usr->matchRefTime) {
            sPtr += lenTotPDS;
            break;
         }
      }
*/
      numBytes = *ptr;
      ptr++;
      strncpy (unit, ptr, numBytes);
      unit[numBytes] = '\0';
      ptr += numBytes;
      numBytes = *ptr;
      ptr++;
      /* Skip comment. */
      ptr += numBytes;
      MEMCPY_LIT (&gdsIndex, ptr, sizeof (uShort2));
      ptr += 2;
      /* Skip center / subcenter. */
      ptr += 2 + 2;
      MEMCPY_LIT (&numPDS, ptr, sizeof (uShort2));
      ptr += 2;
      PDSptr = ptr;
      for (j = 0; j < numPDS; j++) {
         MEMCPY_LIT (&lenPDS, PDSptr, sizeof (uShort2));
         ptr = PDSptr + 2;
         MEMCPY_LIT (&validTime, ptr, sizeof (double));
         ptr += 8;
/*
         if (usr->matchValidTime != -1) {
            if (validTime != usr->matchValidTime) {
               PDSptr += lenPDS;
               break;
            }
         }
*/
         numBytes = *ptr;
         ptr++;
         memcpy (dataFile, ptr, numBytes);
         ptr += numBytes;
         dataFile[numBytes] = '\0';
         MEMCPY_LIT (&dataOffset, ptr, sizeof (sInt4));
         ptr += 4;
         endian = *ptr;
         ptr++;
         scan = *ptr;
         ptr++;
         /* Check if numTable is != 0... if so free table. */
         if (numTable != 0) {
            for (k = 0; k < numTable; k++) {
               free (table[k]);
            }
            free (table);
         }
         MEMCPY_LIT (&numTable, ptr, sizeof (uShort2));
         if (numTable != 0) {
            table = (char **) malloc (numTable * sizeof (char *));
            ptr += 2;
            for (k = 0; k < numTable; k++) {
               MEMCPY_LIT (&sNumBytes, ptr, sizeof (uShort2));
               ptr += 2;
               table[k] = (char *) malloc (sNumBytes + 1);
               memcpy (table[k], ptr, sNumBytes);
               ptr += sNumBytes;
               table[k][sNumBytes] = '\0';
            }
         }
         if (grid_gdsIndex != gdsIndex) {
            gdsPtr = flxArray + HEADLEN + 2 + (gdsIndex - 1) * GDSLEN;
            ReadGDSBuffer (gdsPtr, &gds);

            /* Check that gds is valid before setting up map projection. */
            if (GDSValid (&gds) != 0) {
               preErrSprintf ("ERROR: Sect3 was not Valid.\n");
               goto error;
            }
            /* Set up the map projection. */
            SetMapParamGDS (&map, &gds);

            for (k = 0; k < numPnts; k++) {
               myCll2xy (&map, pnts[k].Y, pnts[k].X, &newX, &newY);
#ifdef DEBUG
/*
               printf ("Testing: lat %f lon %f -> x %f y %f \n", pnts[k].Y,
                       pnts[k].X, newX, newY);
*/
#endif
               /* Find the nearest grid cell. */
               /* Modified so that points falling off the grid are missing */
               if (newX < 1) {
/*                  grid_X[k] = 1; */
                  grid_X[k] = -1;
               } else if ((newX + .5) > gds.Nx) {
/*                  grid_X[k] = gds.Nx;*/
                  grid_X[k] = -1;
               } else {
                  grid_X[k] = (sInt4) (newX + .5);
               }
               if (newY < 1) {
/*                  grid_Y[k] = 1; */
                  grid_Y[k] = -1;
               } else if ((newY + .5) > gds.Ny) {
/*                  grid_Y[k] = gds.Ny;*/
                  grid_Y[k] = -1;
               } else {
                  grid_Y[k] = (sInt4) (newY + .5);
               }
            }
            grid_gdsIndex = gdsIndex;
         }

         /* Print out what we know. */
         tempTime = (time_t) refTime;
         strftime (refBuff, 20, "%Y%m%d%H%M", gmtime (&tempTime));
         tempTime = (time_t) validTime;
         strftime (validBuff, 20, "%Y%m%d%H%M", gmtime (&tempTime));
         if ((data == NULL) || (curDataName == NULL) ||
             (strcmp (curDataName, dataFile) != 0)) {
            if (curDataName == NULL) {
               curDataName = (char *) malloc (strlen (dataFile) +
                                              strlen (usr->inNames[0]) + 1);
               strcpy (curDataName, usr->inNames[0]);
            } else {
               curDataName = (char *) realloc ((void *) curDataName,
                                               (strlen (dataFile) +
                                                strlen (usr->inNames[0]) +
                                                1));
            }
            if ((lastSlash = strrchr (curDataName, '/')) == NULL) {
               if ((lastSlash = strrchr (curDataName, '\\')) == NULL) {
                  strcpy (curDataName, dataFile);
               } else {
                  strcpy (lastSlash + 1, dataFile);
               }
            } else {
               strcpy (lastSlash + 1, dataFile);
            }
            if ((data = fopen (curDataName, "rb")) == NULL) {
               errSprintf ("Problems opening %s\n", curDataName);
               free (curDataName);
               curDataName = NULL;
               goto error;
            }
         }
         if (usr->f_pntStyle == 0) {
            printf ("%s%s%s%s%s%s%s%s", elem, usr->separator, unit,
                    usr->separator, refBuff, usr->separator, validBuff,
                    usr->separator);
            for (k = 0; k < numPnts; k++) {
               offset = dataOffset;
               myAssert (sizeof (float) == 4);
               if ((grid_X[k] != -1) && (grid_Y[k] != -1)) {
                  if (scan == 0) {
                     offset += (((grid_X[k] - 1) +
                                 ((gds.Ny - 1) - (grid_Y[k] - 1)) * gds.Nx) *
                                sizeof (float));
                  } else {
                     offset += (((grid_X[k] - 1) + (grid_Y[k] - 1) * gds.Nx) *
                                sizeof (float));
                  }
                  fseek (data, offset, SEEK_SET);
                  if (endian) {
                     FREAD_BIG (&value, sizeof (float), 1, data);
                  } else {
                     FREAD_LIT (&value, sizeof (float), 1, data);
                  }
               } else {
                  offset = -1;
                  value = 9999;
               }
#ifdef DEBUG
/*
               printf ("offset = %ld, gds.Nx = %ld, CurX,Y = %ld %ld\n",
                       offset, gds.Nx, grid_X[k], grid_Y[k]);
*/
#endif
               if (numTable != 0) {
                  if (offset == -1) {
                     tableIndex = -1;
                  } else {
                     tableIndex = (int) value;
                  }
                  if ((tableIndex >= 0) && (tableIndex < numTable)) {
                     if (strcmp (elem, "Wx") == 0) {
                        if (usr->f_WxParse == 0) {
                           printf ("%s", table[tableIndex]);
                        } else if (usr->f_WxParse == 1) {
                           ParseUglyString (&ugly, table[tableIndex],
                                            usr->f_SimpleVer);
                           for (jj = 0; jj < NUM_UGLY_WORD; jj++) {
                              if (ugly.english[jj] != NULL) {
                                 if (jj != 0) {
                                    printf (" and ");
                                 }
                                 printf ("%s", ugly.english[jj]);
                              } else {
                                 if (jj == 0) {
                                    printf ("No Weather");
                                 }
                                 break;
                              }
                           }
                           FreeUglyString (&ugly);
                        } else if (usr->f_WxParse == 2) {
                           ParseUglyString (&ugly, table[tableIndex],
                                            usr->f_SimpleVer);
                           printf ("%d", ugly.SimpleCode);
                           FreeUglyString (&ugly);
                        }
                     } else if (strcmp (elem, "WWA") == 0) {
                        if (usr->f_WxParse == 0) {
                           printf ("%s", table[tableIndex]);
                        } else if (usr->f_WxParse == 1) {
                           ParseHazardString (&haz, table[tableIndex], usr->f_SimpleWWA);
                           for (jj = 0; jj < NUM_HAZARD_WORD; jj++) {
                              if (haz.english[jj] != NULL) {
                                 if (jj != 0) {
                                    printf (" and ");
                                 }
                                 printf ("%s", haz.english[jj]);
                              } else {
                                 if (jj == 0) {
                                    printf ("No Hazard");
                                 }
                                 break;
                              }
                           }
                           FreeHazardString (&haz);
                        } else if (usr->f_WxParse == 2) {
                           ParseHazardString (&haz, table[tableIndex], usr->f_SimpleWWA);
                           printf ("%d", haz.SimpleCode);
                           FreeHazardString (&haz);
                        }
                     } else {
                        printf ("%s", table[tableIndex]);
                     }
                  } else {
                     printf ("9999");
                  }
               } else {
                  printf (format, myRound (value, usr->decimal));
               }
               if (k != numPnts - 1) {
                  printf ("%s", usr->separator);
               }
            }
            printf ("\n");
         } else {
            for (k = 0; k < numPnts; k++) {
               printf ("%s%s", labels[k], usr->separator);
               printf ("%s%s%s", elem, unit, usr->separator);
               printf ("%s%s%s%s", refBuff, usr->separator, validBuff,
                       usr->separator);
               offset = dataOffset;
               if ((grid_X[k] != -1) && (grid_Y[k] != -1)) {
                  if (scan == 0) {
                     offset += (((grid_X[k] - 1) +
                                 ((gds.Ny - 1) - (grid_Y[k] - 1)) * gds.Nx) *
                                sizeof (float));
                  } else {
                     offset += (((grid_X[k] - 1) + (grid_Y[k] - 1) * gds.Nx) *
                                sizeof (float));
                  }
                  fseek (data, offset, SEEK_SET);
                  if (endian) {
                     FREAD_BIG (&value, sizeof (float), 1, data);
                  } else {
                     FREAD_LIT (&value, sizeof (float), 1, data);
                  }
               } else {
                  offset = -1;
                  value = 9999;
               }
               if (numTable != 0) {
                  if (offset == -1) {
                     tableIndex = -1;
                  } else {
                     tableIndex = (int) value;
                  }
                  if ((tableIndex >= 0) && (tableIndex < numTable)) {
                     if (strcmp (elem, "Wx") == 0) {
                        if (usr->f_WxParse == 0) {
                           printf ("%s", table[tableIndex]);
                        } else if (usr->f_WxParse == 1) {
                           ParseUglyString (&ugly, table[tableIndex],
                                            usr->f_SimpleVer);
                           for (jj = 0; jj < NUM_UGLY_WORD; jj++) {
                              if (ugly.english[jj] != NULL) {
                                 if (jj != 0) {
                                    printf (" and ");
                                 }
                                 printf ("%s", ugly.english[jj]);
                              } else {
                                 if (jj == 0) {
                                    printf ("No Weather");
                                 }
                                 break;
                              }
                           }
                           FreeUglyString (&ugly);
                        } else if (usr->f_WxParse == 2) {
                           ParseUglyString (&ugly, table[tableIndex],
                                            usr->f_SimpleVer);
                           printf ("%d", ugly.SimpleCode);
                           FreeUglyString (&ugly);
                        }
                     } else if (strcmp (elem, "WWA") == 0) {
                        if (usr->f_WxParse == 0) {
                           printf ("%s", table[tableIndex]);
                        } else if (usr->f_WxParse == 1) {
                           ParseHazardString (&haz, table[tableIndex], usr->f_SimpleWWA);
                           for (jj = 0; jj < NUM_HAZARD_WORD; jj++) {
                              if (haz.english[jj] != NULL) {
                                 if (jj != 0) {
                                    printf (" and ");
                                 }
                                 printf ("%s", haz.english[jj]);
                              } else {
                                 if (jj == 0) {
                                    printf ("No Hazard");
                                 }
                                 break;
                              }
                           }
                           FreeHazardString (&haz);
                        } else if (usr->f_WxParse == 2) {
                           ParseHazardString (&haz, table[tableIndex], usr->f_SimpleWWA);
                           printf ("%d", haz.SimpleCode);
                           FreeHazardString (&haz);
                        }
                     } else {
                        printf ("%s", table[tableIndex]);
                     }
                  } else {
                     printf ("9999");
                  }
               } else {
                  printf (format, myRound (value, usr->decimal));
               }
               printf ("\n");
            }
         }
         PDSptr += lenPDS;
      }
      sPtr += lenTotPDS;
   }

 done:
   if (numTable != 0) {
      for (i = 0; i < numTable; i++) {
         free (table[i]);
      }
      free (table);
   }
   free (flxArray);
   free (grid_X);
   free (grid_Y);
   if (curDataName != NULL) {
      fclose (data);
      free (curDataName);
   }
   return 0;

 error:
   if (numTable != 0) {
      for (i = 0; i < numTable; i++) {
         free (table[i]);
      }
      free (table);
   }
   free (flxArray);
   free (grid_X);
   free (grid_Y);
   if (curDataName != NULL) {
      fclose (data);
      free (curDataName);
   }
   return 1;
}

int ProbeCmd (sChar f_Command, userType *usr)
{
   char *msg;           /* Used to print the error stack */
   size_t numPnts = 0;  /* How many points in pnts */
   Point *pnts = NULL;  /* Array of points we are interested in. */
   char **labels = NULL; /* Array of labels for the points. */
   char **pntFiles = NULL; /* Array of filenames for the points. */
   uChar f_fileType;
   PntSectInfo *pntInfo;
   size_t numSector;
   char **sector;
   size_t i;
   int ans, ans2;

#ifdef DP_ONLY
   if (f_Command == CMD_PROBE) {
      printf ("DP only executable doesn't handle -P option\n");
      myAssert (1 == 0);
      return -1;
   }
#endif

   /* Find the points we want to probe. */
   if (usr->numPnt != 0) {
      numPnts = usr->numPnt;
      pnts = (Point *) malloc (numPnts * sizeof (Point));
      labels = (char **) malloc (numPnts * sizeof (char *));
      pntFiles = (char **) malloc (numPnts * sizeof (char *));

      memcpy (pnts, usr->pnt, numPnts * sizeof (Point));
      for (i = 0; i < numPnts; i++) {
         mallocSprintf (&(labels[i]), "(%f,%f)", pnts[i].Y, pnts[i].X);
         pntFiles[i] = NULL;
      }
   }
   if (usr->pntFile != NULL) {
      if (ReadPntFile (usr->pntFile, &pnts, &numPnts, &labels, &pntFiles)
          != 0) {
         preErrSprintf ("ERROR: In call to ReadPntFile.\n");
         ans = -2;
         goto done;
      }
   } else if (usr->numPnt == 0) {
      if (usr->f_pntType != 2) {
         errSprintf ("ERROR: -pnt was not initialized.\n");
         if (numPnts > 0) {
            free (labels[0]);
            free (pntFiles[0]);
            free (pnts);
            free (labels);
            free (pntFiles);
         }
         return -2;
      }
   }

   /* Do XML Parse */
   f_fileType = 0;
   if (f_Command == CMD_DATAPROBE) {
      f_fileType = 1;
   }
   if ((usr->f_XML != 0) || (usr->f_Graph != 0) || (usr->f_MOTD != 0)) {

      /* Find out the Major sectors for all the points? */
      /* Probe geodata for all the points, and get TZ and Daylight If geoData
       * = NULL, or can't find it, TZ = 0, dayLight = 0 */
      pntInfo = (PntSectInfo *) malloc (numPnts * sizeof (PntSectInfo));
      numSector = 0;
      sector = NULL;
      GetSectorList (usr->sectFile, numPnts, pnts, usr->f_pntType,
                     usr->geoDataDir, pntInfo, &numSector, &sector);

/*    Following is used for debugging point/sector info. */
/*
#ifdef DEBUG
      if (1==1) {
         int i, j;
         for (i=0; i < numPnts; i++) {
            printf ("%d %d\n", i, pntInfo[i].numSector);
            for (j = 0; j < pntInfo[i].numSector; j++) {
               printf ("sector[%d] %d\n", j, pntInfo[i].f_sector[j]);
            }
         }
      }
#endif
*/

      /* Create File names by walking through inNames for dir types. If it is
       * a file then keep going.  If it is a dir, tack on all relevant
       * sectors, and files that match the ndfdVars + the filter */
      if (usr->gribFilter == NULL) {
         if (usr->f_Command == CMD_DATAPROBE) {
            mallocSprintf (&(usr->gribFilter), "*.ind");
         } else {
            mallocSprintf (&(usr->gribFilter), "*.bin");
         }
      }
#ifdef OLD
      sectExpandInName (&(usr->numInNames), &(usr->inNames),
                        &(usr->f_inTypes), usr->gribFilter, numSector, sector,
                        usr->f_ndfdConven, usr->numNdfdVars, usr->ndfdVars);
      if (usr->numInNames == 0) {
         free (pntInfo);
         for (i = 0; i < numSector; i++) {
            free (sector[i]);
         }
         free (sector);
         goto done;
      }
#endif
      /* Fill the PntSectInfo member "cwa" of pntInfo. */
      for (i = 0; i < numPnts; i++) {
         if (usr->cwaBuff != NULL) {
            /* Don't have to worry about freeing it since usr is free'd
             * elsewhere */
            pntInfo[i].cwa = usr->cwaBuff[i];
/*            strcpy (pntInfo[i].cwa, usr->cwaBuff[i]); */
         } else {
            /* Initialize to NULL. */
            pntInfo[i].cwa = NULL;
         }
      }

      ans = 0;
      /* numInNames, inNames <check> f_inTypes * file type from stat (1=dir,
       * 2=file, 3=unknown). * gribFilter * filter to use to find files
       * numSecor, sector * names of all sectors that we have points in *
       * f_ndfdConven * NDFD naming convention to use. * numNdfdVars,
       * ndfdVars <check> */
      if (usr->f_XML != 0) {
#ifdef _DWML_
         ans = XMLParse (usr->f_XML, numPnts, pnts, pntInfo, usr->f_pntType,
                         labels, &(usr->numInNames), &(usr->inNames),
                         f_fileType, usr->f_interp, usr->f_unit, usr->majEarth,
                         usr->minEarth, usr->f_icon, usr->f_SimpleVer, usr->f_SimpleWWA,
                         usr->f_valTime, usr->startTime, usr->endTime,
                         usr->numNdfdVars, usr->ndfdVars, usr->f_inTypes,
                         usr->gribFilter, numSector, sector,
                         usr->f_ndfdConven, usr->rtmaDataDir, usr->f_avgInterp,
                         usr->lampDataDir);
/*
         ans = XMLParse (usr->f_XMLDocType, usr->f_XML, numPnts, pnts, pntInfo,
                         usr->f_pntType, labels, &(usr->numInNames), 
                         &(usr->inNames), f_fileType, usr->f_interp, 
                         usr->f_unit, usr->majEarth, usr->minEarth, 
                         usr->f_icon, usr->f_SimpleVer, usr->f_SimpleWWA,
                         usr->f_valTime, usr->startTime, usr->endTime,
                         usr->numNdfdVars, usr->ndfdVars, usr->f_inTypes,
                         usr->gribFilter, numSector, sector,
                         usr->f_ndfdConven, usr->rtmaDataDir, usr->f_avgInterp,
                         usr->lampDataDir);
*/
#endif
      }
      if (usr->f_Graph != 0) {
         ans2 = GraphProbe (usr->f_Graph, numPnts, pnts, pntInfo,
                            usr->f_pntType, labels, usr->numInNames,
                            usr->inNames, f_fileType, usr->f_interp,
                            usr->f_unit, usr->majEarth, usr->minEarth,
                            usr->f_WxParse, usr->f_SimpleVer, usr->f_SimpleWWA, usr->f_valTime,
                            usr->startTime, usr->endTime, usr->numNdfdVars,
                            usr->ndfdVars, usr->f_inTypes, usr->gribFilter,
                            numSector, sector, usr->f_ndfdConven,
                            usr->f_avgInterp);

         if (ans == 0)
            ans = ans2;
      }
      if (usr->f_MOTD != 0) {
         ans2 = MOTDProbe (usr->f_MOTD, numPnts, pnts, pntInfo,
                           usr->f_pntType, labels, usr->numInNames,
                           usr->inNames, f_fileType, usr->f_interp,
                           usr->f_unit, usr->majEarth, usr->minEarth,
                           usr->f_WxParse, usr->f_SimpleVer, usr->f_SimpleWWA, usr->f_valTime,
                           usr->startTime, usr->endTime, usr->numNdfdVars,
                           usr->ndfdVars, usr->f_inTypes, usr->gribFilter,
                           numSector, sector, usr->f_ndfdConven, usr->f_XML,
                           usr->f_avgInterp);
         if (ans == 0)
            ans = ans2;
      }

      free (pntInfo);
      for (i = 0; i < numSector; i++) {
         free (sector[i]);
      }
      free (sector);

   } else {
#ifndef DP_ONLY
      if (f_Command == CMD_PROBE) {
         ans = GRIB2Probe (usr, numPnts, pnts, labels, pntFiles);
         if (ans != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to GRIB2Probe.\n%s\n", msg);
            free (msg);
         }
      } else if (f_Command == CMD_DATAPROBE) {
#endif
         ans = Grib2DataProbe (usr, numPnts, pnts, labels, pntFiles);
         if (ans != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to Grib2DataProbe.\n%s\n", msg);
            free (msg);
         }
#ifndef DP_ONLY
      } else {
         ans = 0;
      }
#endif
   }

 done:
   for (i = 0; i < numPnts; i++) {
      free (labels[i]);
      free (pntFiles[i]);
   }
   free (pnts);
   free (labels);
   free (pntFiles);
   return ans;
}
