#include <string.h>
#include <stdlib.h>
#include "write.h"
#include "userparse.h"
#include "mymapf.h"
#include "myerror.h"
#include "scan.h"
#include "myutil.h"

/* get f_nMissing to work.  */
/* write up the various options. */
/* fixed widths? */
int gribWriteCsv (FILE *out_fp, double *grib_Data, grib_MetaData *meta,
                  sChar decimal, char *separator, char *logName,
                  sChar f_WxParse, sChar f_NoMissing, sChar LatLon_Decimal)
{
   uInt4 i, j;          /* Loop over the grid cells. */
   int k;               /* Used when outputing the weather keys. */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   double missing;      /* Missing value to use. */
   char format[20];     /* Format to print the data with. */
   double lat, lon;     /* The lat/lon at the grid cell. */
   sInt4 row;           /* The index into grib_Data for a given x,y pair *
                         * using scan-mode = 0100 = GRIB2BIT_2 */
   double ans;          /* The interpolated value at a given point. */
   FILE *logFp;         /* Used to log errors in Wx keys. */
   char buffer[100];
   sChar f_IsMissing;

   /* Check that gds is valid before setting up map projection. */
   if (GDSValid (&meta->gds) != 0) {
      preErrSprintf ("ERROR: Sect3 was not Valid.\n");
      free (grib_Data);
      return -4;
   }
   /* Set up the map projection. */
   SetMapParamGDS (&map, &(meta->gds));

   /* Figure out a missing value, if there isn't one, so that when we
    * interpolate and we are out of bounds, we can return something. */
   if (meta->gridAttrib.f_miss == 0) {
      missing = 9999;
      if (meta->gridAttrib.f_maxmin) {
         if ((missing <= meta->gridAttrib.max) &&
             (missing >= meta->gridAttrib.min)) {
            missing = meta->gridAttrib.max + 1;
         }
      }
   } else {
      missing = meta->gridAttrib.missPri;
   }

   /* Print out the header. */
   sprintf (buffer, "%s_%s", meta->element, meta->validTime);
   fprintf (out_fp, "   X%s   Y%s   Latitude%s  Longitude%s%12s\n",
            separator, separator, separator, separator, buffer);
   sprintf (format, "%%12.%df", decimal);
   for (j = 1; j <= meta->gds.Ny; j++) {
      for (i = 1; i <= meta->gds.Nx; i++) {
         /* No point concering ourselves with usr->f_interp, since the
          * bilinear value at a grid cell latice should be the same as the
          * nearest point which is the value at that grid cell. */

         myCxy2ll (&map, i, j, &lat, &lon);
         lat = myRound (lat, LatLon_Decimal);
         lon = myRound (lon, LatLon_Decimal);

         /* Get the i, j value. */
         XY2ScanIndex (&row, i, j, GRIB2BIT_2, meta->gds.Nx, meta->gds.Ny);
         ans = grib_Data[row];

         f_IsMissing = 0;
         if (meta->gridAttrib.f_miss == 1) {
            if (ans == meta->gridAttrib.missPri) {
               f_IsMissing = 1;
            }
         } else if (meta->gridAttrib.f_miss == 2) {
            if ((ans == meta->gridAttrib.missSec) ||
                (ans == meta->gridAttrib.missPri)) {
               f_IsMissing = 1;
            }
         }

         if (f_IsMissing && f_NoMissing) {
         } else {
            /* Print the first part of the line. */
            fprintf (out_fp, "%4ld%s%4ld%s%11.6f%s%11.6f%s", i, separator, j,
                     separator, lat, separator, lon, separator);

            if (strcmp (meta->element, "Wx") != 0) {
               /* Handle the case when it is not weather first. */
               fprintf (out_fp, format, myRound (ans, decimal));
            } else {
               /* Now handle the weather case. */
               row = (sInt4) ans;
               if ((row >= 0) && (row < (sInt4) meta->pds2.sect2.wx.dataLen)) {

                  if ((meta->pds2.sect2.wx.ugly[row].errors != NULL) &&
                      (logName != NULL)) {
                     if ((logFp = fopen (logName, "at")) != NULL) {
                        fprintf (logFp, "%ld%s%ld%s%f%s%f%s", i, separator, j,
                                 separator, lat, separator, lon, separator);
                        fprintf (logFp, "%s\n",
                                 meta->pds2.sect2.wx.ugly[row].errors);
                        fclose (logFp);
                     }
                  }
                  if (f_WxParse == 0) {
                     fprintf (out_fp, "%s", meta->pds2.sect2.wx.data[row]);
                  } else if (f_WxParse == 1) {
                     for (k = 0; k < NUM_UGLY_WORD; k++) {
                        if (meta->pds2.sect2.wx.ugly[row].english[k] != NULL) {
                           if (k != 0) {
                              fprintf (out_fp, " and ");
                           }
                           fprintf (out_fp, "%s",
                                    meta->pds2.sect2.wx.ugly[row].english[k]);
                        } else {
                           if (k == 0) {
                              fprintf (out_fp, "No Weather");
                           }
                           break;
                        }
                     }
                  } else if (f_WxParse == 2) {
                     fprintf (out_fp, "%d",
                              meta->pds2.sect2.wx.ugly[row].SimpleCode);
                  }
               } else {
                  fprintf (out_fp, "%ld", row);
               }
            }
            fprintf (out_fp, "\n");
         }
      }
   }

   return 0;
}
