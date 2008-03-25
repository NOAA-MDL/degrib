/*****************************************************************************
 * sector.c
 *
 * DESCRIPTION
 *    This file contains all the routines used to handle the -Sector option,
 * which determines which sector(s) a given point is in.
 *
 * HISTORY
 *   1/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "myerror.h"
#include "myassert.h"
#include "myutil.h"
#include "mymapf.h"
#include "sector.h"
#include "meta.h"
#include "tendian.h"
#include "genprobe.h"
/*#include "scan.h"*/
#ifdef MEMWATCH
#include "memwatch.h"
#endif

static const gdsType NdfdDefGds[] = {
   {739297, 30, 1, 6371.2, 6371.2,
    1073, 689, 20.191999, 238.445999, 265.000000,
    5079.406000, 5079.406000, 25.000000,
    0, 0, 64, 0, 0,
    25.000000, 25.000000, -90, 0, 0, 0, 0, 0, 0},
   {22833, 10, 1, 6371.2, 6371.2,
    177, 129, 16.828691, 291.804687, 0.000000,
    2500.000000, 2500.000000, 20.000000,
    0, 0, 64, 19.747399, 296.027600,
    0, 0, 0, 0, 0, 0, 0, 0, 0},
   {72225, 10, 1, 6371.2, 6371.2,
    321, 225, 18.066780, 198.374755, 0.000000,
    2500.000000, 2500.000000, 20.000000,
    0, 0, 64, 23.082000, 206.031000,
    0, 0, 0, 0, 0, 0, 0, 0, 0},
   {37249, 10, 1, 6371.2, 6371.2,
    193, 193, 12.349882, 143.686538, 0.000000,
    2500.000000, 2500.000000, 20.000000,
    0, 0, 64, 16.794399, 148.280000,
    0, 0, 0, 0, 0, 0, 0, 0, 0},
   {456225, 20, 1, 6371.2, 6371.2,
    825, 553, 40.530101, 181.429000, 210.000000,
    5953.125000, 5953.125000, 60.000000,
    0, 0, 64, 0, 0,
    90.000000, 90.000000, 0, 0, 0, 0, 0, 0, 0},
   {1509825, 30, 1, 6371.2, 6371.2,
    1473, 1025, 12.1899999, 226.541, 265.000000,
    5079.406, 5079.406, 25.00,
    0, 0, 64, 0, 0,
    25.000000, 25.000000, -90, 0, 0, 0, 0, 0, 0}
};

static const char *NdfdDefSect[] = {
   "conus", "puertori", "hawaii", "guam", "alaska", "nhemi"
};

static size_t NumNdfdDefSect = sizeof (NdfdDefGds) / sizeof (NdfdDefGds[0]);

