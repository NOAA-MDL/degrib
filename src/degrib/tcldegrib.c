/*****************************************************************************
 * tcldegrib.c
 *
 * DESCRIPTION
 *    This file contains the Tcl extension library wrapper around the degrib
 * options.  What that means is, it contains all the interface code needed to
 * call the degrib routines from Tcl/Tk.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <tcl.h>
#if defined(_WINDOWS_)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <locale.h>
#endif

#ifdef _MBCS
  /* 
   * The following ifdef block is the MS "standard" way of creating macros which make
   * exporting from a DLL simpler.  All files within this DLL are compiled with the
   * HALO_EXPORTS symbol defined on the command line.  This symbol should not be defined
   * on any project that uses this DLL.  This way any other project whose source files
   * include this file see HALO_API functions as being imported from a DLL, wheras this
   * DLL sees symbols defined with this macro as being exported.
   */
#ifdef EXPORTS
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
#else
#define API extern
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include "myerror.h"
#include "write.h"
#include "interp.h"
#include "inventory.h"
#include "degrib2.h"
#include "userparse.h"
#include "commands.h"
#include "clock.h"
#ifdef HALO
  #include "imgpix.h"
  #include "mercator.h"
  #include "halo2.h"
  #include "scan.h"
  #include "metaname.h"
#else
typedef struct {
  int pen;
  double min, max;
  int f_type; /* 0 = (a,b), 1=(a,b], 2=[a,b), 3=[a,b] */
} penRangeType;
#endif

/* A structure containing what this particular command knows about the current
 * (or last) grib2 file it was working with.  */
typedef struct {
   unsigned int UID;    /* Command's Unique ID */
   IS_dataType is;      /* Un-parsed meta data for this GRIB2 message. As
                         * well as some memory used by the unpacker. */
/* Inventory info about the file. */
   sChar f_validInv;    /* True if inv is valid for this file */
   uInt4 LenInv;        /* Len of inventory (also # of messages in file) */
   inventoryType *Inv;  /* Inventory of the GRIB2 File. */
} Grib2Type;

/*****************************************************************************
 * Grib2Init() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Initialized all values of the (tcl grib (command / object) data
 * structure) to something... either -1 or NULL.  This does not set them
 * to their default values, rather to their "undefined" values. (Can be
 * thought of as a constructor)
 *
 * ARGUMENTS
 * grib = The structure associated with this tcl grib command. (In/Out)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static void Grib2Init (Grib2Type * grib)
{
   grib->UID = -1;
   IS_Init (&(grib->is));
   grib->f_validInv = 0;
   grib->LenInv = 0;
   grib->Inv = NULL;
}

/*****************************************************************************
 * Grib2CmdDel() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Free's all data associated with this (tcl grib (command / object) data
 * structure).  (Can be thought of as a Destructor)
 *
 * ARGUMENTS
 * clientData = A pointer to the The structure associated with this tcl
 *              grib command. (In/Out)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static void Grib2CmdDel (ClientData clientData)
{
   Grib2Type *grib = (Grib2Type *) clientData;
   int i;               /* Counter used to free each inventory. */

   IS_Free (&(grib->is));
   for (i = 0; i < grib->LenInv; i++) {
      GRIB2InventoryFree (grib->Inv + i);
   }
   free (grib->Inv);
   free (grib);
}

/*****************************************************************************
 * Grib2ParseObj() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Configures the (tcl grib (command / object) data structure) by parsing
 * the user specified options.
 *
 * ARGUMENTS
 *   grib = The structure associated with this tcl grib command. (In/Out)
 * interp = Tcl interpreter (holds the commands / data that Tcl knows) (In)
 *   objc = Number of parameters passed in. (Input)
 *   objv = Parameters as Tcl_Object rather than char ptrs. (Input)
 *    usr = holds the user's options. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int (could use errSprintf())
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *   7/2003 AAT: Added Tcl_ResetResult(interp) to clear any errors made while
 *          trying to figure out if "-nMet" or "-nMet true" were passed in.
 *   7/2003 AAT: Shifted usr out of the Grib2Type structure.
 *
 * NOTES
 *****************************************************************************
 */
