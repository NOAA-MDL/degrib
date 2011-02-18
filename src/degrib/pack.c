/*****************************************************************************
 * pack.c
 *
 * DESCRIPTION
 *    This file contains the code to handle repacking the data cube into a
 * GRIB2 message.
 *
 * HISTORY
 *   2/2004 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "myerror.h"
#include "myassert.h"
#include "meta.h"
#include "metaname.h"
#include "degrib2.h"
#include "mdl_g2c.h"
#include "tendian.h"
#include "pack.h"
#include "myutil.h"
#include "clock.h"

#ifdef FORTRAN_GRIB2
static sInt4 NearestInt (double a)
{
   return (sInt4) floor (a + .5);
}
#endif

#ifdef FORTRAN_GRIB2
/* Skipped over type 5 (Don't know WGS84 sphere) */
/* Skipped over type 4 (minEarth in mm?) */
static void InitEarth (grib_MetaData *meta, IS_dataType *is)
{
   if (meta->gds.f_sphere) {
      if (meta->gds.majEarth == (double) 6367.47) {
         is->is[3][15 - 1] = 0;
         is->is[3][16 - 1] = 0;
         is->is[3][17 - 1] = 6367470;
         is->is[3][21 - 1] = 0;
         is->is[3][22 - 1] = 0;
         is->is[3][26 - 1] = 0;
         is->is[3][27 - 1] = 0;
      } else if (meta->gds.majEarth == 6371.229) {
         is->is[3][15 - 1] = 6;
         is->is[3][16 - 1] = 0;
         is->is[3][17 - 1] = 6371229;
         is->is[3][21 - 1] = 0;
         is->is[3][22 - 1] = 0;
         is->is[3][26 - 1] = 0;
         is->is[3][27 - 1] = 0;
      } else {
         is->is[3][15 - 1] = 1;
         is->is[3][16 - 1] = 0;
         is->is[3][17 - 1] = NearestInt (meta->gds.majEarth * 1000);
         is->is[3][21 - 1] = 0;
         is->is[3][22 - 1] = 0;
         is->is[3][26 - 1] = 0;
         is->is[3][27 - 1] = 0;
      }
   } else {
      if ((meta->gds.majEarth == 6378.16) && (meta->gds.minEarth == 6356.775)) {
         is->is[3][15 - 1] = 2;
         is->is[3][16 - 1] = 0;
         is->is[3][17 - 1] = 0;
         is->is[3][21 - 1] = 0;
         is->is[3][22 - 1] = 6378160;
         is->is[3][26 - 1] = 0;
         is->is[3][27 - 1] = 6356775;
      } else {
         /* Based on Version 3, code table 7 takes m, 3 takes km */
         /* is->is[3][15 - 1] = 3; */
         is->is[3][15 - 1] = 7;
         is->is[3][16 - 1] = 0;
         is->is[3][17 - 1] = 0;
         is->is[3][21 - 1] = 0;
         is->is[3][22 - 1] = NearestInt (meta->gds.majEarth * 1000);
         is->is[3][26 - 1] = 0;
         is->is[3][27 - 1] = NearestInt (meta->gds.minEarth * 1000);
      }
   }
}
#endif

#ifdef FORTRAN_GRIB2
static void PackSect2_Wx (grib_MetaData *meta, IS_dataType *is)
{
   size_t i, j;
   int count;
   int nidat;

   /* Get a maximum estimate for how big this section is going to be. */
   nidat = 32 + meta->pds2.sect2.wx.maxLen * meta->pds2.sect2.wx.dataLen;
   if (nidat > is->nidat) {
      is->nidat = nidat;
      is->idat = (sInt4 *) realloc ((void *) is->idat,
                                    is->nidat * sizeof (sInt4));
   }
   is->idat[2 - 1] = 0; /* 0 decimal scale factor */
   count = 0;
   for (i = 0; i < meta->pds2.sect2.wx.dataLen; i++) {
      for (j = 0; j < strlen (meta->pds2.sect2.wx.data[i]); j++) {
         is->idat[3 - 1 + count] = meta->pds2.sect2.wx.data[i][j];
         count++;
      }
      if (i != meta->pds2.sect2.wx.dataLen - 1) {
         is->idat[3 - 1 + count] = '\0';
         count++;
      }
   }
   is->idat[1 - 1] = count;
   /* Following tells packer to stop packing section2 data. */
   is->idat[3 - 1 + count] = 0;
   is->rdat[0] = 0;
}
#endif

/* compute number of bits needed to store val */
static int power (uInt4 val)
{
   int i;

   if (val == 0) {
      return 1;
   }
   for (i = 0; val != 0; i++) {
      val = val >> 1;
   }
   return i;
}

void fillSect2_Wx (enGribMeta *en, grib_MetaData *meta, sChar f_mdlPack)
{
   int i;
   size_t count;
   uChar *ray;
   uChar max;
   uChar min = 0; /* Since we're including /0 characters */
   uChar *cpack;
   size_t cpackLen;
   uShort2 numGroups; /* Number of groups */
   sInt4 numVal;
   float refVal;
   uShort2 scale;
   uInt4 maxdif;
   uChar dataType;
   uChar bufLoc;
   uChar *ptr;
   size_t numUsed;
   size_t numBits;

   /* Count the number of characters needed. */
   count = 0;
   for (i = 0; i < meta->pds2.sect2.wx.dataLen; i++) {
      count += strlen (meta->pds2.sect2.wx.data[i]) + 1;
   }

   /* Set up array */
   ray = (uChar *) malloc (count * sizeof (char));
   count = 0;
   for (i = 0; i < meta->pds2.sect2.wx.dataLen; ++i) {
      memcpy (ray + count, meta->pds2.sect2.wx.data[i],
              strlen (meta->pds2.sect2.wx.data[i]) + 1);
      count += strlen (meta->pds2.sect2.wx.data[i]) + 1;
   }

   /* Check if we don't want to apply mdl_sect2Pack */
   if (! f_mdlPack) {
      fillSect2 (en, ray, count);
      free (ray);
      return;
   }

   /* Do 1 group simple case of mdl_sect2Pack */

   /* Find the max value of field */
   max = 0;
   for (i = 0; i < count; ++i) {
      if (ray[i] > max)
         max = ray[i];
   }

   /* Allocate enough space in cpack for 1 group, and unpacked ray. */
   /* The 3 is overall header space, the 12 is per group space. */
   cpackLen = count + 3 + 12;
   cpack = (uChar *) malloc (cpackLen * sizeof (char));
   cpack[0] = 1;
   numGroups = 1;
   MEMCPY_BIG ((cpack + 1), &numGroups, sizeof (uShort2));
   numVal = count;
   refVal = 0;
   scale = 0;
   MEMCPY_BIG ((cpack + 3), &numVal, sizeof (sInt4));
   MEMCPY_BIG ((cpack + 7), &refVal, sizeof (float));
   MEMCPY_BIG ((cpack + 11), &scale, sizeof (uShort2));
   maxdif = max - min;
   numBits = power (maxdif);
   cpack[13] = numBits;
   dataType = 1; /* integer */
   cpack[14] = dataType;

   ptr = cpack + 15;
   /* make sure ptr is init to 0 before memBitWrite() calls */
   memset (ptr, 0, count);
   count = 15;
   bufLoc = 8;
   for (i = 0; i < numVal; i++) {
      memBitWrite (&(ray[i]), sizeof (char), ptr, numBits, &bufLoc, &numUsed);
      ptr += numUsed;
      count += numUsed;
   }
   /* Make sure "remainder byte is stored. */
   if (bufLoc != 8) {
      ptr ++;
      count ++;
   }
   fillSect2 (en, cpack, count);
   free (ray);
   free (cpack);
}

