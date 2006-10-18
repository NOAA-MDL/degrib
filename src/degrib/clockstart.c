#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "clock.h"
#include "myutil.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

int main (int argc, char **argv)
{
   int f_timeAdj;       /* 0 no adjust, 1 adjust as LDT, 2 adjust as LST */
   double d_clock;
   double d_amount;
   int i;
   sChar timeZone;
   char f_dayLight;
   char *formatPtr = NULL;
   char ans[200];
   char *buffer = NULL;
   size_t lenBuffer = 0;
   sInt4 year;
   int month;
   int day;
   sInt4 totDay;
   double d_remain;
   char f_isYear;

/* Test routines as if we were in europe */
/*
   putenv ("TZ=CET-1");
   tzset ();
*/

   if (argc < 2) {
      printf ("Usage: %s <clicks/format/scan/seconds/IsDaylightSaving/"
              "IsLeap/add> etc\n", argv[0]);
      return -1;
   }

   /* Handle format option. */
   if (strcmp (argv[1], "format") == 0) {
      if ((argc != 3) && (argc != 5) && (argc != 7) && (argc != 9) &&
          (argc != 11)) {
         printf ("Usage: %s format (<value> or 'stdin') -format <string> "
                 "?-gmt <(0 or 1)>? ?-zone <timeZone>? ?-local <(0 or 1)>?\n",
                 argv[0]);
         return -1;
      }
      strToLower (argv[2]);
      if (strcmp (argv[2], "stdin") == 0) {
         if (reallocFGets (&buffer, &lenBuffer, stdin) == 0) {
            printf ("Expecting stdin to contain one line of data\n");
            printf ("Usage: %s format (<value> or 'stdin') -format <string> "
                    "?-gmt <(0 or 1)>? ?-zone <timeZone>? ?-local <(0 or 1)>"
                    "?\n", argv[0]);
            free (buffer);
            return -1;
         }
/*         strTrim (buffer);*/
         if (!myAtoF (buffer, &d_clock)) {
            printf ("Expecting stdin to contain one double of data\n");
            printf ("Usage: %s format (<value> or 'stdin') -format <string> "
                    "?-gmt <(0 or 1)>? ?-zone <timeZone>? ?-local <(0 or 1)>"
                    "?\n", argv[0]);
            free (buffer);
            return -1;
         }
         free (buffer);
         buffer = NULL;
         lenBuffer = 0;
      } else {
         d_clock = atof (argv[2]);
      }

      f_timeAdj = 1;
      for (i = 3; i < argc; i += 2) {
         if (strcmp (argv[i], "-format") == 0) {
            formatPtr = argv[i + 1];
         } else if (strcmp (argv[i], "-gmt") == 0) {
            if (atoi (argv[i + 1])) {
               f_timeAdj = 0;
            }
         } else if (strcmp (argv[i], "-local") == 0) {
            if (f_timeAdj != 0) {
               if (atoi (argv[i + 1])) {
                  f_timeAdj = 2;
               }
            }
         } else if (strcmp (argv[i], "-zone") == 0) {
            if (Clock_ScanZone2 (argv[i + 1], &timeZone, &f_dayLight) == 0) {
               /* hours to add to local time to get UTC */
               d_clock -= timeZone * 3600.;
               if (f_dayLight == 1)
                  d_clock += 3600.0;
               f_timeAdj = 0;
            }
         }
      }
      if (formatPtr == NULL) {
         printf ("Usage: %s format <value> -format <string> ?-gmt <(0 or"
                 " 1)>? ?-zone <timeZone>? ?-local <(0 or 1)>?\n", argv[0]);
         return -1;
      }
      ans[0] = '\0';
      Clock_Print (ans, 200, d_clock, formatPtr, f_timeAdj);
      printf ("%s\n", ans);

      /* Handle scan option. */
   } else if (strcmp (argv[1], "scan") == 0) {
      if ((argc != 3) && (argc != 5) && (argc != 7) && (argc != 9)) {
         printf ("Usage: %s scan <DateString> ?-gmt <(0 or 1)>? "
                 "?-zone <timeZone>? ?-local <(0 or 1)>?\n", argv[0]);
         return -1;
      }
      d_clock = 0;
      f_timeAdj = 2;
      f_dayLight = 0;
      timeZone = 0;
      for (i = 3; i < argc; i += 2) {
         if (strcmp (argv[i], "-gmt") == 0) {
            if (atoi (argv[i + 1])) {
               f_timeAdj = 0;
            }
         } else if (strcmp (argv[i], "-local") == 0) {
            if (f_timeAdj != 0) {
               if (atoi (argv[i + 1])) {
                  f_timeAdj = 2;
               }
            }
         } else if (strcmp (argv[i], "-zone") == 0) {
            if (Clock_ScanZone2 (argv[i + 1], &timeZone, &f_dayLight) == 0) {
               /* timeZone is number of sec to adjust GMT by. */
               d_clock -= timeZone;
               if (f_dayLight == 1)
                  d_clock += 3600.0;
               f_timeAdj = 0;
            }
         }
      }
      strToLower (argv[2]);
      if (strcmp (argv[2], "stdin") == 0) {
         if (reallocFGets (&buffer, &lenBuffer, stdin) == 0) {
            printf ("Expecting stdin to contain one line of data\n");
            printf ("Usage: %s format (<value> or 'stdin') -format <string> "
                    "?-gmt <(0 or 1)>? ?-zone <timeZone>? ?-local <(0 or 1)>"
                    "?\n", argv[0]);
            free (buffer);
            return -1;
         }
         strTrim (buffer);
         if (Clock_Scan (&d_clock, buffer, f_timeAdj) != 0) {
            fprintf (stderr, "Problems scanning '%s'\n", argv[2]);
            free (buffer);
            return -1;
         }
         free (buffer);
         buffer = NULL;
         lenBuffer = 0;
      } else {
         if (Clock_Scan (&d_clock, argv[2], f_timeAdj) != 0) {
            printf ("Problems scanning '%s'\n", argv[2]);
            return -1;
         }
      }
      /* adjust d_clock based on zone.. */
      /* timeZone is number of sec to adjust GMT by. */
      /* We are going from Local time to GMT so we + timeZone here. */
      d_clock += timeZone;
      if (f_dayLight == 1)
         d_clock -= 3600.0;
      printf ("%f\n", d_clock);
      return 0;

      /* Handle clicks option. */
   } else if (strcmp (argv[1], "clicks") == 0) {
      printf ("%.0f\n", Clock_Clicks ());
      return 0;

      /* Handle seconds option. */
   } else if (strcmp (argv[1], "seconds") == 0) {
      if ((argc != 2) && (argc != 4) && (argc != 6) && (argc != 8)) {
         printf ("Usage: %s seconds ?-gmt <(0 or 1)>? "
                 "?-zone <timeZone>? ?-local <(0 or 1)>?\n", argv[0]);
         return -1;
      }
      d_clock = Clock_Seconds ();
      f_timeAdj = 2;
      f_dayLight = 0;
      timeZone = 0;
      for (i = 2; i < argc; i += 2) {
         if (strcmp (argv[i], "-gmt") == 0) {
            if (atoi (argv[i + 1])) {
               f_timeAdj = 0;
            }
         } else if (strcmp (argv[i], "-local") == 0) {
            if (f_timeAdj != 0) {
               if (atoi (argv[i + 1])) {
                  f_timeAdj = 2;
               }
            }
         } else if (strcmp (argv[i], "-zone") == 0) {
            if (Clock_ScanZone2 (argv[i + 1], &timeZone, &f_dayLight) == 0) {
               /* timeZone is number of hours to add to local time to get UTC 
                */
               f_timeAdj = 1;
            }
         }
      }
      if ((f_timeAdj == 0) || (f_timeAdj == 1)) {
         if (Clock_IsDaylightSaving2 (d_clock, Clock_GetTimeZone ())) {
            d_clock = d_clock - (Clock_GetTimeZone () - 1) * 3600;
         } else {
            d_clock = d_clock - Clock_GetTimeZone () * 3600;
         }
      }
      if (f_timeAdj == 1) {
         d_clock += timeZone * 3600.;
         if (f_dayLight) {
            d_clock -= 3600;
         }
      }
      printf ("%f\n", d_clock);
      return 0;

      /* Handle Daylight option. */
   } else if (strcmp (argv[1], "IsDaylightSaving") == 0) {
      if (argc != 5) {
         printf ("Usage: %s IsDaylightSaving (<value> or 'stdin') "
                 "-inZone <zone>\n", argv[0]);
         return -1;
      }
      strToLower (argv[2]);
      if (strcmp (argv[2], "stdin") == 0) {
         if (reallocFGets (&buffer, &lenBuffer, stdin) == 0) {
            printf ("Expecting stdin to contain one line of data\n");
            printf ("Usage: %s format (<value> or 'stdin') -format <string> "
                    "?-gmt <(0 or 1)>? ?-zone <timeZone>? ?-local <(0 or"
                    " 1)>?\n", argv[0]);
            free (buffer);
            return -1;
         }
/*         strTrim (buffer); */
         if (!myAtoF (buffer, &d_clock)) {
            printf ("Expecting stdin to contain one double of data\n");
            printf ("Usage: %s format (<value> or 'stdin') -format <string> "
                    "?-gmt <(0 or 1)>? ?-zone <timeZone>? ?-local <(0 or"
                    " 1)>?\n", argv[0]);
            free (buffer);
            return -1;
         }
         free (buffer);
         buffer = NULL;
         lenBuffer = 0;
      } else {
         d_clock = atof (argv[2]);
      }
      if (Clock_ScanZone2 (argv[4], &timeZone, &f_dayLight) == 0) {
         /* timeZone is number of hours to add to local time to get UTC */
         d_clock -= timeZone * 3600.;
         if (f_dayLight == 1)
            d_clock += 3600.0;
         f_timeAdj = 0;
      }
      if (Clock_IsDaylightSaving2 (d_clock, timeZone)) {
         printf ("1\n");
      } else {
         printf ("0\n");
      }

   } else if (strcmp (argv[1], "IsLeap") == 0) {
      if (argc != 5) {
         printf ("Usage: %s IsLeap <value or 'stdin' || year> "
                 "-inputIsYear <(0 or 1)>\n", argv[0]);
         return -1;
      }
      f_isYear = 0;
      for (i = 3; i < argc; i += 2) {
         if (strcmp (argv[i], "-inputIsYear") == 0) {
            if (atoi (argv[i + 1])) {
               f_isYear = 1;
            }
         }
      }
      strToLower (argv[2]);
      if (strcmp (argv[2], "stdin") == 0) {
         if (reallocFGets (&buffer, &lenBuffer, stdin) == 0) {
            printf ("Expecting stdin to contain one line of data\n");
            printf ("Usage: %s format (<value> or 'stdin') -format <string> "
                    "?-gmt <(0 or 1)>? ?-zone <timeZone>? ?-local <(0 or 1)>"
                    "?\n", argv[0]);
            free (buffer);
            return -1;
         }
/*         strTrim (buffer); */
         if (!myAtoF (buffer, &d_clock)) {
            printf ("Expecting stdin to contain one double of data\n");
            printf ("Usage: %s format (<value> or 'stdin') -format <string> "
                    "?-gmt <(0 or 1)>? ?-zone <timeZone>? ?-local <(0 or 1)>"
                    "?\n", argv[0]);
            free (buffer);
            return -1;
         }
         if (f_isYear) {
            year = d_clock;
         }
         free (buffer);
         buffer = NULL;
         lenBuffer = 0;
      } else {
         if (f_isYear) {
            year = atof (argv[2]);
         } else {
            d_clock = atof (argv[2]);
         }
      }
      if (!f_isYear) {
         totDay = (sInt4) floor (d_clock / SEC_DAY);
         Clock_Epoch2YearDay (totDay, &day, &year);
      }
      if (!ISLEAPYEAR (year)) {
         printf ("0\n");
      } else {
         printf ("1\n");
      }

   } else if (strcmp (argv[1], "add") == 0) {
      if (argc != 5) {
         printf ("Usage: %s add (<time> or 'stdin') <amount> "
                 "<sec/min/hour/day/month/year>\n", argv[0]);
         return -1;
      }
      if (!myAtoF (argv[3], &d_amount)) {
         printf ("Usage: %s add (<time> or 'stdin') <amount> "
                 "<sec/min/hour/day/month/year>\n", argv[0]);
         return -1;
      }
      strToLower (argv[2]);
      if (strcmp (argv[2], "stdin") == 0) {
         if (reallocFGets (&buffer, &lenBuffer, stdin) == 0) {
            printf ("Expecting stdin to contain one line of data\n");
            printf ("Usage: %s format (<value> or 'stdin') "
                    "-format <string> ?-gmt <(0 or 1)>? "
                    "?-zone <timeZone>? ?-local <(0 or 1)>?\n", argv[0]);
            free (buffer);
            return -1;
         }
/*         strTrim (buffer); */
         if (!myAtoF (buffer, &d_clock)) {
            printf ("Expecting stdin to contain one double of data\n");
            printf ("Usage: %s format (<value> or 'stdin') "
                    "-format <string> ?-gmt <(0 or 1)>? "
                    "?-zone <timeZone>? ?-local <(0 or 1)>?\n", argv[0]);
            free (buffer);
            return -1;
         }
         free (buffer);
         buffer = NULL;
         lenBuffer = 0;
      } else {
         d_clock = atof (argv[2]);
      }
      strToLower (argv[4]);
      if (strcmp (argv[4], "sec") == 0) {
         d_clock = d_clock + d_amount;
      } else if (strcmp (argv[4], "min") == 0) {
         d_clock = d_clock + d_amount * 60.;
      } else if (strcmp (argv[4], "hour") == 0) {
         d_clock = d_clock + d_amount * 3600.;
      } else if (strcmp (argv[4], "day") == 0) {
         d_clock = d_clock + d_amount * 3600. * 24.;
      } else {
         totDay = (sInt4) floor (d_clock / SEC_DAY);
         d_remain = d_clock - totDay * 3600 * 24.0;
         Clock_Epoch2YearDay (totDay, &day, &year);
         month = Clock_MonthNum (day, year);
         day = day - Clock_NumDay (month, 1, year, 1) + 1;
         if (strcmp (argv[4], "month") == 0) {
            month = month + floor (d_amount + .5);
            while (month > 12) {
               year++;
               month -= 12;
            }
            while (month < 1) {
               year--;
               month += 12;
            }
         } else if (strcmp (argv[4], "year") == 0) {
            year = year + floor (d_amount + .5);
         } else {
            printf ("Usage: %s add (<time> or 'stdin') <amount> "
                    "<sec/min/hour/day/month/year>\n", argv[0]);
            return -1;
         }
         i = Clock_NumDay (month, 1, year, 0);
         if (day > i)
            day = i;
         d_clock = 0;
         Clock_ScanDate (&d_clock, year, month, day);
         d_clock = d_clock + d_remain;
      }
      printf ("%f\n", d_clock);
      return 0;

   } else {
      printf ("Usage: %s <clicks/format/scan/seconds/IsDaylightSaving/"
              "IsLeap/add> etc\n", argv[0]);
      return -1;
   }
   return 0;
}
