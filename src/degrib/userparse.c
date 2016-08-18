/*****************************************************************************
 * userparse.c
 *
 * DESCRIPTION
 *    This file contains the code that is common to cstart.c and tcldegrib.c
 * when parsing the options provided by the user.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL / RSIS): Created.
 *  12/2002 Tim Kempisty, Ana Canizares, Tim Boyer, & Marc Saccucci
 *          (TK,AC,TB,&MS): Code Review 1.
 *
 * NOTES
 *    Would have liked to have put the "parse option" commands in here, but
 * the tcldegrib uses Tcl_Objects, which the c program doesn't know about.
 *    I could have use the old Tcl_CreateCommand instead of
 * Tcl_CreateObjCommand, to get argc/argv lists which C could handle, but
 * argc/argv lists are less efficient than objc/objv lists.
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/*#ifdef HAVE_FCNTL_H*/
#include <fcntl.h>
/*#endif*/
#include <math.h>
#include "myassert.h"
#include "myerror.h"
#include "userparse.h"
#include "myutil.h"
#include "genprobe.h"
#include "clock.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

/*****************************************************************************
 * UserInit() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Initializes all userType values to "unknown" (either -1 or NULL).  They
 * will then be filled in by command line options, and finally any that aren't
 * set will be set to the default values by UserDefault.
 *
 * ARGUMENTS
 * usr = The user option structure to initialize. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *
 * NOTES
 *****************************************************************************
 */
void UserInit (userType *usr)
{
   usr->cfgName = NULL;
   usr->numInNames = 0;
   usr->inNames = NULL;
   usr->f_inTypes = NULL;
   usr->f_Command = -1;
   usr->f_Flt = -1;
   usr->f_Met = -1;
   usr->f_IS0 = -1;
   usr->f_Freq = -1;
   usr->f_Map = -1;
   usr->f_Shp = -1;
   usr->f_Kml = -1;
   usr->f_kmlMerge = -1;
   usr->f_Csv = -1;
   usr->f_Tdl = -1;
   usr->f_Grib2 = -1;
   usr->f_Cube = -1;
   usr->f_Append = -1;
   usr->f_poly = -1;
   usr->f_nMissing = -1;
   usr->msgNum = -1;
   usr->subgNum = -1;
   usr->f_unit = -1;
   usr->decimal = -1;
   usr->LatLon_Decimal = -1;
   usr->nameStyle = NULL;
   usr->f_pntStyle = -1;
   usr->namePath = NULL;
   usr->sectFile = NULL;
   usr->outName = NULL;
   usr->f_stdout = -1;
   usr->logName = NULL;
   usr->pntFile = NULL;
   usr->indexFile = NULL;
   usr->mapIniFile = NULL;
   usr->kmlIniFile = NULL;
   usr->mapIniOptions = NULL;
   usr->separator = NULL;
/*   usr->pnt.f_valid = 0;*/
   usr->pnt = NULL;
   usr->numCWA = 0;
   usr->numPnt = 0;
   usr->ndfdVars = NULL;
   usr->numNdfdVars = 0;
   usr->lampDataDir = NULL;
   usr->rtmaDataDir = NULL;
   usr->geoDataDir = NULL;
   usr->gribFilter = NULL;
   usr->lwlf.lat = -100;
   usr->uprt.lat = -100;
   usr->f_pntType = -1;
   usr->f_surface = -1;
   usr->f_nLabel = -1;
   usr->f_interp = -1;
   usr->f_avgInterp = -1;
   usr->f_coverageGrid = -1;
   usr->f_AscGrid = -1;
   usr->f_GrADS = -1;
   usr->f_NetCDF = -1;
   usr->f_XML = -1;
   usr->f_Graph = -1;
   usr->f_MOTD = -1;
   usr->f_SimpleWx = -1;
   usr->f_SimpleWWA = -1;
   usr->f_SimpleVer = -1;
   usr->f_revFlt = -1;
   usr->f_MSB = -1;
   usr->majEarth = -1;
   usr->minEarth = -1;
   usr->f_WxParse = -1;
   usr->f_icon = -1;
   usr->f_Print = -1;
   usr->Asc2Flx_File = NULL;
/*   usr->matchElem = NULL; */
   usr->tmFormat = NULL;
/*   usr->matchRefTime = -1;*/
/*   usr->matchValidTime = -1;*/
   usr->f_validRange = -1;
   usr->validMin = 0;
   usr->validMax = 0;
   usr->f_verboseShp = -1;
   usr->f_valTime = -1;
   usr->f_ndfdConven = -1;
   usr->cwaBuff = NULL;
   usr->ndfdVarsBuff = NULL;
   usr->startTime = 0;
   usr->endTime = 0;
   usr->f_timeFlavor = -1;
   usr->f_stormTotal = -1;
}

/*****************************************************************************
 * UserFree() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Free's any memory that was alloced in the userType data structure.
 *
 * ARGUMENTS
 * usr = The user option structure to free. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *
 * NOTES
 *****************************************************************************
 */
void UserFree (userType *usr)
{
   size_t i;

   free (usr->cfgName);
   for (i = 0; i < usr->numInNames; i++) {
      free (usr->inNames[i]);
   }
   free (usr->inNames);
   free (usr->f_inTypes);
   free (usr->nameStyle);
   free (usr->namePath);
   free (usr->sectFile);
   free (usr->outName);
   free (usr->logName);
   free (usr->pntFile);
   free (usr->indexFile);
   free (usr->mapIniFile);
   free (usr->kmlIniFile);
   if (usr->mapIniOptions != NULL)
      free (usr->mapIniOptions);
   free (usr->separator);
/*   free (usr->matchElem);*/
   free (usr->tmFormat);
   free (usr->Asc2Flx_File);
   if (usr->pnt != NULL)
      free (usr->pnt);
   if (usr->ndfdVars != NULL)
      free (usr->ndfdVars);
   if (usr->lampDataDir != NULL)
      free (usr->lampDataDir);
   if (usr->rtmaDataDir != NULL)
      free (usr->rtmaDataDir);
   if (usr->geoDataDir != NULL)
      free (usr->geoDataDir);
   if (usr->gribFilter != NULL)
      free (usr->gribFilter);
   if (usr->ndfdVarsBuff != NULL)
      free (usr->ndfdVarsBuff);
   if (usr->cwaBuff != NULL)
      free (usr->cwaBuff);
}

/*****************************************************************************
 * UserValidate() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Go through all options after we have looked in the config file, and on
 * the command line.  If anything is still "missing", we set those options
 * to the default values.  In addition we validate certain options to try to
 * catch any user mistakes.
 *
 * ARGUMENTS
 * usr = The user option structure to initialize. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Invalid usage.
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *
 * NOTES
 *****************************************************************************
 */
