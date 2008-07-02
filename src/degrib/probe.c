/*****************************************************************************
 * probe.c
 *
 * DESCRIPTION
 *    This file contains the code needed to probe a GRIB2 file for all info
 * pertaining to the given lat/lon point or file containing points.
 *
 * Stores answer in ".prb" file using "-out" option for output filename.
 *
 *    Does bi-linear interpolation from the 4 surrounding grid cells to that
 * point.  If any of the 4 surrounding cells are missing, the probed point
 * gets a value of missing.
 *
 * HISTORY
 *   12/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "myerror.h"
#include "myutil.h"
#include "type.h"
#include "userparse.h"
#include "meta.h"
#include "degrib2.h"
#include "interp.h"
#include "probe.h"
#include "scan.h"
#include "mymapf.h"
#include "myassert.h"

/*****************************************************************************
 * PrintProbeWx() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Handle printing of Weather while probing.  Just prints enough to describe
 * the weather... Presumes calling routine will handle newlines, etc.
 *
 * ARGUMENTS
 *    out_fp = Where to print to. (Output)
 *       ans = index into the wx table to output. (Input)
 *        wx = The parsed wx data structure. (Input)
 *   logName = The name of a file to log messages to (or NULL) (Input)
 *    x1, y1 = The cell value we are probing (used for diagnostics) (Input)
 *  lat, lon = The location that we are probing (diagnositcs) (Input)
 * separator = User specified separator between fields in the output (Input)
 *   element = The name of this element (wx) (diagnositcs) (Input)
 *  unitName = The unit type of this element (none) (diagnositcs) (Input)
 *   comment = Long form of the name and unit (diagnositcs) (Input)
 *   refTime = The time the data was created (diagnositcs) (Input)
 * validTime = The time the forecast is valid for (diagnositcs) (Input)
 * f_WxParse = The user specified method for parsing this data (Input)
 *
 * FILES/DATABASES: None:
 *
 * RETURNS: Void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Broke this out of Probe()
 *   8/2003 AAT: Added -WxParse option.
 *   3/2004 AAT: Rewrote to be more flexible.
 *
 * NOTES
 *****************************************************************************
 */
static void PrintProbeWx (FILE *out_fp, double ans, sect2_WxType *wx,
                          char *logName, int x1, int y1, double lat,
                          double lon, char *separator, char *element,
                          char *unitName, char *comment, char *refTime,
                          char *validTime, sChar f_WxParse)
{
   sInt4 wxIndex;       /* The index into the wx table. */
   FILE *logFp;         /* Used to log errors in Wx keys. */
   size_t j;            /* loop counter over the weather keys. */

   wxIndex = (sInt4) ans;

   if (logName != NULL) {
      if ((logFp = fopen (logName, "at")) != NULL) {
         for (j = 0; j < wx->dataLen; j++) {
            if (wx->ugly[j].errors != NULL) {
               fprintf (logFp, "%s%s%s%s", refTime, separator, validTime,
                        separator);
               fprintf (logFp, "%s", wx->ugly[j].errors);
               fclose (logFp);
            }
         }
      }
   }

   if ((wxIndex >= 0) && (wxIndex < (sInt4) wx->dataLen)) {
      /* Print out the weather string according to f_WxParse. */
      switch (f_WxParse) {
         case 0:
            fprintf (out_fp, "%s", wx->data[wxIndex]);
            break;
         case 1:
            for (j = 0; j < NUM_UGLY_WORD; j++) {
               if (wx->ugly[wxIndex].english[j] != NULL) {
                  if (j != 0) {
                     if (j + 1 == wx->ugly[wxIndex].numValid) {
                        fprintf (out_fp, " and ");
                     } else {
                        fprintf (out_fp, ", ");
                     }
                  }
                  fprintf (out_fp, "%s", wx->ugly[wxIndex].english[j]);
               } else {
                  if (j == 0) {
                     fprintf (out_fp, "No Weather");
                  }
                  break;
               }
            }
            break;
         case 2:
            fprintf (out_fp, "%d", wx->ugly[wxIndex].SimpleCode);
            break;
      }
   } else {
      fprintf (out_fp, "%ld", wxIndex);
   }
}

static void PrintProbeHazard (FILE *out_fp, double ans, sect2_HazardType *hazard,
                              char *logName, int x1, int y1, double lat,
                              double lon, char *separator, char *element,
                              char *unitName, char *comment, char *refTime,
                              char *validTime, sChar f_WxParse)
{
   sInt4 hazIndex;       /* The index into the wx table. */
   size_t j;            /* loop counter over the weather keys. */

   hazIndex = (sInt4) ans;

   if ((hazIndex >= 0) && (hazIndex < (sInt4) hazard->dataLen)) {
      /* Print out the weather string according to f_WxParse. */
      switch (f_WxParse) {
         case 0:
            fprintf (out_fp, "%s", hazard->data[hazIndex]);
            break;
         case 1:
            for (j = 0; j < NUM_UGLY_WORD; j++) {
               if (hazard->haz[hazIndex].english[j] != NULL) {
                  if (j != 0) {
                     if (j + 1 == hazard->haz[hazIndex].numValid) {
                        fprintf (out_fp, " and ");
                     } else {
                        fprintf (out_fp, ", ");
                     }
                  }
                  fprintf (out_fp, "%s", hazard->haz[hazIndex].english[j]);
               } else {
                  if (j == 0) {
                     fprintf (out_fp, "No Weather");
                  }
                  break;
               }
            }
            break;
         case 2:
            fprintf (out_fp, "%d", hazard->haz[hazIndex].SimpleCode);
            break;
      }
   } else {
      fprintf (out_fp, "%ld", hazIndex);
   }
}

