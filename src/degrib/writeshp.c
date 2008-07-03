/*****************************************************************************
 * writeshp.c
 *
 * DESCRIPTION
 *    This file contains all the routines used to write the grid out to
 * .shp format (which can be used via ArcGIS or ArcExplorer)
 * Associated with the .shp file are a .dbf, and .shx file.

 * .flt format (which can be used via ArcGIS S.A. or by GrADS.
 * Associated with the .flt file are a .prj, .hdr, and .ave files.
 * Also calls gribWriteGradsCTL, to create a .ctl file.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL / RSIS): Created.
 *  12/2002 Rici Yu, Fangyu Chi, Mark Armstrong, & Tim Boyer
 *          (RY,FC,MA,&TB): Code Review 2.
 *   6/2003 AAT: Split write.c into 2 pieces (writeflt and writeshp).
 *
 * NOTES
 * 1) As far as I can tell .flt and .shp files don't allow for two missing
 *    values, so I use missPri for both, if need be.
 * 2) Look up "address" in dbf definition.
 *****************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mymapf.h"
#include "tendian.h"
#include "write.h"
#include "myerror.h"
#include "myutil.h"
#include "myassert.h"
#include <errno.h>
#include "type.h"
#include "weather.h"
#include "chain.h"
#include <math.h>

extern double POWERS_ONE[];

/*****************************************************************************
 * checkFileSize() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This attempts to find out if a file is the expected size.
 *
 * ARGUMENTS
 * filename = Name of file to check the size of. (Input)
 *      len = Expected length. (Input)
 *
 * FILES/DATABASES:
 * Looks at the length of a given file.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems opening the files.
 * -2 = File is either wrong size or errors occured while checking the size.
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (RY,FC,MA,&TB): Code Review.
 *   5/2003 AAT: Removed reliance on errno (since Tcl/Tk confuses the issue).
 *
 * NOTES
 *****************************************************************************
 */
static int checkFileSize (char *filename, sInt4 len)
{
   FILE *fp;
   if ((fp = fopen (filename, "rb")) == NULL) {
      errSprintf ("ERROR: After write, Problems opening %s for file length "
                  "check.", filename);
      return -1;
   }
   fseek (fp, 0L, SEEK_END);
   if (ftell (fp) != len) {
      errSprintf ("ERROR: file %s is %ld not %ld bytes long.\n",
                  filename, ftell (fp), len);
      fclose (fp);
      return -2;
   }
   fclose (fp);
   return 0;
}

/*****************************************************************************
 * CreatePrj() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This creates a .prj file.  The .prj file contains the information that
 * ArcGIS 8 needs to understand the map projection of the .shp files.  Since I
 * have already projected the .shp files to a geographic coordinate system
 * (From a lambert conformal projection), the only thing that I think ESRI
 * needs to know is the assumed shape of the earth.
 *   This could eventually replace the .ave file, but as long as ArcView 3.*
 * is out, I'm going to create both.
 *
 * ARGUMENTS
 * filename = Name of file to save to. (Input/Output)
 *      gds = Grid Definition (from the parsed GRIB message) to write. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems opening the files.
 * -2 = Problems writing entire .shp file
 * -3 = Problems writing entire .shx file
 *
 * HISTORY
 *   1/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int CreatePrj (char *filename, gdsType *gds)
{
   FILE *fp;

   strncpy (filename + strlen (filename) - 3, "prj", 3);
   if ((fp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }
   if (gds->f_sphere) {
      if (gds->majEarth == 6371.2) {
         fprintf (fp, "GEOGCS[\"NCEP_SPHERE\",DATUM[\"NCEP_SPHERE\",SPHEROID"
                  "[\"NCEP_SPHERE\",6371200.0,0.0]],PRIMEM[\"Greenwich\",0.0]"
                  ",UNIT[\"Degree\",0.0174532925199432955]]\n");
      } else {
         fprintf (fp, "GEOGCS[\"NCEP_SPHERE\",DATUM[\"NCEP_SPHERE\",SPHEROID"
                  "[\"NCEP_SPHERE\",%f,0.0]],PRIMEM[\"Greenwich\",0.0],UNIT["
                  "\"Degree\",0.0174532925199432955]]\n", gds->majEarth);
      }
   } else {
      fprintf (fp, "GEOGCS[\"NCEP_SPHERE\",DATUM[\"NCEP_SPHERE\",SPHEROID"
               "[\"NCEP_SPHERE\",%f,%f]],PRIMEM[\"Greenwich\",0.0],UNIT["
               "\"Degree\",0.0174532925199432955]]\n", gds->majEarth,
               gds->minEarth);
   }
   fclose (fp);
   return 0;
}

/*****************************************************************************
 * CreateShpPoly() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This creates a .shp / .shx file.  The .shp / .shx file contains the
 * lat/lon values of the grid as polygons instead of as points.  These formats
 * are specific to Esri ArcView.
 *   The points are assumed to be the corner points of the grid, so there
 * should be 1 more row/column in the dp than given to the shpPnt command,
 * but Nx and Ny are the same as with shpPnt.
 *
 * ARGUMENTS
 *   filename = Name of file to save to. (Output)
 *         dp = Array of lat/lon pairs for the grid cells. (Input)
 *              dp is expected to have 1 extra set of x than Nx
 *              and 1 extra set of y than Ny.
 *         Nx = Number of x values in the grid. (Input)
 *         Ny = Number of y values in the grid. (Input)
 * f_nMissing = True if we should NOT store missing. (Input)
 *  grib_Data = Actual data so we can determine where missing values are. (In)
 *     attrib = Tells what type of missing values we used. (Input)
 *
 * FILES/DATABASES:
 *   Creates a .shp file, which is a binary file (Mixed Endian) consisting of
 *      a set of lat/lon polygon shapes.
 *   Also, creates a .shx file, which is a binary file (Mixed Endian) which is
 *      used as an index into the .shp file.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems opening the files.
 * -2 = Problems writing entire .shp file
 * -3 = Problems writing entire .shx file
 *
 * HISTORY
 *   4/2003 Arthur Taylor (MDL/RSIS): Created.
 *   5/2003 AAT: Modified to allow for f_nMissing.
 *   5/2003 AAT: Fixed orientation to polygon issue.
 *   5/2003 AAT: For poly spanning date line, allow outside of -180..180
 *   5/2003 AAT: Decided to have 1,1 be lower left corner in .shp files.
 *   5/2003 AAT: Removed reliance on errno (since Tcl/Tk confuses the issue).
 *   7/2003 AAT: 1,1 lower left affected orientation of polygons.
 *   3/2004 AAT: Updated to handle Alaska polygons.
 *
 * NOTES
 * 1) Doesn't inter-twine the write to files, as that could cause the hard
 *   drive to slow down.
 * 2) Assumes data goes from left to right before going up and down.
 *****************************************************************************
 */
