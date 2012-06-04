/*****************************************************************************
 * commands.c
 *
 * DESCRIPTION
 *    This file contains the code for the main command type options.  This is
 * mostly a consolidation of what was in "cstart.c" and the "tcldegrib.c".
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "myassert.h"
#include "myerror.h"
#include "myutil.h"
#include "tendian.h"
#include "meta.h"
#include "metaname.h"
#include "degrib2.h"
#include "weather.h"
#include "inventory.h"
#include "split.h"
#include "probe.h"
#include "interp.h"
#include "write.h"
#include "userparse.h"
#include "cube.h"
#include "commands.h"
#include "database.h"
#include "pack.h"
#include "tdlpack.h"
#include "clock.h"
#include "sector.h"
#include "drawgrib.h"
#ifdef _DWML_
#include "xmlparse.h"
#endif
#include "grpprobe.h"

/*****************************************************************************
 * GetOutputName() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Figures out what name to give the output file.  Uses realloc on *buffer.
 *
 * ARGUMENTS
 *     usr = The user option structure to use while Degrib'ing. (Input)
 *    meta = The meta data from the last GRIB2 message read. (Input)
 *  buffer = Where to store the name. (Output)
 * buffLen = String length of buffer. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 *  1 = Invalid usage.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *   9/2003 Peter (peterr@digitalcyclone.com) bug fix for "nameStyle 1"
 *          Allow delta (ref-valid) of 3 characters (instead of 2)
 *  11/2003 Wyatt Miler bug report (name style 3 did not have the size
 *          correct (1 extra in strlen)
 *   7/2004 AAT: Adjusted to convert decimal surface levels (replaces the
 *          '.' in 0.333 with a '_'
 *
 * NOTES
 *   Assumes *buffer is correctly initialized, so it can use realloc.
 *****************************************************************************
 */
int GetOutputName (userType *usr, grib_MetaData *meta, char **buffer,
                   size_t *buffLen)
{
   char c_temp;         /* Used to help create '%lv' option. */
   char *ptr;
   char *ptr2;
   char lclBuff[3];     /* Used to help create '%c' option. */
   int Day;             /* The valid day of the forecast. today is day 1.
                         * yesterday is day -1. tomorrow is day 2. */
   int Remainder;       /* Seconds between the beginning of reference day and 
                         * reference time. */

   myAssert (sizeof (char) == 1);
   if (usr->outName == NULL) {
      if (*buffLen != 0) {
         free (*buffer);
         *buffer = NULL;
         *buffLen = 0;
      }
      myAssert (*buffer == NULL);
      if (usr->namePath != NULL) {
         reallocSprintf (buffer, "%s/", usr->namePath);
      }
      myAssert (usr->nameStyle != NULL);
      for (ptr = usr->nameStyle; *ptr != '\0'; ptr++) {
         if (*ptr != '%') {
            reallocSprintf (buffer, "%c", *ptr);
         } else {
            ptr++;
            switch (*ptr) {
               case '\0':
                  break;
               case 'c':
                  sprintf (lclBuff, "%s", meta->validTime + 8);
                  lclBuff[2] = '\0';
                  reallocSprintf (buffer, "%s", lclBuff);
                  break;
               case 'D':
               case 'd':
                  if (meta->GribVersion == 2) {
                     Remainder = (int) meta->pds2.refTime;
                  } else if (meta->GribVersion == 1) {
                     Remainder = (int) meta->pds1.refTime;
                  } else if (meta->GribVersion == -1) {
                     Remainder = (int) meta->pdsTdlp.refTime;
                  } else {
                     Remainder = 0;
                  }
                  Remainder = Remainder % (24 * 3600);
                  Day = (meta->deltTime + Remainder) / (24 * 3600);
                  if (Day >= 0) {
                     if (*ptr == 'D') {
                        reallocSprintf (buffer, "%04d", Day);
                     } else {
                        reallocSprintf (buffer, "%d", Day);
                     }
                  } else {
                     Day *= -1;
                     if (*ptr == 'D') {
                        reallocSprintf (buffer, "M%03d", Day);
                     } else {
                        reallocSprintf (buffer, "M%d", Day);
                     }
                  }
                  break;
               case 'e':
                  reallocSprintf (buffer, "%s", meta->element);
                  break;
               case 'E':
                  if (meta->GribVersion == 1) {
                     if (meta->pds1.f_hasEns) {
                        reallocSprintf (buffer, "%d-%d-%d",
                                        meta->pds1.ens.Type,
                                        meta->pds1.ens.Number,
                                        meta->pds1.ens.ProdID);
                     }
                  }
                  break;
               case 'V':
                  reallocSprintf (buffer, "%s", meta->validTime);
                  break;
               case 'A':
                  reallocSprintf (buffer, "%03d", meta->center);
                  break;
               case 'a':
                  reallocSprintf (buffer, "%03d", meta->subcenter);
                  break;
               case 'g':
                  if (meta->GribVersion == 2) {
                     if (meta->pds2.sect4.genID == GRIB2MISSING_u1) {
                        reallocSprintf (buffer, "%03d",
                                        meta->pds2.sect4.bgGenID);
                     } else {
                        reallocSprintf (buffer, "%03d",
                                        meta->pds2.sect4.genID);
                     }
                  } else if (meta->GribVersion == 1) {
                     reallocSprintf (buffer, "%03d", meta->pds1.genProcess);
                  } else if (meta->GribVersion == -1) {
                     reallocSprintf (buffer, "%03d", meta->pdsTdlp.DD);
                  }
                  break;
               case 'R':
                  reallocSprintf (buffer, "%s", meta->refTime);
                  break;
               case 'p':
                  reallocSprintf (buffer, "%03ld", (meta->deltTime + 1800)
                                  / 3600);
                  break;
               case 'y':
                  reallocSprintf (buffer, "%03.0f", fabs (meta->gds.lat1));
                  break;
               case 'x':
                  reallocSprintf (buffer, "%03.0f", fabs (meta->gds.lon1));
                  break;
               case 'v':
                  reallocSprintf (buffer, "%s", meta->validTime + 4);
                  break;
               case 's':
                  ptr2 = (char *) malloc (strlen (meta->shortFstLevel) + 1);
                  strcpy (ptr2, meta->shortFstLevel);
                  strReplace (ptr2, '.', '_');
                  reallocSprintf (buffer, "%s", ptr2);
                  free (ptr2);
                  break;
               case 'l':
                  if (*(ptr + 1) == 'v') {
                     c_temp = meta->validTime[10];
                     meta->validTime[10] = '\0';
                     reallocSprintf (buffer, "%s", meta->validTime);
                     meta->validTime[10] = c_temp;
                     ptr++;
                  } else {
                     reallocSprintf (buffer, "%c", *ptr);
                  }
                  break;
               default:
                  reallocSprintf (buffer, "%c", *ptr);
                  break;
            }
         }
      }
      *buffLen = strlen (*buffer);
   } else if (usr->f_stdout) {
      /* Probably don't need this in -out stdout, but it is safer. */
      reallocSprintf (buffer, "sample.shp");
      *buffLen = strlen (*buffer);
   } else {
      *buffLen = strlen (usr->outName);
      if (*buffLen < 4) {
         errSprintf ("ERROR: File %s is too short in length (it may need an "
                     "extension?)", usr->outName);
         return 1;
      }
      *buffer = (char *) realloc ((void *) *buffer, *buffLen + 1);
      strcpy (*buffer, usr->outName);
      (*buffer)[*buffLen] = '\0';
   }
   return 0;
}

