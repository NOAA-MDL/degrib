#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "myutil.h"
#include "mapini.h"
#include "tendian.h"
#include "myassert.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

static char *LayerTypes[] = {
   "Invalid", "Single Symbol", "Graduated", "Text", "DB2", "Png", "Grid",
   "Info", "Lattice", NULL
};
static char *ShpFileTypes[] = {
   "Invalid", "Polygon", "Point", "Memory", "Void", NULL
};

int freeIniSymbol (SymbolType *symbol)
{
   free (symbol->mark);
   return 0;
}

static int ReadColorTable (char *filename, layerType *layer)
{
   FILE *fp;
   char *line = NULL;
   size_t lineLen = 0;
   int cnt;
   double value;
   double min = 0, max = 0;
   int r, g, b;
   float labMin, labMax, labInterval;
   int i;
   size_t numCol;
   char **colList;
   char *ptr;

   if ((fp = fopen (filename, "rt")) == NULL) {
      printf ("Unable to read %s\n", filename);
      return -1;
   }
   cnt = 0;
   layer->ramp.numColors = 0;
   layer->ramp.colors = NULL;
   layer->ramp.numLab = 0;
   layer->ramp.labRay = NULL;
   layer->ramp.labJust = NULL;
   layer->ramp.label = NULL;
   while (reallocFGets (&line, &lineLen, fp) > 0) {
      strTrim (line);
      if (strlen (line) != 0) {
         if (cnt > 8) {
            i = layer->ramp.numColors;
            layer->ramp.numColors++;
            layer->ramp.colors = (colorType *)
                  realloc (layer->ramp.colors,
                           layer->ramp.numColors * sizeof (colorType));
            sscanf (line, "%lf %d %d %d", &value, &r, &g, &b);
            layer->ramp.colors[i].value = value;
            layer->ramp.colors[i].r = r;
            layer->ramp.colors[i].g = g;
            layer->ramp.colors[i].b = b;
            layer->ramp.colors[i].alpha = 255;
            layer->ramp.colors[i].f_null = 0;
            if (cnt == 9) {
               min = max = value;
            } else {
               if (min > value)
                  min = value;
               if (max < value)
                  max = value;
            }
         } else if (cnt == 1) {
            sscanf (line, "%d %d %d", &r, &g, &b);
            layer->ramp.thick = 1;
            layer->ramp.outline.r = r;
            layer->ramp.outline.g = g;
            layer->ramp.outline.b = b;
            layer->ramp.outline.alpha = 255;
            layer->ramp.outline.f_null = 0;
         } else if (cnt == 2) {
            sscanf (line, "%d %d %d", &r, &g, &b);
            layer->ramp.missColor.r = r;
            layer->ramp.missColor.g = g;
            layer->ramp.missColor.b = b;
            layer->ramp.missColor.alpha = 255;
            if ((r == -1) || (g == -1) || (b == -1))
               layer->ramp.missColor.f_null = 1;
         } else if (cnt == 8) {
            sscanf (line, "%d", &r);
            layer->ramp.decimal = r;
         } else if (cnt == 7) {
            if (strncmp (line, "array", 5) == 0) {
               line[strlen(line)-1] = '\0';
               numCol = 0;
               colList = NULL;
               mySplit (line + 6, ',', &numCol, &colList, 1);
               layer->ramp.numLab = numCol;
               layer->ramp.labJust = (int *) malloc(numCol * sizeof(int));
               layer->ramp.labRay = (float *) malloc(numCol * sizeof(float));
               layer->ramp.label = (char **) malloc(numCol * sizeof(char *));
               for (i = 0; i < numCol; i++) {
                  if ((colList[i][0] == 'r') || (colList[i][0] == 'R') ||
                      (colList[i][0] == 'l') || (colList[i][0] == 'L')) {
                     if ((colList[i][0] == 'r') || (colList[i][0] == 'R')) {
                        layer->ramp.labJust[i] = 1;
                     } else {
                        layer->ramp.labJust[i] = 0;
                     }
                     layer->ramp.labRay[i] = atof (colList[i] + 1);
                     if ((ptr = strchr (colList[i] + 1, ':')) != NULL) {
                        layer->ramp.label[i] = (char *) malloc (strlen (ptr + 1) + 1);
                        strcpy (layer->ramp.label[i], ptr + 1);
                     } else {
                        layer->ramp.label[i] = (char *) malloc (strlen (colList[i] + 1) + 1);
                        strcpy (layer->ramp.label[i], colList[i] + 1);
                     }
                  } else {
                     layer->ramp.labJust[i] = 0;
                     layer->ramp.labRay[i] = atof (colList[i]);
                     if ((ptr = strchr (colList[i], ':')) != NULL) {
                        layer->ramp.label[i] = (char *) malloc (strlen (ptr + 1) + 1);
                        strcpy (layer->ramp.label[i], ptr + 1);
                     } else {
                        layer->ramp.label[i] = (char *) malloc (strlen (colList[i]) + 1);
                        strcpy (layer->ramp.label[i], colList[i]);
                     }
                  }
                  free (colList[i]);
               }
               free (colList);
            } else {
               sscanf (line, "%f %f %f", &labMin, &labMax, &labInterval);
               layer->ramp.labInterval = labInterval;
               layer->ramp.labMin = labMin;
               layer->ramp.labMax = labMax;
            }
         }
         cnt++;
      }
   }
   layer->ramp.min = min;
   layer->ramp.max = max;
   fclose (fp);
   free (line);
   return 0;
}

static int ParseZoomSect (mapIniType * mapIni, char *var, char *value)
{
   static char *Vars[] = {
      "lat1", "lon1", "lat2", "lon2", "Filename", NULL
   };
   enum { LAT1, LON1, LAT2, LON2, FILENAME };
   int index;

   if (GetIndexFromStr (var, Vars, &index) < 0) {
      printf ("Invalid variable '%s'\n", var);
      return -1;
   }
   switch (index) {
      case LAT1:
         mapIni->zoom.lat1 = atof (value);
         mapIni->zoom.f_flag |= 1;
         return 0;
      case LON1:
         mapIni->zoom.lon1 = atof (value);
         mapIni->zoom.f_flag |= 2;
         return 0;
      case LAT2:
         mapIni->zoom.lat2 = atof (value);
         mapIni->zoom.f_flag |= 4;
         return 0;
      case LON2:
         mapIni->zoom.lon2 = atof (value);
         mapIni->zoom.f_flag |= 8;
         return 0;
      case FILENAME:
         mapIni->zoom.file = (char *) malloc (strlen (value) + 1);
         strcpy (mapIni->zoom.file, value);
   }
   return 1;
}

