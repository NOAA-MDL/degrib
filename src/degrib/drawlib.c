#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "cmapf.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "tendian.h"
#include "gd.h"
#include "gdfontt.h"
#include "gdfonts.h"
#include "gdfontmb.h"
#include "gdfontl.h"
#include "gdfontg.h"
#include "mapini.h"
#include "drawlib.h"
#include "myassert.h"

#include "clock.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

typedef struct {
   /* Taking advantage of the fact that PointType is X then Y */
   double X, Y;
} PointType;

typedef struct {
   gdPoint *pnts;
   int numPnts;
   int *partBeg;
   int *partLen;
   int numParts;
} ringType;

/*
 isLeft(): tests if a point is Left|On|Right of an infinite line.
    Input:  three points P0, P1, and P2
    Return: >0 for P2 left of the line through P0 and P1
            =0 for P2 on the line
            <0 for P2 right of the line
    See: the January 2001 Algorithm "Area of 2D and 3D Triangles and Polygons"

    NOTE: isLeft expects X, Y to be integers!!!
*/
static int isLeft (gdPoint P0, gdPoint P1, gdPoint P2)
{
   return ((P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y));
}


static int myCmap1EQmap2 (maparam *map1, maparam *map2)
{
   if (memcmp (map1, map2, sizeof (maparam)) == 0)
      return 1;
   return 0;
}

#include <math.h>

/* This is cxy2ll with map1 and then cll2xy with map2 */
/* Assumes user called myCmap1EQmap2 and found that map1 != map2. */
static void myCij2xy (maparam *map1, double x1, double y1, maparam *map2,
                      double *x2, double *y2)
{
   double lat, lon;
   double xi0, eta0, xi, eta;
   double ymerc, temp;
   double arg;
   double radial;
   double slat, gdlong, sndgam, cdgam, mercy, gmercy, rhog1;
   double dlong;

/*   cxy2ll (map1, x1, y1, &lat, &lon);*/
/* ...cc2gxy.c...*/
   xi0 = (x1 - map1->x0) * map1->gridszeq / map1->arad;
   eta0 = (y1 - map1->y0) * map1->gridszeq / map1->arad;
   xi = xi0 * map1->crotate - eta0 * map1->srotate;
   eta = xi0 * map1->srotate + eta0 * map1->crotate;

/*
   cnxyll(map1, xi, eta, &lat, &lon);
*/
#define FSM 1.e-2
#define NEARONE .9999999999999
   radial = 2. * eta - map1->gamma * (xi * xi + eta * eta);
   if (map1->gamma * radial >= NEARONE) {
      lat = map1->gamma > 0 ? 90. : -90.;
/*    *longit = map1->reflon; */
      lon = 90. + lat;
/* Change made 02/12/02 to acommodate WMO reporting conventions.  North
   pole is longitude 180., so "North" points to the Greenwich Meridian,
   South Pole is longitude 0. so "North" again points to the Greenwich
   Meridian.  */
      return;
   }
   ymerc = .5 * log1pabovera (-map1->gamma, radial);
   lat = cymr2l (map1, ymerc);
   temp = map1->gamma * xi;
   arg = 1. - map1->gamma * eta;
   if ((temp < 0 ? -temp : temp) < FSM * arg) {
      temp = temp / arg;
      temp *= temp;
      temp = xi / arg * (1. - temp *
                         (1. / 3. - temp * (1. / 5. - temp * (1. / 7.))));
   } else {
      temp = atan2 (temp, arg) / map1->gamma;
   }
   lon = map1->reflon + DEGPRAD * temp;

   lon = cperiodic (lon, -180., 180.);

/* cll2xy (map2, lat, lon, x2, y2);*/
/* ...stlmbr.c... */
/*   cnllxy (map2, lat, lon, &xi, &eta);*/

   dlong = RADPDEG * cperiodic (lon - map2->reflon, -180., 180.);
   gdlong = map2->gamma * dlong;
   if ((gdlong < -FSM) || (gdlong > FSM)) {
      sndgam = sin (gdlong) / map2->gamma;
      cdgam = (1. - cos (gdlong)) / map2->gamma / map2->gamma;
   } else {
      gdlong *= gdlong;
      sndgam = dlong * (1. - 1. / 6. * gdlong *
                        (1. - 1. / 20. * gdlong * (1. - 1. / 42. * gdlong)));
      cdgam = dlong * dlong * (1. - 1. / 12. * gdlong *
                               (1. - 1. / 30. * gdlong *
                                (1. - 1. / 56. * gdlong)));
   }
   slat = sin (RADPDEG * lat);
   if ((slat >= NEARONE) || (slat <= -NEARONE)) {
      eta = 1. / map2->gamma;
      xi = 0.;
      return;
   }
   mercy = cl2ymr (map2, lat);
   gmercy = mercy * map2->gamma;
   if ((gmercy < -FSM) || (gmercy > FSM)) {
      rhog1 = (1. - exp (-gmercy)) / map2->gamma;
   } else {
      rhog1 = mercy * (1. - 1. / 2. * gmercy *
                       (1. - 1. / 3. * gmercy * (1. - 1. / 4. * gmercy)));
   }
   eta = rhog1 + (1. - map2->gamma * rhog1) * map2->gamma * cdgam;
   xi = (1. - map2->gamma * rhog1) * sndgam;
#undef FSM
#undef REARTH
#undef ARAD
#undef BRAD
#undef ECCEN

   *x2 = map2->x0 + map2->arad / map2->gridszeq *
         (map2->crotate * xi + map2->srotate * eta);
   *y2 = map2->y0 + map2->arad / map2->gridszeq *
         (map2->crotate * eta - map2->srotate * xi);

}

/*
static double isLeft2 (gdPoint P0, gdPoint P1, gdPoint P2)
{
   return ((P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y));
}
*/

/*
 wn_PnPoly(): winding number test for a point in a polygon
      Input:   P = a point,
               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
      Return:  wn = the winding number (=0 only if P is outside V[])
 */
/* Note this only checks the first "outer" ring.*/
static int wn_PnPoly (gdPoint P, ringType ring)
{
   int fstLoopLen;
   int wn = 0;          /* the winding number counter */
   int i;

   if (ring.numParts == 1) {
      fstLoopLen = ring.numPnts;
   } else {
      fstLoopLen = ring.partLen[0];
   }
   /* loop through all edges of the polygon */
   for (i = 0; i < fstLoopLen - 1; i++) { /* edge from V[i] to V[i+1] */
      if (ring.pnts[i].y <= P.y) { /* start y <= P.y */
         if (ring.pnts[i + 1].y > P.y) /* an upward crossing */
            if (isLeft (ring.pnts[i], ring.pnts[i + 1], P) > 0)
               ++wn;    /* have a valid up intersect */
      } else {          /* start y > P.y (no test needed) */
         if (ring.pnts[i + 1].y <= P.y) /* a downward crossing */
            if (isLeft (ring.pnts[i], ring.pnts[i + 1], P) < 0)
               --wn;    /* have a valid down intersect */
      }
   }
   return wn;
}

/* Checks if polygon has same point twice. */
/* only first and last are allowed to be equal. */
/*
static int IsClean (gdPoint * pnt, int numPnts)
{
   int i, j;

   for (i = 0; i < numPnts - 2; i++) {
      for (j = i + 1; j < numPnts - 1; j++) {
         if ((pnt[j].x == pnt[i].x) && (pnt[j].y == pnt[i].y)) {
            printf ("Not Clean at %d of %d pnts\n", j, numPnts);
            return 1;
         }
      }
   }
   return 1;
}
*/

static double OrientNumGd (gdPoint *pnts, int numPnts)
{
   int i;
   double dfSum;

   dfSum = 0.0;
   for (i = 0; i < numPnts - 1; i++) {
      dfSum += pnts[i].x * pnts[i + 1].y - pnts[i].y * pnts[i + 1].x;
   }
   dfSum += (pnts[numPnts - 1].x * pnts[0].y -
             pnts[numPnts - 1].y * pnts[0].x);
   return dfSum;
}

static void AllocGDColor (colorType *color, gdImagePtr im)
{
/*
   color->gdIndex = gdImageColorAllocate (im, color->r, color->g, color->b);
*/
   color->gdIndex = gdImageColorResolve (im, color->r, color->g, color->b);
   return;
}

static void FindBounds (maparam *map, int SizeX, int SizeY, double A,
                        double Bx, double By, double projX1, double projY1,
                        double orientLon, double *minLt, double *minLg,
                        double *maxLt, double *maxLg)
{
   double x, y, X, Y;
   double lat, lon1, lon2, lon3, lon4;

   x = 0;
   y = 0;
   X = (x - Bx) * A + projX1;
   Y = (SizeY - y - By) * A + projY1;
   cxy2ll (map, X, Y, &lat, &lon1);
   *minLt = lat;
   *maxLt = lat;
   x = 0;
   y = SizeY;
   X = (x - Bx) * A + projX1;
   Y = (SizeY - y - By) * A + projY1;
   cxy2ll (map, X, Y, &lat, &lon2);
   if (*minLt > lat) {
      *minLt = lat;
   } else if (*maxLt < lat) {
      *maxLt = lat;
   }
   x = SizeX;
   y = 0;
   X = (x - Bx) * A + projX1;
   Y = (SizeY - y - By) * A + projY1;
   cxy2ll (map, X, Y, &lat, &lon3);
   if (*minLt > lat) {
      *minLt = lat;
   } else if (*maxLt < lat) {
      *maxLt = lat;
   }
   x = SizeX;
   y = SizeY;
   X = (x - Bx) * A + projX1;
   Y = (SizeY - y - By) * A + projY1;
   cxy2ll (map, X, Y, &lat, &lon4);
   if (*minLt > lat) {
      *minLt = lat;
   } else if (*maxLt < lat) {
      *maxLt = lat;
   }

   if ((lon2 < lon1) && (lon1 - lon2 < 180)) {
      *minLg = lon2;
      *maxLg = lon4;
   } else if (lon2 - lon1 < 180) {
      *minLg = lon1;
      *maxLg = lon3;
   } else {
      *minLg = lon2;
      *maxLg = lon4;
   }

   if (*minLg > *maxLg) {
      *minLg = *minLg - 360;
   }

   if ((orientLon > *minLg) && (orientLon < *maxLg)) {
      /* find big X for OrientLon */
      cll2xy (map, *maxLt, orientLon, &X, &Y);
      /* find big Y for y = 0 */
      y = 0;
      Y = (SizeY - y - By) * A + projY1;
      /* find lat / lon for big X , big Y. */
      cxy2ll (map, X, Y, &lat, &lon1);
      /* check lat... probably only need to check maxLt. */
      if (*minLt > lat) {
         *minLt = lat;
      } else if (*maxLt < lat) {
         *maxLt = lat;
      }
   }
}

/* Read and return data from field that have Val[] in the matchField column. */
static int ReadDbfFieldNumMatch (char *filename, char *field,
                                 char *matchField, char **Val, int numVal,
                                 double *data)
{
   FILE *fp;
   sInt4 Offset, Offset1;
   unsigned short int sizeHead;
   unsigned short int lenRec;
   char name[12];
   unsigned char f_found;
   int leftSide;
   int numCol;
   int fldLen;
   int i, j;
   char *buffer;
   int type;
   sInt4 numRec;
   int fieldStart = 0, matchStart = 0;
   int fieldLen = 0, matchLen = 0;
   int maxLen;

   strncpy (filename + strlen (filename) - 3, "dbf", 3);
   if ((fp = fopen (filename, "rb")) == NULL) {
      printf ("ERROR: Problems opening %s for read.", filename);
      return -1;
   }
   Offset = 4;
   fseek (fp, Offset, SEEK_SET); /* Skip first 4 bytes. */
   FREAD_LIT (&numRec, sizeof (sInt4), 1, fp);
   FREAD_LIT (&sizeHead, sizeof (short int), 1, fp);
   FREAD_LIT (&lenRec, sizeof (short int), 1, fp);
   Offset += 8 + 20;
   fseek (fp, Offset, SEEK_SET); /* Skip to first header. */
   numCol = (sizeHead - 1 - 32) / 32;
   leftSide = 1;
   f_found = 0;
   for (i = 0; i < numCol; i++) {
      fread (name, sizeof (char), 11, fp);
      name[11] = '\0';
      type = fgetc (fp);
      Offset += 11 + 1 + 4;
      fseek (fp, Offset, SEEK_SET);
      fldLen = fgetc (fp);
      if (strncmp (name, field, strlen (field)) == 0) {
         f_found++;
         fieldStart = leftSide;
         if (type != 'N') {
            printf ("ERROR: %s was not a numeric field\n", field);
            return -1;
         }
         fieldLen = fldLen;
      } else if (strncmp (name, matchField, strlen (matchField)) == 0) {
         f_found++;
         matchStart = leftSide;
         matchLen = fldLen;
      }
      leftSide += fldLen;
      if (f_found == 2) {
         Offset = sizeHead;
         fseek (fp, Offset, SEEK_SET); /* Seek end of header. */
         break;
      } else {
         Offset += 32 - (11 + 1 + 4);
         fseek (fp, Offset, SEEK_SET);
      }
   }
   if (f_found != 2) {
      printf ("Couldn't find field (either field '%s' or matchField '%s')\n",
              field, matchField);
      return -1;
   }

   maxLen = matchLen + 1;
   if (maxLen < fieldLen + 1) {
      maxLen = fieldLen + 1;
   }
   buffer = (char *) malloc (maxLen * sizeof (char));
   for (j = 0; j < numVal; j++) {
      data[j] = -9999;
   }
   for (i = 0; i < numRec; i++) {
      Offset1 = Offset + matchStart;
      fseek (fp, Offset1, SEEK_SET);
      fread (buffer, sizeof (char), matchLen, fp);
      buffer[matchLen] = '\0';
      for (j = 0; j < numVal; j++) {
         if (strcmp (buffer, Val[j]) == 0) {
            Offset1 = Offset + fieldStart;
            fseek (fp, Offset1, SEEK_SET);
            fread (buffer, sizeof (char), fieldLen, fp);
            buffer[fieldLen] = '\0';
            data[j] = atof (buffer);
            break;
         }
      }
      Offset += lenRec;
   }
   free (buffer);
   fclose (fp);
   return 0;
}

/* used when match field == field, and we just want to know which poly's
   to draw. */
static int ReadDbfFieldMatchRec (char *filename, char *matchField,
                                 char **Val, int numVal, char **Data,
                                 sInt4 *numRec)
{
   FILE *fp;
   sInt4 Offset, Offset1;
   unsigned short int sizeHead;
   unsigned short int lenRec;
   char name[12];
   unsigned char f_found;
   int leftSide;
   int numCol;
   int fldLen;
   int i, j;
   char *buffer;
   int type;
   int matchStart = 0;
   int matchLen = 0;
   int maxLen;

   strncpy (filename + strlen (filename) - 3, "dbf", 3);
   if ((fp = fopen (filename, "rb")) == NULL) {
      printf ("ERROR: Problems opening %s for read.", filename);
      return -1;
   }
   Offset = 4;
   fseek (fp, Offset, SEEK_SET); /* Skip first 4 bytes. */
   FREAD_LIT (numRec, sizeof (sInt4), 1, fp);
   FREAD_LIT (&sizeHead, sizeof (short int), 1, fp);
   FREAD_LIT (&lenRec, sizeof (short int), 1, fp);
   Offset += 8 + 20;
   fseek (fp, Offset, SEEK_SET); /* Skip to first header. */
   numCol = (sizeHead - 1 - 32) / 32;
   leftSide = 1;
   f_found = 0;
   for (i = 0; i < numCol; i++) {
      fread (name, sizeof (char), 11, fp);
      name[11] = '\0';
      type = fgetc (fp);
      Offset += 11 + 1 + 4;
      fseek (fp, Offset, SEEK_SET);
      fldLen = fgetc (fp);
      if (strncmp (name, matchField, strlen (matchField)) == 0) {
         f_found++;
         matchStart = leftSide;
         matchLen = fldLen;
      }
      leftSide += fldLen;
      if (f_found == 1) {
         Offset = sizeHead;
         fseek (fp, Offset, SEEK_SET); /* Seek end of header. */
         break;
      } else {
         Offset += 32 - (11 + 1 + 4);
         fseek (fp, Offset, SEEK_SET);
      }
   }
   if (f_found != 1) {
      printf ("Couldn't find matchField '%s')\n", matchField);
      return -1;
   }

   maxLen = matchLen + 1;
   buffer = (char *) malloc (maxLen * sizeof (char));
   *Data = (char *) malloc (*numRec * sizeof (char));
   for (i = 0; i < *numRec; i++) {
      Offset1 = Offset + matchStart;
      fseek (fp, Offset1, SEEK_SET);
      fread (buffer, sizeof (char), matchLen, fp);
      buffer[matchLen] = '\0';
      (*Data)[i] = 0;
      for (j = 0; j < numVal; j++) {
         if (strcmp (buffer, Val[j]) == 0) {
            /* Draw record i */
            (*Data)[i] = 1;
            break;
         }
      }
      Offset += lenRec;
   }
   free (buffer);
   fclose (fp);
   return 0;
}