/*
meta->pds2.prodType,    meta->pds2.center,     meta->pds2.subcenter,
meta->pds2.mstrVersion, meta->pds2.lclVersion, meta->pds2.sigTime,
meta->pds2.refTime,     meta->pds2.operStatus, meta->pds2.dataType

meta->gds.f_sphere      meta->gds.majEarth      meta->gds.minEarth
meta->gds.numPts,       meta->gds.projType
meta->gds.Nx            meta->gds.Ny            meta->gds.lat1
meta->gds.lon1          meta->gds.resFlag       meta->gds.lat2
meta->gds.lon2          meta->gds.Dx            meta->gds.Dy
meta->gds.scan          meta->gds.meshLat       meta->gds.orientLon
meta->gds.center        meta->gds.scaleLat1     meta->gds.scaleLat2
meta->gds.southLat      meta->gds.southLon

meta->pds2.sect4.templat     meta->pds2.sect4.cat   meta->pds2.sect4.subcat
meta->pds2.sect4.genProcess  meta->pds2.sect4.genID meta->pds2.sect4.numBands
meta->pds2.sect4.numBands

*/

#ifdef FORTRAN_GRIB2

/*****************************************************************************
 * getCodedTime() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Change from seconds to the time units provided in timeCode.
 *
 * ARGUMENTS
 * timeCode = The time units to convert into (see code table 4.4). (Input)
 *     time = The time in seconds to convert. (Input)
 *      ans = The converted answer. (Output)
 *
 * RETURNS: int
 *  0 = OK
 * -1 = could not determine.
 *
 *  4/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int getCodedTime(uChar timeCode, double time, sInt4 *ans)
{
   /* Following is a lookup table for unit conversion (see code table 4.4). */
   static sInt4 unit2sec[] = {
      60, 3600, 86400L, 0, 0,
      0, 0, 0, 0, 0,
      10800, 21600L, 43200L, 1
   };

   if (timeCode < 14) {
      if (unit2sec[timeCode] != 0) {
         *ans = NearestInt(time / unit2sec[timeCode]);
         return 0;
      }
   }
   *ans = 0;
   return -1;
}