static int ParseProjectSect (mapIniType * mapIni, char *var, char *value)
{
   static char *Vars[] = {
      "projType", "majEarth", "minEarth", "lat1", "lon1", "mesh",
      "orientLon", "meshLat", "scaleLat1", "scaleLat2", NULL
   };
   enum {
      PROJTYPE, MAJEARTH, MINEARTH, LAT1, LON1, MESH, ORIENTLON, MESHLAT,
      SCALELAT1, SCALELAT2
   };
   int index;

   if (GetIndexFromStr (var, Vars, &index) < 0) {
      printf ("Invalid variable '%s'\n", var);
      return -1;
   }
   switch (index) {
      case PROJTYPE:
         mapIni->proj.type = atoi (value);
         mapIni->proj.f_flag1 |= 1;
         return 0;
      case MAJEARTH:
         mapIni->proj.majEarth = atof (value);
         mapIni->proj.f_flag1 |= 2;
         return 0;
      case MINEARTH:
         mapIni->proj.minEarth = atof (value);
         mapIni->proj.f_flag1 |= 4;
         return 0;
      case LAT1:
         mapIni->proj.lat1 = atof (value);
         mapIni->proj.f_flag1 |= 8;
         return 0;
      case LON1:
         mapIni->proj.lon1 = atof (value);
         mapIni->proj.f_flag1 |= 16;
         return 0;
      case MESH:
         mapIni->proj.mesh = atof (value);
         mapIni->proj.f_flag2 |= 1;
         return 0;
      case ORIENTLON:
         mapIni->proj.orientLon = atof (value);
         mapIni->proj.f_flag2 |= 2;
         return 0;
      case MESHLAT:
         mapIni->proj.meshLat = atof (value);
         mapIni->proj.f_flag2 |= 4;
         return 0;
      case SCALELAT1:
         mapIni->proj.scaleLat1 = atof (value);
         mapIni->proj.f_flag2 |= 8;
         return 0;
      case SCALELAT2:
         mapIni->proj.scaleLat2 = atof (value);
         mapIni->proj.f_flag2 |= 16;
         return 0;
   }
   return 1;
}

static int ParseOutputSect (allOutputType * all, outputType * out, char *var,
                            char *value)
{
   static char *Vars[] = { "filename", "Layer", NULL };
   enum { FILENAME, LAYER };
   int index;
   char *first;

   if (GetIndexFromStr (var, Vars, &index) < 0) {
      printf ("Invalid variable '%s'\n", var);
      return -1;
   }
   switch (index) {
      case FILENAME:
         out->filename = (char *) malloc ((strlen (value) + 1) *
                                          sizeof (char));
         strcpy (out->filename, value);
         return 0;
      case LAYER:
         first = strtok (value, "[]");
         if (first != NULL) {
            out->numActive = 1;
            out->active = (int *) malloc (out->numActive * sizeof (int));
            out->active[0] = atoi (first);
            while ((first = strtok (NULL, "[]")) != NULL) {
               out->numActive++;
               out->active = (int *) realloc ((void *) out->active,
                                              out->numActive * sizeof (int));
               out->active[out->numActive - 1] = atoi (first);
            }
         }
         return 0;

   }
   return 1;
}

static void InitOutput (outputType * out)
{
   out->filename = NULL;
   out->active = NULL;
   out->numActive = 0;
   out->f_valid = 0;
}

static int ParseColor (colorType *color, char *value)
{
   char *ptr;
   char *ptr2;

   ptr = value;
   ptr2 = strchr (value, ',');
   if (ptr2 == NULL) {
      printf ("Invalid Color string %s\n", value);
      return -1;
   }
   *ptr2 = '\0';
   color->r = atoi (ptr);
   ptr = ptr2 + 1;
   ptr2 = strchr (ptr, ',');
   if (ptr2 == NULL) {
      printf ("Invalid Color string %s\n", value);
      return -1;
   }
   *ptr2 = '\0';
   color->g = atoi (ptr);
   ptr = ptr2 + 1;
   if (ptr == '\0') {
      printf ("Invalid Color string %s\n", value);
      return -1;
   }
   ptr2 = strchr (ptr, ',');
   if (ptr2 == NULL) {
      color->b = atoi (ptr);
   } else {
      *ptr2 = '\0';
      color->b = atoi (ptr);
      ptr = ptr2 + 1;
      if (ptr == '\0') {
         printf ("Invalid Color string %s\n", value);
         return -1;
      }
      color->alpha = atoi (ptr);
   }
   if ((color->r == -1) || (color->g == -1) || (color->b == -1)) {
      color->f_null = 1;
      color->r = 0;
      color->g = 0;
      color->b = 0;
      color->alpha = 255;
   } else {
      color->f_null = 0;
   }
   return 0;
}

int ReadShpBounds (char *filename, double *minX, double *minY, double *maxX,
                   double *maxY)
{
   FILE *sfp;
   sInt4 Head1[7];      /* The Big endian part of the Header. */
   sInt4 Head2[2];      /* The Little endian part of the Header. */
   double Bounds[8];    /* Spatial bounds of the data. minLon, minLat,
                         * maxLon, maxLat, ... */

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
   FREAD_LIT (Bounds, sizeof (double), 8, sfp);
   *maxX = Bounds[0];
   *maxY = Bounds[1];
   *minX = Bounds[2];
   *minY = Bounds[3];
   fclose (sfp);
   return 0;
}

static int ParseAllOutputSect (mapIniType * mapIni, char *var, char *value)
{
   int i;
   static char *Vars[] = {
      "numOutput", "X_Size", "Y_Size", "Layer", "Frame", NULL
   };
   enum { NUMOUTPUT, X_SIZE, Y_SIZE, LAYER, FRAME };
   int index;
   char *first;
   frameType fr_temp;
   char *buffer;

   if (GetIndexFromStr (var, Vars, &index) < 0) {
      printf ("Invalid variable '%s'\n", var);
      return -1;
   }
   switch (index) {
      case NUMOUTPUT:
         mapIni->out.numOutputs = atoi (value);
         mapIni->out.outputs =
               (outputType *) malloc (mapIni->out.numOutputs *
                                      sizeof (outputType));
         for (i = 0; i < mapIni->out.numOutputs; i++) {
            InitOutput (&(mapIni->out.outputs[i]));
         }
         return 0;
      case X_SIZE:
         mapIni->out.X_Size = atoi (value);
         mapIni->out.f_flag |= 1;
         return 0;
      case Y_SIZE:
         mapIni->out.Y_Size = (int) atof (value);
         mapIni->out.f_flag |= 2;
         return 0;
      case LAYER:
         first = strtok (value, "[]");
         if (first != NULL) {
            mapIni->out.numActive = 1;
            mapIni->out.active = (int *) malloc (mapIni->out.numActive *
                                                 sizeof (int));
            mapIni->out.active[0] = atoi (first);
            while ((first = strtok (NULL, "[]")) != NULL) {
               mapIni->out.numActive++;
               mapIni->out.active = (int *)
                     realloc ((void *) mapIni->out.active,
                              mapIni->out.numActive * sizeof (int));
               mapIni->out.active[mapIni->out.numActive - 1] = atoi (first);
            }
         }
         return 0;
      case FRAME:
         first = strtok (value, "[]");
         if (first == NULL)
            return 1;
         fr_temp.X0 = atoi (first);

         first = strtok (NULL, "[]");
         if (first == NULL)
            return 1;
         fr_temp.Y0 = atoi (first);

         first = strtok (NULL, "[]");
         if (first == NULL)
            return 1;
         fr_temp.X1 = atoi (first);

         first = strtok (NULL, "[]");
         if (first == NULL)
            return 1;
         fr_temp.Y1 = atoi (first);

         first = strtok (NULL, "[]");
         if (first == NULL)
            return 1;
         if (ParseColor (&(fr_temp.bg), first) != 0) {
            printf ("Problems with symbol %s\n", first);
            return 1;
         }

         /* Start parsing the zoom. */
         first = strtok (NULL, "[]");
         if (first != NULL) {
            buffer = (char *) malloc (strlen (first) + 1);
            strcpy (buffer, first);
            first = strtok (NULL, "[]");
            /* check if buffer contains the name of a file or lat1. */
            if (first == NULL) {
               if (ReadShpBounds (buffer, &(fr_temp.zoom.lon2),
                                  &(fr_temp.zoom.lat2), &(fr_temp.zoom.lon1),
                                  &(fr_temp.zoom.lat1)) == 0) {
                  fr_temp.zoom.f_flag = 15;
               }
               free (buffer);
            } else {
               fr_temp.zoom.lat1 = atof (buffer);
               free (buffer);
               fr_temp.zoom.lon1 = atof (first);
               first = strtok (NULL, "[]");
               if (first == NULL) {
                  return 1;
               }
               fr_temp.zoom.lat2 = atof (first);
               first = strtok (NULL, "[]");
               if (first == NULL) {
                  return 1;
               }
               fr_temp.zoom.lon2 = atof (first);
               fr_temp.zoom.f_flag = 15;
            }
            fr_temp.f_zoom = 1;
         } else {
            fr_temp.f_zoom = 0;
         }

         mapIni->out.numFrame++;
         mapIni->out.frame =
               (frameType *) realloc ((void *) mapIni->out.frame,
                                      mapIni->out.numFrame *
                                      sizeof (frameType));
         mapIni->out.frame[mapIni->out.numFrame - 1] = fr_temp;
         return 0;
   }
   return 1;
}

