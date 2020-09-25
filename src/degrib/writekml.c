/* #include <stdio.h> */
#include <string.h>
#include <stdlib.h> 
#include "mymapf.h"
/* #include "tendian.h" */
#include "write.h"
#include "myerror.h"
#include "myutil.h"
#include "myassert.h"
#include <ctype.h>
#include "mapini.h"
/* #include <errno.h> */
/* #include "type.h" */
/* #include "weather.h" */
#include "chain.h"
/* #include <math.h> */
#include "clock.h"
#include "myzip.h"

#ifdef USE_XMLLIB
/* Either one. */
#include <libxml/parser.h>
/* #include <libxml/tree.h> */
#endif

#ifdef EXPERIMENT
typedef struct {
   float x, y;
} polyPointType;


typedef struct {
   double value;
   polyPointType **data;
   int numChunk;
   int *chunkLen;
   int *f_hole;
} polyType2;


static inline int isLeft (polyPointType P0, polyPointType P1, polyPointType P2)
{
   return ( (P1.x - P0.x) * (P2.y - P0.y)
          - (P2.x - P0.x) * (P1.y - P0.y) );
}

static int orientation2D_Polygon (int n, polyPointType * V)
{
   /* first find rightmost lowest vertex of the polygon */
   int rmin = 0;
   int xmin = V[0].x;
   int ymin = V[0].y;

   for (int i=1; i<n; i++) {
      if (V[i].y > ymin)
         continue;
      if (V[i].y == ymin) {    /* just as low */
         if (V[i].x < xmin)   /* and to left */
            continue;
      }
      rmin = i;          /* a new rightmost lowest vertex */
      xmin = V[i].x;
      ymin = V[i].y;
   }

   /* test orientation at this rmin vertex */
   /* ccw <=> the edge leaving is left of the entering edge */
   if (rmin == 0)
      return isLeft( V[n-1], V[0], V[1] );
   else
      return isLeft( V[rmin-1], V[rmin], V[rmin+1] );
}

static void chain2chunk (polyType *poly, polyType2 *chunk, int f_fin)
{
   if (f_fin) {
      chunk->numChunk = poly->numFin;
   } else {
      chunk->numChunk = poly->numAct;
   }
   chunk->chunkLen = (int *) malloc (chunk->numChunk * sizeof (int));
   chunk->f_hole = (int *) malloc (chunk->numChunk * sizeof (int));
   chunk->data = (PolyPointType **) malloc (chunk->numChunk *
                                            sizeof (PolyPointType *));
   for (j = 0; j < chunk->numChunk; j++) {
      if (f_fin) {
         node = poly[i].finList[j].head;
      } else {
         node = poly[i].actList[j].head;
      }
      cnt = 0;
      while (node != NULL) {
         cnt++;
         node = node->next;
      }
      chunk->chunkLen[j] = cnt;
      chunk->data[j] = (PolyPointType *) malloc (chunk->chunkLen[j] *
                                                 sizeof (PolyPointType));
      if (f_fin) {
         node = poly[i].finList[j].head;
      } else {
         node = poly[i].actList[j].head;
      }
      cnt = 0;
      while (node != NULL) {
         chunk->data[j][cnt].x = node->x;
         chunk->data[j][cnt].y = node->y;
         cnt++;
         node = node->next;
      }
      chunk->f_hole[j] = orientation2D_Polygon (chunk->chunkLen[j],
                                                chunk->data[j]);
   }
}
#endif

static void PrintPolys (polyType *poly, int numPoly)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   int cnt;             /* Check if we have > 100 nodes. */
   chainNode *node;     /* The current node (to print). */
   chainNode *next;     /* The current node (to print). */
   double A;            /* area of polygon. */

   printf ("Number of poly %d\n", numPoly);
   for (i = 0; i < numPoly; i++) {
      printf ("Poly number %d (value = %f)\n", i, poly[i].value);
      myAssert (poly[i].numFin == 0);
      for (j = 0; j < poly[i].numAct; j++) {
         printf ("  %d :: ", j);
         node = poly[i].actList[j].head;
         next = node->next;
         cnt = 0;
         A = 0;
         while (node != NULL) {
            cnt++;
            if (next != NULL) {
               A += node->x * next->y - node->y * next->x;
            }
            printf ("(%f %f) ,", node->x, node->y);
            if (node == poly[i].actList[j].tail) {
               printf (":: NULL? ");
            }
            node = node->next;
            if (node != NULL) {
               next = node->next;
            } else {
               next = NULL;
            }
         }
         printf ("<check> \n Area = %f", A);
      }
   }
}

/* http://www.faqs.org/faqs/graphics/algorithms-faq/ */
/* 2.03 */
static int pnpoly (int npol, float *xp, float *yp, float x, float y)
{
   int i, j, c = 0;
   for (i = 0, j = npol-1; i < npol; j = i++) {
      if ((((yp[i] <= y) && (y < yp[j])) ||
           ((yp[j] <= y) && (y < yp[i]))) &&
          (x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]))
         c = !c;
   }
   return c;
}

/* NOT EFFICIENT!!! */
static int pnpoly2 (chainNode *head, float x, float y)
{
   float *xp;
   float *yp;
   int cnt;
   int ans;
   chainNode *node;

   node = head;
   cnt = 0;
   while (node != NULL) {
      cnt++;
      node = node->next;
   }
   xp = (float *) malloc (cnt * sizeof (float));
   yp = (float *) malloc (cnt * sizeof (float));

   node = head;
   cnt = 0;
   while (node != NULL) {
      xp[cnt] = node->x;
      yp[cnt] = node->y;
      cnt++;
      node = node->next;
   }
   ans = pnpoly (cnt, xp, yp, x, y);

   free (xp);
   free (yp);
   return ans;
}

