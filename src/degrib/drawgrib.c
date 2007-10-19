#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "myerror.h"
#include "mymapf.h"
#include "drawgrib.h"
#include "mapini.h"
#include "drawlib.h"
#include "myassert.h"

#include "clock.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

typedef struct {
   uChar r, g, b;
} colorTableType;

colorTableType DefaultColors[] = {
   {255, 200, 255}, {255, 180, 255}, {255, 165, 255}, {255, 150, 255},
   {250, 145, 250}, {245, 140, 245}, {240, 135, 240}, {235, 130, 235},
   {230, 125, 230}, {225, 120, 225}, {220, 115, 220}, {215, 110, 215},
   {212, 107, 212}, {210, 105, 210}, {200, 100, 200}, {191, 93, 198},
   {182, 86, 196}, {173, 79, 193}, {164, 72, 191}, {155, 65, 189},
   {146, 58, 187}, {135, 50, 185}, {126, 46, 182}, {117, 42, 178},
   {108, 38, 174}, {99, 34, 170}, {90, 30, 166}, {81, 26, 162},
   {72, 22, 158}, {63, 18, 154}, {55, 14, 152}, {50, 10, 150},
   {45, 32, 160}, {40, 54, 170}, {35, 76, 180}, {30, 98, 190},
   {28, 110, 195}, {25, 120, 200}, {23, 133, 205}, {20, 145, 210},
   {15, 164, 220}, {10, 186, 230}, {5, 207, 243}, {0, 225, 250},
   {0, 224, 239}, {0, 223, 229}, {1, 222, 219}, {1, 221, 209},
   {1, 220, 188}, {1, 215, 167}, {1, 210, 134}, {1, 205, 100},
   {0, 200, 67}, {0, 195, 34}, {0, 190, 0}, {0, 185, 0},
   {0, 190, 0}, {25, 193, 1}, {50, 196, 1}, {75, 199, 2},
   {125, 205, 3}, {150, 208, 3}, {166, 210, 3}, {185, 211, 4},
   {200, 220, 5}, {228, 224, 5}, {234, 229, 4}, {242, 237, 3},
   {245, 241, 3}, {250, 251, 1}, {255, 255, 0}, {255, 240, 0},
   {255, 224, 0}, {255, 216, 0}, {255, 210, 0}, {255, 192, 0},
   {255, 180, 0}, {255, 161, 0}, {255, 155, 0}, {255, 140, 0},
   {255, 125, 0}, {253, 110, 0}, {251, 95, 0}, {250, 80, 0},
   {249, 65, 0}, {248, 35, 0}, {246, 10, 0}, {243, 0, 0},
   {240, 0, 0}, {236, 6, 6}, {233, 9, 9}, {230, 12, 12},
   {220, 19, 19}, {200, 21, 21}, {190, 20, 20}, {185, 18, 18},
   {180, 16, 16}, {176, 12, 12}, {170, 8, 8}, {160, 4, 4},
   {150, 3, 3}
};

static void SetGridRamp (layerType *layer, double min, double max,
                         sChar f_missing, double missing)
{
   int i;
   int defColLen = sizeof (DefaultColors) / sizeof (colorTableType);
   int delt;

   /* + 1 is for missing value. */
   layer->ramp.f_missing = f_missing;
   if (f_missing) {
      layer->ramp.missValue = missing;
      layer->ramp.missColor.r = 255;
      layer->ramp.missColor.g = 255;
      layer->ramp.missColor.b = 255;
      layer->ramp.missColor.f_null = 0;
   }
   layer->ramp.numColors = defColLen;
   layer->ramp.colors = (colorType *) malloc (layer->ramp.numColors *
                                              sizeof (colorType));
   for (i = 0; i < defColLen; i++) {
      layer->ramp.colors[i].r = DefaultColors[i].r;
      layer->ramp.colors[i].g = DefaultColors[i].g;
      layer->ramp.colors[i].b = DefaultColors[i].b;
      layer->ramp.colors[i].f_null = 0;
   }
   layer->ramp.min = min;
   layer->ramp.max = max;
   delt = (int) (2 * (log (max - min) / log (10)) + .5);
   if ((delt % 2) == 0) {
      layer->ramp.labInterval = pow (10, delt / 2);
   } else {
      layer->ramp.labInterval = pow (10, delt / 2) / 2.;
   }
   layer->ramp.labMin = min;
   layer->ramp.labMax = max;
   layer->ramp.decimal = 0;
   layer->ramp.thick = 1;
   layer->ramp.outline.r = 0;
   layer->ramp.outline.g = 0;
   layer->ramp.outline.b = 0;
   layer->ramp.outline.f_null = 0;
}

/*****************************************************************************
 * gribDraw() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This use GD to create an image based on the grid and a control file.
 *
 * ARGUMENTS
 *   Filename = Name of file to save to. (Input)
 *  grib_Data = The grib2 data to write. (Input)
 * mapIniFile = Name of the mapIni file to control the draws. (Input)
 *       meta = Meta Data about the grid. (Input)
 *        usr = User specified options. (Input)
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 *
 * HISTORY
 *  10/2005 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
int drawGrib (const char *Filename, double *grib_Data,
              const char *mapIniFile, const char *mapIniOptions,
              gdsType *gds, double Min, double Max, sChar f_missing,
              double missing, grib_MetaData *meta, userType *usr)
{
   mapIniType mapIni;
   myMaparam map;       /* Used to compute the grid lat/lon points. */