void InitLayer (layerType *layer)
{
   layer->type = INVALID;
   layer->shpType = INVALID_SHP;
   layer->filename = NULL;
   layer->title.name = NULL;
   layer->numMatchVal = 0;
   layer->matchField = NULL;
   layer->matchVal = NULL;
   layer->legend.f_valid = 0;
   layer->lattice.f_valid = 0;
   layer->frameNum = -1;
   layer->legendFrameNum = -1;
   layer->gridData = NULL;
   layer->numPnt = 0;
   layer->pnt = NULL;
}

static void FreeLayer (layerType *layer)
{
   size_t i;
   size_t j;

   free (layer->filename);
   free (layer->title.name);
   for (i = 0; i < layer->numMatchVal; i++) {
      free (layer->matchVal[i]);
   }
   free (layer->matchVal);
   free (layer->matchField);
   switch (layer->type) {
      case GRADUATED:
         for (i = 0; i < layer->grad.numSymbol; i++) {
            freeIniSymbol (& (layer->grad.symbol[i]));
         }
         free (layer->grad.symbol);
         free (layer->grad.field);
         break;
      case GRID:
         free (layer->ramp.colors);
         free (layer->ramp.labRay);
         for (j = 0; j < layer->ramp.numLab; j++) {
            free (layer->ramp.label[j]);
         }
         free (layer->ramp.label);
         free (layer->ramp.labJust);
         break;
      case INFO:
         free (layer->pnt);
         break;
      case SINGLE_SYMBOL:
         freeIniSymbol (& (layer->single));
         break;
      case DB2:
         free (layer->db2.field);
         break;
   }
   InitLayer (layer);
}

int ParseSymbol (SymbolType * symbol, char *value)
{
   char *ptr;
   char *ptr2;
   char *ptr3;
   int cnt;
   enum { OUTLINE, FOREGROUND, MARK, MIN, MAX, DECIMAL, THICK };

   ptr = strchr (value, '[');
   if (ptr == NULL) {
      printf ("Invalid Symbol string %s\n", value);
      return -1;
   } else {
      cnt = -1;
      symbol->out.r = 0;
      symbol->out.g = 0;
      symbol->out.b = 0;
      symbol->out.alpha = 255;
      symbol->fg.r = 0;
      symbol->fg.g = 0;
      symbol->fg.b = 0;
      symbol->fg.alpha = 255;
      symbol->f_mark = 0;
      symbol->Max = 0;
      symbol->Min = 0;
      symbol->f_maxInc = 1;
      symbol->f_minInc = 1;
      symbol->decimal = 1;
      symbol->thick = 1;
      while ((ptr2 = strchr (ptr, ']')) != NULL) {
         cnt++;
         *ptr2 = '\0';
         ptr = ptr + 1;
         while ((ptr3 = strchr (ptr, '}')) != NULL) {
            ptr = ptr3 + 1;
         }
         switch (cnt) {
            case OUTLINE:
               if (ParseColor (&(symbol->out), ptr) != 0) {
                  printf ("Problems with symbol %s\n", value);
                  return -1;
               }
               break;
            case FOREGROUND:
               if (ParseColor (&(symbol->fg), ptr) != 0) {
                  printf ("Problems with symbol %s\n", value);
                  return -1;
               }
               break;
            case MARK:
               symbol->mark = (char *) malloc ((strlen (ptr) + 1) *
                                               sizeof (char));
               strcpy (symbol->mark, ptr);
               if (strcmp (ptr, "dot") == 0) {
                  symbol->f_mark = 1;
               } else if (strcmp (ptr, "value") == 0) {
                  symbol->f_mark = 2;
               } else if (strcmp (ptr, "pixel") == 0) {
                  symbol->f_mark = 3;
               } else if (strcmp (ptr, "dot3") == 0) {
                  symbol->f_mark = 4;
               } else if (strcmp (ptr, "dot2") == 0) {
                  symbol->f_mark = 5;
               } else if (strcmp (ptr, "dot4") == 0) {
                  symbol->f_mark = 6;
               } else if (strcmp (ptr, "dot5") == 0) {
                  symbol->f_mark = 7;
               } else if (strcmp (ptr, "dot6") == 0) {
                  symbol->f_mark = 8;
               } else if (strcmp (ptr, "dot7") == 0) {
                  symbol->f_mark = 9;
               } else if (strcmp (ptr, "dot8") == 0) {
                  symbol->f_mark = 10;
               } else if (strcmp (ptr, "dot9") == 0) {
                  symbol->f_mark = 11;
               } else if (strcmp (ptr, "dot10") == 0) {
                  symbol->f_mark = 12;
               } else if (strcmp (ptr, "dot11") == 0) {
                  symbol->f_mark = 13;
               } else if (strcmp (ptr, "dot12") == 0) {
                  symbol->f_mark = 14;
               } else if (strcmp (ptr, "dot13") == 0) {
                  symbol->f_mark = 15;
               } else if (strcmp (ptr, "dot14") == 0) {
                  symbol->f_mark = 16;
               } else if (strcmp (ptr, "dot15") == 0) {
                  symbol->f_mark = 17;
               } else if (strcmp (ptr, "dot16") == 0) {
                  symbol->f_mark = 18;
               } else if (strcmp (ptr, "dot17") == 0) {
                  symbol->f_mark = 19;
               } else if (strcmp (ptr, "dot18") == 0) {
                  symbol->f_mark = 20;
               } else if (strcmp (ptr, "dot19") == 0) {
                  symbol->f_mark = 21;
               } else if (strcmp (ptr, "dot20") == 0) {
                  symbol->f_mark = 22;
               } else if (strcmp (ptr, "dot21") == 0) {
                  symbol->f_mark = 23;
               } else if (strcmp (ptr, "dot22") == 0) {
                  symbol->f_mark = 24;
               } else if (strcmp (ptr, "dot23") == 0) {
                  symbol->f_mark = 25;
               } else if (strcmp (ptr, "dot24") == 0) {
                  symbol->f_mark = 26;
               } else if (strcmp (ptr, "dot25") == 0) {
                  symbol->f_mark = 27;
               } else if (strcmp (ptr, "dot26") == 0) {
                  symbol->f_mark = 28;
               } else if (strcmp (ptr, "dot27") == 0) {
                  symbol->f_mark = 29;
               } else if (strcmp (ptr, "dot28") == 0) {
                  symbol->f_mark = 30;
               } else if (strcmp (ptr, "dot29") == 0) {
                  symbol->f_mark = 31;
               } else if (strcmp (ptr, "dot30") == 0) {
                  symbol->f_mark = 32;
               } else if (strcmp (ptr, "dot31") == 0) {
                  symbol->f_mark = 33;
               } else if (strcmp (ptr, "dot32") == 0) {
                  symbol->f_mark = 34;
               } else if (strcmp (ptr, "dot33") == 0) {
                  symbol->f_mark = 35;
               } else if (strcmp (ptr, "dot34") == 0) {
                  symbol->f_mark = 36;
               } else if (strcmp (ptr, "dot35") == 0) {
                  symbol->f_mark = 37;
               } else if (strcmp (ptr, "dot36") == 0) {
                  symbol->f_mark = 38;
               } else if (strcmp (ptr, "dot37") == 0) {
                  symbol->f_mark = 39;
               } else if (strcmp (ptr, "dot38") == 0) {
                  symbol->f_mark = 40;
               } else if (strcmp (ptr, "dot39") == 0) {
                  symbol->f_mark = 41;
               } else if (strcmp (ptr, "dot40") == 0) {
                  symbol->f_mark = 42;
               } else {
                  symbol->f_mark = 0;
               }
               break;
            case MAX:
               if (strcmp ((ptr), "-") != 0) {
                  if (ptr[0] == '<') {
                     ptr ++;
                     symbol->f_maxInc = 0;
                  }
                  symbol->Max = atof (ptr);
               }
               break;
            case MIN:
               if (strcmp ((ptr), "-") != 0) {
                  if (ptr[0] == '>') {
                     ptr ++;
                     symbol->f_minInc = 0;
                  }
                  symbol->Min = atof (ptr);
               }
               break;
            case DECIMAL:
               if (strcmp ((ptr), "-") != 0) {
                  symbol->decimal = atoi (ptr);
               }
               break;
            case THICK:
               if (strcmp ((ptr), "-") != 0) {
                  symbol->thick = atoi (ptr);
               }
               break;
         }
         ptr = strchr (ptr2 + 1, '[');
         if (ptr == NULL) {
            break;
         }
      }
   }
   return 0;
}