/* Still doesn't handle island inside a lake on an island */

/* http://www.faqs.org/faqs/graphics/algorithms-faq/ */
/* 2.07, 2.01 */
static void FindAreas (polyType *poly, int numPoly)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   int k;
   int cnt;             /* Check if we have > 100 nodes. */
   chainNode *node;     /* The current node (to print). */
   chainNode *next;     /* The current node (to print). */
   double A;            /* area of polygon. */
   int f_found;

   for (i = 0; i < numPoly; i++) {
      myAssert (poly[i].numFin == 0);
      if (poly[i].crossLink == NULL) {
         poly[i].crossLink = (int *) malloc (poly[i].numAct * sizeof (int));
      }
      for (j = 0; j < poly[i].numAct; j++) {
         node = poly[i].actList[j].head;
         next = node->next;
         cnt = 0;
         A = 0;
         while (node != NULL) {
            cnt++;
            if (next != NULL) {
               A += node->x * next->y - node->y * next->x;
            }
            node = node->next;
            if (node != NULL) {
               next = node->next;
            } else {
               next = NULL;
            }
         }
         if (A >= 0) {
/*            printf ("Hole %d %d: Area %f\n", i, j, A);*/
            f_found = -1;
            for (k = 0; k < poly[i].numAct; k++) {
               if (k != j) {
                  if (pnpoly2 (poly[i].actList[k].head,
                               poly[i].actList[j].head->x,
                               poly[i].actList[j].head->y)) {
                     if (f_found == -1) {
                        f_found = k;
                     } else {
                        printf ("Hole inside 2 or more parents.\n");
                     }
                  }
               }
            }
            if (f_found == -1) {
               printf ("Hole without parent?\n");
            } else {
               poly[i].crossLink[j] = f_found;
/*               printf ("Cross link Hole %d with parent %d?\n", j, k);*/
            }
         } else {
            poly[i].crossLink[j] = -1;
         }
      }
   }
}

static void PrintPolys2 (polyType *poly, int numPoly)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   int cnt;             /* Check if we have > 100 nodes. */
   chainNode *node;     /* The current node (to print). */

   printf ("Number of poly %d\n", numPoly);
   for (i = 0; i < numPoly; i++) {
      printf ("Poly number %d (value = %f)\n", i, poly[i].value);
      myAssert (poly[i].numAct == 0);
      for (j = 0; j < poly[i].numFin; j++) {
         printf ("  %d :: ", j);
         node = poly[i].finList[j].head;
         cnt = 0;
         while (node != NULL) {
            cnt++;
            printf ("(%f %f) ,", node->x, node->y);
            if (node == poly[i].finList[j].tail) {
               printf (":: NULL? ");
            }
            node = node->next;
            if (cnt == 100)
               return;
         }
         printf ("<check> \n");
      }
   }
}

typedef struct {
   char *label;
   char *value;
} TxtPair;