int UserValidate (userType *usr)
{
   char *ptr;           /* Used to check the number of '.' in outName. */
   size_t tailLen;
   size_t len;
   uInt4 i;
   size_t numCol;
   char **colList;
   size_t j;
   size_t valCol;
   uChar ans;

   myAssert (usr->f_Command != -1);
   myAssert (sizeof (char) == 1);

   if ((usr->f_Command != CMD_VERSION) && (usr->f_Command != CMD_SECTOR)) {
      if (usr->numInNames == 0) {
/*#ifdef WIN32*/
#ifdef _WINDOWS_
         /* Set stdin to binary mode */
         setmode (0, O_BINARY);
#endif
         /* Uncommented on 6/4/2008 to deal with "degrib -DP -pnt 1,1" */
         errSprintf ("Please provide an input file. See '-in' option.\n");
         return -1;
      }
   }
   if (usr->numInNames == 1) {
      if (strcmp (usr->inNames[0], "stdin") == 0) {
         usr->numInNames = 0;
         free (usr->inNames[0]);
         free (usr->inNames);
         usr->inNames = NULL;
         free (usr->f_inTypes);
         usr->f_inTypes = NULL;
/*#ifdef WIN32*/
#ifdef _WINDOWS_
         /* Set stdin to binary mode */
         setmode (0, O_BINARY);
#endif
      }
   }

   /* Command specific defaults... */
   switch (usr->f_Command) {
      case CMD_DATA:
         if (usr->indexFile == NULL) {
            errSprintf ("'-Data' (Database) option requires '-Index' "
                        "option\n");
            return -1;
         }
         if (usr->f_Cube == -1)
            usr->f_Cube = 1;
         if (usr->msgNum == -1)
            usr->msgNum = 0;
         if (usr->f_MSB == -1)
            usr->f_MSB = 0;
         if (usr->f_revFlt == -1)
            usr->f_revFlt = 1;
         break;
      case CMD_SECTOR:
         if (usr->numPnt == 0) {
            errSprintf ("'-Sector' option requires a '-pnt' option\n");
            return -1;
         }
         break;
      case CMD_PROBE:
         if ((usr->numPnt == 0) && (usr->pntFile == NULL) &&
             (usr->f_pntType != 2)) {
            errSprintf ("'-P' (Probe) option requires a '-pnt' option "
                        "or '-pntFile' option or '-cells all' option\n");
            return -1;
         }
         if (usr->numPnt == 0) {
            if (usr->f_pntType == 0) {
               /* validate the lat/lon */
               for (i = 0; i < usr->numPnt; i++) {
                  if ((usr->pnt[i].Y < -90) || (usr->pnt[i].Y > 90)) {
                     errSprintf ("in -pnt option, invalid lat of %f\n",
                                 usr->pnt[i].Y);
                     return -1;
                  }
                  if ((usr->pnt[i].X < -360) || (usr->pnt[i].X > 360)) {
                     errSprintf ("in -pnt option, invalid lon of %f\n",
                                 usr->pnt[i].X);
                     return -1;
                  }
               }
            }
         }
         break;
      case CMD_DATAPROBE:
         if ((usr->numPnt == 0) && (usr->pntFile == NULL) &&
             (!usr->f_Print) && (usr->Asc2Flx_File == NULL)) {
            errSprintf ("'-DP' (Database Probe) option requires a '-pnt' "
                        "option or '-pntFile' option or '-Print' option\n");
            return -1;
         }
         if (usr->f_pntType == 2) {
            errSprintf ("'-DP' can't handle a '-cells all' option yet.  If "
                        "needed please make a request.\n");
            return -1;
         }
         if (usr->numPnt == 0) {
            if (usr->f_pntType == 0) {
               /* validate the lat/lon */
               for (i = 0; i < usr->numPnt; i++) {
                  if ((usr->pnt[i].Y < -90) || (usr->pnt[i].Y > 90)) {
                     errSprintf ("in -pnt option, invalid lat of %f\n",
                                 usr->pnt[i].Y);
                     return -1;
                  }
                  if ((usr->pnt[i].X < -360) || (usr->pnt[i].X > 360)) {
                     errSprintf ("in -pnt option, invalid lon of %f\n",
                                 usr->pnt[i].X);
                     return -1;
                  }
               }
            }
         }
         break;
   }

   /* Generic defaults... */
   if (usr->msgNum == -1) {
      usr->msgNum = 1;
   } else if (usr->msgNum < 0) {
      errSprintf ("Please use -msg '%d' in range of [1..n]\n", usr->msgNum);
      errSprintf ("or -msg all or -msg 0 to signify all messages\n");
      return -1;
   }
   if (usr->f_ndfdConven == -1)
      usr->f_ndfdConven = 1;
   if (usr->ndfdVarsBuff != NULL) {
      numCol = 0;
      colList = NULL;
      /* could just do a ptr = strchr (next, ',') ... etc */
      mySplit (usr->ndfdVarsBuff, ',', &numCol, &colList, 1);
      usr->ndfdVars = (uChar *) malloc (numCol * sizeof (uChar));
      valCol = 0;
      for (i = 0; i < numCol; i++) {
         ans = gen_NDFD_NDGD_Lookup (colList[i], 1, usr->f_ndfdConven);
         if (ans != NDFD_UNDEF) {
            /* Check if we already have this element */
            for (j = 0; j < valCol; j++) {
               if (ans == usr->ndfdVars[j])
                  break;
            }
            /* if not, add it, and add 1 to valCol */
            if (j == valCol) {
               usr->ndfdVars[j] = ans;
               valCol++;
            }
         }
         free (colList[i]);
      }
      usr->numNdfdVars = valCol;
      /* Could realloc usr->ndfdVars here, but no real point */
      free (colList);
   }
   if (usr->cwaBuff != NULL) {
      if (usr->numPnt != usr->numCWA) {
         errSprintf (" Hazards were queried for. Each point has one associated "
                     "CWA with it and numPnt != numCWA.\n");
         return -1;
      }
   }
   if (usr->f_validRange == -1)
      usr->f_validRange = 0;
   if (usr->f_verboseShp == -1)
      usr->f_verboseShp = 0;
   if (usr->subgNum == -1)
      usr->subgNum = 0;
   if (usr->f_MSB == -1)
      usr->f_MSB = 1;
   if (usr->f_Flt == -1)
      usr->f_Flt = 0;
   if (usr->f_stdout == -1)
      usr->f_stdout = 0;
   if (usr->f_Shp == -1)
      usr->f_Shp = 0;
   if (usr->f_Kml == -1)
      usr->f_Kml = 0;
   if (usr->f_kmlMerge == -1)
      usr->f_kmlMerge = 0;
   if (usr->f_Csv == -1)
      usr->f_Csv = 0;
   if (usr->f_Tdl == -1)
      usr->f_Tdl = 0;
   if (usr->f_Grib2 == -1)
      usr->f_Grib2 = 0;
   if (usr->f_Met == -1)
      usr->f_Met = 1;
   if (usr->f_IS0 == -1)
      usr->f_IS0 = 0;
   if (usr->f_Freq == -1)
      usr->f_Freq = 0;
   if (usr->f_Map == -1)
      usr->f_Map = 0;
   if (usr->majEarth == -1)
      usr->majEarth = 0;
   if (usr->minEarth == -1)
      usr->minEarth = 0;
   if (usr->f_unit == -1)
      usr->f_unit = 1;  /* Default to English units. */
   if (usr->decimal == -1)
      usr->decimal = 3;
   if (usr->LatLon_Decimal == -1)
      usr->LatLon_Decimal = 6;
   if (usr->f_poly == -1)
      usr->f_poly = 0;
   if (usr->f_nMissing == -1)
      usr->f_nMissing = 0;
   if (usr->f_AscGrid == -1)
      usr->f_AscGrid = 0;
   if (usr->f_GrADS == -1)
      usr->f_GrADS = 0;
   if (usr->f_interp == -1)
      usr->f_interp = 0;
   if (usr->f_avgInterp == -1)
      usr->f_avgInterp = 0;
   if (usr->f_coverageGrid == -1)
      usr->f_coverageGrid = 0;
   if (usr->f_revFlt == -1)
      usr->f_revFlt = 0;
   if (usr->f_NetCDF == -1)
      usr->f_NetCDF = 0;
   if (usr->f_XML == -1)
      usr->f_XML = 0;
   if (usr->f_Graph == -1)
      usr->f_Graph = 0;
   if (usr->f_MOTD == -1)
      usr->f_MOTD = 0;
   if (usr->f_SimpleWx == -1)
      usr->f_SimpleWx = 0;
   if (usr->f_SimpleWWA == -1)
      usr->f_SimpleWWA = 3;
   if (usr->f_SimpleVer == -1)
      usr->f_SimpleVer = 4;
   if (usr->f_pntStyle == -1)
      usr->f_pntStyle = 0;
   if (usr->f_pntType == -1)
      usr->f_pntType = 0;
   if (usr->f_surface == -1)
      usr->f_surface = 0;
   if (usr->f_nLabel == -1)
      usr->f_nLabel = 0;
   if (usr->f_WxParse == -1)
      usr->f_WxParse = 0;
   if (usr->f_icon == -1)
      usr->f_icon = 0;
   if (usr->f_valTime == -1)
      usr->f_valTime = 0;
   if (usr->rtmaDataDir == NULL) {
      char perm;
      char *defDir = "/www/ndfd/public/database/cube/rtma";

      /* check if it is a directory */
      if (myStat (defDir, &perm, NULL, NULL) == MYSTAT_ISDIR) {
         /* check that it is readable */
         if ((perm & 4)) {
            usr->rtmaDataDir = (char *) malloc ((strlen (defDir) + 1) *
                                                 sizeof (char));
            strcpy (usr->rtmaDataDir, defDir);
         }
      }
   }
   if (usr->lampDataDir == NULL) {
      char perm;
      char *defDir = "/www/ndfd/public/database/cube/lamp";

      /* check if it is a directory */
      if (myStat (defDir, &perm, NULL, NULL) == MYSTAT_ISDIR) {
         /* check that it is readable */
         if ((perm & 4)) {
            usr->lampDataDir = (char *) malloc ((strlen (defDir) + 1) *
                                                 sizeof (char));
            strcpy (usr->lampDataDir, defDir);
         }
      }
   }
   if (usr->separator == NULL) {
      usr->separator = (char *) malloc (3 * sizeof (char));
      strcpy (usr->separator, ", ");
   }
   if (usr->f_Cube == -1)
      usr->f_Cube = 0;
   if (usr->f_Append == -1)
      usr->f_Append = 0;
   if (usr->f_Print == -1)
      usr->f_Print = 0;
   if (usr->tmFormat == NULL) {
      usr->tmFormat = (char *) malloc (9 * sizeof (char));
      strcpy (usr->tmFormat, "%D %H:%M");
   }

   /* Validate we have enough information for the output filename. */
   if (usr->outName == NULL) {
      /* if "-C -Cube" you don't need -out.. only for "-Data -Cube" */
      if ((usr->f_Cube) && (usr->f_Command == CMD_DATA)) {
         if (strlen (usr->indexFile) < 4) {
            errSprintf ("-Cube option requires an '-out' option,\n");
            errSprintf ("or an indexFile that has an extension on it.\n");
            return -1;
         }
         usr->outName = (char *) malloc (strlen (usr->indexFile) + 1);
         strcpy (usr->outName, usr->indexFile);
         strncpy (usr->outName + strlen (usr->indexFile) - 3, "dat", 3);
         if (strcmp (usr->outName, usr->indexFile) == 0) {
            errSprintf ("-Index and -out ended up with the same filename "
                        "of '%s'\n", usr->outName);
            errSprintf ("Suggest -Index without a '.dat' extension?\n");
            return -1;
         }
      } else {
         if (usr->nameStyle == NULL) {
            usr->nameStyle = (char *) malloc ((strlen ("%e_%v.txt") + 1) *
                                              sizeof (char));
            strcpy (usr->nameStyle, "%e_%v.txt");
         }
      }
   } else if (strcmp (usr->outName, "stdout") == 0) {
      usr->f_stdout = 1;
   } else {
      if ((ptr = strrchr (usr->outName, '/')) != NULL) {
         ptr = strchr (ptr, '.');
      } else if ((ptr = strrchr (usr->outName, '\\')) != NULL) {
         ptr = strchr (ptr, '.');
      } else {
         ptr = strchr (usr->outName, '.');
      }
      if (ptr != NULL) {
         ptr++;
         if ((strchr (ptr, '.')) != NULL) {
            errSprintf ("Please use only 1 '.' in %s\n", usr->outName);
            errSprintf ("ArcView has problems with multiple '.'\n");
            return -1;
         }
         tailLen = strlen (usr->outName) - (ptr - usr->outName);
         if (tailLen < 3) {
            len = strlen (usr->outName) + (3 - tailLen);
            usr->outName =
                  (char *) realloc (usr->outName, (len + 1) * sizeof (char));
            for (i = len - 3 + tailLen; i < len; i++) {
               usr->outName[i] = ' ';
            }
            usr->outName[i] = '\0';
         }
      } else {
         len = strlen (usr->outName) + 4;
         usr->outName =
               (char *) realloc (usr->outName, (len + 1) * sizeof (char));
         usr->outName[len - 4] = '.';
         for (i = len - 3; i < len; i++) {
            usr->outName[i] = ' ';
         }
         usr->outName[i] = '\0';
      }

      if ((strlen (usr->outName) < 4) ||
          (usr->outName[strlen (usr->outName) - 4] != '.')) {
         errSprintf ("Please use extensions of up to three letters in "
                     "'%s'.\n", usr->outName);
         return -1;
      }
   }
   if (usr->f_stdout) {
      if ((!usr->f_Grib2) && (!usr->f_Met) && (!usr->f_IS0) && (!usr->f_Csv)
          && (!usr->f_Tdl) && (!usr->f_Freq)) {
         errSprintf ("Currently, stdout is only valid with the -Grib2, "
                     "-Met, -IS0, -Csv, -Freq and -Tdl options.\n");
         return -1;
      }
/*#ifdef WIN32*/
#ifdef _WINDOWS_
      /* Set stdout to binary mode */
      setmode (1, O_BINARY);
#endif
/*
      if (freopen (NULL, "wb", stdout)) {
         printf ("error?\n");
         errSprintf ("Couldn't change stdout to binary mode (for -Grib2).\n");
         return -1;
      }
*/
   }
   return 0;
}