#ifdef TESTING
int SimpConvert (userType *usr, grib_MetaData *meta, double *Data,
                 sInt4 DataLen)
{
   char *outName = NULL; /* Name of the output file */
   size_t outLen;       /* String length of outName. */
   int logFirst;        /* flag to put header info in the log file. */
   FILE *fp;            /* File Pointer for various types of output. */
   char *msg;           /* Used to print the error stack, and return version
                         * info. */
   uChar FltScan;       /* Scan mode to use for the .flt/.tlf file. */
   char *cPack;         /* Used to store packed message during test. */
   sInt4 c_len;         /* length of cPack */
   int i;               /* loop counter. */
   int j;               /* Loop counter used to save to .is0 file. */
   int f_continue;      /* Flag to continue the Meta Print Command. */

   /* Figure out the output filename. */
   if (GetOutputName (usr, meta, &outName, &outLen) != 0) {
      free (outName);
      return 1;
   }
   myAssert ((outLen == 0) || (strlen (outName) == outLen));

   /* Create shp file set. */
   if (usr->f_Shp) {
      if (gribWriteShp (outName, Data, meta, usr->f_poly,
                        usr->f_nMissing, usr->decimal, usr->LatLon_Decimal,
                        usr->f_verboseShp)
          != 0) {
         free (outName);
         return 1;
      }
   }

   free (outName);
   return 0;
}
#endif