int SectorFindGDS (gdsType *gds)
{
   size_t i;            /* loop counter. */

   for (i = 0; i < NumNdfdDefSect; i++) {
      if (gds->numPts != NdfdDefGds[i].numPts)
         continue;
      if (gds->projType != NdfdDefGds[i].projType)
         continue;
      if (gds->f_sphere != NdfdDefGds[i].f_sphere)
         continue;
      if (fabs (gds->majEarth - NdfdDefGds[i].majEarth) > 0.1)
         continue;
      if (fabs (gds->minEarth - NdfdDefGds[i].minEarth) > 0.1)
         continue;
      if (gds->Nx != NdfdDefGds[i].Nx)
         continue;
      if (gds->Ny != NdfdDefGds[i].Ny)
         continue;

      /* Guam uncertainty in the lat1 is high.  Trust only 5 decimals */
      if (fabs (gds->lat1 - NdfdDefGds[i].lat1) > 0.1) /* Only took out 1 zero */
         continue;
      if (fabs (gds->lon1 - NdfdDefGds[i].lon1) > 0.1)
         continue;
      if (fabs (gds->orientLon - NdfdDefGds[i].orientLon) > 0.1)
         continue;

      /* Alaska uncertainty in the DX is high.  Trust only 0 decimals */
      /* This is because of the 9/22/2006 correction from DX = 5953.000
       * to 5953.125 */
      if (fabs (gds->Dx - NdfdDefGds[i].Dx) > 1)
         continue;
      if (fabs (gds->Dy - NdfdDefGds[i].Dy) > 1)
         continue;
      if (fabs (gds->meshLat - NdfdDefGds[i].meshLat) > 0.1)
         continue;

/*    Commented out due to RTMA data coming in with gds->resFlag = 8)
      if (gds->resFlag != NdfdDefGds[i].resFlag)
         continue;
*/
      if (gds->center != NdfdDefGds[i].center)
         continue;
      if (gds->scan != NdfdDefGds[i].scan)
         continue;
      if (fabs (gds->lat2 - NdfdDefGds[i].lat2) > 0.1)
         continue;
      if (fabs (gds->lon2 - NdfdDefGds[i].lon2) > 0.1)
         continue;
      if (fabs (gds->scaleLat1 - NdfdDefGds[i].scaleLat1) > 0.1)
         continue;
      if (fabs (gds->scaleLat2 - NdfdDefGds[i].scaleLat2) > 0.1)
         continue;

/*    Commented out due to RTMA data coming in with the diff below = 90)
      if (fabs (gds->southLat - NdfdDefGds[i].southLat) > 0.1)
         continue;
*/
      if (fabs (gds->southLon - NdfdDefGds[i].southLon) > 0.1)
         continue;

/* AngleRotate, poleLat, poleLon, stretchFactor, f_typeLatLon are
 * not stored in the data cube (they are not defined in ReadGDSBuffer
 * routine in database.c).
 */
/*
      if (gds->angleRotate != NdfdDefGds[i].angleRotate)
         continue;
      if (fabs (gds->poleLat - NdfdDefGds[i].poleLat) > 0.1)
         continue;
      if (fabs (gds->poleLon - NdfdDefGds[i].poleLon) > 0.1)
         continue;
      if (fabs (gds->stretchFactor - NdfdDefGds[i].stretchFactor) > 0.1)
         continue;
      if (gds->f_typeLatLon != NdfdDefGds[i].f_typeLatLon)
         continue;
*/
      return i;
   }
   return -1;
}

/*****************************************************************************
 * FillGDSBuffer() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Fill a GDS from a char buffer list.
 *
 * ARGUMENTS
 *     gds = GDS Structure to fill. (Output)
 *    list = The list to read elements from. (Input)
 * numList = Number of elements in list. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   1/2005 Arthur Taylor (MDL): Created
 *
 * NOTES
 *****************************************************************************
 */
static void FillGDSBuffer (gdsType *gds, char **list, int numList)
{
   myAssert (list != NULL);
   myAssert (gds != NULL);
   myAssert (numList == 17);
   myAssert (sizeof (double) == 8);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (char) == 1);

   gds->projType = atoi (list[1]);
   gds->Nx = atoi (list[2]);
   gds->Ny = atoi (list[3]);
   gds->numPts = gds->Nx * gds->Ny;
   gds->lat1 = atof (list[4]);
   gds->lon1 = atof (list[5]);
   gds->orientLon = atof (list[6]);
   gds->Dx = atof (list[7]);
   gds->Dy = atof (list[8]);
   gds->meshLat = atof (list[9]);
   gds->resFlag = 0;
   gds->center = 0;
   gds->scan = atoi (list[10]);
   gds->lat2 = atof (list[11]);
   gds->lon2 = atof (list[12]);
   gds->scaleLat1 = atof (list[13]);
   gds->scaleLat2 = atof (list[14]);
   gds->majEarth = atof (list[15]) / 1000.;
   gds->minEarth = atof (list[16]) / 1000.;
   if (gds->majEarth == gds->minEarth)
      gds->f_sphere = 1;
   else
      gds->f_sphere = 0;
   gds->southLat = -90;
   gds->southLon = 0;
}