int WriteGrib2Record (grib_MetaData *meta, double *Grib_Data,
                      sInt4 grib_DataLen, IS_dataType *is, sChar f_unit,
                      uChar **cPack, sInt4 *c_len, uChar f_stdout)
{
   sInt4 kfildo = 20;   /* Set kfildo to random number say 20. */
   sInt4 nx, ny;
   sInt4 ibitmap = 0;   /* 0 means no bitmap */
   sInt4 iclean = 0;    /* 0 if missing data in grid... 1 if it is clean. */
   float xmissp, xmisss; /* primary and secondary missing values. */
   sInt4 missp, misss;
   sInt4 inew = 1;
   sInt4 minpk = 14;
   sInt4 ndjer = 22;    /* number of error messages allowed. */
   sInt4 jer[22 * 2];
   sInt4 kjer;
   int i;
   sInt4 l3264b;
   sInt4 nd5;
   sInt4 *ipack;
/*   struct tm *tempTime;*/
   sInt4 year;
   int month, day, hour, min;
   double sec;
   double unit;         /* Used to convert from stored value to degrees
                         * lat/lon. See GRIB2 Regulation 92.1.6 */
   double value;        /* The data in the non-GRIB2 units. */
   double unitM, unitB; /* values in y = m x + b used for unit conversion. */
   char unitName[15];   /* Holds the string name of the current unit. */
   float *ain;
   /* Local copy of section lengths. */
/*   sInt4 local_ns[];= {16, 21, 7, 96, 60, 49, 6, 8};*/
   sInt4 local_ns[] = { 16, 21, 7, 96, 130, 49, 6, 8 };

   if (sizeof (sInt4) == 4) {
      l3264b = 32;
   } else if (sizeof (sInt4) == 8) {
      l3264b = 64;
   } else {
      errSprintf ("Error... neither 4 byte or 8 byte sInt4s?\n");
      return -1;
   }

   xmissp = meta->gridAttrib.missPri;
   xmisss = meta->gridAttrib.missSec;
   missp = (sInt4) meta->gridAttrib.missPri;
   misss = (sInt4) meta->gridAttrib.missSec;
   nx = meta->gds.Nx;
   ny = meta->gds.Ny;

   for (i = 0; i < 7; i++) {
      if (local_ns[i] > is->ns[i]) {
         is->ns[i] = local_ns[i];
         is->is[i] = (sInt4 *) realloc ((void *) (is->is[i]),
                                        is->ns[i] * sizeof (sInt4));
      }
   }

   /* 1196575042L is GRIB as a sInt4. */
/*   is->is[0][1-1] = 1196575042L;*/
   is->is[0][7 - 1] = meta->pds2.prodType;
/*   is->is[0][8-1] = meta->GribVersion;*/

   is->is[1][5 - 1] = 1;
   is->is[1][6 - 1] = meta->center;
   is->is[1][8 - 1] = meta->subcenter;
   is->is[1][10 - 1] = meta->pds2.mstrVersion;
   is->is[1][11 - 1] = meta->pds2.lclVersion;
   is->is[1][12 - 1] = meta->pds2.sigTime;
/*   tempTime = gmtime (&(meta->pds2.refTime));
   is->is[1][13 - 1] = tempTime->tm_year + 1900;
   is->is[1][15 - 1] = tempTime->tm_mon + 1;
   is->is[1][16 - 1] = tempTime->tm_mday;
   is->is[1][17 - 1] = tempTime->tm_hour;
   is->is[1][18 - 1] = tempTime->tm_min;
   is->is[1][19 - 1] = tempTime->tm_sec;
*/
   Clock_PrintDate (meta->pds2.refTime, &year, &month, &day, &hour, &min,
                    &sec);
   is->is[1][13 - 1] = year;
   is->is[1][15 - 1] = month;
   is->is[1][16 - 1] = day;
   is->is[1][17 - 1] = hour;
   is->is[1][18 - 1] = min;
   is->is[1][19 - 1] = (sInt4) sec;

   is->is[1][20 - 1] = meta->pds2.operStatus;
   is->is[1][21 - 1] = meta->pds2.dataType;

   if (10 > is->nrdat) {
      is->nrdat = 10;
      is->rdat = (float *) realloc ((void *) is->rdat,
                                    is->nrdat * sizeof (float));
   }
   is->rdat[0] = 0;
   if (meta->pds2.f_sect2) {
      if (meta->pds2.sect2.ptrType == GS2_WXTYPE) {
         /* PackSect2_Wx adjusts the size of is->idat if needed. */
         PackSect2_Wx (meta, is);
      } else {
         errSprintf ("ERROR (pack.c): Don't handle this yet\n");
         return -3;
      }
   } else {
      if (10 > is->nidat) {
         is->nidat = 10;
         is->idat = (sInt4 *) realloc ((void *) is->idat,
                                       is->nidat * sizeof (sInt4));
      }
      is->idat[0] = 0;
   }

   is->is[3][5 - 1] = 3;
   is->is[3][6 - 1] = 0; /* 0 is GDS included, 1 is it is predefined. */
   is->is[3][7 - 1] = meta->gds.numPts;
   is->is[3][11 - 1] = 0; /* # octets for extra info */
   is->is[3][12 - 1] = 0; /* interpretation of extra info */
   is->is[3][13 - 1] = meta->gds.projType;
   unit = pow (10, 6);
   switch (meta->gds.projType) {
      case GS3_LATLON:
         InitEarth (meta, is);
         is->is[3][31 - 1] = meta->gds.Nx;
         is->is[3][35 - 1] = meta->gds.Ny;
         is->is[3][39 - 1] = 0; /* Angle */
         is->is[3][43 - 1] = 0; /* subdivision */
         is->is[3][47 - 1] = NearestInt (meta->gds.lat1 * unit);
         is->is[3][51 - 1] = NearestInt (meta->gds.lon1 * unit);
         is->is[3][55 - 1] = meta->gds.resFlag;
         is->is[3][56 - 1] = NearestInt (meta->gds.lat2 * unit);
         is->is[3][60 - 1] = NearestInt (meta->gds.lon2 * unit);
         is->is[3][64 - 1] = NearestInt (meta->gds.Dx * unit); /* degrees. */
         is->is[3][68 - 1] = NearestInt (meta->gds.Dy * unit); /* degrees. */
         is->is[3][72 - 1] = meta->gds.scan;
         break;
      case GS3_MERCATOR:
         InitEarth (meta, is);
         is->is[3][31 - 1] = meta->gds.Nx;
         is->is[3][35 - 1] = meta->gds.Ny;
         is->is[3][39 - 1] = NearestInt (meta->gds.lat1 * unit);
         is->is[3][43 - 1] = NearestInt (meta->gds.lon1 * unit);
         is->is[3][47 - 1] = meta->gds.resFlag;
         is->is[3][48 - 1] = NearestInt (meta->gds.meshLat * unit);
         is->is[3][52 - 1] = NearestInt (meta->gds.lat2 * unit);
         is->is[3][56 - 1] = NearestInt (meta->gds.lon2 * unit);
         is->is[3][60 - 1] = meta->gds.scan;
         is->is[3][61 - 1] = NearestInt (meta->gds.orientLon * unit);
         is->is[3][65 - 1] = NearestInt (meta->gds.Dx * 1000.); /* m->mm */
         is->is[3][69 - 1] = NearestInt (meta->gds.Dy * 1000.); /* m->mm */
         break;
      case GS3_POLAR:
         InitEarth (meta, is);
         is->is[3][31 - 1] = meta->gds.Nx;
         is->is[3][35 - 1] = meta->gds.Ny;
         is->is[3][39 - 1] = NearestInt (meta->gds.lat1 * unit);
         is->is[3][43 - 1] = NearestInt (meta->gds.lon1 * unit);
         is->is[3][47 - 1] = meta->gds.resFlag;
         is->is[3][48 - 1] = NearestInt (meta->gds.meshLat * unit);
         is->is[3][52 - 1] = NearestInt (meta->gds.orientLon * unit);
         is->is[3][56 - 1] = NearestInt (meta->gds.Dx * 1000.); /* m->mm */
         is->is[3][60 - 1] = NearestInt (meta->gds.Dy * 1000.); /* m->mm */
         is->is[3][64 - 1] = meta->gds.center;
         is->is[3][65 - 1] = meta->gds.scan;
         break;
      case GS3_LAMBERT:
         InitEarth (meta, is);
         is->is[3][31 - 1] = meta->gds.Nx;
         is->is[3][35 - 1] = meta->gds.Ny;
         is->is[3][39 - 1] = NearestInt (meta->gds.lat1 * unit);
         is->is[3][43 - 1] = NearestInt (meta->gds.lon1 * unit);
         is->is[3][47 - 1] = meta->gds.resFlag;
         is->is[3][48 - 1] = NearestInt (meta->gds.meshLat * unit);
         is->is[3][52 - 1] = NearestInt (meta->gds.orientLon * unit);
         is->is[3][56 - 1] = NearestInt (meta->gds.Dx * 1000.); /* m->mm */
         is->is[3][60 - 1] = NearestInt (meta->gds.Dy * 1000.); /* m->mm */
         is->is[3][64 - 1] = meta->gds.center;
         is->is[3][65 - 1] = meta->gds.scan;
         is->is[3][66 - 1] = NearestInt (meta->gds.scaleLat1 * unit);
         is->is[3][70 - 1] = NearestInt (meta->gds.scaleLat2 * unit);
         is->is[3][74 - 1] = NearestInt (meta->gds.southLat * unit);
         is->is[3][78 - 1] = NearestInt (meta->gds.southLon * unit);
         break;
      default:
         errSprintf ("Un-supported Map Projection. %ld\n",
                     meta->gds.projType);
         return -1;
   }

   is->is[4][5 - 1] = 4;
   is->is[4][6 - 1] = 0;
   is->is[4][8 - 1] = meta->pds2.sect4.templat;
   is->is[4][10 - 1] = meta->pds2.sect4.cat;
   is->is[4][11 - 1] = meta->pds2.sect4.subcat;
   is->is[4][12 - 1] = meta->pds2.sect4.genProcess;
   if (meta->pds2.sect4.templat == GS4_SATELLITE) {
      is->is[4][13 - 1] = meta->pds2.sect4.genID;
      is->is[4][14 - 1] = meta->pds2.sect4.numBands;
      for (i = 0; i < meta->pds2.sect4.numBands; i++) {
         is->is[4][14 + 10 * i] = meta->pds2.sect4.bands[i].series;
         is->is[4][16 + 10 * i] = meta->pds2.sect4.bands[i].numbers;
         is->is[4][18 + 10 * i] =
               meta->pds2.sect4.bands[i].centWaveNum.factor;
         is->is[4][19 + 10 * i] = meta->pds2.sect4.bands[i].centWaveNum.value;
      }
   } else {
      is->is[4][13 - 1] = meta->pds2.sect4.bgGenID;
      is->is[4][14 - 1] = meta->pds2.sect4.genID;
      if (meta->pds2.sect4.f_validCutOff) {
         is->is[4][15 - 1] = NearestInt (meta->pds2.sect4.cutOff / 3600);
         is->is[4][17 - 1] = NearestInt ((meta->pds2.sect4.cutOff % 3600) /
                                         60);
      } else {
         is->is[4][15 - 1] = GRIB2MISSING_u2;
         is->is[4][17 - 1] = GRIB2MISSING_u1;
      }
      is->is[4][18 - 1] = meta->pds2.sect4.foreUnit;
      getCodedTime(meta->pds2.sect4.foreUnit, meta->pds2.sect4.foreSec,
                   &(is->is[4][19 - 1]));
      is->is[4][23 - 1] = meta->pds2.sect4.fstSurfType;
      if (meta->pds2.sect4.fstSurfScale == GRIB2MISSING_s1) {
         is->is[4][24 - 1] = GRIB2MISSING_s4;
         is->is[4][25 - 1] = GRIB2MISSING_s4;
      } else {
         is->is[4][24 - 1] = meta->pds2.sect4.fstSurfScale;
         is->is[4][25 - 1] = (sInt4) (meta->pds2.sect4.fstSurfValue *
                                      pow (10,
                                           meta->pds2.sect4.fstSurfScale));
      }
      is->is[4][29 - 1] = meta->pds2.sect4.sndSurfType;
      if (meta->pds2.sect4.sndSurfScale == GRIB2MISSING_s1) {
         is->is[4][30 - 1] = GRIB2MISSING_s4;
         is->is[4][31 - 1] = GRIB2MISSING_s4;
      } else {
         is->is[4][30 - 1] = meta->pds2.sect4.sndSurfScale;
         is->is[4][31 - 1] = (sInt4) (meta->pds2.sect4.sndSurfValue *
                                      pow (10,
                                           meta->pds2.sect4.sndSurfScale));
      }
      switch (meta->pds2.sect4.templat) {
         case GS4_ANALYSIS: /* 4.0 */
         case GS4_ERROR: /* 4.7 */
            break;
         case GS4_ENSEMBLE: /* 4.1 */
            is->is[4][35 - 1] = meta->pds2.sect4.typeEnsemble;
            is->is[4][36 - 1] = meta->pds2.sect4.perturbNum;
            is->is[4][37 - 1] = meta->pds2.sect4.numberFcsts;
            break;
         case GS4_DERIVED: /* 4.2 */
            is->is[4][35 - 1] = meta->pds2.sect4.derivedFcst;
            is->is[4][36 - 1] = meta->pds2.sect4.numberFcsts;
            break;
         case GS4_STATISTIC: /* 4.8 */
            /* tempTime = gmtime (&(meta->pds2.sect4.validTime));
             * is->is[4][35 - 1] = tempTime->tm_year + 1900; is->is[4][37 -
             * 1] = tempTime->tm_mon + 1; is->is[4][38 - 1] =
             * tempTime->tm_mday; is->is[4][39 - 1] = tempTime->tm_hour;
             * is->is[4][40 - 1] = tempTime->tm_min; is->is[4][41 - 1] =
             * tempTime->tm_sec; */
            Clock_PrintDate (meta->pds2.sect4.validTime, &year, &month, &day,
                             &hour, &min, &sec);
            is->is[4][35 - 1] = year;
            is->is[4][37 - 1] = month;
            is->is[4][38 - 1] = day;
            is->is[4][39 - 1] = hour;
            is->is[4][40 - 1] = min;
            is->is[4][41 - 1] = (sInt4) sec;

            is->is[4][42 - 1] = meta->pds2.sect4.numInterval;
            is->is[4][43 - 1] = meta->pds2.sect4.numMissing;
            for (i = 0; i < meta->pds2.sect4.numInterval; i++) {
               is->is[4][46 + i * 12] =
                     meta->pds2.sect4.Interval[i].processID;
               is->is[4][47 + i * 12] = meta->pds2.sect4.Interval[i].incrType;
               is->is[4][48 + i * 12] =
                     meta->pds2.sect4.Interval[i].timeRangeUnit;
               is->is[4][49 + i * 12] = meta->pds2.sect4.Interval[i].lenTime;
               is->is[4][53 + i * 12] = meta->pds2.sect4.Interval[i].incrUnit;
               is->is[4][54 + i * 12] = meta->pds2.sect4.Interval[i].timeIncr;
            }
            break;
         case GS4_PROBABIL_PNT: /* 4.5 */
            is->is[4][35 - 1] = meta->pds2.sect4.foreProbNum;
            is->is[4][36 - 1] = meta->pds2.sect4.numForeProbs;
            is->is[4][37 - 1] = meta->pds2.sect4.probType;
            is->is[4][38 - 1] = meta->pds2.sect4.lowerLimit.factor;
            is->is[4][39 - 1] = meta->pds2.sect4.lowerLimit.value;
            is->is[4][43 - 1] = meta->pds2.sect4.upperLimit.factor;
            is->is[4][44 - 1] = meta->pds2.sect4.upperLimit.value;
            break;
         case GS4_PROBABIL_TIME: /* 4.9 */
            is->is[4][35 - 1] = meta->pds2.sect4.foreProbNum;
            is->is[4][36 - 1] = meta->pds2.sect4.numForeProbs;
            is->is[4][37 - 1] = meta->pds2.sect4.probType;
            is->is[4][38 - 1] = meta->pds2.sect4.lowerLimit.factor;
            is->is[4][39 - 1] = meta->pds2.sect4.lowerLimit.value;
            is->is[4][43 - 1] = meta->pds2.sect4.upperLimit.factor;
            is->is[4][44 - 1] = meta->pds2.sect4.upperLimit.value;
            /* tempTime = gmtime (&(meta->pds2.sect4.validTime));
             * is->is[4][48 - 1] = tempTime->tm_year + 1900; is->is[4][50 -
             * 1] = tempTime->tm_mon + 1; is->is[4][51 - 1] =
             * tempTime->tm_mday; is->is[4][52 - 1] = tempTime->tm_hour;
             * is->is[4][53 - 1] = tempTime->tm_min; is->is[4][54 - 1] =
             * tempTime->tm_sec; */
            Clock_PrintDate (meta->pds2.sect4.validTime, &year, &month, &day,
                             &hour, &min, &sec);
            is->is[4][48 - 1] = year;
            is->is[4][50 - 1] = month;
            is->is[4][51 - 1] = day;
            is->is[4][52 - 1] = hour;
            is->is[4][53 - 1] = min;
            is->is[4][54 - 1] = (sInt4) sec;

            is->is[4][55 - 1] = meta->pds2.sect4.numInterval;
            is->is[4][56 - 1] = meta->pds2.sect4.numMissing;
            for (i = 0; i < meta->pds2.sect4.numInterval; i++) {
               is->is[4][59 + i * 12] =
                     meta->pds2.sect4.Interval[i].processID;
               is->is[4][60 + i * 12] = meta->pds2.sect4.Interval[i].incrType;
               is->is[4][61 + i * 12] =
                     meta->pds2.sect4.Interval[i].timeRangeUnit;
               is->is[4][62 + i * 12] = meta->pds2.sect4.Interval[i].lenTime;
               is->is[4][66 + i * 12] = meta->pds2.sect4.Interval[i].incrUnit;
               is->is[4][67 + i * 12] = meta->pds2.sect4.Interval[i].timeIncr;
            }
            break;
         default:
            errSprintf ("Un-supported Template. %ld\n",
                        meta->pds2.sect4.templat);
            return -1;
      }
   }

   if ((meta->gridAttrib.packType != GS5_SIMPLE) &&
       (meta->gridAttrib.packType != GS5_CMPLX) &&
       (meta->gridAttrib.packType != GS5_CMPLXSEC) &&
       (meta->gridAttrib.packType != GS5_SPECTRAL) &&
       (meta->gridAttrib.packType != GS5_HARMONIC)) {
      errSprintf ("Un-supported Packing method %ld\n",
                  meta->gridAttrib.packType);
      return -1;
   }

   for (i = 0; i < is->ns[5]; i++) {
      is->is[5][i] = 0;
   }

   is->is[5][5 - 1] = 5;
   is->is[5][10 - 1] = meta->gridAttrib.packType;
   is->is[5][16 - 1] = meta->gridAttrib.ESF;
   is->is[5][18 - 1] = meta->gridAttrib.DSF;
   if ((meta->gridAttrib.packType != GS5_SPECTRAL) &&
       (meta->gridAttrib.packType != GS5_HARMONIC)) {
      is->is[5][21 - 1] = meta->gridAttrib.fieldType;
      if (meta->gridAttrib.packType != GS5_SIMPLE) {
         is->is[5][22 - 1] = 1; /* General Group Splitting? */
         is->is[5][23 - 1] = meta->gridAttrib.f_miss;
      }
   }

   is->is[6][5 - 1] = 6;

   is->is[7][5 - 1] = 7;

   if (is->nd2x3 < nx * ny) {
      is->nd2x3 = nx * ny;
      is->iain = (sInt4 *) realloc ((void *) is->iain,
                                    is->nd2x3 * sizeof (sInt4));
      is->ib = (sInt4 *) realloc ((void *) is->ib,
                                  is->nd2x3 * sizeof (sInt4));
   }

   ain = (float *) (is->iain);

   ComputeUnit (meta->convert, meta->unitName, f_unit, &unitM, &unitB,
                unitName);

   /* check if data was a float. */
   if (meta->gridAttrib.fieldType == 0) {
      for (i = 0; i < grib_DataLen; i++) {
         value = Grib_Data[i];
         if ((value == meta->gridAttrib.missPri) ||
             (value == meta->gridAttrib.missSec)) {
            ain[i] = value;
         } else {
            /* convert the units. */
            if (unitM == -10) {
               ain[i] = (float) (myRound (log10 (value), 7));
            } else {
               ain[i] = (float) (myRound ((value - unitB) / unitM, 7));
            }
         }
      }
   } else {
      for (i = 0; i < grib_DataLen; i++) {
         value = Grib_Data[i];
         if ((value == meta->gridAttrib.missPri) ||
             (value == meta->gridAttrib.missSec)) {
            is->iain[i] = (sInt4) value;
         } else {
            /* convert the units. */
            if (unitM == -10) {
               is->iain[i] = (sInt4) (myRound (log10 (value), 0));
            } else {
               is->iain[i] = (sInt4) (myRound ((value - unitB) / unitM, 0));
            }
         }
      }
   }

   /* 4000 is number of bytes for local use data. Not necessarily correct. */
   nd5 = 250 + (nx * ny) + (nx * ny) / 8 + 4000;
   ipack = (sInt4 *) malloc (nd5 * sizeof (sInt4));

   /* Warning messages of "909" can occur if the user claimed missing
    * data, but the packer didn't find any, so the packer reset
    * is5[23] accordingly */
   pk_grib2 (&kfildo, ain, is->iain, &nx, &ny, is->idat, &(is->nidat),
             is->rdat, &(is->nrdat), is->is[0], &(is->ns[0]),
             is->is[1], &(is->ns[1]), is->is[3], &(is->ns[3]),
             is->is[4], &(is->ns[4]), is->is[5], &(is->ns[5]),
             is->is[6], &(is->ns[6]), is->is[7], &(is->ns[7]),
             is->ib, &ibitmap, ipack, &nd5, &missp, &xmissp, &misss,
             &xmisss, &inew, &minpk, &iclean, &l3264b, jer, &ndjer, &kjer);


   for (i = 0; i < kjer; i++) {
      if (jer[ndjer + i] == 0) {
         /* no error. */
      } else if (jer[ndjer + i] == 1) {
         /* Warning. */
#ifdef DEBUG
         if (!f_stdout) {
            printf ("Warning: Pack library warning code (%ld %ld)\n", jer[i],
                    jer[ndjer + i]);
         }
#endif
      } else {
         /* BAD Error. */
         errSprintf ("ERROR: Pack library error code (%ld %ld)\n",
                     jer[i], jer[ndjer + i]);

         *cPack = (uChar *) ipack;
         *c_len = ipack[3];
         return -3;
      }
   }

   /* figure out end of message. */
   *cPack = (uChar *) ipack;
   *c_len = ipack[3];

   /* mem swap if need be. */
#ifdef LITTLE_ENDIAN
   memswp (ipack, sizeof (sInt4), nd5);
#endif
   return 0;
}
#endif