static int ParseKMLIniFile (const char *kmlIni, char *variable,
                            SymbolType **symbol, int *numSymbol,
                            TxtPair **desc, int *numDesc)
{
   FILE *fp;
   char *line = NULL;
   size_t lineLen = 0;
   char *first;
   char *second;
   char *third;
   int i;
   int f_desc = 0;
   int f_validVar;
   int f_EOF;
   int f_foundVar = 0;

   myAssert (*numDesc == 0);
   myAssert (*desc == NULL);
   if (*numSymbol != 0) {
      for (i=0; i < *numSymbol; i++) {
         freeIniSymbol (& (*symbol)[i]);
      }
      free (*symbol);
   }
   *numSymbol = 0;
   *symbol = NULL;

   if ((fp = fopen (kmlIni, "rt")) == NULL) {
      printf ("Unable to read %s\n", kmlIni);
      return -1;
   }
   /* Make sure version number is correct. */
   while (reallocFGets (&line, &lineLen, fp) > 0) {
      first = line;
      while ((isspace (*first)) && (*first != '\0')) {
         first++;
      }
      if ((first != NULL) && (*first != '#')) {
         strTrim (line);
         if (strcmp (line, "TYPE=DEGRIB-KMLINI-1.0") != 0) {
            printf ("Bad kmlIni version information\n");
            fclose (fp);
            free (line);
            return -1;
         } else {
            break;
         }
      }
   }
   /* Read file. */
   f_validVar = 0;
   while (reallocFGets (&line, &lineLen, fp) > 0) {
      first = line;
      while ((isspace (*first)) && (*first != '\0')) {
         first++;
      }
      if ((*first == '\0') || (*first == '#')) {
         continue;
      }
      if (*first == '<') {
         first ++;
         second = strchr (first, '>');
         if (second == NULL) {
            printf ("Bad kmlIni section info\n"); fclose (fp); free (line);
            return -1;
         }
         *second = '\0';
         if (strncmp (first, "Variable", 8) == 0) {
            third = strchr (first, '=');
            if (third != NULL) {
               if (strcmp (third + 1, variable) == 0) {
                  f_validVar = 1;
                  f_foundVar = 1;
               }
            } else {
               f_validVar = 1;
               f_foundVar = 1;
            }
         } else if (strcmp (first, "/Variable") == 0) {
            if (f_validVar) {
               break;
            }
         } else if (f_validVar) {
            if (strcmp (first, "description") == 0) {
               f_desc = 1;
            } else if (strcmp (first, "/description") == 0) {
               f_desc = 0;
            }
         }
      } else if (f_validVar) {
         second = strchr (first, '=');
         if (second == NULL) {
            printf ("Bad kmlIni Symbol info\n"); fclose (fp); free (line);
            return -1;
         }
         *second = '\0';
         second++;
         if (f_desc) {
            if (*second != '[') {
               printf ("values should start with a [\n"); fclose (fp);
               free (line); return -1;
            }
            second++;
            *numDesc = *numDesc + 1;
            *desc = (TxtPair *) realloc ((void *) *desc,
                                         *numDesc * sizeof (TxtPair));
            (*desc)[*numDesc - 1].label = malloc ((strlen (first) + 1) * sizeof (char));
            strcpy ((*desc)[*numDesc - 1].label, first);
            third = strchr (second, ']');
            if (third != NULL) {
               *third = '\0';
            }
            (*desc)[*numDesc - 1].value = malloc ((strlen (second) + 1) * sizeof (char));
            strcpy ((*desc)[*numDesc - 1].value, second);
            f_EOF = 0;
            while (!f_EOF && (third == NULL)) {
               if (reallocFGets (&line, &lineLen, fp) == 0) {
                  f_EOF = 1;
               } else {
                  third = strchr (second, ']');
                  if (third != NULL) {
                     *third = '\0';
                  }
                  (*desc)[*numDesc - 1].value = realloc ((void *) (*desc)[*numDesc - 1].value,
                                                         (strlen ((*desc)[*numDesc - 1].value) +
                                                          strlen (line) + 1) * sizeof (char));
                  strcat ((*desc)[*numDesc - 1].value, line);
               }
            }

         } else {
            if (strcmp (first, "Symbol") != 0) {
               printf ("Bad kmlIni Symbol info\n"); fclose (fp); free (line);
               return -1;
            }
            *numSymbol = *numSymbol + 1;
            *symbol = (SymbolType *) realloc ((void *) *symbol,
                                              *numSymbol * sizeof (SymbolType));
            third = strchr (second, '#');
            if (third != NULL) {
               *third = '\0';
            }
            third = strchr (second, '\n');
            if (third != NULL) {
               *third = '\0';
            }
            ParseSymbol (&((*symbol)[*numSymbol - 1]), second);
         }
      }
   }
   fclose (fp);
   free (line);
   if (! f_foundVar) {
      printf ("Unable to find a <Variable> or <Variable name=%s> section\n",
              variable);
      return -1;
   }
   return 0;
}

/* In tests with psurge data, found this to be 2x faster:
 * on "fortune (my machine at home)" it took: 427 sec to use xml library,
 * and only 272 sec to use fprintf's */
/* Note file size is smaller by about 1 meg because of the indentation
 * which xmllib does, but isn't needed. */