static int ParseLayerSect (allLayerType * all, layerType *layer, char *var,
                           char *value)
{
   static char *Vars[] = {
      "Filename", "Type", "Symbol", "ShpType", "Field", "Title", "Legend",
      "MatchField", "MatchValue", "LegendStyle", "LegendColor",
      "LegendPixelSize", "TitleColor", "Frame", "LegendFrame", "RampFile",
      "Point", "Lattice", NULL
   };
   enum {
      FILENAME, TYPE, SYMBOL, SHPTYPE, FIELD, TITLE, LEGEND, MATCHFIELD,
      MATCHVALUE, LEGENDSTYLE, LEGENDCOLOR, LEGENDPIXELSIZE, TITLECOLOR,
      FRAME, LEGENDFRAME, RAMPFILE, POINT, LATTICE
   };
   int index;
   int cnt;
   char *first;
   double lat;
   double lon;

   if (GetIndexFromStr (var, Vars, &index) < 0) {
      printf ("Invalid variable '%s'\n", var);
      return -1;
   }
   switch (index) {
      case FILENAME:
         layer->filename = (char *) malloc ((strlen (value) + 1) *
                                            sizeof (char));
         strcpy (layer->filename, value);
         return 0;
      case FRAME:
         layer->frameNum = atoi (value);
         return 0;
      case LEGENDFRAME:
         layer->legendFrameNum = atoi (value);
         return 0;
      case RAMPFILE:
         ReadColorTable (value, layer);
         return 0;
      case POINT:
         first = strtok (value, "[]");
         if (first != NULL) {
            lat = atof (first);
            if ((first = strtok (NULL, "[]")) != NULL) {
               lon = atof (first);
               /* Store lat/lon */
               layer->numPnt++;
               layer->pnt = (LatLon *) realloc (layer->pnt,
                                                layer->numPnt *
                                                sizeof (LatLon));
               layer->pnt[layer->numPnt - 1].lat = lat;
               layer->pnt[layer->numPnt - 1].lon = lon;

               while ((first = strtok (NULL, "[]")) != NULL) {
                  lat = atof (first);
                  if ((first = strtok (NULL, "[]")) != NULL) {
                     lon = atof (first);
                     /* Store lat/lon */
                     layer->numPnt++;
                     layer->pnt = (LatLon *) realloc (layer->pnt,
                                                      layer->numPnt *
                                                      sizeof (LatLon));
                     layer->pnt[layer->numPnt - 1].lat = lat;
                     layer->pnt[layer->numPnt - 1].lon = lon;
                  }
               }

            }
         }
         return 0;
      case LATTICE:
         first = strtok (value, "[]");
         if (first != NULL) {
            layer->lattice.f_valid = 1;
            ParseColor (&(layer->lattice.fg), first);
            layer->lattice.spacing = 5;
            layer->lattice.style = 3;
            /* User needs to put something in LRTB spot to disable this
             * default.  "none" would work. */
            strcpy (layer->lattice.labelSite, "-LRTB");
            cnt = 1;
            while ((first = strtok (NULL, "[]")) != NULL) {
               cnt++;
               switch (cnt) {
                  case 2:
                     layer->lattice.spacing = atof (first);
                     break;
                  case 3:
                     layer->lattice.style = atoi (first);
                     break;
                  case 4:
                     layer->lattice.labelSite[0] = '\0';
                     layer->lattice.labelSite[1] = '\0';
                     layer->lattice.labelSite[2] = '\0';
                     layer->lattice.labelSite[3] = '\0';
                     layer->lattice.labelSite[4] = '\0';
                     strncpy (layer->lattice.labelSite, first, 5);
                     layer->lattice.labelSite[5] = '\0';
                     break;
               }
            }
         }
         return 0;
      case TITLE:
         first = strtok (value, "[]");
         if (first != NULL) {
            layer->title.name = (char *) malloc ((strlen (first) + 1) *
                                                 sizeof (char));
            strcpy (layer->title.name, first);
            layer->title.fg.r = 0;
            layer->title.fg.g = 0;
            layer->title.fg.b = 0;
            layer->title.fg.alpha = 255;
            layer->title.bg.r = 0;
            layer->title.bg.g = 0;
            layer->title.bg.b = 0;
            layer->title.bg.alpha = 255;
            layer->title.textC.r = -1;
            layer->title.textC.g = -1;
            layer->title.textC.b = -1;
            layer->title.textC.alpha = 255;
            layer->title.x = 0;
            layer->title.y = 0;
            layer->title.f_font = 5; /* default to giant. */
            cnt = 1;
            while ((first = strtok (NULL, "[]")) != NULL) {
               cnt++;
               switch (cnt) {
                  case 2:
                     ParseColor (&(layer->title.fg), first);
                     break;
                  case 3:
                     ParseColor (&(layer->title.bg), first);
                     break;
                  case 4:
                     layer->title.x = atoi (first);
                     break;
                  case 5:
                     layer->title.y = atoi (first);
                     break;
                  case 6:
                     if (strcmp (first, "tiny") == 0) {
                        layer->title.f_font = 1;
                     } else if (strcmp (first, "small") == 0) {
                        layer->title.f_font = 2;
                     } else if (strcmp (first, "medium") == 0) {
                        layer->title.f_font = 3;
                     } else if (strcmp (first, "large") == 0) {
                        layer->title.f_font = 4;
                     } else if (strcmp (first, "giant") == 0) {
                        layer->title.f_font = 5;
                     }
                     break;
               }
            }
         }
         return 0;
      case LEGEND:
         first = strtok (value, "[]");
         if (first != NULL) {
            layer->legend.f_valid = 1;
            ParseColor (&(layer->legend.fg), first);
            layer->legend.bg.r = 0;
            layer->legend.bg.g = 0;
            layer->legend.bg.b = 0;
            layer->legend.bg.alpha = 255;
            layer->legend.textC.r = -1;
            layer->legend.textC.g = -1;
            layer->legend.textC.b = -1;
            layer->legend.textC.alpha = 255;
            layer->legend.pixelSize = 2;
            layer->legend.x = 0;
            layer->legend.y = 0;
            layer->legend.decimal = 1;
            layer->legend.style = 0;
            layer->legend.f_vert = 1;
            layer->legend.wid = -1;
            layer->legend.hei = -1;
            cnt = 1;
            while ((first = strtok (NULL, "[]")) != NULL) {
               cnt++;
               switch (cnt) {
                  case 2:
                     ParseColor (&(layer->legend.bg), first);
                     break;
                  case 3:
                     layer->legend.x = atoi (first);
                     break;
                  case 4:
                     layer->legend.y = atoi (first);
                     break;
                  case 5:
                     layer->legend.decimal = atoi (first);
                     break;
                  case 6:
                     if ((first[0] == 'h') || (first[0] == 'H')) {
                        layer->legend.f_vert = 0;
                     }
                     break;
                  case 7:
                     layer->legend.wid = atoi (first);
                     break;
                  case 8:
                     layer->legend.hei = atoi (first);
                     break;
               }
            }
         }
         return 0;
      case LEGENDSTYLE:
         if (strcmp (value, "GE") == 0) {
            layer->legend.style = 1;
         } else if (strcmp (value, "LE") == 0) {
            layer->legend.style = 2;
         }
         return 0;
      case TITLECOLOR:
         layer->title.textC.r = 0;
         layer->title.textC.g = 0;
         layer->title.textC.b = 0;
         layer->title.textC.alpha = 255;
         first = strtok (value, "[]");
         if (first != NULL) {
            ParseColor (&(layer->legend.textC), first);
         }
         return 0;
      case LEGENDCOLOR:
         layer->legend.textC.r = 0;
         layer->legend.textC.g = 0;
         layer->legend.textC.b = 0;
         layer->legend.textC.alpha = 255;
         first = strtok (value, "[]");
         if (first != NULL) {
            ParseColor (&(layer->legend.textC), first);
         }
         return 0;
      case LEGENDPIXELSIZE:
         layer->legend.pixelSize = atoi (value);
         return 0;
      case TYPE:
         if (GetIndexFromStr (value, LayerTypes, &index) < 0) {
            printf ("Invalid Layer Type '%s'\n", value);
            FreeLayer (layer);
            return -1;
         }
         layer->type = index;
         switch (layer->type) {
            case INVALID:
               break;
            case GRADUATED:
               layer->grad.numSymbol = 0;
               layer->grad.symbol = NULL;
               layer->grad.field = NULL;
               break;
            case INFO:
               layer->numPnt = 0;
               layer->pnt = NULL;
               break;
            case GRID:
               layer->ramp.numColors = 0;
               layer->ramp.colors = NULL;
               layer->ramp.labRay = NULL;
               layer->ramp.label = NULL;
               layer->ramp.labJust = NULL;
               layer->ramp.numLab = 0;
               layer->ramp.f_missing = 0;
               layer->ramp.thick = 0;
               break;
            case SINGLE_SYMBOL:
               layer->single.mark = NULL;
               break;
            case DB2:
               layer->db2.field = NULL;
               break;
         }
         return 0;
      case SHPTYPE:
         if (GetIndexFromStr (value, ShpFileTypes, &index) < 0) {
            printf ("Invalid Shapefile Type '%s'\n", value);
            FreeLayer (layer);
            return -1;
         }
         layer->shpType = index;
         return 0;
      case SYMBOL:
         switch (layer->type) {
            case SINGLE_SYMBOL:
               ParseSymbol (&(layer->single), value);
               break;
            case GRADUATED:
               layer->grad.numSymbol++;
               layer->grad.symbol = (SymbolType *)
                     realloc ((void *) layer->grad.symbol,
                              layer->grad.numSymbol * sizeof (SymbolType));
               ParseSymbol (&(layer->grad.symbol[layer->grad.numSymbol - 1]),
                            value);
               break;
         }
         return 0;
      case MATCHVALUE:
         layer->numMatchVal++;
         layer->matchVal = (char **) realloc ((void *) layer->matchVal,
                                              layer->numMatchVal *
                                              sizeof (char *));
         layer->matchVal[layer->numMatchVal - 1] = (char *)
               malloc ((strlen (value) + 1) * sizeof (char));
         strcpy (layer->matchVal[layer->numMatchVal - 1], value);
         return 0;
      case FIELD:
         switch (layer->type) {
            case GRADUATED:
               layer->grad.field = (char *) malloc ((strlen (value) + 1) *
                                                    sizeof (char));
               strcpy (layer->grad.field, value);
               break;
            case DB2:
               layer->db2.field = (char *) malloc ((strlen (value) + 1) *
                                                   sizeof (char));
               strcpy (layer->db2.field, value);
               break;
         }
         return 0;
      case MATCHFIELD:
         layer->matchField = (char *) malloc ((strlen (value) + 1) *
                                              sizeof (char));
         strcpy (layer->matchField, value);
         return 0;
   }
   printf ("Unrecognized Index %s -> %d\n", var, index);
   return 0;
}