/*****************************************************************************
 * Grib2About() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Return a string which contains version info.
 *
 * ARGUMENTS
 *
 * FILES/DATABASES: None
 *
 * RETURNS: char *
 *
 * HISTORY
 *  12/2002 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
char *Grib2About (const char *name)
{

/*   char *buffer;
 *
 *      mallocSprintf (&buffer, "%s\nVersion: %s\nDate: %s\nAuthor: Arthur "
 *                        "Taylor\n", name, PROGRAM_VERSION, PROGRAM_DATE);
 *                           return buffer; */

   char *buffer;

   /* char compile_date[12];
   time_t now = time(NULL);
   strftime(compile_date, 12, "%m/%d/%Y", localtime(&now)); */

   mallocSprintf (&buffer, "%s\nVersion: %s\nDate: %s\nCompile Date: %s\nAuthor: Arthur "
                  "Taylor and Michael Allard\n", name, PROGRAM_VERSION, PROGRAM_DATE, __DATE__);
   return buffer;

}

int myCommaDoubleList2 (char *name, double *x, double *y)
{
   char *ptr;

   if ((ptr = strchr (name, ',')) == NULL) {
      return 1;
   }
   *ptr = '\0';
   if (! myAtoF (name, x)) {
      *ptr = ',';
      return 1;
   }
   *ptr = ',';
   if (! myAtoF (ptr + 1, y)) {
      return 1;
   }
   return 0;
}

/*****************************************************************************
 * ParseUserChoice() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Parses the "cur" option and adjusts the "usr" structure accordingly.
 * Some options take 2 arguments, so we pass "next" along, but don't always
 * use it.
 *
 * ARGUMENTS
 *  usr = The user option structure to update. (Output)
 *  cur = The current option to parse. (Input)
 * next = The next element on the command line. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  1 = If we didn't need "next"
 *  2 = If we did need "next"
 * -1 = Invalid option.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *   8/2003 AAT: Better error checking for "next == NULL" case.
 *
 * NOTES
 *****************************************************************************
 */