/*****************************************************************************
 * isPntInASector() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Determines if a lat/lon point is inside any NDFD sector.
 *
 * ARGUMENTS
 *      pnt = The point in question. (Input)
 *
 * RETURNS: int
 * 0 = outside all grids
 * 1 = inside a grid.
 *
 * HISTORY
 *   3/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
int isPntInASector (Point pnt)
{
   size_t i;            /* loop counter. */
   const gdsType *gdsPtr; /* ptr to current default grid definition section. */
   myMaparam map;       /* The map projection to use with this sector. */
   double x;            /* The converted X value. */
   double y;            /* The converted Y value. */

   for (i = 0; i < NumNdfdDefSect; i++) {
      gdsPtr = NdfdDefGds + i;
#ifdef DEBUG
      if (GDSValid (gdsPtr) != 0) {
         preErrSprintf ("Problems with default grid %d\n", i);
         return -3;
      }
#endif
      SetMapParamGDS (&map, gdsPtr);
      myCll2xy (&map, pnt.Y, pnt.X, &x, &y);
      x -= .5;
      y -= .5;
      /* Why the > 0 instead of >= 0? */
      if ((x > 0) && (x <= gdsPtr->Nx) && (y > 0) && (y <= gdsPtr->Ny)) {
         return 1;
      }
   }
   return 0;
}

/*****************************************************************************
 * WhichSector() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Looks through sectFile, and outputs any sectors which the point falls
 * inside.
 *
 * ARGUMENTS
 * sectFile = Contains the grid specs for the sectors. (Input)
 *      pnt = The point in question. (Input)
 *  f_cells = 0 print just the sectors the point (lat/lon) falls in.
 *            1 print the lat,lon,x,y,In/Out for the point (y/x).
 *            2 print the lat,lon,x,y,In/Out for the point (lat/lon).
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems opening the sectFile
 * -2 = Problem with a line in sectFile.
 * -3 = Problem with a line as a grid definition section.
 *
 * HISTORY
 *   1/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
int WhichSector (char *sectFile, Point pnt, sChar f_cells)
{
   FILE *fp;            /* The opened sector file. */
   char *line = NULL;   /* Each line in the file. */
   size_t lineLen = 0;  /* The length of the line. */
   char **list = NULL;  /* Each Line broken up by the ':' */
   size_t numList = 0;  /* number of ':' in a line. */
   size_t i;            /* loop counter. */
   gdsType gds;         /* The grid definition section for this sector. */
   const gdsType *gdsPtr; /* ptr to current default grid definition section. */
   myMaparam map;       /* The map projection to use with this sector. */
   double x;            /* The converted X value. */
   double y;            /* The converted Y value. */
   int err;             /* The value of our error. */
   int f_inside;        /* True if the point is inside the sector. */

   switch (f_cells) {
      case 0:
         break;
      case 1:
      case 2:
         printf ("Sector, lat, lon, X, Y, InSector\n");
         break;
      default:
         errSprintf ("cells '%d' is invalid.  Not 0,1,2\n");
         return -4;
   }
   if (sectFile == NULL) {
      for (i = 0; i < NumNdfdDefSect; i++) {
         gdsPtr = NdfdDefGds + i;
#ifdef DEBUG
         if (GDSValid (gdsPtr) != 0) {
            preErrSprintf ("Problems with default grid %d\n", i);
            return -3;
         }
#endif
         SetMapParamGDS (&map, gdsPtr);
         if (f_cells == 0) {
            myCll2xy (&map, pnt.Y, pnt.X, &x, &y);
            x -= .5;
            y -= .5;
            /* Why the > 0 instead of >= 0? */
            if ((x > 0) && (x <= gdsPtr->Nx) && (y > 0) && (y <= gdsPtr->Ny)) {
               printf ("%s\n", NdfdDefSect[i]);
            }
         } else if (f_cells == 1) {
            myCxy2ll (&map, pnt.X, pnt.Y, &y, &x);
            /* Why the > 0 instead of >= 0? */
            f_inside = ((pnt.X - .5 > 0) && (pnt.Y - .5 > 0) &&
                        (pnt.X - .5 <= gdsPtr->Nx) &&
                        (pnt.Y - .5 <= gdsPtr->Ny));
            printf ("%s, %f, %f, %f, %f, %d\n", NdfdDefSect[i],
                    y, x, pnt.X, pnt.Y, f_inside);
         } else {
            myCll2xy (&map, pnt.Y, pnt.X, &x, &y);
            /* Why the > 0 instead of >= 0? */
            f_inside = ((x - .5 > 0) && (x - .5 <= gdsPtr->Nx) &&
                        (y - .5 > 0) && (y - .5 <= gdsPtr->Ny));
            printf ("%s, %f, %f, %f, %f, %d\n", NdfdDefSect[i],
                    pnt.Y, pnt.X, x, y, f_inside);
         }
      }
      return 0;
   }
   if ((fp = fopen (sectFile, "rt")) == NULL) {
      errSprintf ("Couldn't open %s\n", sectFile);
      return -1;
   }
   while (reallocFGets (&line, &lineLen, fp) > 0) {
      strTrim (line);
      if ((lineLen <= 0) || (line[0] == '#')) {
         continue;
      }
      mySplit (line, ':', &numList, &list, 1);
      if (numList != 17) {
         errSprintf ("Expecting 17 fields in '%s', instead of %d fields.\n"
                     "Should have field headers of:\n"
                     "name: Type: Nx: Ny: Lat1: Lon1: OrientLon:\n"
                     "Dx: Dy: MeshLat: Scan: Lat2: Lon2: ScaleLat1:\n"
                     "ScaleLat2: RadiusMax: RadiusMin: path\n",
                     line, numList);
         err = -2;
         goto error;
      }
      FillGDSBuffer (&gds, list, numList);
      if (GDSValid (&gds) != 0) {
         preErrSprintf ("'%s' is an invalid Grid Definition "
                        "Section\n", line);
         err = -3;
         goto error;
      }

      SetMapParamGDS (&map, &gds);
      if (f_cells == 0) {
         myCll2xy (&map, pnt.Y, pnt.X, &x, &y);
         x -= .5;
         y -= .5;
         /* Why the > 0 instead of >= 0? */
         if ((x > 0) && (x <= gds.Nx) && (y > 0) && (y <= gds.Ny)) {
            printf ("%s\n", list[0]);
         }
      } else if (f_cells == 1) {
         myCxy2ll (&map, pnt.X, pnt.Y, &y, &x);
         /* Why the > 0 instead of >= 0? */
         f_inside = ((pnt.X - .5 > 0) && (pnt.X - .5 <= gds.Nx) &&
                     (pnt.Y - .5 > 0) && (pnt.Y - .5 <= gds.Ny));
         printf ("%s, %f, %f, %f, %f, %d\n", list[0],
                 y, x, pnt.X, pnt.Y, f_inside);
      } else {
         myCll2xy (&map, pnt.Y, pnt.X, &x, &y);
         /* Why the > 0 instead of >= 0? */
         f_inside = ((x - .5 > 0) && (x - .5 <= gds.Nx) &&
                     (y - .5 > 0) && (y - .5 <= gds.Ny));
         printf ("%s, %f, %f, %f, %f, %d\n", list[0],
                 pnt.Y, pnt.X, x, y, f_inside);
      }

      for (i = 0; i < numList; i++) {
         free (list[i]);
      }
      free (list);
      list = NULL;
      numList = 0;
   }
   free (line);
   fclose (fp);
   return 0;
 error:
   for (i = 0; i < numList; i++) {
      free (list[i]);
   }
   free (list);
   free (line);
   fclose (fp);
   return err;
}