static int ParseAllLayerSect (mapIniType * mapIni, char *var, char *value)
{
   int i;

   static char *Vars[] = { "numLayer", "Background", NULL };
   enum { NUMLAYER, BACKGROUND };
   int index;

   if (GetIndexFromStr (var, Vars, &index) < 0) {
      printf ("Invalid variable '%s'\n", var);
      return -1;
   }
   switch (index) {
      case NUMLAYER:
         mapIni->all.numLayer = atoi (value);
         mapIni->all.layers = (layerType *) malloc (mapIni->all.numLayer *
                                                    sizeof (layerType));
         for (i = 0; i < mapIni->all.numLayer; i++) {
            InitLayer (&(mapIni->all.layers[i]));
         }
         return 0;
      case BACKGROUND:
         /* Get rid of ], and then step past [ */
         value[strlen (value) - 1] = '\0';
         ParseColor (&(mapIni->all.bg), value + 1);
         return 0;
   }
   return 1;
}

void InitMapIni (mapIniType * mapIni)
{
   mapIni->zoom.f_flag = 0;
   mapIni->zoom.file = NULL;
   mapIni->proj.f_flag1 = 0;
   mapIni->proj.f_flag2 = 0;
   mapIni->out.numOutputs = 0;
   mapIni->out.outputs = NULL;
   mapIni->out.active = NULL;
   mapIni->out.numActive = 0;
   mapIni->out.f_flag = 0;
   mapIni->out.numFrame = 0;
   mapIni->out.frame = NULL;
   mapIni->all.numLayer = 0;
   mapIni->all.bg.r = 0;
   mapIni->all.bg.g = 0;
   mapIni->all.bg.b = 0;
   mapIni->all.bg.alpha = 255;
   mapIni->all.layers = NULL;
}