int MainConvert (userType *usr, IS_dataType *is, grib_MetaData *meta,
                 double *Data, sInt4 DataLen, int f_unit, int f_first)
{
   char *outName = NULL; /* Name of the output file */
   size_t outLen;       /* String length of outName. */
   int logFirst;        /* flag to put header info in the log file. */
   FILE *fp;            /* File Pointer for various types of output. */
   char *msg;           /* Used to print the error stack, and return version
                         * info. */
   uChar FltScan;       /* Scan mode to use for the .flt/.tlf file. */
   uChar *cPack;        /* Used to store packed message during test. */
   sInt4 c_len;         /* length of cPack */
   size_t i;            /* loop counter. */
   int j;               /* Loop counter used to save to .is0 file. */
   int f_continue;      /* Flag to continue the Meta Print Command. */

   /* Figure out the output filename. */
   if (GetOutputName (usr, meta, &outName, &outLen) != 0) {
      free (outName);
      return 1;
   }
   myAssert ((outLen == 0) || (strlen (outName) == outLen));

   /* log any problems with the weather strings. */
   if (usr->logName != NULL) {
      logFirst = 1;
      if ((meta->pds2.f_sect2) && (meta->pds2.sect2.ptrType == GS2_WXTYPE)) {
         for (i = 0; i < meta->pds2.sect2.wx.dataLen; i++) {
            if (meta->pds2.sect2.wx.ugly[i].errors != NULL) {
               if ((fp = fopen (usr->logName, "at")) != NULL) {
                  if (logFirst) {
                     fprintf (fp, "refTime %s valTime %s\n", meta->refTime,
                              meta->validTime);
                     logFirst = 0;
                  }
                  fprintf (fp, "%s", meta->pds2.sect2.wx.ugly[i].errors);
                  fclose (fp);
               }
            }
         }
      }
   }

   /* Print out meta data. */
   if (usr->f_Met) {
      f_continue = 1;
      if (usr->f_stdout) {
         fp = stdout;
      } else {
         strncpy (outName + outLen - 3, "txt", 3);
         if ((fp = fopen (outName, "wt")) == NULL) {
            f_continue = 0;
         }
      }
      if (f_continue) {
         if (MetaPrint (meta, &msg, usr->decimal, usr->f_unit) != 0) {
            fprintf (fp, "%s", msg);
            free (msg);
            if (!usr->f_stdout) {
               fclose (fp);
            }
            free (outName);
            return 1;
         }
         /* Can't use fprintf (fp, msg) here, because "msg" may have some
          * unprotected '%' in it, instead use (fp, "%s", msg) */
         fprintf (fp, "%s", msg);
         free (msg);
         if (!usr->f_stdout) {
            fclose (fp);
         }
      }
      f_continue = 1;
   }

   /* Print out .is0 info if requested. */
   if (usr->f_IS0) {
      f_continue = 1;
      if (usr->f_stdout) {
         fp = stdout;
      } else {
         strncpy (outName + outLen - 3, "is0", 3);
         if ((fp = fopen (outName, "wt")) == NULL) {
            f_continue = 0;
         }
      }
      if (f_continue) {
         for (i = 0; i < 8; i++) {
            fprintf (fp, "---Section %d---\n", i);
            for (j = 1; j <= is->ns[i]; j++) {
               fprintf (fp, "IS%d Item %d = %ld\n", i, j, is->is[i][j - 1]);
            }
         }
         if (!usr->f_stdout) {
            fclose (fp);
         }
      }
   }

   /* Print out .frq info if requested. */
   if (usr->f_Freq) {
      f_continue = 1;
      if (usr->f_stdout) {
         fp = stdout;
      } else {
         strncpy (outName + outLen - 3, "frq", 3);
         if ((fp = fopen (outName, "wt")) == NULL) {
            f_continue = 0;
         }
      }
      if (f_continue) {
         msg = NULL;
         FreqPrint (&msg, Data, DataLen, meta->gds.Nx, meta->gds.Ny,
                    usr->decimal, meta->comment);
         /* Can't use fprintf (fp, msg) here, because "msg" may have some
          * unprotected '%' in it, instead use (fp, "%s", msg) */
         fprintf (fp, "%s", msg);
         free (msg);
         if (!usr->f_stdout) {
            fclose (fp);
         }
      }
   }

   /* Create flt file set. */
   if (usr->f_Flt) {
      /* Determine if the lower left or upper left is the 0,0 value. */
      if (usr->f_revFlt) {
         FltScan = GRIB2BIT_2;
      } else {
         FltScan = 0;
      }
      /*
       * Determine if we are creating an un-projected (lat/lon) coverage grid
       * using bi-linear interpolation or creating a normal "projected" grid,
       * which is true to the data since it has no interpolation.
       */
      if (usr->f_coverageGrid) {
         if (gribInterpFloat (outName, Data, meta, &(meta->gridAttrib),
                              FltScan, usr->f_MSB, usr->decimal,
                              usr->f_GrADS, usr->f_SimpleWx,
                              usr->f_interp, usr->f_AscGrid,
                              usr->f_avgInterp) != 0) {
            free (outName);
            return 1;
         }
      } else {
         if (gribWriteFloat (outName, Data, meta, &(meta->gridAttrib),
                             FltScan, usr->f_MSB, usr->decimal, usr->f_GrADS,
                             usr->f_SimpleWx, usr->f_AscGrid) != 0) {
            free (outName);
            return 1;
         }
      }
   }

   /* Create shp file set. */
   if (usr->f_Shp) {
      if (gribWriteShp (outName, Data, meta, usr->f_poly,
                        usr->f_nMissing, usr->decimal, usr->LatLon_Decimal,
                        usr->f_verboseShp)
          != 0) {
         free (outName);
         return 1;
      }
   }

   /* Create kml file set. */
   if (usr->f_Kml) {
      /* The -1 is because f_kmz is 1 for kmz, 0 for kml,
       * while ->f_Kml is 0 none, 1 kml, 2 kmz. */
      if (gribWriteKml (outName, Data, meta, usr->f_poly,
                        usr->f_nMissing, usr->decimal, usr->LatLon_Decimal,
                        usr->kmlIniFile, usr->f_Kml - 1, usr->f_kmlMerge)
          != 0) {
         free (outName);
         return 1;
      }
   }

   /* Create Map . */
   if (usr->f_Map) {
      if (drawGrib (outName, Data, usr->mapIniFile, usr->mapIniOptions,
                    &(meta->gds), meta->gridAttrib.min, meta->gridAttrib.max,
                    meta->gridAttrib.f_miss, meta->gridAttrib.missPri,
                    meta, usr) != 0) {
         free (outName);
         return 1;
      }
   }

   /* Create the .csv file */
   if (usr->f_Csv) {
      f_continue = 1;
      if (usr->f_stdout) {
         fp = stdout;
      } else {
         strncpy (outName + outLen - 3, "csv", 3);
         if ((fp = fopen (outName, "wt")) == NULL) {
            f_continue = 0;
         }
      }
      if (f_continue) {
         if (gribWriteCsv (fp, Data, meta, usr->decimal, usr->separator,
                           usr->logName, usr->f_WxParse,
                           usr->f_nMissing, usr->LatLon_Decimal) != 0) {
            free (outName);
            if (!usr->f_stdout) {
               fclose (fp);
            }
            return 1;
         }
         if (!usr->f_stdout) {
            fclose (fp);
         }
      }
   }

   /* Create the .nc file (NetCDF) */
   if (usr->f_NetCDF) {
      strncpy (outName + strlen (outName) - 3, "nc\0", 3);
      if (gribWriteNetCDF (outName, Data, meta, usr->f_NetCDF,
                           usr->decimal, usr->LatLon_Decimal) != 0) {
         free (outName);
         return 1;
      }
      strncpy (outName + strlen (outName) - 2, "txt", 3);
   }

   /* Create the .tdl file */
   if ((usr->f_Tdl) && (meta->GribVersion == -1)) {
      if (usr->f_stdout) {
         if (meta->gridAttrib.f_miss == 0) {
            WriteTDLPRecord (stdout, Data, DataLen, meta->gridAttrib.DSF,
                             meta->gridAttrib.ESF, 0, 0, 0, 0, &(meta->gds),
                             meta->pdsTdlp.Descriptor, meta->pdsTdlp.refTime,
                             meta->pdsTdlp.ID1, meta->pdsTdlp.ID2,
                             meta->pdsTdlp.ID3, meta->pdsTdlp.ID4,
                             meta->pdsTdlp.project, meta->pdsTdlp.procNum,
                             meta->pdsTdlp.seqNum);
         } else if (meta->gridAttrib.f_miss == 1) {
            WriteTDLPRecord (stdout, Data, DataLen, meta->gridAttrib.DSF,
                             meta->gridAttrib.ESF, 1,
                             meta->gridAttrib.missPri, 0, 0, &(meta->gds),
                             meta->pdsTdlp.Descriptor, meta->pdsTdlp.refTime,
                             meta->pdsTdlp.ID1, meta->pdsTdlp.ID2,
                             meta->pdsTdlp.ID3, meta->pdsTdlp.ID4,
                             meta->pdsTdlp.project, meta->pdsTdlp.procNum,
                             meta->pdsTdlp.seqNum);
         } else {
            WriteTDLPRecord (stdout, Data, DataLen, meta->gridAttrib.DSF,
                             meta->gridAttrib.ESF, 1,
                             meta->gridAttrib.missPri, 1,
                             meta->gridAttrib.missSec, &(meta->gds),
                             meta->pdsTdlp.Descriptor, meta->pdsTdlp.refTime,
                             meta->pdsTdlp.ID1, meta->pdsTdlp.ID2,
                             meta->pdsTdlp.ID3, meta->pdsTdlp.ID4,
                             meta->pdsTdlp.project, meta->pdsTdlp.procNum,
                             meta->pdsTdlp.seqNum);
         }
      } else {
         strncpy (outName + strlen (outName) - 3, "tdl", 3);
         if (f_first) {
            if ((fp = fopen (outName, "wb")) != NULL) {
               if (meta->gridAttrib.f_miss == 0) {
                  WriteTDLPRecord (fp, Data, DataLen, meta->gridAttrib.DSF,
                                   meta->gridAttrib.ESF, 0, 0, 0, 0,
                                   &(meta->gds), meta->pdsTdlp.Descriptor,
                                   meta->pdsTdlp.refTime, meta->pdsTdlp.ID1,
                                   meta->pdsTdlp.ID2, meta->pdsTdlp.ID3,
                                   meta->pdsTdlp.ID4, meta->pdsTdlp.project,
                                   meta->pdsTdlp.procNum,
                                   meta->pdsTdlp.seqNum);
               } else if (meta->gridAttrib.f_miss == 1) {
                  WriteTDLPRecord (fp, Data, DataLen, meta->gridAttrib.DSF,
                                   meta->gridAttrib.ESF, 1,
                                   meta->gridAttrib.missPri, 0, 0,
                                   &(meta->gds), meta->pdsTdlp.Descriptor,
                                   meta->pdsTdlp.refTime, meta->pdsTdlp.ID1,
                                   meta->pdsTdlp.ID2, meta->pdsTdlp.ID3,
                                   meta->pdsTdlp.ID4, meta->pdsTdlp.project,
                                   meta->pdsTdlp.procNum,
                                   meta->pdsTdlp.seqNum);
               } else {
                  WriteTDLPRecord (fp, Data, DataLen, meta->gridAttrib.DSF,
                                   meta->gridAttrib.ESF, 1,
                                   meta->gridAttrib.missPri, 1,
                                   meta->gridAttrib.missSec, &(meta->gds),
                                   meta->pdsTdlp.Descriptor,
                                   meta->pdsTdlp.refTime, meta->pdsTdlp.ID1,
                                   meta->pdsTdlp.ID2, meta->pdsTdlp.ID3,
                                   meta->pdsTdlp.ID4, meta->pdsTdlp.project,
                                   meta->pdsTdlp.procNum,
                                   meta->pdsTdlp.seqNum);
               }
               fclose (fp);
            }
         } else {
            if ((fp = fopen (outName, "ab")) != NULL) {
               if (meta->gridAttrib.f_miss == 0) {
                  WriteTDLPRecord (fp, Data, DataLen, meta->gridAttrib.DSF,
                                   meta->gridAttrib.ESF, 0, 0, 0, 0,
                                   &(meta->gds), meta->pdsTdlp.Descriptor,
                                   meta->pdsTdlp.refTime, meta->pdsTdlp.ID1,
                                   meta->pdsTdlp.ID2, meta->pdsTdlp.ID3,
                                   meta->pdsTdlp.ID4, meta->pdsTdlp.project,
                                   meta->pdsTdlp.procNum,
                                   meta->pdsTdlp.seqNum);
               } else if (meta->gridAttrib.f_miss == 1) {
                  WriteTDLPRecord (fp, Data, DataLen, meta->gridAttrib.DSF,
                                   meta->gridAttrib.ESF, 1,
                                   meta->gridAttrib.missPri, 0, 0,
                                   &(meta->gds), meta->pdsTdlp.Descriptor,
                                   meta->pdsTdlp.refTime, meta->pdsTdlp.ID1,
                                   meta->pdsTdlp.ID2, meta->pdsTdlp.ID3,
                                   meta->pdsTdlp.ID4, meta->pdsTdlp.project,
                                   meta->pdsTdlp.procNum,
                                   meta->pdsTdlp.seqNum);
               } else {
                  WriteTDLPRecord (fp, Data, DataLen, meta->gridAttrib.DSF,
                                   meta->gridAttrib.ESF, 1,
                                   meta->gridAttrib.missPri, 1,
                                   meta->gridAttrib.missSec, &(meta->gds),
                                   meta->pdsTdlp.Descriptor,
                                   meta->pdsTdlp.refTime, meta->pdsTdlp.ID1,
                                   meta->pdsTdlp.ID2, meta->pdsTdlp.ID3,
                                   meta->pdsTdlp.ID4, meta->pdsTdlp.project,
                                   meta->pdsTdlp.procNum,
                                   meta->pdsTdlp.seqNum);
               }
               fclose (fp);
            }
         }
      }
   }

   /* Create the .grb file */
   if (usr->f_Grib2) {
      /* The following is predominantly done to allow comparison to the data
       * cube, which uses packType based on the variable (rather than on what
       * was previously determined to be the best.) */
      if (IsData_NDFD (meta->center, meta->subcenter)) {
         if ((strcmp (meta->element, "QPF") == 0) ||
             (strcmp (meta->element, "SnowAmt") == 0) ||
             (strcmp (meta->element, "WaveHt") == 0)) {
            meta->gridAttrib.packType = 2;
         } else {
            meta->gridAttrib.packType = 3;
         }
      }
      if (WriteGrib2Record2 (meta, Data, DataLen, is, f_unit, &cPack, &c_len,
                             usr->f_stdout) != 0) {
         free (cPack);
         free (outName);
         return 1;
      }
/*
      if (WriteGrib2Record (meta, Data, DataLen, is, f_unit, &cPack, &c_len,
                            usr->f_stdout) != 0) {
         free (cPack);
         free (outName);
         return 1;
      }
*/
      if (usr->f_stdout) {
         fwrite (cPack, sizeof (char), c_len, stdout);
      } else {
         strncpy (outName + strlen (outName) - 3, "grb", 3);
         if (f_first) {
            if ((fp = fopen (outName, "wb")) != NULL) {
               fwrite (cPack, sizeof (char), c_len, fp);
               fclose (fp);
            }
         } else {
            if ((fp = fopen (outName, "ab")) != NULL) {
               fwrite (cPack, sizeof (char), c_len, fp);
               fclose (fp);
            }
         }
      }
      free (cPack);
   }

   free (outName);
   return 0;
}