/* f_first is first time through loop, so init "index" */
static int FillOutInfo (const gdsType *gds, const char *sectName,
                        sChar f_first, sChar f_sector, size_t numPnts,
                        Point * pnts, sChar f_cells, const char *geoDataDir,
                        PntSectInfo * pntInfo, size_t *NumSect, char ***Sect)
{
   int j;            /* loop counter. */
   int k;
   myMaparam map;       /* The map projection to use with this sector. */
   sChar f_foundOne;
   size_t curSect = 0;
   char *fileName = NULL;
   FILE *tzFP = NULL;
   FILE *dayFP = NULL;
   sInt4 size;
   char perm;
   sInt4 offset;
   double x;            /* The converted X value. */
   double y;            /* The converted Y value. */
   sInt4 x1, y1;
   float value;

   f_foundOne = 0;
   /* Don't need map set up for f_cells = 1. */
   if (f_cells != 1) {
      SetMapParamGDS (&map, gds);
   }

   for (j = 0; j < numPnts; j++) {
      /* Check if we've already found this point, or init it to -1 if this is
       * the first sector we've looked at. */
      if (f_first) {
         pntInfo[j].numSector = 0;
         for (k = 0; k < NDFD_OCONUS_UNDEF; k++) {
            pntInfo[j].f_sector[k] = NDFD_OCONUS_UNDEF;
         }
/*
      Don't need this continue here since we want to check all point in
      all sectors to complete the pntInfo structure. 
      } else if (pntInfo[j].numSector > 1) {
      } else if (pntInfo[j].index != -1) {
         continue;
*/
      }
      if (f_cells != 1) {
         myCll2xy (&map, pnts[j].Y, pnts[j].X, &x, &y);
         /* Get x1, y1 as integers in range of 1,NX 1,NY */
         x1 = (sInt4) (x + .5);
         y1 = (sInt4) (y + .5);
      } else {
         /* Get x1, y1 in as integers in range of 1,NX 1,NY */
         x1 = (sInt4) (pnts[j].X + .5);
         y1 = (sInt4) (pnts[j].Y + .5);
      }
      if ((x1 >= 1) && (x1 <= (sInt4) gds->Nx) && (y1 >= 1)
          && (y1 <= (sInt4) gds->Ny)) {
         pntInfo[j].f_sector[pntInfo[j].numSector] = f_sector;
         pntInfo[j].numSector++;
         /* The following if test forces a point to only count (when
          * considering the Sector list) in the first sector it is found in.
          * A point could fall in conus,nhemi,alaska, but we only want to add
          * conus (the first one) to the list.
          * This protects against conus/alaska confusion.
          * For conus/nhemi, before we expand the file list, we
          * automatically add nhemi if conus is there, and nhemi is not
          * so it is handled there. */
         if (pntInfo[j].numSector == 1) {
            /* Check if we'd found any point in this sector before */
            if (!f_foundOne) {
               curSect = *NumSect;
               *NumSect = *NumSect + 1;
               *Sect = (char **) realloc (*Sect, *NumSect * sizeof (char *));
               (*Sect)[curSect] = (char *) malloc (strlen (sectName) + 1);
               strcpy ((*Sect)[curSect], sectName);
               if (geoDataDir != NULL) {
                  fileName =
                        (char *) malloc (strlen (geoDataDir) + 1 +
                                         strlen (sectName)
                                         + strlen ("timezone.flt") + 1);
                  sprintf (fileName, "%s/%stimezone.flt", geoDataDir, sectName);
               /* Use myStat to check size / exists / file / perms */
                  if (MYSTAT_ISFILE == myStat (fileName, &perm, &size, NULL)) {
                     if ((size == (sInt4) (gds->Nx * gds->Ny * 4)) && (perm & 4)) {
                        tzFP = fopen (fileName, "rb");
                     } else {
                        free (fileName);
                        errSprintf ("timezone file '%s' is wrong size or "
                                    "unreadable\n", fileName);
                        return -4;
                     }
                  }
                  sprintf (fileName, "%s/%sdaylight.flt", geoDataDir, sectName);
               /* Use myStat to check size / exists / file / perms */
                  if (MYSTAT_ISFILE == myStat (fileName, &perm, &size, NULL)) {
                     if ((size == (sInt4) (gds->Nx * gds->Ny * 4)) && (perm & 4)) {
                        dayFP = fopen (fileName, "rb");
                     } else {
                        free (fileName);
                        fclose (tzFP);
                        errSprintf ("daylight file '%s' is wrong size or "
                                    "unreadable\n", fileName);
                        return -4;
                     }
                  }
                  free (fileName);
               }
               f_foundOne = 1;
            }
            if ((dayFP != NULL) && (tzFP != NULL)) {
            /* GRIB2BIT_2 is because flt is stored in that scan mode */
            /* XY2ScanIndex (&offset, x1, y1, GRIB2BIT_2, gds->Nx, gds->Ny); */
               offset = (x1 - 1) + (y1 - 1) * gds->Nx;

            /* because we're dealing in bytes, we need to multiply the row
             * that we got out of ScanIndex by 4 */
               offset = offset * sizeof (float);

               fseek (tzFP, offset, SEEK_SET);
               FREAD_LIT (&value, sizeof (float), 1, tzFP);
            /* timezone contains # hours to add to UTC to get local time.  */
            /* pntInfo contains # hours to add to local time to get UTC. */
               pntInfo[j].timeZone = (sChar) (-1 * value);
               fseek (dayFP, offset, SEEK_SET);
               FREAD_LIT (&value, sizeof (float), 1, dayFP);
               pntInfo[j].f_dayLight = (sChar) (value);
            } else {
               pntInfo[j].timeZone = 0;
               pntInfo[j].f_dayLight = 0;
            }
         }
      }
   }
   if (dayFP != NULL)
      fclose (dayFP);
   if (tzFP != NULL)
      fclose (tzFP);
   return 0;
}