void FreeMapIni (mapIniType * mapIni)
{
   int i;

   if (mapIni->out.numOutputs > 0) {
      for (i = 0; i < mapIni->out.numOutputs; i++) {
         free (mapIni->out.outputs[i].filename);
         free (mapIni->out.outputs[i].active);
      }
   }
   free (mapIni->out.active);
   if (mapIni->out.numFrame > 0) {
      free (mapIni->out.frame);
      mapIni->out.numFrame = 0;
   }
   free (mapIni->out.outputs);
   if (mapIni->all.numLayer != 0) {
      for (i = 0; i < mapIni->all.numLayer; i++) {
         FreeLayer (&(mapIni->all.layers[i]));
      }
   }
   free (mapIni->all.layers);
   if (mapIni->zoom.file != NULL) {
      free (mapIni->zoom.file);
   }
   InitMapIni (mapIni);
}

int ParseMapIniFile (mapIniType * mapIni, const char *options, FILE *fp)
{
   char *buffer = NULL; /* Holds a line from the file. */
   size_t buffLen = 0;  /* Current length of buffer. */
   char *first;
   char *second;
   char *third;
   static char *Sections[] = {
      "Zoom", "Projection", "AllOutput", "Output", "AllLayer", "Layer", NULL
   };
   enum { ZOOM, PROJECTION, ALLOUTPUT, OUTPUT, ALLLAYER, LAYER };
   int curSect = -1;
   int index;
   int which = -1;
   int orig;
   int i;
   int i_temp;

   while (reallocFGets (&buffer, &buffLen, fp) > 0) {
      first = buffer;
      while ((isspace (*first)) && (*first != '\0')) {
         first++;
      }
      if ((first != NULL) && (*first != '#')) {
         if (*first == '[') {
            second = strchr (first, ']');
            if (second != NULL) {
               *second = '\0';
               if (GetIndexFromStr (first + 1, Sections, &index) < 0) {
                  printf ("Invalid section '%s'\n", first + 1);
               } else {
                  curSect = index;
                  /* if Dealing with a "Layer" or a "Output" section, Grab
                   * which one. */
                  which = -1;
                  if ((curSect == LAYER) || (curSect == OUTPUT)) {
                     *second = ' ';
                     first = strchr (second, '[');
                     if (first != NULL) {
                        second = strchr (first, ']');
                        if (second != NULL) {
                           *second = '\0';
                           which = atoi (first + 1);
                        }
                     }
                     if (curSect == LAYER) {
                        if (which < 1) {
                           printf ("Caution: Skipping this Layer section, "
                                   "since it didn't have a valid index. "
                                   "[%d]\n", which);
                           which = -1;
                        } else if (which > mapIni->all.numLayer) {
                           orig = mapIni->all.numLayer;
                           mapIni->all.numLayer = which;
                           mapIni->all.layers = (layerType *)
                                 realloc ((void *) mapIni->all.layers,
                                          mapIni->all.numLayer *
                                          sizeof (layerType));
                           for (i = orig; i < mapIni->all.numLayer; i++) {
                              InitLayer (&mapIni->all.layers[i]);
                           }
                        } else if (mapIni->all.layers[which - 1].type !=
                                   INVALID) {
                           printf ("Already have a valid layer %d\n", which);
                           which = -1;
                        }
                     } else {
                        if (which < 1) {
                           printf ("Caution: Skipping this Output section, "
                                   "since it didn't have a valid index. "
                                   "[%d]\n", which);
                           which = -1;
                        } else if (which > mapIni->out.numOutputs) {
                           orig = mapIni->out.numOutputs;
                           mapIni->out.numOutputs = which;
                           mapIni->out.outputs = (outputType *)
                                 realloc ((void *) mapIni->out.outputs,
                                          mapIni->out.numOutputs *
                                          sizeof (outputType));
                           for (i = orig; i < mapIni->out.numOutputs; i++) {
                              InitOutput (&mapIni->out.outputs[i]);
                           }
                           mapIni->out.outputs[which - 1].f_valid = 1;
                        } else if (mapIni->out.outputs[which - 1].filename !=
                                   NULL) {
                           printf ("Already have a valid output %d\n", which);
                           which = -1;
                        } else {
                           mapIni->out.outputs[which - 1].f_valid = 1;
                        }
                     }
                  }
               }
            }
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
               switch (curSect) {
                  case -1:
                     break;
                  case ZOOM:
                     ParseZoomSect (mapIni, first, second);
                     break;
                  case PROJECTION:
                     ParseProjectSect (mapIni, first, second);
                     break;
                  case ALLOUTPUT:
                     ParseAllOutputSect (mapIni, first, second);
                  case OUTPUT:
                     if (which >= 1) {
                        if (which > mapIni->out.numOutputs) {
                           printf ("Invalid output %d\n", which);
                        } else {
/* *INDENT-OFF* */
                           ParseOutputSect (&(mapIni->out),
                                            &(mapIni->out.outputs[which - 1]),
                                            first, second);
/* *INDENT-ON*  */
                        }
                     }
                     break;
                  case ALLLAYER:
                     ParseAllLayerSect (mapIni, first, second);
                  case LAYER:
                     if (which >= 1) {
                        /* layer is coming from a [1..n] reference system. */
                        if (which > mapIni->all.numLayer) {
                           printf ("Invalid layer %d\n", which);
                        } else {
/* *INDENT-OFF* */
                           if (ParseLayerSect (&(mapIni->all),
                                             &(mapIni->all.layers[which - 1]),
                                               first, second) != 0) {
                              printf ("Error in ParseLayerSect\n");
                              return -1;
                           }
/* *INDENT-ON*  */
                        }
                     }
                     break;
                  default:
                     printf ("Should be impossible to get here\n");
               }
            }
         }
      }
   }
   free (buffer);

   /* Check if the X_size, Y_size is large enough. */
   for (i = 0; i < mapIni->out.numFrame; i++) {
      if (mapIni->out.frame[i].X0 > mapIni->out.frame[i].X1) {
         i_temp = mapIni->out.frame[i].X0;
         mapIni->out.frame[i].X0 = mapIni->out.frame[i].X1;
         mapIni->out.frame[i].X1 = i_temp;
      }
      mapIni->out.frame[i].X_Size = (mapIni->out.frame[i].X1 -
                                     mapIni->out.frame[i].X0);
      if (mapIni->out.frame[i].Y0 > mapIni->out.frame[i].Y1) {
         i_temp = mapIni->out.frame[i].Y0;
         mapIni->out.frame[i].Y0 = mapIni->out.frame[i].Y1;
         mapIni->out.frame[i].Y1 = i_temp;
      }
      mapIni->out.frame[i].Y_Size = (mapIni->out.frame[i].Y1 -
                                     mapIni->out.frame[i].Y0);
/*
      printf ("%d %d %d %d %d\n", mapIni->out.frame[i].ID,
              mapIni->out.frame[i].X0, mapIni->out.frame[i].Y0,
              mapIni->out.frame[i].X1, mapIni->out.frame[i].Y1);
      printf ("%d %d %d\n", mapIni->out.frame[i].bg.r,
              mapIni->out.frame[i].bg.g, mapIni->out.frame[i].bg.b);
*/
      if (mapIni->out.frame[i].X1 > mapIni->out.X_Size) {
         mapIni->out.X_Size = mapIni->out.frame[i].X1;
      }
      if (mapIni->out.frame[i].Y1 > mapIni->out.Y_Size) {
         mapIni->out.Y_Size = mapIni->out.frame[i].Y1;
      }
   }
   return 0;
}

