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
#include "database.h"
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
/* Finer resolution (2.5km) version of Conus. This is
 * used for conus RTMA elements only.
 */
  {2953665, 30, 1, 6371.2, 6371.2,
    2145, 1377, 20.191999, 238.445999, 265.000000,
    2539.703000, 2539.703000, 25.000000,
    0, 0, 64, 0, 0,
    25.000000, 25.000000, -90, 0, 0, 0, 0, 0, 0},
/* Finer resolution version Puertori. */
   {75936, 10, 1, 6371.2, 6371.2,
    339, 224, 16.977485, 291.972167, 0.000000,
    1250.000000, 1250.000000, 20.000000,
    0, 0, 64, 19.544499, 296.0156,
    0, 0, 0, 0, 0, 0, 0, 0, 0},
/*
   {76953, 10, 1, 6371.2, 6371.2,
    339, 227, 16.977485, 291.972167, 0.000000,
    1250.000000, 1250.000000, 20.000000,
    0, 0, 64, 19.544499, 296.0156,
    0, 0, 0, 0, 0, 0, 0, 0, 0},
*/
/* Coarser resolution version of Puertori. */
/*
   {22833, 10, 1, 6371.2, 6371.2,
    177, 129, 16.828691, 291.804687, 0.000000,
    2500.000000, 2500.000000, 20.000000,
    0, 0, 64, 19.747399, 296.027600,
    0, 0, 0, 0, 0, 0, 0, 0, 0},
*/
   {72225, 10, 1, 6371.2, 6371.2,
    321, 225, 18.072658, 198.475021, 0.000000,
    2500.000000, 2500.000000, 20.000000,
    0, 0, 64, 23.087799, 206.130999,
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
    25.000000, 25.000000, -90, 0, 0, 0, 0, 0, 0},
   {1580529, 10, 1, 6371.2, 6371.2,
    1473, 1073, -25.0, 110.0, 0.0,
    10000.0, 10000.0, 20.0,
    0, 0, 64, 60.643999, 250.871000,
    0.0, 0.0, 0, 0, 0, 0, 0, 0, 0}
};

/* IF YOU ADD ANY SECTORS, MAKE SURE YOU UPDATE THIS IN META.H */
/* enum { NDFD_OCONUS_5CONUS, NDFD_OCONUS_2_5CONUS, NDFD_OCONUS_PR, 
          NDFD_OCONUS_HI, NDFD_OCONUS_GU, NDFD_OCONUS_AK, NDFD_OCONUS_NHEMI,
          NDFD_OCONUS_NPACIFIC, NDFD_OCONUS_UNDEF } */
static const char *NdfdDefSect[] = {
   "conus5", "conus2_5", "puertori", "hawaii", "guam", "alaska", "nhemi", 
   "npacocn"
};

/* 9999 means look in "<sectorName>timezone.flt" file, otherwise value to
 * adjust UTC clock by to get standard time */
static const int NdfdDefTimeZone[] = {
   9999, 9999, +4, +10, -10, 9999, 9999, 9999
};

/* 9999 means look in "<sectorName>daylight.flt" file, otherwise true/false
 * does areas observes daylight savings. */
static const int NdfdDefDayLight[] = {
   9999, 9999, 0, 0, 0, 9999, 9999, 9999
};

static size_t NumNdfdDefSect = sizeof (NdfdDefGds) / sizeof (NdfdDefGds[0]);

