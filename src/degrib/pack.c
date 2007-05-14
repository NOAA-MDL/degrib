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
#include "grib2api.h"
#include "tendian.h"
#include "pack.h"
#include "myutil.h"
#include "clock.h"

static sInt4 NearestInt (double a)
{
   return (sInt4) floor (a + .5);
}

/* Skipped over type 5 (Don't know WGS84 sphere) */
/* Skipped over type 4 (minEarth in mm?) */
static void InitEarth (grib_MetaData *meta, IS_dataType *is)
{
   if (meta->gds.f_sphere) {
      if (meta->gds.majEarth == 6367.47) {
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

int WriteGrib2Record (grib_MetaData *meta, double *Grib_Data,
                      sInt4 grib_DataLen, IS_dataType *is, sChar f_unit,
                      char **cPack, sInt4 *c_len, uChar f_stdout)
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
      if (IsData_NDFD (meta->center, meta->subcenter)) {
         is->is[4][18 - 1] = 1; /* Hours */
         is->is[4][19 - 1] = NearestInt (meta->pds2.sect4.foreSec / 3600);
      } else {
         is->is[4][18 - 1] = 13; /* Seconds */
         is->is[4][19 - 1] = NearestInt (meta->pds2.sect4.foreSec);
      }
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

         *cPack = (char *) ipack;
         *c_len = ipack[3];
         return -3;
      }
   }

   /* figure out end of message. */
   *cPack = (char *) ipack;
   *c_len = ipack[3];

   /* mem swap if need be. */
#ifdef LITTLE_ENDIAN
   memswp (ipack, sizeof (sInt4), nd5);
#endif
   return 0;
}
