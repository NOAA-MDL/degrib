#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "myerror.h"
#include "myassert.h"
#include "myutil.h"
#include "mymapf.h"
#include "database.h"
#include "commands.h"
#include "tendian.h"
#include "probe.h"
#include "weather.h"
#include "write.h"
#include "metaname.h"
#include "cube.h"
#include "clock.h"

/*****************************************************************************
 * Grib2Database() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Convert all the messages in the GRIB file to a set of .flt files, or one
 * .dat file (using WriteGradsCube).  In addition creates an index file so we
 * can probe the .flt files (or the GrADS Cube).
 *
 * ARGUMENTS
 *  usr = The user option structure to use while Degrib'ing. (Input)
 *   is = memory for the Un-parsed meta data for this GRIB2 message.
 *        As well as some memory used by the unpacker. (Input)
 * meta = The meta data from the last GRIB2 message read. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 *  1 = Problems opening the file for read.
 *  2 = File is not a GRIB file.
 *  3 = Problems while reading the GRIB file.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int Grib2Database (userType *usr, IS_dataType *is, grib_MetaData *meta)
{
   FILE *grib_fp;       /* The opened grib2 file for input. */
   sInt4 offset;        /* Where we currently are in grib_fp. */
   double *grib_Data;   /* The read in GRIB2 grid. */
   uInt4 grib_DataLen;  /* Size of Grib_Data. */
   int c;               /* Determine if end of the file without fileLen. */
   char *flxArray;      /* The index file in a char buffer. */
   int flxArrayLen;     /* The length of the flxArray buffer. */
   char *outName = NULL; /* Name of the output file */
   size_t outLen;       /* String length of outName. */
   char *outPtr;        /* Gets rid of directory dependence in outName. */
   uShort2 gdsNum;      /* The corresponding gds index in flxArray. */
   sInt4 flxLen;        /* Length of index buffer / file. */
   uChar scan;          /* Scan mode for the data grids. */
   sInt4 fltOffset;     /* Where is this message in the output grid file? */
   sChar f_delete;      /* Delete the old file when working with a -Cube. */
   sInt4 f_endMsg = 1;  /* 1 if we read the last grid in a GRIB message */
   int subgNum = 0;     /* The subgrid in the message that we are interested
                         * in. */
   int curMsg;
   int inName;

   for (inName = 0; inName < usr->numInNames; inName++) {
      if (usr->inNames[inName] != NULL) {
         if ((grib_fp = fopen (usr->inNames[inName], "rb")) == NULL) {
            errSprintf ("Problems opening %s for read\n",
                        usr->inNames[inName]);
            return 1;
         }
      } else {
         grib_fp = stdin;
      }

      /* Find the desired GRIB message.  Most likely they want "all", but
       * just in case. */
      if (usr->msgNum > 0) {
         if (FindGRIBMsg (grib_fp, usr->msgNum, &offset, &curMsg) == -1) {
            fclose (grib_fp);
            return 2;
         }
      }

      /* Set up inital state of data for unpacker. */
      grib_DataLen = 0;
      grib_Data = NULL;

      f_delete = 0;
      if (inName == 0) {
         if (usr->f_Append) {
            if (ReadFLX (usr->indexFile, &flxArray, &flxArrayLen) != 0) {
               CreateFLX (&flxArray, &flxArrayLen);
               f_delete = 1;
            }
         } else {
            CreateFLX (&flxArray, &flxArrayLen);
            f_delete = 1;
         }
      }

      /* Start loop for all messages. */
      while ((c = fgetc (grib_fp)) != EOF) {
         ungetc (c, grib_fp);
         /* Read the GRIB message. */
         if (ReadGrib2Record (grib_fp, usr->f_unit, &grib_Data, &grib_DataLen,
                              meta, is, subgNum, usr->majEarth, usr->minEarth,
                              usr->f_SimpleVer, usr->f_SimpleWWA, &f_endMsg, &(usr->lwlf),
                              &(usr->uprt)) != 0) {
            preErrSprintf ("ERROR: In call to ReadGrib2Record.\n");
            goto error;
         }
         if (usr->f_validRange > 0) {
            /* valid max. */
            if (usr->f_validRange > 1) {
               if (meta->gridAttrib.max > usr->validMax) {
                  errSprintf ("ERROR: %f > valid Max of %f\n",
                              meta->gridAttrib.max, usr->validMax);
                  goto error;
               }
            }
            /* valid min. */
            if (usr->f_validRange % 2) {
               if (meta->gridAttrib.min < usr->validMin) {
                  errSprintf ("ERROR: %f < valid Min of %f\n",
                              meta->gridAttrib.min, usr->validMin);
                  goto error;
               }
            }
         }
         if (f_endMsg != 1) {
            subgNum++;
         } else {
            subgNum = 0;
         }

         /* Figure out the output filename. */
         if (GetOutputName (usr, meta, &outName, &outLen) != 0) {
            goto error;
         }

         /* Determine if the lower left or upper left is the 0,0 value. */
         if (usr->f_revFlt) {
            scan = GRIB2BIT_2;
         } else {
            scan = 0;
         }

         /* Figure out the extension of outName and call grid write routines. 
          */
         if (usr->f_Cube) {
            strncpy (outName + outLen - 3, "dat", 3);
            fltOffset = -1;
            /* If fltOffset < 0, append, and set fltOffset */
            if (WriteGradsCube (outName, grib_Data, meta, &(meta->gridAttrib),
                                scan, usr->f_MSB, 7, &fltOffset, f_delete)
                != 0) {
               goto error;
            }
/*
            if (WriteGradsCube (outName, grib_Data, meta, &(meta->gridAttrib),
                                scan, usr->f_MSB, usr->decimal, usr->f_GrADS,
                                &fltOffset, f_delete) != 0) {
               goto error;
            }
*/
            f_delete = 0;
            myAssert (fltOffset >= 0);
         } else {
            if (usr->f_revFlt) {
               strncpy (outName + outLen - 3, "tlf", 3);
            } else {
               strncpy (outName + outLen - 3, "flt", 3);
            }
            /* Can't use f_interp unless we update the GDS accordingly. */
            /* Can't use f_SimpleWx unless we update the PDS. */
/*
            if (gribWriteFloat (outName, grib_Data, meta, &(meta->gridAttrib),
                                scan, usr->f_MSB, usr->decimal, usr->f_GrADS,
                                0) != 0) {
               goto error;
            }
*/
            if (gribWriteFloat (outName, grib_Data, meta, &(meta->gridAttrib),
                                scan, usr->f_MSB, 7, usr->f_GrADS, 0,
                                0) != 0) {
               goto error;
            }
            fltOffset = 0;
         }

         /* Try to strip the path from outName. */
         if ((outPtr = strrchr (outName, '/')) == NULL) {
            if ((outPtr = strrchr (outName, '\\')) == NULL) {
               outPtr = outName;
            } else {
               outPtr++;
            }
         } else {
            outPtr++;
         }

         /* Update the index file / buffer. */
         gdsNum = InsertGDS (&flxArray, &flxArrayLen, &(meta->gds));
         myAssert ((meta->GribVersion == -1) || (meta->GribVersion == 1) ||
                   (meta->GribVersion == 2));
         if (meta->GribVersion == -1) {
            if (InsertPDS (&flxArray, &flxArrayLen, meta->element,
                           (time_t) meta->pdsTdlp.refTime, meta->unitName,
                           meta->comment, gdsNum, 7, 14,
                           (time_t) (meta->pdsTdlp.refTime +
                                     meta->pdsTdlp.project), outPtr,
                           fltOffset, usr->f_MSB, scan, NULL, 0) != 0) {
               errSprintf ("ERROR: strlen (%s) > 254\n", outPtr);
               goto error;
            }
         } else if (meta->GribVersion == 1) {
            if (InsertPDS (&flxArray, &flxArrayLen, meta->element,
                           (time_t) meta->pds1.refTime, meta->unitName,
                           meta->comment, gdsNum, meta->center,
                           meta->subcenter, (time_t) meta->pds1.P1, outPtr,
                           fltOffset, usr->f_MSB, scan, NULL, 0) != 0) {
               errSprintf ("ERROR: strlen (%s) > 254\n", outPtr);
               goto error;
            }
         } else {
            if (meta->pds2.f_sect2) {
               myAssert (meta->pds2.sect2.ptrType != GS2_NONE);
               if (meta->pds2.sect2.ptrType == GS2_WXTYPE) {
                  if (InsertPDS (&flxArray, &flxArrayLen, meta->element,
                                 (time_t) meta->pds2.refTime, meta->unitName,
                                 meta->comment, gdsNum, meta->center,
                                 meta->subcenter,
                                 (time_t) meta->pds2.sect4.validTime, outPtr,
                                 fltOffset, usr->f_MSB, scan,
                                 meta->pds2.sect2.wx.data,
                                 meta->pds2.sect2.wx.dataLen) != 0) {
                     errSprintf ("ERROR: strlen (%s) > 254\n", outPtr);
                     goto error;
                  }
               } else if (meta->pds2.sect2.ptrType == GS2_HAZARD) {
                  if (InsertPDS (&flxArray, &flxArrayLen, meta->element,
                                 (time_t) meta->pds2.refTime, meta->unitName,
                                 meta->comment, gdsNum, meta->center,
                                 meta->subcenter,
                                 (time_t) meta->pds2.sect4.validTime, outPtr,
                                 fltOffset, usr->f_MSB, scan,
                                 meta->pds2.sect2.hazard.data,
                                 meta->pds2.sect2.hazard.dataLen) != 0) {
                     errSprintf ("ERROR: strlen (%s) > 254\n", outPtr);
                     goto error;
                  }
               } else {
                  errSprintf ("ERROR: working with element: %s\n",
                              meta->element);
                  errSprintf ("Don't know how to add this section 2 data to"
                              " the database.\n");
                  errSprintf ("Possible answer 1: tack it on to PDS Array\n");
                  errSprintf ("Possible answer 2: use table entry with "
                              "sizeof information\n");
                  goto error;
               }
            } else {
               if (InsertPDS (&flxArray, &flxArrayLen, meta->element,
                              (time_t) meta->pds2.refTime, meta->unitName,
                              meta->comment, gdsNum, meta->center,
                              meta->subcenter,
                              (time_t) meta->pds2.sect4.validTime, outPtr,
                              fltOffset, usr->f_MSB, scan, NULL, 0) != 0) {
                  errSprintf ("ERROR: strlen (%s) > 254\n", outPtr);
                  goto error;
               }
            }
         }

         /* Most likely they added all the messages... but just in case. */
         if (usr->msgNum != 0)
            break;
      }
      /* End loop for all messages. */
      fclose (grib_fp);
   }

   /* Update fileLen and write the index file out. */
   flxLen = flxArrayLen;
   MEMCPY_LIT (flxArray + 3, &flxLen, sizeof (sInt4));
   WriteFLX (usr->indexFile, flxArray, flxArrayLen);
   free (flxArray);

   free (grib_Data);
   free (outName);
   return 0;

 error:
   /* Update fileLen and write the index file out. */
   flxLen = flxArrayLen;
   MEMCPY_LIT (flxArray + 3, &flxLen, sizeof (sInt4));
   WriteFLX (usr->indexFile, flxArray, flxArrayLen);
   free (flxArray);

   free (grib_Data);
   free (outName);
   return 3;
}