int ValidateMapIni (mapIniType * mapIni)
{
/*
   double d_temp;
*/

   /* Validate zoom section. */
   if (mapIni->zoom.f_flag != 15) {
      printf ("Zoom Section was not completed.\n");
      printf ("Using Defaults\n");
      if (!(mapIni->zoom.f_flag & 1))
         mapIni->zoom.lat1 = 20;
      if (!(mapIni->zoom.f_flag & 2))
         mapIni->zoom.lon1 = -121;
      if (!(mapIni->zoom.f_flag & 4))
         mapIni->zoom.lat2 = 40;
      if (!(mapIni->zoom.f_flag & 8))
         mapIni->zoom.lon2 = -70;
      mapIni->zoom.f_flag = 15;
   }
   if ((mapIni->zoom.lat1 == mapIni->zoom.lat2) ||
       (mapIni->zoom.lon1 == mapIni->zoom.lon2)) {
      printf ("ERROR: [zoom] lat1 = lat2 or lon1 = lon2\n");
      return -1;
   }
/*
   if (mapIni->zoom.lat1 > mapIni->zoom.lat2) {
      d_temp = mapIni->zoom.lat1;
      mapIni->zoom.lat1 = mapIni->zoom.lat2;
      mapIni->zoom.lat2 = d_temp;
   }
   if (mapIni->zoom.lon1 > mapIni->zoom.lon2) {
      d_temp = mapIni->zoom.lon1;
      mapIni->zoom.lon1 = mapIni->zoom.lon2;
      mapIni->zoom.lon2 = d_temp;
   }
*/
   if ((mapIni->zoom.lat1 > 90) || (mapIni->zoom.lat1 < -90) ||
       (mapIni->zoom.lat2 > 90) || (mapIni->zoom.lat2 < -90)) {
      printf ("ERROR: [zoom] lat1, or lat2 out of range\n");
      return -1;
   }
   if ((mapIni->zoom.lon1 > 360) || (mapIni->zoom.lon1 < -360) ||
       (mapIni->zoom.lon2 > 360) || (mapIni->zoom.lon2 < -360)) {
      printf ("ERROR: [zoom] lon1, or lon2 out of range\n");
      return -1;
   }

   /* Validate projection section. */
   if (mapIni->proj.f_flag1 != 31) {
      if ((mapIni->proj.f_flag1 & 24) != 24) {
         printf ("Projection Section is missing lat1, lon1\n");
         printf ("Using defaults\n");
      }
      if (!(mapIni->proj.f_flag1 & 1))
         mapIni->proj.type = 30;
      if (!(mapIni->proj.f_flag1 & 2)) {
         if (!(mapIni->proj.f_flag1 & 4)) {
            mapIni->proj.majEarth = 6367.47; /* km */
            mapIni->proj.minEarth = 6367.47;
            mapIni->proj.f_flag1 |= 4;
         } else {
            mapIni->proj.majEarth = mapIni->proj.minEarth;
         }
      }
      if (!(mapIni->proj.f_flag1 & 4))
         mapIni->proj.minEarth = mapIni->proj.majEarth;
      if (!(mapIni->proj.f_flag1 & 8))
         mapIni->proj.lat1 = 20.192;
      if (!(mapIni->proj.f_flag1 & 16))
         mapIni->proj.lon1 = -121.554;
      mapIni->proj.f_flag1 = 31;
   }
   if (mapIni->proj.f_flag2 != 31) {
      if ((mapIni->proj.f_flag2 & 6) != 6) {
         printf ("Projection Section is missing orientLon, meshLat\n");
         printf ("Using defaults\n");
      }
      if (!(mapIni->proj.f_flag2 & 1))
         mapIni->proj.mesh = 5.079406; /* km */
      if (!(mapIni->proj.f_flag2 & 2))
         mapIni->proj.orientLon = -95;
      if (!(mapIni->proj.f_flag2 & 4))
         mapIni->proj.meshLat = 25;
      if (!(mapIni->proj.f_flag2 & 8))
         mapIni->proj.scaleLat1 = mapIni->proj.meshLat;
      if (!(mapIni->proj.f_flag2 & 16))
         mapIni->proj.scaleLat2 = mapIni->proj.scaleLat1;
      mapIni->proj.f_flag2 = 31;
   }
   if (mapIni->proj.majEarth != mapIni->proj.minEarth) {
      printf ("ERROR: [projection] Can not handle a non-spherical earth\n");
      return -1;
   }
   if ((mapIni->proj.lat1 > 90) || (mapIni->proj.lat1 < -90)) {
      printf ("ERROR: [projection] lat1 out of range\n");
      return -1;
   }
   if ((mapIni->proj.lon1 > 360) || (mapIni->proj.lon1 < -360)) {
      printf ("ERROR: [projection] lon1 out of range\n");
      return -1;
   }
   if ((mapIni->proj.orientLon > 360) || (mapIni->proj.orientLon < -360)) {
      printf ("ERROR: [projection] orientLon out of range\n");
      return -1;
   }
   if ((mapIni->proj.meshLat > 90) || (mapIni->proj.meshLat < 0) ||
       (mapIni->proj.scaleLat1 > 90) || (mapIni->proj.scaleLat1 < 0) ||
       (mapIni->proj.scaleLat2 > 90) || (mapIni->proj.scaleLat2 < 0)) {
      printf ("ERROR: [projection] lat1, scaleLat1, or scaleLat2 out of "
              "range\n");
      return -1;
   }

   /* Validate output section. */
   if (mapIni->out.f_flag != 3) {
      printf ("Output Section is missing X_Size, Y_Size\n");
      printf ("Using defaults\n");
      if (!(mapIni->out.f_flag & 1))
         mapIni->out.X_Size = 800;
      if (!(mapIni->out.f_flag & 2))
         mapIni->out.Y_Size = 800;
      mapIni->out.f_flag = 3;
   }
   if ((mapIni->out.X_Size <= 0) || (mapIni->out.Y_Size <= 0)) {
      printf ("ERROR: [output] X_Size or Y_Size is <= 0\\n");
      return -1;
   }
   if (mapIni->out.numOutputs == 0) {
      printf ("ERROR: [output] numOutputs is not defined?\n");
      return -1;
   }

   /* Validate layer section. */
   if (mapIni->all.numLayer == 0) {
      printf ("ERROR: [layer] numLayers is not defined?\n");
      return -1;
   }
   return 0;
}

