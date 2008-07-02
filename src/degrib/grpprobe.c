#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "grpprobe.h"
#include "genprobe.h"
#include "clock.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif
#include "myassert.h"
#include "myutil.h"
#include "myerror.h"

int matchCompare2 (const void *A, const void *B)
{
   const genMatchType *a = (genMatchType *) A;
   const genMatchType *b = (genMatchType *) B;

   if (a->validTime < b->validTime)
      return -1;
   if (a->validTime > b->validTime)
      return 1;
/*
   if (a->elem.ndfdEnum < b->elem.ndfdEnum)
      return -1;
   if (a->elem.ndfdEnum > b->elem.ndfdEnum)
      return 1;
*/
   return 0;
}

static const char *DegToCompass (double windDir)
{
   while (windDir < 0)
      windDir += 360;
   while (windDir > 360)
      windDir -= 360;

   if (windDir <= 11.25)
      return "N";
   if (windDir <= 33.75)
      return "NNE";
   if (windDir <= 56.25)
      return "NE";
   if (windDir <= 78.75)
      return "ENE";
   if (windDir <= 101.25)
      return "E";
   if (windDir <= 123.75)
      return "ESE";
   if (windDir <= 146.25)
      return "SE";
   if (windDir <= 168.75)
      return "SSE";
   if (windDir <= 191.25)
      return "S";
   if (windDir <= 213.75)
      return "SSW";
   if (windDir <= 236.25)
      return "SW";
   if (windDir <= 258.75)
      return "WSW";
   if (windDir <= 281.25)
      return "W";
   if (windDir <= 303.75)
      return "WNW";
   if (windDir <= 326.25)
      return "NW";
   if (windDir <= 348.75)
      return "NNW";
   return "N";
}

static void PrintSameTime (genMatchType * match, size_t pntIndex,
                           int *allElem, sChar pntTimeZone, sChar f_dayCheck)
{
   char timeBuff[25];
   char zone[5];
   size_t i;

   for (i = 0; i < NDFD_MATCHALL + 1; i++) {
      if (allElem[i] != -1) {
         Clock_Print2 (timeBuff, 25, match[allElem[i]].validTime,
                       "%A %D %H", pntTimeZone, f_dayCheck);
         /* The 0 is for non-daylight savings time. */
         if ((f_dayCheck) &&
             (Clock_IsDaylightSaving2 (match[allElem[i]].validTime,
                                       pntTimeZone))) {
            Clock_PrintZone2 (zone, pntTimeZone, 1);
         } else {
            Clock_PrintZone2 (zone, pntTimeZone, 0);
         }
         strcat (timeBuff, zone);
         break;
      }
   }
   /* Check that we have some elements. */
   if (i == NDFD_MATCHALL + 1)
      return;

   printf ("%s ", timeBuff);
   if ((allElem[NDFD_TEMP] != -1) &&
       (match[allElem[NDFD_TEMP]].value[pntIndex].valueType != 2))
      printf ("tt:%.0f ", match[allElem[NDFD_TEMP]].value[pntIndex].data);
   if ((allElem[NDFD_AT] != -1) &&
       (match[allElem[NDFD_AT]].value[pntIndex].valueType != 2))
      printf ("at:%.0f ", match[allElem[NDFD_AT]].value[pntIndex].data);

   if ((allElem[NDFD_MAX] != -1) &&
       (match[allElem[NDFD_MAX]].value[pntIndex].valueType != 2)) {
      printf ("mx:%.0f ", match[allElem[NDFD_MAX]].value[pntIndex].data);
   } else if ((allElem[NDFD_MIN] != -1) &&
              (match[allElem[NDFD_MIN]].value[pntIndex].valueType != 2)) {
      printf ("mn:%.0f ", match[allElem[NDFD_MIN]].value[pntIndex].data);
   } else {
      printf ("  :   ");
   }

   if ((allElem[NDFD_TD] != -1) &&
       (match[allElem[NDFD_TD]].value[pntIndex].valueType != 2))
      printf ("td:%.0f ", match[allElem[NDFD_TD]].value[pntIndex].data);
   if ((allElem[NDFD_POP] != -1) &&
       (match[allElem[NDFD_POP]].value[pntIndex].valueType != 2)) {
      printf ("po:%.0f ", match[allElem[NDFD_POP]].value[pntIndex].data);
   } else {
      printf ("  :   ");
   }
   if ((allElem[NDFD_RH] != -1) &&
       (match[allElem[NDFD_RH]].value[pntIndex].valueType != 2))
      printf ("rh:%.0f ", match[allElem[NDFD_RH]].value[pntIndex].data);
   if ((allElem[NDFD_SKY] != -1) &&
       (match[allElem[NDFD_SKY]].value[pntIndex].valueType != 2))
      printf ("sky:%.0f ", match[allElem[NDFD_SKY]].value[pntIndex].data);
   if ((allElem[NDFD_QPF] != -1) &&
       (match[allElem[NDFD_QPF]].value[pntIndex].valueType != 2)) {
      printf ("qp:%5.2f ", match[allElem[NDFD_QPF]].value[pntIndex].data);
   } else {
      printf ("  :      ");
   }
   if ((allElem[NDFD_SNOW] != -1) &&
       (match[allElem[NDFD_SNOW]].value[pntIndex].valueType != 2)) {
      printf ("sn:%5.2f ", match[allElem[NDFD_SNOW]].value[pntIndex].data);
   } else {
      printf ("  :      ");
   }

   if ((allElem[NDFD_WS] != -1) &&
       (match[allElem[NDFD_WS]].value[pntIndex].valueType != 2))
      printf ("ws:%2.0f ", match[allElem[NDFD_WS]].value[pntIndex].data);
   if ((allElem[NDFD_WD] != -1) &&
       (match[allElem[NDFD_WD]].value[pntIndex].valueType != 2)) {
      printf ("wd:%.0f ", match[allElem[NDFD_WD]].value[pntIndex].data);
/*
      printf ("wd:%3s ",
              DegToCompass (match[allElem[NDFD_WD]].value[pntIndex].data));
*/
   }
   if ((allElem[NDFD_WH] != -1) &&
       (match[allElem[NDFD_WH]].value[pntIndex].valueType != 2))
      printf ("wh:%.0f ", match[allElem[NDFD_WH]].value[pntIndex].data);

   if ((allElem[NDFD_WX] != -1) &&
       (match[allElem[NDFD_WX]].value[pntIndex].valueType != 2))
      printf ("\n\t%s ", match[allElem[NDFD_WX]].value[pntIndex].str);

   printf ("\n");
}