int GetSectorList (char *sectFile, size_t numPnts, Point * pnts,
                   sChar f_cells, const char *geoDataDir,
                   PntSectInfo * pntInfo, size_t *NumSect, char ***Sect)
{
   FILE *fp;            /* The opened sector file. */
   char *line = NULL;   /* Each line in the file. */
   size_t lineLen = 0;  /* The length of the line. */
   char **list = NULL;  /* Each Line broken up by the ':' */
   size_t numList = 0;  /* number of ':' in a line. */
   size_t i;            /* loop counter. */
   gdsType gds;         /* The grid definition section for this sector. */
   int err;             /* The value of our error. */
   int lineCnt;
   sChar f_sector;

   myAssert (*NumSect == 0);
   myAssert (*Sect == NULL);
   myAssert (pntInfo != NULL);

   if ((f_cells != 0) && (f_cells != 1) && (f_cells != 2)) {
      errSprintf ("cells '%d' is invalid.  Not 0,1,2\n");
      return -4;
   }

   if (sectFile == NULL) {
      for (i = 0; i < NumNdfdDefSect; i++) {
#ifdef DEBUG
         if (GDSValid (&(NdfdDefGds[i])) != 0) {
            preErrSprintf ("Problems with default grid %d\n", i);
            return -3;
         }
#endif
         if (FillOutInfo (&(NdfdDefGds[i]), NdfdDefSect[i], (i == 0), i,
                          numPnts, pnts, f_cells, geoDataDir, pntInfo,
                          NumSect, Sect) != 0) {
            return -4;
         }
      }
      return 0;
   }
   if ((fp = fopen (sectFile, "rt")) == NULL) {
      errSprintf ("Couldn't open %s\n", sectFile);
      return -1;
   }
   lineCnt = 0;
   while (reallocFGets (&line, &lineLen, fp) > 0) {
      strTrim (line);
      if ((lineLen <= 0) || (line[0] == '#')) {
         continue;
      }
      lineCnt++;
      mySplit (line, ':', &numList, &list, 1);
      if (numList != 17) {
         errSprintf ("Expecting 17 fields in '%s', instead of %d fields.\n"
                     "Should have field headers of:\n"
                     "name: Type: Nx: Ny: Lat1: Lon1: OrientLon:\n"
                     "Dx: Dy: MeshLat: Scan: Lat2: Lon2: ScaleLat1:\n"
                     "ScaleLat2: RadiusMax: RadiusMin: path\n",
                     line, numList);
         err = -2;
         goto error;
      }
      FillGDSBuffer (&gds, list, numList);
      if (GDSValid (&gds) != 0) {
         preErrSprintf ("'%s' is an invalid Grid Definition "
                        "Section\n", line);
         err = -3;
         goto error;
      }

      f_sector = SectorFindGDS (&gds);
      if (f_sector == -1) {
         f_sector = NDFD_OCONUS_UNDEF;
      }
      if (FillOutInfo (&gds, list[0], (lineCnt == 1), f_sector, numPnts,
                       pnts, f_cells, geoDataDir, pntInfo, NumSect,
                       Sect) != 0) {
         err = -4;
         goto error;
      }

      for (i = 0; i < numList; i++) {
         free (list[i]);
      }
      free (list);
      list = NULL;
      numList = 0;
   }
   free (line);
   fclose (fp);
   return 0;
 error:
   for (i = 0; i < numList; i++) {
      free (list[i]);
   }
   free (list);
   free (line);
   fclose (fp);
   return err;
}