/******************************************************************************
 *StormTotal() --
 *
 * Michael Allard / MDL
 *
 * PURPOSE
 *    Add values from several different msgs into a total at each point.
 *    Must include a startTime and endTime and an optional CONVERT
 *    OPTION.  Currently available for only QPF, SnowAmt, and IceAccum grids.
 *    Times are exlusive.  For example, -startTime '2012-05-25T01:00:00' will
 *    not include the 6-hour window data that ends at 2012-05-25T06:00:00,
 *    whereas '2012-05-25T00:00:00' will.
 * 
 * ARGUMENTS
 *     usr = The user option structure to use while Degrib'ing. (Input)
 * grib_fp = The opened GRIB2 file to read from (Input)
 *      is = memory for the Un-parsed meta data for this GRIB2 message.
 *           As well as some memory used by the unpacker. (Output)
 *    meta = memory for the meta data from last GRIB2 message read. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 *  1 = Invalid usage.
 *
 * HISTORY
 *   5/2012 Michael Allard (MDL): Created.
 *
 * NOTES
 *
 ******************************************************************************
 */
static int StormTotal (userType *usr, FILE *grib_fp, IS_dataType *is, 
                       grib_MetaData *meta) 
{
   sInt4 f_endMsg = 1;  /* 1 if we read the last grid in a GRIB message, or
                         * we haven't read any messages. */
   int subgNum;         /* The subgrid that we are looking for */
   int i=0;
   int f_first = 1;     /* A hack to make the MainConvert work. */
   uInt4 grib_DataLen = 0;  /* Size of Grib_Data. */
   double *grib_Data = NULL;   /* The read in GRIB2 grid. */
   double *grib_total = NULL; 
   char *prev_element = NULL; /* The element string from the previous msg. Used for comparsion. */
   char * prev_unitName = NULL;  /* The unitName string from the previous msg. Used for comparsion. */
   int prev_mapProj=0;  /* The mapProj from the previous msg. Used for comparsion. */
   int error=0;
   int c;               /* Determine if end of the file without fileLen. */
   double msgEnd = 0;   /* End of time for current message.  Initialized to 1/1/1970. */
   double msgStart = 0; /* Start of time for current message.  Initialized to 1/1/1970. */
   int f_haveMsg = 0;   /* True if we have a message in the valid time range. */
   int f_haveMsgBeforeEnd = 0; /* True if we have a message before the end time. */
   int msgLen;
   int firstTime = 0; /* The first time of the data being used in totaling of the data */
   int timeRange = 0; /* The time range of the data being totalled */
   char *buffer;
    
   /* Set up inital state of data for unpacker. */
   if (usr->msgNum == 0) {
      subgNum = 0;
   } else {
      subgNum = usr->subgNum;
   }

/* Start loop for all messages. */
   while (((c = fgetc (grib_fp)) != EOF) || (f_endMsg != 1)) {
      if (c != EOF) {
         ungetc (c, grib_fp);
      }

      /* Read the GRIB message. */
      if (ReadGrib2Record (grib_fp, usr->f_unit, &grib_Data, &grib_DataLen,
                           meta, is, subgNum, usr->majEarth, usr->minEarth,
                           usr->f_SimpleVer, usr->f_SimpleWWA, &f_endMsg, &(usr->lwlf),
                           &(usr->uprt)) != 0) {
         preErrSprintf ("ERROR: In call to ReadGrib2Record.\n");
         error=1;
         break;
      }
      /* Test that the version of the message is valid. */
      if (meta->GribVersion != 2) {
         preErrSprintf ("ERROR: Can only handle GRIB2 messages.\n");
         error=1;
         break;
      }

      /* We've tested for the assumption that the type of message is GRIB2 (as opposed to GRIB1 or TDLP) */
      msgEnd = meta->pds2.sect4.validTime;

      if (meta->pds2.sect4.numInterval != 1) {
         preErrSprintf ("ERROR: Expecting messages to contain a single time range rather than be instantaneous.\n");
         error=3;
         break;
      }
      msgLen = meta->pds2.sect4.Interval[0].lenTime;
      msgStart = msgEnd - msgLen * 3600;

      /* Test if the current time from the file is greater than the user's endTime. */

/* Uncomment the following if we want inclusive EndTime. */
/*      if (msgStart > usr->endTime) { */
/* Uncomment the following if we want to exclude EndTime. */
       if (msgEnd > usr->endTime) {  

         break;
         /* Assumes that the file is sequential in validTimes so we can break the loop.  */
      }
      f_haveMsgBeforeEnd = 1;

      /* Handle adding multiple messages together. */

/* Uncomment the following if we want inclusive StartTime. */
/*      if (msgEnd < usr->startTime) { */
/* Uncomment the following if we want to exclude StartTime. */
      if (msgStart < usr->startTime) { 

         continue;
         /* We need to see if the next message is >= startTime, so we can't break the loop yet. */
      }
      f_haveMsg = 1;


      /* Make sure we have memory and that it is initialized. */
      if (grib_total == NULL) {
         prev_element = malloc ((strlen (meta->element) + 1) * sizeof (char));
         strcpy (prev_element,meta->element);
         prev_unitName = malloc ((strlen (meta->unitName) + 1) * sizeof (char)); 
         strcpy (prev_unitName,meta->unitName);
         prev_mapProj = meta->gds.projType; 
         /* Assumes that grib_DataLen doesn't change between grib messages. */
         grib_total = malloc (grib_DataLen * sizeof (double));
         for (i = 0; i < grib_DataLen; i++) {
            grib_total[i] = 0; 
              /* Assumes 9999 is missing.  'meta' structure should be used instead
             * to determine the misssing value. */
         }
         firstTime = msgStart; 
      /*  Checking to see if the previous msg and this msg's elements are the same. If not we cannot do a stormTotal */
      } else if (strcmp (prev_element, meta->element) != 0) {
         preErrSprintf ("Msgs have different element types.  A stormTotal cannot be performed.\n"); 
         error=1;
         break;
      } else if (strcmp (prev_unitName, meta->unitName) != 0) {
         preErrSprintf ("Msgs have different unitName types.  A stormTotal cannot be performed.\n");
         error=1;
         break;
      }  else if (prev_mapProj != meta->gds.projType) { 
         preErrSprintf ("Msgs have different map projections.  A stormTotal cannot be performed.\n");
         error=1;
         break;
      }  

      /* Diagnostic to indicate which messages were included... */
      printf ("Including message with valid Time %s\n", meta->validTime);

      for (i = 0; i < grib_DataLen; i++) {
         if (grib_Data[i] == meta->gridAttrib.missPri){
            grib_total[i] = meta->gridAttrib.missPri;
         } 
         else if (grib_Data[i] != meta->gridAttrib.missPri) {
            grib_total[i] = grib_total[i] + grib_Data[i];
         } 
      } 

      timeRange = (double)msgEnd - firstTime;

   }

   /* Test the msgEnd to see if we have an error. */
   if (msgEnd == 0) {
      preErrSprintf ("ERROR: We didn't have any messages.\n");
      error = 2;

   } else if (msgEnd < usr->startTime) {
      preErrSprintf ("ERROR: All messages in the file were before the StartTime.\n");
      error = 2;

   } else if (msgEnd > usr->endTime) {
      if (! f_haveMsg) {
         if (f_haveMsgBeforeEnd) {
            preErrSprintf ("ERROR: Couldn't find message in range of StartTime to EndTime.\n");
         } else {
            preErrSprintf ("ERROR: All messages in the file were after the EndTime.\n");
         }
         error = 2;
      }
   }  

   if (error==0) {
      /* set the validTime and lenTime for the outgoing message. */
      /* firstTime is set with the first message kept. */
      /* timeRange is updated each time we keep a message. */
      meta->pds2.sect4.validTime = firstTime + timeRange;
      Clock_Print (meta->validTime, 20, meta->pds2.sect4.validTime, "%Y%m%d%H%M", 0);
      meta->pds2.sect4.Interval[0].lenTime = timeRange/3600;

      buffer = malloc ((5 + strlen (meta->element) + 1) * sizeof (char));
      sprintf (buffer, "Total%s", meta->element);
      free (meta->element);
      meta->element = buffer;

      if (MainConvert (usr, is, meta, grib_total, grib_DataLen,
                       usr->f_unit, f_first) != 0) {
         preErrSprintf ("ERROR: In call to MainConvert.\n");
         error=1;
      }
   }

   free (grib_Data);
   free (grib_total);
   free (prev_element);
   free (prev_unitName);
   return error;
}

