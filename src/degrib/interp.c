/*****************************************************************************
 * interp.c
 *
 * DESCRIPTION
 *    This file contains the routines used to interpolate the grid from the
 * NDFD grid to a simple geographic (lat/lon) grid.  It then stores the
 * results to a .flt file, and creates the associated .prj, and .hdr files.
 *    Note: this file takes advantage of write.c for the .prj / .hdr files
 *
 * HISTORY
 *  10/2002 Arthur Taylor (MDL / RSIS): Created.
 *  12/2002 Rici Yu, Fangyu Chi, Mark Armstrong, & Tim Boyer
 *          (RY,FC,MA,&TB): Code Review 2.
 *
 * NOTES
 * 1) Not sure if given a lat/lon grid cmapf would work.
 *****************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mymapf.h"
#include "tendian.h"
#include "write.h"
#include "interp.h"
#include "myerror.h"
#include "scan.h"
#include "myassert.h"

extern double POWERS_ONE[];

/*****************************************************************************
 * BiLinearBorder() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Performs a bi-linear interpolation to the given lat/lon.  It assumes:
 * 1) That the grid is lat/lon.  2) That the interpolated point is outside
 * the normal definition of the grid.  3) That the deltaX is such that 1 cell
 * from the right edge of the grid is on the left edge of the grid.
 *
 * ARGUMENTS
 *   gribData = The grib2 data to write. (Input)
 *        map = Holds the current map projection info to interpolate from.(In)
 * newX, newY = The location of the desired point on the input grid.
 *     Nx, Ny = Dimensions of input grid (Input)
 *     f_miss = How missing values are handled in grib_Data (Input)
 *    missPri = The value to use for missing data. (Input)
 *    missSec = Secondary missing value if there is one. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: double
 *   Interpolated value, or "missPri" if it couldn't compute it.
 *
 * HISTORY
 *   4/2004 Arthur Taylor (MDL/RSIS): Created to solve a border probe problem.
 *
 * NOTES
 *****************************************************************************
 */