/*
 *  numInNames        number of input files.
 *  inNames           inName = -in
 *  f_inTypes         file type from stat (1=dir, 2=file, 3=unknown).
 *         after procedure f_inType may not corespond with inNames...
 *         so free it?
 *  filter          (Input) used to filter the directory search
 */

/* Pass in the element list? */
void expandInName (size_t numInNames, char **inNames, char *f_inTypes,
                   char *filter, size_t numSect, char **sect,
                   sChar f_ndfdConven, size_t numElem, genElemDescript * elem,
                   size_t *NumOutNames, char ***OutNames)
{
   size_t i;
   size_t j;
   size_t ii;
   char *rootname;
   char perm;
   size_t numAns;
   char **ans;
   size_t valAns;
   const char *ptr;
   char *buffer;
   size_t lenInName;
   char f_usedRoot; /* True if we've included looking in the root directory */
   numAns = numInNames;
   ans = (char **) malloc (numAns * sizeof (char *));
   valAns = 0;

   myAssert (numElem != 0);
   for (i = 0; i < numInNames; i++) {
      /* If it is a directory, Glob it, and free it */
      if (f_inTypes[i] == MYSTAT_ISDIR) {
         lenInName = strlen (inNames[i]);
         f_usedRoot = 0;
         for (j = 0; j < numSect; j++) {
            /* Check if "root/sector is a directory */
            rootname = (char *) malloc (lenInName + strlen (sect[j]) + 2);
            sprintf (rootname, "%s/%s", inNames[i], sect[j]);
            if (myStat (rootname, &perm, NULL, NULL) == MYSTAT_ISDIR) {
               if ((perm & 4) == 0) {
                  rootname[lenInName] = '\0';
               }
            } else if (f_usedRoot) {
               /* We've already included the root directory, so don't include
                * it a second time. */
               free (rootname);
               continue;
            } else {
               f_usedRoot = 1;
               rootname[lenInName] = '\0';
            }
            /* Create file name, and check if it exists. */
            for (ii = 0; ii < numElem; ii++) {
               ptr = genNdfdEnumToStr (elem[ii].ndfdEnum, f_ndfdConven);
/*               myAssert (ptr != NULL);*/
               if (ptr != NULL) {
                  /* The extra + 3 is for the second test. */
                  buffer = (char *) malloc (strlen (rootname) + strlen (ptr) +
                                            strlen (filter) + 3 - 2 + 3);
                  sprintf (buffer, "%s/%s.%s", rootname, ptr, filter + 2);
                  if (myStat (buffer, &perm, NULL, NULL) == MYSTAT_ISFILE) {
                     if (perm & 4) {
                        /* If it is a readable file, give memory to ans */
                        if (valAns == numAns) {
                           numAns++;
                           ans = (char **) realloc (ans,
                                                    numAns * sizeof (char *));
                        }
                        ans[valAns] = buffer;
                        valAns++;
                        buffer = NULL;
                     }
                  } else {
                     /* Try second choice. */
                     sprintf (buffer, "%s/ds.%s.%s", rootname, ptr,
                              filter + 2);
                     if (myStat (buffer, &perm, NULL, NULL) == MYSTAT_ISFILE) {
                        if (perm & 4) {
                           /* If it is a readable file, give memory to ans */
                           if (valAns == numAns) {
                              numAns++;
                              ans = (char **) realloc (ans,
                                                       numAns *
                                                       sizeof (char *));
                           }
                           ans[valAns] = buffer;
                           valAns++;
                           buffer = NULL;
                        }
                     }
                  }
                  if (buffer != NULL) {
                     free (buffer);
                  }
               }
            }
            free (rootname);
         }
      } else {
         /* If it is a readable file, give a copy to ans */
         if (valAns == numAns) {
            numAns++;
            ans = (char **) realloc (ans, numAns * sizeof (char *));
         }
         ans[valAns] = (char *) malloc (strlen (inNames[i]) + 1);
         strcpy (ans[valAns], inNames[i]);
         valAns++;
      }
   }
   *OutNames = ans;
   *NumOutNames = valAns;
}