/*****************************************************************************
 * Grib2Convert() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Controls converting a GRIB2 file from one format to another based on the
 * options passed in "usr".
 *
 * ARGUMENTS
 *     usr = The user option structure to use while Degrib'ing. (Input)
 * grib_fp = The opened GRIB2 file to read from (Input)
 *      is = memory for the Un-parsed meta data for this GRIB2 message.
 *           As well as some memory used by the unpacker. (Output)
 *    meta = memory for the meta data from last GRIB2 message read. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 *  1 = Invalid usage.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int Grib2Convert (userType *usr, FILE *grib_fp, IS_dataType *is,
                  grib_MetaData *meta)
{
   double *grib_Data;   /* The read in GRIB2 grid. */
   uInt4 grib_DataLen;  /* Size of Grib_Data. */
   int c;               /* Determine if end of the file without fileLen. */
   sInt4 f_endMsg = 1;  /* 1 if we read the last grid in a GRIB message, or
                         * we haven't read any messages. */
   int subgNum;         /* The subgrid that we are looking for */
   int msgNum = 1;      /* The message number we are working on. */
   int f_first = 1;     /* Is this the first message? */

   if ((!usr->f_Met) && (!usr->f_IS0) && (!usr->f_Flt) && (!usr->f_Shp) &&
       (!usr->f_Csv) && (!usr->f_Grib2) && (!usr->f_NetCDF) &&
       (!usr->f_Map) && (!usr->f_Freq)) {
      errSprintf ("You did not choose what to convert it to.\n");
      errSprintf ("You need one or more of:\n");
      errSprintf ("('-Flt', '-Met', '-IS0', '-Shp', '-Csv', '-Grib2', "
                  "'-NetCDF 1,2,3', '-Map')");
      return 1;
   }

   /* Set up inital state of data for unpacker. */
   grib_DataLen = 0;
   grib_Data = NULL;
   if (usr->msgNum == 0) {
      subgNum = 0;
   } else {
      subgNum = usr->subgNum;
   }