int SaveMapIniFile (mapIniType * mapIni, char *filename)
{
   FILE *fp;
   int i;
   size_t j;

   if ((fp = fopen (filename, "wt")) == NULL) {
      printf ("Couldn't open %s for write\n", filename);
      return -1;
   }
   /* Print Zoom section. */
   fprintf (fp, "[Zoom]\n");
   fprintf (fp, "lat1=%f\n", mapIni->zoom.lat1);
   fprintf (fp, "lon1=%f\n", mapIni->zoom.lon1);
   fprintf (fp, "lat2=%f\n", mapIni->zoom.lat2);
   fprintf (fp, "lon2=%f\n", mapIni->zoom.lon2);
   fprintf (fp, "\n");
   /* Print Projection section. */
   fprintf (fp, "[Projection]\n");
   fprintf (fp, "projType=%d\n", mapIni->proj.type);
   fprintf (fp, "majEarth=%f\n", mapIni->proj.majEarth);
   fprintf (fp, "minEarth=%f\n", mapIni->proj.minEarth);
   fprintf (fp, "lat1=%f\n", mapIni->proj.lat1);
   fprintf (fp, "lon1=%f\n", mapIni->proj.lon1);
   fprintf (fp, "mesh=%f\n", mapIni->proj.mesh);
   fprintf (fp, "orientLon=%f\n", mapIni->proj.orientLon);
   fprintf (fp, "meshLat=%f\n", mapIni->proj.meshLat);
   fprintf (fp, "scaleLat1=%f\n", mapIni->proj.scaleLat1);
   fprintf (fp, "scaleLat2=%f\n", mapIni->proj.scaleLat2);
   fprintf (fp, "\n");
   /* Print Output section */
   fprintf (fp, "[AllOutput]\n");
/*
   fprintf (fp, "numOutput=%d\n", mapIni->out.numOutputs);
*/
   fprintf (fp, "X_Size=%d\n", mapIni->out.X_Size);
   fprintf (fp, "Y_Size=%d\n", mapIni->out.Y_Size);
   if (mapIni->out.numActive > 0) {
      fprintf (fp, "Layer=");
      for (j = 0; j < mapIni->out.numActive; j++) {
         fprintf (fp, "[%d]", mapIni->out.active[j]);
      }
      fprintf (fp, "\n");
   }
   fprintf (fp, "\n");
   for (i = 0; i < mapIni->out.numOutputs; i++) {
      if (mapIni->out.outputs[i].f_valid) {
         fprintf (fp, "[Ouput][%d]\n", i + 1);
         fprintf (fp, "Filename=%s\n", mapIni->out.outputs[i].filename);
         fprintf (fp, "Layer=");
         for (j = 0; j < mapIni->out.outputs[i].numActive; j++) {
            fprintf (fp, "[%d]", mapIni->out.outputs[i].active[j]);
         }
         fprintf (fp, "\n\n");
      }
   }
   /* Print AllLayer section */
   fprintf (fp, "[AllLayer]\n");
   fprintf (fp, "Background=[%d,%d,%d]\n", mapIni->all.bg.r,
            mapIni->all.bg.g, mapIni->all.bg.b);
   fprintf (fp, "\n");
   /* Print Layers. */
   fprintf (fp, "#----------\n");
   fprintf (fp, "# Note: Symbol is [outline][foreground][symbol][min][max]"
            "[decimal]\n");
   fprintf (fp, "#----------\n\n");
   for (i = 0; i < mapIni->all.numLayer; i++) {
      if (mapIni->all.layers[i].type != INVALID) {
         fprintf (fp, "[Layer][%d]\n", i + 1);
         fprintf (fp, "Filename=%s\n", mapIni->all.layers[i].filename);
         fprintf (fp, "ShpType=%s\n",
                  ShpFileTypes[mapIni->all.layers[i].shpType]);
         fprintf (fp, "Type=%s\n", LayerTypes[mapIni->all.layers[i].type]);
         switch (mapIni->all.layers[i].type) {
            case SINGLE_SYMBOL:
               fprintf (fp, "Symbol=[%d,%d,%d][%d,%d,%d][%s][#][#][#][%d]\n",
                        mapIni->all.layers[i].single.out.r,
                        mapIni->all.layers[i].single.out.g,
                        mapIni->all.layers[i].single.out.b,
                        mapIni->all.layers[i].single.fg.r,
                        mapIni->all.layers[i].single.fg.g,
                        mapIni->all.layers[i].single.fg.b,
                        mapIni->all.layers[i].single.mark,
                        mapIni->all.layers[i].single.thick);
               break;
            case GRADUATED:
               for (j = 0; j < mapIni->all.layers[i].grad.numSymbol; j++) {
                  fprintf (fp, "Symbol=[%d,%d,%d][%d,%d,%d][%s][",
                           mapIni->all.layers[i].grad.symbol[j].out.r,
                           mapIni->all.layers[i].grad.symbol[j].out.g,
                           mapIni->all.layers[i].grad.symbol[j].out.b,
                           mapIni->all.layers[i].grad.symbol[j].fg.r,
                           mapIni->all.layers[i].grad.symbol[j].fg.g,
                           mapIni->all.layers[i].grad.symbol[j].fg.b,
                           mapIni->all.layers[i].grad.symbol[j].mark);
                  if (! mapIni->all.layers[i].grad.symbol[j].f_minInc) {
                     fprintf (fp, ">");
                  }
                  fprintf (fp, "%f][",
                           mapIni->all.layers[i].grad.symbol[j].Min);
                  if (! mapIni->all.layers[i].grad.symbol[j].f_maxInc) {
                     fprintf (fp, "<");
                  }
                  fprintf (fp, "%f][%d][%d]\n",
                           mapIni->all.layers[i].grad.symbol[j].Max,
                           mapIni->all.layers[i].grad.symbol[j].decimal,
                           mapIni->all.layers[i].grad.symbol[j].thick);
               }
               break;
         }
         switch (mapIni->all.layers[i].type) {
            case GRADUATED:
               fprintf (fp, "Field=%s\n", mapIni->all.layers[i].grad.field);
               break;
            case DB2:
               fprintf (fp, "Field=%s\n", mapIni->all.layers[i].db2.field);
               break;
         }
         if (mapIni->all.layers[i].matchField != NULL) {
            fprintf (fp, "MatchField=%s\n", mapIni->all.layers[i].matchField);
            for (j = 0; j < mapIni->all.layers[i].numMatchVal; j++) {
               fprintf (fp, "MatchValue=%s\n",
                        mapIni->all.layers[i].matchVal[j]);
            }
         }
         if (mapIni->all.layers[i].title.name != NULL) {
            fprintf (fp, "Title=[%s][%d,%d,%d][%d,%d,%d][%d][%d]\n",
                     mapIni->all.layers[i].title.name,
                     mapIni->all.layers[i].title.fg.r,
                     mapIni->all.layers[i].title.fg.g,
                     mapIni->all.layers[i].title.fg.b,
                     mapIni->all.layers[i].title.bg.r,
                     mapIni->all.layers[i].title.bg.g,
                     mapIni->all.layers[i].title.bg.b,
                     mapIni->all.layers[i].title.x,
                     mapIni->all.layers[i].title.y);
         }
         if (mapIni->all.layers[i].lattice.f_valid) {
            fprintf (fp, "Lattice=[%d,%d,%d][%f][%d][%s]\n",
                     mapIni->all.layers[i].lattice.fg.r,
                     mapIni->all.layers[i].lattice.fg.g,
                     mapIni->all.layers[i].lattice.fg.b,
                     mapIni->all.layers[i].lattice.spacing,
                     mapIni->all.layers[i].lattice.style,
                     mapIni->all.layers[i].lattice.labelSite);
         }
         if (mapIni->all.layers[i].legend.f_valid) {
            fprintf (fp, "Legend=[%d,%d,%d][%d,%d,%d][%d][%d][%d]\n",
                     mapIni->all.layers[i].legend.fg.r,
                     mapIni->all.layers[i].legend.fg.g,
                     mapIni->all.layers[i].legend.fg.b,
                     mapIni->all.layers[i].legend.bg.r,
                     mapIni->all.layers[i].legend.bg.g,
                     mapIni->all.layers[i].legend.bg.b,
                     mapIni->all.layers[i].legend.x,
                     mapIni->all.layers[i].legend.y,
                     mapIni->all.layers[i].legend.decimal);
            switch (mapIni->all.layers[i].legend.style) {
               case 1:
                  fprintf (fp, "LegendStyle=GE\n");
                  break;
               case 2:
                  fprintf (fp, "LegendStyle=LE\n");
                  break;
               default:
                  fprintf (fp, "LegendStyle=Range\n");
                  break;
            }
         }
         fprintf (fp, "\n");
      }
   }
   return 0;
}
