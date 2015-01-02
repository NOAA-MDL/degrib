/*****************************************************************************
 * shpfile.c
 *
 * DESCRIPTION
 *    This file contains some functions to save various types of ESRI
 * shapefiles
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libaat.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

enum { SHPT_NULL, SHPT_POINT, SHPT_ARC = 3, SHPT_POLYGON = 5,
   SHPT_MULTIPOINT = 8, SHPT_POINTZ = 11, SHPT_ARCZ = 13,
   SHPT_POLYGONZ = 15, SHPT_MULTIPOINTZ = 18, SHPT_POINTM = 21,
   SHPT_ARCM = 23, SHPT_POLYGONM = 25, SHPT_MULTIPOINTM = 28,
   SHPT_MULTIPATCH = 31
};

/*****************************************************************************
 * shpCreatePnt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This creates a POINT .shp/.shx file.  The .shp/.shx file contains the
 * lat/lon values of a given vector as points.
 *    If one wants to skip points then create a new vector which points to
 * just the desired points.  This is more memory efficient than passing in a
 * mask vector.
 *    Does NOT check that the lat/lon are in correct range.  User may want to
 * call "myCyclicBounds()"
 *
 * ARGUMENTS
 * Filename = Name of file to save to. (Output)
 *       dp = Vector of lat/lon pairs. (Input)
 *    numDP = number of pairs in dp. (Input)
 *
 * RETURNS: int
 *  0 = OK
 * -1 = Memory allocation error.
 * -2 = Opening either .shp or .shx file
 * -3 = Problems writing entire .shp or .shx file
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *    Doesn't inter-twine the write to files, as that could cause the hard
 * drive to slow down.
 ****************************************************************************/