static void PrintSameDay1 (genMatchType * match, size_t pntIndex,
                           collateType * collate, size_t numCollate,
                           sChar pntTimeZone, sChar f_dayCheck)
{
   size_t i;
   size_t j;
   size_t *dayIndex;
   size_t numDayIndex = 1;
   sInt4 totDay;        /* # of days since epoch in LST (LDT ignored). */
   sInt4 curTotDay;     /* # of days since epoch in LST (LDT ignored). */

   dayIndex = (size_t *) malloc (numDayIndex * sizeof (size_t));
   dayIndex[numDayIndex - 1] = 0;

   curTotDay = (sInt4) floor ((collate[0].validTime -
                               pntTimeZone * 3600) / SEC_DAY);
   for (i = 1; i < numCollate; i++) {
      totDay = (sInt4) floor ((collate[i].validTime -
                               pntTimeZone * 3600) / SEC_DAY);
      if (totDay != curTotDay) {
         curTotDay = totDay;
         numDayIndex++;
         dayIndex =
               (size_t *) realloc (dayIndex, numDayIndex * sizeof (size_t));
         dayIndex[numDayIndex - 1] = i;
      }
   }
   if (dayIndex[numDayIndex - 1] != i) {
      numDayIndex++;
      dayIndex = (size_t *) realloc (dayIndex, numDayIndex * sizeof (size_t));
      dayIndex[numDayIndex - 1] = i;
   }

   for (i = 0; i + 1 < numDayIndex; i++) {
      for (j = dayIndex[i]; j < dayIndex[i + 1]; j++) {
         PrintSameTime (match, pntIndex, collate[j].allElem, pntTimeZone,
                        f_dayCheck);
      }
      printf ("--- End of day ---\n");
   }

   free (dayIndex);
}