#define KML_NEWLINE myZip_fputs ("\n", zp);
static int savePolysKmlFast (char *filename, polyType *poly, int numPoly,
                             grib_MetaData *meta, sChar decimal,
                             sChar LatLon_Decimal, SymbolType *symbol,
                             int numSymbol, int f_kmz, TxtPair *desc,
                             int numDesc, char f_reduce)
{
   myZipFile *zp;
   char buf[256];
   char buf2[256];
   char valTime[25];
   char title[256];
   char styleUrl[256];
   int styleIndex;
   chainNode *Pnode;     /* The current node (to print). */
   int i, j, k;
   sInt4 deltSec;
   char rangeTxt[100];
   char *ptr;

   if (f_kmz) {
      strncpy (filename + strlen (filename) - 3, "kmz", 3);
   }
   if ((zp = myZipInit (filename, f_kmz, 0)) == NULL) {
      printf ("error opening %s\n", filename);
      return -1;
   }
   strncpy (filename + strlen (filename) - 3, "kml", 3);
   if ((ptr = strrchr (filename, '/')) == NULL) {
      if ((ptr = strrchr (filename, '\\')) == NULL) {
         ptr = filename;
      } else {
         ptr++;
      }
   } else {
      ptr++;
   }
   if (myZip_fopen (zp, ptr, "wb", 2009, 0, 16, 1, 58, 59)) {
      printf ("Problems opening file %s\n", filename);
      return -1;
   }
   Clock_Print (valTime, 25, meta->pds2.sect4.validTime, "%m/%d/%Y %H:%M:%S UTC", 0);

   myZip_fputs ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", zp); KML_NEWLINE
   myZip_fputs ("<kml xmlns=\"http://www.opengis.net/kml/2.2\">", zp); KML_NEWLINE
   myZip_fputs ("<Document>", zp); KML_NEWLINE

   myZip_fputs ("<name>", zp);
   if (meta->unitName != NULL) {
      sprintf (title, "%s %s, %s", meta->element, meta->unitName, valTime);
   } else {
      sprintf (title, "%s, %s", meta->element, valTime);
   }
   sprintf (buf, "<![CDATA[%s]]>", title); myZip_fputs (buf, zp);
   myZip_fputs ("</name>", zp); KML_NEWLINE

   myZip_fputs ("<open>1</open>", zp); KML_NEWLINE

   myZip_fputs ("<description>", zp);
   myZip_fputs ("<![CDATA[<TABLE>\n", zp);
   if (meta->convert != UC_NONE) {
      sprintf (buf, "<TR><TD><B>Title</B></TD><TD><B>%s (converted to %s)"
               "</B></TD></TR>\n", meta->comment, meta->unitName);
      myZip_fputs (buf, zp);

   } else {
      sprintf (buf, "<TR><TD><B>Title</B></TD><TD><B>%s</B></TD></TR>\n",
               meta->comment); myZip_fputs (buf, zp);
   }

   for (i = 0; i < numDesc; i++) {
      printf ("%s :: %s\n", desc[i].label, desc[i].value);
      sprintf (buf, "<TR><TD><B>"); myZip_fputs (buf, zp);
      myZip_fputs (desc[i].label, zp);
      sprintf (buf, "</B></TD><TD><B>"); myZip_fputs (buf, zp);
      myZip_fputs (desc[i].value, zp);
      sprintf (buf, "</B></TD></TR>\n"); myZip_fputs (buf, zp);
   }

   Clock_Print (buf2, 25, meta->pds2.refTime, "%m/%d/%Y %H:%M:%S UTC", 0);
   sprintf (buf, "<TR><TD><B>Date Created</B></TD><TD><B>%s</B></TD></TR>\n",
            buf2); myZip_fputs (buf, zp);
   sprintf (buf, "<TR><TD><B>Valid</B></TD><TD><B>%s</B></TD></TR>\n",
            valTime); myZip_fputs (buf, zp);
   sprintf (buf, "<TR><TD><B>Surface</B></TD><TD><B>%s</B></TD></TR>\n",
            meta->longFstLevel); myZip_fputs (buf, zp);
   myZip_fputs ("</TABLE>]]>", zp);

/*
   sprintf (buf, "<![CDATA[%s\n", meta->comment); myZip_fputs (buf, zp);
   if (meta->convert != UC_NONE) {
      sprintf (buf, "Converted to %s\n", meta->unitName); myZip_fputs (buf, zp);
   }
   Clock_Print (buf2, 25, meta->pds2.refTime, "%m/%d/%Y %H:%M:%S UTC", 0);
   sprintf (buf, "Made %s\n", buf2); myZip_fputs (buf, zp);
   sprintf (buf, "Valid %s\n", valTime); myZip_fputs (buf, zp);
   sprintf (buf, "Surface: %s]]>", meta->longFstLevel); myZip_fputs (buf, zp);
*/
   myZip_fputs ("</description>", zp); KML_NEWLINE

   /* Try to handle the time stamp. */
   if (meta->GribVersion == 2) {
      switch (meta->pds2.sect4.templat) {
         case GS4_STATISTIC:
         case GS4_PROBABIL_TIME:
         case GS4_PERCENT_TIME:
         case GS4_ENSEMBLE_STAT:
         case GS4_DERIVED_INTERVAL:
            myZip_fputs ("<TimeSpan>", zp); KML_NEWLINE
            deltSec = meta->pds2.sect4.Interval[0].lenTime;
            if (meta->pds2.sect4.Interval[0].timeRangeUnit == 0) {
               deltSec *= 60;
            } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 1) {
               deltSec *= 3600;
            } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 2) {
               deltSec *= 24 * 3600;
            } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 10) {
               deltSec *= 3 * 3600;
            } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 11) {
               deltSec *= 6 * 3600;
            } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 12) {
               deltSec *= 12 * 3600;
            } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 13) {
               /* Already in seconds. */
            } else {
               /* can't handle month, year, decade, normal, century or
                * reserved */
               deltSec = 0;
            }
            Clock_Print (buf2, 256, meta->pds2.sect4.validTime - deltSec,
                         "%Y-%m-%dT%H:%M:%S", 0);
            sprintf (buf, "<begin>%s</begin>", buf2); myZip_fputs (buf, zp);
            KML_NEWLINE
            Clock_Print (buf2, 256, meta->pds2.sect4.validTime,
                         "%Y-%m-%dT%H:%M:%S", 0);
            sprintf (buf, "<end>%s</end>", buf2); myZip_fputs (buf, zp);
            KML_NEWLINE
            myZip_fputs ("</TimeSpan>", zp); KML_NEWLINE
            break;
         default:
            myZip_fputs ("<TimeStamp>", zp); KML_NEWLINE
            Clock_Print (buf2, 256, meta->pds2.sect4.validTime,
                         "%Y-%m-%dT%H:%M:%S", 0);
            sprintf (buf, "<when>%s</when>", buf2); myZip_fputs (buf, zp);
            KML_NEWLINE
            myZip_fputs ("</TimeStamp>", zp); KML_NEWLINE
            break;
      }
   }

   /* Work on Symbol table. */
   for (i=0; i < numSymbol; i++) {
      sprintf (buf, "<Style id=\"%d\">", i); myZip_fputs (buf, zp);
      KML_NEWLINE
      myZip_fputs ("<LineStyle>", zp); KML_NEWLINE
      sprintf (buf2, "%02X%02X%02X%02X", symbol[i].out.alpha,
               symbol[i].out.b, symbol[i].out.g, symbol[i].out.r);
      sprintf (buf, "<color>%s</color>", buf2); myZip_fputs (buf, zp);
      KML_NEWLINE
      sprintf (buf, "<width>%3.1f</width>", symbol[i].thick / 10.);
      myZip_fputs (buf, zp); KML_NEWLINE
      myZip_fputs ("</LineStyle>", zp); KML_NEWLINE
      myZip_fputs ("<PolyStyle>", zp); KML_NEWLINE
      if (symbol[i].out.f_null) {
         myZip_fputs ("<outline>0</outline>", zp); KML_NEWLINE
      } else {
         myZip_fputs ("<outline>1</outline>", zp); KML_NEWLINE
      }
      if (symbol[i].fg.f_null) {
         myZip_fputs ("<fill>0</fill>", zp); KML_NEWLINE
      } else {
         myZip_fputs ("<fill>1</fill>", zp); KML_NEWLINE
      }
      sprintf (buf2, "%02X%02X%02X%02X", symbol[i].fg.alpha,
               symbol[i].fg.b, symbol[i].fg.g, symbol[i].fg.r);
      sprintf (buf, "<color>%s</color>", buf2); myZip_fputs (buf, zp);
      KML_NEWLINE
      myZip_fputs ("</PolyStyle>", zp); KML_NEWLINE
      myZip_fputs ("</Style>", zp); KML_NEWLINE
   }

   /* Work on Dataset. */
   myZip_fputs ("<Folder>", zp); KML_NEWLINE
   sprintf (buf, "<name>Features (%s)</name>", title); myZip_fputs (buf, zp);
   KML_NEWLINE
   myZip_fputs ("<open>0</open>", zp); KML_NEWLINE
   for (i=0; i < numPoly; i++) {
      myZip_fputs ("<Folder>", zp); KML_NEWLINE
      /* Determine style URL here. */
      styleIndex = -1;
      for (j=0; j < numSymbol; j++) {
         if (((poly[i].value < symbol[j].Max) ||
              (symbol[j].f_maxInc && (poly[i].value == symbol[j].Max))) &&
             ((poly[i].value > symbol[j].Min) ||
              (symbol[j].f_minInc && (poly[i].value == symbol[j].Min)))) {
            styleIndex = j;
            break;
         }
      }
      if (styleIndex != -1) {
         sprintf (styleUrl, "#%d", styleIndex);
         if (f_reduce) {
            sprintf (rangeTxt, "%c%.*f,%.*f%c", symbol[j].f_minInc ? '[' : '(',
                     decimal, symbol[j].Min, decimal, symbol[j].Max,
                     symbol[j].f_maxInc ? ']' : ')');
         } else {
            sprintf (rangeTxt, "%.*f", decimal, poly[i].value);
         }
      } else {
         sprintf (rangeTxt, "%.*f", decimal, poly[i].value);
      }
      myZip_fputs ("<name>", zp);
      sprintf (buf, "<![CDATA[%s]]>", rangeTxt);
      myZip_fputs (buf, zp);
      myZip_fputs ("</name>", zp); KML_NEWLINE
      for (j = 0; j < poly[i].numAct; j++) {
         if (poly[i].crossLink[j] == -1) {
            myZip_fputs ("<Placemark>", zp); KML_NEWLINE
            myZip_fputs ("<name>", zp);
            sprintf (buf, "<![CDATA[%s]]>", rangeTxt);
            myZip_fputs (buf, zp);
            myZip_fputs ("</name>", zp); KML_NEWLINE
            myZip_fputs ("<description>", zp);
            myZip_fputs ("<![CDATA[]]>", zp);
            myZip_fputs ("</description>", zp); KML_NEWLINE
            if (styleIndex != -1) {
               sprintf (buf, "<styleUrl>%s</styleUrl>", styleUrl);
               myZip_fputs (buf, zp); KML_NEWLINE
            }
            myZip_fputs ("<Polygon>", zp); KML_NEWLINE
            myZip_fputs ("<extrude>0</extrude>", zp); KML_NEWLINE
            myZip_fputs ("<altitudeMode>clampedToGround</altitudeMode>", zp); KML_NEWLINE
            myZip_fputs ("<outerBoundaryIs>", zp); KML_NEWLINE
            myZip_fputs ("<LinearRing>", zp); KML_NEWLINE
            myZip_fputs ("<coordinates>", zp);
            Pnode = poly[i].actList[j].head;
            while (Pnode != NULL) {
               sprintf (buf, "%.*f,%.*f,0\n", LatLon_Decimal, Pnode->x,
                        LatLon_Decimal, Pnode->y); myZip_fputs (buf, zp);
               Pnode = Pnode->next;
            }
            myZip_fputs ("</coordinates>", zp); KML_NEWLINE
            myZip_fputs ("</LinearRing>", zp); KML_NEWLINE
            myZip_fputs ("</outerBoundaryIs>", zp); KML_NEWLINE

            for (k = 0; k < poly[i].numAct; k++) {
               if (poly[i].crossLink[k] == j) {
                  myZip_fputs ("<innerBoundaryIs>", zp); KML_NEWLINE
                  myZip_fputs ("<LinearRing>", zp); KML_NEWLINE
                  myZip_fputs ("<coordinates>", zp);
                  Pnode = poly[i].actList[k].head;
                  while (Pnode != NULL) {
                     sprintf (buf, "%.*f,%.*f,0\n", LatLon_Decimal, Pnode->x,
                             LatLon_Decimal, Pnode->y); myZip_fputs (buf, zp);
                     Pnode = Pnode->next;
                  }
                  myZip_fputs ("</coordinates>", zp); KML_NEWLINE
                  myZip_fputs ("</LinearRing>", zp); KML_NEWLINE
                  myZip_fputs ("</innerBoundaryIs>", zp); KML_NEWLINE
               }
            }

            myZip_fputs ("</Polygon>", zp); KML_NEWLINE
            myZip_fputs ("</Placemark>", zp); KML_NEWLINE
         }
      }
      myZip_fputs ("</Folder>", zp); KML_NEWLINE
   }

   myZip_fputs ("</Folder>", zp); KML_NEWLINE
   myZip_fputs ("</Document>", zp); KML_NEWLINE
   myZip_fputs ("</kml>", zp); KML_NEWLINE
   if (myZip_fclose (zp) == EOF) {
      printf ("Error closing file in the zipfile\n");
      return -1;
   }
   if (myZipClose (zp) == EOF) {
      printf ("Error closing the zipfile\n");
      return -1;
   }
   return 0;
}