static int ReadDbfFieldNum (char *filename, char *field, double **Data,
                            sInt4 *numRec)
{
   FILE *fp;
   sInt4 Offset;
   unsigned short int sizeHead;
   unsigned short int lenRec;
   char name[12];
   unsigned char f_found;
   int leftSide;
   int numCol;
   int fldLen;
   int i;
   char *buffer;
   int type;
   int recLen = 0;

   *Data = NULL;
   *numRec = 0;
   strncpy (filename + strlen (filename) - 3, "dbf", 3);
   if ((fp = fopen (filename, "rb")) == NULL) {
      printf ("ERROR: Problems opening %s for read.", filename);
      return -1;
   }
   Offset = 4;
   fseek (fp, Offset, SEEK_SET); /* Skip first 4 bytes. */
   FREAD_LIT (numRec, sizeof (sInt4), 1, fp);
   FREAD_LIT (&sizeHead, sizeof (short int), 1, fp);
   FREAD_LIT (&lenRec, sizeof (short int), 1, fp);
   Offset += 8 + 20;
   fseek (fp, Offset, SEEK_SET); /* Skip to first header. */
   numCol = (sizeHead - 1 - 32) / 32;
   leftSide = 1;
   f_found = 0;
   for (i = 0; i < numCol; i++) {
      fread (name, sizeof (char), 11, fp);
      name[11] = '\0';
      if (strncmp (name, field, strlen (field)) == 0) {
         f_found = 1;
         type = fgetc (fp);
         if (type != 'N') {
            printf ("ERROR: %s was not a numeric field\n", field);
            *numRec = 0;
            return -1;
         }
         Offset += 11 + 1 + 4;
         fseek (fp, Offset, SEEK_SET);
         recLen = fgetc (fp);
         recLen++;      /* make room for '\0' */
         Offset = sizeHead;
         fseek (fp, Offset, SEEK_SET); /* Seek end of header. */
         break;
      } else {
         Offset += 11 + 1 + 4;
         fseek (fp, Offset, SEEK_SET);
         fldLen = fgetc (fp);
         leftSide += fldLen;
         Offset += 32 - (11 + 1 + 4);
         fseek (fp, Offset, SEEK_SET);
      }
   }
   if (!f_found) {
      printf ("Couldn't find the field '%s'\n", field);
      *numRec = 0;
      return -1;
   }
   *Data = (double *) malloc (*numRec * sizeof (double));
   buffer = (char *) malloc (recLen * sizeof (char));
   for (i = 0; i < *numRec; i++) {
      Offset += leftSide;
      fseek (fp, Offset, SEEK_SET);
      fread (buffer, sizeof (char), recLen - 1, fp);
      buffer[recLen - 1] = '\0';
      (*Data)[i] = atof (buffer);
      Offset += lenRec - leftSide;
   }
   free (buffer);
   fclose (fp);
   return 0;
}

static void DrawDB2 (layerType *layer, gdImagePtr im, double *Data)
{
   int h, w, x, y;
   size_t i;
   size_t len;
   char buffer[100];
   char format[50];

   h = layer->numMatchVal * 13 + 8;
   len = 0;
   sprintf (format, "%%s = %%.%df", layer->legend.decimal);
   for (i = 0; i < layer->numMatchVal; i++) {
      sprintf (buffer, format, layer->matchVal[i], Data[i]);
      if (len < strlen (buffer)) {
         len = strlen (buffer);
      }
   }
   w = 8 + len * 7;
   x = layer->legend.x;
   y = layer->legend.y;
   gdImageFilledRectangle (im, x, y, x + w, y + h, layer->legend.fg.gdIndex);
   gdImageRectangle (im, x, y, x + w, y + h, layer->legend.bg.gdIndex);
   y += 9;
   for (i = 0; i < layer->numMatchVal; i++) {
      sprintf (buffer, format, layer->matchVal[i], Data[i]);
      gdImageString (im, gdFontMediumBold, x + 3, y - 6,
                     (unsigned char *) buffer, layer->legend.bg.gdIndex);
      y += 13;
   }
}

static void DrawPng (char *filename, gdImagePtr im, int x, int y)
{
   gdImagePtr im2;
   FILE *in;

   if ((in = fopen (filename, "rb")) == NULL) {
      return;
   }
   im2 = gdImageCreateFromPng (in);
   fclose (in);
   gdImageCopy (im, im2, x, y, 0, 0, im2->sx, im2->sy);
   gdImageDestroy (im2);
}

/* need a value for -Decimal.. */
static void DrawLegend (layerType *layer, gdImagePtr im)
{
   int h, w, x, y;
   int wid, hei;
   int start, stop, just;
   size_t i, j;
   size_t len;
   char buffer[100];
   double val;
   double tic;

   switch (layer->type) {
      case GRID:
         for (i = 0; i < layer->ramp.numColors; i++) {
            AllocGDColor (&layer->ramp.colors[i], im);
         }
         if (layer->ramp.thick != 0) {
            AllocGDColor (&layer->ramp.outline, im);
         }
         x = layer->legend.x;
         y = layer->legend.y;
         wid = layer->legend.wid;
         hei = layer->legend.hei;
         if (wid == -1) {
            wid = 6;
         }
         if (hei == -1) {
            hei = layer->ramp.numColors;
         }
         if (layer->legend.f_vert) {
            for (i = 0; i < layer->ramp.numColors; i++) {
               stop = y - (int) (i * hei / (layer->ramp.numColors - 1.0));
               start = y - (int) ((i + 1) * hei / (layer->ramp.numColors - 1.0));
               gdImageFilledRectangle (im, x, start, x + wid, stop, layer->ramp.colors[i].gdIndex);
            }
            if (layer->ramp.numLab == 0) {
               tic = layer->ramp.labMin;
               for (i = 0; i < layer->ramp.numColors; i++) {
                  stop = y - (int) (i * hei / (layer->ramp.numColors - 1.0));
                  start = y - (int) ((i + 1) * hei / (layer->ramp.numColors - 1.0));
                  val = layer->ramp.min + i * (layer->ramp.max - layer->ramp.min) / (layer->ramp.numColors - 1.0);
                  if (val >= tic) {
                     gdImageLine (im, x, start, x + wid, start, layer->ramp.outline.gdIndex);
                     if (layer->ramp.labInterval >= 1) {
                        sprintf (buffer, "%.0f", tic);
                     } else {
                        sprintf (buffer, "%0.1f", tic);
                     }
                     gdImageString (im, gdFontMediumBold, x - 3, start, (unsigned char *) buffer, layer->ramp.outline.gdIndex);
                     tic += layer->ramp.labInterval;
                     if (tic > layer->ramp.labMax)
                        break;
                  }
               }
            } else {
               for (i = 0; i < layer->ramp.numColors; i++) {
                  stop = y - (int) (i * hei / (layer->ramp.numColors - 1.0));
                  start = y - (int) ((i + 1) * hei / (layer->ramp.numColors - 1.0));
                  val = layer->ramp.min + i * (layer->ramp.max - layer->ramp.min) / (layer->ramp.numColors - 1.0);
                  for (j = 0; j < layer->ramp.numLab; j++) {
                     if (val == layer->ramp.labRay[j]) {
                        break;
                     }
                  }
                  if (j != layer->ramp.numLab) {
                     if (layer->ramp.labJust[j]) {
                        just = stop;
                     } else {
                        just = start;
                     }
                     gdImageLine (im, x, just, x + wid, just, layer->ramp.outline.gdIndex);
                     gdImageString (im, gdFontMediumBold, x - 3, just, (unsigned char *) layer->ramp.label[j], layer->ramp.outline.gdIndex);
                  }
               }
            }
         } else {
            for (i = 0; i < layer->ramp.numColors; i++) {
               start = x + (int) (i * hei / (layer->ramp.numColors - 1.0));
               stop = x + (int) ((i + 1) * hei / (layer->ramp.numColors - 1.0));
               gdImageFilledRectangle (im, start, y - wid, stop, y, layer->ramp.colors[i].gdIndex);
            }
            if (layer->ramp.numLab == 0) {
               tic = layer->ramp.labMin;
               for (i = 0; i < layer->ramp.numColors; i++) {
                  start = x + (int) (i * hei / (layer->ramp.numColors - 1.0));
                  stop = x + (int) ((i + 1) * hei / (layer->ramp.numColors - 1.0));
                  val = layer->ramp.min + i * (layer->ramp.max - layer->ramp.min) / (layer->ramp.numColors - 1.0);
            /*      val = layer->ramp.colors[i].value;*/
                  if (val >= tic) {
                     gdImageLine (im, start, y - wid, start, y, layer->ramp.outline.gdIndex);
                     if (layer->ramp.labInterval >= 1) {
                        sprintf (buffer, "%.0f", tic);
                     } else {
                        sprintf (buffer, "%0.1f", tic);
                     }
                     gdImageString (im, gdFontMediumBold, start - 6, y, (unsigned char *) buffer, layer->ramp.outline.gdIndex);
                     tic += layer->ramp.labInterval;
                     if (tic > layer->ramp.labMax)
                        break;
                  }
               }
            } else {
               for (i = 0; i < layer->ramp.numColors; i++) {
                  start = x + (int) (i * hei / (layer->ramp.numColors - 1.0));
                  stop = x + (int) ((i + 1) * hei / (layer->ramp.numColors - 1.0));
                  val = layer->ramp.min + i * (layer->ramp.max - layer->ramp.min) / (layer->ramp.numColors - 1.0);
                  for (j = 0; j < layer->ramp.numLab; j++) {
                     if (val == layer->ramp.labRay[j]) {
                        break;
                     }
                  }
                  if (j != layer->ramp.numLab) {
                     if (layer->ramp.labJust[j]) {
                        just = stop;
                     } else {
                        just = start;
                     }
                     gdImageLine (im, just, y - wid, just, y, layer->ramp.outline.gdIndex);
                     gdImageString (im, gdFontMediumBold, just - 6, y, (unsigned char *) layer->ramp.label[j], layer->ramp.outline.gdIndex);
                  }
               }
            }
         }

         break;
      case GRADUATED:
         for (i = 0; i < layer->grad.numSymbol; i++) {
            AllocGDColor (&layer->grad.symbol[i].fg, im);
            AllocGDColor (&layer->grad.symbol[i].out, im);
         }

         h = layer->grad.numSymbol * 13 + 8;
         len = 0;
         for (i = 0; i < layer->grad.numSymbol; i++) {
            if (layer->legend.style == 0) {
               sprintf (buffer, "%c%.*f %.*f%c",
                        (layer->grad.symbol[i].f_minInc) ? '[' : '(',
                        layer->legend.decimal, layer->grad.symbol[i].Min,
                        layer->legend.decimal, layer->grad.symbol[i].Max,
                        (layer->grad.symbol[i].f_maxInc) ? ']' : ')');
            } else if (layer->legend.style == 1) {
               if (layer->grad.symbol[i].f_minInc) {
                  sprintf (buffer, ">= %.*f",
                           layer->legend.decimal, layer->grad.symbol[i].Min);
               } else {
                  sprintf (buffer, "> %.*f",
                           layer->legend.decimal, layer->grad.symbol[i].Min);
               }
            } else if (layer->legend.style == 2) {
               if (layer->grad.symbol[i].f_maxInc) {
                  sprintf (buffer, "<= %.*f",
                           layer->legend.decimal, layer->grad.symbol[i].Max);
               } else {
                  sprintf (buffer, "< %.*f",
                           layer->legend.decimal, layer->grad.symbol[i].Max);
               }
            }
            if (len < strlen (buffer)) {
               len = strlen (buffer);
            }
         }
         w = 20 + len * 7;
         x = layer->legend.x;
         y = layer->legend.y;
         gdImageFilledRectangle (im, x, y, x + w, y + h,
                                 layer->legend.fg.gdIndex);
         if (!layer->legend.bg.f_null) {
            gdImageRectangle (im, x, y, x + w, y + h,
                              layer->legend.bg.gdIndex);
         }
         x += 9;
         y += 9;
         for (i = 0; i < layer->grad.numSymbol; i++) {
            /* Draw the symbol */
/* *INDENT-OFF* */
            if (layer->legend.pixelSize != 0) {
               if (layer->grad.symbol[i].f_mark == 0) {
                  gdImageString (im, gdFontMediumBold, x - 3, y - 6,
                                 (unsigned char *) layer->grad.symbol[i].mark,
                                 layer->grad.symbol[i].fg.gdIndex);
               } else {
                  if (layer->legend.pixelSize == 1) {
                     gdImageFilledRectangle(im, x - 2, y - 2, x + 2, y + 2,
                                            layer->grad.symbol[i].fg.gdIndex);
                  } else {
                     gdImageFilledRectangle(im, x - 3, y - 5, x + 3, y + 5,
                                            layer->grad.symbol[i].fg.gdIndex);
                  }
               }
            }
/* *INDENT-ON* */

            /* Draw the text */
            if (layer->legend.style == 0) {
               sprintf (buffer, "%c%.*f %.*f%c",
                        (layer->grad.symbol[i].f_minInc) ? '[' : '(',
                        layer->legend.decimal, layer->grad.symbol[i].Min,
                        layer->legend.decimal, layer->grad.symbol[i].Max,
                        (layer->grad.symbol[i].f_maxInc) ? ']' : ')');
            } else if (layer->legend.style == 1) {
               if (layer->grad.symbol[i].f_minInc) {
                  sprintf (buffer, ">= %.*f",
                           layer->legend.decimal, layer->grad.symbol[i].Min);
               } else {
                  sprintf (buffer, "> %.*f",
                           layer->legend.decimal, layer->grad.symbol[i].Min);
               }
            } else if (layer->legend.style == 2) {
               if (layer->grad.symbol[i].f_maxInc) {
                  sprintf (buffer, "<= %.*f",
                           layer->legend.decimal, layer->grad.symbol[i].Max);
               } else {
                  sprintf (buffer, "< %.*f",
                           layer->legend.decimal, layer->grad.symbol[i].Max);
               }
            }
            if (layer->legend.textC.r == -1) {
               gdImageString (im, gdFontMediumBold, x - 3 + 10, y - 6,
                              (unsigned char *) buffer,
                              layer->grad.symbol[i].fg.gdIndex);
            } else {
               gdImageString (im, gdFontMediumBold, x - 3 + 10, y - 6,
                              (unsigned char *) buffer,
                              layer->legend.textC.gdIndex);
            }
            y += 13;
         }
         break;
   }
}