int SectorFindGDS (gdsType *gds)
{
   size_t i;            /* loop counter. */
   double lon;          /* Used to adjust lon to a range of 0..360. */
                        /* We still assume lon is an East longitude rather
                         * than a West one, but if the value is 190E,
                         * we'd also like to accept -170E. */

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
      if ((lon = gds->lon1) < 0)
         lon += 360;
      if (fabs (lon - NdfdDefGds[i].lon1) > 0.2)
         continue;
      if ((lon = gds->orientLon) < 0)
         lon += 360;
      if (fabs (lon - NdfdDefGds[i].orientLon) > 0.1)
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
      if ((lon = gds->lon2) < 0)
         lon += 360;
      if (fabs (lon - NdfdDefGds[i].lon2) > 0.1)
         continue;
      if (fabs (gds->scaleLat1 - NdfdDefGds[i].scaleLat1) > 0.1)
         continue;
      if (fabs (gds->scaleLat2 - NdfdDefGds[i].scaleLat2) > 0.1)
         continue;

/*    Commented out due to RTMA data coming in with the diff below = 90)
      if (fabs (gds->southLat - NdfdDefGds[i].southLat) > 0.1)
         continue;
*/
      if ((lon = gds->southLon) < 0)
         lon += 360;
      if (fabs (lon - NdfdDefGds[i].southLon) > 0.1)
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

/*****************************************************************************
 * SectorFillPnt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   For a given sector, determine which points are in it, and fill out their
 * sector information in the PntSectInfo structure.  Stays away from the
 * timeZone or f_dayLight questions.
 *
 * ARGUMENTS
 * f_sector = Enumerated value of this sector (Input)
 *      gds = GDS Structure to fill. (Input)
 *  f_first = Flag to init PntSect struct, since first time we've called
 *            the routine (Input)
 *  numPnts = number of input points (Input)
 *     pnts = The points to look at (Input)
 *  f_cells = flag as to whether pnts contains X,Y (false is lat/lon) (Input)
 *  pntInfo = pointInfo structure we're updating with sector info [only] (Out)
 *
 * RETURNS: int
 *  0 = no point in sector
 *  1 = this was the first (primary) sector at least one point was found in.
 *
 * NOTES
 *****************************************************************************
 */
static int SectorFillPnt (sChar f_sector, const gdsType *gds, sChar f_first,
                          size_t numPnts, Point * pnts, sChar f_cells,
                          PntSectInfo * pntInfo)
{
   sChar f_foundOne = 0; /* flag containing return value */
   myMaparam map;       /* The map projection to use with this sector. */
   int j;               /* loop counter over number of points. */
   int k;               /* loop counter used to init the f_sector[] array. */
   double x, y;         /* The converted X,Y value. */
   sInt4 x1, y1;        /* nearest integer values of X,Y. */

   SetMapParamGDS (&map, gds);
   for (j = 0; j < numPnts; j++) {
      /* If first sector we've looked at, init f_sector array to UNDEF. */
      if (f_first) {
         pntInfo[j].numSector = 0;
         for (k = 0; k < NDFD_OCONUS_UNDEF; k++) {
            pntInfo[j].f_sector[k] = NDFD_OCONUS_UNDEF;
            pntInfo[j].X[k] = -1;
            pntInfo[j].Y[k] = -1;
         }
      }
      /* Find x1, y1 as integers in range of 1,NX 1,NY */
      if (f_cells != 1) {
         myCll2xy (&map, pnts[j].Y, pnts[j].X, &x, &y);
      } else {
         x = pnts[j].X;
         y = pnts[j].Y;
      }
      x1 = (sInt4) (x + .5);
      y1 = (sInt4) (y + .5);
      /* Determine if they are in the grid */
      if ((x1 >= 1) && (x1 <= (sInt4) gds->Nx) && (y1 >= 1)
          && (y1 <= (sInt4) gds->Ny)) {
         /* Update f_sector[] and numSector */
         pntInfo[j].f_sector[pntInfo[j].numSector] = f_sector;
         pntInfo[j].X[pntInfo[j].numSector] = x;
         pntInfo[j].Y[pntInfo[j].numSector] = y;
         pntInfo[j].numSector++;
         myCxy2ll (&map, floor(x), floor(y),
                   &(pntInfo[j].pnt1[pntInfo[j].numSector].lat),
                   &(pntInfo[j].pnt1[pntInfo[j].numSector].lon));
         myCxy2ll (&map, floor(x), ceil(y),
                   &(pntInfo[j].pnt2[pntInfo[j].numSector].lat),
                   &(pntInfo[j].pnt2[pntInfo[j].numSector].lon));
         myCxy2ll (&map, ceil(x), floor(y),
                   &(pntInfo[j].pnt3[pntInfo[j].numSector].lat),
                   &(pntInfo[j].pnt3[pntInfo[j].numSector].lon));
         myCxy2ll (&map, ceil(x), ceil(y),
                   &(pntInfo[j].pnt4[pntInfo[j].numSector].lat),
                   &(pntInfo[j].pnt4[pntInfo[j].numSector].lon));

         /* We update f_foundOne if this is the first (primary) sector a
          * point was found in.  Example: a point falls in conus/alaska,
          * we only want to add conus (the primary one) to the list. */
         /* For conus/nhemi, we automatically expand the file list
          * to included nhemi if conus is there, but nhemi is not. */
         if (!f_foundOne) {
            if (pntInfo[j].numSector == 1) {
               f_foundOne = 1;
            }
         }
      }
   }
   return f_foundOne;
}

/*****************************************************************************
 * SectorElev() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   For a given sector, determine the elevations for each point based on its
 * primary sector (aka pntInfo[i].f_sector[0])
 *
 * ARGUMENTS
 *   f_sector = Enumerated value of this sector (Input)
 *   sectName = Sector name (For the file to look in) (Input)
 *    numPnts = number of input points (Input)
 *       pnts = The points to look at (Input)
 * geoDataDir = The user defined directory to look for geoData. (Input)
 *    pntInfo = pointInfo structure we're updating (Out)
 *
 * RETURNS: void
 *
 * NOTES
 *   The elevation GDS sections can differ from the current default
 * GDS for a sector (because the elevation ones are stale).
 *****************************************************************************
 */
static void SectorElev (sChar f_sector, const char *sectName,
                        size_t numPnts, Point * pnts,
                        const char *geoDataDir, PntSectInfo * pntInfo)
{
   size_t i;            /* loop counter. */
   char *fileName = NULL; /* used to create the timezone/daylight filename. */
   FILE *Flt;           /* Open elevation file */
   char *flxArray;      /* Array containing contents of .ind file */
   int flxArrayLen;     /* Length of flxArray */
   sInt4 x1, y1;        /* nearest integer values of X,Y. */
   float value;         /* The current value from .flt file. */
   sInt4 offset;        /* Where to read in the .flt file. */
   int gdsIndex = 1;    /* Which gds to use in the .ind file. */
   gdsType gds;         /* GDS section associated with Flt/Ind files. */
   char f_bad;          /* flag to check if we had problems opening the files.*/

   /* Open the files */
   f_bad = 0;
   if (geoDataDir == NULL) {
      f_bad = 1;
   } else {
      fileName = (char *) malloc (strlen (geoDataDir) + 1 + strlen (sectName)
                                  + strlen ("elev.flt") + 1);
      /* Try to open elev files */
      sprintf (fileName, "%s/%selev.flt", geoDataDir, sectName);
      if ((Flt = fopen (fileName, "rb")) == NULL) {
         f_bad = 1;
      } else {
         sprintf (fileName, "%s/%selev.ind", geoDataDir, sectName);
         if (ReadFLX (fileName, &flxArray, &flxArrayLen) != 0) {
            f_bad = 1;
            fclose (Flt);
         } else {
            /* Read gds projection. */
            ReadGDSBuffer (flxArray + HEADLEN + 2 + (gdsIndex - 1) * 129, &gds);
            free (flxArray);
         }
      }
      free (fileName);
   }

   /* Handle bad fileOpen case. */
   if (f_bad) {
      for (i = 0; i < numPnts; i++) {
         /* Don't need to look at numSector, because array is init to UNDEF. */
         /* Only care if this is the "primary" sector for the point */
         if (pntInfo[i].f_sector[0] == f_sector) {
            pntInfo[i].elev = 9999;
         }
      }
      return;
   }

   /* Convert point, seek to correct location, read file, store value. */
   for (i = 0; i < numPnts; i++) {
      if (pntInfo[i].f_sector[0] == f_sector) {
         x1 = (sInt4) (pntInfo[i].X[0] + .5);
         y1 = (sInt4) (pntInfo[i].Y[0] + .5);
         /* Possible point could fall outside the elevation grid, but still
          * be inside the NDFD grid (stale definition of elevation file). */
         if ((x1 >= 1) && (x1 <= (sInt4) gds.Nx) && (y1 >= 1)
             && (y1 <= (sInt4) gds.Ny)) {
            if (gds.scan == 0) {
               offset = ((x1 - 1) + (gds.Ny - y1) * gds.Nx) * sizeof (float);
            } else {
               offset = ((x1 - 1) + (y1 - 1) * gds.Nx) * sizeof (float);
            }
            fseek (Flt, offset, SEEK_SET);
            FREAD_LIT (&value, sizeof (float), 1, Flt);
            pntInfo[i].elev = value;
         } else {
            pntInfo[i].elev = 9999;
         }
      }
   }
   fclose (Flt);
}

/*****************************************************************************
 * SectorTimeZones() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   For a given sector, determine which points have it as their "primary"
 * sector, and determine those points timezone information, based either on
 * the timezone grids, or on knowledge about the sector.
 *
 * ARGUMENTS
 *   f_sector = Enumerated value of this sector (Input)
 *   sectName = Sector name (For the file to look in) (Input)
 *    numPnts = number of input points (Input)
 *       pnts = The points to look at (Input)
 *    f_cells = flag as to whether pnts contains X,Y (false is lat/lon) (In)
 * geoDataDir = The user defined directory to look for geoData. (Input)
 *   timeZone = 9999 (look in file) or what to set f_timeZone to. (Input)
 * f_dayLight = 9999 (look in file) or what to set f_dayLight to. (Input)
 *    pntInfo = pointInfo structure we're updating (Out)
 *
 * RETURNS: void
 *
 * NOTES
 *   The timezone/daylight GDS sections can differ from the current default
 * GDS for a sector (because the timezone ones are stale).
 *   Assumes for speed sake that GDS section of daylight.ind = timezone.ind
 *****************************************************************************
 */
static void SectorTimeZones (sChar f_sector, const char *sectName,
                             size_t numPnts, Point * pnts, sChar f_cells,
                             const char *geoDataDir, int timeZone,
                             int f_dayLight, PntSectInfo * pntInfo)
{
   size_t i;            /* loop counter. */
   char *fileName = NULL; /* used to create the timezone/daylight filename. */
   FILE *DayFlt;        /* Open daylight file */
   FILE *TZFlt;         /* Open timezone file */
   char *flxArray;      /* Array containing contents of .ind file */
   int flxArrayLen;     /* Length of flxArray */
   double x, y;         /* The converted X,Y value. */
   sInt4 x1, y1;        /* nearest integer values of X,Y. */
   float value;         /* The current value from .flt file. */
   sInt4 offset;        /* Where to read in the .flt file. */
   int gdsIndex = 1;    /* Which gds to use in the .ind file. */
   gdsType gds;         /* GDS section associated with Flt/Ind files. */
   myMaparam map;       /* The map projection associated with GDS. */

   myAssert (((f_dayLight == 9999) && (timeZone == 9999)) ||
             ((f_dayLight != 9999) && (timeZone != 9999)));

   /* Handle trivial case. */
   if (f_dayLight != 9999) {
      for (i = 0; i < numPnts; i++) {
         /* Don't need to look at numSector, because array is init to UNDEF. */
         /* Only care if this is the "primary" sector for the point */
         if (pntInfo[i].f_sector[0] == f_sector) {
            pntInfo[i].f_dayLight = f_dayLight;
            pntInfo[i].timeZone = timeZone;
         }
      }
      return;
   }

   /* Open the files */
   if (geoDataDir == NULL) {
      f_dayLight = 0;
      timeZone = 0;
   } else if (f_dayLight == 9999) {
      /* note len ("daylight.flt") = len ("timezone.flt") */
      if (strcmp(sectName, "conus5") == 0) {
         /* The +50 is to get some extra memory for safety.  The following
            sprintfs to deal with conus5 has been hacked from time to time. */
         fileName = (char *) malloc (strlen (geoDataDir) + 1 + strlen ("conus")
                                     + strlen ("timezone.flt") + 1 + 50);
         sprintf (fileName, "%s/%sdaylight.flt", geoDataDir, "conus");
      } else {
         /* The +50 is to get some extra memory for safety.  The following
            sprintfs to deal with conus5 has been hacked from time to time. */
         fileName = (char *) malloc (strlen (geoDataDir) + 1 + strlen (sectName)
                                     + strlen ("timezone.flt") + 1 + 50);
         sprintf (fileName, "%s/%sdaylight.flt", geoDataDir, sectName);
      }   
      /* Try to open dayLight files */
      if ((DayFlt = fopen (fileName, "rb")) == NULL) {
         f_dayLight = 0;
         timeZone = 0;
         TZFlt = NULL;
      } else {
         if (strcmp(sectName, "conus5") == 0) {
            sprintf (fileName, "%s/%stimezone.flt", geoDataDir, "conus");
         } else {
            sprintf (fileName, "%s/%stimezone.flt", geoDataDir, sectName);
         }
         if ((TZFlt = fopen (fileName, "rb")) == NULL) {
            fclose (DayFlt);
            f_dayLight = 0;
            timeZone = 0;
         } else {
            if (strcmp(sectName, "conus5") == 0) {
               sprintf (fileName, "%s/%stimezone.ind", geoDataDir, "conus");
            } else {
               sprintf (fileName, "%s/%stimezone.ind", geoDataDir, sectName);
            }
            if (ReadFLX (fileName, &flxArray, &flxArrayLen) != 0) {
               f_dayLight = 0;
               timeZone = 0;
               fclose (DayFlt);
               fclose (TZFlt);
            } else {
               /* Set up map projection. */
               ReadGDSBuffer (flxArray + HEADLEN + 2 + (gdsIndex - 1) * 129,
                              &gds);
               free (flxArray);
               /* Don't need map set up for f_cells = 1 (do need gds). */
               if (f_cells != 1) {
                  SetMapParamGDS (&map, &gds);
               }
            }
         }
      }
      free (fileName);
   }

   /* Handle bad fileOpen case. */
   if (f_dayLight != 9999) {
      for (i = 0; i < numPnts; i++) {
         /* Don't need to look at numSector, because array is init to UNDEF. */
         /* Only care if this is the "primary" sector for the point */
         if (pntInfo[i].f_sector[0] == f_sector) {
            pntInfo[i].f_dayLight = f_dayLight;
            pntInfo[i].timeZone = timeZone;
         }
      }
      return;
   }

   /* Convert point, seek to correct location, read file, store value. */
   for (i = 0; i < numPnts; i++) {
      if (pntInfo[i].f_sector[0] == f_sector) {
         if (f_cells != 1) {
            myCll2xy (&map, pnts[i].Y, pnts[i].X, &x, &y);
            /* Get x1, y1 as integers in range of 1,NX 1,NY */
            x1 = (sInt4) (x + .5);
            y1 = (sInt4) (y + .5);
         } else {
            /* Get x1, y1 in as integers in range of 1,NX 1,NY */
            x1 = (sInt4) (pnts[i].X + .5);
            y1 = (sInt4) (pnts[i].Y + .5);
         }
         /* Possible point could fall outside the timezone grid, but still
          * be inside the NDFD grid (stale definition of timezone file). */
         if ((x1 >= 1) && (x1 <= (sInt4) gds.Nx) && (y1 >= 1)
             && (y1 <= (sInt4) gds.Ny)) {
            offset = ((x1 - 1) + (y1 - 1) * gds.Nx) * sizeof (float);
            fseek (TZFlt, offset, SEEK_SET);
            FREAD_LIT (&value, sizeof (float), 1, TZFlt);
            /* timezone contains # hours to add to UTC to get local time.  */
            pntInfo[i].timeZone = (sChar) (-1 * value);
            fseek (DayFlt, offset, SEEK_SET);
            FREAD_LIT (&value, sizeof (float), 1, DayFlt);
            pntInfo[i].f_dayLight = (sChar) (value);
         } else {
            pntInfo[i].f_dayLight = 0;
            pntInfo[i].timeZone = 0;
         }
      }
   }
   fclose (DayFlt);
   fclose (TZFlt);
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
         if (SectorFillPnt (i, &(NdfdDefGds[i]), (i == 0), numPnts, pnts,
                            f_cells, pntInfo)) {
            /* update NumSect, Sect list */
            *Sect = (char **) realloc (*Sect, (*NumSect + 1) * sizeof (char *));
            (*Sect)[*NumSect] = (char *) malloc (strlen ( NdfdDefSect[i]) + 1);
            strcpy ((*Sect)[*NumSect],  NdfdDefSect[i]);
            *NumSect = *NumSect + 1;
            SectorTimeZones (i, NdfdDefSect[i], numPnts, pnts, f_cells, geoDataDir,
                             NdfdDefTimeZone[i], NdfdDefDayLight[i], pntInfo);
            SectorElev (i, NdfdDefSect[i], numPnts, pnts, geoDataDir, pntInfo);
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

      if (SectorFillPnt (f_sector, &gds, (lineCnt == 0), numPnts, pnts,
                         f_cells, pntInfo)) {
         /* update NumSect, Sect list */
         *Sect = (char **) realloc (*Sect, (*NumSect + 1) * sizeof (char *));
         (*Sect)[*NumSect] = (char *) malloc (strlen (NdfdDefSect[f_sector]) + 1);
         strcpy ((*Sect)[*NumSect],  NdfdDefSect[f_sector]);
         *NumSect = *NumSect + 1;
         SectorTimeZones (f_sector, NdfdDefSect[f_sector], numPnts, pnts, f_cells, geoDataDir,
                          NdfdDefTimeZone[f_sector], NdfdDefDayLight[f_sector],
                          pntInfo);
         SectorElev (f_sector, NdfdDefSect[f_sector], numPnts, pnts, geoDataDir,
                     pntInfo);
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
            if (strcmp (sect[j], "conus5") == 0) {
               rootname = (char *) malloc (lenInName + strlen ("conus") + 2);
               sprintf (rootname, "%s/%s", inNames[i], "conus");
            } else {
               rootname = (char *) malloc (lenInName + strlen (sect[j]) + 2);
               sprintf (rootname, "%s/%s", inNames[i], sect[j]);
            }
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