/*****************************************************************************
 * GRIB2ProbeStyle0() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Handle output'ing the original output style of probed data.  This form
 * is: "Element, unit, refTime, valTime, location1, location2, ... "
 *   Changed the algorithm for this to be concerned with just one line at a
 * time... The calling routine will loop appropriately.
 *
 * ARGUMENTS
 *      f_label = Flag if we just want to print the header out. (Input)
 *      pnt_fps = Where to print a particular point to. (Output)
 *   f_firstFps = If this is the first instance of the file pointer (In)
 *    grib_Data = Extracted grid to probe from. (Input)
 * grib_DataLen = Size of grib_Data. (Input)
 *          usr = User choices. (Input)
 *      numPnts = number of points to probe. (Input)
 *         pnts = lat/lon of points to probe. (Input)
 *       labels = Station Names for each point. (Input)
 *         meta = The meta structure for a GRIB2 message (Input)
 *          map = Used to compute the lat/lon points (Input)
 *      missing = The missing value for this grid (Input)
 *    f_surface = 0 => no surface info, 1 => short form of surface name
 *                2 => long form of surface name (In)
 *
 * FILES/DATABASES: None:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = Ok.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Broke this out of Probe()
 *   8/2003 AAT: Removed dependence on fileLen
 *   3/2004 AAT: Rewrote to be more flexible.
 *   5/2004 AAT: Modified so probes that are off the grid, return missing.
 *   1/2005 AAT: Added ability to send point outputs to different files.
 *
 * NOTES
 *****************************************************************************
 */
void GRIB2ProbeLabel0 (FILE **pnt_fps, char *f_firstFps,
                       char *separator, int numPnts, char **labels,
                       sChar f_surface)
{
   int i;               /* Counter for the points. */

   for (i = 0; i < numPnts; i++) {
         /* Print labels */
      if (f_firstFps[i]) {
         if (f_surface != 0) {
            fprintf (pnt_fps[i], "element%sunit%sSurface%srefTime"
                     "%svalidTime%s", separator, separator, separator,
                     separator, separator);
         } else {
            fprintf (pnt_fps[i], "element%sunit%srefTime%svalidTime%s",
                     separator, separator, separator, separator);
         }
      }
      if (i != numPnts - 1) {
         fprintf (pnt_fps[i], "%s%s", labels[i], separator);
      } else {
         fprintf (pnt_fps[i], "%s", labels[i]);
      }
   }
   for (i = 0; i < numPnts; i++) {
      if (f_firstFps[i]) {
         fprintf (pnt_fps[i], "\n");
      }
   }
}

static void GRIB2ProbeStyle0 (FILE **pnt_fps, char *f_firstFps,
                              double *grib_Data, sInt4 grib_DataLen,
                              userType *usr, int numPnts, Point * pnts,
                              grib_MetaData *meta, myMaparam *map,
                              double missing, sChar f_surface)
{
   int i;               /* Counter for the points. */
   char format[20];     /* Format to print the data with. */
   double newX, newY;   /* The location of lat/lon on the input grid. */
   sInt4 x1, y1;        /* The nearest grid point. */
   sInt4 row;           /* The index into grib_Data for a given x,y pair *
                         * using scan-mode = 0100 = GRIB2BIT_2 */
   double ans;          /* The interpolated value at a given point. */
   sChar f_missing;     /* flag whether the cell fell off the grid. */

   /* Print out probe data. */
   for (i = 0; i < numPnts; i++) {
      if (f_firstFps[i]) {
         if (meta->unitName != NULL) {
            fprintf (pnt_fps[i], "%s%s%s%s", meta->element,
                     usr->separator, meta->unitName, usr->separator);
         } else {
            fprintf (pnt_fps[i], "%s%s%s%s", meta->element,
                     usr->separator, meta->comment, usr->separator);
         }
         if (f_surface == 1) {
            fprintf (pnt_fps[i], "%s%s", meta->shortFstLevel,
                     usr->separator);
         } else if (f_surface == 2) {
            fprintf (pnt_fps[i], "%s%s", meta->longFstLevel,
                     usr->separator);
         }
         fprintf (pnt_fps[i], "%s%s%s%s", meta->refTime, usr->separator,
                  meta->validTime, usr->separator);
      }
   }

   sprintf (format, "%%.%df", usr->decimal);
   for (i = 0; i < numPnts; i++) {
      myCll2xy (map, pnts[i].Y, pnts[i].X, &newX, &newY);
      f_missing = 0;
      /* Find the nearest grid cell. */
      if (newX < .5) {
         x1 = 1;
         f_missing = 1;
      } else if ((newX + .5) > meta->gds.Nx) {
         x1 = meta->gds.Nx;
         f_missing = 1;
      } else {
         x1 = (sInt4) (newX + .5);
      }
      if (newY < .5) {
         y1 = 1;
         f_missing = 1;
      } else if ((newY + .5) > meta->gds.Ny) {
         y1 = meta->gds.Ny;
         f_missing = 1;
      } else {
         y1 = (sInt4) (newY + .5);
      }
      if (!(usr->f_interp)) {
         /* Get the x1, y1 value. */
         if (!f_missing) {
            XY2ScanIndex (&row, x1, y1, GRIB2BIT_2, meta->gds.Nx,
                          meta->gds.Ny);
            ans = grib_Data[row];
         } else {
            ans = missing;
         }
      } else {
         /* Figure out data value at this lat/lon */
         ans = BiLinearCompute (grib_Data, map, pnts[i].Y, pnts[i].X,
                                meta->gds.Nx, meta->gds.Ny,
                                meta->gridAttrib.f_miss, missing,
                                meta->gridAttrib.missSec, usr->f_avgInterp);
      }
      if (strcmp (meta->element, "Wx") == 0) {
         /* Handle the weather case. */
         PrintProbeWx (pnt_fps[i], ans, &(meta->pds2.sect2.wx),
                       usr->logName, x1, y1, pnts[i].Y, pnts[i].X,
                       usr->separator, meta->element, meta->unitName,
                       meta->comment, meta->refTime, meta->validTime,
                       usr->f_WxParse);
      } else if (strcmp (meta->element, "WWA") == 0) {
         /* Handle the hazard case. */
         PrintProbeHazard (pnt_fps[i], ans, &(meta->pds2.sect2.hazard),
                        usr->logName, x1, y1, pnts[i].Y, pnts[i].X,
                        usr->separator, meta->element, meta->unitName,
                        meta->comment, meta->refTime, meta->validTime,
                        usr->f_WxParse);
      } else {
         fprintf (pnt_fps[i], format, myRound (ans, usr->decimal));
      }
      if (i != numPnts - 1) {
         fprintf (pnt_fps[i], "%s", usr->separator);
      }
   }
   for (i = 0; i < numPnts; i++) {
      if (f_firstFps[i]) {
         fprintf (pnt_fps[i], "\n");
      }
   }
}