static void DrawTitle (layerType *layer, gdImagePtr im)
{
   char *start;
   char *ptr;
   char *ptr2;
   int numLines;
   int y;
   int maxLen;
   int len;
   gdFontPtr font;
   int fontHght;
   int fontWidth;

   switch (layer->title.f_font) {
      case 1:
         font = gdFontTiny;
         fontHght = 9;
         fontWidth = 5;
         break;
      case 2:
         font = gdFontSmall;
         fontHght = 11;
         fontWidth = 6;
         break;
      case 3:
         font = gdFontMediumBold;
         fontHght = 11;
         fontWidth = 7;
         break;
      case 4:
         font = gdFontLarge;
         fontHght = 13;
         fontWidth = 8;
         break;
      case 5:
         font = gdFontGiant;
         fontHght = 13;
         fontWidth = 9;
         break;
      default:
         printf ("Unknown font type\n");
         font = gdFontGiant;
         fontHght = 13;
         fontWidth = 9;
         break;
   }

   /* count # of lines, and find max length. */
   numLines = 1;
   maxLen = 0;
   start = layer->title.name;
   ptr = start;
   while ((ptr2 = strchr (ptr, '\\')) != NULL) {
      if (*(ptr2 + 1) == 'n') {
         *ptr2 = '\0';
         len = strlen (start);
         if (len > maxLen)
            maxLen = len;
         start = ptr2 + 2;
         ptr = start;
         *ptr2 = '\\';
         numLines++;
      } else {
         ptr = ptr2 + 1;
         if (ptr != '\0') {
            ptr++;
         }
      }
   }
   len = strlen (start);
   if (len > maxLen)
      maxLen = len;

   /* Note... bg / fg switch here for historic purposes. */
   if (!layer->title.bg.f_null) {
      gdImageFilledRectangle (im, layer->title.x, layer->title.y,
                              layer->title.x + maxLen * fontWidth + 5,
                              layer->title.y + 2 + numLines * fontHght,
                              layer->title.bg.gdIndex);
   }
   if (!layer->title.fg.f_null) {
      gdImageRectangle (im, layer->title.x, layer->title.y,
                        layer->title.x + maxLen * fontWidth + 5,
                        layer->title.y + 2 + numLines * fontHght,
                        layer->title.fg.gdIndex);
   }

   start = layer->title.name;
   ptr = start;
   y = layer->title.y;
   while ((ptr2 = strchr (ptr, '\\')) != NULL) {
      if (*(ptr2 + 1) == 'n') {
         *ptr2 = '\0';
         if (layer->title.textC.r == -1) {
            gdImageString (im, font, layer->title.x + 3, y,
                           (unsigned char *) (start),
                           layer->title.fg.gdIndex);
         } else {
            gdImageString (im, font, layer->title.x + 3, y,
                           (unsigned char *) (start),
                           layer->title.textC.gdIndex);
         }

         start = ptr2 + 2;
         ptr = start;
         *ptr2 = '\\';
         y += fontHght;
      } else {
         ptr = ptr2 + 1;
         if (ptr != '\0') {
            ptr++;
         }
      }
   }

   if (layer->title.textC.r == -1) {
      gdImageString (im, font, layer->title.x + 3, y,
                     (unsigned char *) (start), layer->title.fg.gdIndex);
   } else {
      gdImageString (im, font, layer->title.x + 3, y,
                     (unsigned char *) (start), layer->title.textC.gdIndex);
   }
}

/* Not sure if the "LINES" code helps or hurts. */
static void fill4SidedObject (gdImagePtr im, gdPoint pnts[5], int fillIndex)
{
#ifdef LINES
   int x, y;
#endif
   int minX, minY, maxX, maxY;

   /* Test rectangle case 1. */
   /* and test point case. */
   if ((pnts[0].x == pnts[1].x) && (pnts[0].y == pnts[3].y) &&
       (pnts[2].x == pnts[3].x) && (pnts[1].y == pnts[2].y)) {
      if (pnts[0].x == pnts[2].x) {
         /* Test point case. */
         if (pnts[0].y == pnts[2].y) {
            gdImageSetPixel (im, pnts[0].x, pnts[0].y, fillIndex);
            return;
#ifdef LINES
         } else if (pnts[0].y < pnts[2].y) {
            for (y = pnts[0].y; y <= pnts[2].y; y++) {
               gdImageSetPixel (im, pnts[0].x, y, fillIndex);
            }
            return;
         } else {
            for (y = pnts[2].y; y <= pnts[0].y; y++) {
               gdImageSetPixel (im, pnts[0].x, y, fillIndex);
            }
            return;
#endif
         }
      }
#ifdef LINES
      if (pnts[0].y == pnts[2].y) {
         if (pnts[0].x < pnts[2].x) {
            for (x = pnts[0].x; x <= pnts[2].x; x++) {
               gdImageSetPixel (im, x, pnts[0].y, fillIndex);
            }
            return;
         } else {
            for (x = pnts[2].x; x <= pnts[0].x; x++) {
               gdImageSetPixel (im, x, pnts[0].y, fillIndex);
            }
            return;
         }
      }
#endif
      if (pnts[0].x < pnts[2].x) {
         minX = pnts[0].x;
         maxX = pnts[2].x;
      } else {
         minX = pnts[2].x;
         maxX = pnts[0].x;
      }
      if (pnts[0].y < pnts[2].y) {
         minY = pnts[0].y;
         maxY = pnts[2].y;
      } else {
         minY = pnts[2].y;
         maxY = pnts[0].y;
      }
      gdImageFilledRectangle (im, minX, minY, maxX, maxY, fillIndex);
      return;
   }
   /* Test rectangle case 2. */
   if ((pnts[0].y == pnts[1].y) && (pnts[2].y == pnts[3].y) &&
       (pnts[0].x == pnts[3].x) && (pnts[1].x == pnts[2].x)) {
      if (pnts[0].x < pnts[2].x) {
         minX = pnts[0].x;
         maxX = pnts[2].x;
      } else {
         minX = pnts[2].x;
         maxX = pnts[0].x;
      }
      if (pnts[0].y < pnts[2].y) {
         minY = pnts[0].y;
         maxY = pnts[2].y;
      } else {
         minY = pnts[2].y;
         maxY = pnts[0].y;
      }
      gdImageFilledRectangle (im, minX, minY, maxX, maxY, fillIndex);
      return;
   }
   pnts[4] = pnts[0];
   gdImageFilledPolygon (im, pnts, 5, fillIndex);
   return;
}

/* Ideas:
  1: Merge cells together reducing calls to fill
  2: allocate and calculate along a strip instead of whole lattice.
*/
static void DrawGrid (layerType *layer, maparam *map, gdImagePtr im,
                      double projX1, double projY1, double A, double Bx,
                      double By, int Y_Size, graduatedType * grad)
{
   int i, j;
   gdPoint pnts[5];
   gdPoint lastPnts[5];
   maparam gridMap;
   double X, Y;
   int index;
   int fillIndex;
   double value;
   double mesh;
   double step;
   gdPoint *lattice, *ptr;
   int latIndex;
   sChar *f_valLat, *f_ptr;
   sChar f_map1EQmap2;

   mkGeoid (&gridMap, AB, layer->gridProj.majEarth, layer->gridProj.majEarth);
   stlmbr (&gridMap, eqvlat (&gridMap, layer->gridProj.scaleLat1,
                             layer->gridProj.scaleLat2),
           layer->gridProj.orientLon);
   /* need mesh in KM */
   mesh = layer->gridProj.mesh / 1000.;
   stcm1p (&gridMap, 1, 1, layer->gridProj.lat1, layer->gridProj.lon1,
           layer->gridProj.meshLat, layer->gridProj.orientLon, mesh, 0);

   f_valLat = (sChar *) malloc ((layer->gridNy + 1) * (layer->gridNx + 1) *
                                sizeof (sChar));
   memset (f_valLat, 0, (layer->gridNy + 1) * (layer->gridNx + 1));
   lattice = (gdPoint *) malloc ((layer->gridNy + 1) * (layer->gridNx + 1) *
                                 sizeof (gdPoint));
   ptr = lattice;
   f_ptr = f_valLat;
#ifdef DEBUG
   printf ("Arthur 1 :: %f\n", clock () / (double) (CLOCKS_PER_SEC));
#endif
   f_map1EQmap2 = myCmap1EQmap2 (&gridMap, map);
#ifdef OLD_METHOD
   if (!f_map1EQmap2) {
      for (j = 0; j <= layer->gridNy; j++) {
         for (i = 0; i <= layer->gridNx; i++) {
            /* index = i + j * (layer->gridNx + 1); */
            myCij2xy (&gridMap, i + .5, j + .5, map, &X, &Y);
            /* x, y are now in projected space. */
            /* need to scale them to pixel space. */
            ptr->x = Bx + (X - projX1) / A;
            ptr->y = Y_Size - (By + (Y - projY1) / A);
            ptr++;
         }
      }
   } else {
      for (j = 0; j <= layer->gridNy; j++) {
         for (i = 0; i <= layer->gridNx; i++) {
            /* index = i + j * (layer->gridNx + 1); */
            ptr->x = Bx + ((i + .5) - projX1) / A;
            ptr->y = Y_Size - (By + ((j + .5) - projY1) / A);
            ptr++;
         }
      }
   }
#endif
#ifdef DEBUG
   printf ("Arthur 2 :: %f\n", clock () / (double) (CLOCKS_PER_SEC));
#endif

   for (j = 1; j <= layer->gridNy; j++) {
      for (i = 1; i <= layer->gridNx; i++) {

         value = layer->gridData[i - 1 + (j - 1) * layer->gridNx];
         if ((layer->ramp.f_missing) && (value == layer->ramp.missValue)) {
            if (!layer->ramp.missColor.f_null) {
               fillIndex = layer->ramp.missColor.gdIndex;
            } else {
               continue;
            }
         } else if ((value >= layer->ramp.min) && (value <= layer->ramp.max)) {
            /* step should be max - min / numColors - 1.
             * Example either A) [-3, -1, 1, 3] or B) [-3, -1, 1) the step is 2
             * For A) (max - min) / (# value - 1) = 6 / 3 = 2
             * For B) (max - min) / (# value - 1) = 4 / 2 = 2                                    
             * Example either A) [-6, -3, 0, 3, 6] or B) [-6, -3, 0, 3) the step is 3
             * For A) (max - min) / (# value - 1) = 12 / 4 = 3
             * For B) (max - min) / (# value - 1) = 9 / 3 = 2                                    
             */          
            step = (layer->ramp.max - layer->ramp.min) / (layer->ramp.numColors - 1);
            index = (int) (value - layer->ramp.min) / step;
/*
            if (index == (int) layer->ramp.numColors) {
               index = layer->ramp.numColors - 1;
            }
*/
            if (!layer->ramp.colors[index].f_null) {
               fillIndex = layer->ramp.colors[index].gdIndex;
            } else {
               continue;
            }
         } else {
            continue;
         }

         latIndex = (i - 1) + (j - 1) * (layer->gridNx + 1);
         if (!f_valLat[latIndex]) {
            /* i - .5 since it is (i - 1) + .5 */
            myCij2xy (&gridMap, i - .5, j - .5, map, &X, &Y);
            /* x, y are now in projected space. */
            /* need to scale them to pixel space. */
            lattice[latIndex].x = (int) (Bx + (X - projX1) / A);
            lattice[latIndex].y = (int) (Y_Size - (By + (Y - projY1) / A));
            f_valLat[latIndex] = 1;
         }
         pnts[0] = lattice[latIndex];
         latIndex = (i - 1) + (j) * (layer->gridNx + 1);
         if (!f_valLat[latIndex]) {
            /* i - .5 since it is (i - 1) + .5 */
            myCij2xy (&gridMap, i - .5, j + .5, map, &X, &Y);
            /* x, y are now in projected space. */
            /* need to scale them to pixel space. */
            lattice[latIndex].x = (int) (Bx + (X - projX1) / A);
            lattice[latIndex].y = (int) (Y_Size - (By + (Y - projY1) / A));
            f_valLat[latIndex] = 1;
         }
         pnts[1] = lattice[latIndex];
         latIndex = (i) + (j) * (layer->gridNx + 1);
         if (!f_valLat[latIndex]) {
            /* i - .5 since it is (i - 1) + .5 */
            myCij2xy (&gridMap, i + .5, j + .5, map, &X, &Y);
            /* x, y are now in projected space. */
            /* need to scale them to pixel space. */
            lattice[latIndex].x = (int) (Bx + (X - projX1) / A);
            lattice[latIndex].y = (int) (Y_Size - (By + (Y - projY1) / A));
            f_valLat[latIndex] = 1;
         }
         pnts[2] = lattice[latIndex];
         latIndex = (i) + (j - 1) * (layer->gridNx + 1);
         if (!f_valLat[latIndex]) {
            /* i - .5 since it is (i - 1) + .5 */
            myCij2xy (&gridMap, i + .5, j - .5, map, &X, &Y);
            /* x, y are now in projected space. */
            /* need to scale them to pixel space. */
            lattice[latIndex].x = (int) (Bx + (X - projX1) / A);
            lattice[latIndex].y = (int) (Y_Size - (By + (Y - projY1) / A));
            f_valLat[latIndex] = 1;
         }
         pnts[3] = lattice[latIndex];
#ifdef OLD_METHOD
         pnts[0] = lattice[(i - 1) + (j - 1) * (layer->gridNx + 1)];
         pnts[1] = lattice[(i - 1) + j * (layer->gridNx + 1)];
         pnts[2] = lattice[i + j * (layer->gridNx + 1)];
         pnts[3] = lattice[i + (j - 1) * (layer->gridNx + 1)];
#endif
         /* Thin out the draws.  */
         if ((pnts[0].x != lastPnts[0].x) || (pnts[0].y != lastPnts[0].y) ||
             (pnts[1].x != lastPnts[1].x) || (pnts[1].y != lastPnts[1].y) ||
             (pnts[2].x != lastPnts[2].x) || (pnts[2].y != lastPnts[2].y) ||
             (pnts[3].x != lastPnts[3].x) || (pnts[3].y != lastPnts[3].y)) {
/*
         printf ("%d %d, %d %d, %d %d, %d %d\n", pnts[0].x, pnts[0].y,
                 pnts[1].x, pnts[1].y, pnts[2].x, pnts[2].y,
                 pnts[3].x, pnts[3].y);
*/
            fill4SidedObject (im, pnts, fillIndex);
         }
         memcpy (lastPnts, pnts, 4 * sizeof (gdPoint));
      }
   }
   free (lattice);
   free (f_valLat);
}

/* Slower for all grid cells...
 * (27.139 sec for 8 maxt grids vs 23.173 sec)
 * possibly faster if skip the missing values.
 * (18.276 sec for 8 maxt grids vs 17.695 sec)
 */