#ifdef USE_XMLLIB
/* Need to use the TimeSpan element... (end at validTime...) */
/* Need to handle weather and hazards. */
static int savePolysKml (const char *filename, polyType *poly, int numPoly,
                         grib_MetaData *meta, sChar decimal,
                         sChar LatLon_Decimal, SymbolType *symbol,
                         int numSymbol)
{
   xmlDocPtr doc = NULL; /* document pointer */
   xmlNodePtr root_node = NULL; /* node pointers */
   xmlNodePtr node = NULL; /* node pointers */
   xmlNodePtr node1 = NULL; /* node pointers */
   xmlNodePtr node2 = NULL; /* node pointers */
   xmlNodePtr node3 = NULL; /* node pointers */
   xmlNodePtr node4 = NULL; /* node pointers */
   xmlNodePtr node5 = NULL; /* node pointers */
   xmlNodePtr node6 = NULL; /* node pointers */
   xmlNodePtr cdata = NULL; /* node pointers */
   char buffer[256];
   char valTime[25];
   char title[256];
   char styleUrl[256];
   int styleIndex;
   char *stream = NULL;
   chainNode *Pnode;     /* The current node (to print). */
   int i, j;
   sInt4 deltSec;

   doc = xmlNewDoc (BAD_CAST "1.0");
   root_node = xmlNewNode (NULL, BAD_CAST "kml");
   xmlNewProp (root_node, BAD_CAST "xmlns", BAD_CAST
               "http://www.opengis.net/kml/2.2");
   xmlDocSetRootElement (doc, root_node);

   Clock_Print (valTime, 25, meta->pds2.sect4.validTime, "%m/%d/%Y %H:%M:%S UTC", 0);
   node = xmlNewChild (root_node, NULL, BAD_CAST "Document", NULL);
    node1 = xmlNewChild (node, NULL, BAD_CAST "name", NULL);
     if (meta->unitName != NULL) {
        sprintf (title, "%s %s, %s", meta->element, meta->unitName, valTime);
     } else {
        sprintf (title, "%s, %s", meta->element, valTime);
     }
     cdata = xmlNewCDataBlock (doc, BAD_CAST title, strlen (title));
     xmlAddChild (node1, cdata);
    xmlNewChild (node, NULL, BAD_CAST "open", BAD_CAST "1");

    node1 = xmlNewChild (node, NULL, BAD_CAST "description", NULL);
     reallocSprintf (&stream, "%s\n", meta->comment);
     if (meta->convert != UC_NONE) {
        reallocSprintf (&stream, "Converted to %s\n", meta->unitName);
     }
     Clock_Print (buffer, 25, meta->pds2.refTime, "%m/%d/%Y %H:%M:%S UTC", 0);
     reallocSprintf (&stream, "Made %s\n", buffer);
     reallocSprintf (&stream, "Valid %s\n", valTime);
     reallocSprintf (&stream, "Surface: %s", meta->longFstLevel);
     cdata = xmlNewCDataBlock (doc, BAD_CAST stream, strlen (stream));
     free (stream);
     stream = NULL;
     xmlAddChild (node1, cdata);

     if (meta->GribVersion == 2) {
        switch (meta->pds2.sect4.templat) {
/*           case GS4_ANALYSIS:*/
/*           case GS4_ERROR:*/
/*           case GS4_ENSEMBLE:*/
/*           case GS4_DERIVED:*/
/*           case GS4_PROBABIL_PNT:*/
/*           case GS4_PERCENT_PNT:*/
           case GS4_STATISTIC:
           case GS4_PROBABIL_TIME:
           case GS4_PERCENT_TIME:
           case GS4_ENSEMBLE_STAT:
           case GS4_DERIVED_INTERVAL:
/*           case GS4_RADAR:*/
/*           case GS4_SATELLITE:*/
              node1 = xmlNewChild (node, NULL, BAD_CAST "TimeSpan", NULL);
              deltSec = meta->pds2.sect4.Interval[0].lenTime;
              if (meta->pds2.sect4.Interval[0].timeRangeUnit == 0) {
                 deltSec *= 60;
              } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 1) {
                 deltSec *= 3600;
              } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 2) {
                 deltSec *= 24 * 3600;
              } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 10) {
                 deltSec *= 3 * 3600;
              } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 11) {
                 deltSec *= 6 * 3600;
              } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 12) {
                 deltSec *= 12 * 3600;
              } else if (meta->pds2.sect4.Interval[0].timeRangeUnit == 13) {
                 /* Already in seconds. */
              } else {
                 /* can't handle month, year, decade, normal, century or
                  * reserved */
                 deltSec = 0;
              }
              Clock_Print (buffer, 256, meta->pds2.sect4.validTime - deltSec,
                           "%Y-%m-%dT%H:%M:%S", 0);
              xmlNewChild (node1, NULL, BAD_CAST "begin", BAD_CAST buffer);
              Clock_Print (buffer, 256, meta->pds2.sect4.validTime,
                           "%Y-%m-%dT%H:%M:%S", 0);
              xmlNewChild (node1, NULL, BAD_CAST "end", BAD_CAST buffer);
              break;
           default:
              node1 = xmlNewChild (node, NULL, BAD_CAST "TimeStamp", NULL);
              Clock_Print (buffer, 256, meta->pds2.sect4.validTime,
                           "%Y-%m-%dT%H:%M:%S", 0);
              xmlNewChild (node1, NULL, BAD_CAST "when", BAD_CAST buffer);
              break;
        }
     }

    for (i=0; i < numSymbol; i++) {
       node1 = xmlNewChild (node, NULL, BAD_CAST "Style", NULL);
       sprintf (buffer, "%d", i);
       xmlNewProp(node1, BAD_CAST "id", BAD_CAST buffer);
       node2 = xmlNewChild (node1, NULL, BAD_CAST "LineStyle", NULL);
        sprintf (buffer, "%02X%02X%02X%02X", symbol[i].out.alpha,
                 symbol[i].out.b, symbol[i].out.g, symbol[i].out.r);
        xmlNewChild (node2, NULL, BAD_CAST "color", BAD_CAST buffer);
        sprintf (buffer, "%3.1f", symbol[i].thick / 10.);
        xmlNewChild (node2, NULL, BAD_CAST "width", BAD_CAST buffer);
       node2 = xmlNewChild (node1, NULL, BAD_CAST "PolyStyle", NULL);
        if (symbol[i].out.f_null) {
           xmlNewChild (node2, NULL, BAD_CAST "outline", BAD_CAST "0");
        } else {
           xmlNewChild (node2, NULL, BAD_CAST "outline", BAD_CAST "1");
        }
        if (symbol[i].fg.f_null) {
           xmlNewChild (node2, NULL, BAD_CAST "fill", BAD_CAST "0");
        } else {
           xmlNewChild (node2, NULL, BAD_CAST "fill", BAD_CAST "1");
        }
        sprintf (buffer, "%02X%02X%02X%02X", symbol[i].fg.alpha,
                 symbol[i].fg.b, symbol[i].fg.g, symbol[i].fg.r);
        xmlNewChild (node2, NULL, BAD_CAST "color", BAD_CAST buffer);
    }