/*
 * uChar processID     = Statistical process method used.
 * uChar incrType      = Type of time increment between intervals
 * uChar timeRangeUnit = Time range unit. [Code Table 4.4]
 * sInt4 lenTime       = Range or length of time interval.
 * uChar incrUnit      = Unit of time increment. [Code Table 4.4]
 * sInt4 timeIncr      = Time increment between intervals.
 */
void fillSect4_Interval (sect4IntervalType * val, uChar processID,
                         uChar incrType, uChar timeRangeUnit, sInt4 lenTime,
                         uChar incrUnit, sInt4 timeIncr)
{
   val->processID = processID;
   val->incrType = incrType;
   val->timeRangeUnit = timeRangeUnit;
   val->lenTime = lenTime;
   val->incrUnit = incrUnit;
   val->timeIncr = timeIncr;
}

/*****************************************************************************
 * fillGridUnit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Completes the data portion.  If f_boustify, then it walks through the
 * data winding back and forth.  Note it does this in a row oriented fashion
 * If you need a column oriented fashion because your grid is defined the
 * other way, then swap your Nx and Ny in your call.
 *    It also detects if the missing value is used.  If it is not, it sets
 * the correct portion of section 5
 *
 * ARGUMENTS
 *         en = A pointer to meta data to pass to GRIB2 encoder. (Output)
 *       data = Data array to add. (Input)
 *    lenData = Length of Data array. (Input)
 *         Nx = Number of X coordinates (Input)
 *         Ny = Number of Y coordinates (Input)
 *      ibmap = [Code 6.0] Bitmap indicator (Input)
 *              0 = bitmap applies and is included in Section 6.
 *              1-253 = Predefined bitmap applies
 *              254 = Previously defined bitmap applies to this field
 *              255 = Bit map does not apply to this product.
 * f_boustify = true if we should re-Wrap the grid. (Input)
 *     f_miss = 1 if missPri valid, 2 if missSec valid. (Input)
 *    missPri = Primary missing value (Input)
 *    missSec = Secondary missing value (Input)
 *      unitM = -10 => unit conversion by log10(value), otherwise the slope of
 *              unit conversion formula
 *      unitB = The rise of the unit conversion formula
 *
 * RETURNS: int
 *    > 0 (max length of sect 6 and sect 7).
 *    -1 Can't handle this kind of bitmap (pre-defined).
 *    -2 No missing value when trying to create the bmap.
 *    -3 Can't handle Nx * Ny != lenData.
 *    -4 Discrepancy in what has been stored in section 5.
 *
 *  4/2006 Arthur Taylor (MDL): Created.
 *  5/2007 Arthur Taylor (MDL): Added unitM, unitB to call
 *
 * NOTES:
 *****************************************************************************
 */