/*
static void DrawGrid_orig (layerType * layer, maparam * map, gdImagePtr im,
                      double projX1, double projY1, double A, double Bx,
                      double By, int Y_Size, graduatedType * grad)
{
   int i, j;
   gdPoint pnts[5];
   double xray[5];
   double yray[5];
   int numPnts = 5;
   maparam gridMap;
   double X, Y;
   int index;
   int fillIndex;
   double value;
   double mesh;
   char f_sameMap;
   char f_compute0;
   char f_compute1;

   mkGeoid (&gridMap, AB, layer->gridProj.majEarth, layer->gridProj.majEarth);
   stlmbr (&gridMap, eqvlat (&gridMap, layer->gridProj.scaleLat1,
           layer->gridProj.scaleLat2), layer->gridProj.orientLon);
   * need mesh in KM *
   mesh = layer->gridProj.mesh / 1000.;
   stcm1p (&gridMap, 1, 1, layer->gridProj.lat1, layer->gridProj.lon1,
           layer->gridProj.meshLat, layer->gridProj.orientLon, mesh, 0);

   f_sameMap = myCmap1EQmap2 (&gridMap, map);

   for (j = 1; j <= layer->gridNy; j++) {
      for (i = 1; i <= layer->gridNx; i++) {

         value = layer->gridData[i - 1 + (j - 1) * layer->gridNx];
         if ((layer->ramp.f_missing) && (value == layer->ramp.missValue)) {
            if (!layer->ramp.missColor.f_null) {
               fillIndex = layer->ramp.missColor.gdIndex;
            } else {
               continue;
            }
         } else if ((value >= layer->ramp.min) && (value <= layer->ramp.max)) {
            index = (value - layer->ramp.min) /
                    (layer->ramp.max - layer->ramp.min) * layer->ramp.numColors;
            if (index == layer->ramp.numColors) {
               index = layer->ramp.numColors - 1;
            }
            if (!layer->ramp.colors[index].f_null) {
               fillIndex = layer->ramp.colors[index].gdIndex;
            } else {
               continue;
            }
         } else {
            continue;
         }

#ifdef TESTING
         if (!f_sameMap) {
            myCij2xy (&gridMap, i, j, map, &X, &Y);
         } else {
            X = i;
            Y = j;
         }
         * x, y are now in projected space. *
         * need to scale them to pixel space. *
         pnts[0].x = Bx + (X - projX1) / A;
         pnts[0].y = Y_Size - (By + (Y - projY1) / A);
         gdImageFilledRectangle (im, pnts[0].x - 1, pnts[0].y -1,
                                 pnts[0].x + 1, pnts[0].y + 1, fillIndex);
#else
         xray[0] = i - .5;
         yray[0] = j - .5;
         xray[1] = i - .5;
         yray[1] = j + .5;
         f_compute0 = 1;
         f_compute1 = 1;
         if (i != 1) {
            if ((xray[0] == xray[3]) && (yray[0] == yray[3])) {
               pnts[0] = pnts[3];
               f_compute0 = 0;
            }
            if ((xray[1] == xray[2]) && (yray[1] == yray[2])) {
               pnts[1] = pnts[2];
               f_compute1 = 0;
            }
         }
         if (f_compute0) {
            if (!f_sameMap) {
               myCij2xy (&gridMap, xray[0], yray[0], map, &X, &Y);
               * x, y are now in projected space. *
               * need to scale them to pixel space. *
               pnts[0].x = Bx + (X - projX1) / A;
               pnts[0].y = Y_Size - (By + (Y - projY1) / A);
            } else {
               * x, y are now in projected space. *
               * need to scale them to pixel space. *
               pnts[0].x = Bx + (xray[0] - projX1) / A;
               pnts[0].y = Y_Size - (By + (yray[0] - projY1) / A);
            }
         }
         if (f_compute1) {
            if (!f_sameMap) {
               myCij2xy (&gridMap, xray[1], yray[1], map, &X, &Y);
               * x, y are now in projected space. *
               * need to scale them to pixel space. *
               pnts[1].x = Bx + (X - projX1) / A;
               pnts[1].y = Y_Size - (By + (Y - projY1) / A);
            } else {
               * x, y are now in projected space. *
               * need to scale them to pixel space. *
               pnts[1].x = Bx + (xray[1] - projX1) / A;
               pnts[1].y = Y_Size - (By + (yray[1] - projY1) / A);
            }
         }
         xray[2] = i + .5;
         yray[2] = j + .5;
         if (!f_sameMap) {
            myCij2xy (&gridMap, xray[2], yray[2], map, &X, &Y);
            * x, y are now in projected space. *
            * need to scale them to pixel space. *
            pnts[2].x = Bx + (X - projX1) / A;
            pnts[2].y = Y_Size - (By + (Y - projY1) / A);
         } else {
            * x, y are now in projected space. *
            * need to scale them to pixel space. *
            pnts[2].x = Bx + (xray[2] - projX1) / A;
            pnts[2].y = Y_Size - (By + (yray[2] - projY1) / A);
         }

         xray[3] = i + .5;
         yray[3] = j - .5;
         if (!f_sameMap) {
            myCij2xy (&gridMap, xray[3], yray[3], map, &X, &Y);
            * x, y are now in projected space. *
            * need to scale them to pixel space. *
            pnts[3].x = Bx + (X - projX1) / A;
            pnts[3].y = Y_Size - (By + (Y - projY1) / A);
         } else {
            * x, y are now in projected space. *
            * need to scale them to pixel space. *
            pnts[3].x = Bx + (xray[3] - projX1) / A;
            pnts[3].y = Y_Size - (By + (yray[3] - projY1) / A);
         }
         pnts[4] = pnts[0];
         gdImageFilledPolygon (im, pnts, numPnts, fillIndex);
#endif
      }
   }
}
*/

static int DrawGradPointShpFile (char *filename, maparam *map,
                                 gdImagePtr im, graduatedType * grad,
                                 mapIniType * mapIni, double projX1,
                                 double projY1, double A, double Bx,
                                 double By, double *Data, sInt4 dataLen,
                                 char *FDraw, sInt4 numFDraw)
{
   FILE *sfp;
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   double Bounds[8];    /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */
   sInt4 curRec[2];     /* rec number, and content length. */
   PointType point;
   sInt4 Offset;
   sInt4 type;
   double X, Y;
   sInt4 x, y;
   int t1, t2;
   size_t i;
   char buffer[100];
   char format[50];
   int recNum;
   double value;
   char f_draw;

   strncpy (filename + strlen (filename) - 3, "shp", 3);
   if ((sfp = fopen (filename, "rb")) == NULL) {
      printf ("Problems opening %s for read\n", filename);
      return -1;
   }

   /* Read in the headers... */
   FREAD_BIG (Head1, sizeof (sInt4), 7, sfp);
   if (Head1[0] != 9994) {
      printf ("Invalid .shp file\n");
      fclose (sfp);
      return -1;
   }
   FREAD_LIT (Head2, sizeof (sInt4), 2, sfp);
   if (Head2[0] != 1000) {
      printf ("Invalid .shp file version \n");
      fclose (sfp);
      return -1;
   }
   if (Head2[1] != 1) {
      printf ("Expecting to draw points (type 1) yours is %ld\n", Head2[1]);
      fclose (sfp);
      return -1;
   }
   FREAD_LIT (Bounds, sizeof (double), 8, sfp);

   Offset = 100;
   recNum = -1;
   while (Offset < Head1[6] * 2) {
      recNum++;
      FREAD_BIG (curRec, sizeof (sInt4), 2, sfp);
      FREAD_LIT (&type, sizeof (sInt4), 1, sfp);
      if (type != 1) {
         printf ("Corrupt .shp file: point file with non-point shapes\n");
         printf ("Record: %ld Type = %ld \n", curRec[0], type);
         printf ("Offset = %ld, FileLen = %ld\n", Offset, Head1[6]);
         fclose (sfp);
         return -1;
      }
      f_draw = 1;
      if (numFDraw != -1) {
         if (numFDraw < recNum) {
            printf ("BUG!!\n");
         } else if (!FDraw[recNum]) {
            f_draw = 0;
         }
      }
      if (f_draw) {
         /* Taking advantage of the fact that PointType is X then Y */
         FREAD_LIT (&point, sizeof (double), 2, sfp);
         cll2xy (map, point.Y, point.X, &X, &Y);
         /* x, y are now in projected space. */
         /* need to scale them to pixel space. */
         x = (sInt4) (Bx + (X - projX1) / A);
         y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
         if (recNum > dataLen) {
            printf ("%d %ld [%ld] [%ld == %ld] [%ld == 28] "
                    "[offset Should be %ld\n", recNum, dataLen, curRec[0],
                    Offset, Head1[6] * 2, curRec[1] * 2,
                    100 + 28 * curRec[0]);
            break;
         }

         value = Data[recNum];
         for (i = 0; i < grad->numSymbol; i++) {
            if (((value < grad->symbol[i].Max) ||
                 (grad->symbol[i].f_maxInc && (value == grad->symbol[i].Max))) &&
                ((value > grad->symbol[i].Min) ||
                 (grad->symbol[i].f_minInc && (value == grad->symbol[i].Min)))) {
               if (grad->symbol[i].f_mark == 1) {
                  gdImageFilledRectangle (im, x - 2, y - 2, x + 2, y + 2,
                                          grad->symbol[i].fg.gdIndex);
/*               gdImageFilledArc(im, x, y, 6, 6, 0, 360,
                                  grad->symbol[i].fg.gdIndex, 0); */
               } else if (grad->symbol[i].f_mark == 2) {
                  sprintf (format, "%%.%df", grad->symbol[i].decimal);
                  sprintf (buffer, format, value);
                  gdImageString (im, gdFontMediumBold, x - 3, y - 6,
                                 (unsigned char *) (buffer),
                                 grad->symbol[i].fg.gdIndex);
               } else if (grad->symbol[i].f_mark == 3) {
                  gdImageSetPixel (im, x, y, grad->symbol[i].fg.gdIndex);
               } else if (grad->symbol[i].f_mark == 4) {
                  gdImageFilledRectangle (im, x - 1, y - 1, x + 1, y + 1,
                                          grad->symbol[i].fg.gdIndex);
               } else if (grad->symbol[i].f_mark == 5) {
                  gdImageFilledRectangle (im, x, y, x + 1, y + 1,
                                          grad->symbol[i].fg.gdIndex);
               } else if (grad->symbol[i].f_mark >= 6) {
                  t1 = (grad->symbol[i].f_mark - 3) / 2;
                  t2 = (grad->symbol[i].f_mark - 2) / 2;
                  gdImageFilledRectangle (im, x - t1, y - t1, x + t2, y + t2,
                                          grad->symbol[i].fg.gdIndex);
                  /* Draw the border (if the symbol is not -1,-1,-1) */
                  if (! grad->symbol[i].out.f_null) {
                     gdImageRectangle (im, x - t1, y - t1, x + t2, y + t2,
                                       grad->symbol[i].out.gdIndex);
                  }
               } else {
                  gdImageString (im, gdFontMediumBold, x - 3, y - 6,
                                 (unsigned char *) (grad->symbol[i].mark),
                                 grad->symbol[i].fg.gdIndex);
               }
               break;
            }
         }
         Offset += curRec[1] * 2 + 8;
      } else {
         Offset += curRec[1] * 2 + 8;
         fseek (sfp, Offset, SEEK_SET);
      }
   }
   fclose (sfp);
   return 0;
}

static int DrawPointShpFile (char *filename, maparam *map, gdImagePtr im,
                             SymbolType sym, mapIniType * mapIni,
                             double projX1, double projY1, double A,
                             double Bx, double By, char *FDraw,
                             sInt4 numFDraw)
{
   FILE *sfp;
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   double Bounds[8];    /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */
   sInt4 curRec[2];     /* rec number, and content length. */
   PointType point;
   sInt4 Offset;
   sInt4 type;
   double X, Y;
   sInt4 x, y;
   int recNum;
   char f_draw;

   strncpy (filename + strlen (filename) - 3, "shp", 3);
   if ((sfp = fopen (filename, "rb")) == NULL) {
      printf ("Problems opening %s for read\n", filename);
      return -1;
   }

   /* Read in the headers... */
   FREAD_BIG (Head1, sizeof (sInt4), 7, sfp);
   if (Head1[0] != 9994) {
      printf ("Invalid .shp file\n");
      fclose (sfp);
      return -1;
   }
   FREAD_LIT (Head2, sizeof (sInt4), 2, sfp);
   if (Head2[0] != 1000) {
      printf ("Invalid .shp file version \n");
      fclose (sfp);
      return -1;
   }
   if (Head2[1] != 1) {
      printf ("Expecting to draw points (type 1) yours is %ld\n", Head2[1]);
      fclose (sfp);
      return -1;
   }
   FREAD_LIT (Bounds, sizeof (double), 8, sfp);

   Offset = 100;
   recNum = -1;
   while (Offset < Head1[6] * 2) {
      recNum++;
      FREAD_BIG (curRec, sizeof (sInt4), 2, sfp);
      FREAD_LIT (&type, sizeof (sInt4), 1, sfp);
      if (type != 1) {
         printf ("Corrupt .shp file: point file with non-point shapes\n");
         printf ("Record: %ld Type = %ld \n", curRec[0], type);
         printf ("Offset = %ld, FileLen = %ld\n", Offset, Head1[6]);
         fclose (sfp);
         return -1;
      }
      f_draw = 1;
      if (numFDraw != -1) {
         if (numFDraw < recNum) {
            printf ("BUG!!\n");
         } else if (!FDraw[recNum]) {
            f_draw = 0;
         }
      }
      if (f_draw) {
         /* Taking advantage of the fact that PointType is X then Y */
         FREAD_LIT (&point, sizeof (double), 2, sfp);
         cll2xy (map, point.Y, point.X, &X, &Y);
         /* x, y are now in projected space. */
         /* need to scale them to pixel space. */
         x = (sInt4) (Bx + (X - projX1) / A);
         y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
         if (sym.f_mark != 0) {
            gdImageFilledRectangle (im, x - 2, y - 2, x + 2, y + 2,
                                    sym.fg.gdIndex);
/*         gdImageFilledArc(im, x, y, 6, 6, 0, 360, sym.fg.gdIndex, 0); */
         } else {
            gdImageString (im, gdFontMediumBold, x - 3, y - 6,
                           (unsigned char *) (sym.mark), sym.fg.gdIndex);
/*         gdImageChar(im, gdFontMediumBold, x - 3, y - 6, sym.mark[0],
                       sym.out.gdIndex);*/
/*         gdImageChar(im, gdFontLarge, x - 3, y - 8, sym.mark[0],
                       sym.fg.gdIndex); */
         }
         Offset += curRec[1] * 2 + 8;
      } else {
         Offset += curRec[1] * 2 + 8;
         fseek (sfp, Offset, SEEK_SET);
      }
   }
   fclose (sfp);
   return 0;
}

/*
 * Control line:
 * minLat, minLon, maxLat, maxLon, space, color, style (1,2,3),
 *         subspace (only matters for style 3),
 *         label site (combination of LTRB (left top right bottom))
 *         A '-' in the labelSite means "strict" keep lat labels on sides,
 *         lon label on top/bottom
 *
 */