/*
    node1 = xmlNewChild (node, NULL, BAD_CAST "ExtendedData", NULL);
     reallocSprintf (&stream, "Created by degrib\n");
     cdata = xmlNewCDataBlock (doc, BAD_CAST stream, strlen (stream));
     free (stream);
     stream = NULL;
     xmlAddChild (node1, cdata);
*/

    /* ... various styles ... */
    /* node1 = xmlNewChild (node, NULL, BAD_CAST "Style", NULL);*/
    node1 = xmlNewChild (node, NULL, BAD_CAST "Folder", NULL);
     sprintf (buffer, "Features (%s)", title);
     xmlNewChild (node1, NULL, BAD_CAST "name", BAD_CAST buffer);
     xmlNewChild (node1, NULL, BAD_CAST "open", BAD_CAST "0");
     for (i=0; i < numPoly; i++) {
        node2 = xmlNewChild (node1, NULL, BAD_CAST "Folder", NULL);
         node3 = xmlNewChild (node2, NULL, BAD_CAST "name", NULL);
          sprintf (buffer, "%.*f", decimal, poly[i].value);
          cdata = xmlNewCDataBlock (doc, BAD_CAST buffer, strlen (buffer));
          xmlAddChild (node3, cdata);
          /* Determine style URL here. */
          styleIndex = -1;
          for (j=0; j < numSymbol; j++) {
             if ((poly[i].value >= symbol[j].min) &&
                 (poly[i].value <= symbol[j].max)) {
                styleIndex = j;
                break;
             }
          }
          if (styleIndex != -1) {
             sprintf (styleUrl, "#%d", styleIndex);
          }
         for (j = 0; j < poly[i].numAct; j++) {
            node3 = xmlNewChild (node2, NULL, BAD_CAST "Placemark", NULL);
             node4 = xmlNewChild (node3, NULL, BAD_CAST "name", NULL);
              sprintf (buffer, "%.*f", decimal, poly[i].value);
              cdata = xmlNewCDataBlock (doc, BAD_CAST buffer, strlen (buffer));
              xmlAddChild (node4, cdata);
             node4 = xmlNewChild (node3, NULL, BAD_CAST "description", NULL);
              sprintf (buffer, " ");
              cdata = xmlNewCDataBlock (doc, BAD_CAST buffer, strlen (buffer));
              xmlAddChild (node4, cdata);
             if (styleIndex != -1) {
                xmlNewChild (node3, NULL, BAD_CAST "styleUrl", BAD_CAST styleUrl);
             }
             node4 = xmlNewChild (node3, NULL, BAD_CAST "Polygon", NULL);
              xmlNewChild (node4, NULL, BAD_CAST "extrude", BAD_CAST "0");
              xmlNewChild (node4, NULL, BAD_CAST "altitudeMode", BAD_CAST "clampedToGround");
              node5 = xmlNewChild (node4, NULL, BAD_CAST "outerBoundaryIs", NULL);
               node6 = xmlNewChild (node5, NULL, BAD_CAST "LinearRing", NULL);
               Pnode = poly[i].actList[j].head;
               while (Pnode != NULL) {
                  sprintf (buffer, "%.*f,%.*f,0\n", LatLon_Decimal, Pnode->x,
                          LatLon_Decimal, Pnode->y);
                  reallocSprintf (&stream, "%s", buffer);
                  Pnode = Pnode->next;
               }
              xmlNewChild (node6, NULL, BAD_CAST "coordinates", BAD_CAST stream);
              free (stream);
              stream = NULL;
        }
     }

   /* The 1 is for space indenting */
   xmlSaveFormatFileEnc (filename, doc, "UTF-8", 1);

   xmlFreeDoc (doc);
   xmlCleanupParser ();
   xmlMemoryDump ();
   return 0;
}