/* Start loop for all messages. */
   while (((c = fgetc (grib_fp)) != EOF) || (f_endMsg != 1)) {
      if (c != EOF) {
         ungetc (c, grib_fp);
      }
      /* Read the GRIB message. */
      if (ReadGrib2Record (grib_fp, usr->f_unit, &grib_Data, &grib_DataLen,
                           meta, is, subgNum, usr->majEarth, usr->minEarth,
                           usr->f_SimpleVer, usr->f_SimpleWWA, &f_endMsg, &(usr->lwlf),
                           &(usr->uprt)) != 0) {
         preErrSprintf ("ERROR: In call to ReadGrib2Record.\n");
         free (grib_Data);
         return 1;
      }
      if (usr->f_validRange > 0) {
         /* valid max. */
         if (usr->f_validRange > 1) {
            if (meta->gridAttrib.max > usr->validMax) {
               errSprintf ("ERROR: %f > valid Max of %f\n",
                           meta->gridAttrib.max, usr->validMax);
               free (grib_Data);
               return 1;
            }
         }
         /* valid min. */
         if (usr->f_validRange % 2) {
            if (meta->gridAttrib.min < usr->validMin) {
               errSprintf ("ERROR: %f < valid Min of %f\n",
                           meta->gridAttrib.min, usr->validMin);
               free (grib_Data);
               return 1;
            }
         }
      }
/*
      if ((usr->lwlf.lt != -100) && (usr->uprt.lt != -100)) {
*/
      /* compute the subgrid. */
/*
         if (computeSubGrid (&(usr->lwlf), &x1, &y1, &(usr->uprt), &x2, &y2,
                             meta->gds, &newGds) != 0) {
            preErrSprintf ("ERROR: In compute subgrid.\n");
            free (grib_Data);
            return 1;
         }
      }
*/
      if (MainConvert (usr, is, meta, grib_Data, grib_DataLen,
                       usr->f_unit, f_first) != 0) {
         preErrSprintf ("ERROR: In call to MainConvert.\n");
         free (grib_Data);
         return 1;
      }
      f_first = 0;

      /* Break out if we're only converting one message. */
      if (usr->msgNum != 0) {
         break;
      }
/*      printf ("Just finished message: %d %d\n", msgNum, subgNum);*/

      /* If we haven't found the end of the message, increase the subgrid
       * we're interested in. */
      if (f_endMsg != 1) {
         subgNum++;
      } else {
         subgNum = 0;
         msgNum++;
      }
/*
      IS_Free (is);
      IS_Init (is);
*/
   }