typedef struct {
   char f_valSky;
   double sky;
   char f_valPop;
   double pop;
   char f_valQPF;
   double qpf;
   char f_valSnow;
   double snow;

   char f_valMaxMin;
   double MaxMin;
   char f_valTemp;
   double minTemp;
   double maxTemp;
   char f_valAppTemp;
   double appMinTemp;
   double appMaxTemp;

   char f_valWindSpd;
   double windSpd;
   char f_valWindDir;
   double windDir;

   size_t numWx;
   char **wx;
} halfDayStruct;

static void PrintSameDay2 (genMatchType * match, size_t pntIndex,
                           collateType * collate, size_t numCollate,
                           sChar pntTimeZone, sChar f_dayCheck,
                           double refTime, uChar f_MOTD)
{
   size_t i;
   size_t j;
   size_t m;
   size_t *halfDayIndex;
   size_t numHalfDayIndex = 1;
   sInt4 totHalfDay;    /* # of 1/2 days since epoch in LST (LDT ignored). */
   sInt4 curTotHalfDay; /* # of 1/2 days since epoch in LST (LDT ignored). */
   char timeBuff[25];
   sInt4 valDay;
   sInt4 valSec;
   halfDayStruct hd;
   int k;
   sChar f_night;
   const char *ptr;
   sChar f_firstDay;

   halfDayIndex = (size_t *) malloc (numHalfDayIndex * sizeof (size_t));
   halfDayIndex[numHalfDayIndex - 1] = 0;
   /* The 6 * 3600 is to shift from 0am - 12am to 6am - 6pm "chunks" */
   curTotHalfDay = (sInt4) floor ((collate[0].validTime + 6 * 3600 -
                                   pntTimeZone * 3600) / (SEC_DAY / 2));
   for (i = 1; i < numCollate; i++) {
      totHalfDay = (sInt4) floor ((collate[i].validTime + 6 * 3600 -
                                   pntTimeZone * 3600) / (SEC_DAY / 2));
      if (totHalfDay != curTotHalfDay) {
         curTotHalfDay = totHalfDay;
         numHalfDayIndex++;
         halfDayIndex = (size_t *) realloc (halfDayIndex,
                                            numHalfDayIndex *
                                            sizeof (size_t));
         halfDayIndex[numHalfDayIndex - 1] = i;
      }
   }
   if (halfDayIndex[numHalfDayIndex - 1] != i) {
      numHalfDayIndex++;
      halfDayIndex =
            (size_t *) realloc (halfDayIndex,
                                numHalfDayIndex * sizeof (size_t));
      halfDayIndex[numHalfDayIndex - 1] = i;
   }

   f_firstDay = 1;
   for (i = 0; i < numHalfDayIndex - 1; i++) {
      /* Fill out Half day struct. */
      memset (&hd, 0, sizeof (hd));
      hd.sky = 0;
      hd.maxTemp = 9999;
      hd.minTemp = -9999;
      hd.appMaxTemp = 9999;
      hd.appMinTemp = -9999;
      hd.wx = NULL;
      for (j = halfDayIndex[i]; j < halfDayIndex[i + 1]; j++) {
         if (f_MOTD == 2) {
            k = collate[j].allElem[NDFD_TEMP];
            if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
               if (!hd.f_valTemp) {
                  hd.minTemp = match[k].value[pntIndex].data;
                  hd.maxTemp = match[k].value[pntIndex].data;
                  hd.f_valTemp = 1;
               } else {
                  if (match[k].value[pntIndex].data < hd.minTemp)
                     hd.minTemp = match[k].value[pntIndex].data;
                  if (match[k].value[pntIndex].data > hd.maxTemp)
                     hd.maxTemp = match[k].value[pntIndex].data;
               }
            }
            k = collate[j].allElem[NDFD_AT];
            if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
               if (!hd.f_valAppTemp) {
                  hd.appMinTemp = match[k].value[pntIndex].data;
                  hd.appMaxTemp = match[k].value[pntIndex].data;
                  hd.f_valAppTemp = 1;
               } else {
                  if (match[k].value[pntIndex].data < hd.appMinTemp)
                     hd.appMinTemp = match[k].value[pntIndex].data;
                  if (match[k].value[pntIndex].data > hd.appMaxTemp)
                     hd.appMaxTemp = match[k].value[pntIndex].data;
               }
            }
         }

         /* typically sky is not "most cloudy" but range from one to
          * another... see ScalarPhrases.py */
         k = collate[j].allElem[NDFD_SKY];
         if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
            if (match[k].value[pntIndex].data > hd.sky) {
               hd.sky = match[k].value[pntIndex].data;
               hd.f_valSky = 1;
            }
         }

         k = collate[j].allElem[NDFD_QPF];
         if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
            hd.qpf += match[k].value[pntIndex].data;
            hd.f_valQPF = 1;
         }
         k = collate[j].allElem[NDFD_SNOW];
         if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
            if (match[k].value[pntIndex].data > hd.snow) {
               hd.snow += match[k].value[pntIndex].data;
               hd.f_valSnow = 1;
            }
         }
         k = collate[j].allElem[NDFD_WS];
         if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
            if (match[k].value[pntIndex].data > hd.windSpd) {
               hd.windSpd = match[k].value[pntIndex].data;
               hd.f_valWindSpd = 1;
               k = collate[j].allElem[NDFD_WD];
               if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
                  hd.windDir = match[k].value[pntIndex].data;
                  hd.f_valWindDir = 1;
               }
            }
         }
         k = collate[j].allElem[NDFD_WX];
         if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
            if (strcmp (match[k].value[pntIndex].str, "No Weather") != 0) {
               if (hd.numWx == 0) {
                  hd.numWx = 1;
                  hd.wx = (char **) malloc (hd.numWx * sizeof (char *));
                  hd.wx[0] =
                        (char *) malloc (strlen (match[k].value[pntIndex].str)
                                         + 1);
                  strcpy (hd.wx[0], match[k].value[pntIndex].str);
               } else {
                  for (m = 0; m < hd.numWx; m++) {
                     if (strcmp (hd.wx[m], match[k].value[pntIndex].str) == 0) {
                        break;
                     }
                  }
                  if (m == hd.numWx) {
                     hd.numWx++;
                     hd.wx =
                           (char **) realloc (hd.wx,
                                              hd.numWx * sizeof (char *));
                     hd.wx[m] = (char *)
                           malloc (strlen (match[k].value[pntIndex].str)
                                   + 1);
                     strcpy (hd.wx[m], match[k].value[pntIndex].str);
                  }
               }
            }
         }
         k = collate[j].allElem[NDFD_POP];
         if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
            if (hd.f_valPop) {
               /* In theory should never get here. */
               printf ("\nduplicate pops\n");
               myAssert (1 == 0);
               if (match[k].value[pntIndex].data > hd.pop) {
                  hd.pop = match[k].value[pntIndex].data;
               }
            } else {
               hd.pop = match[k].value[pntIndex].data;
            }
            hd.f_valPop = 1;
         }
         k = collate[j].allElem[NDFD_MAX];
         if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
            if (hd.f_valMaxMin == 1) {
               /* In theory should never get here. */
               printf ("\nduplicate maxs\n");
               myAssert (1 == 0);
               if (match[k].value[pntIndex].data > hd.MaxMin) {
                  hd.MaxMin = match[k].value[pntIndex].data;
               }
            } else {
               hd.MaxMin = match[k].value[pntIndex].data;
            }
            hd.f_valMaxMin = 1;
         }
         k = collate[j].allElem[NDFD_MIN];
         if ((k != -1) && (match[k].value[pntIndex].valueType != 2)) {
            if (hd.f_valMaxMin == 2) {
               /* In theory should never get here. */
               printf ("\nduplicate mins\n");
               myAssert (1 == 0);
               if (match[k].value[pntIndex].data < hd.MaxMin) {
                  hd.MaxMin = match[k].value[pntIndex].data;
               }
            } else {
               hd.MaxMin = match[k].value[pntIndex].data;
            }
            hd.f_valMaxMin = 2;
         }
      }

      valDay = (sInt4) floor ((collate[halfDayIndex[i]].validTime -
                               pntTimeZone * 3600) / SEC_DAY);
      valSec = (sInt4) ((collate[halfDayIndex[i]].validTime -
                         pntTimeZone * 3600) - ((double) valDay) * SEC_DAY);
      if ((((valSec % 86400L) / 3600) >= 18) ||
          (((valSec % 86400L) / 3600) < 6)) {
         /* The 0 is to not check for daylight savings time. */
/*
         Clock_Print2 (timeBuff, 25, collate[halfDayIndex[i]].validTime,
                       "%a", pntTimeZone, f_dayCheck);
         printf ("%s Eve ", timeBuff);
*/
         if (f_MOTD == 3) {
            printf ("   Eve ");
         } else {
            printf ("    Eve ");
         }
         f_night = 1;
      } else {
         /* The 0 is to not check for daylight savings time. */
         Clock_Print2 (timeBuff, 25, collate[halfDayIndex[i]].validTime,
                       "%a", pntTimeZone, f_dayCheck);
         if (f_firstDay) {
            f_firstDay = 0;
         } else {
            if (f_MOTD == 3) {
               printf ("\n");
            }
         }
         if (f_MOTD == 3) {
            printf ("%sDay ", timeBuff);
         } else {
            printf ("%s Day ", timeBuff);
         }
         f_night = 0;
      }

      if (f_MOTD == 3) {
         if (hd.f_valMaxMin == 1) {
            printf ("%.0fH ", hd.MaxMin);
         } else if (hd.f_valMaxMin == 2) {
            printf ("%.0fL ", hd.MaxMin);
         } else {
            printf ("    ");
         }
      } else {
         if (hd.f_valMaxMin == 1) {
            printf ("H=%.0f ", hd.MaxMin);
         } else if (hd.f_valMaxMin == 2) {
            printf ("L=%.0f ", hd.MaxMin);
         } else {
            printf ("     ");
         }
      }

      if (f_MOTD == 2) {
         if (hd.f_valTemp) {
            printf ("T=%02.0f,%02.0f ", hd.minTemp, hd.maxTemp);
         } else {
            printf ("        ");
         }
         if (hd.f_valAppTemp) {
            printf ("AppT=%02.0f,%02.0f ", hd.appMinTemp, hd.appMaxTemp);
         } else {
            printf ("           ");
         }
      }

      if (hd.f_valWindSpd) {
         if (f_MOTD == 3) {
            printf ("%2.0f", hd.windSpd);
         } else {
            printf ("Wnd=%2.0f ", hd.windSpd);
         }
         if (hd.f_valWindDir) {
/*
            printf (" from %3s ", DegToCompass (hd.windDir));
*/
            ptr = DegToCompass (hd.windDir);
            if (strlen (ptr) == 1) {
               printf ("%s   ", ptr);
            } else if (strlen (ptr) == 2) {
               printf ("%s  ", ptr);
            } else {
               printf ("%s ", ptr);
            }
         }
      } else {
         if (f_MOTD == 3) {
            printf ("      ");
         } else {
            printf ("           ");
         }
      }

      /* 100% sky is fully cloudy! */
      if (f_MOTD == 3) {
         if (hd.f_valSky) {
            if (hd.sky < 6) {
               if (!f_night) {
                  printf (" Sunny");
               } else {
                  printf (" Clear");
               }
            } else if (hd.sky < 31) {
               if (!f_night) {
                  printf ("MstSun");
               } else {
                  printf ("MstClr");
               }
            } else if (hd.sky < 69) {
               printf ("PrtCld");
            } else if (hd.sky < 94) {
               printf ("MstCld");
            } else {
               printf ("Cloudy");
            }
         } else {
            printf ("      ");
         }
      } else {
         if (hd.f_valSky) {
            if (hd.sky < 6) {
               if (!f_night) {
                  printf ("    Sunny");
               } else {
                  printf ("    Clear");
               }
            } else if (hd.sky < 31) {
               if (!f_night) {
                  printf (" Most sun");
               } else {
                  printf (" Most clr");
               }
            } else if (hd.sky < 69) {
               printf ("Part cldy");
            } else if (hd.sky < 94) {
               printf ("Most cldy");
            } else {
               printf ("   Cloudy");
            }
         } else {
            printf ("         ");
         }
      }

      if (f_MOTD == 3) {
         if (hd.f_valPop) {
            if (hd.pop > 0) {
               if (hd.f_valSnow) {
                  if (hd.snow != 0) {
                     printf ("%3.0f%%%4.2fS", hd.pop, hd.snow);
                  } else if ((hd.f_valQPF) && (hd.qpf != 0)) {
                     printf ("%3.0f%%%4.2fR", hd.pop, hd.qpf);
                  } else {
                     printf ("%3.0f%% prcp", hd.pop);
                  }
               } else {
                  if ((hd.f_valQPF) && (hd.qpf != 0)) {
                     printf ("%3.0f%%%4.2fR", hd.pop, hd.qpf);
                  } else {
                     printf ("%3.0f%% prcp", hd.pop);
                  }
               }
            }
         }
      } else {
         printf (" ");
         if (hd.f_valPop) {
            if (hd.pop > 0) {
               if (hd.f_valSnow) {
                  if (hd.snow != 0) {
                     printf ("%3.0f%% %4.2f snow", hd.pop, hd.snow);
                  } else if ((hd.f_valQPF) && (hd.qpf != 0)) {
                     printf ("%3.0f%% %4.2f rain", hd.pop, hd.qpf);
                  } else {
                     printf ("%3.0f%% precip", hd.pop);
                  }
               } else {
                  if ((hd.f_valQPF) && (hd.qpf != 0)) {
                     printf ("%3.0f%% %4.2f qpf", hd.pop, hd.qpf);
                  } else {
                     printf ("%3.0f%% precip", hd.pop);
                  }
               }
            }
         }
      }
      if (hd.numWx != 0) {
         for (m = 0; m < hd.numWx; m++) {
            if (f_MOTD == 3) {
               printf ("\n%s", hd.wx[m]);
            } else {
               printf ("\n\t%s", hd.wx[m]);
            }
            free (hd.wx[m]);
         }
         free (hd.wx);
         hd.wx = NULL;
      }

      printf ("\n");
   }

   free (halfDayIndex);
}