static int CreateShpPoly (char *filename, LatLon *dp, int Nx, int Ny,
                          sChar f_nMissing, double *grib_Data,
                          gridAttribType *attrib, sChar LatLon_Decimal)
{
   FILE *sfp;           /* The open file pointer for .shp */
   FILE *xfp;           /* The open file pointer for .shx. */
   int i;               /* A counter used for the shx values. */
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   double Bounds[] = {
      0., 0., 0., 0., 0., 0., 0., 0.
   };                   /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */
   sInt4 dataType = 5;  /* Polygon shp type. */
   sInt4 curRec[2];     /* rec number, and content length. */
   LatLon *curDp;       /* current data point. */
   double *curData;     /* Current Grid cell data point (for f_nMissing) */
   int err;             /* Internal err number. */
   int recLen;          /* Length in bytes of a record in the .shp file. */
   int recLen2;         /* Length in bytes of a 2 poly record. */
   sInt4 dpLen;         /* Number of data polygons in output file. */
   sInt4 numRec;        /* The total number of records actually stored. */
   int x, y;            /* Counters used for traversing the grid. */
   double pts[10];      /* Holds the lon/lat points for the cur polygon. */
   double pts1[20];     /* The points for cur poly if split on dateline. */
   double pts2[20];     /* The points for cur poly if split on dateline. */
   int k;               /* Used for traversing the polygon record. */
   double *cur;         /* Used for traversing the polygon record. */
   double PolyBound[4]; /* Used for holding the polygon bounds. */
   sInt4 PolygonSpecs[] = {
      1, 5, 0
   };                   /* NumParts, NumPnts, Index of Ring are constant for
                         * This type of .shp polygon. */
   sChar *f_dateline;   /* array of flags if the poly crosses the dateline. */
   int indexLf = 0;     /* Where to add data to the left polygon chain. */
   int indexRt = 0;     /* Where to add data to the right polygon chain. */
   sChar f_left;        /* Flag to add to the left or right polygon */
   double delt;         /* change in lon of a given line segment. */
   double newLat;       /* latitude where line segment crosses the dateline */
   sInt4 secChain;      /* first index to the second chain. */
   int dateLineCnt;     /* A count of number of polys that cross dateline. */

   strncpy (filename + strlen (filename) - 3, "shp", 3);
   if ((sfp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }
   dpLen = Nx * Ny;
   /* Perform a simple file size check.  File has to be less than
    * 4,294,967,294 bytes. (2^31-1) * 2.  Reasoning: file size in 2 byte words
    * is stored as a sInt4 */
   if (!f_nMissing) {
      /* If include all cells (assuming no dateline issues), the size of the
       * .shp file is: 100 + dpLen * (8 +4 +4*8 +3*4 +10*8= 180bytes)
       * So dpLen <= (2^31-1) * 2 - 100 / 180 = 23,860,928.8 */
      if (dpLen > 23860928L) {
         errSprintf ("Trying to create a small poly shp file with %d cells.\n"
                     "This is > small polygon maximum of 23,860,928\n",
                     dpLen);
         return -1;
      }
   }

   /* Start Writing header in first 100 bytes. */
   Head1[0] = 9994;     /* ArcView identifier. */
   memset ((Head1 + 1), 0, 5 * sizeof (sInt4)); /* set 5 unused to 0 */
   recLen = sizeof (sInt4) + 4 * sizeof (double) + 3 * sizeof (sInt4) +
         10 * sizeof (double);
   recLen2 = sizeof (sInt4) + 4 * sizeof (double) +
         2 * sizeof (sInt4) + 2 * sizeof (sInt4) + 10 * sizeof (double) +
         10 * sizeof (double);
   /* .shp file size (in 2 byte words). */
   Head1[6] = (100 + (2 * sizeof (sInt4) + recLen) * dpLen) / 2;
   FWRITE_BIG (Head1, sizeof (sInt4), 7, sfp);
   Head2[0] = 1000;     /* ArcView version identifier. */
   Head2[1] = dataType; /* Signal that these are polygon data. */
   FWRITE_LIT (Head2, sizeof (sInt4), 2, sfp);
   /* Write out initial guess for bounds... Need to revisit. */
   FWRITE_LIT (Bounds, sizeof (double), 8, sfp);

   /* Start Writing data. */
   curRec[1] = recLen / 2; /* Content length in (2 byte words) */
   curRec[0] = 1;
   dateLineCnt = 0;
   f_dateline = (sChar *) malloc ((Nx * Ny) * sizeof (sChar));

   /* If the simple case where we don't have to worry about removing the
    * missing values.  */
   for (y = 0; y < Ny; y++) {
      curData = grib_Data + y * Nx;
      curDp = dp + y * (Nx + 1);
      for (x = 0; x < Nx; x++) {
         if ((!f_nMissing) || (attrib->f_miss == 0) ||
             ((*curData != attrib->missPri) &&
              ((attrib->f_miss != 2) || (*curData != attrib->missSec)))) {
            /* Get the current polygon. */
            cur = pts;
            /* Order matters here.  Must be clockwise !!! */
            *(cur++) = curDp->lon;
            *(cur++) = curDp->lat;
            /* Switched again on 7/14/2003 because of lower left adjustment
             * in 5/2003. */
            *(cur++) = curDp[Nx + 1].lon; /* 1 row up. */
            *(cur++) = curDp[Nx + 1].lat; /* 1 row up. */
            *(cur++) = curDp[Nx + 2].lon; /* 1 row up + 1 accross. */
            *(cur++) = curDp[Nx + 2].lat; /* 1 row up + 1 accross. */
            *(cur++) = curDp[1].lon;
            *(cur++) = curDp[1].lat;
            *(cur++) = curDp->lon;
            *cur = curDp->lat;

            /* Compute the bounds of this polygon. */
            cur = pts;
            PolyBound[0] = *cur; /* min x */
            PolyBound[2] = *cur; /* max x */
            cur++;
            PolyBound[1] = *cur; /* min y */
            PolyBound[3] = *cur; /* max y */
            cur++;
            for (k = 1; k <= 3; k++) {
               if (PolyBound[0] > *cur)
                  PolyBound[0] = *cur;
               else if (PolyBound[2] < *cur)
                  PolyBound[2] = *cur;
               cur++;
               if (PolyBound[1] > *cur)
                  PolyBound[1] = *cur;
               else if (PolyBound[3] < *cur)
                  PolyBound[3] = *cur;
               cur++;
            }
            f_dateline[curRec[0] - 1] = 0;
            if ((PolyBound[2] - PolyBound[0]) > 180) {
               f_dateline[curRec[0] - 1] = 1;
               dateLineCnt++;
               PolyBound[2] = 180;
               PolyBound[0] = -180;
               f_left = 1;
               indexLf = 0;
               indexRt = 0;
               pts1[indexLf++] = pts[0];
               pts1[indexLf++] = pts[1];
               for (k = 1; k <= 4; k++) {
                  delt = pts[k * 2] - pts[(k - 1) * 2];
                  if ((delt > 180) || (delt < -180)) {
                     DateLineLat (pts[k * 2], pts[k * 2 + 1],
                                  pts[(k - 1) * 2], pts[(k - 1) * 2 + 1],
                                  &newLat);
                     newLat = myRound (newLat, LatLon_Decimal);
                     if (f_left) {
                        if (pts1[0] < 0) {
                           pts1[indexLf++] = -180;
                           pts2[indexRt++] = 180;
                        } else {
                           pts1[indexLf++] = 180;
                           pts2[indexRt++] = -180;
                        }
                        pts1[indexLf++] = newLat;
                        pts2[indexRt++] = newLat;
                        pts2[indexRt++] = pts[k * 2];
                        pts2[indexRt++] = pts[k * 2 + 1];
                        f_left = 0;
                     } else {
                        if (pts1[0] < 0) {
                           pts1[indexLf++] = -180;
                           pts2[indexRt++] = 180;
                        } else {
                           pts1[indexLf++] = 180;
                           pts2[indexRt++] = -180;
                        }
                        pts2[indexRt++] = newLat;
                        pts1[indexLf++] = newLat;
                        pts1[indexLf++] = pts[k * 2];
                        pts1[indexLf++] = pts[k * 2 + 1];
                        f_left = 1;
                     }
                  } else {
                     if (f_left) {
                        pts1[indexLf++] = pts[k * 2];
                        pts1[indexLf++] = pts[k * 2 + 1];
                     } else {
                        pts2[indexRt++] = pts[k * 2];
                        pts2[indexRt++] = pts[k * 2 + 1];
                     }
                  }
               }
               pts2[indexRt++] = pts2[0];
               pts2[indexRt++] = pts2[1];
#ifdef DEBUG
/*
               for (k = 0; k < indexLf; k++) {
                  printf ("%f,", pts1[k]);
               }
               printf ("\n");
               for (k = 0; k < indexRt; k++) {
                  printf ("%f,", pts2[k]);
               }
               printf ("\n");
*/
#endif
            }

            /* Update Bounds of all data. */
            if (curRec[0] == 1) {
               Bounds[0] = PolyBound[0];
               Bounds[1] = PolyBound[1];
               Bounds[2] = PolyBound[2];
               Bounds[3] = PolyBound[3];
            } else {
               if (Bounds[0] > PolyBound[0])
                  Bounds[0] = PolyBound[0];
               if (Bounds[1] > PolyBound[1])
                  Bounds[1] = PolyBound[1];
               if (Bounds[2] < PolyBound[2])
                  Bounds[2] = PolyBound[2];
               if (Bounds[3] < PolyBound[3])
                  Bounds[3] = PolyBound[3];
            }
            if (f_dateline[curRec[0] - 1]) {
               /* Write record header. */
               curRec[1] = recLen2 / 2; /* Content length in (2 byte words) */
               FWRITE_BIG (curRec, sizeof (sInt4), 2, sfp);
               curRec[1] = recLen / 2; /* Content length in (2 byte words) */
               /* Write the data type. */
               FWRITE_LIT (&dataType, sizeof (sInt4), 1, sfp);
               /* Write polygons bounds */
               FWRITE_LIT (PolyBound, sizeof (double), 4, sfp);
               /* Write out the Polygon Specs. */
               PolygonSpecs[0] = 2;
               PolygonSpecs[1] = 10;
               FWRITE_LIT (PolygonSpecs, sizeof (sInt4), 3, sfp);
               PolygonSpecs[0] = 1;
               PolygonSpecs[1] = 5;
               /* Write out index of second chain. */
               secChain = indexLf / 2;
               FWRITE_LIT (&secChain, sizeof (sInt4), 1, sfp);
               /* Points ... 10 of them indexLf + indexRt = 20 (10 points) */
               FWRITE_LIT (pts1, sizeof (double), indexLf, sfp);
               FWRITE_LIT (pts2, sizeof (double), indexRt, sfp);
            } else {
               /* Write record header. */
               FWRITE_BIG (curRec, sizeof (sInt4), 2, sfp);
               /* Write the data type. */
               FWRITE_LIT (&dataType, sizeof (sInt4), 1, sfp);
               /* Write polygons bounds */
               FWRITE_LIT (PolyBound, sizeof (double), 4, sfp);
               /* Write out the Polygon Specs. */
               FWRITE_LIT (PolygonSpecs, sizeof (sInt4), 3, sfp);
               /* Points ... 5 of them */
               FWRITE_LIT (pts, sizeof (double), 10, sfp);
            }
            curRec[0]++;
            /* Assuming no dateline issues, the size of the .shp file is:
             * 100 + dpLen * (8 +4 +4*8 +3*4 +10*8= 180bytes)
             * So the max number of records allowed is <=
             * (2^31-1) * 2 - 100 / 180 = 23,860,928.8 */
            if (curRec[0] > 23860928) {
               errSprintf ("Trying to create a small poly shp file with %d cells.\n"
                           "This is > small polygon maximum of 23,860,928\n",
                           curRec[0]);
               return -1;
            }
         }
         curData++;
         curDp++;
      }
   }
   numRec = curRec[0] - 1;

   /* The dateline polys are bigger than the normal poly's by: 10 * sizeof
    * (double) + 1 * sizeof (sInt4); */

   /* Store the updated file length. */
   /* .shp file size (in 2 byte words). */
   Head1[6] = (100 + (2 * sizeof (sInt4) + recLen) * numRec) / 2;

   /* The dateline polys are bigger than the normal poly's by: 10 * sizeof
    * (double) + 1 * sizeof (sInt4); so we have to add that. */
   Head1[6] += (recLen2 - recLen) * dateLineCnt / 2;

   fseek (sfp, 24, SEEK_SET);
   FWRITE_BIG (&(Head1[6]), sizeof (sInt4), 1, sfp);

   /* Store the updated Bounds. */
   fseek (sfp, 36, SEEK_SET);
   /* FWRITE use 4 since we are only updating 4 bounds (not 8). */
   FWRITE_LIT (Bounds, sizeof (double), 4, sfp);
   fflush (sfp);
   fclose (sfp);

   /* Check that .shp is now the correct file size. */
   if ((err = checkFileSize (filename, Head1[6] * 2)) != 0) {
      free (f_dateline);
      return err;
   }

   filename[strlen (filename) - 1] = 'x';
   if ((xfp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      free (f_dateline);
      return -1;
   }

   /* Write ArcView header. */
   Head1[6] = (100 + 8 * numRec) / 2; /* shx file size (in words). */
   FWRITE_BIG (Head1, sizeof (sInt4), 7, xfp);
   FWRITE_LIT (Head2, sizeof (sInt4), 2, xfp);
   FWRITE_LIT (Bounds, sizeof (double), 8, xfp);

   curRec[0] = 50;      /* 100 bytes / 2 = 50 words */
   for (i = 0; i < numRec; i++) {
      if (f_dateline[i]) {
         curRec[1] = recLen2 / 2; /* Content length in words (2 bytes) */
      } else {
         curRec[1] = recLen / 2; /* Content length in words (2 bytes) */
      }
      FWRITE_BIG (curRec, sizeof (sInt4), 2, xfp);
      if (f_dateline[i]) {
         /* X / 2 because of (2 byte words) */
         curRec[0] += (recLen2 + 2 * sizeof (sInt4)) / 2;
      } else {
         /* X / 2 because of (2 byte words) */
         curRec[0] += (recLen + 2 * sizeof (sInt4)) / 2;
      }
   }
   fflush (xfp);
   fclose (xfp);

   /* Check that .shx is now the correct file size. */
   free (f_dateline);
   return checkFileSize (filename, 100 + 8 * numRec);
}

/*****************************************************************************
 * CreateShpPnt() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This creates a .shp / .shx file.  The .shp / .shx file contains the
 * lat/lon values of the grid as points instead of as polygons.  These formats
 * are specific to Esri ArcView.
 *
 * ARGUMENTS
 *       filename = Name of file to save to. (Output)
 *            map = Holds the current map projection info to compute from. (In)
 *            gds = Grid Definiton from the parsed GRIB msg to write. (Input)
 * LatLon_Decimal = Number of decimals to round lat/lon's to. (Input)
 *     f_nMissing = True if we should NOT store missing. (Input)
 *      grib_Data = Actual data to determine where missing values are. (In)
 *         attrib = Tells what type of missing values we used. (Input)
 *
 * FILES/DATABASES:
 *   Creates a .shp file, which is a binary file (Mixed Endian) consisting of
 *      a set of lat/lon point shapes.
 *   Also, creates a .shx file, which is a binary file (Mixed Endian) which is
 *      used as an index into the .shp file.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems opening the files.
 * -2 = Problems writing entire .shp file
 * -3 = Problems writing entire .shx file
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *  11/2002 AAT: Noticed on HP that it didn't write entire .shp file
 *              (due to nfs mount timeouts).. Added a number of error checks.
 *  12/2002 (RY,FC,MA,&TB): Code Review.
 *   5/2003 AAT: Modified to allow us to not store missing values.
 *   5/2003 AAT: Decided to have 1,1 be lower left corner in .shp files.
 *   5/2003 AAT: Removed reliance on errno (since Tcl/Tk confuses the issue).
 *   8/2007 AAT: Modified to internally compute dp as needed, so we don't
 *               have to store massive quantities of lat/lons.
 *
 * NOTES
 * Doesn't inter-twine the write to files, as that could cause the hard
 *   drive to slow down.
 *****************************************************************************
 */
static int CreateShpPnt (char *filename, myMaparam *map, gdsType *gds,
                         sChar LatLon_Decimal, sChar f_nMissing,
                         double *grib_Data, gridAttribType *attrib)
{
   FILE *sfp;           /* The open file pointer for .shp */
   FILE *xfp;           /* The open file pointer for .shx. */
   int i;               /* A counter used for the shx values. */
   int x, y;            /* Counters used for traversing the grid. */
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   double Bounds[] = {
      0., 0., 0., 0., 0., 0., 0., 0.
   };                   /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */
   sInt4 dataType = 1;  /* Point shp type data. */
   sInt4 curRec[2];     /* rec number, and content length. */
   LatLon curDp;        /* Current lat/lon data point. */
   double *curData;     /* Current Grid cell data point (for f_nMissing) */
   int err;             /* Internal err number. */
   int recLen;          /* Length in bytes of a record in the .shp file. */
   sInt4 numRec;        /* The total number of records actually stored. */

   strncpy (filename + strlen (filename) - 3, "shp", 3);
   if ((sfp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }
   /* Perform a simple file size check.  File has to be less than
    * 4,294,967,294 bytes. (2^31-1) * 2.  Reasoning: file size in 2 byte words
    * is stored as a sInt4 */
   if ((!f_nMissing) || (attrib->f_miss == 0)) {
      /* If include all cells, the size of the
       * .shp file is: 100 + dpLen * (8 +4 +2*8= 28bytes)
       * So gds->Ny * gds->Nx <= (2^31-1) * 2 - 100 / 28 = 153,391,685.5 */
      if (gds->Ny * gds->Nx > 153391685) {
         errSprintf ("Trying to create a point shp file with %d cells.\n"
                     "This is > point maximum of 153,391,685\n",
                     gds->Ny * gds->Nx);
         return -1;
      }
   }

   /* Start Writing header in first 100 bytes. */
   Head1[0] = 9994;     /* ArcView identifier. */
   memset ((Head1 + 1), 0, 5 * sizeof (sInt4)); /* set 5 unused to 0 */
   recLen = sizeof (sInt4) + 2 * sizeof (double);
   /* .shp file size (in 2 byte words). */
   Head1[6] = (100 + (2 * sizeof (sInt4) + recLen) * gds->Nx * gds->Ny) / 2;
   FWRITE_BIG (Head1, sizeof (sInt4), 7, sfp);
   Head2[0] = 1000;     /* ArcView version identifier. */
   Head2[1] = dataType; /* Signal that these are point data. */
   FWRITE_LIT (Head2, sizeof (sInt4), 2, sfp);
   /* Write out initial guess for bounds... Need to revisit. */
   FWRITE_LIT (Bounds, sizeof (double), 8, sfp);

   /* Start Writing data. */
   curRec[1] = recLen / 2; /* Content length in (2 byte words) */
/*   curDp = dp;*/
   if ((!f_nMissing) || (attrib->f_miss == 0)) {
      myCxy2ll (map, 1, 1, &(curDp.lat), &(curDp.lon));
      curDp.lat = myRound (curDp.lat, LatLon_Decimal);
      curDp.lon = myRound (curDp.lon, LatLon_Decimal);
      Bounds[0] = Bounds[2] = curDp.lon;
      Bounds[1] = Bounds[3] = curDp.lat;
      for (y = 0; y < gds->Ny; y++) {
         for (x = 0; x < gds->Nx; x++) {
            curRec[0] = 1 + x + y * gds->Nx;
            myCxy2ll (map, x + 1, y + 1, &(curDp.lat), &(curDp.lon));
            curDp.lat = myRound (curDp.lat, LatLon_Decimal);
            curDp.lon = myRound (curDp.lon, LatLon_Decimal);

            FWRITE_BIG (curRec, sizeof (sInt4), 2, sfp);
            FWRITE_LIT (&dataType, sizeof (sInt4), 1, sfp);
            FWRITE_LIT (&(curDp.lon), sizeof (double), 1, sfp);
            FWRITE_LIT (&(curDp.lat), sizeof (double), 1, sfp);
            /* Update Bounds. */
            if (curDp.lon < Bounds[0]) {
               Bounds[0] = curDp.lon;
            } else if (curDp.lon > Bounds[2]) {
               Bounds[2] = curDp.lon;
            }
            if (curDp.lat < Bounds[1]) {
               Bounds[1] = curDp.lat;
            } else if (curDp.lat > Bounds[3]) {
               Bounds[3] = curDp.lat;
            }
         }
      }
      numRec = gds->Nx * gds->Ny;
   } else {
      curRec[0] = 1;
      for (y = 0; y < gds->Ny; y++) {
         curData = grib_Data + y * gds->Nx;
         for (x = 0; x < gds->Nx; x++) {
            if ((*curData != attrib->missPri) &&
                ((attrib->f_miss != 2) || (*curData != attrib->missSec))) {
               myCxy2ll (map, x + 1, y + 1, &(curDp.lat), &(curDp.lon));
               curDp.lat = myRound (curDp.lat, LatLon_Decimal);
               curDp.lon = myRound (curDp.lon, LatLon_Decimal);

               FWRITE_BIG (curRec, sizeof (sInt4), 2, sfp);
               FWRITE_LIT (&dataType, sizeof (sInt4), 1, sfp);
               FWRITE_LIT (&(curDp.lon), sizeof (double), 1, sfp);
               FWRITE_LIT (&(curDp.lat), sizeof (double), 1, sfp);
               /* Update Bounds. */
               if (curRec[0] == 1) {
                  Bounds[0] = Bounds[2] = curDp.lon;
                  Bounds[1] = Bounds[3] = curDp.lat;
               } else {
                  if (curDp.lon < Bounds[0]) {
                     Bounds[0] = curDp.lon;
                  } else if (curDp.lon > Bounds[2]) {
                     Bounds[2] = curDp.lon;
                  }
                  if (curDp.lat < Bounds[1]) {
                     Bounds[1] = curDp.lat;
                  } else if (curDp.lat > Bounds[3]) {
                     Bounds[3] = curDp.lat;
                  }
               }
               curRec[0]++;
               /* The size of the .shp file is:
                * 100 + dpLen * (8 +4 +2*8= 28bytes)
                * So curRec[0] <= (2^31-1) * 2 - 100 / 28 = 153,391,685.5 */
               if (curRec[0] > 153391685) {
                  errSprintf ("Trying to create a point shp file with %d cells.\n"
                              "This is > point maximum of 153,391,685\n",
                              curRec[0]);
                  return -1;
               }
            }
            curData++;
/*            curDp++;*/
         }
      }
      numRec = curRec[0] - 1;
      /* Store the updated file length. */
      /* .shp file size (in 2 byte words). */
      Head1[6] = (100 + (2 * sizeof (sInt4) + recLen) * numRec) / 2;
      fseek (sfp, 24, SEEK_SET);
      FWRITE_BIG (&(Head1[6]), sizeof (sInt4), 1, sfp);
   }
   /* Store the updated Bounds. */
   fseek (sfp, 36, SEEK_SET);
   /* FWRITE - 4 since we are only updating 4 bounds (not 8). */
   FWRITE_LIT (Bounds, sizeof (double), 4, sfp);
   fflush (sfp);
   fclose (sfp);
   /* Check that .shp is now the correct file size. */
   if ((err = checkFileSize (filename, Head1[6] * 2)) != 0) {
      return err;
   }

   filename[strlen (filename) - 1] = 'x';
   if ((xfp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }

   /* Write ArcView header (.shx file). */
   Head1[6] = (100 + 8 * numRec) / 2; /* shx file size (in words). */
   FWRITE_BIG (Head1, sizeof (sInt4), 7, xfp);
   FWRITE_LIT (Head2, sizeof (sInt4), 2, xfp);
   FWRITE_LIT (Bounds, sizeof (double), 8, xfp);

   curRec[0] = 50;      /* 100 bytes / 2 = 50 words */
   curRec[1] = recLen / 2; /* Content length in words (2 bytes) */
   for (i = 0; i < numRec; i++) {
      FWRITE_BIG (curRec, sizeof (sInt4), 2, xfp);
      curRec[0] += (recLen + 2 * sizeof (sInt4)) / 2; /* (2 byte words) */
   }
   fflush (xfp);
   fclose (xfp);

   /* Check that .shx is now the correct file size. */
   return checkFileSize (filename, 100 + 8 * numRec);
}

/*****************************************************************************
 * CreateDbf() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This creates a .dbf file.  The .dbf file contains the actual info, which
 * is associated with the points.  The .dbf file format is very flexible but
 * unfortunately is mostly in ASCII.
 *
 * ARGUMENTS
 *   filename = Name of file to save to. (Output)
 *     Nx, Ny = The dimensions of the grid. (Input)
 *  grib_Data = The actual data (assumed parsed by ParseGrid) (Input)
 *    element = Name of current variable. (Input)
 *   fieldLen = How many char to store the # in (includes decLen). (Input)
 *     decLen = How many char to store decimal part of # in. (Input)
 * f_nMissing = True if we should NOT store missing. (Input)
 *     attrib = What kind of missing value management is used. (Input)
 *  f_verbose = True if we want the verbose form. (Input)
 *        map = Holds the current map projection info to compute from. (In)
 *
 * FILES/DATABASES:
 *   Creates a .dbf file, which is a binary file (Little Endian) consisting of
 *      the info to associate with the lat/lon point shapes stored in the .shp
 *      and .shx files.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems opening the files.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (RY,FC,MA,&TB): Code Review.
 *   4/2003 AAT: Removed "include lat/lon option", because it was not being
 *          used and because it is difficult to implement for polygon .shp
 *          files.
 *   5/2003 AAT Switched from using " %08ld%10.3f" to having the fieldLen
 *          and decLen passed in because GRIB1 default missing is so large
 *          that it couldn't be stored.
 *   5/2003 AAT Added f_nMissing option.
 *   5/2003 AAT: Added rounding to decimal.
 *   5/2003 AAT: Decided to have 1,1 be lower left corner in .shp files.
 *   1/2005 AAT: Modified for verbose output
 *
 * NOTES
 * 1) Look up "address" in dbf definition.
 *****************************************************************************
 */
static int CreateDbf (char *filename, sInt4 Nx, sInt4 Ny,
                      double *grib_Data, const char *element, uChar fieldLen,
                      sChar decLen, sChar f_nMissing,
                      gridAttribType *attrib, char f_verbose, myMaparam *map)
{
   uChar header[] = { 3, 101, 4, 20 }; /* Header info for dbf. */
   int numCol;          /* The number of columns we use. */
   char names[6][11];   /* Field (column) names. */
   char type[6];        /* Type of data (number in all columns.) */
   uChar fldLen[6];     /* field len for columns. */
   uChar fldDec[6];     /* field decimal lens for columns. */
   sInt4 reserved[] = { 0, 0, 0, 0, 0 }; /* need 20 bytes of 0. */
   sInt4 address = 0;   /* Address to find data? */
   sInt4 NxNy = Nx * Ny; /* The total number of values. */
   FILE *fp;            /* The open .dbf file pointer. */
   int x, y;            /* Current grid cell location. */
   short int recLen;    /* Size in bytes of one cell of data */
   sInt4 id;            /* A unique decimal id for the current cell. */
   double *curData;     /* A pointer to Grib data for current cell. */
   sInt4 totSize;       /* Total size of the .dbf file. */
   uChar uc_temp;       /* Temp. storage of type unsigned char. */
   short int si_temp;   /* Temp. storage of type short int. */
   char formatBuf[20];  /* Used to format float output sent to .dbf */
   sInt4 numRec;        /* The total number of records actually stored. */
   double shift;        /* power of 10 used in rounding. */
   double lat;          /* Latitude of the current point for f_verbose */
   double lon;          /* Longitude of the current point for f_verbose */

   if (decLen > 17)
      decLen = 17;
   if (decLen < 0)
      decLen = 0;
   shift = POWERS_ONE[decLen];
   strncpy (filename + strlen (filename) - 3, "dbf", 3);
   if ((fp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }
   if (f_verbose) {
      numCol = 6;
      strncpy (names[0], "POINTID", 11);
      type[0] = 'N';
      fldLen[0] = 8;
      fldDec[0] = 0;
      strncpy (names[1], "X", 11);
      type[1] = 'N';
      fldLen[1] = 4;
      fldDec[1] = 0;
      strncpy (names[2], "Y", 11);
      type[2] = 'N';
      fldLen[2] = 4;
      fldDec[2] = 0;
      strncpy (names[3], "LON", 11);
      type[3] = 'N';
      fldLen[3] = 10;
      fldDec[3] = 5;
      strncpy (names[4], "LAT", 11);
      type[4] = 'N';
      fldLen[4] = 9;
      fldDec[4] = 5;
      strncpy (names[5], element, 11);
      type[5] = 'N';
      fldLen[5] = fieldLen;
      fldDec[5] = decLen;
      recLen = (short int) (1 + fldLen[0] + fldLen[1] + fldLen[2] + fldLen[3]
                            + fldLen[4] + fldLen[5]);
   } else {
      numCol = 2;
      strncpy (names[0], "POINTID", 11);
      type[0] = 'N';
      fldLen[0] = 8;
      fldDec[0] = 0;
      strncpy (names[1], element, 11);
      type[1] = 'N';
      fldLen[1] = fieldLen;
      fldDec[1] = decLen;
      recLen = (short int) (1 + fldLen[0] + fldLen[1]);
   }
   totSize = 1 + 32 + 32 * numCol + recLen * NxNy;

   /* Start writing the header. */
   /* Write the ID and date of last update. */
   fwrite (header, sizeof (char), 4, fp);
   /* Write number of records, size of header, then length of record. */
   FWRITE_LIT (&NxNy, sizeof (sInt4), 1, fp);
   si_temp = (short int) (32 + 32 * numCol + 1);
   FWRITE_LIT (&si_temp, sizeof (short int), 1, fp);
   FWRITE_LIT (&recLen, sizeof (short int), 1, fp);
   /* Write Reserved bytes.. 20 bytes of them.. all 0. */
   fwrite (reserved, sizeof (char), 20, fp);

   /* Write field descriptor array. */
   for (x = 0; x < numCol; x++) {
      /* Already taken care of padding with 0's since we did a strncpy */
      fwrite (names[x], sizeof (char), 11, fp);
      fputc (type[x], fp);
      /* Should be FWRITE_LIT, but doesn't matter. 11/8/2004 */
      fwrite (&(address), sizeof (sInt4), 1, fp); /* with address 0 */
      fputc (fldLen[x], fp);
      fputc (fldDec[x], fp);
      fwrite (reserved, sizeof (char), 14, fp); /* reserved already has 0's */
   }
   /* Write trailing header character. */
   uc_temp = 13;
   fputc (uc_temp, fp);

   /* Start writing the body. */
   if (f_verbose) {
      sprintf (formatBuf, " %%08ld%%0%dd%%0%dd%%%d.%df%%%d.%df%%%d.%df",
               fldLen[1], fldLen[2], fldLen[3], fldDec[3], fldLen[4],
               fldDec[4], fldLen[5], fldDec[5]);
   } else {
      sprintf (formatBuf, " %%08ld%%%d.%df", fldLen[1], fldDec[1]);
   }
   if ((!f_nMissing) || (attrib->f_miss == 0)) {
      for (y = 0; y < Ny; y++) {
         curData = grib_Data + y * Nx;
         /* Ny is 1 if we are creating BigPolyDbf files. */
         if (Ny == 1) {
            id = y + 1;
         } else {
            id = 10000 + y + 1;
         }
         /* May have 2 missing values (ie 9999 && 9998) in the .dbf file. */
         for (x = 0; x < Nx; x++) {
            if (f_verbose) {
               myCxy2ll (map, x + 1, y + 1, &lat, &lon);
               lat = myRound (lat, 5);
               lon = myRound (lon, 5);
               fprintf (fp, formatBuf, id, x + 1, y + 1, lon, lat,
                        ((floor (*curData * shift + .5)) / shift));
            } else {
               fprintf (fp, formatBuf, id, ((floor (*curData * shift + .5))
                                            / shift));
            }
            curData++;
            /* Ny is 1 if we are creating BigPolyDbf files. */
            if (Ny == 1) {
               id++;
            } else {
               id += 10000;
            }
         }
      }
   } else {
      numRec = 0;
      for (y = 0; y < Ny; y++) {
         curData = grib_Data + y * Nx;
         /* Ny is 1 if we are creating BigPolyDbf files. */
         if (Ny == 1) {
            id = y + 1;
         } else {
            id = 10000 + y + 1;
         }
         for (x = 0; x < Nx; x++) {
            if ((*curData != attrib->missPri) &&
                ((attrib->f_miss != 2) || (*curData != attrib->missSec))) {
               if (f_verbose) {
                  myCxy2ll (map, x + 1, y + 1, &lat, &lon);
                  lat = myRound (lat, 5);
                  lon = myRound (lon, 5);
                  fprintf (fp, formatBuf, id, x + 1, y + 1, lon, lat,
                           ((floor (*curData * shift + .5)) / shift));
               } else {
                  fprintf (fp, formatBuf, id,
                           ((floor (*curData * shift + .5)) / shift));
               }
               numRec++;
            }
            curData++;
            /* Ny is 1 if we are creating BigPolyDbf files. */
            if (Ny == 1) {
               id++;
            } else {
               id += 10000;
            }
         }
      }
      /* Update file total # of records. */
      fseek (fp, 4, SEEK_SET);
      FWRITE_LIT (&numRec, sizeof (sInt4), 1, fp);
      totSize = 1 + 32 + 32 * numCol + recLen * numRec;
   }
   fclose (fp);

   /* Check that .dbf is now the correct file size. */
   return checkFileSize (filename, totSize);
}


/*****************************************************************************
 * CreateWxDbf() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This creates a .dbf file.  The .dbf file contains the actual info, which
 * is associated with the points.  The .dbf file format is very flexible but
 * unfortunately is mostly in ASCII.
 *   The reason this is separated from CreateDbf() is because the weather
 * data is a lot more complex than regular elements, so we need to provide
 * more columns to the user.
 *
 * ARGUMENTS
 *   filename = Name of file to save to. (Output)
 *     Nx, Ny = The dimensions of the grid. (Input)
 *  grib_Data = The actual data (assumed parsed by ParseGrid) (Input)
 *    element = Name of current variable. (Input)
 * f_nMissing = True if we should NOT store missing. (Input)
 *     attrib = What kind of missing value management is used. (Input)
 *     wxType = The lookup table associated with the weather. (Input)
 *  f_verbose = True if we want the verbose form. (Input)
 *        map = Holds the current map projection info to compute from. (In)
 *
 * FILES/DATABASES:
 *   Creates a .dbf file, which is a binary file (Little Endian) consisting of
 *      the info to associate with the lat/lon point shapes stored in the .shp
 *      and .shx files.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems opening the files.
 *
 * HISTORY
 *   5/2003 Arthur Taylor (MDL/RSIS): Created.
 *   5/2003 AAT: Decided to have 1,1 be lower left corner in .shp files.
 *   7/2003 AAT: Added "SimpleWx" column.
 *   1/2005 AAT: Modified for verbose output
 *
 * NOTES
 *****************************************************************************
 */
static int CreateWxDbf (char *filename, sInt4 Nx, sInt4 Ny,
                        double *grib_Data, const char *element,
                        sChar f_nMissing, gridAttribType *attrib,
                        sect2_WxType *WxType, char f_verbose, myMaparam *map)
{
   uChar header[] = { 3, 101, 4, 20 }; /* Header info for dbf. */
   int numColN = 25;    /* The number of columns we use. */
   int numColV = 29;    /* The number of columns we use. */
   char namesN[][11] = {
      "POINTID", "element", "WX-INDEX", "Visibility",
      "NDFDWxCode",
      "Weather_1", "WX-Inten_1", "Cover_1", "Hazard_1",
      "Weather_2", "WX-Inten_2", "Cover_2", "Hazard_2",
      "Weather_3", "WX-Inten_3", "Cover_3", "Hazard_3",
      "Weather_4", "WX-Inten_4", "Cover_4", "Hazard_4",
      "Weather_5", "WX-Inten_5", "Cover_5", "Hazard_5",
   };                   /* Field (column) names. */
   char namesV[][11] = {
      "POINTID", "X", "Y", "LON", "LAT",
      "element", "WX-INDEX", "Visibility", "NDFDWxCode",
      "Weather_1", "WX-Inten_1", "Cover_1", "Hazard_1",
      "Weather_2", "WX-Inten_2", "Cover_2", "Hazard_2",
      "Weather_3", "WX-Inten_3", "Cover_3", "Hazard_3",
      "Weather_4", "WX-Inten_4", "Cover_4", "Hazard_4",
      "Weather_5", "WX-Inten_5", "Cover_5", "Hazard_5",
   };                   /* Field (column) names. */
   char typeN[] = {
      'N', 'C', 'N', 'N',
      'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
   };                   /* Type of data for columns. */
   char typeV[] = {
      'N', 'N', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
      'C', 'N', 'N', 'N',
   };                   /* Type of data for columns. */
   uChar fldLenN[] = {
      8, 0, 8, 8,
      4,
      0, 3, 2, 10,
      0, 3, 2, 10,
      0, 3, 2, 10,
      0, 3, 2, 10,
      0, 3, 2, 10,
   };                   /* field len for columns. */
   uChar fldLenV[] = {
      8, 4, 4, 10, 9,
      0, 8, 8, 4,
      0, 3, 2, 10,
      0, 3, 2, 10,
      0, 3, 2, 10,
      0, 3, 2, 10,
      0, 3, 2, 10,
   };                   /* field len for columns. */
   uChar fldDecN[] = {
      0, 0, 0, 2,
      0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0
   };                   /* field decimal lens for columns. */
   uChar fldDecV[] = {
      0, 0, 0, 5, 5,
      0, 0, 2, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0,
      0, 0, 0, 0
   };                   /* field decimal lens for columns. */
   int cnt;             /* Used to loop over fldLen[] to compute recLen */
   sInt4 reserved[] = { 0, 0, 0, 0, 0 }; /* need 20 bytes of 0. */
   sInt4 address = 0;   /* Address to find data? */
   sInt4 NxNy = Nx * Ny; /* The total number of values. */
   FILE *fp;            /* The open .dbf file pointer. */
   int x, y;            /* Current grid cell location. */
   short int recLen;    /* Size in bytes of one cell of data */
   sInt4 id;            /* A unique decimal id for the current cell. */
   double *curData;     /* A pointer to Grib data for current cell. */
   sInt4 totSize;       /* Total size of the .dbf file. */
   uChar uc_temp;       /* Temp. storage of type unsigned char. */
   short int si_temp;   /* Temp. storage of type short int. */
   char formBuf1[100];  /* Used to format float output sent to .dbf */
   char formBuf[NUM_UGLY_WORD][100]; /* Used to format weather columns */
   sInt4 numRec;        /* The total number of records actually stored. */
   char buffer[100];    /* Stores index if we can't look it up in table. */
   uInt4 index;         /* Current index into Wx table. */
   double vis;          /* Used to determine if we have "missing" vis. */
   char *ptr;           /* Help print english version of Wx elements. */
   uChar wx_inten;      /* Help print Wx code. */
   sInt4 HazCode;       /* Help print hazard codes. */
   uChar cover;         /* Help print coverage code. */
   int i;               /* Helps init ptr, wx_inten, hazard, and cover. */
   double lat;          /* Latitude of the current point for f_verbose */
   double lon;          /* Longitude of the current point for f_verbose */

   strncpy (filename + strlen (filename) - 3, "dbf", 3);
   if ((fp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }
/*
   codeLen = WxType->dataLen;
   CodeTable = (int *) malloc (codeLen * sizeof (int));
   for (i = 0; i < codeLen; i++) {
      CodeTable[i] = NDFD_WxTable (&(WxType->ugly[i]));
   }
*/

   /* Figure out the field lengths, and the fprintf format. */
   if (f_verbose) {
      fldLenV[5] = WxType->maxLen;
      sprintf (formBuf1, " %%08ld%%0%dd%%0%dd%%%d.%df%%%d.%df%%%ds%%08ld"
               "%%8.2f%%04ld", fldLenV[1], fldLenV[2], fldLenV[3],
               fldDecV[3], fldLenV[4], fldDecV[4], fldLenV[5]);
   } else {
      fldLenN[1] = WxType->maxLen;
      sprintf (formBuf1, " %%08ld%%%ds%%08ld%%8.2f%%04ld", fldLenN[1]);
   }

   /* Reserve enough space in each english column for "unknown" */
   for (i = 0; i < NUM_UGLY_WORD; i++) {
      if (WxType->maxEng[i] != 0) {
         sprintf (formBuf[i], "%%%ds%%03d%%02d%%010ld", WxType->maxEng[i]);
         if (f_verbose) {
            fldLenV[9 + i * 4] = WxType->maxEng[i];
         } else {
            fldLenN[5 + i * 4] = WxType->maxEng[i];
         }
      } else {
         sprintf (formBuf[i], "%%%ds%%01d%%01d%%01ld", 7);
         if (f_verbose) {
            fldLenV[9 + i * 4] = 7;
            fldLenV[10 + i * 4] = 1;
            fldLenV[11 + i * 4] = 1;
            fldLenV[12 + i * 4] = 1;
         } else {
            fldLenN[5 + i * 4] = 7;
            fldLenN[6 + i * 4] = 1;
            fldLenN[7 + i * 4] = 1;
            fldLenN[8 + i * 4] = 1;
         }
      }
   }

   if (f_verbose) {
      strncpy (namesV[5], element, 11);
      recLen = (short int) 1;
      for (cnt = 0; cnt < numColV; cnt++) {
         recLen += (short int) (fldLenV[cnt]);
      }
   } else {
      strncpy (namesN[1], element, 11);
      recLen = (short int) 1;
      for (cnt = 0; cnt < numColN; cnt++) {
         recLen += (short int) (fldLenN[cnt]);
      }
   }

   /* Start writing the header. */
   /* Write the ID and date of last update. */
   fwrite (header, sizeof (char), 4, fp);
   /* Write number of records, size of header, then length of record. */
   FWRITE_LIT (&NxNy, sizeof (sInt4), 1, fp);
   if (f_verbose) {
      si_temp = (short int) (32 + 32 * numColV + 1);
   } else {
      si_temp = (short int) (32 + 32 * numColN + 1);
   }
   FWRITE_LIT (&si_temp, sizeof (short int), 1, fp);
   FWRITE_LIT (&recLen, sizeof (short int), 1, fp);
   /* Write Reserved bytes.. 20 bytes of them.. all 0. */
   fwrite (reserved, sizeof (char), 20, fp);

   /* Write field descriptor array. */
   if (f_verbose) {
      for (x = 0; x < numColV; x++) {
         /* Already taken care of padding with 0's since we did a strncpy */
         fwrite (namesV[x], sizeof (char), 11, fp);
         fputc (typeV[x], fp);
         /* with address 0 */
         FWRITE_LIT (&(address), sizeof (sInt4), 1, fp);
         fputc (fldLenV[x], fp);
         fputc (fldDecV[x], fp);
         /* reserved already has 0's */
         fwrite (reserved, sizeof (char), 14, fp);
      }
   } else {
      for (x = 0; x < numColN; x++) {
         /* Already taken care of padding with 0's since we did a strncpy */
         fwrite (namesN[x], sizeof (char), 11, fp);
         fputc (typeN[x], fp);
         /* with address 0 */
         FWRITE_LIT (&(address), sizeof (sInt4), 1, fp);
         fputc (fldLenN[x], fp);
         fputc (fldDecN[x], fp);
         /* reserved already has 0's */
         fwrite (reserved, sizeof (char), 14, fp);
      }
   }
   /* Write trailing header character. */
   uc_temp = 13;
   fputc (uc_temp, fp);

   /* Start writing the body. */
   numRec = 0;
   for (y = 0; y < Ny; y++) {
      curData = grib_Data + y * Nx;
      /* Ny is 1 if we are creating BigPolyDbf files. */
      if (Ny == 1) {
         id = y + 1;
      } else {
         id = 10000 + y + 1;
      }
      for (x = 0; x < Nx; x++) {
         if ((!f_nMissing) || (attrib->f_miss == 0) ||
             ((attrib->f_miss == 1) && (*curData != attrib->missPri)) ||
             ((attrib->f_miss == 2) && (*curData != attrib->missPri)
              && (*curData != attrib->missSec))) {
            index = (uInt4) *curData;
            vis = 9999;
            if (index < WxType->dataLen) {
               if (WxType->f_valid[index]) {
                  if (WxType->ugly[index].minVis != 255) {
                     vis = WxType->ugly[index].minVis / 32.;
                  }
                  if (f_verbose) {
                     myCxy2ll (map, x + 1, y + 1, &lat, &lon);
                     lat = myRound (lat, 5);
                     lon = myRound (lon, 5);
                     fprintf (fp, formBuf1, id, x + 1, y + 1, lon, lat,
                              WxType->data[index],
                              WxType->ugly[index].validIndex, vis,
                              (sInt4) WxType->ugly[index].SimpleCode);
                  } else {
                     fprintf (fp, formBuf1, id, WxType->data[index],
                              WxType->ugly[index].validIndex, vis,
                              (sInt4) WxType->ugly[index].SimpleCode);
                  }
                  for (i = 0; i < NUM_UGLY_WORD; i++) {
                     ptr = WxType->ugly[index].english[i];
                     if (ptr != NULL) {
                        wx_inten = WxType->ugly[index].wx_inten[i];
                        HazCode = WxType->ugly[index].HazCode[i];
                        cover = WxType->ugly[index].cover[i];
                        fprintf (fp, formBuf[i], ptr, wx_inten,
                                 cover, HazCode);
                     } else {
                        fprintf (fp, formBuf[i], "", 0, 0, 0);
                     }
                  }
               } else {
                  /* Handles a string of <Invalid><Invalid>... A rare event
                   * to begin with, typically this will be turned into a
                   * missing value in metaparse.c, but if we don't have any
                   * missing management, it sets f_valid to false. so we
                   * handle it here. */
                  if (f_verbose) {
                     myCxy2ll (map, x + 1, y + 1, &lat, &lon);
                     lat = myRound (lat, 5);
                     lon = myRound (lon, 5);
                     fprintf (fp, formBuf1, id, x + 1, y + 1, lon, lat,
                              WxType->data[index],
                              WxType->ugly[index].validIndex, vis,
                              (sInt4) 9999);
                  } else {
                     fprintf (fp, formBuf1, id, WxType->data[index],
                              WxType->ugly[index].validIndex, vis,
                              (sInt4) 9999);
                  }
                  fprintf (fp, formBuf[0], "Unkown", 0, 0, 0);
                  fprintf (fp, formBuf[1], "Unkown", 0, 0, 0);
                  fprintf (fp, formBuf[2], "Unkown", 0, 0, 0);
                  fprintf (fp, formBuf[3], "Unkown", 0, 0, 0);
                  fprintf (fp, formBuf[4], "Unkown", 0, 0, 0);
               }
            } else {
               sprintf (buffer, "%ld", index);
/*               fprintf (fp, formBuf1, id, buffer, index, vis, index); */
               if (f_verbose) {
                  myCxy2ll (map, x + 1, y + 1, &lat, &lon);
                  lat = myRound (lat, 5);
                  lon = myRound (lon, 5);
                  fprintf (fp, formBuf1, id, x + 1, y + 1, lon, lat, buffer,
                           0, vis, 0);
               } else {
                  fprintf (fp, formBuf1, id, buffer, 0, vis, 0);
               }
               fprintf (fp, formBuf[0], "Unkown", 0, 0, 0);
               fprintf (fp, formBuf[1], "Unkown", 0, 0, 0);
               fprintf (fp, formBuf[2], "Unkown", 0, 0, 0);
               fprintf (fp, formBuf[3], "Unkown", 0, 0, 0);
               fprintf (fp, formBuf[4], "Unkown", 0, 0, 0);
            }
            numRec++;
         }
         curData++;
         /* Ny is 1 if we are creating BigPolyDbf files. */
         if (Ny == 1) {
            id++;
         } else {
            id += 10000;
         }
      }
   }
   /* Update file total # of records. */
   fseek (fp, 4, SEEK_SET);
   FWRITE_LIT (&numRec, sizeof (sInt4), 1, fp);
   if (f_verbose) {
      totSize = 1 + 32 + 32 * numColV + recLen * numRec;
   } else {
      totSize = 1 + 32 + 32 * numColN + recLen * numRec;
   }
   fclose (fp);

   /* Check that .dbf is now the correct file size. */
   return checkFileSize (filename, totSize);
}

static int CreateWWADbf (char *filename, sInt4 Nx, sInt4 Ny,
                        double *grib_Data, const char *element,
                        sChar f_nMissing, gridAttribType *attrib,
                        sect2_HazardType *HazType, char f_verbose, myMaparam *map)
{
   uChar header[] = { 3, 101, 4, 20 }; /* Header info for dbf. */
   int numColN = 9;    /* The number of columns we use. */
   int numColV = 13;    /* The number of columns we use. */
   char namesN[][11] = {
      "POINTID", "element", "WWA-INDEX",
      "WWA_Code",
      "WWA_1",
      "WWA_2",
      "WWA_3",
      "WWA_4",
      "WWA_5",
   };                   /* Field (column) names. */
   char namesV[][11] = {
      "POINTID", "X", "Y", "LON", "LAT",
      "element", "WWA-INDEX",
      "WWA_Code",
      "WWA_1",
      "WWA_2",
      "WWA_3",
      "WWA_4",
      "WWA_5",
   };                   /* Field (column) names. */
   char typeN[] = {
      'N', 'C', 'N',
      'N',
      'C',
      'C',
      'C',
      'C',
      'C',
   };                   /* Type of data for columns. */
   char typeV[] = {
      'N', 'N', 'N', 'N', 'N',
      'C', 'N', 'N',
      'C',
      'C',
      'C',
      'C',
      'C',
   };                   /* Type of data for columns. */
   uChar fldLenN[] = {
      8, 0, 8,
      4,
      0,
      0,
      0,
      0,
      0,
   };                   /* field len for columns. */
   uChar fldLenV[] = {
      8, 4, 4, 10, 9,
      0, 8, 4,
      0,
      0,
      0,
      0,
      0,
   };                   /* field len for columns. */
   uChar fldDecN[] = {
      0, 0, 0,
      0,
      0,
      0,
      0,
      0,
      0,
   };                   /* field decimal lens for columns. */
   uChar fldDecV[] = {
      0, 0, 0, 5, 5,
      0, 0, 0,
      0,
      0,
      0,
      0,
      0,
   };                   /* field decimal lens for columns. */
   int cnt;             /* Used to loop over fldLen[] to compute recLen */
   sInt4 reserved[] = { 0, 0, 0, 0, 0 }; /* need 20 bytes of 0. */
   sInt4 address = 0;   /* Address to find data? */
   sInt4 NxNy = Nx * Ny; /* The total number of values. */
   FILE *fp;            /* The open .dbf file pointer. */
   int x, y;            /* Current grid cell location. */
   short int recLen;    /* Size in bytes of one cell of data */
   sInt4 id;            /* A unique decimal id for the current cell. */
   double *curData;     /* A pointer to Grib data for current cell. */
   sInt4 totSize;       /* Total size of the .dbf file. */
   uChar uc_temp;       /* Temp. storage of type unsigned char. */
   short int si_temp;   /* Temp. storage of type short int. */
   char formBuf1[100];  /* Used to format float output sent to .dbf */
   char formBuf[NUM_HAZARD_WORD][100]; /* Used to format hazard columns */
   sInt4 numRec;        /* The total number of records actually stored. */
   char buffer[100];    /* Stores index if we can't look it up in table. */
   uInt4 index;         /* Current index into WWA table. */
   char *ptr;           /* Help print english version of WWA elements. */
   int i;               /* Helps init ptr, wwa_inten. */
   double lat;          /* Latitude of the current point for f_verbose */
   double lon;          /* Longitude of the current point for f_verbose */

   strncpy (filename + strlen (filename) - 3, "dbf", 3);
   if ((fp = fopen (filename, "wb")) == NULL) {
      errSprintf ("ERROR: Problems opening %s for write.", filename);
      return -1;
   }

   /* Figure out the field lengths, and the fprintf format. */
   if (f_verbose) {
      fldLenV[5] = HazType->maxLen;
      sprintf (formBuf1, " %%08ld%%0%dd%%0%dd%%%d.%df%%%d.%df%%%ds%%08ld"
               "%%04ld", fldLenV[1], fldLenV[2], fldLenV[3],
               fldDecV[3], fldLenV[4], fldDecV[4], fldLenV[5]);
   } else {
      fldLenN[1] = HazType->maxLen;
      sprintf (formBuf1, " %%08ld%%%ds%%08ld%%04ld", fldLenN[1]);
   }

   /* Reserve enough space in each english column for "unknown" */
   for (i = 0; i < NUM_HAZARD_WORD; i++) {
      if (HazType->maxEng[i] != 0) {
         sprintf (formBuf[i], "%%%ds", HazType->maxEng[i]);
         if (f_verbose) {
            fldLenV[8 + i] = HazType->maxEng[i];
         } else {
            fldLenN[4 + i] = HazType->maxEng[i];
         }
      } else {
         sprintf (formBuf[i], "%%%ds", 7);
         if (f_verbose) {
            fldLenV[8 + i] = 7;
         } else {
            fldLenN[4 + i] = 7;
         }
      }
   }

   if (f_verbose) {
      strncpy (namesV[5], element, 11);
      recLen = (short int) 1;
      for (cnt = 0; cnt < numColV; cnt++) {
         recLen += (short int) (fldLenV[cnt]);
      }
   } else {
      strncpy (namesN[1], element, 11);
      recLen = (short int) 1;
      for (cnt = 0; cnt < numColN; cnt++) {
         recLen += (short int) (fldLenN[cnt]);
      }
   }

   /* Start writing the header. */
   /* Write the ID and date of last update. */
   fwrite (header, sizeof (char), 4, fp);
   /* Write number of records, size of header, then length of record. */
   FWRITE_LIT (&NxNy, sizeof (sInt4), 1, fp);
   if (f_verbose) {
      si_temp = (short int) (32 + 32 * numColV + 1);
   } else {
      si_temp = (short int) (32 + 32 * numColN + 1);
   }
   FWRITE_LIT (&si_temp, sizeof (short int), 1, fp);
   FWRITE_LIT (&recLen, sizeof (short int), 1, fp);
   /* Write Reserved bytes.. 20 bytes of them.. all 0. */
   fwrite (reserved, sizeof (char), 20, fp);

   /* Write field descriptor array. */
   if (f_verbose) {
      for (x = 0; x < numColV; x++) {
         /* Already taken care of padding with 0's since we did a strncpy */
         fwrite (namesV[x], sizeof (char), 11, fp);
         fputc (typeV[x], fp);
         /* with address 0 */
         FWRITE_LIT (&(address), sizeof (sInt4), 1, fp);
         fputc (fldLenV[x], fp);
         fputc (fldDecV[x], fp);
         /* reserved already has 0's */
         fwrite (reserved, sizeof (char), 14, fp);
      }
   } else {
      for (x = 0; x < numColN; x++) {
         /* Already taken care of padding with 0's since we did a strncpy */
         fwrite (namesN[x], sizeof (char), 11, fp);
         fputc (typeN[x], fp);
         /* with address 0 */
         FWRITE_LIT (&(address), sizeof (sInt4), 1, fp);
         fputc (fldLenN[x], fp);
         fputc (fldDecN[x], fp);
         /* reserved already has 0's */
         fwrite (reserved, sizeof (char), 14, fp);
      }
   }
   /* Write trailing header character. */
   uc_temp = 13;
   fputc (uc_temp, fp);

   /* Start writing the body. */
   numRec = 0;
   for (y = 0; y < Ny; y++) {
      curData = grib_Data + y * Nx;
      /* Ny is 1 if we are creating BigPolyDbf files. */
      if (Ny == 1) {
         id = y + 1;
      } else {
         id = 10000 + y + 1;
      }
      for (x = 0; x < Nx; x++) {
         if ((!f_nMissing) || (attrib->f_miss == 0) ||
             ((attrib->f_miss == 1) && (*curData != attrib->missPri)) ||
             ((attrib->f_miss == 2) && (*curData != attrib->missPri)
              && (*curData != attrib->missSec))) {
            index = (uInt4) *curData;
            if (index < HazType->dataLen) {
               if (HazType->f_valid[index]) {
                  if (f_verbose) {
                     myCxy2ll (map, x + 1, y + 1, &lat, &lon);
                     lat = myRound (lat, 5);
                     lon = myRound (lon, 5);
                     fprintf (fp, formBuf1, id, x + 1, y + 1, lon, lat,
                              HazType->data[index],
                              HazType->haz[index].validIndex,
                              (sInt4) HazType->haz[index].SimpleCode);
                  } else {
                     fprintf (fp, formBuf1, id, HazType->data[index],
                              HazType->haz[index].validIndex,
                              (sInt4) HazType->haz[index].SimpleCode);
                  }
                  for (i = 0; i < NUM_HAZARD_WORD; i++) {
                     ptr = HazType->haz[index].english[i];
                     if (ptr != NULL) {
                        fprintf (fp, formBuf[i], ptr);
                     } else {
                        fprintf (fp, formBuf[i], "");
                     }
                  }
               } else {
                  /* Handles a string of <Invalid><Invalid>... A rare event
                   * to begin with, typically this will be turned into a
                   * missing value in metaparse.c, but if we don't have any
                   * missing management, it sets f_valid to false. so we
                   * handle it here. */
                  if (f_verbose) {
                     myCxy2ll (map, x + 1, y + 1, &lat, &lon);
                     lat = myRound (lat, 5);
                     lon = myRound (lon, 5);
                     fprintf (fp, formBuf1, id, x + 1, y + 1, lon, lat,
                              HazType->data[index],
                              HazType->haz[index].validIndex,
                              (sInt4) 9999);
                  } else {
                     fprintf (fp, formBuf1, id, HazType->data[index],
                              HazType->haz[index].validIndex,
                              (sInt4) 9999);
                  }
                  fprintf (fp, formBuf[0], "Unkown");
                  fprintf (fp, formBuf[1], "Unkown");
                  fprintf (fp, formBuf[2], "Unkown");
                  fprintf (fp, formBuf[3], "Unkown");
                  fprintf (fp, formBuf[4], "Unkown");
               }
            } else {
               sprintf (buffer, "%ld", index);
/*               fprintf (fp, formBuf1, id, buffer, index, vis, index); */
               if (f_verbose) {
                  myCxy2ll (map, x + 1, y + 1, &lat, &lon);
                  lat = myRound (lat, 5);
                  lon = myRound (lon, 5);
                  fprintf (fp, formBuf1, id, x + 1, y + 1, lon, lat, buffer,
                           0, 0);
               } else {
                  fprintf (fp, formBuf1, id, buffer, 0, 0);
               }
               fprintf (fp, formBuf[0], "Unkown");
               fprintf (fp, formBuf[1], "Unkown");
               fprintf (fp, formBuf[2], "Unkown");
               fprintf (fp, formBuf[3], "Unkown");
               fprintf (fp, formBuf[4], "Unkown");
            }
            numRec++;
         }
         curData++;
         /* Ny is 1 if we are creating BigPolyDbf files. */
         if (Ny == 1) {
            id++;
         } else {
            id += 10000;
         }
      }
   }
   /* Update file total # of records. */
   fseek (fp, 4, SEEK_SET);
   FWRITE_LIT (&numRec, sizeof (sInt4), 1, fp);
   if (f_verbose) {
      totSize = 1 + 32 + 32 * numColV + recLen * numRec;
   } else {
      totSize = 1 + 32 + 32 * numColN + recLen * numRec;
   }
   fclose (fp);

   /* Check that .dbf is now the correct file size. */
   return checkFileSize (filename, totSize);
}

/*****************************************************************************
 * GridCompute() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This computes all the lat/lon values of the grid, and returns the
 * bounding lat/lon region of the grid.
 *   If f_corner, then it computes the 4 corners of the grid cells, instead
 * of the center points.
 *
 * ARGUMENTS
 *      map = Holds the current map projection info to compute from. (Input)
 *      gds = Grid Definiton from the parsed GRIB msg to write. (Input)
 *       dp = A lat/lon array large enough for gds->Nx*Ny points. (Output)
 * f_corner = 1 compute corners of grid cells, 0 compute centers. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  11/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (RY,FC,MA,&TB): Code Review.
 *   4/2003 AAT: Added f_corner option.
 *   5/2003 AAT: Removed requirement for computation of min/max lat/lon.
 *   5/2003 AAT: Decided to have 1,1 be lower left corner in .shp files.
 *   3/2004 AAT: Switched to 6 decimal accuracy on lat/lon
 *               (because that is native of GRIB2 messages)
 *
 * NOTES
 * 1) Round to 6 decimals (because that is the accuracy of GRIB2 messages)
 *    (6 is too much for HP/Linux match)
 *    (7 is too much for Linux/Cygwin match)
 * 2) The grid for spatial analyst starts in upper left corner.
 *    So for consistancy sake we also start in upper left corner.
 *****************************************************************************
 */
static void GridCompute (myMaparam *map, gdsType *gds, LatLon *dp,
                         sChar f_corner, sChar LatLon_Decimal)
{
   LatLon *cur_dp;      /* Current lat/lon point. */
   uInt4 x, y;          /* loop index while computing the grid. */

   cur_dp = dp;
   for (y = 1; y <= gds->Ny + f_corner; y++) {
      for (x = 1; x <= gds->Nx + f_corner; x++) {
         myCxy2ll (map, x - f_corner * .5, y - f_corner * .5, &cur_dp->lat,
                   &cur_dp->lon);
         cur_dp->lat = myRound (cur_dp->lat, LatLon_Decimal);
         cur_dp->lon = myRound (cur_dp->lon, LatLon_Decimal);
         cur_dp++;
      }
   }
}

/*****************************************************************************
 * gribWriteShp() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This controls the creation of a .shp/.shx/.dbf file set.  These are
 * Esri ArcView specific file formats, but it was felt that we should provide
 * the capability of writing to them as it is a common GIS package.  After
 * it generates the .shp file set, it then creates a .ave script which can
 * be run to make sure that the projection is set in ArcView the way the Grib
 * data wanted it.  Main reason is to get the correct radius of the earth.
 *
 * ARGUMENTS
 *   Filename = Name of file to save to. (Input)
 *  grib_Data = The grib2 data to write. (Input)
 *        gds = Grid Definition from the parsed GRIB msg to write. (Input)
 *     f_poly = 2 => BigPoly, 1 => polygon, 0 => point .shp file, (Input)
 * f_nMissing = True if we do not want missing values in the .shp file. (In)
 *    decimal = How many decimals to round to. (Input)
 *  f_verbose = True if we want the verbose output. (Input)
 *
 * FILES/DATABASES:
 *   Either Calls CreateShpPnt to create the Esri .shp and .shx files.
 *   or    Calls CreateShpPoly to create the ESRI .shp and .shx files.
 *   Calls CreateDbf to create the Esri .dbf file.
 *   Calls gribWriteEsriAve to create the Esri .ave script.
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = illegal declaration of the grid size.
 * -2 = Problems opening the files.
 * -3 = un-supported map projection.
 * -4 = Error in CreateShpPnt
 * -5 = Error in CreateDbf
 * -6 = Error in CreatePrj
 *  1 = invalid parameters for gribWriteEsriAve.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (RY,FC,MA,&TB): Code Review.
 *   4/2003 AAT: Added calls to CreateShpPoly.
 *   5/2003 AAT: Switched from gdsType to grib_MetaData type so we can use
 *          the info record to determine good field lengths for the .dbf file.
 *   5/2003 AAT: Added rounding to decimal.
 *   5/2003 AAT: Enabled other spherical earths.
 *   6/2003 AAT: Switched to a Warning and then averaging if Dx != Dy.
 *   7/2003 AAT: Bug dealing with min = max = 0 for dbf file.
 *   7/2003 AAT: Proper handling of Dx != Dy.
 *   7/2003 AAT: switched to checking against element name for Wx instead
 *          of pds2.sect2.ptrType == GS2_WXTYPE
 *  10/2003 AAT: Added Calls to CreateBigPolyShp()
 *   1/2005 AAT: Added Call to CreatePrj()
 *   1/2005 AAT: Modified for verbose output
 *
 * NOTES
 * 1) Order is .shp/.shx then .dbf, then .ave.  If .ave doesn't work they have
 * the important stuff.
 *****************************************************************************
 */
int gribWriteShp (const char *Filename, double *grib_Data,
                  grib_MetaData *meta, sChar f_poly, sChar f_nMissing,
                  sChar decimal, sChar LatLon_Decimal, char f_verbose)
{
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   char *filename;      /* local copy of the filename. */
   int nameLen;         /* length of filename. */
   LatLon *dp;          /* Array of lat/lon points. */
   double orient;       /* Orientation longitude of projection (where N is
                         * up.) (between -180 and 180) */
   gdsType *gds = &(meta->gds); /* Simplifies references to the gds data. */
   double max, min;     /* Used to figure out the largest number we have to
                         * store in the .dbf file. */
   uChar fieldLen1;     /* How many char to store the max number in. */
   uChar fieldLen2;     /* How many char to store the min number in. */
   polyType *poly;      /* list of chains that represent large polygons. */
   int numPoly;         /* number of element in poly. */
   double *polyData;    /* Data values for each poly (no missing values) */
   char EsriName[11];   /* element name shortened to 11 characters */
   int len;             /* Length of desired element name. */

   nameLen = strlen (Filename);
   if (nameLen < 4) {
      errSprintf ("ERROR: File %s is too short in length (it may need an "
                  "extension?)", Filename);
      return -2;
   }
   filename = (char *) malloc (nameLen + 1 * sizeof (char));
   strncpy (filename, Filename, nameLen);
   filename[nameLen] = '\0';

   /* Check that gds is valid before setting up map projection. */
   if (GDSValid (&meta->gds) != 0) {
      preErrSprintf ("ERROR: Sect3 was not Valid.\n");
      free (filename);
      return -3;
   }
   /* Set up the map projection. */
   SetMapParamGDS (&map, gds);
   /* Create the .shp/.shx files */
   if (f_poly == 1) {   /* Small poly */
      dp = (LatLon *) malloc ((gds->Nx + 1) * (gds->Ny + 1) *
                              sizeof (LatLon));
      /* Compute the lat/lon grid. */
      GridCompute (&map, gds, dp, 1, LatLon_Decimal);
      if (CreateShpPoly (filename, dp, gds->Nx, gds->Ny, f_nMissing,
                         grib_Data, &(meta->gridAttrib), LatLon_Decimal)
          != 0) {
         free (dp);
         free (filename);
         return -4;
      }
      free (dp);
   } else if (f_poly == 2) { /* Big poly */
      NewPolys (&poly, &numPoly);

      /* Convert the grid to a list of polygon chains. */
      /* Assumes data came from ParseGrid() so scan flag == "0100". */
      Grid2BigPoly (&poly, &numPoly, gds->Nx, gds->Ny, grib_Data);

      /* Following removes all "missing" values from the chains. */
      gribCompactPolys (poly, &numPoly, f_nMissing, &(meta->gridAttrib),
                        &polyData);

      /* Following is for experimenting with dateline issue. */
      ConvertChain2LtLn (poly, numPoly, &map, LatLon_Decimal);

      /* Following saves the chain of lat/lons to a shp/shx file. */
      if (CreateBigPolyShp (filename, poly, numPoly) != 0) {
         FreePolys (poly, numPoly);
         free (filename);
         return -4;
      }

      FreePolys (poly, numPoly);
   } else {             /* point */
#ifdef TEST
      dp = (LatLon *) malloc (gds->numPts * (sizeof (LatLon)));
      /* Compute the lat/lon grid. */
      GridCompute (&map, gds, dp, 0, LatLon_Decimal);
#endif
      if (CreateShpPnt (filename, &map, gds, LatLon_Decimal, f_nMissing,
                        grib_Data, &(meta->gridAttrib)) != 0) {
#ifdef TEST
         free (dp);
#endif
         free (filename);
         return -4;
      }
#ifdef TEST
      free (dp);
#endif
   }

   /* Create the .prj file */
   if (CreatePrj (filename, gds) != 0) {
      free (filename);
      return -6;
   }

   /* Since ESRI was lazy, we null terminate the .dbf column name.  Excel
    * doesn't need this, but ESRI does. */
   strncpy (EsriName, meta->element, 10);
   len = strlen (meta->element);
   if (len > 10) {
      if (strncmp (meta->element, "Prob", 4) == 0) {
         EsriName[0] = 'P';
         if ((len > 13) && (strncmp (meta->element + 4, "Wind", 4) == 0)) {
            EsriName[1] = 'w';
            strncpy (EsriName + 2, meta->element + 6, 10);
         } else {
            strncpy (EsriName + 1, meta->element + 4, 10);
         }
      }
   }
   EsriName[10] = '\0';


   /* Create the .dbf files */
   if (strcmp (EsriName, "Wx") == 0) {
      if (f_poly == 2) {
         if (CreateWxDbf (filename, numPoly, 1, polyData, EsriName,
                          f_nMissing, &(meta->gridAttrib),
                          (sect2_WxType *) &(meta->pds2.sect2.wx), 0,
                          &map) != 0) {
            free (filename);
            free (polyData);
            return -5;
         }
         free (polyData);
      } else {
         if (CreateWxDbf (filename, gds->Nx, gds->Ny, grib_Data,
                          EsriName, f_nMissing, &(meta->gridAttrib),
                          (sect2_WxType *) &(meta->pds2.sect2.wx),
                          f_verbose, &map) != 0) {
            free (filename);
            return -5;
         }
      }
   } else if (strcmp (EsriName, "WWA") == 0) {
      if (f_poly == 2) {
         if (CreateWWADbf (filename, numPoly, 1, polyData, EsriName,
                          f_nMissing, &(meta->gridAttrib),
                          (sect2_HazardType *) &(meta->pds2.sect2.hazard), 0,
                          &map) != 0) {
            free (filename);
            free (polyData);
            return -5;
         }
         free (polyData);
      } else {
         if (CreateWWADbf (filename, gds->Nx, gds->Ny, grib_Data,
                          EsriName, f_nMissing, &(meta->gridAttrib),
                          (sect2_HazardType *) &(meta->pds2.sect2.hazard),
                          f_verbose, &map) != 0) {
            free (filename);
            return -5;
         }
      }
   } else {
      max = meta->gridAttrib.max;
      min = meta->gridAttrib.min;
      switch (meta->gridAttrib.f_miss) {
         case 2:
            if (max < meta->gridAttrib.missSec) {
               max = meta->gridAttrib.missSec;
            } else if (min > meta->gridAttrib.missSec) {
               min = meta->gridAttrib.missSec;
            }
            /* Intentionally fall through. */
         case 1:
            if (max < meta->gridAttrib.missPri) {
               max = meta->gridAttrib.missPri;
            } else if (min > meta->gridAttrib.missPri) {
               min = meta->gridAttrib.missPri;
            }
      }
      if (max >= 1) {
         fieldLen1 = decimal + 1 + (int) (1 + log10 (max));
      } else if (max >= 0) {
         fieldLen1 = decimal + 1 + 1;
      } else if (max > -1) {
         fieldLen1 = decimal + 1 + 1 + 1;
      } else {
         fieldLen1 = decimal + 1 + (int) (1 + log10 (-1 * max)) + 1;
      }
      if (min >= 1) {
         fieldLen2 = decimal + 1 + (int) (1 + log10 (min));
      } else if (min >= 0) {
         fieldLen2 = decimal + 1 + 1;
      } else if (min > -1) {
         fieldLen2 = decimal + 1 + 1 + 1;
      } else {
         fieldLen2 = decimal + 1 + (int) (1 + log10 (-1 * min)) + 1;
      }
      if (fieldLen1 > fieldLen2) {
         if (f_poly == 2) {
            if (CreateDbf (filename, numPoly, 1, polyData, EsriName,
                           fieldLen1, decimal, f_nMissing,
                           &(meta->gridAttrib), 0, &map) != 0) {
               free (filename);
               free (polyData);
               return -5;
            }
            free (polyData);
         } else {
            if (CreateDbf (filename, gds->Nx, gds->Ny, grib_Data,
                           EsriName, fieldLen1, decimal, f_nMissing,
                           &(meta->gridAttrib), f_verbose, &map) != 0) {
               free (filename);
               return -5;
            }
         }
      } else {
         if (f_poly == 2) {
            if (CreateDbf (filename, numPoly, 1, polyData, EsriName,
                           fieldLen2, decimal, f_nMissing,
                           &(meta->gridAttrib), 0, &map) != 0) {
               free (filename);
               free (polyData);
               return -5;
            }
            free (polyData);
         } else {
            if (CreateDbf (filename, gds->Nx, gds->Ny, grib_Data,
                           EsriName, fieldLen2, decimal, f_nMissing,
                           &(meta->gridAttrib), f_verbose, &map) != 0) {
               free (filename);
               return -5;
            }
         }
      }
   }

   /* Create the .ave file */
   strncpy (filename + nameLen - 3, "ave", 3);
   orient = gds->orientLon;
   while (orient > 180) {
      orient -= 360;
   }
   while (orient < -180) {
      orient += 360;
   }
   if (gribWriteEsriAve (filename, gds, orient) != 0) {
      preErrSprintf ("gribWriteEsriAve had the following error\n");
      free (filename);
      return 1;
   }
   free (filename);
   return 0;
}