double BiLinearBorder (const double *gribData, myMaparam *map, double newX,
                       double newY, sInt4 Nx, sInt4 Ny, uChar f_miss,
                       double missPri, double missSec, sChar f_avgInterp)
{
   sInt4 row;           /* The index into gribData for a given x,y pair using
                         * scan-mode = 0100 = GRIB2BIT_2 */
   sInt4 x1, x2, y1, y2; /* Corners of bounding box lat/lon is in. */
   double d11, d12, d21, d22; /* grib values of bounding box corners. */
   sInt4 numPts = Nx * Ny; /* Total number of points. */
   double d_temp1, d_temp2; /* Temp storage during interpolation. */
   double testLon;      /* Used to test if we should call BiLinearBorder() */
   double sumDist;      /* The total sum of the distances. */
   double dist11 = 0;   /* Distance from point to 11 cell */
   double dist12 = 0;   /* Distance from point to 12 cell */
   double dist21 = 0;   /* Distance from point to 21 cell */
   double dist22 = 0;   /* Distance from point to 22 cell */
   double val;          /* sum of the distance weighted values. */

   myAssert (map->f_latlon);

   /* Check if lonN + Dx = lon1 mod 360. */
   testLon = map->lonN + map->Dx;
   while (testLon < 0) {
      testLon += 360;
   }
   while (testLon >= 360) {
      testLon -= 360;
   }
   if (testLon != map->lon1) {
      return missPri;
   }

   x1 = Nx;             /* lonN (or Nx) side. */
   x2 = 1;              /* lon1 (or 1) side. */
   y1 = (sInt4) (newY);
   y2 = (y1 + 1);

   /* Get the first (1,1) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   /* XY2ScanIndex (&row, x1, y1, GRIB2BIT_2, Nx, Ny); */
   row = (x1 - 1) + (y1 - 1) * Nx;
   /* Following check is probably unnecessary, but just in case. */
   if ((row < numPts) && (row >= 0)) {
      d11 = gribData[row];
      if (f_miss == 2) {
         if (d11 == missSec) {
            d11 = missPri;
         }
      }
   } else {
      return missPri;
   }

   /* Get the second (1,2) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   /* XY2ScanIndex (&row, x1, y2, GRIB2BIT_2, Nx, Ny); */
   row = (x1 - 1) + (y2 - 1) * Nx;
   if ((row < numPts) && (row >= 0)) {
      d12 = gribData[row];
      if (f_miss == 2) {
         if (d12 == missSec) {
            d12 = missPri;
         }
      }
   } else {
      return missPri;
   }

   /* Get the third (2,1) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   /* XY2ScanIndex (&row, x2, y1, GRIB2BIT_2, Nx, Ny); */
   row = (x2 - 1) + (y1 - 1) * Nx;
   if ((row < numPts) && (row >= 0)) {
      d21 = gribData[row];
      if (f_miss == 2) {
         if (d21 == missSec) {
            d21 = missPri;
         }
      }
   } else {
      return missPri;
   }

   /* Get the fourth (2,2) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   /* XY2ScanIndex (&row, x2, y2, GRIB2BIT_2, Nx, Ny); */
   row = (x2 - 1) + (y2 - 1) * Nx;
   if ((row < numPts) && (row >= 0)) {
      d22 = gribData[row];
      if (f_miss == 2) {
         if (d22 == missSec) {
            d22 = missPri;
         }
      }
   } else {
      return missPri;
   }

   /* Do Bi-linear interpolation to get value. */
   if ((d11 != missPri) && (d12 != missPri) &&
       (d21 != missPri) && (d22 != missPri)) {
#ifdef JUNK
      /* Note the use of fabs() and Dx and their implications for the sign. */
      /* Corrected 1/24/2007 due to email from jeff.sharkey */
      /* Was (d11 - d12) and (d21 - d22), but d12 is x1,y2 and d21 is x2,y1.
       * d_temp1 is holding y1 constant so should be dealing with d?1 */
      if (fabs (newX - x1) <= map->Dx) {
         d_temp1 = d11 - fabs (newX - x1) * (d11 - d21) / map->Dx;
         d_temp2 = d12 - fabs (newX - x1) * (d12 - d22) / map->Dx;
         return (float) (d_temp1 + (newY - y1) *
                         (d_temp1 - d_temp2) / (y1 - y2));
      } else {
         d_temp1 = d11 - fabs (newX - x2) * (d11 - d21) / map->Dx;
         d_temp2 = d12 - fabs (newX - x2) * (d12 - d22) / map->Dx;
         return (float) (d_temp1 + (newY - y1) *
                         (d_temp1 - d_temp2) / (y1 - y2));
      }
#endif
      x1 = 0;
      d_temp1 = d11 + (newX - x1) * (d21 - d11) / (x2 - x1);
      d_temp2 = d12 + (newX - x1) * (d22 - d12) / (x2 - x1);
      return (float) (d_temp1 + (newY - y1) * (d_temp2 - d_temp1) / (y2 - y1));

   } else if (f_avgInterp) {
      /* Calculate sum of distances... */
      sumDist = 0;
      x1 = 0;
      if (d11 != missPri) {
         dist11 = sqrt ((x1 - newX) * (x1 - newX) + (y1 - newY) * (y1 - newY));
         sumDist += dist11;
      }
      if (d12 != missPri) {
         dist12 = sqrt ((x1 - newX) * (x1 - newX) + (y2 - newY) * (y2 - newY));
         sumDist += dist12;
      }
      if (d21 != missPri) {
         dist21 = sqrt ((x2 - newX) * (x2 - newX) + (y1 - newY) * (y1 - newY));
         sumDist += dist21;
      }
      if (d22 != missPri) {
         dist22 = sqrt ((x2 - newX) * (x2 - newX) + (y2 - newY) * (y2 - newY));
         sumDist += dist22;
      }

#ifdef JUNK
      if (fabs (newX - x1) <= map->Dx) {
         if (d11 != missPri) {
            dist11 = sqrt ((x1 - newX) * (x1 - newX) + (y1 - newY) * (y1 - newY));
            sumDist += dist11;
         }
         if (d12 != missPri) {
            dist12 = sqrt ((x1 - newX) * (x1 - newX) + (y2 - newY) * (y2 - newY));
            sumDist += dist12;
         }
         if (d21 != missPri) {
            dist21 = sqrt ((map->Dx - (newX - x1)) * (map->Dx - (newX - x1)) +
                           (y1 - newY) * (y1 - newY));
            sumDist += dist21;
         }
         if (d22 != missPri) {
            dist22 = sqrt ((map->Dx - (newX - x1)) * (map->Dx - (newX - x1)) +
                           (y2 - newY) * (y2 - newY));
            sumDist += dist22;
         }
      } else {
         if (d11 != missPri) {
            dist11 = sqrt ((map->Dx - (newX - x2)) * (map->Dx - (newX - x2)) +
                           (y1 - newY) * (y1 - newY));
            sumDist += dist11;
         }
         if (d12 != missPri) {
            dist12 = sqrt ((map->Dx - (newX - x2)) * (map->Dx - (newX - x2)) +
                           (y2 - newY) * (y2 - newY));
            sumDist += dist12;
         }
         if (d21 != missPri) {
            dist21 = sqrt ((x2 - newX) * (x2 - newX) + (y1 - newY) * (y1 - newY));
            sumDist += dist21;
         }
         if (d22 != missPri) {
            dist22 = sqrt ((x2 - newX) * (x2 - newX) + (y2 - newY) * (y2 - newY));
            sumDist += dist22;
         }
      }
#endif
      if (sumDist == 0) {
         return missPri;
      }
      /* Calculate inverse distance weighted value */
      val = 0;
      if (d11 != missPri) {
         val += (dist11 / sumDist) * d11;
      }
      if (d12 != missPri) {
         val += (dist12 / sumDist) * d12;
      }
      if (d21 != missPri) {
         val += (dist21 / sumDist) * d21;
      }
      if (d22 != missPri) {
         val += (dist22 / sumDist) * d22;
      }
      return (float) (val);
   } else {
      return missPri;
   }
}