#endif

/* This procedure reduces the complexity of the grid by choosing one
 * value in each color range as the representative value.  This will
 * result in fewer polygons which should result in Google maps/earth
 * redrawing the images faster. */
static void ShrinkGrid (int Nx, int Ny, double *Data, SymbolType *symbol,
                        int numSymbol)
{
   int x;
   int i;

   for (x = 0; x < Nx*Ny; ++x) {
      for (i = 0; i < numSymbol; ++i) {
         if (((Data[x] < symbol[i].Max) ||
              (symbol[i].f_maxInc && (Data[x] == symbol[i].Max))) &&
             ((Data[x] > symbol[i].Min) ||
              (symbol[i].f_minInc && (Data[x] == symbol[i].Min)))) {
            if (symbol[i].f_minInc) {
               Data[x] = symbol[i].Min;
            } else if (symbol[i].f_maxInc) {
               Data[x] = symbol[i].Max;
            } else {
               Data[x] = (symbol[i].Min + symbol[i].Max) / 2.;
            }
            break;
         }
      }
   }
}

int gribWriteKml (const char *Filename, double *grib_Data,
                  grib_MetaData *meta, sChar f_poly, sChar f_nMissing,
                  sChar decimal, sChar LatLon_Decimal, const char *kmlIni,
                  int f_kmz, sChar f_kmlMerge)
{
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   char *filename;      /* local copy of the filename. */
   int nameLen;         /* length of filename. */
   gdsType *gds = &(meta->gds); /* Simplifies references to the gds data. */
   polyType *poly;      /* list of chains that represent large polygons. */
   int numPoly;         /* number of element in poly. */
   double *polyData;    /* Data values for each poly (no missing values) */
   SymbolType *symbol = NULL;
   int numSymbol = 0;
   int i;
   TxtPair *desc = NULL;
   int numDesc = 0;

   nameLen = strlen (Filename);
   if (nameLen < 4) {
      errSprintf ("ERROR: File %s is too short in length (it may need an "
                  "extension?)", Filename);
      return -2;
   }
   filename = (char *) malloc (nameLen + 1 * sizeof (char));
   strncpy (filename, Filename, nameLen);
   filename[nameLen] = '\0';
   strncpy (filename + strlen (filename) - 3, "kml", 3);

   /* Check that gds is valid before setting up map projection. */
   if (GDSValid (&meta->gds) != 0) {
      preErrSprintf ("ERROR: Sect3 was not Valid.\n");
      free (filename);
      return -3;
   }
   /* Set up the map projection. */
   SetMapParamGDS (&map, gds);

   if (kmlIni != NULL) {
      if (ParseKMLIniFile (kmlIni, meta->element, &symbol, &numSymbol, &desc,
                           &numDesc) != 0) {
         errSprintf ("Had problems parsing %s", Filename);
         free (filename);
         return -2;
      }
   }

   if (f_poly == 1) {   /* Small poly */
      printf ("Small poly does not currently work\n");
   } else if (f_poly == 2) { /* Big poly */
      NewPolys (&poly, &numPoly);

      if (f_kmlMerge) {
         ShrinkGrid (gds->Nx, gds->Ny, grib_Data, symbol, numSymbol);
      }

      /* Convert the grid to a list of polygon chains. */
      /* Assumes data came from ParseGrid() so scan flag == "0100". */
      Grid2BigPoly (&poly, &numPoly, gds->Nx, gds->Ny, grib_Data);

      /* Following removes all "missing" values from the chains. */
      gribCompactPolys (poly, &numPoly, f_nMissing, &(meta->gridAttrib),
                        &polyData);

/*
      PrintPolys2 (poly, numPoly);
      printf ("\n");
*/

      /* Following is for experimenting with dateline issue. */
      ConvertChain2LtLn (poly, numPoly, &map, LatLon_Decimal);

/*
      PrintPolys (poly, numPoly);
      printf ("\n");
*/

      FindAreas (poly, numPoly);

      savePolysKmlFast (filename, poly, numPoly, meta, decimal, LatLon_Decimal,
                        symbol, numSymbol, f_kmz, desc, numDesc, f_kmlMerge);

      FreePolys (poly, numPoly);
      free (polyData);

   } else {             /* point */
      printf ("point does not currently work\n");
   }

   for (i=0; i < numDesc; i++) {
      free (desc[i].label);
      free (desc[i].value);
   }
   free (desc);

   for (i=0; i < numSymbol; i++) {
      freeIniSymbol (&(symbol[i]));
   }
   free (symbol);
   free (filename);
   return 0;
}