static void DrawLattice(layerType *layer, maparam *map, gdImagePtr im,
                        mapIniType * mapIni, double projX1, double projY1,
                        double A, double Bx, double By, double minLon,
                        double minLat, double maxLon, double maxLat,
                        double X_Size, double Y_Size, double space,
                        int style, char *labelSite)
{
   double i, j, k;
   double X, Y;
   sInt4 x = 0, y = 0;
   sInt4 x0, y0, x1, y1, x2, y2, x3, y3;
   char buffer[100];
   sInt4 minX, maxX, minY, maxY;
   double subspace = space/25.;
   double val;
   char f_strict;

   /* Round bounds to nearest "space" unit. */
   minLat = (floor (minLat / space)) * space;
   minLon = (floor (minLon / space)) * space;
   maxLat = (ceil (maxLat / space)) * space;
   maxLon = (ceil (maxLon / space)) * space;

   if (style == 1) {
      for (i = minLon; i <= maxLon; i += space) {
         for (j = minLat; j <= maxLat; j += space) {
            cll2xy (map, j, i, &X, &Y);
            /* x, y are now in projected space. */
            /* need to scale them to pixel space. */
            x = (sInt4) (Bx + (X - projX1) / A);
            y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            gdImageSetPixel (im, x, y, layer->lattice.fg.gdIndex);
         }
      }
   } else if (style == 2) {
      for (i = minLon; i < maxLon; i += space) {
         for (j = minLat; j < maxLat; j += space) {
            cll2xy (map, j, i, &X, &Y);
            /* x, y are now in projected space. */
            /* need to scale them to pixel space. */
            x0 = (sInt4) (Bx + (X - projX1) / A);
            y0 = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            cll2xy (map, j + space, i, &X, &Y);
            x1 = (sInt4) (Bx + (X - projX1) / A);
            y1 = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            gdImageLine (im, x0, y0, x1, y1, layer->lattice.fg.gdIndex);
            cll2xy (map, j + space, i + space, &X, &Y);
            x2 = (sInt4) (Bx + (X - projX1) / A);
            y2 = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            gdImageLine (im, x1, y1, x2, y2, layer->lattice.fg.gdIndex);
            cll2xy (map, j, i + space, &X, &Y);
            x3 = (sInt4) (Bx + (X - projX1) / A);
            y3 = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            gdImageLine (im, x2, y2, x3, y3, layer->lattice.fg.gdIndex);
            gdImageLine (im, x3, y3, x0, y0, layer->lattice.fg.gdIndex);
         }
      }
   } else {
      for (i = minLon; i < maxLon; i += space) {
         for (j = minLat; j < maxLat; j += space) {
            cll2xy (map, j, i, &X, &Y);
            /* x, y are now in projected space. */
            /* need to scale them to pixel space. */
            x = (sInt4) (Bx + (X - projX1) / A);
            y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            gdImageSetPixel (im, x, y, layer->lattice.fg.gdIndex);
            for (k = 1; k <= space / subspace; k++) {
               cll2xy (map, j + k * subspace, i, &X, &Y);
               x = (sInt4) (Bx + (X - projX1) / A);
               y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
               gdImageSetPixel (im, x, y, layer->lattice.fg.gdIndex);
            }
            for (k = 1; k <= space / subspace; k++) {
               cll2xy (map, j + space, i + k * subspace, &X, &Y);
               x = (sInt4) (Bx + (X - projX1) / A);
               y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
               gdImageSetPixel (im, x, y, layer->lattice.fg.gdIndex);
            }
            for (k = 1; k <= space / subspace; k++) {
               cll2xy (map, j + k * subspace, i + space, &X, &Y);
               x = (sInt4) (Bx + (X - projX1) / A);
               y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
               gdImageSetPixel (im, x, y, layer->lattice.fg.gdIndex);
            }
            for (k = 1; k <= space / subspace; k++) {
               cll2xy (map, j, i + k * subspace, &X, &Y);
               x = (sInt4) (Bx + (X - projX1) / A);
               y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
               gdImageSetPixel (im, x, y, layer->lattice.fg.gdIndex);
            }
         }
      }
   }
/* Create labels */
   f_strict = 0;
   if (strchr (labelSite, '-') != NULL) {
      f_strict = 1;
   }

   minX = 2;
   minY = 0;
   maxY = Y_Size - 15;
   if (f_strict) {
      if ((minLat >= 0) && (maxLat < 10)) {
         maxX = X_Size - 7;
      } else if (minLat > -10) {
         maxX = X_Size - 15;
      } else {
         maxX = X_Size - 22;
      }
   } else {
      if ((minLat >= 0) && (maxLat < 10) &&
          (minLon >= 0) && (maxLon < 10)) {
         maxX = X_Size - 7;
      } else if ((minLat > -10) &&
                 (minLon > -10) && (maxLon < 100)) {
         maxX = X_Size - 15;
      } else if (minLon > -100) {
         maxX = X_Size - 22;
      } else {
         maxX = X_Size - 30;
      }
   }

   if (strchr (labelSite, 'L') != NULL) {
      for (j = minLat; j <= maxLat; j += space) {
         /* The following loop should be run once, but just in case, init x */
         x = 0;
         for (i = minLon; i <= maxLon; i += subspace) {
            cll2xy (map, j, i, &X, &Y);
            x = (sInt4) (Bx + (X - projX1) / A);
            y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            if ((x >= minX) && (x <= maxX) && (y >= minY) && (y <= maxY)) {
               break;
            }
         }
         if (f_strict && (x > minX + 10)) {
         } else if (i <= maxLon) {
            sprintf (buffer, "%.0f", j);
            gdImageString (im, gdFontMediumBold, x, y, (unsigned char *) buffer,
                           layer->lattice.fg.gdIndex);
         }
      }
   }
   if (strchr (labelSite, 'R') != NULL) {
      for (j = minLat; j <= maxLat; j += space) {
         /* The following loop should be run once, but just in case, init x */
         x = 0;
         for (i = maxLon; i >= minLon; i -= subspace) {
            cll2xy (map, j, i, &X, &Y);
            x = (sInt4) (Bx + (X - projX1) / A);
            y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            if ((x >= minX) && (x <= maxX) && (y >= minY) && (y <= maxY)) {
               break;
            }
         }
         if (f_strict && (x < maxX - 10)) {
         } else if (i >= minLon) {
            sprintf (buffer, "%.0f", j);
            gdImageString (im, gdFontMediumBold, x, y, (unsigned char *) buffer,
                           layer->lattice.fg.gdIndex);
         }
      }
   }
   if (strchr (labelSite, 'T') != NULL) {
      for (i = minLon; i <= maxLon; i += space) {
         /* The following loop should be run once, but just in case, init y */
         y = 0;
         for (j = maxLat; j >= minLat; j -= subspace) {
            cll2xy (map, j, i, &X, &Y);
            x = (sInt4) (Bx + (X - projX1) / A);
            y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            if ((x >= minX) && (x <= maxX) && (y >= minY) && (y <= maxY)) {
               break;
            }
         }
         if (f_strict && (y > minY + 10)) {
         } else if (j >= minLat) {
            val = i;
            while (val > 180) {
               val -= 360;
            }
            while (val < -180) {
               val += 360;
            }
            sprintf (buffer, "%.0f", val);
            gdImageString (im, gdFontMediumBold, x, y, (unsigned char *) buffer,
                           layer->lattice.fg.gdIndex);
         }
      }
   }
   if (strchr (labelSite, 'B') != NULL) {
      for (i = minLon; i <= maxLon; i += space) {
         /* The following loop should be run once, but just in case, init y */
         y = 0;
         for (j = minLat; j <= maxLat; j += subspace) {
            cll2xy (map, j, i, &X, &Y);
            x = (sInt4) (Bx + (X - projX1) / A);
            y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            if ((x >= minX) && (x <= maxX) && (y >= minY) && (y <= maxY)) {
               break;
            }
         }
         if (f_strict && (y < maxY - 10)) {
         } else if (j <= maxLat) {
            val = i;
            while (val > 180) {
               val -= 360;
            }
            while (val < -180) {
               val += 360;
            }
            sprintf (buffer, "%.0f", val);
            gdImageString (im, gdFontMediumBold, x, y, (unsigned char *) buffer,
                           layer->lattice.fg.gdIndex);
         }
      }
   }
}

/*
static void joinRingParts_old (ringType *ring, gdPoint **PolyPnts, int *NumPolyPnts)
{
   int i;
   int jj, kk;
   int numPolyPnts;
   gdPoint *polyPnts, *ptr;
   int dx, dy;
   int dist;
   int minDist, minJJ, minKK;
   int len;

   * Start with outter ring (part 0) *
   numPolyPnts = ring->partLen[0];
   polyPnts = (gdPoint *) malloc (numPolyPnts * sizeof (gdPoint));
   memcpy (polyPnts, ring->pnts + ring->partBeg[0], ring->partLen[0] * sizeof (gdPoint));
   * Handle simple case. *
   if (ring->numParts == 1) {
      *PolyPnts = polyPnts;
      *NumPolyPnts = numPolyPnts;
      return;
   }

   * find shortest distance from other rings to solution. (including which points did it) *
   for (i = 1; i < ring->numParts; i++) {
      minDist = -1;
      minJJ = -1;
      minKK = -1;
      for (jj = 0; jj < ring->partLen[i]; jj++) {
         for (kk = 0; kk < numPolyPnts; kk++) {
            dx = ring->pnts[ring->partBeg[i] + jj].x - polyPnts[kk].x;
            dy = ring->pnts[ring->partBeg[i] + jj].y - polyPnts[kk].y;
            dist = dx * dx + dy * dy;
            if ((minDist == -1) || (dist < minDist)) {
               minDist = dist;
               minJJ = jj;
               minKK = kk;
            }
         }
      }
      * Join at point minJJ, minKK. *
      len = numPolyPnts;
      numPolyPnts += ring->partLen[i] + 1;
      polyPnts = (gdPoint *) realloc (polyPnts, numPolyPnts * sizeof (gdPoint));
      * Make room. *
      ptr = polyPnts + numPolyPnts - 1;
      for (kk = len - 1; kk >= minKK; kk--) {
         *ptr = polyPnts[kk];
         ptr--;
      }
      * Insert. *
      ptr = polyPnts + minKK + 1;
      for (jj = minJJ; jj < ring->partLen[i]; jj++) {
         *ptr = ring->pnts[ring->partBeg[i] + jj];
         ptr++;
      }
      for (jj = 1; jj <= minJJ; jj++) {
         *ptr = ring->pnts[ring->partBeg[i] + jj];
         ptr++;
      }
   }

* Improvement: Find shortest distance, add just that ring,
   repeat computing new shortest distance using combined ring. *

   *PolyPnts = polyPnts;
   *NumPolyPnts = numPolyPnts;
}
*/

static void joinRingParts (ringType *ring, gdPoint **PolyPnts,
                           int *NumPolyPnts)
{
   typedef struct {
      int minAA, minBB;
      int partA, partB;
      int minDist;
   } distCalcType;
   distCalcType *distCalc;
   int numDistCalc;
   int cur;
   int i;
   int a, b;
   int aa, bb;
   int dx, dy;
   int dist;
   int minAA, minBB;
   int part;
   int numPolyPnts;
   gdPoint *polyPnts, *ptr;
   int len;

   /* Start with outter ring (part 0) */
   numPolyPnts = ring->partLen[0];
   polyPnts = (gdPoint *) malloc (numPolyPnts * sizeof (gdPoint));
   memcpy (polyPnts, ring->pnts + ring->partBeg[0],
           ring->partLen[0] * sizeof (gdPoint));
   /* Handle simple case. */
   if (ring->numParts == 1) {
      *PolyPnts = polyPnts;
      *NumPolyPnts = numPolyPnts;
      return;
   }

   /* Do all the distance calculations. */
   numDistCalc = ring->numParts * (ring->numParts - 1) / 2;
   distCalc = (distCalcType *) malloc (numDistCalc * sizeof (distCalcType));
   cur = 0;
   for (a = 0; a < ring->numParts - 1; a++) {
      for (b = a + 1; b < ring->numParts; b++) {
         /* find shortest distance from ring a to ring b */
         distCalc[cur].minDist = -1;
         distCalc[cur].partA = a;
         distCalc[cur].partB = b;
         distCalc[cur].minAA = -1;
         distCalc[cur].minBB = -1;
         for (aa = 0; aa < ring->partLen[a]; aa++) {
            for (bb = 0; bb < ring->partLen[b]; bb++) {
               dx = (ring->pnts[ring->partBeg[a] + aa].x -
                     ring->pnts[ring->partBeg[b] + bb].x);
               dy = (ring->pnts[ring->partBeg[a] + aa].y -
                     ring->pnts[ring->partBeg[b] + bb].y);
               dist = dx * dx + dy * dy;
               if ((distCalc[cur].minDist == -1) ||
                   (dist < distCalc[cur].minDist)) {
                  distCalc[cur].minDist = dist;
                  distCalc[cur].minAA = aa;
                  distCalc[cur].minBB = bb;
               }
            }
         }
         cur++;
      }
   }

   /* Search set of distCalc for shortest distance from ringA to ring0. */
   cur = 0;
   for (i = 1; i < numDistCalc; i++) {
      if ((distCalc[i].partA == 0) || (distCalc[i].partB == 0)) {
         if (distCalc[i].minDist < distCalc[cur].minDist) {
            cur = i;
         }
      }
   }

   while (cur != -1) {
      /* Join ring != 0 in cur with polyPnts. */
      if (distCalc[cur].partB != 0) {
         part = distCalc[cur].partB;
         minAA = distCalc[cur].minAA;
         minBB = distCalc[cur].minBB;
      } else {
/*         printf ("partA (%d) should != 0\n", distCalc[cur].partA);*/
         part = distCalc[cur].partA;
         minAA = distCalc[cur].minBB;
         minBB = distCalc[cur].minAA;
      }
      /* minAA refers to 0 part, minBB refers to part != 0 (part) */

      len = numPolyPnts;
      numPolyPnts += ring->partLen[part] + 1;
      polyPnts = (gdPoint *) realloc (polyPnts,
                                      numPolyPnts * sizeof (gdPoint));

      /* Make room. */
      ptr = polyPnts + numPolyPnts - 1;
      for (aa = len - 1; aa >= minAA; aa--) {
         *ptr = polyPnts[aa];
         ptr--;
      }
      /* Update indexes in distCalc. */
      for (i = 0; i < numDistCalc; i++) {
         if (distCalc[i].partA == 0) {
            if (distCalc[i].minAA >= minAA) {
               distCalc[i].minAA += ring->partLen[part] + 1;
            }
         }
         if (distCalc[i].partB == 0) {
            if (distCalc[i].minBB >= minAA) {
               distCalc[i].minBB += ring->partLen[part] + 1;
            }
         }
      }

      /* Insert. */
      ptr = polyPnts + minAA + 1;
      for (bb = minBB; bb < ring->partLen[part]; bb++) {
         *ptr = ring->pnts[ring->partBeg[part] + bb];
         ptr++;
      }
      for (bb = 1; bb <= minBB; bb++) {
         *ptr = ring->pnts[ring->partBeg[part] + bb];
         ptr++;
      }
      /* Update indexes in distCalc. */
      for (i = 0; i < numDistCalc; i++) {
         if (distCalc[i].partA == part) {
            distCalc[i].partA = 0;
            if (distCalc[i].minAA >= minBB) {
               distCalc[i].minAA = minAA + 1 + (distCalc[i].minAA - minBB);
            } else {
               distCalc[i].minAA = (minAA + 1 +
                                    (ring->partLen[part] - minBB) +
                                    distCalc[i].minAA);
            }
         }
         if (distCalc[i].partB == part) {
            distCalc[i].partB = 0;
            if (distCalc[i].minBB >= minBB) {
               distCalc[i].minBB = minAA + 1 + (distCalc[i].minBB - minBB);
            } else {
               distCalc[i].minBB = (minAA + 1 +
                                    (ring->partLen[part] - minBB) +
                                    distCalc[i].minBB);
            }
         }
         if ((distCalc[i].partA == 0) && (distCalc[i].partB == 0)) {
            distCalc[i].partA = -1;
            distCalc[i].partB = -1;
         }
      }

      /* Search set of distCalc for shortest distance from ringA to ring0. */
      cur = -1;
      for (i = 0; i < numDistCalc; i++) {
         if ((distCalc[i].partA == 0) || (distCalc[i].partB == 0)) {
            if (cur == -1) {
               cur = i;
            } else if (distCalc[i].minDist < distCalc[cur].minDist) {
               cur = i;
            }
         }
      }
   }

   *PolyPnts = polyPnts;
   *NumPolyPnts = numPolyPnts;
   free (distCalc);
}