int MOTDProbe (uChar f_MOTD, size_t numPnts, Point * pnts,
               PntSectInfo * pntInfo, sChar f_pntType, char **labels,
               size_t numInFiles, char **inFiles, uChar f_fileType,
               sChar f_interp, sChar f_unit, double majEarth,
               double minEarth, sChar f_WxParse, sChar f_SimpleVer, sChar f_SimpleWWA,
               sChar f_valTime, double startTime, double endTime,
               size_t numNdfdVars, uChar *ndfdVars, char *f_inTypes,
               char *gribFilter, size_t numSector, char **sector,
               sChar f_ndfdConven, uChar f_XML, sChar f_avgInterp)
{
   size_t numElem = 0;
   genElemDescript *elem = NULL;
   uChar varFilter[NDFD_MATCHALL + 1];
   size_t numMatch = 0;
   genMatchType *match = NULL;
   size_t i;
   size_t j;
   size_t k;
   collateType *collate = NULL;
   size_t numCollate = 0;
   char *f_pntHasData = NULL;
   double curTime;
   double refTime;

   /* Set up the varFilter array to show what NDFD variables are of
    * interest(1) or vital(2) to this procedure. */
   memset (varFilter, 0, NDFD_MATCHALL + 1);
   varFilter[NDFD_MAX] = 1;
   varFilter[NDFD_MIN] = 1;
   varFilter[NDFD_POP] = 1;
   varFilter[NDFD_SKY] = 1;
   varFilter[NDFD_WD] = 1;
   varFilter[NDFD_WS] = 1;
   varFilter[NDFD_QPF] = 1;
   varFilter[NDFD_SNOW] = 1;
   varFilter[NDFD_WX] = 1;
   if (f_MOTD == 2) {
      varFilter[NDFD_TEMP] = 1;
      varFilter[NDFD_AT] = 1;
   } else if (f_MOTD == 1) {
      varFilter[NDFD_TEMP] = 1;
      varFilter[NDFD_AT] = 1;
      varFilter[NDFD_TD] = 1;
      varFilter[NDFD_WH] = 1;
      varFilter[NDFD_RH] = 1;
   }

   /* Allow user filter of variables to reduce the element list, but include
    * all "vital" (Filter == 2) variables. */
   genElemListInit2 (varFilter, numNdfdVars, ndfdVars, &numElem, &elem);

   /* f_WxParse = 0, is the weather code */
   /* Force english translation instead of weather code */
   f_WxParse = 1;
   if (genProbe (numPnts, pnts, f_pntType, numInFiles, inFiles, f_fileType,
                 f_interp, f_unit, majEarth, minEarth, f_WxParse,
                 f_SimpleVer, f_SimpleWWA, numElem, elem, f_valTime, startTime, endTime,
                 f_XML, &numMatch, &match, f_inTypes, gribFilter, numSector,
                 sector, f_ndfdConven, f_avgInterp) != 0) {
      for (i = 0; i < numElem; i++) {
         genElemFree (elem + i);
      }
      free (elem);
      for (i = 0; i < numMatch; i++) {
         genMatchFree (match + i);
      }
      free (match);
      return -1;
   }
   if (numMatch == 0) {
      goto done;
   }

   /* Sort by valid time. */
   qsort (match, numMatch, sizeof (match[0]), matchCompare2);

#ifdef DEBUG
/*
   for (i = 0; i < numMatch; i++) {
      printf ("%d : %s\n", i, genNdfdEnumToStr (match[i].elem.ndfdEnum, 1));
   }
*/
#endif

   /* allocate f_pntNoData */
   f_pntHasData = (char *) calloc (numPnts, sizeof (char));

   /* colate the matches. */
   curTime = match[0].validTime - 1;
   refTime = match[0].refTime;
   for (i = 0; i < numMatch; i++) {
      if (match[0].refTime < refTime)
         refTime = match[0].refTime;
      if (curTime != match[i].validTime) {
         j = numCollate;
         numCollate++;
         collate =
               (collateType *) realloc (collate,
                                        numCollate * sizeof (collateType));
         for (k = 0; k < NDFD_MATCHALL + 1; k++) {
            collate[numCollate - 1].allElem[k] = -1;
         }
         collate[numCollate - 1].validTime = match[i].validTime;
         curTime = match[i].validTime;
      }
      myAssert (numCollate > 0);
      collate[numCollate - 1].allElem[match[i].elem.ndfdEnum] = i;
      /* update f_pntHasData based on this "match". */
      for (j = 0; j < numPnts; j++) {
         if (match[i].value[j].valueType != 2)
            f_pntHasData[j] = 1;
      }
   }

   /* loop by point. */
   for (j = 0; j < numPnts; j++) {
      /* Check if this point has any data at all */
      if (f_pntHasData[j]) {
         if (f_MOTD == 1) {
            PrintSameDay1 (match, j, collate, numCollate,
                           pntInfo[j].timeZone, pntInfo[j].f_dayLight);
         } else if (f_MOTD == 2) {
            PrintSameDay2 (match, j, collate, numCollate,
                           pntInfo[j].timeZone, pntInfo[j].f_dayLight,
                           refTime, f_MOTD);
         } else if (f_MOTD == 3) {
            PrintSameDay2 (match, j, collate, numCollate,
                           pntInfo[j].timeZone, pntInfo[j].f_dayLight,
                           refTime, f_MOTD);
         }
      }
      printf ("-----------------\n");
   }

 done:
   free (f_pntHasData);
   free (collate);
   for (i = 0; i < numElem; i++) {
      genElemFree (elem + i);
   }
   free (elem);
   for (i = 0; i < numMatch; i++) {
      genMatchFree (match + i);
   }
   free (match);
   return 0;
}