static char *UsrOpt[] = { "-cfg", "-in", "-I", "-C", "-P", "-V", "-Flt",
   "-nFlt", "-Shp", "-Shp2", "-nShp", "-Met", "-nMet", "-msg", "-nameStyle",
   "-out", "-Interp", "-revFlt", "-nRevFlt", "-MSB", "-nMSB", "-Unit",
   "-namePath", "-pnt", "-pntFile", "-poly", "-nMissing", "-Decimal", "-IS0",
   "-GrADS", "-SimpleWx", "-radEarth", "-Separator", "-WxParse", "-pntStyle",
   "-Data", "-Index", "-Cube", "-nCube", "-DP", "-Print", "-Append",
   "-cells", "-log", "-SimpleVer", "-Csv", "-nCsv", "-Grib2", "-nGrib2",
   "-DC", "-lwlf", "-uprt", "-Asc2Flx", "-majEarth", "-minEarth", "-surface",
   "-nLabel", "-LatLon_Decimal", "-NetCDF", "-refTime", "-tmFormat",
   "-stdout", "-validMax", "-validMin", "-Tdl", "-nTdl", "-Sector",
   "-sectFile", "-NC", "-AscGrid", "-Map", "-MapIni", "-MapIniOptions",
   "-XML", "-MOTD", "-Graph", "-startTime", "-endTime", "-startDate",
   "-numDays", "-ndfdVars", "-geoData", "-gribFilter", "-ndfdConven", "-Freq",
   "-Icon", "-curTime", "-rtmaDir", "-avgInterp", "-cwa", "-SimpleWWA",
   "-TxtParse", "-Kml", "-KmlIni", "-Kmz", "-kmlMerge", "-lampDir", "-Split",
   "-StormTotal", NULL
};

int IsUserOpt (char *str)
{
   int index;           /* Used to check if argv[1] is an option. */
   return (GetIndexFromStr (str, UsrOpt, &index) >= 0);
}