static int DrawGradPolyShpFile (char *filename, maparam *map, gdImagePtr im,
                                graduatedType * grad, mapIniType * mapIni,
                                double projX1, double projY1, double A,
                                double Bx, double By, double *Data,
                                sInt4 dataLen, double minLt, double minLg,
                                double maxLt, double maxLg, char *FDraw,
                                sInt4 numFDraw)
{
   FILE *sfp;
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   double Bounds[8];    /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */
   double PolyBound[4]; /* Used for holding the polygon bounds. */
   sInt4 curRec[2];     /* rec number, and content length. */
   sInt4 numParts;
   sInt4 numPoints;
   sInt4 partsLen = 0;
   sInt4 pointsLen = 0;
   sInt4 *parts = NULL;
   PointType *points = NULL;
   sInt4 numGdPnts;
   sInt4 gdPntsLen = 0;
   gdPoint *gdPnts = NULL; /* Points of polygon */
   double X, Y;
   sInt4 x, y;
   int i, j;
   size_t ii;
   sInt4 Offset;
   sInt4 type;
   int curPart;
   int numExter = 0;
   int numInter = 0;
   ringType *inter = NULL;
   ringType *exter = NULL;
#ifdef DEBUG
/*   int numPoly = 0;*/
#endif
   double orient;
   int recNum;
   char f_draw;
   double value;
   int tmpIndex;
   gdPoint *polyPnts;
   int numPolyPnts;

   strncpy (filename + strlen (filename) - 3, "shp", 3);
   if ((sfp = fopen (filename, "rb")) == NULL) {
      printf ("Problems opening %s for read\n", filename);
      return -1;
   }
   /* Read in the headers... */
   FREAD_BIG (Head1, sizeof (sInt4), 7, sfp);
   if (Head1[0] != 9994) {
      printf ("Invalid .shp file\n");
      fclose (sfp);
      return -1;
   }
   FREAD_LIT (Head2, sizeof (sInt4), 2, sfp);
   if (Head2[0] != 1000) {
      printf ("Invalid .shp file version \n");
      fclose (sfp);
      return -1;
   }
   if (Head2[1] != 5) {
      printf ("Can only draw polygons (type 5) yours is %ld\n", Head2[1]);
      fclose (sfp);
      return -1;
   }
   FREAD_LIT (Bounds, sizeof (double), 8, sfp);
   if ((Bounds[0] > maxLg) || (Bounds[2] < minLg) ||
       (Bounds[1] > maxLt) || (Bounds[3] < minLt)) {
      fclose (sfp);
      return 0;
   }
   Offset = 100;
   recNum = -1;
   while (Offset < Head1[6] * 2) {
      recNum++;
#ifdef DEBUG
/*
      numPoly++;
      if (numPoly == 4) {
         break;
      }
*/
#endif
      FREAD_BIG (curRec, sizeof (sInt4), 2, sfp);
      FREAD_LIT (&type, sizeof (sInt4), 1, sfp);
      if (type != 5) {
         printf ("Corrupt .shp file: poly file with non-poly shapes\n");
         printf ("Record: %ld Type = %ld \n", curRec[0], type);
         printf ("Offset = %ld, FileLen = %ld\n", Offset, Head1[6]);
         free (parts);
         free (points);
         free (gdPnts);
         fclose (sfp);
         return -1;
      }
      f_draw = 1;
      if (numFDraw != -1) {
         if (numFDraw < recNum) {
            printf ("BUG!!\n");
         } else if (!FDraw[recNum]) {
            f_draw = 0;
         }
      }
      if (f_draw) {
         FREAD_LIT (PolyBound, sizeof (double), 4, sfp);
         if ((PolyBound[0] > maxLg) || (PolyBound[2] < minLg) ||
             (PolyBound[1] > maxLt) || (PolyBound[3] < minLt)) {
            Offset += curRec[1] * 2 + 8;
            fseek (sfp, Offset, SEEK_SET);
         } else {
            FREAD_LIT (&numParts, sizeof (sInt4), 1, sfp);
            FREAD_LIT (&numPoints, sizeof (sInt4), 1, sfp);
            if (partsLen < numParts) {
               partsLen = numParts;
               parts = (sInt4 *) realloc ((void *) parts,
                                          partsLen * sizeof (sInt4));
            }
            FREAD_LIT (parts, sizeof (sInt4), numParts, sfp);
            if (pointsLen < numPoints) {
               pointsLen = numPoints;
               points = (PointType *) realloc ((void *) points,
                                               pointsLen *
                                               sizeof (PointType));
            }
            /* Taking advantage of the fact that PointType is X then Y */
            FREAD_LIT (points, sizeof (double), numPoints * 2, sfp);

            /* Have read in the polygon with mult rings... Start converting
             * to GD. */
            curPart = 1;
            numGdPnts = 0;
            for (i = 0; i < numPoints; i++) {
               cll2xy (map, points[i].Y, points[i].X, &X, &Y);
               /* x, y are now in projected space. */
               /* need to scale them to pixel space. */
               x = (sInt4) (Bx + (X - projX1) / A);
               y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
               if ((numGdPnts == 0) ||
                   ((gdPnts[numGdPnts - 1].x != x) ||
                    (gdPnts[numGdPnts - 1].y != y))) {
                  numGdPnts++;
                  if (gdPntsLen < numGdPnts) {
                     /* Uses 100 for a buffer so we don't keep realloc'ing. */
                     gdPntsLen = numGdPnts + 100;
                     gdPnts = (gdPoint *) realloc ((void *) gdPnts,
                                                   gdPntsLen *
                                                   sizeof (gdPoint));
                  }
                  gdPnts[numGdPnts - 1].x = x;
                  gdPnts[numGdPnts - 1].y = y;
               }

               /* Check if finished with part... */
               if (curPart < numParts) {
                  /* i + 1 because they indexed parts starting at 0. */
                  if (i + 1 >= parts[curPart]) {
                     /* Check if it is properly closed. */
                     if ((x != gdPnts[0].x) || (y != gdPnts[0].y)) {
                        numGdPnts++;
                        if (gdPntsLen < numGdPnts) {
                           /* Uses 100 for a buffer so we don't keep
                            * realloc'ing. */
                           gdPntsLen = numGdPnts + 100;
                           gdPnts = (gdPoint *) realloc ((void *) gdPnts,
                                                         gdPntsLen *
                                                         sizeof (gdPoint));
                        }
                        gdPnts[numGdPnts - 1] = gdPnts[0];
                     }
                     /* Finished a part.  */
                     /* Check if there are at least 4 points (since first and
                      * last are equal). If so then find out if it is
                      * clockwise or counter-clockwise. Store data in
                      * clockwise or counter-clockwise list. */
                     if (numGdPnts >= 4) {
#ifdef DEBUG
/*
                        printf ("Finished a part numInter %d numExter %d\n",
                                numInter, numExter);
*/
#endif
                        orient = OrientNumGd (gdPnts, numGdPnts);
                        if (orient > 0) {
                           /* Could check if there are any self
                            * intersections. */
                           numExter++;
                           exter = (ringType *) realloc ((void *) exter,
                                                         numExter *
                                                         sizeof (ringType));
                           exter[numExter - 1].pnts =
                                 (gdPoint *) malloc (numGdPnts *
                                                     sizeof (gdPoint));
                           exter[numExter - 1].numPnts = numGdPnts;
                           memcpy (exter[numExter - 1].pnts, gdPnts,
                                   numGdPnts * sizeof (gdPoint));
                           exter[numExter - 1].partBeg =
                                 (int *) malloc (1 * sizeof (int));
                           exter[numExter - 1].partBeg[0] = 0;
                           exter[numExter - 1].partLen =
                                 (int *) malloc (1 * sizeof (int));
                           exter[numExter - 1].partLen[0] = numGdPnts;
                           exter[numExter - 1].numParts = 1;

                        } else if (orient < 0) {
                           /* Could check if there are any self
                            * intersections. */
                           numInter++;
                           inter = (ringType *) realloc ((void *) inter,
                                                         numInter *
                                                         sizeof (ringType));
                           inter[numInter - 1].pnts =
                                 (gdPoint *) malloc (numGdPnts *
                                                     sizeof (gdPoint));
                           inter[numInter - 1].numPnts = numGdPnts;
                           memcpy (inter[numInter - 1].pnts, gdPnts,
                                   numGdPnts * sizeof (gdPoint));
                           inter[numInter - 1].partBeg =
                                 (int *) malloc (1 * sizeof (int));
                           inter[numInter - 1].partBeg[0] = 0;
                           inter[numInter - 1].partLen =
                                 (int *) malloc (1 * sizeof (int));
                           inter[numInter - 1].partLen[0] = numGdPnts;
                           inter[numInter - 1].numParts = 1;
#ifdef DEBUG
/*
                        } else {
                           printf ("Flat\n");
*/
#endif
                        }
                     }
                     numGdPnts = 0;
                     curPart++;
                  }
               }
            }
            /* Finish last part. */
            /* Check if it is properly closed. */
            if ((gdPnts[numGdPnts - 1].x != gdPnts[0].x) ||
                (gdPnts[numGdPnts - 1].y != gdPnts[0].y)) {
               numGdPnts++;
               if (gdPntsLen < numGdPnts) {
                  /* Uses 100 for a buffer so we don't keep realloc'ing. */
                  gdPntsLen = numGdPnts + 100;
                  gdPnts = (gdPoint *) realloc ((void *) gdPnts,
                                                gdPntsLen * sizeof (gdPoint));
               }
               gdPnts[numGdPnts - 1] = gdPnts[0];
            }
            if (numGdPnts >= 4) {
               orient = OrientNumGd (gdPnts, numGdPnts);
               if (orient > 0) {
                  numExter++;
                  exter = (ringType *) realloc ((void *) exter,
                                                numExter * sizeof (ringType));
                  exter[numExter - 1].pnts =
                        (gdPoint *) malloc (numGdPnts * sizeof (gdPoint));
                  exter[numExter - 1].numPnts = numGdPnts;
                  memcpy (exter[numExter - 1].pnts, gdPnts,
                          numGdPnts * sizeof (gdPoint));
                  exter[numExter - 1].partBeg =
                        (int *) malloc (1 * sizeof (int));
                  exter[numExter - 1].partBeg[0] = 0;
                  exter[numExter - 1].partLen =
                        (int *) malloc (1 * sizeof (int));
                  exter[numExter - 1].partLen[0] = numGdPnts;
                  exter[numExter - 1].numParts = 1;
               } else if (orient < 0) {
                  numInter++;
                  inter = (ringType *) realloc ((void *) inter,
                                                numInter * sizeof (ringType));
                  inter[numInter - 1].pnts =
                        (gdPoint *) malloc (numGdPnts * sizeof (gdPoint));
                  inter[numInter - 1].numPnts = numGdPnts;
                  memcpy (inter[numInter - 1].pnts, gdPnts,
                          numGdPnts * sizeof (gdPoint));
                  inter[numInter - 1].partBeg =
                        (int *) malloc (1 * sizeof (int));
                  inter[numInter - 1].partBeg[0] = 0;
                  inter[numInter - 1].partLen =
                        (int *) malloc (1 * sizeof (int));
                  inter[numInter - 1].partLen[0] = numGdPnts;
                  inter[numInter - 1].numParts = 1;
               }
            }
            /* Have converted to GD, storing rings in either inter or exter
             * lists. */

            /* Join inter / exter data. */
            /* Algorithm still has problems... we join ABCDA to abcda, by
             * creating ABCDAabcdaA.  However line from Aa and aA appears in
             * poly draws, if Aa goes through another "hole" (12341) in
             * ABCDA. */
            for (i = 0; i < numInter; i++) {
               for (j = 0; j < numExter; j++) {
                  if (wn_PnPoly (inter[i].pnts[0], exter[j]) != 0) {
#ifdef DEBUG
/*
                     printf ("%d is inside %d \n", i, j);
*/
#endif
                     exter[j].numParts++;
                     exter[j].partBeg =
                           (int *) realloc ((void *) exter[j].partBeg,
                                            exter[j].numParts * sizeof (int));
                     exter[j].partBeg[exter[j].numParts - 1] =
                           exter[j].numPnts;
                     exter[j].partLen =
                           (int *) realloc ((void *) exter[j].partLen,
                                            exter[j].numParts * sizeof (int));
                     exter[j].partLen[exter[j].numParts - 1] =
                           inter[i].numPnts;

                     tmpIndex = exter[j].numPnts;
#ifdef STOP_GAP
                     exter[j].numPnts += inter[i].numPnts + 1;
#else
                     exter[j].numPnts += inter[i].numPnts;
#endif
                     exter[j].pnts =
                           (gdPoint *) realloc ((void *) exter[j].pnts,
                                                exter[j].numPnts *
                                                sizeof (gdPoint));
                     memcpy (exter[j].pnts + tmpIndex, inter[i].pnts,
                             inter[i].numPnts * sizeof (gdPoint));
#ifdef STOP_GAP
                     exter[j].pnts[exter[j].numPnts - 1] =
                           exter[j].pnts[tmpIndex - 1];
#endif
                     break;
                  }
               }
            }
            /* Draw exter list of polygons... */
            value = Data[recNum];
            for (ii = 0; ii < grad->numSymbol; ii++) {
               if (((value < grad->symbol[ii].Max) ||
                    (grad->symbol[ii].f_maxInc && (value == grad->symbol[ii].Max))) &&
                   ((value > grad->symbol[ii].Min) ||
                    (grad->symbol[ii].f_minInc && (value == grad->symbol[ii].Min)))) {
                  if (!grad->symbol[ii].fg.f_null) {
                     for (i = 0; i < numExter; i++) {
/* Join parts together. */
                        joinRingParts (&(exter[i]), &polyPnts, &numPolyPnts);
                        gdImageFilledPolygon (im, polyPnts,
                                              numPolyPnts,
                                              grad->symbol[ii].fg.gdIndex);
/*
                        gdImageFilledPolygon (im, exter[i].pnts,
                                              exter[i].numPnts,
                                              grad->symbol[ii].fg.gdIndex);
*/
                        free (polyPnts);
                     }
                  }
                  /* Draw exter list of poly-lines. */
                  if (!grad->symbol[ii].out.f_null) {
                     for (i = 0; i < numExter; i++) {
                        for (j = 0; j < exter[i].numParts; j++) {
                           if (grad->symbol[ii].thick != 1) {
                              gdImageSetThickness (im,
                                                   grad->symbol[ii].thick);
                           }
                           gdImagePolygon (im,
                                           exter[i].pnts +
                                           exter[i].partBeg[j],
                                           exter[i].partLen[j],
                                           grad->symbol[ii].out.gdIndex);
                           if (grad->symbol[ii].thick != 1) {
                              gdImageSetThickness (im, 1);
                           }
                        }
                     }
                  }
                  break;
               }
            }

            /* Free exter list and inter list. */
            for (i = 0; i < numExter; i++) {
               free (exter[i].pnts);
               free (exter[i].partBeg);
               free (exter[i].partLen);
            }
            free (exter);
            exter = NULL;
            numExter = 0;
            for (i = 0; i < numInter; i++) {
               free (inter[i].pnts);
               free (inter[i].partBeg);
               free (inter[i].partLen);
            }
            free (inter);
            inter = NULL;
            numInter = 0;
            Offset += curRec[1] * 2 + 8;
         }
      } else {
         Offset += curRec[1] * 2 + 8;
         fseek (sfp, Offset, SEEK_SET);
      }
   }

   /* Paint it in white */
   /* Outline it in red; must be done second */
/*
   gdImageSetThickness(im, 4);
   gdImagePolygon(im, points+5, 4, out);
   gdImageSetThickness(im, 2);
   gdImagePolygon(im, points, 4, out);
*/
   free (parts);
   free (points);
   free (gdPnts);
   fclose (sfp);
   return 0;
}