int GraphProbe (uChar f_XML, size_t numPnts, Point * pnts,
                PntSectInfo * pntInfo, sChar f_pntType, char **labels,
                size_t numInFiles, char **inFiles, uChar f_fileType,
                sChar f_interp, sChar f_unit, double majEarth,
                double minEarth, sChar f_WxParse, sChar f_SimpleVer, sChar f_SimpleWWA,
                sChar f_valTime, double startTime, double endTime,
                size_t numNdfdVars, uChar *ndfdVars, char *f_inTypes,
                char *gribFilter, size_t numSector, char **sector,
                sChar f_ndfdConven, sChar f_avgInterp)
{
   size_t numElem = 0;
   genElemDescript *elem = NULL;
   uChar varFilter[NDFD_MATCHALL + 1];
   size_t numMatch = 0;
   genMatchType *match = NULL;
   size_t i;
   size_t j;
   size_t k;
   collateType *collate = NULL;
   size_t numCollate = 0;
   char *f_pntHasData;
   double curTime;

   /* Set up the varFilter array to show what NDFD variables are of
    * interest(1) or vital(2) to this procedure. */
   memset (varFilter, 0, NDFD_MATCHALL + 1);
   varFilter[NDFD_MAX] = 1;
   varFilter[NDFD_MIN] = 1;
   varFilter[NDFD_POP] = 1;
   varFilter[NDFD_TEMP] = 1;
   varFilter[NDFD_WD] = 1;
   varFilter[NDFD_WS] = 1;
   varFilter[NDFD_TD] = 1;
   varFilter[NDFD_SKY] = 1;
   varFilter[NDFD_QPF] = 1;
   varFilter[NDFD_SNOW] = 1;
   varFilter[NDFD_WX] = 1;
   varFilter[NDFD_WH] = 1;
   varFilter[NDFD_AT] = 1;
   varFilter[NDFD_RH] = 1;

   /* Allow user filter of variables to reduce the element list, but include
    * all "vital" (Filter == 2) variables. */
   genElemListInit2 (varFilter, numNdfdVars, ndfdVars, &numElem, &elem);

   /* f_WxParse = 0, is the weather code */
   /* Force english translation instead of weather code */
   f_WxParse = 1;
   if (genProbe (numPnts, pnts, f_pntType, numInFiles, inFiles, f_fileType,
                 f_interp, f_unit, majEarth, minEarth, f_WxParse,
                 f_SimpleVer, f_SimpleWWA, numElem, elem, f_valTime, startTime, endTime,
                 f_XML, &numMatch, &match, f_inTypes, gribFilter, numSector,
                 sector, f_ndfdConven, f_avgInterp) != 0) {
      for (i = 0; i < numElem; i++) {
         genElemFree (elem + i);
      }
      free (elem);
      for (i = 0; i < numMatch; i++) {
         genMatchFree (match + i);
      }
      free (match);
      return -1;
   }

   /* Sort by valid time. */
   qsort (match, numMatch, sizeof (match[0]), matchCompare2);

   /* allocate f_pntNoData */
   f_pntHasData = (char *) calloc (numPnts, sizeof (char));

   /* colate the matches. */
   curTime = -1;
   for (i = 0; i < numMatch; i++) {
      if (curTime != match[i].validTime) {
         j = numCollate;
         numCollate++;
         collate =
               (collateType *) realloc (collate,
                                        numCollate * sizeof (collateType));
         for (k = 0; k < NDFD_MATCHALL + 1; k++) {
            collate[numCollate - 1].allElem[k] = -1;
         }
         collate[numCollate - 1].validTime = match[i].validTime;
         curTime = match[i].validTime;
      }
      myAssert (numCollate > 0);
      collate[numCollate - 1].allElem[match[i].elem.ndfdEnum] = i;
      /* update f_pntHasData based on this "match". */
      for (j = 0; j < numPnts; j++) {
         if (match[i].value[j].valueType != 2)
            f_pntHasData[j] = 1;
      }
   }

   /* loop by point. */
   for (j = 0; j < numPnts; j++) {
      /* Check if this point has any data at all */
      if (f_pntHasData[j]) {
         PrintSameDay1 (match, j, collate, numCollate, pntInfo[j].timeZone,
                        pntInfo[j].f_dayLight);
      }
      printf ("-----------------\n");
   }

   free (f_pntHasData);
   free (collate);
   for (i = 0; i < numElem; i++) {
      genElemFree (elem + i);
   }
   free (elem);
   for (i = 0; i < numMatch; i++) {
      genMatchFree (match + i);
   }
   free (match);
   return 0;
}

#ifdef TEST_XML
int main (int argc, char **argv)
{
   int numInFiles;
   char **inFiles;
   int f_XML = 1;       /* version of xml... 1 = DWML */

   numInFiles = 1;
   inFiles = (char **) malloc (sizeof (char *));
   myAssert (sizeof (char) == 1);
   inFiles[0] = (char *) malloc (strlen ("maxt.bin") + 1);
   strcpy (inFiles[0], "maxt.bint");
/*   XMLParse (f_XML, , , numInFiles, inFiles);*/
}
#endif