void NDFD_Cube2MetaWx (sect2_WxType *Wx, char **keys, int numKeys,
                       int simpVer)
{
   int i;
   int j;
   int len;             /* length of current english phrases during creation
                         * of the maxEng[] data. */

   Wx->dataLen = numKeys;
   Wx->data = (char **) malloc (numKeys * sizeof (char *));
   Wx->ugly = (UglyStringType *) malloc (numKeys * sizeof (UglyStringType));
   Wx->maxLen = 0;
   for (i = 0; i < NUM_UGLY_WORD; i++) {
      Wx->maxEng[i] = 0;
   }

   for (i = 0; i < numKeys; i++) {
      len = strlen (keys[i]);
      Wx->data[i] = (char *) malloc ((len + 1) * sizeof (char));
      strcpy (Wx->data[i], keys[i]);
      if (Wx->maxLen < len) {
         Wx->maxLen = len;
      }
      ParseUglyString (&(Wx->ugly[i]), Wx->data[i], simpVer);
      /* We want to know how many bytes we need for each english phrase
       * column, so we walk through each column calculating that value. */
      for (j = 0; j < NUM_UGLY_WORD; j++) {
         if (Wx->ugly[i].english[j] != NULL) {
            len = strlen (Wx->ugly[i].english[j]);
            if (len > Wx->maxEng[j]) {
               Wx->maxEng[j] = len;
            }
         }
      }
   }
}