static int DrawPolyShpFile (char *filename, maparam *map, gdImagePtr im,
                            SymbolType sym, mapIniType * mapIni,
                            double projX1, double projY1, double A,
                            double Bx, double By, double minLt, double minLg,
                            double maxLt, double maxLg, char *FDraw,
                            sInt4 numFDraw)
{
   FILE *sfp;
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   double Bounds[8];    /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */
   double PolyBound[4]; /* Used for holding the polygon bounds. */
   sInt4 curRec[2];     /* rec number, and content length. */
   sInt4 numParts;
   sInt4 numPoints;
   sInt4 partsLen = 0;
   sInt4 pointsLen = 0;
   sInt4 *parts = NULL;
   PointType *points = NULL;
   sInt4 numGdPnts;
   sInt4 gdPntsLen = 0;
   gdPoint *gdPnts = NULL; /* Points of polygon */
   double X, Y;
   sInt4 x, y;
   int i, j;
   sInt4 Offset;
   sInt4 type;
   int curPart;
   int numExter = 0;
   int numInter = 0;
   ringType *inter = NULL;
   ringType *exter = NULL;
#ifdef DEBUG
/*
   int numPoly = 0;
*/
#endif
   double orient;
   int recNum;
   char f_draw;
   int tmpIndex;
   gdPoint *polyPnts;
   int numPolyPnts;

   strncpy (filename + strlen (filename) - 3, "shp", 3);
   if ((sfp = fopen (filename, "rb")) == NULL) {
      printf ("Problems opening %s for read\n", filename);
      return -1;
   }
   /* Read in the headers... */
   FREAD_BIG (Head1, sizeof (sInt4), 7, sfp);
   if (Head1[0] != 9994) {
      printf ("Invalid .shp file\n");
      fclose (sfp);
      return -1;
   }
   FREAD_LIT (Head2, sizeof (sInt4), 2, sfp);
   if (Head2[0] != 1000) {
      printf ("Invalid .shp file version \n");
      fclose (sfp);
      return -1;
   }
   if (Head2[1] != 5) {
      printf ("Can only draw polygons (type 5) yours is %ld\n", Head2[1]);
      fclose (sfp);
      return -1;
   }
   FREAD_LIT (Bounds, sizeof (double), 8, sfp);
   if ((Bounds[0] > maxLg) || (Bounds[2] < minLg) ||
       (Bounds[1] > maxLt) || (Bounds[3] < minLt)) {
      fclose (sfp);
      return 0;
   }
   Offset = 100;
   recNum = -1;
   while (Offset < Head1[6] * 2) {
      recNum++;
#ifdef DEBUG
/*
      numPoly++;
      if (numPoly == 4) {
         break;
      }
*/
#endif
      FREAD_BIG (curRec, sizeof (sInt4), 2, sfp);
      FREAD_LIT (&type, sizeof (sInt4), 1, sfp);
      if (type != 5) {
         printf ("Corrupt .shp file: poly file with non-poly shapes\n");
         printf ("Record: %ld Type = %ld \n", curRec[0], type);
         printf ("Offset = %ld, FileLen = %ld\n", Offset, Head1[6]);
         free (parts);
         free (points);
         free (gdPnts);
         fclose (sfp);
         return -1;
      }
      f_draw = 1;
      if (numFDraw != -1) {
         if (numFDraw < recNum) {
            printf ("BUG!!\n");
         } else if (!FDraw[recNum]) {
            f_draw = 0;
         }
      }
      if (f_draw) {
         FREAD_LIT (PolyBound, sizeof (double), 4, sfp);
         if ((PolyBound[0] > maxLg) || (PolyBound[2] < minLg) ||
             (PolyBound[1] > maxLt) || (PolyBound[3] < minLt)) {
            Offset += curRec[1] * 2 + 8;
            fseek (sfp, Offset, SEEK_SET);
         } else {
            FREAD_LIT (&numParts, sizeof (sInt4), 1, sfp);
            FREAD_LIT (&numPoints, sizeof (sInt4), 1, sfp);
            if (partsLen < numParts) {
               partsLen = numParts;
               parts = (sInt4 *) realloc ((void *) parts,
                                          partsLen * sizeof (sInt4));
            }
            FREAD_LIT (parts, sizeof (sInt4), numParts, sfp);
            if (pointsLen < numPoints) {
               pointsLen = numPoints;
               points = (PointType *) realloc ((void *) points,
                                               pointsLen *
                                               sizeof (PointType));
            }
            /* Taking advantage of the fact that PointType is X then Y */
            FREAD_LIT (points, sizeof (double), numPoints * 2, sfp);

            /* Have read in the polygon with mult rings... Start converting
             * to GD. */
            curPart = 1;
            numGdPnts = 0;
            for (i = 0; i < numPoints; i++) {
               cll2xy (map, points[i].Y, points[i].X, &X, &Y);
               /* x, y are now in projected space. */
               /* need to scale them to pixel space. */
               x = (sInt4) (Bx + (X - projX1) / A);
               y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
               if ((numGdPnts == 0) ||
                   ((gdPnts[numGdPnts - 1].x != x) ||
                    (gdPnts[numGdPnts - 1].y != y))) {
                  numGdPnts++;
                  if (gdPntsLen < numGdPnts) {
                     /* Uses 100 for a buffer so we don't keep realloc'ing. */
                     gdPntsLen = numGdPnts + 100;
                     gdPnts = (gdPoint *) realloc ((void *) gdPnts,
                                                   gdPntsLen *
                                                   sizeof (gdPoint));
                  }
                  gdPnts[numGdPnts - 1].x = x;
                  gdPnts[numGdPnts - 1].y = y;
               }

               /* Check if finished with part... */
               if (curPart < numParts) {
                  /* i + 1 because they indexed parts starting at 0. */
                  if (i + 1 >= parts[curPart]) {
                     /* Check if it is properly closed. */
                     if ((x != gdPnts[0].x) || (y != gdPnts[0].y)) {
                        numGdPnts++;
                        if (gdPntsLen < numGdPnts) {
                           /* Uses 100 for a buffer so we don't keep
                            * realloc'ing. */
                           gdPntsLen = numGdPnts + 100;
                           gdPnts = (gdPoint *) realloc ((void *) gdPnts,
                                                         gdPntsLen *
                                                         sizeof (gdPoint));
                        }
                        gdPnts[numGdPnts - 1] = gdPnts[0];
                     }
                     /* Finished a part.  */
                     /* Check if there are at least 4 points (since first and 
                      * last are equal). If so then find out if it is
                      * clockwise or counter-clockwise. Store data in
                      * clockwise or counter-clockwise list. */
                     if (numGdPnts >= 4) {
#ifdef DEBUG
/*
                        printf ("Finished a part numInter %d numExter %d\n",
                                numInter, numExter);
*/
#endif
                        orient = OrientNumGd (gdPnts, numGdPnts);
                        if (orient > 0) {
                           /* Could check if there are any self
                            * intersections. */
                           numExter++;
                           exter = (ringType *) realloc ((void *) exter,
                                                         numExter *
                                                         sizeof (ringType));
                           exter[numExter - 1].pnts =
                                 (gdPoint *) malloc (numGdPnts *
                                                     sizeof (gdPoint));
                           exter[numExter - 1].numPnts = numGdPnts;
                           memcpy (exter[numExter - 1].pnts, gdPnts,
                                   numGdPnts * sizeof (gdPoint));
                           exter[numExter - 1].partBeg =
                                 (int *) malloc (1 * sizeof (int));
                           exter[numExter - 1].partBeg[0] = 0;
                           exter[numExter - 1].partLen =
                                 (int *) malloc (1 * sizeof (int));
                           exter[numExter - 1].partLen[0] = numGdPnts;
                           exter[numExter - 1].numParts = 1;
                        } else if (orient < 0) {
                           /* Could check if there are any self
                            * intersections. */
                           numInter++;
                           inter = (ringType *) realloc ((void *) inter,
                                                         numInter *
                                                         sizeof (ringType));
                           inter[numInter - 1].pnts =
                                 (gdPoint *) malloc (numGdPnts *
                                                     sizeof (gdPoint));
                           inter[numInter - 1].numPnts = numGdPnts;
                           memcpy (inter[numInter - 1].pnts, gdPnts,
                                   numGdPnts * sizeof (gdPoint));
                           inter[numInter - 1].partBeg =
                                 (int *) malloc (1 * sizeof (int));
                           inter[numInter - 1].partBeg[0] = 0;
                           inter[numInter - 1].partLen =
                                 (int *) malloc (1 * sizeof (int));
                           inter[numInter - 1].partLen[0] = numGdPnts;
                           inter[numInter - 1].numParts = 1;
#ifdef DEBUG
/*
                        } else {
                           printf ("Flat\n");
*/
#endif
                        }
                     }
                     numGdPnts = 0;
                     curPart++;
                  }
               }
            }
            /* Finish last part. */
            /* Check if it is properly closed. */
            if ((gdPnts[numGdPnts - 1].x != gdPnts[0].x) ||
                (gdPnts[numGdPnts - 1].y != gdPnts[0].y)) {
               numGdPnts++;
               if (gdPntsLen < numGdPnts) {
                  /* Uses 100 for a buffer so we don't keep realloc'ing. */
                  gdPntsLen = numGdPnts + 100;
                  gdPnts = (gdPoint *) realloc ((void *) gdPnts,
                                                gdPntsLen * sizeof (gdPoint));
               }
               gdPnts[numGdPnts - 1] = gdPnts[0];
            }
            if (numGdPnts >= 4) {
               orient = OrientNumGd (gdPnts, numGdPnts);
               if (orient > 0) {
                  numExter++;
                  exter = (ringType *) realloc ((void *) exter,
                                                numExter * sizeof (ringType));
                  exter[numExter - 1].pnts =
                        (gdPoint *) malloc (numGdPnts * sizeof (gdPoint));
                  exter[numExter - 1].numPnts = numGdPnts;
                  memcpy (exter[numExter - 1].pnts, gdPnts,
                          numGdPnts * sizeof (gdPoint));
                  exter[numExter - 1].partBeg =
                        (int *) malloc (1 * sizeof (int));
                  exter[numExter - 1].partBeg[0] = 0;
                  exter[numExter - 1].partLen =
                        (int *) malloc (1 * sizeof (int));
                  exter[numExter - 1].partLen[0] = numGdPnts;
                  exter[numExter - 1].numParts = 1;
               } else if (orient < 0) {
                  numInter++;
                  inter = (ringType *) realloc ((void *) inter,
                                                numInter * sizeof (ringType));
                  inter[numInter - 1].pnts =
                        (gdPoint *) malloc (numGdPnts * sizeof (gdPoint));
                  inter[numInter - 1].numPnts = numGdPnts;
                  memcpy (inter[numInter - 1].pnts, gdPnts,
                          numGdPnts * sizeof (gdPoint));
                  inter[numInter - 1].partBeg =
                        (int *) malloc (1 * sizeof (int));
                  inter[numInter - 1].partBeg[0] = 0;
                  inter[numInter - 1].partLen =
                        (int *) malloc (1 * sizeof (int));
                  inter[numInter - 1].partLen[0] = numGdPnts;
                  inter[numInter - 1].numParts = 1;
               }
            }
            /* Have converted to GD, storing rings in either inter or exter
             * lists. */

            /* Join inter / exter data. */
            for (i = 0; i < numInter; i++) {
               for (j = 0; j < numExter; j++) {
                  if (wn_PnPoly (inter[i].pnts[0], exter[j]) != 0) {
#ifdef DEBUG
/*
                     printf ("%d is inside %d \n", i, j);
*/
#endif
                     exter[j].numParts++;
                     exter[j].partBeg =
                           (int *) realloc ((void *) exter[j].partBeg,
                                            exter[j].numParts * sizeof (int));
                     exter[j].partBeg[exter[j].numParts - 1] =
                           exter[j].numPnts;
                     exter[j].partLen =
                           (int *) realloc ((void *) exter[j].partLen,
                                            exter[j].numParts * sizeof (int));
                     exter[j].partLen[exter[j].numParts - 1] =
                           inter[i].numPnts;
                     tmpIndex = exter[j].numPnts;
#ifdef STOP_GAP
                     exter[j].numPnts += inter[i].numPnts + 1;
#else
                     exter[j].numPnts += inter[i].numPnts;
#endif
                     exter[j].pnts =
                           (gdPoint *) realloc ((void *) exter[j].pnts,
                                                exter[j].numPnts *
                                                sizeof (gdPoint));
                     memcpy (exter[j].pnts + tmpIndex, inter[i].pnts,
                             inter[i].numPnts * sizeof (gdPoint));
#ifdef STOP_GAP
                     exter[j].pnts[exter[j].numPnts - 1] =
                           exter[j].pnts[tmpIndex - 1];
#endif
                     break;
                  }
               }
            }
            /* Draw exter list of polygons... */
            if (!sym.fg.f_null) {
               for (i = 0; i < numExter; i++) {
                  joinRingParts (&(exter[i]), &polyPnts, &numPolyPnts);
                  gdImageFilledPolygon (im, polyPnts, numPolyPnts,
                                        sym.fg.gdIndex);
/*
                  gdImageFilledPolygon (im, exter[i].pnts, exter[i].numPnts,
                                        sym.fg.gdIndex);
*/
                  free (polyPnts);
               }
            }
            /* Draw exter list of poly-lines. */
            if (!sym.out.f_null) {
               for (i = 0; i < numExter; i++) {
                  for (j = 0; j < exter[i].numParts; j++) {
                     if (sym.thick != 1) {
                        gdImageSetThickness (im, sym.thick);
                     }
                     gdImagePolygon (im, exter[i].pnts + exter[i].partBeg[j],
                                     exter[i].partLen[j], sym.out.gdIndex);
                     if (sym.thick != 1) {
                        gdImageSetThickness (im, 1);
                     }
                  }
               }
            }

            /* Free exter list and inter list. */
            for (i = 0; i < numExter; i++) {
               free (exter[i].pnts);
               free (exter[i].partBeg);
               free (exter[i].partLen);
            }
            free (exter);
            exter = NULL;
            numExter = 0;
            for (i = 0; i < numInter; i++) {
               free (inter[i].pnts);
               free (inter[i].partBeg);
               free (inter[i].partLen);
            }
            free (inter);
            inter = NULL;
            numInter = 0;
            Offset += curRec[1] * 2 + 8;
         }
      } else {
         Offset += curRec[1] * 2 + 8;
         fseek (sfp, Offset, SEEK_SET);
      }
   }

   /* Paint it in white */
   /* Outline it in red; must be done second */
/*
   gdImageSetThickness(im, 4);
   gdImagePolygon(im, points+5, 4, out);
   gdImageSetThickness(im, 2);
   gdImagePolygon(im, points, 4, out);
*/
   free (parts);
   free (points);
   free (gdPnts);
   fclose (sfp);
   return 0;
}