int shpCreatePnt(const char *Filename, const LatLon *dp, size_t numDP)
{
   char *filename;      /* Local copy of the name of the file. */
   FILE *sfp;           /* The open file pointer for .shp */
   FILE *xfp;           /* The open file pointer for .shx */
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   size_t recLen;       /* Length in bytes of a record in the .shp file. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   /* Spatial bounds of the data. minLon, minLat, maxLon, maxLat, ... */
   double Bounds[] = { 0., 0., 0., 0., 0., 0., 0., 0. };
   sInt4 curRec[2];     /* rec number, and content length. */
   size_t i;            /* Loop counter over number of points. */

   /* Open the files. */
   if (fileAllocNewExten(Filename, ".shp", &filename) != 0) {
      return -1;
   }
   if ((sfp = fopen(filename, "wb")) == NULL) {
      myWarn_Err2Arg("Problems opening %s for write.\n", filename);
      free(filename);
      return -2;
   }
   filename[strlen(filename) - 1] = 'x';
   if ((xfp = fopen(filename, "wb")) == NULL) {
      myWarn_Err2Arg("Problems opening %s for write.\n", filename);
      free(filename);
      fclose(sfp);
      return -2;
   }

   /* Start Writing header in first 100 bytes. */
   Head1[0] = 9994;     /* ArcView identifier. */
   /* set 5 unused to 0 */
   memset((Head1 + 1), 0, 5 * sizeof(sInt4));
   recLen = sizeof(sInt4) + 2 * sizeof(double);
   /* .shp file size (in 2 byte words) */
   Head1[6] = (100 + (2 * sizeof(sInt4) + recLen) * numDP) / 2;
   FWRITE_BIG(Head1, sizeof(sInt4), 7, sfp);
   Head2[0] = 1000;     /* ArcView version identifier. */
   Head2[1] = SHPT_POINT; /* Signal that these are point data. */
   FWRITE_LIT(Head2, sizeof(sInt4), 2, sfp);
   /* Write out initial guess for bounds... Need to revisit. */
   FWRITE_LIT(Bounds, sizeof(double), 8, sfp);

   /* Start Writing data. */
   curRec[0] = 1;
   curRec[1] = recLen / 2; /* Content length in (2 byte words) */
   Bounds[0] = Bounds[2] = dp->lon;
   Bounds[1] = Bounds[3] = dp->lat;
   for (i = 0; i < numDP; ++i) {
      FWRITE_BIG(curRec, sizeof(sInt4), 2, sfp);
      FWRITE_LIT(&(Head2[1]), sizeof(sInt4), 1, sfp);
      FWRITE_LIT(&(dp->lon), sizeof(double), 1, sfp);
      FWRITE_LIT(&(dp->lat), sizeof(double), 1, sfp);
      /* Update Bounds. */
      if (dp->lon < Bounds[0]) {
         Bounds[0] = dp->lon;
      } else if (dp->lon > Bounds[2]) {
         Bounds[2] = dp->lon;
      }
      if (dp->lat < Bounds[1]) {
         Bounds[1] = dp->lat;
      } else if (dp->lat > Bounds[3]) {
         Bounds[3] = dp->lat;
      }
      ++curRec[0];
      ++dp;
   }
   /* Store the updated Bounds (only 4 (not 8) of them). */
   fseek(sfp, 36, SEEK_SET);
   FWRITE_LIT(Bounds, sizeof(double), 4, sfp);
   fflush(sfp);

   /* Check that .shp is now the correct file size. */
   fseek(sfp, 0L, SEEK_END);
   if (ftell(sfp) != Head1[6] * 2) {
      filename[strlen(filename) - 1] = 'p';
      myWarn_Err3Arg("shp file %s is not %ld bytes long.\n", filename,
                     Head1[6] * 2);
      free(filename);
      fclose(sfp);
      fclose(xfp);
      return -3;
   }
   fclose(sfp);

   /* Write ArcView header (.shx file). */
   Head1[6] = (100 + 8 * numDP) / 2; /* shx file size (in words). */
   FWRITE_BIG(Head1, sizeof(sInt4), 7, xfp);
   FWRITE_LIT(Head2, sizeof(sInt4), 2, xfp);
   FWRITE_LIT(Bounds, sizeof(double), 8, xfp);
   curRec[0] = 50;      /* 100 bytes / 2 = 50 words */
   curRec[1] = recLen / 2; /* Content length in words (2 bytes) */
   for (i = 0; i < numDP; ++i) {
      FWRITE_BIG(curRec, sizeof(sInt4), 2, xfp);
      curRec[0] += (recLen + 2 * sizeof(sInt4)) / 2; /* (2 byte words) */
   }
   fflush(xfp);

   /* Check that .shx is now the correct file size. */
   fseek(xfp, 0L, SEEK_END);
   if (ftell(xfp) != 100 + 8 * numDP) {
      myWarn_Err3Arg("shx file '%s' is not %ld bytes long.\n", filename,
                     100 + 8 * numDP);
      free(filename);
      fclose(xfp);
      return -3;
   }
   free(filename);
   fclose(xfp);
   return 0;
}

/* Following should take function for xy2ll */
/* int shpCreateGridLatice();*/
/* Following needs to handle the dateline */
/* int shpCreateGridPoly();*/

int shpCreatePrj(const char *Filename, const char *gcs, const char *datum,
                 const char *spheroid, double A, double B)
{
   char *filename;
   FILE *fp;
   double invFlat;

   /* Open the files. */
   if (fileAllocNewExten(Filename, ".prj", &filename) != 0) {
      return -1;
   }
   if ((fp = fopen(filename, "wb")) == NULL) {
      myWarn_Err2Arg("Problems opening %s for write.\n", filename);
      free(filename);
      return -2;
   }
   invFlat = A / (A - B);
   fprintf(fp, "GEOGCS[\"%s\",DATUM[\"%s\",SPHEROID[\"%s\",%.1f,%.13f]],"
           "PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]",
           gcs, datum, spheroid, A, invFlat);
   fclose(fp);
   free(filename);
   return 0;
}