#ifdef TESTING
   double lat, lon;
#endif
   size_t nameLen = strlen (Filename);
   double orientLon;
   FILE *fp;
   int i;

#ifdef DEBUG
   printf ("start drawGrib. %f\n", clock () / (double) (CLOCKS_PER_SEC));
#endif

   if (nameLen < 4) {
      errSprintf ("ERROR: File %s is too short in length (it may need an "
                  "extension?)", Filename);
      return -2;
   }

   /* Set up map projection. */
   if (GDSValid (gds) != 0) {
      preErrSprintf ("ERROR: Grid Definition Section was not Valid.\n");
      return -1;
   }
   SetMapParamGDS (&map, gds);

   InitMapIni (&mapIni);
   if (mapIniFile != NULL) {
      if ((fp = fopen (mapIniFile, "rt")) == NULL) {
         errSprintf ("Couldn't open %s for read\n", mapIniFile);
         FreeMapIni (&mapIni);
         return -1;
      }
      if (ParseMapIniFile (&mapIni, mapIniOptions, fp) != 0) {
         FreeMapIni (&mapIni);
         return 0;
      }
      fclose (fp);
   } else {
      /* Set default background color. */
      mapIni.all.bg.f_null = 1;
      mapIni.all.bg.r = 255;
      mapIni.all.bg.g = 255;
      mapIni.all.bg.b = 255;
   }

   /* Set defaults for zoom. */
   if (mapIni.zoom.f_flag == 0) {
      if (map.f_latlon == 1) {
         mapIni.zoom.lat1 = map.lat1;
         mapIni.zoom.lon1 = map.lon1;
         mapIni.zoom.lat2 = map.latN;
         mapIni.zoom.lon2 = map.lonN;
      } else {
         myCxy2ll (&map, .5, .5, &(mapIni.zoom.lat1), &(mapIni.zoom.lon1));
         myCxy2ll (&map, gds->Nx + .5, gds->Ny + .5, &(mapIni.zoom.lat2),
                   &(mapIni.zoom.lon2));
#ifdef TESTING
         myCxy2ll (&map, .5, .5, &lat, &lon);
         printf ("%f %f\n", lat, lon);
         mapIni.zoom.lat1 = mapIni.zoom.lat2 = lat;
         mapIni.zoom.lon1 = mapIni.zoom.lon2 = lon;
         myCxy2ll (&map, .5, gds->Ny + .5, &lat, &lon);
         printf ("%f %f\n", lat, lon);
         if (lat < mapIni.zoom.lat1)
            mapIni.zoom.lat1 = lat;
         if (lat > mapIni.zoom.lat2)
            mapIni.zoom.lat2 = lat;
         if (lon < mapIni.zoom.lon1)
            mapIni.zoom.lon1 = lon;
         if (lon > mapIni.zoom.lon2)
            mapIni.zoom.lon2 = lon;
         myCxy2ll (&map, gds->Nx + .5, gds->Ny + .5, &lat, &lon);
         printf ("%f %f\n", lat, lon);
         if (lat < mapIni.zoom.lat1)
            mapIni.zoom.lat1 = lat;
         if (lat > mapIni.zoom.lat2)
            mapIni.zoom.lat2 = lat;
         if (lon < mapIni.zoom.lon1)
            mapIni.zoom.lon1 = lon;
         if (lon > mapIni.zoom.lon2)
            mapIni.zoom.lon2 = lon;
         myCxy2ll (&map, gds->Nx + .5, .5, &lat, &lon);
         printf ("%f %f\n", lat, lon);
         if (lat < mapIni.zoom.lat1)
            mapIni.zoom.lat1 = lat;
         if (lat > mapIni.zoom.lat2)
            mapIni.zoom.lat2 = lat;
         if (lon < mapIni.zoom.lon1)
            mapIni.zoom.lon1 = lon;
         if (lon > mapIni.zoom.lon2)
            mapIni.zoom.lon2 = lon;
         printf ("%f %f\n", lat, lon);
         printf ("%f %f %f %f\n", mapIni.zoom.lat1, mapIni.zoom.lon1,
                 mapIni.zoom.lat2, mapIni.zoom.lon2);
#endif
      }
      mapIni.zoom.f_flag = 15;
   }

   /* Set defaults for projection section. */
   if ((mapIni.proj.f_flag1 != 31) || (mapIni.proj.f_flag2 != 31)) {
      mapIni.proj.type = gds->projType;
      mapIni.proj.minEarth = gds->minEarth;
      mapIni.proj.majEarth = gds->majEarth;
      mapIni.proj.lat1 = gds->lat1;
      mapIni.proj.lon1 = gds->lon1;
      mapIni.proj.f_flag1 = 31;
      mapIni.proj.mesh = gds->Dx;
      orientLon = gds->orientLon;
      if (orientLon > 180) {
         orientLon -= 360;
      }
      if (orientLon < -180) {
         orientLon += 360;
      }
      mapIni.proj.orientLon = orientLon;
      mapIni.proj.meshLat = gds->meshLat;
      mapIni.proj.scaleLat1 = gds->scaleLat1;
      mapIni.proj.scaleLat2 = gds->scaleLat2;
      mapIni.proj.f_flag2 = 31;
   }

   if (mapIni.all.numLayer == 0) {
      mapIni.all.numLayer = 1;
      mapIni.all.layers = (layerType *) malloc (mapIni.all.numLayer *
                                                sizeof (layerType));
      InitLayer (&(mapIni.all.layers[0]));
      mapIni.all.layers[0].type = GRID;
      mapIni.all.layers[0].shpType = MEMORY;
      mapIni.all.layers[0].ramp.numColors = 0;
      mapIni.all.layers[0].ramp.colors = NULL;
      mapIni.all.layers[0].ramp.numLab = 0;
      mapIni.all.layers[0].ramp.labRay = NULL;
      mapIni.all.layers[0].ramp.labJust = NULL;
      mapIni.all.layers[0].ramp.label = NULL;
      mapIni.all.layers[0].ramp.f_missing = 0;
      mapIni.all.layers[0].ramp.thick = 0;
   }
   for (i = 0; i < mapIni.all.numLayer; i++) {
      if (mapIni.all.layers[i].shpType == MEMORY) {
         mapIni.all.layers[i].gridData = grib_Data;
         mapIni.all.layers[i].gridNx = gds->Nx;
         mapIni.all.layers[i].gridNy = gds->Ny;
         mapIni.all.layers[i].gridProj.type = gds->projType;
         mapIni.all.layers[i].gridProj.minEarth = gds->minEarth;
         mapIni.all.layers[i].gridProj.majEarth = gds->majEarth;
         mapIni.all.layers[i].gridProj.lat1 = gds->lat1;
         mapIni.all.layers[i].gridProj.lon1 = gds->lon1;
         mapIni.all.layers[i].gridProj.f_flag1 = 31;
         mapIni.all.layers[i].gridProj.mesh = gds->Dx;
         orientLon = gds->orientLon;
         if (orientLon > 180) {
            orientLon -= 360;
         }
         if (orientLon < -180) {
            orientLon += 360;
         }
         mapIni.all.layers[i].gridProj.orientLon = orientLon;
         mapIni.all.layers[i].gridProj.meshLat = gds->meshLat;
         mapIni.all.layers[i].gridProj.scaleLat1 = gds->scaleLat1;
         mapIni.all.layers[i].gridProj.scaleLat2 = gds->scaleLat2;
         mapIni.all.layers[i].gridProj.f_flag2 = 31;
         if (mapIni.all.layers[i].ramp.numColors == 0) {
            SetGridRamp (&(mapIni.all.layers[i]), Min, Max, f_missing,
                         missing);
         } else {
            if (f_missing) {
               mapIni.all.layers[i].ramp.missValue = missing;
               mapIni.all.layers[i].ramp.f_missing = f_missing;
            }
         }
      }
   }

   /* Set defaults for output. */
   if (mapIni.out.f_flag != 3) {
      mapIni.out.X_Size = 520;
      mapIni.out.Y_Size = 420;
      mapIni.out.f_flag = 3;
   }

   if (mapIni.out.numOutputs == 0) {
      mapIni.out.numOutputs = 1;
      mapIni.out.outputs = (outputType *) malloc (mapIni.out.numOutputs *
                                                  sizeof (outputType));
      mapIni.out.outputs[0].numActive = 1;
      mapIni.out.outputs[0].active =
            (int *) malloc (mapIni.out.outputs[0].numActive * sizeof (int));
      mapIni.out.outputs[0].active[0] = 1;
      mapIni.out.outputs[0].filename = NULL;
   }

   for (i = 0; i < mapIni.out.numOutputs; i++) {
      if (mapIni.out.outputs[i].filename == NULL) {
         mapIni.out.outputs[i].filename =
               (char *) malloc (nameLen + 1 * sizeof (char));
         strncpy (mapIni.out.outputs[i].filename, Filename, nameLen - 3);
         strncpy (mapIni.out.outputs[i].filename + nameLen - 3, "png", 3);
         mapIni.out.outputs[i].filename[nameLen] = '\0';
      }
   }

   if (ValidateMapIni (&mapIni) != 0) {
      printf ("MapINI was invalid \n");
      FreeMapIni (&mapIni);
      return 0;
   }
#ifdef DEBUG
   printf ("Before ControlDraw. %f\n", clock () / (double) (CLOCKS_PER_SEC));
#endif
   ControlDraw (&mapIni);
#ifdef DEBUG
   printf ("after ControlDraw. %f\n", clock () / (double) (CLOCKS_PER_SEC));
#endif

   FreeMapIni (&mapIni);
   return 0;
}