/*****************************************************************************
 * BiLinearCompute() -- Review 12/2002
 *
 *  -- Depricating in favor of "getValAtPnt", since that doesn't cause an
 *     extra call to: "myCll2xy (map, lat, lon, &newX, &newY);" in the case
 *     when you are dealing with f_pntType == 1 (grid cells).
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Performs a bi-linear interpolation to the given lat/lon.  If it can find
 * the four surrounding corners, uses them to interpolate, otherwise it
 * returns the missPri value.
 *
 * ARGUMENTS
 *  grib_Data = The grib2 data to write. (Input)
 *        map = Holds the current map projection info to interpolate from.(In)
 *   lat, lon = The point we are interested in. (Input)
 *    missPri = The value to use for missing data. (Input)
 *     Nx, Ny = Dimensions of input grid (Input)
 * f_miss = How missing values are handled in grib_Data (Input)
 *    missSec = Secondary missing value if there is one. (Input)
 * f_avgInterp = 1 if some of corners are missing, we should dist weight
 *               average the values, 0 return missing. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: double
 *   Interpolated value, or "missPri" if it couldn't compute it.
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (RY,FC,MA,&TB): Code Review.
 *   4/2004 AAT: Added call to BiLinearBorder()
 *  10/2007 AAT: Added f_avgInterp option. 
 *
 * NOTES
 *    Could speed this up a bit, since we know scan is GRIB2BIT_2
 *    (Note: map usually has Grid defined for 1..Nx and 1..Ny)
 *****************************************************************************
 */