/* When we read the data, we have to get rid of unit conversion. */
/* Have to assume 9999 is missing. */
/* Have to assume level is surface. */
int NDFD_Cube2Meta (grib_MetaData *meta, char *elem, char *unit,
                    char *comment, uShort2 center, uShort2 subCenter,
                    double refTime, double valTime, uShort2 gdsIndex,
                    char *flxArray, char **keys, int numKeys, int simpVer)
{
   typedef struct {
      uChar prodType, cat, subcat;
      uShort2 templat;
      uChar DSF, fieldType, packType;
   } NDFD_ValuesTable;

   static char *NDFD_Type[] = { "T", "MaxT", "MinT", "Td", "QPF",
      "SnowAmt", "WindDir", "WindSpd", "Sky", "WaveHeight", "Wx",
      "PoP12", "ApparentT", "RH", "WindGust", "ConvOutlook", "TornadoProb",
      "HailProb", "WindProb", "XtrmTornProb", "XtrmHailProb", "XtrmWindProb",
      "TotalSvrProb", "TotalXtrmProb", "ProbWindSpd34c", "ProbWindSpd34i",
      "ProbWindSpd50c", "ProbWindSpd50i", "ProbWindSpd64c", "ProbWindSpd64i",
      "ProbTMPAbv144", "ProbTMPBlw144", "ProbPrcpAbv144", "ProbPrcpBlw144",
      "ProbTMPAbv01m", "ProbTMPBlw01m", "ProbPrcpAbv01m", "ProbPrcpBlw01m",
      "ProbTMPAbv03m", "ProbTMPBlw03m", "ProbPrcpAbv03m", "ProbPrcpBlw03m",
      "CANL", NULL
   };
   enum { TEMP, MAXT, MINT, TD, QPF, SNOW, WINDDIR, WINDSPD, SKY,
      WAVEHEIGHT, WX, POP12, APPARENTT, RH, WINDGUST, CONVOUTLOOK, PTORN,
      PHAIL, PWIND, PXTRMTORN, PXTRMHAIL, PXTRMWIND, PTOTSVR, PTOTXTRM,
      PROBWINDSPD34C, PROBWINDSPD34I, PROBWINDSPD50C, PROBWINDSPD50I,
      PROBWINDSPD64C, PROBWINDSPD64I, PROBTMPABV144, PROBTMPBLW144,
      PROBPRCPABV144, PROBPRCPBLW144, PROBTMPABV01M, PROBTMPBLW01M,
      PROBPRCPABV01M, PROBPRCPBLW01M, PROBTMPABV03M, PROBTMPBLW03M,
      PROBPRCPABV03M, PROBPRCPBLW03M, CANL, DEFAULT_NUM
   };
/* *INDENT-OFF* */
   static NDFD_ValuesTable NDFD_Values[] = {
      /* T */       { 0, 0,   0, 0, 1, 0, 3},
      /* MaxT */    { 0, 0,   4, 8, 1, 0, 3},
      /* MinT */    { 0, 0,   5, 8, 1, 0, 3},
      /* Td */      { 0, 0,   6, 0, 1, 0, 3},
      /* QPF */     { 0, 1,   8, 8, 2, 0, 2}, /* Why this packtype? */
      /* Snow */    { 0, 1,  29, 8, 3, 0, 3},
      /* WndDir */  { 0, 2,   0, 0, 0, 0, 3},
      /* WndSpd */  { 0, 2,   1, 0, 1, 0, 3}, /* DSF actually 0? */
      /* Sky */     { 0, 6,   1, 0, 0, 0, 3},
      /* WaveHt */  {10, 0,   5, 0, 1, 0, 2}, /* Why this packtype? */
      /* Wx */      { 0, 1, 192, 0, 0, 1, 3},
      /* PoP12 */   { 0, 1,   8, 9, 0, 0, 3},
      /* AppT */    { 0, 0, 193, 0, 1, 0, 2},
      /* RHM */     { 0, 1,   1, 0, 0, 0, 2},
      /* WindGust*/ { 0, 2,  22, 0, 1, 0, 3},
   /* ConvOutlook*/ { 0, 19, 194, 8, 1, 0, 2},
   /* Ptornado */   { 0, 19, 197, 9, 1, 0, 2},
   /* Phail */      { 0, 19, 198, 9, 1, 0, 2},
   /* Pwind */      { 0, 19, 199, 9, 1, 0, 2},
   /* PXtrmTorn */  { 0, 19, 200, 9, 1, 0, 2},
   /* PXtrmHail */  { 0, 19, 201, 9, 1, 0, 2},
   /* PXtrmWind */  { 0, 19, 202, 9, 1, 0, 2},
/* Following two lines changed from 203->215 and 204->216 9/19/2007 */
   /* TotSvrProb */ { 0, 19, 215, 9, 1, 0, 2},
   /* TotXtrmProb */{ 0, 19, 216, 9, 1, 0, 2},
   /* PWndSpd34c */ { 0, 2,   1, 9, 0, 0, 2},
   /* PWndSpd34i */ { 0, 2,   1, 9, 0, 0, 2},
   /* PWndSpd50c */ { 0, 2,   1, 9, 0, 0, 2},
   /* PWndSpd50i */ { 0, 2,   1, 9, 0, 0, 2},
   /* PWndSpd64c */ { 0, 2,   1, 9, 0, 0, 2},
   /* PWndSpd64i */ { 0, 2,   1, 9, 0, 0, 2},

   /* PTmpAbv144 */ { 0, 0,   0, 9, 0, 0, 3},
   /* PTmpBlw144 */ { 0, 0,   0, 9, 0, 0, 3},
   /* PPrcpAbv144 */{ 0, 1,   8, 9, 0, 0, 3},
   /* PPrcpAbv144 */{ 0, 1,   8, 9, 0, 0, 3},
   /* PTmpAbv01m */ { 0, 0,   0, 9, 0, 0, 3},
   /* PTmpBlw01m */ { 0, 0,   0, 9, 0, 0, 3},
   /* PPrcpAbv01m */{ 0, 1,   8, 9, 0, 0, 3},
   /* PPrcpAbv01m */{ 0, 1,   8, 9, 0, 0, 3},
   /* PTmpAbv03m */ { 0, 0,   0, 9, 0, 0, 3},
   /* PTmpBlw03m */ { 0, 0,   0, 9, 0, 0, 3},
   /* PPrcpAbv03m */{ 0, 1,   8, 9, 0, 0, 3},
   /* PPrcpAbv03m */{ 0, 1,   8, 9, 0, 0, 3},
   /* CANL */       { 2, 1,   192, 8, 1, 0, 2},

      /* Default */ { 0, 0,   0, 0, 0, 0, 3},
   };
/* *INDENT-ON* */

   char shortFstLevel[] = "0-SFC";
   char longFstLevel[] = "0[SFC] Ground or water surface (-)";
   int elemNum;         /* "elem"'s index into NDFD_Type */
   time_t tempTime;     /* Used when printing out a "double" time_t value. */

   sInt4 totDay;
   int day;
   sInt4 year;
   int month;
   double d_tempTime;

/*
   if (!IsData_NDFD (center, subCenter)) {
      errSprintf ("Can only convert NDFD cube index files to GRIB2");
      return 1;
   }
*/
   /* Figure out which NDFD Element. */
/*
   if (GetIndexFromStr (elem, NDFD_Type, &elemNum) < 0) {
      errSprintf ("Unrecognized NDFD variable '%s'\n", elem);
      return 1;
   }
*/
   if (GetIndexFromStr (elem, NDFD_Type, &elemNum) < 0) {
      elemNum = DEFAULT_NUM;
   }

   meta->GribVersion = 2;
   meta->element = (char *) realloc (meta->element,
                                     (strlen (elem) + 1) * sizeof (char));
   strcpy (meta->element, elem);
   meta->unitName = (char *) realloc (meta->unitName,
                                      (strlen (unit) + 1) * sizeof (char));
   strcpy (meta->unitName, unit);
   meta->comment = (char *) realloc (meta->comment,
                                     (strlen (comment) + 1) * sizeof (char));
   strcpy (meta->comment, comment);
   meta->shortFstLevel = (char *) realloc (meta->shortFstLevel,
                                           (strlen (shortFstLevel) + 1) *
                                           sizeof (char));
   strcpy (meta->shortFstLevel, shortFstLevel);
   meta->longFstLevel = (char *) realloc (meta->longFstLevel,
                                          (strlen (longFstLevel) + 1) *
                                          sizeof (char));
   strcpy (meta->longFstLevel, longFstLevel);
   tempTime = (time_t) refTime;
   strftime (meta->refTime, 20, "%Y%m%d%H%M", gmtime (&tempTime));
   tempTime = (time_t) valTime;
   strftime (meta->validTime, 20, "%Y%m%d%H%M", gmtime (&tempTime));
   meta->deltTime = (sInt4) (valTime - refTime);

   /* Now fill out pds2. */
   meta->pds2.prodType = NDFD_Values[elemNum].prodType;
   meta->center = center;
   meta->subcenter = subCenter;
   meta->pds2.mstrVersion = 1;
   meta->pds2.lclVersion = 0;
   meta->pds2.sigTime = 1; /* refTime is start of forecast. */
   meta->pds2.refTime = (time_t) refTime;
   if ((elemNum == CONVOUTLOOK) || (elemNum == PHAIL) || (elemNum == PTORN) ||
       (elemNum == PWIND) || (elemNum == PXTRMTORN) || (elemNum == PXTRMHAIL) ||
       (elemNum == PXTRMWIND) || (elemNum == PTOTSVR) || (elemNum == PTOTXTRM) ||
       (elemNum == PROBWINDSPD34C) || (elemNum == PROBWINDSPD34I) ||
       (elemNum == PROBWINDSPD50C) || (elemNum == PROBWINDSPD50I) ||
       (elemNum == PROBWINDSPD64C) || (elemNum == PROBWINDSPD64I) ||
       (elemNum == QPF) || (elemNum == SKY) || (elemNum == SNOW) ||
       (elemNum == WAVEHEIGHT) || (elemNum == WINDGUST) ||
       (elemNum == PROBTMPABV144) || (elemNum == PROBTMPBLW144) ||
       (elemNum == PROBPRCPABV144) || (elemNum == PROBPRCPBLW144) ||
       (elemNum == PROBTMPABV01M) || (elemNum == PROBTMPBLW01M) ||
       (elemNum == PROBPRCPABV01M) || (elemNum == PROBPRCPBLW01M) ||
       (elemNum == PROBTMPABV03M) || (elemNum == PROBTMPBLW03M) ||
       (elemNum == PROBPRCPABV03M) || (elemNum == PROBPRCPBLW03M) ||
       (elemNum == CANL)) {
      meta->pds2.operStatus = 1;
   } else {
      meta->pds2.operStatus = 0; /* Pretend NDFD is operational. */
   }
   meta->pds2.dataType = 1; /* NDFD is a forecast */
   if (elemNum == WX) {
      meta->pds2.f_sect2 = 1;
      meta->pds2.sect2NumGroups = 1;
      /* Now fill out pds2.sect2 */
      meta->pds2.sect2.ptrType = GS2_WXTYPE;
      NDFD_Cube2MetaWx (&(meta->pds2.sect2.wx), keys, numKeys, simpVer);
   } else {
      meta->pds2.f_sect2 = 0;
      meta->pds2.sect2NumGroups = 0;
   }
   /* Now fill out pds2.sect4 */
   meta->pds2.sect4.templat = NDFD_Values[elemNum].templat;
   meta->pds2.sect4.cat = NDFD_Values[elemNum].cat;
   meta->pds2.sect4.subcat = NDFD_Values[elemNum].subcat;
   meta->pds2.sect4.genProcess = 2; /* forecast */
   meta->pds2.sect4.bgGenID = 0; /* Background Generating process id */
   meta->pds2.sect4.genID = 0; /* Generating process id */
   meta->pds2.sect4.f_validCutOff = 0; /* Cutoff data missing */
   meta->pds2.sect4.foreUnit = 1;
   if ((elemNum == PROBWINDSPD34C) || (elemNum == PROBWINDSPD50C) ||
       (elemNum == PROBWINDSPD64C)) {
      meta->pds2.sect4.foreSec = 0;
   } else if ((elemNum == PROBWINDSPD34I) || (elemNum == PROBWINDSPD50I) ||
       (elemNum == PROBWINDSPD64I)) {
      meta->pds2.sect4.foreSec = meta->deltTime - 6 * 3600;
   } else if ((elemNum == PROBTMPABV144) || (elemNum == PROBTMPBLW144) ||
          (elemNum == PROBPRCPABV144) || (elemNum == PROBPRCPBLW144)) {
      meta->pds2.sect4.foreSec = 8 * 24 * 3600;
   } else if ((elemNum == PROBTMPABV01M) || (elemNum == PROBTMPBLW01M) ||
          (elemNum == PROBPRCPABV01M) || (elemNum == PROBPRCPBLW01M)) {
      /* Find first day of month of refTime. */
      totDay = (sInt4) floor (refTime / SEC_DAY);
      Clock_Epoch2YearDay (totDay, &day, &year);
      month = Clock_MonthNum (day, year);
      d_tempTime = 0;
      Clock_ScanDate (&d_tempTime, year, month, 1);
      /* ForeSec is 1 month before valTime - first day of month of refTime */
      meta->pds2.sect4.foreSec = Clock_AddMonthYear (valTime, -1, 0) - d_tempTime;
   } else if ((elemNum == PROBTMPABV03M) || (elemNum == PROBTMPBLW03M) ||
          (elemNum == PROBPRCPABV03M) || (elemNum == PROBPRCPBLW03M)) {
      /* Find first day of month of refTime. */
      totDay = (sInt4) floor (refTime / SEC_DAY);
      Clock_Epoch2YearDay (totDay, &day, &year);
      month = Clock_MonthNum (day, year);
      d_tempTime = 0;
      Clock_ScanDate (&d_tempTime, year, month, 1);
      /* ForeSec is 3 month before valTime - first day of month of refTime */
      meta->pds2.sect4.foreSec = Clock_AddMonthYear (valTime, -3, 0) - d_tempTime;
   } else {
      meta->pds2.sect4.foreSec = meta->deltTime;
   }
   if ((elemNum == PROBWINDSPD34C) || (elemNum == PROBWINDSPD34I) ||
       (elemNum == PROBWINDSPD50C) || (elemNum == PROBWINDSPD50I) ||
       (elemNum == PROBWINDSPD64C) || (elemNum == PROBWINDSPD64I)) {
      meta->pds2.sect4.fstSurfType = 103; /* Surface level */
      meta->pds2.sect4.fstSurfValue = 10;
      meta->pds2.sect4.fstSurfScale = 0;
   } else {
      meta->pds2.sect4.fstSurfType = 1; /* Surface level */
      meta->pds2.sect4.fstSurfValue = 0;
      meta->pds2.sect4.fstSurfScale = 0;
   }
   meta->pds2.sect4.sndSurfType = GRIB2MISSING_u1;
   meta->pds2.sect4.sndSurfValue = -1; /* NDFD used missing == -1. */
   meta->pds2.sect4.sndSurfScale = -1; /* NDFD used missing == -1. */
   meta->pds2.sect4.validTime = (time_t) valTime;

   if (meta->pds2.sect4.templat == 8) {
      meta->pds2.sect4.numInterval = 1;
      meta->pds2.sect4.Interval = (sect4_IntervalType *)
            realloc (meta->pds2.sect4.Interval,
                     meta->pds2.sect4.numInterval *
                     sizeof (sect4_IntervalType));
      meta->pds2.sect4.numMissing = 0;
      if (elemNum == MAXT) {
         /* Statistical process = Maximum */
         meta->pds2.sect4.Interval[0].processID = 2;
      } else if (elemNum == MINT) {
         /* Statistical process = Minimum */
         meta->pds2.sect4.Interval[0].processID = 3;
      } else if ((elemNum == QPF) || (elemNum == SNOW)) {
         /* Statistical process = Accumulation */
         meta->pds2.sect4.Interval[0].processID = 1;
      } else if ((elemNum == CONVOUTLOOK) || (elemNum == CANL)) {
         /* Statistical process = Average */
         meta->pds2.sect4.Interval[0].processID = 0;
      }
      meta->pds2.sect4.Interval[0].incrType = GRIB2MISSING_u1;
      meta->pds2.sect4.Interval[0].timeRangeUnit = 1;
      if ((elemNum == MAXT) || (elemNum == MINT)) {
         meta->pds2.sect4.Interval[0].lenTime = 12;
      } else if ((elemNum == QPF) || (elemNum == SNOW) || (elemNum == CANL)) {
         meta->pds2.sect4.Interval[0].lenTime = 6;
      } else if (elemNum == CONVOUTLOOK) {
         meta->pds2.sect4.Interval[0].lenTime = 24;
      }
      meta->pds2.sect4.Interval[0].incrUnit = 1;
      meta->pds2.sect4.Interval[0].timeIncr = 0;

   } else if (meta->pds2.sect4.templat == 9) {
      meta->pds2.sect4.foreProbNum = GRIB2MISSING_u1;
      if ((elemNum == PROBTMPABV144) || (elemNum == PROBTMPBLW144) ||
          (elemNum == PROBPRCPABV144) || (elemNum == PROBPRCPBLW144) ||
          (elemNum == PROBTMPABV01M) || (elemNum == PROBTMPBLW01M) ||
          (elemNum == PROBPRCPABV01M) || (elemNum == PROBPRCPBLW01M) ||
          (elemNum == PROBTMPABV03M) || (elemNum == PROBTMPBLW03M) ||
          (elemNum == PROBPRCPABV03M) || (elemNum == PROBPRCPBLW03M)) {
         meta->pds2.sect4.numForeProbs = 2;
      } else {
         meta->pds2.sect4.numForeProbs = GRIB2MISSING_u1;
      }
      if ((elemNum == PROBTMPABV144) || (elemNum == PROBPRCPABV144) ||
          (elemNum == PROBTMPABV01M) || (elemNum == PROBPRCPABV01M) ||
          (elemNum == PROBTMPABV03M) || (elemNum == PROBPRCPABV03M)) {
         meta->pds2.sect4.probType = 3;
      } else if ((elemNum == PROBTMPBLW144) || (elemNum == PROBPRCPBLW144) ||
          (elemNum == PROBTMPBLW01M) || (elemNum == PROBPRCPBLW01M) ||
          (elemNum == PROBTMPBLW03M) || (elemNum == PROBPRCPBLW03M)) {
         meta->pds2.sect4.probType = 0;
      } else {
         meta->pds2.sect4.probType = 1;
      }

      if ((elemNum == PROBWINDSPD34C) || (elemNum == PROBWINDSPD34I) ||
          (elemNum == PROBWINDSPD50C) || (elemNum == PROBWINDSPD50I) ||
          (elemNum == PROBWINDSPD64C) || (elemNum == PROBWINDSPD64I) ||
          (elemNum == PROBTMPABV144) || (elemNum == PROBTMPBLW144) ||
          (elemNum == PROBPRCPABV144) || (elemNum == PROBPRCPBLW144) ||
          (elemNum == PROBTMPABV01M) || (elemNum == PROBTMPBLW01M) ||
          (elemNum == PROBPRCPABV01M) || (elemNum == PROBPRCPBLW01M) ||
          (elemNum == PROBTMPABV03M) || (elemNum == PROBTMPBLW03M) ||
          (elemNum == PROBPRCPABV03M) || (elemNum == PROBPRCPBLW03M)) {
         meta->pds2.sect4.lowerLimit.factor = 0;
         meta->pds2.sect4.lowerLimit.value = 0;
      } else {
/* The following probably should be GRIB2MISSING_s1, but to mirror what the
 * NDFD used, we have to use -1. */
         meta->pds2.sect4.lowerLimit.factor = -1;
         meta->pds2.sect4.lowerLimit.value = GRIB2MISSING_s4;
      }

      if (elemNum == POP12) {
         meta->pds2.sect4.upperLimit.factor = 3; /* Defin of scale factor. */
         meta->pds2.sect4.upperLimit.value = 254;
      } else if ((elemNum == PHAIL) || (elemNum == PTORN) || (elemNum == PWIND) ||
                 (elemNum == PXTRMTORN) || (elemNum == PXTRMHAIL) || (elemNum == PXTRMWIND) ||
                 (elemNum == PTOTSVR) || (elemNum == PTOTXTRM) ||
                 (elemNum == PROBTMPABV144) || (elemNum == PROBTMPBLW144) ||
                 (elemNum == PROBPRCPABV144) || (elemNum == PROBPRCPBLW144) ||
                 (elemNum == PROBTMPABV01M) || (elemNum == PROBTMPBLW01M) ||
                 (elemNum == PROBPRCPABV01M) || (elemNum == PROBPRCPBLW01M) ||
                 (elemNum == PROBTMPABV03M) || (elemNum == PROBTMPBLW03M) ||
                 (elemNum == PROBPRCPABV03M) || (elemNum == PROBPRCPBLW03M)) {
         meta->pds2.sect4.upperLimit.factor = 0; /* Defin of scale factor. */
         meta->pds2.sect4.upperLimit.value = 0;
      } else if ((elemNum == PROBWINDSPD34C) || (elemNum == PROBWINDSPD34I)) {
         meta->pds2.sect4.upperLimit.factor = 3; /* Defin of scale factor. */
         meta->pds2.sect4.upperLimit.value = 17491;
      } else if ((elemNum == PROBWINDSPD50C) || (elemNum == PROBWINDSPD50I)) {
         meta->pds2.sect4.upperLimit.factor = 3; /* Defin of scale factor. */
         meta->pds2.sect4.upperLimit.value = 25722;
      } else if ((elemNum == PROBWINDSPD64C) || (elemNum == PROBWINDSPD64I)) {
         meta->pds2.sect4.upperLimit.factor = 3; /* Defin of scale factor. */
         meta->pds2.sect4.upperLimit.value = 32924;
      }
      meta->pds2.sect4.numInterval = 1;
      meta->pds2.sect4.Interval = (sect4_IntervalType *)
            realloc (meta->pds2.sect4.Interval,
                     meta->pds2.sect4.numInterval *
                     sizeof (sect4_IntervalType));
      meta->pds2.sect4.numMissing = 0;
      if ((elemNum == POP12) || (elemNum == PROBWINDSPD34C) ||
          (elemNum == PROBWINDSPD34I) || (elemNum == PROBWINDSPD50C) ||
          (elemNum == PROBWINDSPD50I) || (elemNum == PROBWINDSPD64C) ||
          (elemNum == PROBWINDSPD64I)) {
         meta->pds2.sect4.Interval[0].processID = 1; /* For Accumulation */
      } else if ((elemNum == PHAIL) || (elemNum == PTORN) || (elemNum == PWIND) ||
                 (elemNum == PXTRMTORN) || (elemNum == PXTRMHAIL) || (elemNum == PXTRMWIND) ||
                 (elemNum == PTOTSVR) || (elemNum == PTOTXTRM) ||
                 (elemNum == PROBTMPABV144) || (elemNum == PROBTMPBLW144) ||
                 (elemNum == PROBPRCPABV144) || (elemNum == PROBPRCPBLW144) ||
                 (elemNum == PROBTMPABV01M) || (elemNum == PROBTMPBLW01M) ||
                 (elemNum == PROBPRCPABV01M) || (elemNum == PROBPRCPBLW01M) ||
                 (elemNum == PROBTMPABV03M) || (elemNum == PROBTMPBLW03M) ||
                 (elemNum == PROBPRCPABV03M) || (elemNum == PROBPRCPBLW03M)) {
         meta->pds2.sect4.Interval[0].processID = 0; /* For Average */
      }
      if ((elemNum == PROBWINDSPD34C) || (elemNum == PROBWINDSPD50C) ||
          (elemNum == PROBWINDSPD64C)) {
         meta->pds2.sect4.Interval[0].incrType = 192;
      } else if ((elemNum == PROBWINDSPD34I) || (elemNum == PROBWINDSPD50I) ||
          (elemNum == PROBWINDSPD64I)) {
         meta->pds2.sect4.Interval[0].incrType = 2;
      } else {
         meta->pds2.sect4.Interval[0].incrType = GRIB2MISSING_u1;
      }
      if ((elemNum == PROBTMPABV144) || (elemNum == PROBTMPBLW144) ||
          (elemNum == PROBPRCPABV144) || (elemNum == PROBPRCPBLW144)) {
         meta->pds2.sect4.Interval[0].timeRangeUnit = 2;
      } else if ((elemNum == PROBTMPABV01M) || (elemNum == PROBTMPBLW01M) ||
          (elemNum == PROBPRCPABV01M) || (elemNum == PROBPRCPBLW01M) ||
          (elemNum == PROBTMPABV03M) || (elemNum == PROBTMPBLW03M) ||
          (elemNum == PROBPRCPABV03M) || (elemNum == PROBPRCPBLW03M)) {
         meta->pds2.sect4.Interval[0].timeRangeUnit = 3;
      } else {
         meta->pds2.sect4.Interval[0].timeRangeUnit = 1;
      }
      if (elemNum == POP12) {
         meta->pds2.sect4.Interval[0].lenTime = 12;
      } else if ((elemNum == PHAIL) || (elemNum == PTORN) || (elemNum == PWIND) ||
                 (elemNum == PXTRMTORN) || (elemNum == PXTRMHAIL) || (elemNum == PXTRMWIND) ||
                 (elemNum == PTOTSVR) || (elemNum == PTOTXTRM)) {
         meta->pds2.sect4.Interval[0].lenTime = 24;
      } else if ((elemNum == PROBWINDSPD34I) || (elemNum == PROBWINDSPD50I) ||
                 (elemNum == PROBWINDSPD64I)) {
         meta->pds2.sect4.Interval[0].lenTime = 6;
      } else if ((elemNum == PROBWINDSPD34C) || (elemNum == PROBWINDSPD50C) ||
                 (elemNum == PROBWINDSPD64C)) {
         meta->pds2.sect4.Interval[0].lenTime = (valTime - refTime) / 3600;
      } else if ((elemNum == PROBTMPABV144) || (elemNum == PROBTMPBLW144) ||
                 (elemNum == PROBPRCPABV144) || (elemNum == PROBPRCPBLW144)) {
         meta->pds2.sect4.Interval[0].lenTime = 6;
      } else if ((elemNum == PROBTMPABV01M) || (elemNum == PROBTMPBLW01M) ||
                 (elemNum == PROBPRCPABV01M) || (elemNum == PROBPRCPBLW01M)) {
         meta->pds2.sect4.Interval[0].lenTime = 1;
      } else if ((elemNum == PROBTMPABV03M) || (elemNum == PROBTMPBLW03M) ||
                 (elemNum == PROBPRCPABV03M) || (elemNum == PROBPRCPBLW03M)) {
         meta->pds2.sect4.Interval[0].lenTime = 3;
      }
      meta->pds2.sect4.Interval[0].incrUnit = 1;
      meta->pds2.sect4.Interval[0].timeIncr = 0;
   }

   /* Now fill out gds */
   ReadGDSBuffer (flxArray + HEADLEN + 2 + (gdsIndex - 1) * 129,
                  &(meta->gds));

   /* now fill out the attrib type? */
   /* Start with 3 ... should downgrade to 2? */
/*   meta->gridAttrib.packType = 3; */
   meta->gridAttrib.packType = NDFD_Values[elemNum].packType;
   if ((elemNum == CONVOUTLOOK) || (elemNum == PHAIL) || (elemNum == PTORN) ||
       (elemNum == PWIND) || (elemNum == PXTRMTORN) || (elemNum == PXTRMHAIL) ||
       (elemNum == PXTRMWIND) || (elemNum == PTOTSVR) || (elemNum == PTOTXTRM)) {
      meta->gridAttrib.ESF = 1;
   } else {
      meta->gridAttrib.ESF = 0;
   }
   meta->gridAttrib.DSF = NDFD_Values[elemNum].DSF;
   meta->gridAttrib.fieldType = NDFD_Values[elemNum].fieldType;
   meta->gridAttrib.f_maxmin = 0;
   /* Missing values are set to 9999, but if no missing values are detected
    * in the field, then f_miss is reset to 0 by pack::fillGridUnit() */
   meta->gridAttrib.f_miss = 1;
   meta->gridAttrib.missPri = 9999;
   meta->gridAttrib.missSec = 0;

   return 0;
}