static int ParseUserChoice (userType *usr, char *cur, char *next)
{
   enum { CFG, IN, INVENTORY, CONVERT, PROBE, VERSION, FLT, NO_FLT, SHP,
      SHP2, NO_SHP, META, NO_META, MSG_NUM, NAME_STYLE, OUT, INTERPOLATE,
      REVFLT, NO_REVFLT, MSB, NO_MSB, UNIT, NAMEPATH, PNT, PNTFILE, POLYSHP,
      NOMISS_SHP, DECIMAL, IS0, GRADS, SIMPLEWX, RADEARTH, SEPARATOR,
      WXPARSE, PNTSTYLE, DATABASE, INDEXFILE, CUBE, NO_CUBE, DATAPROBE,
      PRINT, APPEND, CELLS, LOG, SIMPLEWX_VER, CSV, NO_CSV, GRIB2, NO_GRIB2,
      DATACONVERT, LWLF, UPRT, ASC2FLX, MAJEARTH, MINEARTH, SURFACE,
      NO_LABEL, LATLON_DECIMAL, NETCDF, REFTIME, TMFORMAT, STDOUT, VALIDMAX,
      VALIDMIN, TDL, NO_TDL, SECTOR, SECTFILE, NCCONVERT, ASCGRID, MAP,
      MAPINIFILE, MAPINIOPTIONS, XML, MOTD, GRAPH, STARTTIME, ENDTIME,
      STARTDATE, NUMDAYS, NDFDVARS, GEODATA, GRIBFILTER, NDFDCONVEN,
      FREQUENCY, ICON, CURTIME, RTMADIR, AVGINTERP, CWA, SIMPLEWWA, TXTPARSE,
      KML, KMLINIFILE, KMZ, KMLMERGE, LAMPDIR, SPLIT, TOTAL
   };
   int index;           /* "cur"'s index into Opt, which matches enum val. */
   double lat, lon;     /* Used to check on the -pnt option. */
   char *ptr;           /* Used to help parse -msg. */
   int i_temp;          /* Used to parse the -nameStyle options. */
   int len;             /* String length of "next" used with -nameStyle */
   sInt4 li_temp;
   int type;
   char perm;
   double curTime;
   sInt4 totDay;

   /* Figure out which option. */
   if (GetIndexFromStr (cur, UsrOpt, &index) < 0) {
      errSprintf ("Invalid option '%s'\n", cur);
      return -1;
   }
   /* Handle the 1 argument options first. */
   switch (index) {
      case SPLIT:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_SPLIT;
         } else if (usr->f_Command != CMD_SPLIT) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case INVENTORY:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_INVENTORY;
         } else if (usr->f_Command != CMD_INVENTORY) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case CONVERT:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_CONVERT;
         } else if (usr->f_Command != CMD_CONVERT) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case PROBE:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_PROBE;
         } else if (usr->f_Command != CMD_PROBE) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case VERSION:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_VERSION;
         } else if (usr->f_Command != CMD_VERSION) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case DATABASE:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_DATA;
         } else if (usr->f_Command != CMD_DATA) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case DATAPROBE:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_DATAPROBE;
         } else if (usr->f_Command != CMD_DATAPROBE) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case DATACONVERT:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_DATACONVERT;
         } else if (usr->f_Command != CMD_DATACONVERT) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case NCCONVERT:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_NCCONVERT;
         } else if (usr->f_Command != CMD_NCCONVERT) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case REFTIME:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_REFTIME;
         } else if (usr->f_Command != CMD_REFTIME) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case SECTOR:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_SECTOR;
         } else if (usr->f_Command != CMD_SECTOR) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case TOTAL:
         if (usr->f_Command == -1) {
            usr->f_Command = CMD_TOTAL;
         } else if (usr->f_Command != CMD_TOTAL) {
            errSprintf ("Can only handle one command option at a time.\n");
            usr->f_Command = -1;
            return -1;
         }
         return 1;
      case FLT:
         if (usr->f_Flt == -1)
            usr->f_Flt = 1;
         return 1;
      case NO_FLT:
         if (usr->f_Flt == -1)
            usr->f_Flt = 0;
         return 1;
      case META:
         if (usr->f_Met == -1)
            usr->f_Met = 1;
         return 1;
      case NO_META:
         if (usr->f_Met == -1)
            usr->f_Met = 0;
         return 1;
      case IS0:
         if (usr->f_IS0 == -1)
            usr->f_IS0 = 1;
         return 1;
      case FREQUENCY:
         if (usr->f_Freq == -1)
            usr->f_Freq = 1;
         return 1;
      case MAP:
         if (usr->f_Map == -1)
            usr->f_Map = 1;
         return 1;
      case SHP:
         if (usr->f_Shp == -1)
            usr->f_Shp = 1;
         return 1;
      case KML:
         if (usr->f_Kml == -1)
            usr->f_Kml = 1;
         return 1;
      case KMZ:
         if (usr->f_Kml == -1)
            usr->f_Kml = 2;
         return 1;
      case KMLMERGE:
         if (usr->f_kmlMerge == -1)
            usr->f_kmlMerge = 1;
         return 1;
      case SHP2:
         if (usr->f_Shp == -1)
            usr->f_Shp = 1;
         if (usr->f_verboseShp == -1)
            usr->f_verboseShp = 1;
         return 1;
      case NO_SHP:
         if (usr->f_Shp == -1)
            usr->f_Shp = 0;
         return 1;
      case CSV:
         if (usr->f_Csv == -1)
            usr->f_Csv = 1;
         return 1;
      case NO_CSV:
         if (usr->f_Csv == -1)
            usr->f_Csv = 0;
         return 1;
      case TDL:
         if (usr->f_Tdl == -1)
            usr->f_Tdl = 1;
         return 1;
      case NO_TDL:
         if (usr->f_Tdl == -1)
            usr->f_Tdl = 0;
         return 1;
      case GRIB2:
         if (usr->f_Grib2 == -1)
            usr->f_Grib2 = 1;
         return 1;
      case NO_GRIB2:
         if (usr->f_Grib2 == -1)
            usr->f_Grib2 = 0;
         return 1;
      case CUBE:
         if (usr->f_Cube == -1)
            usr->f_Cube = 1;
         return 1;
      case NO_CUBE:
         if (usr->f_Cube == -1)
            usr->f_Cube = 0;
         return 1;
      case APPEND:
         if (usr->f_Append == -1)
            usr->f_Append = 1;
         return 1;
      case NOMISS_SHP:
         if (usr->f_nMissing == -1)
            usr->f_nMissing = 1;
         return 1;
      case SIMPLEWX:
         if (usr->f_SimpleWx == -1)
            usr->f_SimpleWx = 1;
         return 1;
      case REVFLT:
         if (usr->f_revFlt == -1)
            usr->f_revFlt = 1;
         return 1;
      case NO_REVFLT:
         if (usr->f_revFlt == -1)
            usr->f_revFlt = 0;
         return 1;
      case MSB:
         if (usr->f_MSB == -1)
            usr->f_MSB = 1;
         return 1;
      case NO_MSB:
         if (usr->f_MSB == -1)
            usr->f_MSB = 0;
         return 1;
      case STDOUT:
         if (usr->f_stdout == -1) {
            usr->f_stdout = 1;
         }
         return 1;
      case PRINT:
         if (usr->f_Print == -1)
            usr->f_Print = 1;
         return 1;
      case NO_LABEL:
         if (usr->f_nLabel == -1)
            usr->f_nLabel = 1;
         return 1;
      case ASCGRID:
         if (usr->f_AscGrid == -1)
            usr->f_AscGrid = 1;
         return 1;
      case AVGINTERP:
         if (usr->f_avgInterp == -1)
            usr->f_avgInterp = 1;
         return 1;
   }
   /* It is possibly a 2 argument option, so check if next is NULL, or
    * next[0] is '-'.  If so, then it is a 1 argument option. */
   if ((next == NULL) || (next[0] == '-')) {
      switch (index) {
         case GRADS:
            if (usr->f_GrADS == -1)
               usr->f_GrADS = 1;
            return 1;
         case POLYSHP:
            if (usr->f_poly == -1)
               usr->f_poly = 1;
            return 1;
         case INTERPOLATE:
            /* true f_coverageGrid, false f_interp */
            if (usr->f_coverageGrid == -1)
               usr->f_coverageGrid = 1;
            if (usr->f_interp == -1)
               usr->f_interp = 1;
            return 1;
      }
   }
   /* It is definitely a 2 argument option, so check if next is NULL. */
   if (next == NULL) {
      errSprintf ("%s needs another argument\n", cur);
      return -1;
   }
   /* Handle the 2 argument options. */
   switch (index) {
      case CFG:
         if (usr->cfgName == NULL) {
            usr->cfgName =
                  (char *) malloc ((strlen (next) + 1) * sizeof (char));
            strcpy (usr->cfgName, next);
         }
         return 2;
      case IN:
         type = myStat (next, &perm, NULL, NULL);
         /* check if it is a file or directory */
         if ((type != MYSTAT_ISDIR) && (type != MYSTAT_ISFILE)) {
            errSprintf ("'%s' is not a file or directory\n", next);
            return -1;
         }
         /* check that it is readable */
         if (!(perm & 4)) {
            errSprintf ("No read permissions on '%s'\n", next);
            return -1;
         }
         i_temp = usr->numInNames;
         usr->numInNames++;
         usr->inNames = (char **) realloc (usr->inNames,
                                           usr->numInNames * sizeof (char *));
         usr->inNames[i_temp] =
               (char *) malloc ((strlen (next) + 1) * sizeof (char));
         strcpy (usr->inNames[i_temp], next);
         usr->f_inTypes = (char *) realloc (usr->f_inTypes,
                                            usr->numInNames * sizeof (char));
         usr->f_inTypes[i_temp] = type;
         return 2;
      case OUT:
         if (usr->outName == NULL) {
            usr->outName = (char *) malloc ((strlen (next) + 1) *
                                            sizeof (char));
            strcpy (usr->outName, next);
         }
         return 2;
      case LOG:
         if (usr->logName == NULL) {
            usr->logName = (char *) malloc ((strlen (next) + 1) *
                                            sizeof (char));
            strcpy (usr->logName, next);
         }
         return 2;
      case NAMEPATH:
         if (usr->namePath == NULL) {
            usr->namePath = (char *) malloc ((strlen (next) + 1) *
                                             sizeof (char));
            strcpy (usr->namePath, next);
         }
         return 2;
      case SECTFILE:
         if (usr->sectFile == NULL) {
            usr->sectFile = (char *) malloc ((strlen (next) + 1) *
                                             sizeof (char));
            strcpy (usr->sectFile, next);
         }
         return 2;
      case PNTFILE:
         if (usr->pntFile == NULL) {
            usr->pntFile = (char *) malloc ((strlen (next) + 1) *
                                            sizeof (char));
            strcpy (usr->pntFile, next);
            return 2;
         }
      case ASC2FLX:
         if (usr->Asc2Flx_File == NULL) {
            usr->Asc2Flx_File = (char *) malloc ((strlen (next) + 1) *
                                                 sizeof (char));
            strcpy (usr->Asc2Flx_File, next);
            return 2;
         }
      case INDEXFILE:
         if (usr->indexFile == NULL) {
            usr->indexFile = (char *) malloc ((strlen (next) + 1) *
                                              sizeof (char));
            strcpy (usr->indexFile, next);
            return 2;
         }
      case MAPINIFILE:
         if (usr->mapIniFile == NULL) {
            usr->mapIniFile = (char *) malloc ((strlen (next) + 1) *
                                               sizeof (char));
            strcpy (usr->mapIniFile, next);
            return 2;
         }
      case KMLINIFILE:
         if (usr->kmlIniFile == NULL) {
            usr->kmlIniFile = (char *) malloc ((strlen (next) + 1) *
                                               sizeof (char));
            strcpy (usr->kmlIniFile, next);
            return 2;
         }
      case MAPINIOPTIONS:
         if (usr->mapIniOptions == NULL) {
            usr->mapIniOptions = (char *) malloc ((strlen (next) + 1) *
                                                  sizeof (char));
            strcpy (usr->mapIniOptions, next);
            return 2;
         }
      case SEPARATOR:
         if (usr->separator == NULL) {
            usr->separator = (char *) malloc ((strlen (next) + 1) *
                                              sizeof (char));
            strcpy (usr->separator, next);
         }
         return 2;
      case TMFORMAT:
         if (usr->tmFormat == NULL) {
            usr->tmFormat = (char *) malloc ((strlen (next) + 1) *
                                             sizeof (char));
            strcpy (usr->tmFormat, next);
         }
         return 2;
      case CURTIME:
         if (Clock_Scan (&curTime, next, 0) != 0) {
            return 2;
         }
         Clock_SetSeconds (&curTime, 1);
         return 2;
      case STARTTIME:
         if (usr->f_timeFlavor == 2) {
            errSprintf ("Inconsistent time flavor.\n"
                        "Either use startTime, endTime, or startDate, numDays,"
                        "but don't mix them.\n");
            return -1;
         }
         usr->f_timeFlavor = 1;
         if ((usr->f_valTime != 1) && (usr->f_valTime != 3)) {
            /* f_gmt = 0 no adjust, 1 adjust as LDT, 2 adjust as LST */
            if (Clock_Scan (&(usr->startTime), next, 0) != 0) {
               return 2;
            }
            if (usr->f_valTime == -1) {
               usr->f_valTime = 1;
            } else {
               usr->f_valTime = 3;
            }
/*            printf ("start Time '%s' %f\n", next, usr->startTime);*/
            return 2;
         }
         return 2;
      case ENDTIME:
         if (usr->f_timeFlavor == 2) {
            errSprintf ("Inconsistent time flavor.\n"
                        "Either use startTime, endTime, or startDate, numDays,"
                        "but don't mix them.\n");
            return -1;
         }
         usr->f_timeFlavor = 1;
         if ((usr->f_valTime != 2) && (usr->f_valTime != 3)) {
            /* f_gmt = 0 no adjust, 1 adjust as LDT, 2 adjust as LST */
            if (Clock_Scan (&(usr->endTime), next, 0) != 0) {
               return 2;
            }
            if (usr->f_valTime == -1) {
               usr->f_valTime = 2;
            } else {
               usr->f_valTime = 3;
            }
/*            printf ("end Time '%s' %f\n", next, usr->endTime);*/
         }
         return 2;
      case STARTDATE:
         if (usr->f_timeFlavor == 1) {
            errSprintf ("Inconsistent time flavor.\n"
                        "Either use startTime, endTime, or startDate, numDays,"
                        "but don't mix them.\n");
            return -1;
         }
         usr->f_timeFlavor = 2;
         if ((usr->f_valTime != 1) && (usr->f_valTime != 3)) {
            /* f_gmt = 0 no adjust, 1 adjust as LDT, 2 adjust as LST */
            if (Clock_Scan (&(usr->startTime), next, 0) != 0) {
               return 2;
            }
            /* Shift startTime to 5 z of that day. */
            totDay = (sInt4) floor (usr->startTime / SEC_DAY);
            usr->startTime = totDay * SEC_DAY + 5 * 3600.;
            if (usr->f_valTime == -1) {
               /* No num days yet */
               usr->f_valTime = 1;
            } else {
               /* have num days */
               usr->f_valTime = 3;
               usr->endTime = (totDay + usr->numDays) * SEC_DAY + 12 * 3600.;
            }
/*            printf ("start Time '%s' %f\n", next, usr->startTime); */
            return 2;
         }
         return 2;
      case NUMDAYS:
         if (usr->f_timeFlavor == 1) {
            errSprintf ("Inconsistent time flavor.\n"
                        "Either use startTime, endTime, or startDate, numDays,"
                        "but don't mix them.\n");
            return -1;
         }
         usr->f_timeFlavor = 2;
         if ((usr->f_valTime != 2) && (usr->f_valTime != 3)) {
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->numDays = li_temp;
            if (usr->f_valTime == -1) {
               /* No start date yet */
               usr->f_valTime = 2;
               /* Calc endTime based on cur date. */
               totDay = (sInt4) floor (Clock_Seconds() / SEC_DAY);
               usr->endTime = (totDay + usr->numDays) * SEC_DAY + 12 * 3600.;
            } else {
               /* Have start date */
               usr->f_valTime = 3;
               totDay = (sInt4) floor (usr->startTime / SEC_DAY);
               usr->endTime = (totDay + usr->numDays) * SEC_DAY + 12 * 3600.;
            }
         }
      case SIMPLEWX_VER:
         if (usr->f_SimpleVer == -1) {
/*            usr->f_SimpleVer = atoi (next); */
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_SimpleVer = li_temp;
         }
         return 2;
      case SIMPLEWWA:
         if (usr->f_SimpleWWA == -1) {
/*            usr->f_SimpleVer = atoi (next); */
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            if ((li_temp != 1) && (li_temp != 2) && (li_temp != 3)) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_SimpleWWA = li_temp;
         }
         return 2;
      case GRADS:
         if (usr->f_GrADS == -1) {
/*            usr->f_GrADS = atoi (next);*/
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_GrADS = li_temp;
         }
         return 2;
      case NETCDF:
         if (usr->f_NetCDF == -1) {
/*            usr->f_NetCDF = atoi (next); */
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_NetCDF = li_temp;
         }
         return 2;
      case XML:
         if (usr->f_XML == -1) {
            if (strcmp (next, "time-series") == 0) {
               usr->f_XML = 1;
            } else if (strcmp (next, "glance") == 0) {
               usr->f_XML = 2;
            } else if ((strcmp (next, "12-hourly") == 0) ||
                       (strcmp (next, "12 hourly") == 0)) {
               usr->f_XML = 3;
            } else if ((strcmp (next, "24-hourly") == 0) ||
                       (strcmp (next, "24 hourly") == 0)) {
               usr->f_XML = 4;
            } else if (strcmp (next, "rtma") == 0) {
               usr->f_XML = 5;
            } else {
               if (myAtoI (next, &(li_temp)) != 1) {
                  errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
                  return -1;
               }
               usr->f_XML = li_temp;
            }
            if ((usr->f_XML == 1) || (usr->f_XML == 2) || (usr->f_XML == 5)) {
               if (usr->f_timeFlavor == 2) {
                  errSprintf ("Inconsistent time flavor.\n"
                              "XML 1,2,5 use startTime, endTime\n"
                              "XML 3,4 use startDate, numDays\n");
                  return -1;
               }
               usr->f_timeFlavor = 1;
            } else if ((usr->f_XML == 3) || (usr->f_XML == 4)) {
               if (usr->f_timeFlavor == 1) {
                  errSprintf ("Inconsistent time flavor.\n"
                              "XML 1,2,5 use startTime, endTime\n"
                              "XML 3,4 use startDate, numDays\n");
                  return -1;
               }
               usr->f_timeFlavor = 2;
            }
         }
         return 2;
      case NDFDCONVEN:
         if (usr->f_ndfdConven == -1) {
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_ndfdConven = li_temp;
         }
         return 2;
      case GRAPH:
         if (usr->f_Graph == -1) {
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_Graph = li_temp;
         }
         return 2;
      case MOTD:
         if (usr->f_MOTD == -1) {
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_MOTD = li_temp;
         }
         return 2;
      case GEODATA:
         if (usr->geoDataDir == NULL) {
            type = myStat (next, &perm, NULL, NULL);
            /* check if it is a directory */
            if (type != MYSTAT_ISDIR) {
               errSprintf ("'%s' is not a directory\n", next);
               return -1;
            }
            /* check that it is readable */
            if (!(perm & 4)) {
               errSprintf ("No read permissions on '%s'\n", next);
               return -1;
            }
            usr->geoDataDir = (char *) malloc ((strlen (next) + 1) *
                                               sizeof (char));
            strcpy (usr->geoDataDir, next);
            return 2;
         }
      case RTMADIR:
         if (usr->rtmaDataDir == NULL) {
            type = myStat (next, &perm, NULL, NULL);
            /* check if it is a directory */
            if (type != MYSTAT_ISDIR) {
               errSprintf ("'%s' is not a directory\n", next);
               return -1;
            }
            /* check that it is readable */
            if (!(perm & 4)) {
               errSprintf ("No read permissions on '%s'\n", next);
               return -1;
            }
            usr->rtmaDataDir = (char *) malloc ((strlen (next) + 1) *
                                                 sizeof (char));
            strcpy (usr->rtmaDataDir, next);
            return 2;
         }
      case LAMPDIR:
         if (usr->lampDataDir == NULL) {
            type = myStat (next, &perm, NULL, NULL);
            /* check if it is a directory */
            if (type != MYSTAT_ISDIR) {
               errSprintf ("'%s' is not a directory\n", next);
               return -1;
            }
            /* check that it is readable */
            if (!(perm & 4)) {
               errSprintf ("No read permissions on '%s'\n", next);
               return -1;
            }
            usr->lampDataDir = (char *) malloc ((strlen (next) + 1) *
                                                 sizeof (char));
            strcpy (usr->lampDataDir, next);
            return 2;
         }
      case GRIBFILTER:
         if (usr->gribFilter == NULL) {
            usr->gribFilter = (char *) malloc ((strlen (next) + 1) *
                                               sizeof (char));
            strcpy (usr->gribFilter, next);
         }
         return 2;
      case NDFDVARS:
         if (usr->ndfdVarsBuff == NULL) {
            usr->ndfdVarsBuff = (char *) malloc ((strlen (next) + 1) *
                                                 sizeof (char));
            strcpy (usr->ndfdVarsBuff, next);
         }
         return 2;
      case CWA:
         usr->numCWA++;
         usr->cwaBuff = (char **) realloc (usr->cwaBuff, usr->numCWA 
                        * sizeof (char*));
         usr->cwaBuff[usr->numCWA - 1] = (char *) malloc ((strlen (next)
                                         + 1) * sizeof (char));
         strToLower(next);
         strcpy (usr->cwaBuff[usr->numCWA - 1], next);
         return 2;
      case PNT:
         if (myCommaDoubleList2 (next, &lat, &lon) != 0) {
            errSprintf ("invalid -pnt option '%s'\n", next);
            return -1;
         }
         usr->numPnt++;
         usr->pnt =
               (Point *) realloc (usr->pnt, usr->numPnt * sizeof (Point));
         usr->pnt[usr->numPnt - 1].Y = lat;
         usr->pnt[usr->numPnt - 1].X = lon;
         /* Reason we don't check bounds here is because "f_pntType" can
          * over-ride whether it is lat/lon or grid X/Y. */
         return 2;
      case LWLF:
         if (usr->lwlf.lat == -100) {
            if (myCommaDoubleList2 (next, &lat, &lon) != 0) {
               errSprintf ("invalid -lwlf option '%s'\n", next);
               return -1;
            }
            if ((lat < -90) || (lat > 90)) {
               errSprintf ("in -lwlf option, invalid lat of %f\n", lat);
               return -1;
            }
            if ((lon < -360) || (lon > 360)) {
               errSprintf ("in -lwlf option, invalid lat of %f\n", lat);
               return -1;
            }
            usr->lwlf.lat = lat;
            usr->lwlf.lon = lon;
         }
         return 2;
      case UPRT:
         if (usr->uprt.lat == -100) {
            if (myCommaDoubleList2 (next, &lat, &lon) != 0) {
               errSprintf ("invalid -uprt option '%s'\n", next);
               return -1;
            }
            if ((lat < -90) || (lat > 90)) {
               errSprintf ("in -uprt option, invalid lat of %f\n", lat);
               return -1;
            }
            if ((lon < -360) || (lon > 360)) {
               errSprintf ("in -uprt option, invalid lat of %f\n", lat);
               return -1;
            }
            usr->uprt.lat = lat;
            usr->uprt.lon = lon;
         }
         return 2;
      case CELLS:
         if (usr->f_pntType == -1) {
            if (strcmp (next, "true") == 0) {
               usr->f_pntType = 1;
            } else if (strcmp (next, "all") == 0) {
               usr->f_pntType = 2;
            } else {
/*               usr->f_pntType = atoi (next);*/
               if (myAtoI (next, &(li_temp)) != 1) {
                  errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
                  return -1;
               }
               usr->f_pntType = li_temp;
            }
         }
         return 2;
      case SURFACE:
         if (usr->f_surface == -1) {
            if (strcmp (next, "short") == 0) {
               usr->f_surface = 1;
            } else if (strcmp (next, "long") == 0) {
               usr->f_surface = 2;
            } else {
               usr->f_surface = 0;
            }
         }
         return 2;
      case VALIDMAX:
         if (usr->f_validRange == -1) {
/*            usr->validMax = atof (next); */
            if (myAtoF (next, &(usr->validMax)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_validRange = 2;
         } else if (usr->f_validRange == 1) {
/*            usr->validMax = atof (next); */
            if (myAtoF (next, &(usr->validMax)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_validRange = 3;
         }
         return 2;
      case VALIDMIN:
         if (usr->f_validRange == -1) {
/*            usr->validMin = atof (next);*/
            if (myAtoF (next, &(usr->validMin)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_validRange = 1;
         } else if (usr->f_validRange == 2) {
/*            usr->validMin = atof (next);*/
            if (myAtoF (next, &(usr->validMin)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_validRange = 3;
         }
         return 2;
      case MSG_NUM:
         if (usr->msgNum == -1) {
            if (strcmp (next, "all") == 0) {
               usr->msgNum = 0;
               usr->subgNum = 0;
            } else {
               usr->msgNum = (int) atof (next);
               usr->subgNum = 0;
               ptr = strchr (next, '.');
               if (ptr != NULL) {
                  usr->subgNum = atoi (ptr + 1);
               }
            }
         }
         return 2;
      case POLYSHP:
         if (usr->f_poly == -1) {
            if ((strcmp (next, "small") == 0) || (strcmp (next, "1") == 0)) {
               usr->f_poly = 1;
            } else if ((strcmp (next, "big") == 0) ||
                       (strcmp (next, "2") == 0)) {
               usr->f_poly = 2;
            }
         }
         return 2;
      case INTERPOLATE:
         if (usr->f_coverageGrid == -1)
            usr->f_coverageGrid = 1;
         if (usr->f_interp == -1) {
            if ((strcmp (next, "near") == 0) || (strcmp (next, "1") == 0)) {
               /* true f_coverageGrid, false f_interp */
               usr->f_interp = 0;
            } else if ((strcmp (next, "bilinear") == 0) ||
                       (strcmp (next, "2") == 0)) {
               /* true f_coverageGrid, true f_interp */
               usr->f_interp = 1;
            }
         }
         return 2;
      case DECIMAL:
         if (usr->decimal == -1) {
/*            usr->decimal = (sChar) atof (next); */
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->decimal = (sChar) li_temp;
         }
         return 2;
      case LATLON_DECIMAL:
         if (usr->LatLon_Decimal == -1) {
/*            usr->LatLon_Decimal = (sChar) atof (next); */
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->LatLon_Decimal = (sChar) li_temp;
         }
         return 2;
      case NAME_STYLE:
         if (usr->nameStyle == NULL) {
            i_temp = -1;
            len = strlen (next);
            if ((len == 2) || (len == 1)) {
               if ((isdigit (next[0])) && ((len == 1) || (isdigit (next[1])))) {
                  i_temp = atoi (next);
               }
            }
            switch (i_temp) {
               case -1:
                  usr->nameStyle = (char *) malloc ((strlen (next) + 1) *
                                                    sizeof (char));
                  strcpy (usr->nameStyle, next);
                  break;
               case 0:
                  usr->nameStyle = (char *)
                        malloc ((strlen ("%e_%V_%y%x.txt")
                                 + 1) * sizeof (char));
                  strcpy (usr->nameStyle, "%e_%V_%y%x.txt");
                  break;
               case 1:
                  usr->nameStyle = (char *) malloc ((strlen ("%e_%R_%p_%y%x."
                                                             "txt") + 1) *
                                                    sizeof (char));
                  strcpy (usr->nameStyle, "%e_%R_%p_%y%x.txt");
                  break;
               case 2:
                  usr->nameStyle = (char *) malloc ((strlen ("%e_%v.txt")
                                                     + 1) * sizeof (char));
                  strcpy (usr->nameStyle, "%e_%v.txt");
                  break;
               case 3:
                  usr->nameStyle = (char *) malloc ((strlen ("%e_%lv.txt")
                                                     + 1) * sizeof (char));
                  strcpy (usr->nameStyle, "%e_%lv.txt");
                  break;
               case 4:
                  usr->nameStyle = (char *) malloc ((strlen ("%e_%v_%s.txt")
                                                     + 1) * sizeof (char));
                  strcpy (usr->nameStyle, "%e_%v_%s.txt");
                  break;
            }
         }
         return 2;
      case PNTSTYLE:
         if (usr->f_pntStyle == -1) {
/*            usr->f_pntStyle = (sChar) atoi (next); */
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_pntStyle = (sChar) li_temp;
         }
         return 2;
      case WXPARSE:
      case TXTPARSE:
         if (usr->f_WxParse == -1) {
/*            usr->f_WxParse = (sChar) atoi (next); */
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_WxParse = (sChar) li_temp;
         }
         return 2;
      case ICON:
         if (usr->f_icon == -1) {
            if (myAtoI (next, &(li_temp)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
            usr->f_icon = (sChar) li_temp;
         }
         return 2;
      case UNIT:
         if (usr->f_unit == -1) {
            if ((strcmp (next, "e") == 0) || (strcmp (next, "english") == 0)) {
               usr->f_unit = 1;
            } else if ((strcmp (next, "m") == 0) ||
                       (strcmp (next, "metric") == 0)) {
               usr->f_unit = 2;
            } else if ((strcmp (next, "n") == 0) ||
                       (strcmp (next, "none") == 0)) {
               usr->f_unit = 0;
            } else {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
         }
         return 2;
      case RADEARTH:
      case MAJEARTH:
         if (usr->majEarth == -1) {
/*            usr->majEarth = atof (next); */
            if (myAtoF (next, &(usr->majEarth)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
         }
         return 2;
      case MINEARTH:
         if (usr->minEarth == -1) {
/*            usr->minEarth = atof (next); */
            if (myAtoF (next, &(usr->minEarth)) != 1) {
               errSprintf ("Bad value to '%s' of '%s'\n", cur, next);
               return -1;
            }
         }
         return 2;
      default:
         errSprintf ("Invalid option '%s'\n", cur);
         return -1;
   }
}

/*****************************************************************************
 * UserParseCommandLine() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Parses the command line for options
 *
 * ARGUMENTS
 *    usr = The user option structure to update. (Output)
 * myArgc = How many arguments on command line to parse. (Input)
 * myArgv = The arguments on the command line. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Invalid usage.
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *
 * NOTES
 *****************************************************************************
 */
int UserParseCommandLine (userType *usr, int myArgc, char **myArgv)
{
   int ans;             /* The returned value from ParseUserChoice */

   while (myArgc > 0) {
      if (myArgc != 1) {
         ans = ParseUserChoice (usr, *myArgv, myArgv[1]);
      } else {
         ans = ParseUserChoice (usr, *myArgv, NULL);
         if (ans == 2) {
            errSprintf ("Option '%s' requires a second part\n", *myArgv);
            return -1;
         }
      }
      if (ans == -1) {
         return -1;
      }
      myArgc -= ans;
      myArgv += ans;
   }
   return 0;
}

/*****************************************************************************
 * UserParseConfigFile() -- Review 12/2002 -- Modified
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Parses a given file for user options
 *
 * ARGUMENTS
 *      usr = The user option structure to update. (Output)
 * filename = The file to parse. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Couldn't open the file for read.
 * -2 = Invalid usage.
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *  12/2002 AAT: Re-did fgets to use AllocFGets()
 *
 * NOTES
 *****************************************************************************
 */
int UserParseConfigFile (userType *usr, char *filename)
{
   FILE *fp;            /* Ptr to configuration file, to read options from. */
   char *buffer = NULL; /* Holds a line from the file. */
   size_t buffLen = 0;  /* Current length of buffer. */
   char *first;         /* The first word in buffer. */
   char *second;        /* The second word in buffer. */
   int ans;             /* The returned value from ParseUserChoice */

   if ((fp = fopen (filename, "rt")) == NULL) {
      errSprintf ("Couldn't open %s for read\n", filename);
      return -1;
   }
   while (reallocFGets (&buffer, &buffLen, fp) > 0) {
      first = strtok (buffer, " \n");
      if ((first != NULL) && (*first != '#')) {
         second = strtok (NULL, " \n");
         if ((second != NULL) && (*second == '#')) {
            second = NULL;
         }

         /* Add this line to the usr choices. */
         ans = ParseUserChoice (usr, first, second);
         if ((ans == 2) && (second == NULL)) {
            errSprintf ("In '%s', Option '%s' requires a second part\n",
                        filename, first);
            fclose (fp);
            free (buffer);
            return -2;
         } else if (ans == -1) {
            fclose (fp);
            free (buffer);
            return -2;
         }
      }
   }
   fclose (fp);
   free (buffer);
   return 0;
}

/*
 *  numInNames        number of input files.
 *  inNames           inName = -in
 *  f_inTypes         file type from stat (1=dir, 2=file, 3=unknown).
 *         after procedure f_inType may not corespond with inNames...
 *         so free it?
 *  filter          (Input) used to filter the directory search
 */
int expand_inName (size_t *NumInNames, char ***InNames,
                   char **F_inTypes, const char *filter)
{
   size_t i;
   size_t j;
   size_t argc = 0;
   char **argv = NULL;
   size_t numAns;
   size_t valAns;
   char **ans;
   char perm;

   numAns = *NumInNames;
   ans = (char **) malloc (numAns * sizeof (char *));
   valAns = 0;

   for (i = 0; i < *NumInNames; i++) {
      /* If it is a directory, Glob it, and free it */
      if ((*F_inTypes)[i] == MYSTAT_ISDIR) {
         if (myGlob ((*InNames)[i], filter, &argc, &argv) != 0) {
            errSprintf ("ERROR: couldn't open directory %s", (*InNames)[i]);
            /* give control of rest of memory in InNames to ans. */
            for (; i < *NumInNames; i++) {
               ans[valAns] = (*InNames)[i];
               valAns++;
            }
            /* Free InNames */
            free (*InNames);
            free (*F_inTypes);
            /* Return Ans */
            *InNames = ans;
            *NumInNames = valAns;
            *F_inTypes = NULL;
            return 1;
         }
         /* could test numAns vs valAns + argc + *NumInNames - i or something 
          * like that, prior to realloc but if I don't get the equation
          * right, could cause problems. */
         numAns += argc;
         ans = (char **) realloc (ans, numAns * sizeof (char *));
         for (j = 0; j < argc; j++) {
            /* If it is a file... give memory control to ans */
            if (myStat (argv[j], &perm, NULL, NULL) == MYSTAT_ISFILE) {
               /* check that it is readable */
               if (perm & 4) {
                  ans[valAns] = argv[j];
                  valAns++;
               } else {
                  free (argv[j]);
               }
               /* If it is a directory... free it */
            } else {
               free (argv[j]);
            }
         }
         argc = 0;
         free (argv);
         argv = NULL;
         free ((*InNames)[i]);
         (*InNames)[i] = NULL;
         /* If it is a file... give memory control to ans */
      } else {
         myAssert ((*F_inTypes)[i] == MYSTAT_ISFILE);
         ans[valAns] = (*InNames)[i];
         valAns++;
      }
   }
   /* Free InNames */
   free (*InNames);
   free (*F_inTypes);
   /* Return Ans */
   *InNames = ans;
   *NumInNames = valAns;
   *F_inTypes = NULL;
   return 0;
}