/*****************************************************************************
 * GRIB2ProbeStyle1() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Handle output'ing the first output style (after the original) of probed
 * data.  This form is: "location, Element[unit], refTime, valTime, value"
 *   Changed the algorithm for this to be concerned with just one line at a
 * time... The calling routine will loop appropriately.
 *
 * ARGUMENTS
 *      f_label = Flag if we just want to print the header out. (Input)
 *      pnt_fps = Where to print a particular point to. (Output)
 *   f_firstFps = If this is the first instance of the file pointer (In)
 *    grib_Data = Extracted grid to probe from. (Input)
 * grib_DataLen = Size of grib_Data. (Input)
 *          usr = User choices. (Input)
 *      numPnts = number of points to probe. (Input)
 *         pnts = lat/lon of points to probe. (Input)
 *       labels = Station Names for each point. (Input)
 *         meta = The meta structure for a GRIB2 message (Input)
 *          map = Used to compute the lat/lon points (Input)
 *      missing = The missing value for this grid (Input)
 *    f_surface = 0 => no surface info, 1 => short form of surface name
 *                2 => long form of surface name (In)
 *      f_cells = 0 => lat/lon pnts, 1 => cells in pnts, 2 => all Cells (In)
 *
 * FILES/DATABASES: None:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = Ok.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created
 *   8/2003 AAT: Removed dependence on fileLen
 *   3/2004 AAT: Rewrote to be more flexible, and to combine Style2() with
 *          Style1()... Style2() is now f_cells = 2, f_surface = 0, and
 *          the original Style1() is f_cells = 0, f_surface = 0
 *   5/2004 AAT: Modified so probes that are off the grid, return missing.
 *   1/2005 AAT: Added ability to send point outputs to different files.
 *
 * NOTES
 *****************************************************************************
 */
void GRIB2ProbeLabel1 (FILE **pnt_fps, char *f_firstFps,
                       char *separator, uInt4 numPnts,
                       char **labels, sChar f_surface, sChar f_cells)
{
   size_t i;            /* Counter for the points. */

   if (f_cells == 2) { /* All cells. */
      if (f_surface != 0) {
         fprintf (pnt_fps[0], "Location%sElement[Unit]%sSurface%srefTime"
                  "%svalidTime%sValue\n", separator, separator,
                  separator, separator, separator);
      } else {
         fprintf (pnt_fps[0], "Location%sElement[Unit]%srefTime"
                  "%svalidTime%sValue\n", separator, separator,
                  separator, separator);
      }
   } else {
      for (i = 0; i < numPnts; i++) {
         if (f_firstFps[i]) {
            if (f_surface != 0) {
               fprintf (pnt_fps[i], "Location%sElement[Unit]%sSurface"
                        "%srefTime%svalidTime%sValue\n", separator,
                        separator, separator, separator, separator);
            } else {
               fprintf (pnt_fps[i], "Location%sElement[Unit]%srefTime"
                        "%svalidTime%sValue\n", separator,
                        separator, separator, separator);
            }
         }
      }
   }
}