static int Grib2ParseObj (Grib2Type * grib, Tcl_Interp * interp, int objc,
                          Tcl_Obj * CONST objv[], userType *usr)
{
   Tcl_Obj *CONST * objPtr = objv; /* Used to help parse objv. */
/* Following is const char in 8.5 but plain char in 8.3 */
   const char *Opt[] = {
      "-in", "-Flt", "-Shp", "-Met", "-msg", "-nameStyle", "-out", "-Interp",
      "-revFlt", "-MSB", "-Unit", "-namePath", "-nMSB", "-nFlt", "-nShp",
      "-nMet", "-reset", "-poly", "-nMissing", "-Decimal", "-IS0", "-GrADS",
      "-SimpleWx", "-radEarth", "-Csv", "-nCsv", "-majEarth", "-minEarth",
      "-NetCDF", "-verboseShp", "-AscGrid", "-SimpleVer", NULL
   };
   enum {
      INFILE, FLT, SHP, META, MSG_NUM, NAME_STYLE, OUTFILE, INTERPOLATE,
      REVFLT, MSB, UNIT, NAMEPATH, NO_MSB, NO_FLT, NO_SHP, NO_META, RESET,
      POLY, NOMISS_SHP, DECIMAL, IS0, GRADS, SIMPLEWX, RADEARTH, CSV, NO_CSV,
      MAJEARTH, MINEARTH, NETCDF, VERBOSESHP, ASCGRID, SIMPLEVER
   };
   int index;           /* The current index into Opt. */
   int i_temp;          /* A temporary integer for use with Tcl APIs. */
   double d_temp;       /* A temporary double for use with Tcl APIs. */
   int len;             /* The length of string retrieved from Tcl object. */
   char *strPtr;        /* The string retrieved from a Tcl object. */
   int useArgs;         /* The number of arguments used for this option. */
   char *ptr;           /* Used to help parse -msg. */

   while (objc > 0) {
      if (Tcl_GetIndexFromObj (interp, objPtr[0], Opt, "option", 0, &index)
          != TCL_OK) {
         /* GetIndex puts a resonable return message in the interp. */
         return TCL_ERROR;
      }
      switch (index) {
         case INFILE:  /* Input file name of GRIB2 file. */
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            strPtr = Tcl_GetStringFromObj (objPtr[1], &len);
            usr->numInNames = 1;
            usr->inNames = (char **) malloc (usr->numInNames *
                                             sizeof (char *));
            usr->inNames[0] = (char *) malloc ((len + 1) * sizeof (char));
            strcpy (usr->inNames[0], strPtr);
            grib->f_validInv = 0;
/*
            i_temp = usr->numInNames;
            usr->numInNames++;
            usr->inNames = (char **) realloc (usr->inNames,
                                              usr->numInNames * sizeof (char *));
            usr->inNames[i_temp] = (char *) malloc ((len + 1) * sizeof (char));
*/
/*
            if ((usr->inName == NULL) || (strcmp (strPtr, usr->inName) != 0)) {
               usr->inName = (char *) realloc ((void *) usr->inName,
                                               (len + 1) * sizeof (char));
               strcpy (usr->inName, strPtr);
               grib->f_validInv = 0;
            }
*/
            useArgs = 2;
            break;
         case RESET:
            /* Reset the inventory for this command.  Intended for use after
             * one downloads a file. */
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               grib->f_validInv = 0;
               useArgs = 1;
            } else {
               if (i_temp) {
                  grib->f_validInv = 0;
               }
               useArgs = 2;
            }
            break;
         case OUTFILE: /* Name of output file. */
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            strPtr = Tcl_GetStringFromObj (objPtr[1], &len);
            usr->outName = (char *) realloc ((void *) usr->outName,
                                             (len + 1) * sizeof (char));
            strcpy (usr->outName, strPtr);
            useArgs = 2;
            break;
         case NAMEPATH: /* Name of path to output file (see -nameStyle) */
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            strPtr = Tcl_GetStringFromObj (objPtr[1], &len);
            usr->namePath = (char *) realloc ((void *) usr->namePath,
                                              (len + 1) * sizeof (char));
            strcpy (usr->namePath, strPtr);
            useArgs = 2;
            break;
         case FLT:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_Flt = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_Flt = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case NO_FLT:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_Flt = (sChar) 0;
               useArgs = 1;
            } else {
               usr->f_Flt = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case META:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_Met = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_Met = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case NO_META:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_Met = (sChar) 0;
               useArgs = 1;
            } else {
               usr->f_Met = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case IS0:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_IS0 = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_IS0 = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case SHP:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_Shp = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_Shp = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case VERBOSESHP:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_verboseShp = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_verboseShp = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case NO_SHP:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_Shp = (sChar) 0;
               useArgs = 1;
            } else {
               usr->f_Shp = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case CSV:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_Csv = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_Csv = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case NO_CSV:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_Csv = (sChar) 0;
               useArgs = 1;
            } else {
               usr->f_Csv = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case NETCDF:
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            if (Tcl_GetIntFromObj (interp, objPtr[1], &i_temp) == TCL_OK) {
               usr->f_NetCDF = (sChar) i_temp;
            } else {
               errSprintf ("Problems parsing -NetCDF option\n");
               return TCL_ERROR;
            }
            useArgs = 2;
            break;
         case SIMPLEVER:
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            if (Tcl_GetIntFromObj (interp, objPtr[1], &i_temp) == TCL_OK) {
               usr->f_SimpleVer = (sChar) i_temp;
            } else {
               errSprintf ("Problems parsing -SimpleVer option\n");
               return TCL_ERROR;
            }
            useArgs = 2;
            break;
         case MSG_NUM:
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            strPtr = Tcl_GetStringFromObj (objPtr[1], &len);
            if (strcmp (strPtr, "all") == 0) {
               usr->msgNum = 0;
               usr->subgNum = 0;
            } else if ((ptr = strchr (strPtr, '.')) != NULL) {
               usr->msgNum = (int) atof (strPtr);
               usr->subgNum = atoi (ptr + 1);
            } else if (Tcl_GetIntFromObj (interp, objPtr[1], &i_temp) ==
                       TCL_OK) {
               usr->msgNum = i_temp;
               usr->subgNum = 0;
            } else {
               errSprintf ("Problems parsing -msg option\n");
               return TCL_ERROR;
            }
            useArgs = 2;
            break;
         case DECIMAL:
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            if (Tcl_GetIntFromObj (interp, objPtr[1], &i_temp) == TCL_OK) {
               usr->decimal = i_temp;
            } else {
               errSprintf ("Problems parsing -Decimal option\n");
               return TCL_ERROR;
            }
            useArgs = 2;
            break;
         case UNIT:
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            strPtr = Tcl_GetStringFromObj (objPtr[1], &len);
            if ((strcmp (strPtr, "e") == 0) ||
                (strcmp (strPtr, "english") == 0)) {
               usr->f_unit = 1;
            } else if ((strcmp (strPtr, "m") == 0) ||
                       (strcmp (strPtr, "metric") == 0)) {
               usr->f_unit = 2;
            } else if ((strcmp (strPtr, "n") == 0) ||
                       (strcmp (strPtr, "none") == 0)) {
               usr->f_unit = 0;
            }
            useArgs = 2;
            break;
         case RADEARTH:
            if ((objc == 1) ||
                (Tcl_GetDoubleFromObj (interp, objPtr[1], &(d_temp)) !=
                 TCL_OK)) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            usr->majEarth = d_temp;
            useArgs = 2;
            break;
         case MAJEARTH:
            if ((objc == 1) ||
                (Tcl_GetDoubleFromObj (interp, objPtr[1], &(d_temp)) !=
                 TCL_OK)) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            usr->majEarth = d_temp;
            useArgs = 2;
            break;
         case MINEARTH:
            if ((objc == 1) ||
                (Tcl_GetDoubleFromObj (interp, objPtr[1], &(d_temp)) !=
                 TCL_OK)) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            usr->minEarth = d_temp;
            useArgs = 2;
            break;
         case ASCGRID:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_AscGrid = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_AscGrid = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case NAME_STYLE:
            if (objc == 1) {
               errSprintf ("Invalid usage\n");
               errSprintf ("Possible options <%S>\n", Opt);
               return TCL_ERROR;
            }
            strPtr = Tcl_GetStringFromObj (objPtr[1], &len);
            i_temp = -1;
            if ((len == 2) || (len == 1)) {
               if ((isdigit (strPtr[0])) &&
                   ((len == 1) || (isdigit (strPtr[1])))) {
                  i_temp = atoi (strPtr);
               }
            }
            switch (i_temp) {
               case -1:
                  usr->nameStyle = (char *) malloc ((strlen (strPtr) + 1) *
                                                    sizeof (char));
                  strcpy (usr->nameStyle, strPtr);
                  break;
               case 0:
                  usr->nameStyle = (char *)
                        malloc ((strlen ("%e_%V_%y%x.txt") +
                                 1) * sizeof (char));
                  strcpy (usr->nameStyle, "%e_%V_%y%x.txt");
                  break;
               case 1:
                  usr->nameStyle = (char *)
                        malloc ((strlen ("%e_%R_%p_%y%x.txt")
                                 + 1) * sizeof (char));
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
            useArgs = 2;
            break;
         case INTERPOLATE:
            usr->f_coverageGrid = 1;
            if (objc == 1) {
               usr->f_interp = 1;
               useArgs = 1;
            } else {
               if (Tcl_GetIntFromObj (interp, objPtr[1], &i_temp) != TCL_OK) {
                  strPtr = Tcl_GetStringFromObj (objPtr[1], &len);
                  if (strcmp (strPtr, "near") == 0) {
                     usr->f_interp = 0;
                  } else if (strcmp (strPtr, "bilinear") == 0) {
                     usr->f_interp = 1;
                  } else {
                     errSprintf ("Problems parsing -Interp option\n");
                     return TCL_ERROR;
                  }
               } else {
                  if (i_temp == 1) {
                     usr->f_interp = 0;
                  } else {
                     usr->f_interp = 1;
                  }
               }
               useArgs = 2;
            }
            break;
         case POLY:
            if (objc == 1) {
               usr->f_poly = (sChar) 1;
               useArgs = 1;
            } else {
               if (Tcl_GetIntFromObj (interp, objPtr[1], &i_temp) != TCL_OK) {
                  strPtr = Tcl_GetStringFromObj (objPtr[1], &len);
                  if (strcmp (strPtr, "small") == 0) {
                     usr->f_poly = (sChar) 1;
                     useArgs = 2;
                     break;
                  } else if (strcmp (strPtr, "big") == 0) {
                     usr->f_poly = (sChar) 2;
                     useArgs = 2;
                     break;
                  } else {
                     errSprintf ("Problems parsing -poly option\n");
                     return TCL_ERROR;
                  }
               }
               usr->f_poly = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case NOMISS_SHP:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_nMissing = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_nMissing = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case GRADS:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_GrADS = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_GrADS = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case SIMPLEWX:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_SimpleWx = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_SimpleWx = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case REVFLT:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_revFlt = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_revFlt = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case MSB:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_MSB = (sChar) 1;
               useArgs = 1;
            } else {
               usr->f_MSB = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         case NO_MSB:
            if ((objc == 1) ||
                (Tcl_GetBooleanFromObj (interp, objPtr[1], &i_temp) !=
                 TCL_OK)) {
               usr->f_MSB = (sChar) 0;
               useArgs = 1;
            } else {
               usr->f_MSB = (sChar) i_temp;
               useArgs = 2;
            }
            break;
         default:
            /* Should not get here. */
            errSprintf ("Possible options <%S>\n", Opt);
            return TCL_ERROR;
      }                 /* end switch */
      objPtr += useArgs;
      objc -= useArgs;
   }
   Tcl_ResetResult (interp);
   return TCL_OK;

}

/*****************************************************************************
 * UpdateInventory() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Updates the (tcl grib (command / object) data structure) by doing an
 * inventory of the file specified by grib->name.
 *
 * ARGUMENTS
 *   grib = The structure associated with this tcl grib command. (In/Out)
 *    usr = holds the user's options. (Input)
 * numMsg = # of messages to inventory (0 = all, 1 = just first) (In)
 *
 * FILES/DATABASES:
 *   The grib2 file specified in grib->inName
 *
 * RETURNS: int (could use errSprintf())
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *   7/2003 AAT: Passed in numMsg to be # of messages to inventory.
 *               0 means all, 1 means just first one.
 *
 * NOTES
 *****************************************************************************
 */
static int UpdateInventory (Grib2Type * grib, userType *usr, int numMsg)
{
   int i;               /* Counter used to free each inventory. */
   int msgNum = 0;

   if (usr->numInNames == 0) {
      errSprintf ("ERROR: UpdateInventory called with usr->numInNames == "
                  "0\n");
      return TCL_ERROR;
   }
   /* Make sure that Inv is free before filling it. */
   for (i = 0; i < grib->LenInv; i++) {
      GRIB2InventoryFree (grib->Inv + i);
   }
   free (grib->Inv);
   grib->Inv = NULL;
   grib->LenInv = 0;
   if (GRIB2Inventory (usr->inNames[0], &(grib->Inv), &(grib->LenInv),
                       numMsg, &msgNum) < 0) {
      errSprintf ("Problems inside of GRIB2Inventory?\n");
      errSprintf ("File = %s\n", usr->inNames[0]);
      return TCL_ERROR;
   }
   /* If numMsg is 0, then we have a valid Inventory */
   if (numMsg == 0) {
      grib->f_validInv = 1;
   } else {
      grib->f_validInv = 0;
   }
   return TCL_OK;
}

/*****************************************************************************
 * Grib2InventoryObj() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Performs an inventory on a GRIB2 file.  After calling UpdateInventory,
 * it parses the results and passes the resulting list back to Tcl.
 *
 * ARGUMENTS
 *   grib = The structure associated with this tcl grib command. (In/Out)
 * interp = Tcl interpreter (holds the commands that Tcl knows) (In)
 *    usr = holds the user's options. (Input)
 * resPtr = Used to push error messages onto Tcl Error stack. (Output)
 * numMsg = # of messages to inventory (0 = all, 1 = just first) (In)
 *
 * FILES/DATABASES:
 *   The grib2 file specified in grib->inName
 *
 * RETURNS: int
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *   3/2003 AAT: Changed so it returns the message number to Tcl/Tk.
 *   7/2003 AAT: Shifted the parsing of the user choices out of routine.
 *   7/2003 AAT: Passed in numMsg to be # of messages to inventory.
 *               0 means all, 1 means just first one.
 *
 * NOTES
 * 1) May need to be more strict about options passed into this command.
 *    Currently, -in doesn't have to be passed in, rather it grabs the most
 *    recent file it used.  Also it could pass in -Flt or similar Configure
 *    command options, and it wouldn't complain.
 *****************************************************************************
 */
static int Grib2InventoryObj (Grib2Type * grib, Tcl_Interp * interp,
                              userType *usr, Tcl_Obj * resPtr, int numMsg)
{
   char *msg;           /* Used to pop messages off the error Stack. */
   int i;               /* counter for each message in the inventory. */
   char timeBuff[25];   /* Used to pass back the time to Tcl. */
   double delta;        /* The change in time between reference and valid
                         * time in hours.  Can be thought of as projection. */
   Tcl_Obj *tempList;   /* A list of all data for a particular GRIB2 message. 
                         */
   Tcl_Obj *ansList;    /* The list of "tempList"s to return to Tcl. */
   char buffer[30];     /* Used when constructing msgNum.subgNum. Size should 
                         * be at most 10.10\0 = 22. */

   /* Inventory this file, if we haven't inventoried it before. */
   if ((!grib->f_validInv) || (numMsg != 0)) {
      if (UpdateInventory (grib, usr, numMsg) != TCL_OK) {
         msg = errSprintf (NULL);
         Tcl_AppendStringsToObj (resPtr, "ERROR: In call to "
                                 "UpdateInventory.\n", msg, NULL);
         free (msg);
         return TCL_ERROR;
      }
   }
   /* Do the Tcl equivalent of InventoryPrint. */
   ansList = Tcl_NewListObj (0, NULL);
   for (i = 0; i < grib->LenInv; i++) {
      tempList = Tcl_NewListObj (0, NULL);
      sprintf (buffer, "%d.%d", grib->Inv[i].msgNum, grib->Inv[i].subgNum);
/* *INDENT-OFF* */
      Tcl_ListObjAppendElement (interp, tempList, Tcl_NewStringObj (
                                buffer, strlen (buffer)));
      Tcl_ListObjAppendElement (interp, tempList, Tcl_NewStringObj (
                                grib->Inv[i].element,
                                strlen (grib->Inv[i].element)));
/*      strftime (timeBuff, 25, "%m/%d/%Y %H:%M",
                gmtime (&(grib->Inv[i].refTime)));
*/
      Clock_Print (timeBuff, 25, grib->Inv[i].refTime, "%m/%d/%Y %H:%M", 0);
      Tcl_ListObjAppendElement (interp, tempList, Tcl_NewStringObj (
                                timeBuff, strlen (timeBuff)));
/*      strftime (timeBuff, 25, "%m/%d/%Y %H:%M",
                gmtime (&(grib->Inv[i].validTime)));
*/
      Clock_Print (timeBuff, 25, grib->Inv[i].validTime, "%m/%d/%Y %H:%M", 0);
      Tcl_ListObjAppendElement (interp, tempList, Tcl_NewStringObj (
                                timeBuff, strlen (timeBuff)));
/* *INDENT-ON* */
      delta = (grib->Inv[i].validTime - grib->Inv[i].refTime) / 3600.;
      Tcl_ListObjAppendElement (interp, tempList, Tcl_NewDoubleObj (delta));
      delta = grib->Inv[i].foreSec / 3600.;
      Tcl_ListObjAppendElement (interp, tempList, Tcl_NewDoubleObj (delta));
/* *INDENT-OFF* */
      Tcl_ListObjAppendElement (interp, tempList, Tcl_NewStringObj (
                                grib->Inv[i].comment,
                                strlen (grib->Inv[i].comment)));
      if (grib->Inv[i].longFstLevel != NULL) {
         Tcl_ListObjAppendElement (interp, tempList, Tcl_NewStringObj (
                                   grib->Inv[i].longFstLevel,
                                   strlen (grib->Inv[i].longFstLevel)));
      } else {
         Tcl_ListObjAppendElement (interp, tempList, Tcl_NewStringObj (
                                   "unknown", strlen ("unknown")));
      }
/* *INDENT-ON* */
      Tcl_ListObjAppendElement (interp, ansList, tempList);
   }
   Tcl_SetObjResult (interp, ansList);
   return TCL_OK;
}

static int Grib2RefTimeObj (Tcl_Interp * interp, userType *usr,
                            Tcl_Obj * resPtr)
{
   char *msg;           /* Used to pop messages off the error Stack. */
   double refTime = 0;

   /* Force an inventory of this file. */
   if (GRIB2RefTime (usr->inNames[0], &refTime) < 0) {
      msg = errSprintf (NULL);
      Tcl_AppendStringsToObj (resPtr, "ERROR: In call to "
                              "GRIB2RefTime.\n%s", msg, NULL);
      free (msg);
      return TCL_ERROR;
   }
   Tcl_SetObjResult (interp, Tcl_NewDoubleObj (refTime));
   return TCL_OK;
}

/*****************************************************************************
 * Grib2IsGrib2Obj() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Checks to see if it can find a GRIB message in the file.
 *
 * ARGUMENTS
 *    usr = holds the user's options. (Input)
 * interp = Tcl interpreter (holds the commands / data that Tcl knows) (In)
 *
 * FILES/DATABASES:
 *   The grib2 file specified in grib->inName
 *
 * RETURNS: int
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *   3/2003 Arthur Taylor (MDL/RSIS): Created.
 *   7/2003 AAT: Shifted the parsing of the user choices out of routine.
 *
 * NOTES
 * 1) May need to be more strict about options passed into this command.
 *    Currently, -in doesn't have to be passed in, rather it grabs the most
 *    recent file it used.  Also it could pass in -Flt or similar Configure
 *    command options, and it wouldn't complain.
 *****************************************************************************
 */
static int Grib2IsGrib2Obj (userType *usr, Tcl_Interp * interp)
{
   char *msg;           /* Used to pop messages off the error Stack. */
   FILE *fp;            /* The opened GRIB2 file. */
   char *buff;          /* Holds the info between records. */
   uInt4 buffLen;       /* Length of info between records. */
   sInt4 sect0[SECT0LEN_WORD]; /* Holds the current Section 0. */
   uInt4 gribLen;       /* Length of the current GRIB message. */
   int version;         /* Which version of GRIB is in this message. */
   int grib_limit;      /* How many bytes to look for before the first "GRIB" 
                         * in the file.  If not found, is not a GRIB file. */
   char *ptr;           /* used to find the file extension. */

   buffLen = 0;
   buff = NULL;
   if ((fp = fopen (usr->inNames[0], "rb")) == NULL) {
      Tcl_SetObjResult (interp, Tcl_NewIntObj (0));
      return TCL_OK;
   }
   grib_limit = GRIB_LIMIT;
   ptr = strrchr (usr->inNames[0], '.');
   if (ptr != NULL) {
      if (strcmp (ptr, ".tar") == 0) {
         grib_limit = 5000;
      }
   }
   if (ReadSECT0 (fp, &buff, &buffLen, grib_limit, sect0, &gribLen,
                  &version) < 0) {
      msg = errSprintf (NULL);
      free (msg);
      Tcl_SetObjResult (interp, Tcl_NewIntObj (0));
   } else {
      Tcl_SetObjResult (interp, Tcl_NewIntObj (version));
   }
   fclose (fp);
   free (buff);
   return TCL_OK;
}

#ifdef HALO
static int GribDraw (Tcl_Obj *resPtr, userType *usr, FILE *grib_fp, IS_dataType *is,
                     grib_MetaData *meta, int UID, int zoomID, int drawPen,
                     int numRanges, penRangeType *ranges) {
   double *grib_Data;   /* The read in GRIB2 grid. */
   uInt4 grib_DataLen;  /* Size of Grib_Data. */
   sInt4 f_endMsg = 1;  /* 1 if we read the last grid in a GRIB message, or
                         * we haven't read any messages. */
   int subgNum;         /* The subgrid that we are looking for */
   int f_first = 1;     /* Is this the first message? */
   double min_lat, min_lon, max_lat, max_lon, split_lon;
   uInt4 i, j;          /* Loop over the grid cells. */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   sInt4 row;           /* The index into grib_Data for a given x,y pair *
                         * using scan-mode = 0100 = GRIB2BIT_2 */
   float ans;          /* The interpolated value at a given point. */
   LatLon point;
   XPoint pts[5];
   int pen;
   sInt4 * iain;
   float *ain;
   int k;
   double unitM, unitB; /* values in y = m x + b used for unit conversion. */
   char unitName[15];   /* Holds the string name of the current unit. */

   /* Set up inital state of data for unpacker. */
   grib_DataLen = 0;
   grib_Data = NULL;
   if (usr->msgNum == 0) {
      subgNum = 0;
   } else {
      subgNum = usr->subgNum;
   }

   /* Read the GRIB message. */
   if (ReadGrib2RecordFast (grib_fp, usr->f_unit, &grib_Data, &grib_DataLen,
                        meta, is, subgNum, usr->majEarth, usr->minEarth,
                        usr->f_SimpleVer, usr->f_SimpleWWA, &f_endMsg, &(usr->lwlf),
                        &(usr->uprt)) != 0) {
      free (grib_Data);
      return 1;
   }
   ComputeUnit (meta->convert, meta->unitName, usr->f_unit, &unitM, &unitB,
                unitName);

   /* Check that gds is valid before setting up map projection. */
   if (GDSValid (&meta->gds) != 0) {
      free (grib_Data);
      return 1;
   }
   /* Set up the map projection. */
   SetMapParamGDS (&map, &(meta->gds));
   Zoom_Bounds (zoomID, &min_lon, &min_lat, &max_lon, &max_lat);
   llm2llx_a (zoomID, &min_lat, &min_lon);
   llm2llx_a (zoomID, &max_lat, &max_lon);
   if (max_lon < min_lon) {
      min_lon -= 360;
   }
/* min_lon and max_lon are in range of -360..360 going east, but sma_lon and
	big_lon will be in range of -180..180 going east*/
   if ((min_lon < 0) && (max_lon > 0)) split_lon = min_lon;
   else                                split_lon = 0;
   while (split_lon <= -180) split_lon += 360;

   iain = is->iain;
   ain = (float *) is->iain;
   for (j = 1; j <= meta->gds.Ny; j++) {
      for (i = 1; i <= meta->gds.Nx; i++) {
         /* Get the i, j value. */
         XY2ScanIndex (&row, i, j, GRIB2BIT_2, meta->gds.Nx, meta->gds.Ny);
         if (meta->gridAttrib.fieldType) {
            ans = iain[row];
         } else {
            ans = ain[row];
         }
         if (meta->gridAttrib.f_miss == 1) {
            if (ans == meta->gridAttrib.missPri) {
               continue;
            }
         } else if (meta->gridAttrib.f_miss == 2) {
            if ((ans == meta->gridAttrib.missSec) ||
                (ans == meta->gridAttrib.missPri)) {
               continue;
            }
         }

         /* Convert the units. */
         if (unitM == -10) {
            ans = pow (10, ans);
         } else {
            ans = unitM * ans + unitB;
         }

         /* Use ans to determine pen */
         pen = -2;
         for (k=0; k < numRanges; k++) {
            if (ranges[k].f_type == 0) {
               if ((ans > ranges[k].min) && (ans < ranges[k].max)) {
                  pen = ranges[k].pen;
                  break;
               }
            } else if (ranges[k].f_type == 1) {
               if ((ans > ranges[k].min) && (ans <= ranges[k].max)) {
                  pen = ranges[k].pen;
                  break;
               }
            } else if (ranges[k].f_type == 2) {
               if ((ans >= ranges[k].min) && (ans < ranges[k].max)) {
                  pen = ranges[k].pen;
                  break;
               }
            } else {
               if ((ans >= ranges[k].min) && (ans <= ranges[k].max)) {
                  pen = ranges[k].pen;
                  break;
               }
            }
         }
         if (pen < 0) {
            continue;
         }

         /* No point concering ourselves with usr->f_interp, since the
          * bilinear value at a grid cell latice should be the same as the
          * nearest point which is the value at that grid cell. */
         myCxy2ll (&map, i, j, &point.lat, &point.lon);
         if ((point.lat < min_lat) || (point.lat > max_lat) ||
             (point.lon < min_lon) || (point.lon > max_lon)) {
            continue;
         }

         myCxy2ll (&map, i - .5, j  - .5, &point.lat, &point.lon);
         llx2llm (zoomID, &point);
         pts[0] = Zoom_ltln2xy (zoomID, point);
         myCxy2ll (&map, i + .5, j  - .5, &point.lat, &point.lon);
         llx2llm (zoomID, &point);
         pts[1] = Zoom_ltln2xy (zoomID, point);
         myCxy2ll (&map, i + .5, j  + .5, &point.lat, &point.lon);
         llx2llm (zoomID, &point);
         pts[2] = Zoom_ltln2xy (zoomID, point);
         myCxy2ll (&map, i - .5, j  + .5, &point.lat, &point.lon);
         llx2llm (zoomID, &point);
         pts[3] = Zoom_ltln2xy (zoomID, point);
         pts[4] = pts[0];
         Halo_FillPolygon (UID, pen, pts, 5);
         if (drawPen >= 0) {
            Halo_lines (UID, drawPen, pts, 5);
         }
      }
   }
/*
   if (MainConvert (usr, is, meta, grib_Data, grib_DataLen,
                    usr->f_unit, f_first) != 0) {
      free (grib_Data);
      return 1;
   }
*/

   free (grib_Data);
   return 0;
}
#endif

/*****************************************************************************
 * Grib2ConvertObj() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Converts a particular GRIB message to a file format as specified in the
 * options.  This could be either a .Flt set, or a .Shp set, with the
 * possibility of generating meta data.
 *
 * ARGUMENTS
 *     grib = The structure associated with this tcl grib command. (In/Out)
 *      usr = holds the user's options. (Input)
 * haloName = NULL, or if drawing, then name of image to draw to. (Input)
 *   zoomID = if drawing, unique id for zoom (Input)
 *  drawPen = if drawing, unique id for draw pen (Input)
 *  fillPen = if drawing, unique id for fill pen (Input)
 *   resPtr = Used to push error messages onto Tcl Error stack. (Output)
 *
 * FILES/DATABASES:
 *   The grib2 file specified in grib->inName
 *
 * RETURNS: int
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *   3/2003 AAT: Change the way we find the message since we now have
 *          msgNum.subgNum
 *   7/2003 AAT: memleak by free'ing outfile outside the loop.
 *   7/2003 AAT: Shifted the parsing of the user choices out of routine.
 *   7/2003 AAT: Shifted to MetaInit / MetaFree here, instead of as part
 *          of the grib data structure.  Helps Wx for some reason.
 *   8/2003 AAT Shifted most of this code to "commands.c::Grib2Convert()"
 *   5/2009 AAT Added f_draw.
 *
 * NOTES
 * 1) May need to be more strict about options passed into this command.
 *    Currently, -in doesn't have to be passed in, rather it grabs the most
 *    recent file it used.
 * 2) May have problems using nameStyle after a call using outName.
 * 3) nameStyle parse has to be done after message is read because it uses
 *    elements from the message in the file name.
 *****************************************************************************
 */
static int Grib2ConvertObj (Grib2Type * grib, userType *usr,
                            const char *haloName, int zoomID, int drawPen,
                            int numRanges, penRangeType *ranges, Tcl_Obj * resPtr)
{
   char *msg;           /* Used to pop messages off the error Stack. */
   FILE *grib_fp;       /* A file pointer to the GRIB2 file in question. */
   sInt4 offset;        /* Where we are in the GRIB2 file. */
   int i;               /* Loop counter to figure out the desired GRIB2
                         * Message. */
   grib_MetaData meta;  /* The meta structure for this GRIB2 message. */
#ifdef HALO
   int UID;             /* If drawing, then Unique ID for the image */
#endif

   /*
    * Inventory this file, if we haven't inventoried it before.  This is so
    * we can find a 2nd or 3rd message without having to do any work.
    */
   if (!grib->f_validInv) {
      if (UpdateInventory (grib, usr, 0) != TCL_OK) {
         msg = errSprintf (NULL);
         Tcl_AppendStringsToObj (resPtr, "ERROR: In call to "
                                 "UpdateInventory.\n", msg, NULL);
         free (msg);
         return TCL_ERROR;
      }
   }

   /* Make sure this message is in the file. */
   if ((usr->msgNum < 0) ||
       (usr->msgNum > grib->Inv[grib->LenInv - 1].msgNum)) {
      mallocSprintf (&msg, "%d is not in range [0..%d]", usr->msgNum,
                     grib->Inv[grib->LenInv - 1].msgNum);
      Tcl_AppendStringsToObj (resPtr, msg, NULL);
      free (msg);
      return TCL_ERROR;
   }

   /* Open file, and get to correct GRIB message */
   if ((grib_fp = fopen (usr->inNames[0], "rb")) == NULL) {
      Tcl_AppendStringsToObj (resPtr, "Problems opening ", usr->inNames[0],
                              " for read\n", NULL);
      return TCL_ERROR;
   }
   offset = 0;
   if (usr->msgNum != 0) {
      for (i = 0; i < grib->LenInv; i++) {
         if (grib->Inv[i].msgNum == usr->msgNum) {
            offset = grib->Inv[i].start;
            break;
         }
      }
      if (i == grib->LenInv) {
         Tcl_AppendStringsToObj (resPtr, "Problems finding the requested "
                                 "GRIB2 message number.", NULL);
         return TCL_ERROR;
      }
      fseek (grib_fp, offset, SEEK_SET);
   }
   MetaInit (&meta);

   if (haloName != NULL) {
      #ifdef HALO
      UID = Halo_UID(haloName);
      if (UID == -1) {
         Tcl_AppendStringsToObj (resPtr, "Couldn't find image: ",
                                 haloName, (char *) NULL);
         return TCL_ERROR;
      }
      if (GribDraw (resPtr, usr, grib_fp, &(grib->is), &meta, UID, zoomID,
                    drawPen, numRanges, ranges) != 0) {
         MetaFree (&meta);
         fclose (grib_fp);
         return TCL_ERROR;
      }
      #endif
   } else {
      printf ("%ld\n", ftell (grib_fp));
      if (Grib2Convert (usr, grib_fp, &(grib->is), &meta) != 0) {
         msg = errSprintf (NULL);
         Tcl_AppendStringsToObj (resPtr, "\nERROR: In call to "
                                 "Grib2Convert().\n", msg, NULL);
         free (msg);
         MetaFree (&meta);
         fclose (grib_fp);
         return TCL_ERROR;
      }
   }

   MetaFree (&meta);
   fclose (grib_fp);
   return TCL_OK;
}

/*****************************************************************************
 * Grib2CmdObj() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Main entry point from Tcl for the GRIB2 commands.  Figures out which
 * option the user is after, and calls the apporpriate C function.
 *
 * ARGUMENTS
 * clientData = The structure associated with this tcl grib command. (In/Out)
 *     interp = Tcl interpreter (holds the commands / data that Tcl knows)(In)
 *       objc = Number of parameters passed in. (Input)
 *       objv = Parameters as Tcl_Object rather than char ptrs. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *   7/2003 AAT: Shifted the parsing of the user choices to this routine.
 *  10/2003 AAT: Fixed memleak when called with invalid options.
 *               Added a UserFree() call before the return. 
 *
 * NOTES
 *****************************************************************************
 */
static int Grib2CmdObj (ClientData clientData, Tcl_Interp * interp,
                        int objc, Tcl_Obj * CONST objv[])
{
   enum {
      INQUIRE, CONVERT, ABOUT, INQUIRE2, CONVERT2, ABOUT2, ISGRIB2, REFTIME,
      DRAW
   };
/* Following is const char in 8.5 but plain char in 8.3 */
   const char *Cmd[] = { "inquire", "convert", "about", "-I", "-C", "-V",
      "isGrib2", "-refTime", "Draw", NULL
   };

   Grib2Type *grib = (Grib2Type *) clientData;
   int index;           /* The current index into Opt. */
   char *msg;           /* Used to generate error messages, and return
                         * verison info. */
   Tcl_Obj *resPtr;     /* Used to push error messages onto Tcl Error stack */
   userType usr;        /* holds the user's options. */
   int ans;             /* Return value of running the command */
   const char * haloName = NULL; /* For draw option, is the image's Unique ID */
   int zoomID = -1;     /* For draw option, is the zoom Unique ID */
   int drawPen = -1;    /* For draw option, is the drawPen Unique ID */
   const char **argvPtr;
   const char **argvPtr2;
   int argcPtr;
   int argcPtr2;
   int numRanges = 0;
   int i;
   penRangeType *ranges = NULL;

   /* Set errno to 0 when we start, if it is not already. */
   if (errno != 0) {
      errno = 0;
   }
   if (objc < 2) {
      mallocSprintf (&msg, "<%S> ?options?", Cmd);
      Tcl_WrongNumArgs (interp, 1, objv, msg);
      free (msg);
      return TCL_ERROR;
   }
   if (Tcl_GetIndexFromObj (interp, objv[1], Cmd, "command", 0, &index)
       != TCL_OK) {
      /* GetIndex puts a resonable return message in the interp. */
      return TCL_ERROR;
   }

   if (index == DRAW) {
      haloName = Tcl_GetString(objv[2]);
      zoomID = atoi (Tcl_GetString(objv[3]));
      drawPen = atoi (Tcl_GetString(objv[4]));
      Tcl_SplitList (interp, Tcl_GetString(objv[5]), &argcPtr, &argvPtr);
      numRanges = argcPtr;
      ranges = (penRangeType *) malloc (numRanges * sizeof (penRangeType));
      for (i = 0; i < numRanges; i++) {
         Tcl_SplitList (interp, argvPtr[i], &argcPtr2, &argvPtr2);
         ranges[i].pen = atoi (argvPtr2[0]);
         ranges[i].min = atof (argvPtr2[1]);
         ranges[i].max = atof (argvPtr2[2]);
         ranges[i].f_type = atoi (argvPtr2[3]);
         Tcl_Free((char *)argvPtr2);
      }
      Tcl_Free((char *)argvPtr);

      objc -= 4;
      objv += 4;
   }

   resPtr = Tcl_GetObjResult (interp);
   /* Parse the passed in command line options... */
   UserInit (&usr);
   if (Grib2ParseObj (grib, interp, objc - 2, objv + 2, &usr) != TCL_OK) {
      msg = errSprintf (NULL);
      /* It is NULL if something is already on the resPtr. */
      if (msg == NULL) {
         Tcl_AppendStringsToObj (resPtr, "\nERROR: in Grib2Parse.\n", NULL);
      } else {
         Tcl_AppendStringsToObj (resPtr, "ERROR: in Grib2Parse.\n", msg,
                                 NULL);
      }
      free (msg);
      UserFree (&usr);
      return TCL_ERROR;
   }

   switch (index) {
      case INQUIRE:
      case INQUIRE2:
         usr.f_Command = CMD_INVENTORY;
         break;
      case REFTIME:
         usr.f_Command = CMD_REFTIME;
         break;
      case CONVERT:
      case CONVERT2:
      case DRAW:
         usr.f_Command = CMD_CONVERT;
         break;
      case ABOUT:
      case ABOUT2:
         usr.f_Command = CMD_VERSION;
         break;
      case ISGRIB2:
         /* Don't have this on C side... Use CMD_INQUIRE. */
         usr.f_Command = CMD_INVENTORY;
         break;
      default:
         ans = TCL_ERROR; /* Should never be reached. */
   }
   /*
    * Validate that the options passed in won't break anything, and set some
    * defaults if needed.
    */
   if (UserValidate (&usr) != 0) {
      msg = errSprintf (NULL);
      Tcl_AppendStringsToObj (resPtr, "\nERROR: In UserValidate.\n", msg,
                              NULL);
      free (msg);
      UserFree (&usr);
      return TCL_ERROR;
   }
   switch (index) {
      case INQUIRE:
      case INQUIRE2:
         ans = Grib2InventoryObj (grib, interp, &usr, resPtr, 0);
         break;
      case REFTIME:
         ans = Grib2RefTimeObj (interp, &usr, resPtr);
         break;
      case CONVERT:
      case CONVERT2:
         ans = Grib2ConvertObj (grib, &usr, NULL, zoomID, drawPen, 0, NULL, resPtr);
         break;
      case DRAW:
         ans = Grib2ConvertObj (grib, &usr, haloName, zoomID, drawPen,
                                numRanges, ranges, resPtr);
         break;
      case ABOUT:
      case ABOUT2:
         msg = Grib2About ("tcldegrib");
         Tcl_AppendStringsToObj (resPtr, msg, NULL);
         free (msg);
         ans = TCL_OK;
         break;
      case ISGRIB2:
         ans = Grib2IsGrib2Obj (&usr, interp);
         break;
      default:
         ans = TCL_ERROR; /* Should never be reached. */
   }

   UserFree (&usr);
   return ans;
}

/*****************************************************************************
 * Grib2Init_CmdObj() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Creates a new GRIB2 command.  Can be thought of as the GRIB2 class
 * constructor.  After this, there is a new "instance" of the grib2 object,
 * which Tcl interacts with via Grib2CmdObj.
 *
 * ARGUMENTS
 * clientData = NULL (used to interface with Tcl_API's) ()
 *     interp = Tcl interpreter (holds the commands / data that Tcl knows)(In)
 *       objc = Number of parameters passed in. (Input)
 *       objv = Parameters as Tcl_Object rather than char ptrs. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
API int DLLEXPORT Grib2Init_CmdObj (ClientData clientData, Tcl_Interp * interp,
                          int objc, Tcl_Obj * CONST objv[])
{
   Grib2Type *grib;     /* Data structure associated with this command. */
   static int UID = 0;  /* A possible way to tell GRIB instance apart. */
   int len;             /* The length of string retrieved from Tcl object. */

   if ((objc < 2) || ((objc % 2) == 1)) {
      Tcl_WrongNumArgs (interp, 1, objv, "GribHandle ?options?");
      return TCL_ERROR;
   }

   /* Set up a new grib2 data structure. */
   grib = (Grib2Type *) malloc (sizeof (Grib2Type));
   grib->UID = UID++;
   Grib2Init (grib);
   /* Associate a command to the data structure. */
   Tcl_CreateObjCommand (interp, Tcl_GetStringFromObj (objv[1], &len),
                         Grib2CmdObj, (ClientData) grib,
                         (Tcl_CmdDeleteProc *) Grib2CmdDel);
   return TCL_OK;
}

/*****************************************************************************
 * Grib2_Init() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Initializes this extension in Tcl.  Creates a new command so Tcl can
 * call Grib2Init when it wants to create a new "instance" of the grib2
 * object.  Tcl then interacts the instance via Grib2CmdObj.
 *
 * ARGUMENTS
 *     interp = Tcl interpreter (holds the commands / data that Tcl knows)(In)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */

API int DLLEXPORT Grib2_Init (Tcl_Interp * interp)
{
/* If we are using stub libraries, we need the Tcl ones... */
#ifdef USE_TCL_STUBS
   if (Tcl_InitStubs (interp, "8.1", 0) == NULL) {
      return TCL_ERROR;
   }
#endif

   Tcl_CreateObjCommand (interp, "Grib2Init", Grib2Init_CmdObj,
                         (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

   /*
    * If we are using packages, tell people we have provided
    * grib2 version 1.0
    */
   if (Tcl_PkgProvide (interp, "grib2", "1.0") != TCL_OK) {
      return TCL_ERROR;
   }

   return TCL_OK;
}