void NDFD_ReverseComputeUnit (char *elem, char *unit, int *convert,
                              int *f_unit)
{
   static char *NDFD_UnitType[] = { "[C]", "[F]", "[inch]", "[feet]",
      "[knots]", NULL
   };
   enum { C2K, F2K, INCH2GRIB, Feet2M, Knots2MS };
   int unitNum;

   /* Figure out which NDFD Element. */
   if (GetIndexFromStr (unit, NDFD_UnitType, &unitNum) < 0) {
      /* Could be an already recognized GRIB2 unit. */
      /* example: [K] */
      *f_unit = 0;
      *convert = UC_NONE;
      return;
   }
   switch (unitNum) {
      case C2K:
         *f_unit = 2;
         *convert = UC_K2F;
         break;
      case F2K:
         *f_unit = 1;
         *convert = UC_K2F;
         break;
      case INCH2GRIB:
         if (strcmp (elem, "SnowAmt") == 0) {
            *f_unit = 1;
            *convert = UC_M2Inch;
         } else if (strcmp (elem, "QPF") == 0) {
            *f_unit = 1;
            *convert = UC_InchWater;
         } else {
            *f_unit = 1;
            *convert = UC_InchWater;
         }
         break;
      case Feet2M:
         *f_unit = 1;
         *convert = UC_M2Feet;
         break;
      case Knots2MS:
         *f_unit = 1;
         *convert = UC_MS2Knots;
         break;
   }
}