double BiLinearCompute (double *grib_Data, myMaparam *map, double lat,
                        double lon, sInt4 Nx, sInt4 Ny, uChar f_miss,
                        double missPri, double missSec, sChar f_avgInterp)
{
   double newX, newY;   /* The location of lat/lon on the input grid. */
   sInt4 row;           /* The index into grib_Data for a given x,y pair
                         * using scan-mode = 0100 = GRIB2BIT_2 */
   sInt4 x1, x2, y1, y2; /* Corners of bounding box lat/lon is in. */
   double d11, d12, d21, d22; /* grib values of bounding box corners. */
   sInt4 numPts = Nx * Ny; /* Total number of points. */
   double d_temp1, d_temp2; /* Temp storage during interpolation. */
   double sumDist;      /* The total sum of the distances. */
   double dist11 = 0;   /* Distance from point to 11 cell */
   double dist12 = 0;   /* Distance from point to 12 cell */
   double dist21 = 0;   /* Distance from point to 21 cell */
   double dist22 = 0;   /* Distance from point to 22 cell */
   double val;          /* sum of the distance weighted values. */

   myCll2xy (map, lat, lon, &newX, &newY);
   if ((newX < 1) || (newX > Nx) || (newY < 1) || (newY > Ny)) {
      if (map->f_latlon) {
         /* Find out if we can do a border interpolation. */
         return BiLinearBorder (grib_Data, map, newX, newY, Nx, Ny, f_miss,
                                missPri, missSec, f_avgInterp);
      }
      return missPri;
   }
   x1 = (sInt4) (newX);
   x2 = (x1 + 1);
   y1 = (sInt4) (newY);
   y2 = (y1 + 1);

   /* Get the first (1,1) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   /* XY2ScanIndex (&row, x1, y1, GRIB2BIT_2, Nx, Ny); */
   row = (x1 - 1) + (y1 - 1) * Nx;
   /* Following check is probably unnecessary, but just in case. */
   if ((row < numPts) && (row >= 0)) {
      d11 = grib_Data[row];
      if (f_miss == 2) {
         if (d11 == missSec) {
            d11 = missPri;
         }
      }
   } else {
      return missPri;
   }

   /* Get the second (1,2) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   /* XY2ScanIndex (&row, x1, y2, GRIB2BIT_2, Nx, Ny); */
   row = (x1 - 1) + (y2 - 1) * Nx;
   if ((row < numPts) && (row >= 0)) {
      d12 = grib_Data[row];
      if (f_miss == 2) {
         if (d12 == missSec) {
            d12 = missPri;
         }
      }
   } else {
      return missPri;
   }

   /* Get the third (2,1) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   /* XY2ScanIndex (&row, x2, y1, GRIB2BIT_2, Nx, Ny); */
   row = (x2 - 1) + (y1 - 1) * Nx;
   if ((row < numPts) && (row >= 0)) {
      d21 = grib_Data[row];
      if (f_miss == 2) {
         if (d21 == missSec) {
            d21 = missPri;
         }
      }
   } else {
      return missPri;
   }

   /* Get the fourth (2,2) corner value. */
   /* Assumes memory is in scan mode 64 (see XY2ScanIndex(GRIB2BIT_2)) */
   /* XY2ScanIndex (&row, x2, y2, GRIB2BIT_2, Nx, Ny); */
   row = (x2 - 1) + (y2 - 1) * Nx;
   if ((row < numPts) && (row >= 0)) {
      d22 = grib_Data[row];
      if (f_miss == 2) {
         if (d22 == missSec) {
            d22 = missPri;
         }
      }
   } else {
      return missPri;
   }

   /* Do Bi-linear interpolation to get value. */
   if ((d11 != missPri) && (d12 != missPri) &&
       (d21 != missPri) && (d22 != missPri)) {
      /* Corrected 1/24/2007 due to email from jeff.sharkey */
      /* Was (d11 - d12) and (d21 - d22), but d12 is x1,y2 and d21 is x2,y1.
       * d_temp1 is holding y1 constant so should be dealing with d?1 */
/*
      d_temp1 = d11 + (newX - x1) * (d11 - d21) / (x1 - x2);
      d_temp2 = d12 + (newX - x1) * (d12 - d22) / (x1 - x2);
      return (float) (d_temp1 + (newY - y1) *
                      (d_temp1 - d_temp2) / (y1 - y2));
*/
      d_temp1 = d11 + (newX - x1) * (d21 - d11) / (x2 - x1);
      d_temp2 = d12 + (newX - x1) * (d22 - d12) / (x2 - x1);
      return (float) (d_temp1 + (newY - y1) * (d_temp2 - d_temp1) / (y2 - y1));
   } else if (f_avgInterp) {
      /* Calculate sum of distances... */
      sumDist = 0;
      if (d11 != missPri) {
         dist11 = sqrt ((x1 - newX) * (x1 - newX) + (y1 - newY) * (y1 - newY));
         sumDist += dist11;
      }
      if (d12 != missPri) {
         dist12 = sqrt ((x1 - newX) * (x1 - newX) + (y2 - newY) * (y2 - newY));
         sumDist += dist12;
      }
      if (d21 != missPri) {
         dist21 = sqrt ((x2 - newX) * (x2 - newX) + (y1 - newY) * (y1 - newY));
         sumDist += dist21;
      }
      if (d22 != missPri) {
         dist22 = sqrt ((x2 - newX) * (x2 - newX) + (y2 - newY) * (y2 - newY));
         sumDist += dist22;
      }
      if (sumDist == 0) {
         return missPri;
      }
      /* Calculate inverse distance weighted value */
      val = 0;
      if (d11 != missPri) {
         val += (dist11 / sumDist) * d11;
      }
      if (d12 != missPri) {
         val += (dist12 / sumDist) * d12;
      }
      if (d21 != missPri) {
         val += (dist21 / sumDist) * d21;
      }
      if (d22 != missPri) {
         val += (dist22 / sumDist) * d22;
      }
      return (float) (val);
   } else {
      return missPri;
   }
}
