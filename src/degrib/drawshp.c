#define PROGRAM_VERSION "1.07"
#define PROGRAM_DATE "5/20/2009"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mapini.h"
#include "drawlib.h"

void DrawShpAbout (void)
{
   printf ("drawshp\nVersion: %s\nDate: %s\n"
           "Author: Arthur Taylor\n", PROGRAM_VERSION, PROGRAM_DATE);
}

/*
 * drawshp foo.ini
 * "layer:12:Title=[Cindy2005\n\nAs of:\n05/07 09Z\n\nPrimary\n Surge\n Feet][-1,-1,-1][255,255,255][0][20]"
 * "layer:11:Filename=./cindy_07050900_prim.shp"
 * "Frame:0:[90][0][670][600][0,128,255][./cindy_07050900_prim.shp]"
 */
int main (int argc, char **argv)
{
   mapIniType mapIni;
   FILE *fp;
   char f_stdin;

   if (argc != 2) {
      printf ("Usage: %s <user .ini control file>\n", argv[0]);
      printf ("Typically the .ini control file is 'demo.ini'\n");
      return 0;
   }
   if (strcmp (argv[1], "-V") == 0) {
      DrawShpAbout ();
      return 0;
   }
   f_stdin = 0;
   if (strcmp (argv[1], "-stdin") == 0) {
      f_stdin = 1;
   }

   fprintf (stderr, "start :: %f [sec]\n", clock () /
            (double) (CLOCKS_PER_SEC));
   InitMapIni (&mapIni);
   if (!f_stdin) {
      if ((fp = fopen (argv[1], "rt")) == NULL) {
         printf ("Couldn't open %s for read\n", argv[1]);
         FreeMapIni (&mapIni);
         return 0;
      }
      if (ParseMapIniFile (&mapIni, NULL, fp) != 0) {
         FreeMapIni (&mapIni);
         return 0;
      }
      fclose (fp);
   } else {
      if (ParseMapIniFile (&mapIni, NULL, stdin) != 0) {
         FreeMapIni (&mapIni);
         return 0;
      }
   }

   if (mapIni.zoom.file != NULL) {
      if (ReadShpBounds (mapIni.zoom.file, &(mapIni.zoom.lon2),
                         &(mapIni.zoom.lat2), &(mapIni.zoom.lon1),
                         &(mapIni.zoom.lat1)) == 0) {
         mapIni.zoom.f_flag = 15;
      }
   }

   if (ValidateMapIni (&mapIni) != 0) {
      FreeMapIni (&mapIni);
      return 0;
   }
/*
   SaveMapIniFile (&mapIni, "dummy.ini");
*/

   ControlDraw (&mapIni);

   FreeMapIni (&mapIni);
   fprintf (stderr, "Stop :: %f [sec]\n", clock () /
            (double) (CLOCKS_PER_SEC));
   return 0;
}