static int DrawLayer (layerType *layer, maparam *map, gdImagePtr im,
                      mapIniType * mapIni, double projX1, double projY1,
                      double A, double Bx, double By, double minLt,
                      double minLg, double maxLt, double maxLg,
                      double X_Size, double Y_Size)
{
   double *Data;
   sInt4 numRec;
   size_t i;
   int ans = 0;
   sInt4 numFDraw = -1;
   char *FDraw;
   double X, Y;
   sInt4 x, y;

#ifdef TEST
   printf ("%s\n", layer->filename);
#endif
   switch (layer->type) {
      case DB2:
         Data = (double *) malloc (layer->numMatchVal * sizeof (double));
         ans = ReadDbfFieldNumMatch (layer->filename, layer->db2.field,
                                     layer->matchField, layer->matchVal,
                                     layer->numMatchVal, Data);
         AllocGDColor (&layer->legend.fg, im);
         AllocGDColor (&layer->legend.bg, im);
         if (ans == 0) {
            DrawDB2 (layer, im, Data);
         }
         free (Data);
         break;
      case GRID:
         myAssert (layer->ramp.numColors >= 0);
         for (i = 0; i < layer->ramp.numColors; i++) {
            AllocGDColor (&layer->ramp.colors[i], im);
         }
         if (layer->ramp.f_missing) {
            AllocGDColor (&layer->ramp.missColor, im);
         }
         if (layer->ramp.thick != 0) {
            AllocGDColor (&layer->ramp.outline, im);
         }
         if (layer->gridData != NULL) {
            DrawGrid (layer, map, im, projX1, projY1, A, Bx, By,
                      mapIni->out.Y_Size, &(layer->grad));
         } else {

         }
         break;
      case INFO:
         for (i = 0; i < layer->numPnt; i++) {
            cll2xy (map, layer->pnt[i].lat, layer->pnt[i].lon, &X, &Y);
            x = (sInt4) (Bx + (X - projX1) / A);
            y = (sInt4) (mapIni->out.Y_Size - (By + (Y - projY1) / A));
            printf ("Info: %f %f => %ld %ld\n", layer->pnt[i].lat,
                    layer->pnt[i].lon, x, y);
         }
         break;
      case LATTICE:
         AllocGDColor (&layer->lattice.fg, im);
         DrawLattice(layer, map, im, mapIni, projX1, projY1, A, Bx, By,
                     minLg, minLt, maxLg, maxLt, X_Size, Y_Size,
                     layer->lattice.spacing, layer->lattice.style,
                     layer->lattice.labelSite);
         break;
      case GRADUATED:
         /* We should read in the .dbf file first. */
         for (i = 0; i < layer->grad.numSymbol; i++) {
            AllocGDColor (&layer->grad.symbol[i].fg, im);
            AllocGDColor (&layer->grad.symbol[i].out, im);
         }
         numFDraw = -1;
         if (layer->matchField != NULL) {
            if (layer->grad.field != NULL) {
               if (strcmp (layer->matchField, layer->grad.field) == 0) {
                  ReadDbfFieldMatchRec (layer->filename, layer->matchField,
                                        layer->matchVal, layer->numMatchVal,
                                        &FDraw, &numFDraw);
               }
            } else {
               ReadDbfFieldMatchRec (layer->filename, layer->matchField,
                                     layer->matchVal, layer->numMatchVal,
                                     &FDraw, &numFDraw);
            }
         }
         Data = NULL;
         ans = ReadDbfFieldNum (layer->filename, layer->grad.field, &Data,
                                &numRec);
#ifdef TEST
         printf ("numRec = %ld ans = %d\n", numRec, ans);
#endif
         if (ans == 0) {
            if (layer->shpType == POINT) {
               ans = DrawGradPointShpFile (layer->filename, map, im,
                                           &(layer->grad), mapIni, projX1,
                                           projY1, A, Bx, By, Data, numRec,
                                           FDraw, numFDraw);
            } else if (layer->shpType == POLYGON) {
               DrawGradPolyShpFile (layer->filename, map, im, &(layer->grad),
                                    mapIni, projX1, projY1, A, Bx, By, Data,
                                    numRec, minLt, minLg, maxLt, maxLg,
                                    FDraw, numFDraw);
            } else {
               printf ("ERROR: !!!Can't do that yet.!!!\n");
            }
         }
         free (Data);
         break;
      case SINGLE_SYMBOL:
         AllocGDColor (&layer->single.fg, im);
         AllocGDColor (&layer->single.out, im);
         if (layer->matchField != NULL) {
            ReadDbfFieldMatchRec (layer->filename, layer->matchField,
                                  layer->matchVal, layer->numMatchVal,
                                  &FDraw, &numFDraw);
         } else {
            numFDraw = -1;
         }
         if (layer->shpType == POLYGON) {
            DrawPolyShpFile (layer->filename, map, im, layer->single, mapIni,
                             projX1, projY1, A, Bx, By, minLt, minLg,
                             maxLt, maxLg, FDraw, numFDraw);
         } else if (layer->shpType == POINT) {
            DrawPointShpFile (layer->filename, map, im, layer->single,
                              mapIni, projX1, projY1, A, Bx, By, FDraw,
                              numFDraw);
         }
         break;
   }
   if (ans != 0) {
      return ans;
   }
   if (layer->type == PNG) {
      DrawPng (layer->filename, im, layer->title.x, layer->title.y);
   } else if (layer->title.name != NULL) {
      AllocGDColor (&layer->title.fg, im);
      AllocGDColor (&layer->title.bg, im);
      if (layer->title.textC.r != -1) {
         AllocGDColor (&layer->title.textC, im);
      }
      DrawTitle (layer, im);
#ifdef TEST
      printf ("%s\n", layer->title.name);
#endif
   }
   return ans;
}

typedef struct {
   gdImagePtr imG;      /* Declare the general image. */
   gdImagePtr imL;      /* Declare the local image. */
   int x0, y0;
   int sizeX, sizeY;
   maparam map;
   double A, Bx, By;
   double minLt, minLg, maxLt, maxLg;
   double projX1, projY1;
} gdFrameType;

static void SetMapParam (maparam *map, mapIniType * mapIni, zoomType *zoom,
                         int sizeX, int sizeY, double *A, double *Bx,
                         double *By, double *minLt, double *minLg,
                         double *maxLt, double *maxLg, double *projX1,
                         double *projY1)
{
   double projX2, projY2;
   double d_temp;
   double DelX, DelY;
   double mesh;

   mkGeoid (map, AB, mapIni->proj.majEarth, mapIni->proj.majEarth);
   stlmbr (map, eqvlat (map, mapIni->proj.scaleLat1, mapIni->proj.scaleLat2),
           mapIni->proj.orientLon);

   /* Don't actually need to scale the projection correctly, since we are
    * going to scale this to the screen in the next step. */
   /* need km */
   mesh = mapIni->proj.mesh / 1000.;
   stcm1p (map, 1, 1, mapIni->proj.lat1, mapIni->proj.lon1,
           mapIni->proj.meshLat, mapIni->proj.orientLon, mesh, 0);
/*
   stcm1p (map, 1, 1, mapIni->proj.lat1, mapIni->proj.lon1, 0,
           mapIni->proj.orientLon, 1, 0);
*/

   /* Set up XY to projected space computations. */
   cll2xy (map, zoom->lat1, zoom->lon1, projX1, projY1);
   cll2xy (map, zoom->lat2, zoom->lon2, &projX2, &projY2);

   if (*projX1 > projX2) {
      d_temp = *projX1;
      *projX1 = projX2;
      projX2 = d_temp;
   }
   if (*projY1 > projY2) {
      d_temp = *projY1;
      *projY1 = projY2;
      projY2 = d_temp;
   }
   DelX = projX2 - *projX1;
   DelY = projY2 - *projY1;
   if ((DelX / (double) sizeX) > (DelY / (double) sizeY)) {
      *A = DelX / (double) sizeX;
      *Bx = 0;
      *By = (sizeY - DelY / *A) / 2.;
   } else {
      *A = DelY / (double) sizeY;
      *Bx = (sizeX - DelX / *A) / 2.;
      *By = 0;
   }

   FindBounds (map, sizeX, sizeY, *A, *Bx, *By, *projX1,
               *projY1, mapIni->proj.orientLon, minLt, minLg, maxLt, maxLg);
}

void ControlDraw (mapIniType * mapIni)
{
   gdImagePtr master;
   gdFrameType *gdFrame;
   int numGdFrame;
   FILE *pngout;        /* Declare output files */
   int i, k, m;
   size_t j;
   int f_break;
   int frameNum;

   /* Set up the image... */
   master = gdImageCreate (mapIni->out.X_Size, mapIni->out.Y_Size);
/*   AllocGDColor (&mapIni.all.bg, imG);*/

   if (mapIni->out.numFrame == 0) {
      numGdFrame = 1;
      gdFrame = (gdFrameType *) malloc (numGdFrame * sizeof (gdFrameType));
      gdFrame[0].x0 = 0;
      gdFrame[0].y0 = 0;
      gdFrame[0].sizeX = mapIni->out.X_Size;
      gdFrame[0].sizeY = mapIni->out.Y_Size;
      gdFrame[0].imG = gdImageCreate (gdFrame[0].sizeX, gdFrame[0].sizeY);
      AllocGDColor (&mapIni->all.bg, gdFrame[0].imG);
      /* ------------------ */
      /* Set up map projection. */
      /* ------------------ */
      SetMapParam (&gdFrame[0].map, mapIni, &(mapIni->zoom),
                   gdFrame[0].sizeX, gdFrame[0].sizeY, &gdFrame[0].A,
                   &gdFrame[0].Bx, &gdFrame[0].By, &gdFrame[0].minLt,
                   &gdFrame[0].minLg, &gdFrame[0].maxLt, &gdFrame[0].maxLg,
                   &gdFrame[0].projX1, &gdFrame[0].projY1);
   } else {
      numGdFrame = mapIni->out.numFrame;
      gdFrame = (gdFrameType *) malloc (numGdFrame * sizeof (gdFrameType));
      for (m = 0; m < numGdFrame; m++) {
         gdFrame[m].x0 = mapIni->out.frame[m].X0;
         gdFrame[m].y0 = mapIni->out.frame[m].Y0;
         gdFrame[m].sizeX = mapIni->out.frame[m].X_Size;
         gdFrame[m].sizeY = mapIni->out.frame[m].Y_Size;
         gdFrame[m].imG = gdImageCreate (gdFrame[m].sizeX, gdFrame[m].sizeY);
         AllocGDColor (&mapIni->out.frame[m].bg, gdFrame[m].imG);
         /* ------------------ */
         /* Set up map projection. */
         /* ------------------ */
         if (mapIni->out.frame[m].f_zoom) {
            SetMapParam (&gdFrame[m].map, mapIni,
                         &(mapIni->out.frame[m].zoom), gdFrame[m].sizeX,
                         gdFrame[m].sizeY, &gdFrame[m].A, &gdFrame[m].Bx,
                         &gdFrame[m].By, &gdFrame[m].minLt,
                         &gdFrame[m].minLg, &gdFrame[m].maxLt,
                         &gdFrame[m].maxLg, &gdFrame[m].projX1,
                         &gdFrame[m].projY1);
         } else {
            SetMapParam (&gdFrame[m].map, mapIni, &(mapIni->zoom),
                         gdFrame[m].sizeX, gdFrame[m].sizeY, &gdFrame[m].A,
                         &gdFrame[m].Bx, &gdFrame[m].By, &gdFrame[m].minLt,
                         &gdFrame[m].minLg, &gdFrame[m].maxLt,
                         &gdFrame[m].maxLg, &gdFrame[m].projX1,
                         &gdFrame[m].projY1);
/*
printf ("Why is this needed here?\n");
*/
            gdFrame[m].By += mapIni->out.Y_Size - gdFrame[m].sizeY;
         }
      }
   }
#ifdef DEBUG
   printf ("Begin Drawing now. %f\n", clock () / (double) (CLOCKS_PER_SEC));
#endif

   /* ------------------ */
   /* Begin drawing now. */
   /* ------------------ */
   myAssert (numGdFrame > 0);
   /* Do generic draws now.  */
   for (j = 0; j < mapIni->out.numActive; j++) {
      k = mapIni->out.active[j] - 1;
      if ((k >= 0) && (k < mapIni->all.numLayer)) {
         if ((mapIni->all.layers[k].frameNum == -1) ||
             (mapIni->all.layers[k].frameNum > numGdFrame)) {
            frameNum = 0;
         } else {
            frameNum = mapIni->all.layers[k].frameNum;
         }
         DrawLayer (&(mapIni->all.layers[k]), &gdFrame[frameNum].map,
                    gdFrame[frameNum].imG, mapIni, gdFrame[frameNum].projX1,
                    gdFrame[frameNum].projY1, gdFrame[frameNum].A,
                    gdFrame[frameNum].Bx, gdFrame[frameNum].By,
                    gdFrame[frameNum].minLt, gdFrame[frameNum].minLg,
                    gdFrame[frameNum].maxLt, gdFrame[frameNum].maxLg,
                    gdFrame[frameNum].sizeX, gdFrame[frameNum].sizeY);
      }
   }
#ifdef DEBUG
   printf ("Done with Generic draws. %f\n", clock () /
           (double) (CLOCKS_PER_SEC));
#endif

   for (i = 0; i < mapIni->out.numOutputs; i++) {
      f_break = 0;
      if (mapIni->out.outputs[i].f_valid) {
         for (m = 0; m < numGdFrame; m++) {
            gdFrame[m].imL = gdImageCreate (gdFrame[m].sizeX,
                                            gdFrame[m].sizeY);
            gdImageCopy (gdFrame[m].imL, gdFrame[m].imG, 0, 0, 0, 0,
                         gdFrame[m].sizeX, gdFrame[m].sizeY);
         }

         /* Draw Layers. */
         for (j = 0; j < mapIni->out.outputs[i].numActive; j++) {
            k = mapIni->out.outputs[i].active[j] - 1;
            if ((k >= 0) && (k < mapIni->all.numLayer)) {
               if ((mapIni->all.layers[k].frameNum == -1) ||
                   (mapIni->all.layers[k].frameNum > numGdFrame)) {
                  frameNum = 0;
               } else {
                  frameNum = mapIni->all.layers[k].frameNum;
               }
               if (DrawLayer (&(mapIni->all.layers[k]),
                              &gdFrame[frameNum].map, gdFrame[frameNum].imL,
                              mapIni, gdFrame[frameNum].projX1,
                              gdFrame[frameNum].projY1, gdFrame[frameNum].A,
                              gdFrame[frameNum].Bx, gdFrame[frameNum].By,
                              gdFrame[frameNum].minLt,
                              gdFrame[frameNum].minLg,
                              gdFrame[frameNum].maxLt,
                              gdFrame[frameNum].maxLg,
                              gdFrame[frameNum].sizeX,
                              gdFrame[frameNum].sizeY) != 0) {
                  f_break = 1;
               }
            }
         }
         /* Draw Legends. (Used to be in DrawLayer). */
         for (j = 0; j < mapIni->out.outputs[i].numActive; j++) {
            k = mapIni->out.outputs[i].active[j] - 1;
            if ((k >= 0) && (k < mapIni->all.numLayer)) {
               if (mapIni->all.layers[k].legend.f_valid) {
                  if ((mapIni->all.layers[k].legendFrameNum == -1) ||
                      (mapIni->all.layers[k].legendFrameNum > numGdFrame)) {
                     frameNum = mapIni->all.layers[k].frameNum;
                     if ((frameNum == -1) || (frameNum > numGdFrame)) {
                        frameNum = 0;
                     }
                  } else {
                     frameNum = mapIni->all.layers[k].legendFrameNum;
                  }
                  AllocGDColor (&(mapIni->all.layers[k].legend.fg),
                                gdFrame[frameNum].imL);
                  AllocGDColor (&(mapIni->all.layers[k].legend.bg),
                                gdFrame[frameNum].imL);
                  if (mapIni->all.layers[k].legend.textC.r != -1) {
                     AllocGDColor (&(mapIni->all.layers[k].legend.textC),
                                   gdFrame[frameNum].imL);
                  }
                  DrawLegend (&(mapIni->all.layers[k]),
                              gdFrame[frameNum].imL);
               }
            }
         }

         /* ------------------ */
         /* Done with drawing... save . */
         /* ------------------ */
         if (!f_break) {
            if (strcmp (mapIni->out.outputs[i].filename, "-stdout") == 0) {
               for (m = 0; m < numGdFrame; m++) {
                  gdImageCopy (master, gdFrame[m].imL, gdFrame[m].x0,
                               gdFrame[m].y0, 0, 0, gdFrame[m].sizeX,
                               gdFrame[m].sizeY);
               }
/*#ifdef WIN32*/
#ifdef _WINDOWS_
               /* Set stdout to binary mode (the 1 means stdout) */
               setmode (1, O_BINARY);
#endif
               gdImagePng (master, stdout);
            } else {
               if ((pngout = fopen (mapIni->out.outputs[i].filename,
                                    "wb")) == NULL) {
                  printf ("Problems opening %s for write\n",
                          mapIni->out.outputs[i].filename);
               } else {
                  for (m = 0; m < numGdFrame; m++) {
                     gdImageCopy (master, gdFrame[m].imL, gdFrame[m].x0,
                                  gdFrame[m].y0, 0, 0, gdFrame[m].sizeX,
                                  gdFrame[m].sizeY);
                  }
                  gdImagePng (master, pngout);
                  fclose (pngout);
               }
            }
         }
         for (m = 0; m < numGdFrame; m++) {
            gdImageDestroy (gdFrame[m].imL);
         }
      }
   }
   for (m = 0; m < numGdFrame; m++) {
      gdImageDestroy (gdFrame[m].imG);
   }
   free (gdFrame);
   gdImageDestroy (master);
}