/* End loop for all messages. */

   free (grib_Data);
   return 0;
}

/*****************************************************************************
 * DegribIt() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Main routine.  Calls various functions based on the user options that
 * were passed into it using "usr".
 *
 * ARGUMENTS
 * usr = The user option structure to use while Degrib'ing. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = OK
 *  1 = Invalid usage.
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *   7/2003 AAT: memleak by free'ing outName outside the loop.
 *
 * NOTES
 *   printf ("Timing info. %f\n", clock() / (double) (CLOCKS_PER_SEC));
 * 1) May want to get the convert stuff together into a procedure, which
 *    Reads and Writes for 1 message.
 *****************************************************************************
 */
int DegribIt (userType *usr)
{
   inventoryType *Inv;  /* Contains an GRIB2 message inventory of the file */
   uInt4 LenInv;        /* size of Inv (also # of GRIB2 messages) */
   uInt4 i;             /* loop counter. */
   char *msg;           /* Used to print the error stack, and return version
                         * info. */
   IS_dataType is;      /* Un-parsed meta data for this GRIB2 message. As
                         * well as some memory used by the unpacker. */
   grib_MetaData meta;  /* The meta structure for this GRIB2 message. */
   FILE *grib_fp;       /* The opened grib2 file for input. */
   sInt4 offset;        /* Where we currently are in grib_fp. */
   double refTime;      /* The oldest reference time in the file. */
   char timeBuff[200];  /* Time buffer for formated refTime. */
   size_t inName;       /* Index into the input name array */
   int msgNum;          /* The messageNumber during the inventory. */
   int curMsg;          /* The current message used during the FindGRIB */

#ifdef DEBUG
   if (!usr->f_stdout) {
      fprintf (stderr, "Timing info. %f\n", clock () /
               (double) (CLOCKS_PER_SEC));
   }
#endif

   /* Create an Inventory of this file. */
   switch (usr->f_Command) {
      case CMD_SPLIT:
         msgNum = 0;
         for (inName = 0; inName < usr->numInNames; inName++) {
            msgNum = GRIB2Split (usr->inNames[inName], usr->msgNum, msgNum);
            if (msgNum < 0) {
               printf ("ERROR: with split\n");
               msg = errSprintf (NULL);
               printf ("ERROR: In call to GRIB2Split.\n%s", msg);
               free (msg);
               return 1;
            }
         }
         break;

      case CMD_INVENTORY:
         Inv = NULL;
         LenInv = 0;
         msgNum = 0;
         for (inName = 0; inName < usr->numInNames; inName++) {
            if (GRIB2Inventory (usr->inNames[inName], &Inv, &LenInv, 0,
                                &msgNum) < 0) {
               printf ("ERROR: with inventory, so far:\n");
               GRIB2InventoryPrint (Inv, LenInv);
               msg = errSprintf (NULL);
               printf ("ERROR: In call to GRIB2Inventory.\n%s", msg);
               free (msg);
               for (i = 0; i < LenInv; i++) {
                  GRIB2InventoryFree (Inv + i);
               }
               free (Inv);
               return 1;
            }
         }
         GRIB2InventoryPrint (Inv, LenInv);
         for (i = 0; i < LenInv; i++) {
            GRIB2InventoryFree (Inv + i);
         }
         free (Inv);
         break;

      case CMD_REFTIME:
         if (GRIB2RefTime (usr->inNames[0], &refTime) < 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to GRIB2RefTime.\n%s", msg);
            free (msg);
            return 1;
         }
         timeBuff[0] = '\0';
         Clock_Print (timeBuff, 200, refTime, usr->tmFormat, 0);
         printf ("%s\n", timeBuff);
         break;

         /* Convert GRIB2 record. */
      case CMD_CONVERT:
         if (usr->msgNum > 0) {
            curMsg = 0;
            for (inName = 0; inName < usr->numInNames; inName++) {
               if (usr->inNames[inName] != NULL) {
                  if ((grib_fp = fopen (usr->inNames[inName], "rb")) == NULL) {
                     printf ("Problems opening %s for read\n",
                             usr->inNames[inName]);
                     return 1;
                  }
               } else {
                  grib_fp = stdin;
               }
               offset = 0;
               /* Find the desired GRIB message. */
               if (FindGRIBMsg (grib_fp, usr->msgNum, &offset, &curMsg) == -1) {
                  msg = errSprintf (NULL);
                  printf ("ERROR: In call to FindGRIBMsg.\n%s", msg);
                  free (msg);
                  fclose (grib_fp);
                  return 1;
               }
               if (curMsg == usr->msgNum) {
                  IS_Init (&is);
                  MetaInit (&meta);
                  if (Grib2Convert (usr, grib_fp, &is, &meta) != 0) {
                     msg = errSprintf (NULL);
                     printf ("ERROR: In call to Grib2Convert.\n%s\n", msg);
                     free (msg);
                     MetaFree (&meta);
                     IS_Free (&is);
                     fclose (grib_fp);
                     return 1;
                  }
                  MetaFree (&meta);
                  IS_Free (&is);
                  fclose (grib_fp);
                  break;
               }
               fclose (grib_fp);
            }
            if (curMsg != usr->msgNum) {
               printf ("Couldn't find message number %d\n", usr->msgNum);
            }
         } else {
            IS_Init (&is);
            MetaInit (&meta);
            for (inName = 0; inName < usr->numInNames; inName++) {
               if (usr->inNames[inName] != NULL) {
                  if ((grib_fp = fopen (usr->inNames[inName], "rb")) == NULL) {
                     printf ("Problems opening %s for read\n",
                             usr->inNames[inName]);
                     return 1;
                  }
               } else {
                  grib_fp = stdin;
               }
               if (Grib2Convert (usr, grib_fp, &is, &meta) != 0) {
                  msg = errSprintf (NULL);
                  printf ("ERROR: In call to Grib2Convert.\n%s\n", msg);
                  free (msg);
                  MetaFree (&meta);
                  IS_Free (&is);
                  fclose (grib_fp);
                  return 1;
               }
               fclose (grib_fp);
            }
            MetaFree (&meta);
            IS_Free (&is);
         }
         break;

         /* Perform the StormTotal algorithm. */
      case CMD_TOTAL:
         if ( (usr->startTime == 0) || (usr->endTime == 0) || (usr->endTime < usr->startTime) ){
            printf("'-stormTotal' (stormTotal) option requires a valid '-startTime' and '-endTime' option. endTime must be greater than startTime\n");
            return -1;
         }

         IS_Init (&is);
         MetaInit (&meta);
         for (inName = 0; inName < usr->numInNames; inName++) {
            if (usr->inNames[inName] != NULL) {
               if ((grib_fp = fopen (usr->inNames[inName], "rb")) == NULL) {
                  printf ("Problems opening %s for read\n",
                          usr->inNames[inName]);
                  return 1;
               }
            } else {
               grib_fp = stdin;
            }
                
            if (StormTotal (usr, grib_fp, &is, &meta) != 0) {
               msg = errSprintf (NULL);
               printf ("ERROR: In call to StormTotal.\n%s\n", msg);
               free (msg);
               MetaFree (&meta);
               IS_Free (&is);
               fclose (grib_fp);
               return 1;
            }
            fclose (grib_fp);
         }
         MetaFree (&meta);
         IS_Free (&is);
	 break;
         /* Probe GRIB2 file for a given lat/lon (or lat/lon set). */

      case CMD_PROBE:
      case CMD_DATAPROBE:
         if (ProbeCmd (usr->f_Command, usr) != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to GRIB2Probe.\n%s\n", msg);
            free (msg);
         }
         break;

         /* Create a Database (cube + index) from the GRIB2 file. */
      case CMD_DATA:
         IS_Init (&is);
         MetaInit (&meta);
         if (Grib2Database (usr, &is, &meta) != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to Grib2Database.\n%s\n", msg);
            free (msg);
         }
         MetaFree (&meta);
         IS_Free (&is);
         break;

      case CMD_DATACONVERT:
         if (Grib2DataConvert (usr) != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to Grib2DataConvert.\n%s\n", msg);
            free (msg);
         }
         break;

      case CMD_NCCONVERT:
         if (Grib2NCConvert (usr) != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to Grib2NetCDFConvert.\n%s\n", msg);
            free (msg);
         }
         break;

         /* Print version info. */
      case CMD_VERSION:
         /* shouldn't get here because it is handled in cstart.c */
         myAssert (1 == 0);
         msg = Grib2About ("degrib");
         printf ("%s", msg);
         free (msg);
         break;

      case CMD_SECTOR:
         for (i = 0; i < usr->numPnt; i++) {
            if (WhichSector (usr->sectFile, usr->pnt[i], usr->f_pntType) != 0) {
               msg = errSprintf (NULL);
               printf ("ERROR: In call to Grib2DataProbe.\n%s\n", msg);
               free (msg);
            }
         }
         break;

#ifdef CALC
      case CMD_CALC:
         IS_Init (&is);
         MetaInit (&meta);
         if (GRIB2Calc (usr, &is, &meta) != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to GRIB2Probe.\n%s", msg);
            free (msg);
         }
         MetaFree (&meta);
         IS_Free (&is);
         break;
#endif
         /* Unknown command. */
      default:
         printf ("Unknown command?");
   }
#ifdef DEBUG
   if (!usr->f_stdout) {
      fprintf (stderr, "Timing info. %f\n", clock () /
               (double) (CLOCKS_PER_SEC));
   }
#endif
   return 0;
}