/*
 *  numInNames        number of input files.
 *  inNames           inName = -in
 *  f_inTypes         file type from stat (1=dir, 2=file, 3=unknown).
 *         after procedure f_inType may not corespond with inNames...
 *         so free it?
 *  filter          (Input) used to filter the directory search
 */

#ifdef OLD_CODE
/* Pass in the element list? */
void sectExpandInName (size_t *NumInNames, char ***InNames,
                       char **F_inTypes, const char *filter, size_t numSect,
                       char **sect, sChar f_ndfdConven, size_t numNdfdVars,
                       uChar *ndfdVars)
{
   size_t i;
   size_t j;
   size_t k;
   size_t ii;
   size_t argc = 0;
   char **argv = NULL;
   size_t numAns;
   size_t valAns;
   char **ans;
   char *buffer;
   char *rootname;
   char perm;
   const char *ptr;

   numAns = *NumInNames;
   ans = malloc (numAns * sizeof (char *));
   valAns = 0;

   for (i = 0; i < *NumInNames; i++) {
      /* If it is a directory, Glob it, and free it */
      if ((*F_inTypes)[i] == MYSTAT_ISDIR) {
         for (j = 0; j < numSect; j++) {

            rootname = malloc (strlen ((*InNames)[i]) + strlen (sect[j]) + 2);
            sprintf (rootname, "%s/%s", (*InNames)[i], sect[j]);
            /* Check if "root/sector is a directory */
            if (myStat (rootname, &perm, NULL, NULL) == MYSTAT_ISDIR) {
               /* check that it is readable */
               if (perm & 4) {
               } else {
                  strcpy (rootname, (*InNames)[i]);
               }
            } else {
               strcpy (rootname, (*InNames)[i]);
            }

            if (numNdfdVars == 0) {
               if (myGlob (rootname, filter, &argc, &argv) != 0) {
                  free (rootname);
#ifdef DEBUG
                  printf ("Warning: couldn't open directory %s", rootname);
#endif
                  continue;
               }
               free (rootname);
               if (argc == 0) {
                  continue;
               }
               /* could test numAns vs valAns + argc + *NumInNames - i or
                * something like that, prior to realloc but if I don't get
                * the equation right, could cause problems. */
               numAns += argc;
               ans = realloc (ans, numAns * sizeof (char *));
               for (k = 0; k < argc; k++) {
                  /* If it is a file... give memory control to ans */
                  if (myStat (argv[k], &perm, NULL, NULL) == MYSTAT_ISFILE) {
                     /* check that it is readable */
                     if (perm & 4) {
                        ans[valAns] = argv[k];
                        valAns++;
                     } else {
                        free (argv[k]);
                     }
                     /* If it is a directory... free it */
                  } else {
                     free (argv[k]);
                  }
               }
               argc = 0;
               free (argv);
               argv = NULL;
            } else {
               /* Could either ask for each file in directory (with filter),
                * am I interested, or for each var, create file name, and
                * check if it exists.  Chose the second choice. */
               for (ii = 0; ii < numNdfdVars; ii++) {
                  ptr = genNdfdEnumToStr (ndfdVars[ii], f_ndfdConven);
/*                  myAssert (ptr != NULL);*/
                  if (ptr != NULL) {
                     /* The extra + 3 is for the second test. */
                     buffer = malloc (strlen (rootname) + strlen (ptr) +
                                      strlen (filter) + 3 - 2 + 3);
                     sprintf (buffer, "%s/%s.%s", rootname, ptr, filter + 2);
                     /* If it is a file... give memory control to ans */
                     if (myStat (buffer, &perm, NULL, NULL) == MYSTAT_ISFILE) {
                        /* check that it is readable */
                        if (perm & 4) {
                           if (valAns == numAns) {
                              numAns++;
                              ans = realloc (ans, numAns * sizeof (char *));
                           }
                           ans[valAns] = buffer;
                           valAns++;
                        } else {
                           free (buffer);
                        }
                        /* If it doesn't exist, or it is a directory: check
                         * second choice. */
                     } else {
                        sprintf (buffer, "%s/ds.%s.%s", rootname, ptr,
                                 filter + 2);
                        if (myStat (buffer, &perm, NULL, NULL) == MYSTAT_ISFILE) {
                           /* check that it is readable */
                           if (perm & 4) {
                              if (valAns == numAns) {
                                 numAns++;
                                 ans = realloc (ans, numAns * sizeof (char *));
                              }
                              ans[valAns] = buffer;
                              valAns++;
                           } else {
                              free (buffer);
                           }
                        } else {
                           /* If doesn't exist, or is a directory: free it */
                           free (buffer);
                        }
                     }
                  }
               }
               free (rootname);

            }
         }
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
}
#endif