/* No need for unit conversion here (internal copy is in user units)...
   pack converts to local version... Only need a unit conversion if they
   want to read a float file, and do something other than pack it, since
   packing to grib2, which uses GRIB2 SI units, has a conversion step. */
int gribReadFloat (char *inName, char *dataFile, sInt4 offset,
                   uChar f_BigEndian, uChar scan, double *gribData, sInt4 Nx,
                   sInt4 Ny, gridAttribType *attrib, int startX, int startY,
                   int stopX, int stopY)
{
   char *dataName;
   char *lastSlash;
   FILE *fp;
   float value;
   int x, y;
   sInt4 subNx;         /* The Nx dimmension of the subgrid. */
   sInt4 subNy;         /* The Ny dimmension of the subgrid. */
   sInt4 localOffset;

   subNx = stopX - startX + 1;
   subNy = stopY - startY + 1;

   dataName = (char *) malloc (strlen (inName) + strlen (dataFile) + 1);
   strcpy (dataName, inName);
   if ((lastSlash = strrchr (dataName, '/')) == NULL) {
      if ((lastSlash = strrchr (dataName, '\\')) == NULL) {
         strcpy (dataName, dataFile);
      } else {
         strcpy (lastSlash + 1, dataFile);
      }
   } else {
      strcpy (lastSlash + 1, dataFile);
   }

   if ((fp = fopen (dataName, "rb")) == NULL) {
      errSprintf ("Problems opening %s for read\n", dataName);
      free (dataName);
      return 1;
   }

   attrib->f_maxmin = 0;
   attrib->numMiss = 0;
   if (attrib->f_miss == 1) {
      for (y = 0; y < subNy; y++) {
         if (((startY + y - 1) < 0) || ((startY + y - 1) >= Ny)) {
            for (x = 0; x < subNx; x++) {
               attrib->numMiss++;
               *gribData++ = attrib->missPri;
            }
         } else {
            /* (startY + y) is the row (starting at 1) from the original file
             * that we are looking at. */
            if (scan == 0) {
               localOffset = (((subNy - 1) + (startY - 1) - y) * Nx) * 4 + offset;
            } else {
               localOffset = ((startY - 1 + y) * Nx) * 4 + offset;
            }
            /* Only want to add startX - 1 if it is > 0.  Otherwise we "fill"
             * with missings before we start reading. */
            if (startX - 1 > 0) {
               localOffset += (startX - 1) * 4;
            }
            fseek (fp, localOffset, SEEK_SET);
            for (x = 0; x < subNx; x++) {
               if (((startX + x - 1) < 0) || ((startX + x - 1) >= Nx)) {
                  attrib->numMiss++;
                  *gribData++ = attrib->missPri;
               } else {
                  if (f_BigEndian) {
                     FREAD_BIG (&value, sizeof (float), 1, fp);
                  } else {
                     FREAD_LIT (&value, sizeof (float), 1, fp);
                  }
                  if (attrib->missPri == value) {
                     attrib->numMiss++;
                  } else {
                     if (attrib->f_maxmin == 0) {
                        attrib->f_maxmin = 1;
                        attrib->max = value;
                        attrib->min = value;
                     } else {
                        if (attrib->max < value) {
                           attrib->max = value;
                        }
                        if (attrib->min > value) {
                           attrib->min = value;
                        }
                     }
                  }
                  *gribData++ = value;
               }
            }
         }
      }
   } else if (attrib->f_miss == 2) {
      for (y = 0; y < subNy; y++) {
         if (((startY + y - 1) < 0) || ((startY + y - 1) >= Ny)) {
            for (x = 0; x < subNx; x++) {
               attrib->numMiss++;
               *gribData++ = attrib->missPri;
            }
         } else {
            if (scan == 0) {
               localOffset = (((subNy - 1) + (startY - 1) - y) * Nx +
                              (startX - 1)) * 4 + offset;
            } else {
               localOffset = ((startY - 1 + y) * Nx +
                              (startX - 1)) * 4 + offset;
            }
            fseek (fp, localOffset, SEEK_SET);
            for (x = 0; x < subNx; x++) {
               if (((startX + x - 1) < 0) || ((startX + x - 1) >= Nx)) {
                  attrib->numMiss++;
                  *gribData++ = attrib->missPri;
               } else {
                  if (f_BigEndian) {
                     FREAD_BIG (&value, sizeof (float), 1, fp);
                  } else {
                     FREAD_LIT (&value, sizeof (float), 1, fp);
                  }
                  if ((attrib->missPri == value) ||
                      (attrib->missSec == value)) {
                     attrib->numMiss++;
                  } else {
                     if (attrib->f_maxmin == 0) {
                        attrib->f_maxmin = 1;
                        attrib->max = value;
                        attrib->min = value;
                     } else {
                        if (attrib->max < value) {
                           attrib->max = value;
                        }
                        if (attrib->min > value) {
                           attrib->min = value;
                        }
                     }
                  }
                  *gribData++ = value;
               }
            }
         }
      }
   } else {
      for (y = 0; y < subNy; y++) {
         if (((startY + y - 1) < 0) || ((startY + y - 1) >= Ny)) {
            for (x = 0; x < subNx; x++) {
               *gribData++ = 9999;
            }
         } else {
            if (scan == 0) {
               localOffset = (((subNy - 1) + (startY - 1) - y) * Nx +
                              (startX - 1)) * 4 + offset;
            } else {
               localOffset = ((startY - 1 + y) * Nx +
                              (startX - 1)) * 4 + offset;
            }
            fseek (fp, localOffset, SEEK_SET);
            for (x = 0; x < subNx; x++) {
               if (((startX + x - 1) < 0) || ((startX + x - 1) >= Nx)) {
                  *gribData++ = 9999;
               } else {
                  if (f_BigEndian) {
                     FREAD_BIG (&value, sizeof (float), 1, fp);
                  } else {
                     FREAD_LIT (&value, sizeof (float), 1, fp);
                  }
                  if (attrib->f_maxmin == 0) {
                     attrib->f_maxmin = 1;
                     attrib->max = value;
                     attrib->min = value;
                  } else {
                     if (attrib->max < value) {
                        attrib->max = value;
                     }
                     if (attrib->min > value) {
                        attrib->min = value;
                     }
                  }
                  *gribData++ = value;
               }
            }
         }
      }
   }
   fclose (fp);
   free (dataName);
   return 0;
}