static void GRIB2ProbeStyle1 (FILE **pnt_fps,
                              char *f_firstFps, double *grib_Data,
                              sInt4 grib_DataLen, userType *usr,
                              uInt4 numPnts, Point * pnts, char **labels,
                              grib_MetaData *meta, myMaparam *map,
                              double missing, sChar f_surface, sChar f_cells)
{
   size_t i;            /* Counter for the points. */
   char format[20];     /* Format to print the data with. */
   double newX, newY;   /* The location of lat/lon on the input grid. */
   sInt4 x1, y1;        /* The nearest grid point. */
   sInt4 row;           /* The index into grib_Data for a given x,y pair *
                         * using scan-mode = 0100 = GRIB2BIT_2 */
   double ans;          /* The interpolated value at a given point. */
   double lat, lon;     /* The lat/lon at the grid cell. */
   sChar f_continue;    /* Flag to continue looping over the points or grid */
   sChar f_missing;     /* flag whether the cell fell off the grid. */

   /* Print labels */
   sprintf (format, "%%.%df", usr->decimal);
   f_continue = 1;
   i = 0;            /* counter over cells or lat/lon. */
   while (f_continue) {
      f_missing = 0;
      if (f_cells == 2) { /* All cells. */
         if (i == meta->gds.Nx * meta->gds.Ny) {
            f_continue = 0;
            break;
         }
         x1 = (i % meta->gds.Nx) + 1;
         y1 = (i / meta->gds.Nx) + 1;
         newX = x1;
         newY = y1;
         myCxy2ll (map, x1, y1, &lat, &lon);
         lat = myRound (lat, usr->LatLon_Decimal);
         lon = myRound (lon, usr->LatLon_Decimal);
         /* Get the x1, y1 value. */
         XY2ScanIndex (&row, x1, y1, GRIB2BIT_2, meta->gds.Nx,
                       meta->gds.Ny);
         ans = grib_Data[row];

      } else if (f_cells == 1) { /* Specified cells. */
         if (i == numPnts) {
            f_continue = 0;
            break;
         }
         x1 = (sInt4) pnts[i].X;
         y1 = (sInt4) pnts[i].Y;
         newX = x1;
         newY = y1;
         myCxy2ll (map, x1, y1, &lat, &lon);
         lat = myRound (lat, usr->LatLon_Decimal);
         lon = myRound (lon, usr->LatLon_Decimal);
         if (x1 < .5) {
            f_missing = 1;
         } else if ((x1 + .5) > meta->gds.Nx) {
            f_missing = 1;
         }
         if (y1 < .5) {
            f_missing = 1;
         } else if ((y1 + .5) > meta->gds.Ny) {
            f_missing = 1;
         }
         /* Get the x1, y1 value. */
         if (!f_missing) {
            XY2ScanIndex (&row, x1, y1, GRIB2BIT_2, meta->gds.Nx,
                          meta->gds.Ny);
            ans = grib_Data[row];
         } else {
            ans = missing;
         }

      } else {       /* lat/lon point. */
         if (i == numPnts) {
            f_continue = 0;
            break;
         }
         lat = pnts[i].Y;
         lon = pnts[i].X;
         /* Find the nearest grid cell. */
         myCll2xy (map, lat, lon, &newX, &newY);
         if (newX < .5) {
            x1 = 1;
            f_missing = 1;
         } else if ((newX + .5) > meta->gds.Nx) {
            x1 = meta->gds.Nx;
            f_missing = 1;
         } else {
            x1 = (sInt4) (newX + .5);
         }
         if (newY < .5) {
            y1 = 1;
            f_missing = 1;
         } else if ((newY + .5) > meta->gds.Ny) {
            y1 = meta->gds.Ny;
            f_missing = 1;
         } else {
            y1 = (sInt4) (newY + .5);
         }
         if (!(usr->f_interp)) {
            /* Get the x1, y1 value. */
            if (!f_missing) {
               XY2ScanIndex (&row, x1, y1, GRIB2BIT_2, meta->gds.Nx,
                             meta->gds.Ny);
               ans = grib_Data[row];
            } else {
               ans = missing;
            }
         } else {
            /* Figure out data value at this lat/lon */
            ans = BiLinearCompute (grib_Data, map, pnts[i].Y, pnts[i].X,
                                   meta->gds.Nx, meta->gds.Ny,
                                   meta->gridAttrib.f_miss, missing,
                                   meta->gridAttrib.missSec, usr->f_avgInterp);
         }
      }

      /* Print the first part of the line. */
      /* Find out if user doesn't want us to use labels[], for -cells all,
       * we never use labels[]. */
      if (f_cells == 2) {
         fprintf (pnt_fps[0], "(%f,%f,%f,%f)%s",
                  myRound (newX, usr->LatLon_Decimal),
                  myRound (newY, usr->LatLon_Decimal), lat, lon,
                  usr->separator);
         if (meta->unitName != NULL) {
            fprintf (pnt_fps[0], "%s%s%s", meta->element, meta->unitName,
                     usr->separator);
         } else {
            fprintf (pnt_fps[0], "%s%s%s", meta->element, meta->comment,
                     usr->separator);
         }
         if (f_surface == 1) {
            fprintf (pnt_fps[0], "%s%s", meta->shortFstLevel,
                     usr->separator);
         } else if (f_surface == 2) {
            fprintf (pnt_fps[0], "%s%s", meta->longFstLevel,
                     usr->separator);
         }
         fprintf (pnt_fps[0], "%s%s%s%s", meta->refTime, usr->separator,
                  meta->validTime, usr->separator);
         if (strcmp (meta->element, "Wx") != 0) {
            fprintf (pnt_fps[0], format, myRound (ans, usr->decimal));
         } else {
            /* Handle the weather case. */
            if (!f_missing) {
               PrintProbeWx (pnt_fps[0], ans, &(meta->pds2.sect2.wx),
                             usr->logName, x1, y1, lat, lon,
                             usr->separator, meta->element,
                             meta->unitName, meta->comment, meta->refTime,
                             meta->validTime, usr->f_WxParse);
            } else {
               fprintf (pnt_fps[0], "%.0f", ans);
            }
         }
         fprintf (pnt_fps[0], "\n");

      } else {
         if (usr->f_nLabel) {
            fprintf (pnt_fps[i], "(%f,%f,%f,%f)%s",
                     myRound (newX, usr->LatLon_Decimal),
                     myRound (newY, usr->LatLon_Decimal), lat, lon,
                     usr->separator);
         } else {
            fprintf (pnt_fps[i], "%s%s", labels[i], usr->separator);
         }
         if (meta->unitName != NULL) {
            fprintf (pnt_fps[i], "%s%s%s", meta->element, meta->unitName,
                     usr->separator);
         } else {
            fprintf (pnt_fps[i], "%s%s%s", meta->element, meta->comment,
                     usr->separator);
         }
         if (f_surface == 1) {
            fprintf (pnt_fps[i], "%s%s", meta->shortFstLevel,
                     usr->separator);
         } else if (f_surface == 2) {
            fprintf (pnt_fps[i], "%s%s", meta->longFstLevel,
                     usr->separator);
         }
         fprintf (pnt_fps[i], "%s%s%s%s", meta->refTime, usr->separator,
                  meta->validTime, usr->separator);
         if (strcmp (meta->element, "Wx") != 0) {
            fprintf (pnt_fps[i], format, myRound (ans, usr->decimal));
         } else {
            /* Handle the weather case. */
            if (!f_missing) {
               PrintProbeWx (pnt_fps[i], ans, &(meta->pds2.sect2.wx),
                             usr->logName, x1, y1, lat, lon,
                             usr->separator, meta->element,
                             meta->unitName, meta->comment, meta->refTime,
                             meta->validTime, usr->f_WxParse);
            } else {
               fprintf (pnt_fps[i], "%.0f", ans);
            }
         }
         fprintf (pnt_fps[i], "\n");
      }

      i++;
   }
}