int fillGridUnit (enGribMeta *en, double *data, sInt4 lenData, sInt4 Nx, sInt4 Ny,
                  sInt4 ibmap, sChar f_boustify, uChar fieldType, uChar f_miss,
                  double missPri, double missSec, double unitM, double unitB)
{
   uChar f_flip;        /* Used to help keep track of the direction when
                         * "boustifying" the data. */
   sInt4 x;             /* loop counter over Nx. */
   sInt4 y;             /* loop counter over Ny. */
   sInt4 ind1;          /* index to copy to. */
   sInt4 ind2;          /* index to copy from. */
   uChar f_detectMiss = 0; /* If we detect a missing value. */
   float enMissPri;     /* encoded primary missing value. */
   float enMissSec;     /* encoded secondary missing value. */
   int ans;             /* computed max length of sect6, sect7 */

   if ((ibmap != 0) && (ibmap != 255)) {
      /* Can't handle this kind of bitmap (pre-defined). */
      return -1;
   }
   if ((ibmap == 0) && (f_miss != 1) && (f_miss != 2)) {
      /* No missing value when trying to create the bmap. */
      return -2;
   }
   if (Nx * Ny != lenData) {
      /* Can't handle Nx * Ny != lenData. */
      return -3;
   }
   if ((en->idrsnum == 2) || (en->idrsnum == 3)) {
      /* Discrepancy in what has been stored in Sect5 */
      if (en->drsTmpl[6] != f_miss) {
         return -4;
      }
      if (f_miss == 1) {
         if (fieldType == 1) {
            enMissPri = en->drsTmpl[7];
         } else {
            memcpy (&(enMissPri), &(en->drsTmpl[7]), sizeof (float));
         }
         if (enMissPri != (float) missPri) {
            return -4;
         }
      } else if (f_miss == 2) {
         if (fieldType == 1) {
            enMissPri = en->drsTmpl[7];
            enMissSec = en->drsTmpl[8];
         } else {
            memcpy (&(enMissPri), &(en->drsTmpl[7]), sizeof (float));
            memcpy (&(enMissSec), &(en->drsTmpl[8]), sizeof (float));
         }
         if ((enMissPri != (float)missPri) || (enMissSec != (float)missSec)) {
            return -4;
         }
      }
   }

   if (en->ngrdpts < lenData) {
      if (en->fld != NULL) {
         free (en->fld);
      }
      en->fld = (float *) malloc (lenData * sizeof (float));
      if (ibmap == 0) {
         if (en->bmap != NULL) {
            free (en->bmap);
         }
         en->bmap = (sInt4 *) malloc (lenData * sizeof (sInt4));
      }
   }
   en->ngrdpts = lenData;
   en->ibmap = ibmap;

   /* Now need to walk over data and boustify it and create bmap. */

   if (ibmap == 0) {
      /* boustify uses row oriented boustification, however for column
       * oriented, swap the Ny and Nx in the call to the procedure. */
      if (f_boustify) {
         f_flip = 0;
         for (y = 0; y < Ny; y++) {
            for (x = 0; x < Nx; x++) {
               ind1 = x + y * Nx;
               if (!f_flip) {
                  ind2 = ind1;
               } else {
                  ind2 = (Nx - x - 1) + y * Nx;
               }
               if ((data[ind2] == missPri) ||
                   ((f_miss == 2) && (data[ind2] == missSec))) {
                  en->bmap[ind1] = 0;
                  en->fld[ind1] = (float) data[ind2];
                  f_detectMiss = 1;
               } else {
                  en->bmap[ind1] = 1;
                  if (unitM == -10) {
                     en->fld[ind1] = (float) myRound (log10 (data[ind2]), 7);
                  } else {
                     en->fld[ind1] = (float) myRound (((data[ind2] - unitB) / unitM), 7);
                  }
               }
            }
            f_flip = (!f_flip);
         }
      } else {
         for (ind1 = 0; ind1 < lenData; ind1++) {
            if ((data[ind1] == missPri) ||
                ((f_miss == 2) && (data[ind1] == missSec))) {
               en->bmap[ind1] = 0;
               en->fld[ind1] = (float) data[ind1];
               f_detectMiss = 1;
            } else {
               en->bmap[ind1] = 1;
               if (unitM == -10) {
                  en->fld[ind1] = (float) myRound (log10 (data[ind1]), 7);
               } else {
                  en->fld[ind1] = (float) myRound (((data[ind1] - unitB) / unitM), 7);
               }
            }
         }
      }
      /* len(sect6) < 6 + (lenData/8 + 1), len(sect7) < 5 + lenData * 4 */
      ans = (6 + lenData / 8 + 1) + (5 + lenData * 4);
   } else {
      /* boustify uses row oriented boustification, however for column
       * oriented, swap the Ny and Nx in the call to the procedure. */
      if (f_boustify) {
         f_flip = 0;
         for (y = 0; y < Ny; y++) {
            for (x = 0; x < Nx; x++) {
               ind1 = x + y * Nx;
               if (!f_flip) {
                  ind2 = ind1;
               } else {
                  ind2 = (Nx - x - 1) + y * Nx;
               }
               if ((data[ind2] == missPri) ||
                   ((f_miss == 2) && (data[ind2] == missSec))) {
                  en->fld[ind1] = (float) data[ind2];
                  f_detectMiss = 1;
               } else {
                  if (unitM == -10) {
                     en->fld[ind1] = (float) myRound (log10 (data[ind2]), 7);
                  } else {
                     en->fld[ind1] = (float) myRound (((data[ind2] - unitB) / unitM), 7);
                  }
               }
            }
            f_flip = (!f_flip);
         }
      } else {
         for (ind1 = 0; ind1 < lenData; ind1++) {
            if ((data[ind1] == missPri) ||
                ((f_miss == 2) && (data[ind1] == missSec))) {
               en->fld[ind1] = (float) data[ind1];
               f_detectMiss = 1;
            } else {
               if (unitM == -10) {
                  en->fld[ind1] = (float) myRound (log10 (data[ind1]), 7);
               } else {
                  en->fld[ind1] = (float) myRound (((data[ind1] - unitB) / unitM), 7);
               }
            }
         }
      }
      /* len(sect6) = 6, len(sect7) < 5 + lenData * 4 */
      ans = 6 + (5 + lenData * 4);
   }
   /* For tmplNum 2,3 we stored the missing value in the drsTmpl.  We check
    * here to see if we ever used the missing value.  If it wasn't used, there
    * isn't any point in hurting the packing algorithm, so we update drsTmpl.
    */
   if ((en->idrsnum == 2) || (en->idrsnum == 3)) {
      if ((f_miss != 0) && (f_detectMiss == 0)) {
         en->drsTmpl[6] = 0;
      }
   }
   return ans;
}

