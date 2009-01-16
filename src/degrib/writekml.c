/* #include <stdio.h> */
#include <string.h>
/* #include <stdlib.h> */
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

#ifdef USE_XMLLIB
/* Either one. */
#include <libxml/parser.h>
/* #include <libxml/tree.h> */
#endif

#ifdef TEST_POLYOUT
static void PrintPolys (polyType *poly, int numPoly)
{
   int i;               /* Loop counter over number of Polys. */
   int j;               /* Loop counter over list of chains. */
   int cnt;             /* Check if we have > 100 nodes. */
   chainNode *node;     /* The current node (to print). */

   printf ("Number of poly %d\n", numPoly);
   for (i = 0; i < numPoly; i++) {
      printf ("Poly number %d (value = %f)\n", i, poly[i].value);
      myAssert (poly[i].numFin == 0);
      for (j = 0; j < poly[i].numAct; j++) {
         printf ("  %d :: ", j);
         node = poly[i].actList[j].head;
         cnt = 0;
         while (node != NULL) {
            cnt++;
            printf ("(%f %f) ,", node->x, node->y);
            if (node == poly[i].actList[j].tail) {
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
#endif

static int ParseKMLIniFile (const char *kmlIni, SymbolType **symbol,
                            int *numSymbol)
{
   FILE *fp;
   char *line = NULL;
   size_t lineLen = 0;
   char *first;
   char *second;
   char *third;
   int f_valid;
   int i;

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
   f_valid = 0;
   while (reallocFGets (&line, &lineLen, fp) > 0) {
      first = line;
      while ((isspace (*first)) && (*first != '\0')) {
         first++;
      }
      if ((first != NULL) && (*first != '#')) {
         if (! f_valid) {
            strTrim (line);
            if (strcmp (line, "TYPE=DEGRIB-KMLINI-1.0") != 0) {
               printf ("Bad kmlIni version information\n");
               fclose (fp);
               free (line);
               return -1;
            }
            f_valid = 1;
         } else {
            second = strchr (first, '=');
            if (second != NULL) {
               *second = '\0';
               second++;
               third = strchr (second, '#');
               if (third != NULL) {
                  *third = '\0';
               }
               third = strchr (second, '\n');
               if (third != NULL) {
                  *third = '\0';
               }
            }
            if (strcmp (first, "Symbol") == 0) {
               *numSymbol = *numSymbol + 1;
               *symbol = (SymbolType *) realloc ((void *) *symbol,
                                                 *numSymbol * sizeof (SymbolType));
               ParseSymbol (&((*symbol)[*numSymbol - 1]), second);
            }
         }
      }
   }
   fclose (fp);
   free (line);
   return 0;
}

/* In tests with psurge data, found this to be 2x faster:
 * on "fortune (my machine at home)" it took: 427 sec to use xml library,
 * and only 272 sec to use fprintf's */
/* Note file size is smaller by about 1 meg because of the indentation
 * which xmllib does, but isn't needed. */
#define KML_NEWLINE fprintf (fp, "\n");
static int savePolysKmlFast (const char *filename, polyType *poly, int numPoly,
                             grib_MetaData *meta, sChar decimal,
                             sChar LatLon_Decimal, SymbolType *symbol,
                             int numSymbol)
{
   FILE *fp;
   char buffer[256];
   char valTime[25];
   char title[256];
   char styleUrl[256];
   int styleIndex;
   chainNode *Pnode;     /* The current node (to print). */
   int i, j;
   sInt4 deltSec;

   if ((fp = fopen (filename, "wt")) == NULL) {
      printf ("Unable to write to %s\n", filename);
      return -1;
   }
   Clock_Print (valTime, 25, meta->pds2.sect4.validTime, "%m/%d/%Y %H:%M:%S UTC", 0);

   fprintf (fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"); KML_NEWLINE
   fprintf (fp, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">"); KML_NEWLINE
   fprintf (fp, "<Document>"); KML_NEWLINE

   fprintf (fp, "<name>");
   if (meta->unitName != NULL) {
      sprintf (title, "%s %s, %s", meta->element, meta->unitName, valTime);
   } else {
      sprintf (title, "%s, %s", meta->element, valTime);
   }
   fprintf (fp, "<![CDATA[%s]]>", title);
   fprintf (fp, "</name>"); KML_NEWLINE

   fprintf (fp, "<open>1</open>"); KML_NEWLINE

   fprintf (fp, "<description>");
   fprintf (fp, "<![CDATA[%s\n", meta->comment);
   if (meta->convert != UC_NONE) {
      fprintf (fp, "Converted to %s\n", meta->unitName);
   }
   Clock_Print (buffer, 25, meta->pds2.refTime, "%m/%d/%Y %H:%M:%S UTC", 0);
   fprintf (fp, "Made %s\n", buffer);
   fprintf (fp, "Valid %s\n", valTime);
   fprintf (fp, "Surface: %s]]>", meta->longFstLevel);
/*   fprintf (fp, "Surface: %s\n]]>", meta->longFstLevel);*/
   fprintf (fp, "</description>"); KML_NEWLINE

   /* Try to handle the time stamp. */
   if (meta->GribVersion == 2) {
      switch (meta->pds2.sect4.templat) {
         case GS4_STATISTIC:
         case GS4_PROBABIL_TIME:
         case GS4_PERCENT_TIME:
         case GS4_ENSEMBLE_STAT:
         case GS4_DERIVED_INTERVAL:
            fprintf (fp, "<TimeSpan>"); KML_NEWLINE
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
            fprintf (fp, "<begin>%s</begin>", buffer); KML_NEWLINE
            Clock_Print (buffer, 256, meta->pds2.sect4.validTime,
                         "%Y-%m-%dT%H:%M:%S", 0);
            fprintf (fp, "<end>%s</end>", buffer); KML_NEWLINE
            fprintf (fp, "</TimeSpan>"); KML_NEWLINE
            break;
         default:
            fprintf (fp, "<TimeStamp>"); KML_NEWLINE
            Clock_Print (buffer, 256, meta->pds2.sect4.validTime,
                         "%Y-%m-%dT%H:%M:%S", 0);
            fprintf (fp, "<when>%s</when>", buffer); KML_NEWLINE
            fprintf (fp, "</TimeStamp>"); KML_NEWLINE
            break;
      }
   }

   /* Work on Symbol table. */
   for (i=0; i < numSymbol; i++) {
      fprintf (fp, "<Style id=\"%d\">", i); KML_NEWLINE
      fprintf (fp, "<LineStyle>"); KML_NEWLINE
      sprintf (buffer, "%02X%02X%02X%02X", symbol[i].out.alpha,
               symbol[i].out.b, symbol[i].out.g, symbol[i].out.r);
      fprintf (fp, "<color>%s</color>", buffer); KML_NEWLINE
      fprintf (fp, "<width>%3.1f</width>", symbol[i].thick / 10.); KML_NEWLINE
      fprintf (fp, "</LineStyle>"); KML_NEWLINE
      fprintf (fp, "<PolyStyle>"); KML_NEWLINE
      if (symbol[i].out.f_null) {
         fprintf (fp, "<outline>0</outline>"); KML_NEWLINE
      } else {
         fprintf (fp, "<outline>1</outline>"); KML_NEWLINE
      }
      if (symbol[i].fg.f_null) {
         fprintf (fp, "<fill>0</fill>"); KML_NEWLINE
      } else {
         fprintf (fp, "<fill>1</fill>"); KML_NEWLINE
      }
      sprintf (buffer, "%02X%02X%02X%02X", symbol[i].fg.alpha,
               symbol[i].fg.b, symbol[i].fg.g, symbol[i].fg.r);
      fprintf (fp, "<color>%s</color>", buffer); KML_NEWLINE
      fprintf (fp, "</PolyStyle>"); KML_NEWLINE
      fprintf (fp, "</Style>"); KML_NEWLINE
   }

   /* Work on Dataset. */
   fprintf (fp, "<Folder>"); KML_NEWLINE
   fprintf (fp, "<name>Features (%s)</name>", title); KML_NEWLINE
   fprintf (fp, "<open>0</open>"); KML_NEWLINE
   for (i=0; i < numPoly; i++) {
      fprintf (fp, "<Folder>"); KML_NEWLINE
      fprintf (fp, "<name>");
      fprintf (fp, "<![CDATA[%.*f]]>", decimal, poly[i].value);
      fprintf (fp, "</name>"); KML_NEWLINE
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
         fprintf (fp, "<Placemark>"); KML_NEWLINE
         fprintf (fp, "<name>");
         fprintf (fp, "<![CDATA[%.*f]]>", decimal, poly[i].value);
         fprintf (fp, "</name>"); KML_NEWLINE
         fprintf (fp, "<description>");
         fprintf (fp, "<![CDATA[]]>");
/*         fprintf (fp, "<![CDATA[ ]]>");*/
         fprintf (fp, "</description>"); KML_NEWLINE
         if (styleIndex != -1) {
            fprintf (fp, "<styleUrl>%s</styleUrl>", styleUrl); KML_NEWLINE
         }
         fprintf (fp, "<Polygon>"); KML_NEWLINE
         fprintf (fp, "<extrude>0</extrude>"); KML_NEWLINE
         fprintf (fp, "<altitudeMode>clampedToGround</altitudeMode>"); KML_NEWLINE
         fprintf (fp, "<outerBoundaryIs>"); KML_NEWLINE
         fprintf (fp, "<LinearRing>"); KML_NEWLINE
         fprintf (fp, "<coordinates>");
         Pnode = poly[i].actList[j].head;
         while (Pnode != NULL) {
            fprintf (fp, "%.*f,%.*f,0\n", LatLon_Decimal, Pnode->x,
                     LatLon_Decimal, Pnode->y);
            Pnode = Pnode->next;
         }
         fprintf (fp, "</coordinates>"); KML_NEWLINE
         fprintf (fp, "</LinearRing>"); KML_NEWLINE
         fprintf (fp, "</outerBoundaryIs>"); KML_NEWLINE
         fprintf (fp, "</Polygon>"); KML_NEWLINE
         fprintf (fp, "</Placemark>"); KML_NEWLINE
      }
      fprintf (fp, "</Folder>"); KML_NEWLINE
   }

   fprintf (fp, "</Folder>"); KML_NEWLINE
   fprintf (fp, "</Document>"); KML_NEWLINE
   fprintf (fp, "</kml>"); KML_NEWLINE
   fclose (fp);
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

int gribWriteKml (const char *Filename, double *grib_Data,
                  grib_MetaData *meta, sChar f_poly, sChar f_nMissing,
                  sChar decimal, sChar LatLon_Decimal, const char *kmlIni)
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
      ParseKMLIniFile (kmlIni, &symbol, &numSymbol);
   }

   if (f_poly == 1) {   /* Small poly */
      printf ("Small poly does not currently work\n");
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

/*
      PrintPolys (poly, numPoly);
      printf ("\n");
*/
      savePolysKmlFast (filename, poly, numPoly, meta, decimal, LatLon_Decimal,
                    symbol, numSymbol);
      printf ("\n\n Saved to file %s\n", filename);

      FreePolys (poly, numPoly);
      free (polyData);

   } else {             /* point */
      printf ("point does not currently work\n");
   }

   for (i=0; i < numSymbol; i++) {
      freeIniSymbol (&(symbol[i]));
   }
   free (symbol);
   free (filename);
   return 0;
}