int GRIB2ProbeOpenOutFile (userType *usr, int numPnts, char **pntFiles,
                           FILE *** pnt_fps, char ** f_firstFps)
{
   char f_default;      /* True if we need to open the default output file. */
   int i, j;            /* Loop counters. */
   char *outfile;       /* Temporary storage for output filename. */
   int outLen;          /* Length of outfile. */
   FILE *out_fp = NULL; /* The default output file. */
   char f_usedOut;      /* Flag if we've used out_fp yet. */

   /* Open the output files. */
   /* Find out if we need the "default" output file. */
   f_default = 1;
   if (usr->f_pntType != 2) {
      f_default = 0;
      for (i = 0; i < numPnts; i++) {
         if (pntFiles[i] == NULL) {
            f_default = 1;
            break;
         }
      }
   }

   if (f_default) {
      if (usr->f_stdout) {
         out_fp = stdout;
      } else if (usr->outName != NULL) {
         outLen = strlen (usr->outName);
         outfile = (char *) malloc ((outLen + 1) * sizeof (char));
         strcpy (outfile, usr->outName);
         outfile[outLen] = '\0';
         strncpy (outfile + outLen - 3, "prb", 3);
         if ((out_fp = fopen (outfile, "wt")) == NULL) {
            errSprintf ("ERROR: unable to open %s.\n", outfile);

            free (outfile);
            return -2;
         }
         free (outfile);
      } else {
         out_fp = stdout;
      }
   }

   if (usr->f_pntType != 2) {
      *pnt_fps = (FILE **) malloc (numPnts * sizeof (FILE *));
      *f_firstFps = (char *) malloc (numPnts * sizeof (char));
      f_usedOut = 0;
      for (i = 0; i < numPnts; i++) {
         if (pntFiles[i] == NULL) {
            (*pnt_fps)[i] = out_fp;
            if (f_usedOut) {
               (*f_firstFps)[i] = 0;
            } else {
               (*f_firstFps)[i] = 1;
               f_usedOut = 1;
            }
         } else {
            /* Find out if this is the first instance of this file. */
            (*f_firstFps)[i] = 1;
            for (j = 0; j < i; j++) {
               if (pntFiles[j] != NULL) {
                  if (strcmpNoCase (pntFiles[i], pntFiles[j]) == 0) {
                     (*f_firstFps)[i] = 0;
                     break;
                  }
               }
            }
            if (!((*f_firstFps)[i])) {
               (*pnt_fps)[i] = (*pnt_fps)[j];
            } else {
               if (((*pnt_fps)[i] = fopen (pntFiles[i], "wt")) == NULL) {
                  errSprintf ("ERROR: unable to open '%s'.\n", pntFiles[i]);
                  free (*pnt_fps);
                  free (*f_firstFps);
                  return -2;
               }
            }
         }
      }
   } else {
      (*pnt_fps) = (FILE **) malloc (sizeof (FILE *));
      (*f_firstFps) = (char *) malloc (sizeof (char));
      (*pnt_fps)[0] = out_fp;
      (*f_firstFps)[0] = 1;
   }
   return 0;
}

void GRIB2ProbeCloseOutFile (userType *usr, int numPnts, FILE ** pnt_fps,
                             char * f_firstFps)
{
   int i;            /* Loop counters. */

   if (usr->f_pntType != 2) {
      for (i = 0; i < numPnts; i++) {
         if (f_firstFps[i]) {
            fclose (pnt_fps[i]);
         }
      }
   } else {
      fclose (pnt_fps[0]);
   }
}

/*****************************************************************************
 * GRIB2Probe() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Main control procedure for the -P option.
 *
 * ARGUMENTS
 *        usr = User choices. (Input)
 *         is = Un-parsed meta data for this GRIB2 message. As well as some
 *              memory used by the unpacker. (Reduce memory load) (In)
 *       meta = The meta structure for a GRIB2 message.
 *              (Passed in to reduce memory load) (Input)
 * f_fileType = 0 for GRIB, 1 for data cube. (Input)
 *
 * FILES/DATABASES:
 *    Opens a GRIB2 file for reading given its filename.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = Ok.
 *
 * HISTORY
 *  12/2002 Arthur Taylor (MDL/RSIS): Created.
 *   5/2003 AAT: Modified it to look at usr->Interp to determine if we should
 *          perform bi-linear interpolation or just find nearest point.
 *   6/2003 AAT: Added refdate/time to just below label string.
 *   6/2003 AAT: Switched to a Warning and then averaging if Dx != Dy.
 *   7/2003 AAT: Proper handling of Dx != Dy.
 *   7/2003 Matthew T. Kallio (matt@wunderground.com):
 *          "If tests" had allowed : 0 <= newX < Nx + .5 (same for y).
 *          Should be : 0 <= newX < Nx
 *   7/2003 AAT: Added ability to override the radEarth.
 *   8/2003 AAT: Added -separator option.
 *   8/2003 AAT: Separated out the weather from this procedure.
 *   8/2003 AAT: Separated out the style 0 output.
 *   8/2003 AAT: Removed dependence on fileLen
 *  10/2003 AAT: Added ability to handle "-pnt all"
 *   3/2004 AAT: Rewrote to take some of the work out of Style0() and Style1()
 *   1/2005 AAT: Added ability to send point outputs to different files.
 *   9/2005 AAT: Fixed different behavior of -out stdout vs -stdout
 *
 * NOTES
 *   Passing 'is' and 'meta' in, mainly for tcldegrib memory considerations.
 *****************************************************************************
 */