/* #define TESTING*/
int WriteGrib2Record2 (grib_MetaData *meta, double *Grib_Data,
                       sInt4 grib_DataLen, IS_dataType *is, sChar f_unit,
                       uChar **cPack, sInt4 *c_len, uChar f_stdout)
{
   enGribMeta en;
   sInt4 cgribLen;
   sInt4 year;
   int month;
   int day;
   int hour;
   int min;
   double sec;
   int ans;
#ifdef TESTING
   int ans2;
   uChar *cPack2;
#endif
   sect4IntervalType * interval;
   uShort2 tmplNum;
   int i;
   double dlowVal;
   double dupVal;
   double unitM, unitB; /* values in y = m x + b used for unit conversion. */
   char unitName[15];   /* Holds the string name of the current unit. */
   uChar scanFlag;
   sChar f_boustify;

   initEnGribMeta (&en);

   fillSect0 (&en, meta->pds2.prodType);

   Clock_PrintDate (meta->pds2.refTime, &year, &month, &day, &hour, &min,
                    &sec);
   fillSect1 (&en, meta->center, meta->subcenter, meta->pds2.mstrVersion,
              meta->pds2.lclVersion, meta->pds2.sigTime, year, month, day,
              hour, min, sec, meta->pds2.operStatus, meta->pds2.dataType);
   /* length based on section 0 and 1. */
   cgribLen = 16 + 21;

   if (meta->pds2.f_sect2) {
      if (meta->pds2.sect2.ptrType == GS2_WXTYPE) {
         /* The 1 is because we want to use the MDL Sect2 SimpPack method */
         fillSect2_Wx (&en, meta, 1);
      } else {
         printf ("ERROR (pack.c): Don't handle this yet\n");
         return -3;
      }
   } else {
      /* No section 2 data */
      fillSect2 (&en, NULL, 0);
   }
   /* length based on section 2. */
   cgribLen += 5 + en.lenSec2;

   /* The user passed in a scan mode of 64 of their data.  We want to
    * boustify this for them.  Presumably we should allow the user control of
    * this in the future. */
   scanFlag = meta->gds.scan;
   if (scanFlag == 64) {
      scanFlag = 80;
      f_boustify = 1;
   } else {
      f_boustify = 0;
   }
   /* angle and subdivision are not tracked in metaparse.c.
    * This should not be angleRotate because that has a different
    * meaning entirely. */
   ans = fillSect3 (&en, meta->gds.projType, meta->gds.majEarth,
                    meta->gds.minEarth, meta->gds.Nx, meta->gds.Ny,
                    meta->gds.lat1, meta->gds.lon1, meta->gds.lat2,
                    meta->gds.lon2, meta->gds.Dx, meta->gds.Dy,
                    meta->gds.resFlag, scanFlag, meta->gds.center,
                    0, 0, meta->gds.meshLat,
                    meta->gds.orientLon, meta->gds.scaleLat1,
                    meta->gds.scaleLat2, meta->gds.southLat,
                    meta->gds.southLon);
   if (ans < 0) {
      freeEnGribMeta (&en);
      printf ("Problems packing section 3\n");
      return 1;
   }
   cgribLen += ans;

   tmplNum = meta->pds2.sect4.templat;
   /* Fill out section 4.0 info which is common to the following templates */
   if ((tmplNum == 0) || (tmplNum == 1) || (tmplNum == 2) || (tmplNum == 5) ||
       (tmplNum == 8) || (tmplNum == 9) || (tmplNum == 10) ||
       (tmplNum == 12)) {
      ans = fillSect4_0 (&en, tmplNum, meta->pds2.sect4.cat,
                         meta->pds2.sect4.subcat, meta->pds2.sect4.genProcess,
                         meta->pds2.sect4.bgGenID, meta->pds2.sect4.genID,
                         meta->pds2.sect4.f_validCutOff,
                         meta->pds2.sect4.cutOff, meta->pds2.sect4.foreUnit,
                         meta->pds2.sect4.foreSec, meta->pds2.sect4.fstSurfType,
                         meta->pds2.sect4.fstSurfScale,
                         meta->pds2.sect4.fstSurfValue,
                         meta->pds2.sect4.sndSurfType,
                         meta->pds2.sect4.sndSurfScale,
                         meta->pds2.sect4.sndSurfValue);
      /* Don't add ans to cgribLen yet (because of extra section 4 info */
      if (ans != 34) {
         freeEnGribMeta (&en);
         printf ("Problems packing section 4.0 info\n");
         return 1;
      }
   } else {
      freeEnGribMeta (&en);
      printf ("Problems packing section 4\n");
      return 1;
   }

   /* Fill out extra section 4 information */
   if (tmplNum == 1) {
      ans = fillSect4_1 (&en, tmplNum, meta->pds2.sect4.typeEnsemble,
                         meta->pds2.sect4.perturbNum,
                         meta->pds2.sect4.numberFcsts);
   } else if (tmplNum == 2) {
      ans = fillSect4_2 (&en, tmplNum, meta->pds2.sect4.numberFcsts,
                         meta->pds2.sect4.derivedFcst);
   } else if (tmplNum == 5) {
      dlowVal = (meta->pds2.sect4.lowerLimit.value /
                 pow (10, meta->pds2.sect4.lowerLimit.factor));
      dupVal = (meta->pds2.sect4.upperLimit.value /
                pow (10, meta->pds2.sect4.upperLimit.factor));
      ans = fillSect4_5 (&en, tmplNum, meta->pds2.sect4.numForeProbs,
                         meta->pds2.sect4.foreProbNum,
                         meta->pds2.sect4.probType,
                         meta->pds2.sect4.lowerLimit.factor, dlowVal,
                         meta->pds2.sect4.upperLimit.factor, dupVal);
   } else if (tmplNum == 8) {
      Clock_PrintDate (meta->pds2.sect4.validTime, &year, &month, &day, &hour,
                       &min, &sec);
      interval = (sect4IntervalType *) malloc (meta->pds2.sect4.numInterval *
                                               sizeof (sect4IntervalType));
      for (i = 0; i < meta->pds2.sect4.numInterval; ++i) {
         fillSect4_Interval (&(interval[i]),
                             meta->pds2.sect4.Interval[i].processID,
                             meta->pds2.sect4.Interval[i].incrType,
                             meta->pds2.sect4.Interval[i].timeRangeUnit,
                             meta->pds2.sect4.Interval[i].lenTime,
                             meta->pds2.sect4.Interval[i].incrUnit,
                             meta->pds2.sect4.Interval[i].timeIncr);
      }
      if (meta->pds2.sect4.numInterval != 0) {
         ans = fillSect4_8 (&en, tmplNum, year, month, day, hour, min, sec,
                            meta->pds2.sect4.numInterval,
                            meta->pds2.sect4.numMissing, interval);
      } else {
         /* Creating a special exception for GMOS maxt/mint */
         if (IsData_MOS (meta->center, meta->subcenter) &&
             (meta->pds2.prodType == 0) && (meta->pds2.sect4.cat == 0) &&
             ((meta->pds2.sect4.subcat == 4) || (meta->pds2.sect4.subcat == 5))) {
            /* maxt or mint respectively */
            interval = (sect4IntervalType *) malloc (sizeof (sect4IntervalType));
            if (meta->pds2.sect4.subcat == 4) {
               /* maxt */
               fillSect4_Interval (&(interval[0]), 2, 255, 1, 12, 1, 0);
            } else {
               /* mint */
               fillSect4_Interval (&(interval[0]), 3, 255, 1, 12, 1, 0);
            }
            ans = fillSect4_8 (&en, tmplNum, year, month, day, hour, min, sec,
                               1, meta->pds2.sect4.numMissing, interval);
         } else {
            ans = -4;
         }
      }
      free (interval);
   } else if (tmplNum == 9) {
      Clock_PrintDate (meta->pds2.sect4.validTime, &year, &month, &day, &hour,
                       &min, &sec);
      interval = (sect4IntervalType *) malloc (meta->pds2.sect4.numInterval *
                                               sizeof (sect4IntervalType));
      for (i = 0; i < meta->pds2.sect4.numInterval; ++i) {
         fillSect4_Interval (&(interval[i]),
                             meta->pds2.sect4.Interval[i].processID,
                             meta->pds2.sect4.Interval[i].incrType,
                             meta->pds2.sect4.Interval[i].timeRangeUnit,
                             meta->pds2.sect4.Interval[i].lenTime,
                             meta->pds2.sect4.Interval[i].incrUnit,
                             meta->pds2.sect4.Interval[i].timeIncr);
      }
      dlowVal = (meta->pds2.sect4.lowerLimit.value /
                 pow (10, meta->pds2.sect4.lowerLimit.factor));
      dupVal = (meta->pds2.sect4.upperLimit.value /
                pow (10, meta->pds2.sect4.upperLimit.factor));
      ans = fillSect4_9 (&en, tmplNum, meta->pds2.sect4.numForeProbs,
                         meta->pds2.sect4.foreProbNum,
                         meta->pds2.sect4.probType,
                         meta->pds2.sect4.lowerLimit.factor, dlowVal,
                         meta->pds2.sect4.upperLimit.factor, dupVal,
                         year, month, day, hour, min, sec,
                         meta->pds2.sect4.numInterval,
                         meta->pds2.sect4.numMissing, interval);
      free (interval);
   } else if (tmplNum == 10) {
      Clock_PrintDate (meta->pds2.sect4.validTime, &year, &month, &day, &hour,
                       &min, &sec);
      interval = (sect4IntervalType *) malloc (meta->pds2.sect4.numInterval *
                                               sizeof (sect4IntervalType));
      for (i = 0; i < meta->pds2.sect4.numInterval; ++i) {
         fillSect4_Interval (&(interval[i]),
                             meta->pds2.sect4.Interval[i].processID,
                             meta->pds2.sect4.Interval[i].incrType,
                             meta->pds2.sect4.Interval[i].timeRangeUnit,
                             meta->pds2.sect4.Interval[i].lenTime,
                             meta->pds2.sect4.Interval[i].incrUnit,
                             meta->pds2.sect4.Interval[i].timeIncr);
      }
      ans = fillSect4_10 (&en, tmplNum, meta->pds2.sect4.percentile, year,
                          month, day, hour, min, sec,
                          meta->pds2.sect4.numInterval,
                          meta->pds2.sect4.numMissing, interval);
      free (interval);
   } else if (tmplNum == 12) {
      Clock_PrintDate (meta->pds2.sect4.validTime, &year, &month, &day, &hour,
                       &min, &sec);
      interval = (sect4IntervalType *) malloc (meta->pds2.sect4.numInterval *
                                               sizeof (sect4IntervalType));
      for (i = 0; i < meta->pds2.sect4.numInterval; ++i) {
         fillSect4_Interval (&(interval[i]),
                             meta->pds2.sect4.Interval[i].processID,
                             meta->pds2.sect4.Interval[i].incrType,
                             meta->pds2.sect4.Interval[i].timeRangeUnit,
                             meta->pds2.sect4.Interval[i].lenTime,
                             meta->pds2.sect4.Interval[i].incrUnit,
                             meta->pds2.sect4.Interval[i].timeIncr);
      }
      ans = fillSect4_12 (&en, tmplNum, meta->pds2.sect4.numberFcsts,
                          meta->pds2.sect4.derivedFcst, year,
                          month, day, hour, min, sec,
                          meta->pds2.sect4.numInterval,
                          meta->pds2.sect4.numMissing, interval);
      free (interval);
   }
   if (ans < 0) {
      freeEnGribMeta (&en);
      printf ("Problems packing section 4\n");
      return 1;
   }
   cgribLen += ans;

   /* The 2 is because we currently always use second order differences...
    * Could change this in the future.
    */
   ans = fillSect5 (&en, meta->gridAttrib.packType, meta->gridAttrib.ESF,
                    meta->gridAttrib.DSF, meta->gridAttrib.fieldType,
                    meta->gridAttrib.f_miss, (float) meta->gridAttrib.missPri,
                    (float) meta->gridAttrib.missSec, 2);
   if (ans < 0) {
      freeEnGribMeta (&en);
      printf ("Problems packing section 5\n");
      return 1;
   }
   cgribLen += ans;

   ComputeUnit (meta->convert, meta->unitName, f_unit, &unitM, &unitB,
                unitName);
   /* 255 is because we don't currently use bitmaps.  The code exists to
    * allow other choices, but we need to pass the info into this proc. */
   ans = fillGridUnit (&en, Grib_Data, grib_DataLen, meta->gds.Nx,
                       meta->gds.Ny, 255, f_boustify, meta->gridAttrib.fieldType,
                       meta->gridAttrib.f_miss, meta->gridAttrib.missPri,
                       meta->gridAttrib.missSec, unitM, unitB);
   if (ans < 0) {
      freeEnGribMeta (&en);
      printf ("Error in Fill Grid2 %d\n", ans);
      return 1;
   }
   cgribLen += ans;

   *cPack = (uChar *) malloc (cgribLen * sizeof (char));

   /* Ideally we would determine which packing method (2 or 3) is more
    * efficient.  Dr. Glahn attempted to do so in his routine "pk_missp.f"
    * and "pk_nomiss.f".  Unfortunately the input to those routines would
    * take some massaging (time and memory allocation) to get from en.fld, and
    * it is an approximation.
    *
    * The correct time to test for 2 vs 3 (complex vs complex + spatial diff)
    * would be inside NCEP's "cmplxpack.c", or inside "compack.c",
    * "misspack.c" when the data has been massaged.
    *
    * I have worked on various packing / grouping algorithms inside "tdlpack.c"
    * and TDL_UseSecDiff_Prim, TDL_UseSecDiff are supposed to be solving this
    * question, but they don't appear to be working properly.
    *
    * The most wasteful method is to pack both ways from here (see TESTING
    * code).  This creates the optimal solution, but is slow.  I also found
    * that the majority of the time the optimal solution is 2, so that is the
    * default until I have time to modify cmplxpack.c.
    */
   if ((en.idrsnum == 2) || (en.idrsnum == 3)) {
      en.idrsnum = 2;
   }
/*
#ifdef TESTING
*/
   /* If idrsnum == 2 or 3, try 3 first, then try 2, then compare. */
/*
   if ((en.idrsnum == 2) || (en.idrsnum == 3)) {
      en.idrsnum = 3;
      cPack2 = (uChar *) malloc (cgribLen * sizeof (char));
   } else {
      cPack2 = NULL;
   }
#endif
*/
   ans = C_pkGrib2 (*cPack, en.sec0, en.sec1, en.sec2, en.lenSec2,
                    en.gds, en.gdsTmpl, en.idefList, en.idefnum, en.ipdsnum,
                    en.pdsTmpl, en.coordlist, en.numcoord, en.idrsnum,
                    en.drsTmpl, en.fld, en.ngrdpts, en.ibmap, en.bmap);
   if (ans < 0) {
      printf ("Error in pkGrib2 %d\n", ans);
      freeEnGribMeta (&en);
      free (*cPack);
      *c_len = 0;
      *cPack = NULL;
      return 1;
   }
   /* Set c_len to valid length of cPack */
   *c_len = ans;

/*
#ifdef TESTING
*/
   /* If idrsnum == 2 or 3, now try 2, then compare. */
/*
   if (en.idrsnum == 3) {
      en.idrsnum = 2;
      ans2 = C_pkGrib2 (cPack2, en.sec0, en.sec1, en.sec2, en.lenSec2,
                        en.gds, en.gdsTmpl, en.idefList, en.idefnum, en.ipdsnum,
                        en.pdsTmpl, en.coordlist, en.numcoord, en.idrsnum,
                        en.drsTmpl, en.fld, en.ngrdpts, en.ibmap, en.bmap);
      if (ans2 < 0) {
         printf ("Error in pkGrib2 %d\n", ans2);
         freeEnGribMeta (&en);
         free (*cPack);
         free (cPack2);
         *c_len = 0;
         *cPack = NULL;
         return 1;
      }
      if (ans2 < ans) {
         free (*cPack);
         *cPack = cPack2;
*/
         /* Set c_len to valid length of cPack */
/*
         *c_len = ans2;
      } else {
         free (cPack2);
*/
         /* Set c_len to valid length of cPack */
/*
         *c_len = ans;
      }
   }
#endif
*/

#ifdef MEMWATCH
   /* memwatch isn't being done in library. so... */
   if (en.fld != NULL) {
      free (en.fld);
   }
   en.fld = NULL;
#endif
   freeEnGribMeta (&en);
   return 0;
}