int Grib2DataConvert (userType *usr)
{
   char *flxArray = NULL; /* The index file in a char buffer. */
   int flxArrayLen;     /* The length of the flxArray buffer. */
   char *ptr;           /* A pointer to where we are in the array. */
   uShort2 numGDS;      /* number of GDS Sections. */
   uShort2 numSupPDS;   /* # of Super PDS Sections. */
   uShort2 numPDS;      /* number of PDS Sections. */
   int i;               /* Loop counter over super PDS. */
   int j;               /* Loop counter over PDS Array. */
   int k;               /* Loop counter over number of Keys */
   sInt4 index;
   sInt4 li_temp;       /* A holder for 4 byte integers. */
   char elem[256];      /* A holder for < 256 long ASCII strings. */
   char unit[256];      /* A holder for < 256 long ASCII strings. */
   char comment[256];   /* A holder for < 256 long ASCII strings. */
   uChar uc_temp;       /* A holder for char. */
   double refTime;      /* Reference time of this data set. */
   double valTime;      /* Valid time of this PDS. */
   uShort2 gdsIndex;    /* Which GDS is associated with this data. */
   uShort2 center;
   uShort2 subCenter;
   char dataFile[256];  /* A holder for the Data file for this record. */
   sInt4 offset;        /* An offset into dataFile for this record. */
   uChar f_BigEndian;   /* Endian'ness of the data grid. */
   uChar scan;          /* Scan mode for the data grid. */
   uShort2 numKeys;     /* Number of strings in the table */
   uShort2 keyLen;      /* number of bytes in a key. */
   char **keys = NULL;  /* Table of strings associated with this PDS. */
   IS_dataType is;      /* Un-parsed meta data for this GRIB2 message. As
                         * well as some memory used by the unpacker. */
   grib_MetaData meta;  /* The meta structure for this GRIB2 message. */
   double *gribData;
   uInt4 gribDataLen;
   int f_unit;
   int msgNum = 0;
   gdsType newGds;      /* The GDS of the subgrid if needed. */
   int x1, y1;          /* The original grid coordinates of the lower left
                         * corner of the subgrid. */
   int x2, y2;          /* The original grid coordinates of the upper right
                         * corner of the subgrid. */
   sInt4 Nx, Ny;        /* original size of the data. */

   if (ReadFLX (usr->inNames[0], &flxArray, &flxArrayLen) != 0) {
      errSprintf ("Problems Reading %s\n", usr->inNames[0]);
      free (flxArray);
      return 1;
   }
   if (usr->f_Print) {
      PrintFLXBuffer (flxArray, flxArrayLen);
      free (flxArray);
      return 0;
   }

   myAssert (flxArray != NULL);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (uShort2) == 2);

   IS_Init (&is);
   MetaInit (&meta);
   gribDataLen = 0;
   gribData = NULL;

   /* find out how many GDS there are, and get to the PDS data */
   ptr = flxArray + HEADLEN;
   MEMCPY_LIT (&numGDS, ptr, sizeof (uShort2));
   index = HEADLEN + numGDS * 129 + sizeof (uShort2);

   /* Get the number of PDS super headers. */
   MEMCPY_LIT (&numSupPDS, flxArray + index, sizeof (uShort2));

   index += sizeof (uShort2);
   for (i = 0; i < numSupPDS; i++) {
      ptr = flxArray + index;
      MEMCPY_LIT (&li_temp, ptr, sizeof (sInt4));
      index += li_temp;
      ptr += 4;
      ptr += 2;         /* skip size of super header. */
      uc_temp = *ptr;
      ptr++;
      memcpy (elem, ptr, uc_temp);
      elem[uc_temp] = '\0';
      ptr += uc_temp;
      MEMCPY_LIT (&refTime, ptr, sizeof (double));
      ptr += 8;
      uc_temp = *ptr;
      ptr++;
      memcpy (unit, ptr, uc_temp);
      unit[uc_temp] = '\0';
      ptr += uc_temp;
      uc_temp = *ptr;
      ptr++;
      memcpy (comment, ptr, uc_temp);
      comment[uc_temp] = '\0';
      ptr += uc_temp;
      MEMCPY_LIT (&gdsIndex, ptr, sizeof (uShort2));
      ptr += 2;
      MEMCPY_LIT (&center, ptr, sizeof (uShort2));
      ptr += 2;
      MEMCPY_LIT (&subCenter, ptr, sizeof (uShort2));
      ptr += 2;
      MEMCPY_LIT (&numPDS, ptr, sizeof (uShort2));
      ptr += 2;
      for (j = 0; j < numPDS; j++) {
         ptr += 2;      /* skip size of PDS. */
         MEMCPY_LIT (&valTime, ptr, sizeof (double));
         ptr += 8;
         uc_temp = *ptr;
         ptr++;
         memcpy (dataFile, ptr, uc_temp);
         dataFile[uc_temp] = '\0';
         ptr += uc_temp;
         MEMCPY_LIT (&offset, ptr, sizeof (sInt4));
         ptr += 4;
         f_BigEndian = *ptr;
         ptr++;
         scan = *ptr;
         ptr++;
         MEMCPY_LIT (&numKeys, ptr, sizeof (uShort2));
         ptr += 2;
         keys = (char **) malloc (numKeys * sizeof (char *));
         for (k = 0; k < numKeys; k++) {
            MEMCPY_LIT (&keyLen, ptr, sizeof (uShort2));
            ptr += 2;
            keys[k] = (char *) malloc ((keyLen + 1) * sizeof (char));
            memcpy (keys[k], ptr, keyLen);
            keys[k][keyLen] = '\0';
            ptr += keyLen;
         }

         /* Method 1... convert to meta, call convert.. which in turn
          * converts meta to is-array...  */
         if (NDFD_Cube2Meta (&meta, elem, unit, comment, center, subCenter,
                             refTime, valTime, gdsIndex, flxArray, keys,
                             numKeys, usr->f_SimpleVer) != 0) {
            errSprintf ("Problems creating meta data from %s\n",
                        usr->inNames[0]);
            free (gribData);
            free (flxArray);
            MetaFree (&meta);
            IS_Free (&is);
            for (k = 0; k < numKeys; k++) {
               free (keys[k]);
            }
            free (keys);
            return 1;
         }
         NDFD_ReverseComputeUnit (elem, unit, &(meta.convert), &f_unit);
         Nx = meta.gds.Nx;
         Ny = meta.gds.Ny;
         if ((usr->lwlf.lat != -100) && (usr->uprt.lat != -100)) {
            if (computeSubGrid (&(usr->lwlf), &x1, &y1, &(usr->uprt), &x2,
                                &y2, &(meta.gds), &newGds) != 0) {
               errSprintf ("ERROR: In compute subgrid.\n");
               free (gribData);
               free (flxArray);
               MetaFree (&meta);
               IS_Free (&is);
               for (k = 0; k < numKeys; k++) {
                  free (keys[k]);
               }
               free (keys);
               return 1;
            }
            /* I couldn't decide if I should "permanently" change the GDS or
             * not. when I wrote computeSubGrid.  If next line stays, really
             * should rewrite computeSubGrid. */
            memcpy (&(meta.gds), &newGds, sizeof (gdsType));
         } else {
            x1 = 1;
            x2 = Nx;
            y1 = 1;
            y2 = Ny;
         }

         if (gribDataLen != meta.gds.Nx * meta.gds.Ny) {
            gribDataLen = meta.gds.Nx * meta.gds.Ny;
            gribData = (double *) realloc (gribData,
                                           gribDataLen * sizeof (double));
         }
         if (gribReadFloat (usr->inNames[0], dataFile, offset, f_BigEndian,
                            scan, gribData, Nx, Ny, &(meta.gridAttrib), x1,
                            y1, x2, y2) != 0) {
            preErrSprintf ("ERROR: In gribReadFloat.\n");
            free (gribData);
            free (flxArray);
            MetaFree (&meta);
            IS_Free (&is);
            for (k = 0; k < numKeys; k++) {
               free (keys[k]);
            }
            free (keys);
            return 1;
         }

         if (MainConvert (usr, &is, &meta, gribData, gribDataLen, f_unit,
                          msgNum) != 0) {
            preErrSprintf ("ERROR: In MainConvert.\n");
            free (gribData);
            free (flxArray);
            MetaFree (&meta);
            IS_Free (&is);
            for (k = 0; k < numKeys; k++) {
               free (keys[k]);
            }
            free (keys);
            return 1;
         }
         msgNum++;

         for (k = 0; k < numKeys; k++) {
            free (keys[k]);
         }
         free (keys);
         MetaSect2Free (&meta);
      }
   }
   free (gribData);
   free (flxArray);
   MetaFree (&meta);
   IS_Free (&is);
   return 0;
}