int GRIB2Probe (userType *usr, int numPnts, Point * pnts, char **labels,
                char **pntFiles)
{
   FILE **pnt_fps = NULL; /* Array of output file pointers for the points. */
   char *f_firstFps = NULL; /* Array of flags saying it is the first pointer
                             * to a given output file. */
   FILE *grib_fp;       /* The opened grib2 file for input. */
   int i;               /* Loop counters. */
   sChar f_style;       /* 0 use Style0(), 1 use Style1() */
   sChar f_surface;     /* 0 no surface info, 1 short form of surface name */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   double missing = 0;  /* Missing value to use. */
   double *grib_Data;   /* Holds the grid retrieved from a GRIB2 message. */
   uInt4 grib_DataLen;  /* Current length of grib_Data. */
   int c;               /* Determine if end of the file without fileLen. */
   int subgNum = 0;     /* The subgrid in the message that we are interested
                         * in. */
   sInt4 f_endMsg = 1;  /* 1 if we read the last grid in a GRIB message */
#ifndef DP_ONLY
   IS_dataType is;      /* Un-parsed meta data for this GRIB2 message. As
                         * well as some memory used by the unpacker. */
   grib_MetaData meta;  /* The meta structure for this GRIB2 message. */
#endif

   /* Open the grib file. */
   if (usr->inNames[0] != NULL) {
      if ((grib_fp = fopen (usr->inNames[0], "rb")) == NULL) {
         errSprintf ("Problems opening %s for read\n", usr->inNames[0]);
         return -1;
      }
   } else {
      grib_fp = stdin;
   }

   if (GRIB2ProbeOpenOutFile (usr, numPnts, pntFiles, &pnt_fps,
                              &f_firstFps) != 0) {
      fclose (grib_fp);
      return -2;
   }

   f_surface = usr->f_surface;
   f_style = usr->f_pntStyle;
   if (f_style == 2) {
      f_style = 1;
      if (f_surface == 0) {
         f_surface = 1;
      }
   }
   if (usr->f_pntType == 2) {
      f_style = 1;
   }

   if (f_style == 0) {
/* Call GRIB2ProbeStyle0 for just header. */
      GRIB2ProbeLabel0 (pnt_fps, f_firstFps, usr->separator, numPnts, labels,
                        f_surface);
   } else {
/* Call GRIB2ProbeStyle1 for just header. */
      GRIB2ProbeLabel1 (pnt_fps, f_firstFps, usr->separator, numPnts, labels,
                        f_surface, usr->f_pntType);
   }

   /* Start loop for all messages. */
   grib_DataLen = 0;
   grib_Data = NULL;

#ifndef DP_ONLY
   MetaInit (&meta);
   IS_Init (&is);
#endif
   while ((c = fgetc (grib_fp)) != EOF) {
      ungetc (c, grib_fp);
      /* Read the GRIB message. */
      if (ReadGrib2Record (grib_fp, usr->f_unit, &grib_Data, &grib_DataLen,
                           &meta, &is, subgNum, usr->majEarth, usr->minEarth,
                           usr->f_SimpleVer, usr->f_SimpleWWA, &f_endMsg, &(usr->lwlf),
                           &(usr->uprt)) != 0) {
         preErrSprintf ("ERROR: In call to ReadGrib2Record.\n");
         for (i = 0; i < numPnts; i++) {
            if (f_firstFps[i]) {
               fclose (pnt_fps[i]);
            }
         }
         free (pnt_fps);
         free (f_firstFps);
         fclose (grib_fp);
         free (grib_Data);
#ifndef DP_ONLY
         MetaFree (&meta);
         IS_Free (&is);
#endif
         return -3;
      }

      if (usr->f_validRange > 0) {
         /* valid max. */
         if (usr->f_validRange > 1) {
            if (meta.gridAttrib.max > usr->validMax) {
               errSprintf ("ERROR: %f > valid Max of %f\n",
                           meta.gridAttrib.max, usr->validMax);
               for (i = 0; i < numPnts; i++) {
                  if (f_firstFps[i]) {
                     fclose (pnt_fps[i]);
                  }
               }
               free (pnt_fps);
               free (f_firstFps);
               fclose (grib_fp);
               free (grib_Data);
#ifndef DP_ONLY
               MetaFree (&meta);
               IS_Free (&is);
#endif
               return -3;
            }
         }
         /* valid min. */
         if (usr->f_validRange % 2) {
            if (meta.gridAttrib.min < usr->validMin) {
               errSprintf ("ERROR: %f < valid Min of %f\n",
                           meta.gridAttrib.min, usr->validMin);
               for (i = 0; i < numPnts; i++) {
                  if (f_firstFps[i]) {
                     fclose (pnt_fps[i]);
                  }
               }
               free (pnt_fps);
               free (f_firstFps);
               fclose (grib_fp);
               free (grib_Data);
#ifndef DP_ONLY
               MetaFree (&meta);
               IS_Free (&is);
#endif
               return -3;
            }
         }
      }
      if (f_endMsg != 1) {
         subgNum++;
      } else {
         subgNum = 0;
      }

      /* Check that gds is valid before setting up map projection. */
      if (GDSValid (&(meta.gds)) != 0) {
         preErrSprintf ("ERROR: Sect3 was not Valid.\n");
         for (i = 0; i < numPnts; i++) {
            if (f_firstFps[i]) {
               fclose (pnt_fps[i]);
            }
         }
         free (pnt_fps);
         free (f_firstFps);
         fclose (grib_fp);
         free (grib_Data);
#ifndef DP_ONLY
         MetaFree (&meta);
         IS_Free (&is);
#endif
         return -4;
      }
      /* Set up the map projection. */
      SetMapParamGDS (&map, &(meta.gds));

      /* Figure out a missing value, if there isn't one, so that when we
       * interpolate and we are out of bounds, we can return something. */
      if (meta.gridAttrib.f_miss == 0) {
         missing = 9999;
         if (meta.gridAttrib.f_maxmin) {
            if ((missing <= meta.gridAttrib.max) &&
                (missing >= meta.gridAttrib.min)) {
               missing = meta.gridAttrib.max + 1;
            }
         }
      } else {
         missing = meta.gridAttrib.missPri;
      }

      if (f_style == 0) {
         GRIB2ProbeStyle0 (pnt_fps, f_firstFps, grib_Data, grib_DataLen,
                           usr, numPnts, pnts, &meta, &map, missing, f_surface);
      } else {
         GRIB2ProbeStyle1 (pnt_fps, f_firstFps, grib_Data, grib_DataLen,
                           usr, numPnts, pnts, labels, &meta, &map, missing,
                           f_surface, usr->f_pntType);
      }
      MetaFree (&meta);
   }
   /* End loop for all messages. */
   free (grib_Data);
#ifndef DP_ONLY
   MetaFree (&meta);
   IS_Free (&is);
#endif

   GRIB2ProbeCloseOutFile (usr, numPnts, pnt_fps, f_firstFps);

   free (pnt_fps);
   free (f_firstFps);
   fclose (grib_fp);
   /* May need to close out_fp?  Probably not since it was in pnt_fps*/
   return 0;
}

/*****************************************************************************
 * GenericProbe() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Provide a hook to the probe routines that can be called by a fortran
 * program, in such a way as to avoid the "user" module.
 *
 * ARGUMENTS
 * filename = The GRIB file to probe. (Input)
 *  numPnts = The number of points to probe. (Input)
 *      lat = The latitudes of the points to probe. (Input)
 *      lon = The longitudes of the points to probe. (Input)
 * f_interp = true (1) if we want to perform bi-linear interp
 *            false (0) if we want nearest neighbor (Input)
 *  lenTime = The number of messages (or validTimes) in the file (Output)
 *  valTime = The array of valid times (as strings). (Output)
 *     data = The values at the various points. (Output)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int
 *   0 = Ok,
 *  -3 = couldn't open the file,
 *  -4 = Grid Definition Section was not valid.
 *  -1 = can't handle weather GRIB2 files.
 *
 * HISTORY
 *   8/2004 Arthur Taylor (MDL) + Xiaobiao Fan (OHD/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
/*
       DIMENSION LAT(NUMPNTS)
       DIMENSION LON(NUMPNTS)
       I_INTERP = 0
C FIGURE OUT HOW TO "DIMMENSION" 'DATA' AND 'VALTIME'
       DATA ("10",NUMPNTS)
       CHARACTER VALTIME ("10",LENOFVALTIME)
C       LENTIME TAKE THE PLACE OF "10"
       CALL GENERICPROBE (FILENAME, NUMPNTS, LAT, LON, I_INTERP, LENTIME,
      1                   VALTIME, DATA)
C FIGURE OUT HOW TO FREE 'DATA' AND 'VALTIME'

int GenericProbe_ (char *filename, int *numPnts, double *lat, double *lon,
                  int *f_interp, int *lenTime, char ***valTime,
                  double ***data) {
   return GenericProbe (filename, * numPnts, lat, lon, * f_interp, lenTime,
                        valTime, data);
}
*/

int GenericProbe (char *filename, int numPnts, double *lat, double *lon,
                  int f_interp, int *lenTime, char ***valTime, double ***data)
{
   FILE *grib_fp;       /* The opened grib2 file for input. */
   uInt4 grib_DataLen;  /* Current length of grib_Data. */
   double *grib_Data;   /* Holds the grid retrieved from a GRIB2 message. */
   int c;               /* Determine if end of the file without fileLen. */
   int f_unit = 1;      /* Convert to "english" (use F instead of K for
                         * temperature) */
   IS_dataType is;      /* Un-parsed meta data for this GRIB2 message. As
                         * well as some memory used by the unpacker. */
   grib_MetaData meta;  /* The meta structure for this GRIB2 message. */
   int subgNum = 0;     /* The subgrid in the message that we are interested
                         * in. */
   double majEarth = 0; /* If > 6000 use this to over-ride the radEarth. */
   double minEarth = 0; /* If > 6000 use this to over-ride the radEarth. */
   int f_SimpleVer = 4; /* We don't use Weather, but if we did use most
                         * recent version of the weather tables. */
   int f_SimpleWWA = 1; /* We don't use WWA, but if we did use most
                         * recent version of the weather tables. */
   sInt4 f_endMsg = 1;  /* 1 if we read the last grid in a GRIB message */
   LatLon lwlf;         /* ReadGrib2Record allows subgrids.  We want entire
                         * grid, so set the lat to -100. */
   LatLon uprt;         /* ReadGrib2Record allows subgrids.  We want entire
                         * grid, so set the lat to -100. */
   char *msg;           /* Used to print the error stack. */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   double missing = 0;  /* Missing value to use. */
   int i;               /* Counter for the points. */
   double newX, newY;   /* The location of lat/lon on the input grid. */
   sChar f_missing;     /* flag whether the cell fell off the grid. */
   sInt4 x1, y1;        /* The nearest grid point. */
   sInt4 row;           /* The index into grib_Data for a given x,y pair *
                         * using scan-mode = 0100 = GRIB2BIT_2 */
   double ans;          /* The interpolated value at a given point. */
   int retVal;          /* The return value on error. */

   /* Open the grib file. */
   if (filename != NULL) {
      if ((grib_fp = fopen (filename, "rb")) == NULL) {
         fprintf (stderr, "Problems opening %s for read\n", filename);
         return -1;
      }
   } else {
      grib_fp = stdin;
   }
   IS_Init (&is);
   MetaInit (&meta);

   /* Start loop for all messages. */
   grib_DataLen = 0;
   grib_Data = NULL;
   *lenTime = 0;
   *valTime = NULL;
   *data = NULL;
   lwlf.lat = -100;
   uprt.lat = -100;

   while ((c = fgetc (grib_fp)) != EOF) {
      ungetc (c, grib_fp);
      /* Read the GRIB message. */
      if (ReadGrib2Record (grib_fp, f_unit, &grib_Data, &grib_DataLen, &meta,
                           &is, subgNum, majEarth, minEarth, f_SimpleVer, f_SimpleWWA,
                           &f_endMsg, &lwlf, &uprt) != 0) {
         msg = errSprintf (NULL);
         fprintf (stderr, "ERROR: In call to GenericProbe().\n%s", msg);
         free (msg);
         retVal = -3;
         goto error;
      }
      /* Up to caller to validate the range of the data.  Could do it here if 
       * the caller provided f_validRand and validMax, validMin. */
/*
      if (f_validRange > 0) {
         * valid max. *
         if (f_validRange > 1) {
            if (meta->gridAttrib.max > validMax) {
               fprintf (stderr, "ERROR: %f > valid Max of %f\n",
                        meta->gridAttrib.max, validMax);
               retVal = -3;
               goto error;
            }
         }
         * valid min. *
         if (f_validRange % 2) {
            if (meta->gridAttrib.min < validMin) {
               fprintf (stderr, "ERROR: %f < valid Min of %f\n",
                        meta->gridAttrib.min, validMin);
               retVal = -3;
               goto error;
            }
         }
      }
*/
      if (f_endMsg != 1) {
         subgNum++;
      } else {
         subgNum = 0;
      }

      /* Check that gds is valid before setting up map projection. */
      if (GDSValid (&(meta.gds)) != 0) {
         fprintf (stderr, "ERROR: Sect3 was not Valid.\n");
         retVal = -4;
         goto error;
      }
      /* Set up the map projection. */
      SetMapParamGDS (&map, &(meta.gds));

      /* Figure out a missing value, if there isn't one, so that when we
       * interpolate and we are out of bounds, we can return something. */
      if (meta.gridAttrib.f_miss == 0) {
         missing = 9999;
         if (meta.gridAttrib.f_maxmin) {
            if ((missing <= meta.gridAttrib.max) &&
                (missing >= meta.gridAttrib.min)) {
               missing = meta.gridAttrib.max + 1;
            }
         }
      } else {
         missing = meta.gridAttrib.missPri;
      }

      /* Allocate space for the returned data. */
      *lenTime = *lenTime + 1;
      *valTime = (char **) realloc ((void *) *valTime,
                                    *lenTime * sizeof (char *));
      (*valTime)[*lenTime - 1] = (char *) malloc ((strlen (meta.validTime)
                                                   + 1) * sizeof (char));
      strcpy ((*valTime)[*lenTime - 1], meta.validTime);
      *data = (double **) realloc ((void *) *data,
                                   *lenTime * sizeof (double *));
      (*data)[*lenTime - 1] = (double *) malloc (numPnts * sizeof (double));

      for (i = 0; i < numPnts; i++) {
         myCll2xy (&map, lat[i], lon[i], &newX, &newY);
         f_missing = 0;
         /* Find the nearest grid cell. */
         if (newX < .5) {
            x1 = 1;
            f_missing = 1;
         } else if ((newX + .5) > meta.gds.Nx) {
            x1 = meta.gds.Nx;
            f_missing = 1;
         } else {
            x1 = (sInt4) (newX + .5);
         }
         if (newY < .5) {
            y1 = 1;
            f_missing = 1;
         } else if ((newY + .5) > meta.gds.Ny) {
            y1 = meta.gds.Ny;
            f_missing = 1;
         } else {
            y1 = (sInt4) (newY + .5);
         }
         if (!f_interp) {
            /* Get the x1, y1 value. */
            if (!f_missing) {
               XY2ScanIndex (&row, x1, y1, GRIB2BIT_2, meta.gds.Nx,
                             meta.gds.Ny);
               ans = grib_Data[row];
            } else {
               ans = missing;
            }
         } else {
            /* Figure out data value at this lat/lon */
            /* Generic probe... don't perform "AvgInterp" option */
            ans = BiLinearCompute (grib_Data, &map, lat[i], lon[i],
                                   meta.gds.Nx, meta.gds.Ny,
                                   meta.gridAttrib.f_miss, missing,
                                   meta.gridAttrib.missSec, 0);
         }
         if (strcmp (meta.element, "Wx") != 0) {
            (*data)[*lenTime - 1][i] = ans;

         } else {
            /* Handle the weather case. */
            fprintf (stderr, "ERROR: Currently doesn't handle weather "
                     "strings.\n");
            retVal = -1;
            goto error;
         }
      }
/*
      IS_Free (&is);
      IS_Init (&is);
*/
      MetaFree (&meta);
   }
   /* End loop for all messages. */
   free (grib_Data);
   fclose (grib_fp);
   MetaFree (&meta);
   IS_Free (&is);
   return 0;
 error:
   free (grib_Data);
   fclose (grib_fp);
   MetaFree (&meta);
   IS_Free (&is);
   return retVal;
}

#ifdef PROBE_DEBUG
int main (int argc, char **argv)
{
   double lat[5];
   double lon[5];
   int f_interp = 0;
   int lenTime;
   char **valTime;
   double **data;
   int i, j;

   if (argc != 2) {
      printf ("Usage: %s <grib file>\n", argv[0]);
      return -1;
   }

   lat[0] = 30;
   lon[0] = -90;
   lat[1] = 31;
   lon[1] = -91;
   lat[2] = 32;
   lon[2] = -92;
   lat[3] = 33;
   lon[3] = -93;
   lat[4] = 34;
   lon[4] = -94;

   GenericProbe (argv[1], 5, lat, lon, f_interp, &lenTime, &valTime, &data);

   for (i = 0; i < lenTime; i++) {
      printf ("%s\n", valTime[i]);
      for (j = 0; j < 5; j++) {
         printf ("%f,%f=%f ", lat[j], lon[j], data[i][j]);
      }
      printf ("\n");
   }

   for (i = 0; i < lenTime; i++) {
      free (data[i]);
      free (valTime[i]);
   }
   free (data);
   free (valTime);
   return 0;
}
#endif
