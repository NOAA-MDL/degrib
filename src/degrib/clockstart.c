#define PROGRAM_VERSION "1.12"
#define PROGRAM_DATE "2014-08-19"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "clock.h"
#include "myutil.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

enum { CMD_HELP, CMD_VERSION, CMD_FORMAT, CMD_SCAN, CMD_CLICKS, CMD_SECONDS,
       CMD_ISDAYLIGHT, CMD_ISLEAP, CMD_ADD };

static char *UsrOpt[] = {
   "-help", "-V", "format", "scan", "clicks", "seconds", "IsDaylightSaving",
   "IsLeap", "add",
   "-format", "-gmt", "-local", "-zone", "-inputIsYear", "-Newline", NULL
};

typedef struct {
   signed char cmd;
   char f_isYear;
   double d_clock;
   char *str;
   char *fmtArgv;   /* Allocated by system. */
   char f_timeAdj;  /* 0 no adjust, 1 adjust as LDT, 2 adjust as LST
                     * f_timeAdj default is 1, init is -1. */
   sChar timeZone;  /* used to help with the -zone option */
   char f_dayLight; /* Also used to help with the -zone option */
   double amnt;     /* Amount to add */
   char amntUnit;   /* 0 = seconds, 1 = month, 2 = year */
   char f_Newline;  /* 1 print newline, 0 don't */
} userType;

void Usage (char *argv0, userType *usr)
{
   static char *UsrDes[] = {
      "(1 arg) usage or help command",
      "(1 arg) version command",
      "(2 arg) Print time string, given # secs since 1/1/1970 or stdin",
      "(2 arg) Print secs since 1/1/1970, given 'date string' or stdin",
      "(1 arg) Print the # of clicks since program started",
      "(1 arg) Print the current secs since 1/1/1970",
      "(2 arg) 1 if it is daylight savings, 0 if it isn't\n"
            "\t\tgiven <# secs since 1/1/1970, or stdin>",
      "(2 arg) 1 if it is a leap year, 0 if it isn't\n"
            "\t\tgiven <# secs since 1/1/1970, stdin, or year>",
      "(4 arg) The sum of adding X units to # secs since 1/1/1970.\n"
            "\t\tgiven <# secs since 1/1/1970, or stdin>\n"
            "\t\t<X amount> <units (sec/min/hour/day/month/year)>\n",
      "(2 arg) The format string to use",
      "(2 arg) 1=Do things using zone GMT, 0=Don't",
      "(2 arg) (-gmt overrides) 1=Use only standard time,\n"
            "\t\t0=Use mixed standard/daylight time",
      "(2 arg) TimeZone to use",
      "(1 arg) IsLeap input is a year rather than secs since 1/1/1970\n",
      "(2 arg) 1=print newLine, 0=don't print newLine",
      NULL
   };
   int i, j;
   char buffer[21];
   int blanks = 15;

   printf ("Usage: %s [OPTION]...\n", argv0);
   printf ("\nOptions:\n");
   for (i = 0; i < sizeof (UsrOpt) / sizeof (UsrOpt[0]) - 1; ++i) {
      if (strlen (UsrOpt[i]) <= blanks) {
         for (j = 0; j < blanks; ++j) {
            if (j < strlen (UsrOpt[i])) {
               buffer[j] = UsrOpt[i][j];
            } else {
               buffer[j] = ' ';
            }
         }
         buffer[blanks] = '\0';
         printf ("%s %s\n", buffer, UsrDes[i]);
      } else {
         printf ("%s %s\n", UsrOpt[i], UsrDes[i]);
      }
   }
}

static void UserInit (userType *usr)
{
   usr->cmd = CMD_HELP; /* Default to the help command. */
   usr->f_isYear = 0;
   usr->d_clock = 0;
   usr->str = NULL;
   usr->fmtArgv = NULL;
   usr->f_timeAdj = -1;
   usr->timeZone = 0;
   usr->f_dayLight = 0;
   usr->amnt = 0;
   usr->amntUnit = 0;
   usr->f_Newline = 1;
}

static void UserFree (userType *usr)
{
   if (usr->str != NULL) {
      free (usr->str);
   }
}

static int ParseUserChoice (userType *usr, char *cur, char *n1, char *n2,
                            char *n3)
{
   enum { HELP, VERSION, FORMAT, SCAN, CLICKS, SECONDS, ISDAYLIGHT, ISLEAP,
      ADD, FORMAT_OPTION, GMT, LOCAL, ZONE, INPUTISYEAR, NEWLINE
   };
   int index;           /* "cur"'s index into Opt, which matches enum val. */
   long int li_temp;
   double d_temp;
   char *buffer = NULL;
   size_t lenBuffer = 0;
   sChar timeZone;
   char f_dayLight;

   /* Figure out which option. */
   if (GetIndexFromStr (cur, UsrOpt, &index) < 0) {
      printf ("Invalid option '%s'\n", cur);
      return -1;
   }
   /* Handle the 1 argument options first. */
   switch (index) {
      case HELP:
         usr->cmd = CMD_HELP;
         return 1;
      case VERSION:
         usr->cmd = CMD_VERSION;
         return 1;
      case CLICKS:
         usr->cmd = CMD_CLICKS;
         return 1;
      case SECONDS:
         usr->cmd = CMD_SECONDS;
         return 1;
      case INPUTISYEAR:
         usr->f_isYear = 1;
         return 1;
   }
   /* It is definitely a 2 or more argument option, check if n1 is NULL. */
   if (n1 == NULL) {
      printf ("%s needs another argument\n", cur);
      return -1;
   }
   /* Handle the 2 argument options. */
   switch (index) {
      /* strToLower (argv[2]); */
      case FORMAT:
      case ISDAYLIGHT:
      case ISLEAP:
         strToLower (n1);
         if (strcmp (n1, "stdin") == 0) {
            if (reallocFGets (&buffer, &lenBuffer, stdin) == 0) {
               printf ("Expecting stdin to contain one line of data\n");
               free (buffer);
               return -1;
            }
/*            strTrim (buffer); */
            if (!myAtoF (buffer, &d_temp)) {
               return -1;
            }
            free (buffer);
            usr->d_clock = d_temp;
         } else if (!myAtoF (n1, &d_temp)) {
            return -1;
         }
         usr->d_clock = d_temp;
         if (index == FORMAT) {
            usr->cmd = CMD_FORMAT;
         } else if (index == ISDAYLIGHT) {
            usr->cmd = CMD_ISDAYLIGHT;
         } else {
            usr->cmd = CMD_ISLEAP;
         }
         return 2;
      case SCAN:
         strToLower (n1);
         if (strcmp (n1, "stdin") == 0) {
            if (reallocFGets (&buffer, &lenBuffer, stdin) == 0) {
               printf ("Expecting stdin to contain one line of data\n");
               free (buffer);
               return -1;
            }
            usr->str = buffer;
         } else {
            usr->str = (char *) malloc ((strlen (n1) + 1) * sizeof (char));
            strcpy (usr->str, n1);
         }
         strTrim (usr->str);
         usr->cmd = CMD_SCAN;
         return 2;
      case FORMAT_OPTION:
         usr->fmtArgv = n1;
         return 2;
      case GMT:
         if (myAtoI (n1, &(li_temp)) != 1) {
            printf ("Bad value to '%s' of '%s'\n", cur, n1);
            return -1;
         }
         if (li_temp != 0) {
            /* true -gmt option */
            usr->f_timeAdj = 0;
         } else {
            /* false -gmt option */
            usr->f_timeAdj = 1;
         }
         return 2;
      case NEWLINE:
         if (myAtoI (n1, &(li_temp)) != 1) {
            printf ("Bad value to '%s' of '%s'\n", cur, n1);
            return -1;
         }
         usr->f_Newline = li_temp;
         return 2; 
      case LOCAL:
         if (myAtoI (n1, &(li_temp)) != 1) {
            printf ("Bad value to '%s' of '%s'\n", cur, n1);
            return -1;
         }
         /* -gmt option has precedence */
         if (usr->f_timeAdj == -1) {
            if (li_temp != 0) {
               /* true -local option */
               usr->f_timeAdj = 2;
            }
         }
         return 2;
      case ZONE:
         if (Clock_ScanZone2 (n1, &timeZone, &f_dayLight) != 0) {
            printf ("Bad value to '%s' of '%s'\n", cur, n1);
            return -1;
         }
         usr->f_dayLight = f_dayLight;
         usr->timeZone = timeZone;
         return 2;
   }
   /* It is definitely a 3 or more argument option, check if n2 is NULL. */
   if (n2 == NULL) {
      printf ("%s needs another argument\n", cur);
      return -1;
   }
   /* It is definitely a 4 or more argument option, check if n3 is NULL. */
   if (n3 == NULL) {
      printf ("%s needs another argument\n", cur);
      return -1;
   }
   /* Handle the 4 argument options. */
   switch (index) {
      case ADD:
         strToLower (n1);
         if (strcmp (n1, "stdin") == 0) {
            if (reallocFGets (&buffer, &lenBuffer, stdin) == 0) {
               printf ("Expecting stdin to contain one line of data\n");
               free (buffer);
               return -1;
            }
/*            strTrim (buffer); */
            if (!myAtoF (buffer, &d_temp)) {
               return -1;
            }
            free (buffer);
            usr->d_clock = d_temp;
         } else if (!myAtoF (n1, &d_temp)) {
            return -1;
         }
         usr->d_clock = d_temp;
         if (!myAtoF (n2, &d_temp)) {
            return -1;
         }
         strToLower (n3);
         usr->amntUnit = 0;
         if (strcmp (n3, "sec") == 0) {
         } else if (strcmp (n3, "min") == 0) {
            d_temp *= 60.;
         } else if (strcmp (n3, "hour") == 0) {
            d_temp *= 3600.;
         } else if (strcmp (n3, "day") == 0) {
            d_temp *= 3600. * 24.;
         } else if (strcmp (n3, "month") == 0) {
            usr->amntUnit = 1;
         } else if (strcmp (n3, "year") == 0) {
            usr->amntUnit = 2;
         } else {
            printf ("Expecting a unit of <sec/min/hour/day/month/year>\n");
            return -1;
         }
         usr->amnt = d_temp;
         usr->cmd = CMD_ADD;
         return 4;
      default:
         printf ("Invalid option '%s'\n", cur);
         return -1;
   }
}

int ParseCmdLine (userType *usr, int myArgc, char **myArgv)
{
   int ans;             /* The returned value from ParseUserChoice */

   while (myArgc > 0) {
      if (myArgc == 1) {
         ans = ParseUserChoice (usr, *myArgv, NULL, NULL, NULL);
         if (ans > 1) {
            printf ("Option '%s' requires %d part(s)\n", *myArgv, ans);
            return -1;
         }
      } else if (myArgc == 2) {
         ans = ParseUserChoice (usr, *myArgv, myArgv[1], NULL, NULL);
         if (ans > 2) {
            printf ("Option '%s' requires %d parts\n", *myArgv, ans);
            return -1;
         }
      } else if (myArgc == 3) {
         ans = ParseUserChoice (usr, *myArgv, myArgv[1], myArgv[2], NULL);
         if (ans > 3) {
            printf ("Option '%s' requires %d parts\n", *myArgv, ans);
            return -1;
         }
      } else {
         ans = ParseUserChoice (usr, *myArgv, myArgv[1], myArgv[2],
                                myArgv[3]);
      }
      if (ans == -1) {
         return -1;
      }
      myArgc -= ans;
      myArgv += ans;
   }
   return 0;
}

int main (int argc, char **argv)
{
   userType usr;
   char buffer[1000];
   sInt4 year;
   int month;
   int day;
   sInt4 totDay;
   double d_remain;
   int i;

   UserInit (&usr);
   if (ParseCmdLine (&usr, argc - 1, argv + 1) != 0) {
      Usage (argv[0], &usr);
      UserFree (&usr);
      return 1;
   }
   switch (usr.cmd) {
      case CMD_HELP:
         Usage (argv[0], &usr);
         UserFree (&usr);
         return 0;
      case CMD_VERSION:
         printf ("%s\nVersion: %s\nDate: %s\nAuthor: Arthur Taylor\n",
                 argv[0], PROGRAM_VERSION, PROGRAM_DATE);
         UserFree (&usr);
         return 0;
      case CMD_FORMAT:
         usr.d_clock -= usr.timeZone * 3600.;
         if (usr.f_dayLight == 1) {
            usr.d_clock += 3600.;
         }
         if (usr.fmtArgv == NULL) {
            printf ("format option requires a format string\n");
            UserFree (&usr);
            return -1;
         }
         buffer[0] = '\0';
         Clock_Print (buffer, 1000, usr.d_clock, usr.fmtArgv, usr.f_timeAdj);
         if (usr.f_Newline) { 
            printf ("%s\n", buffer);
         } else {
            printf ("%s", buffer);
         }
         UserFree (&usr);
         return 0;
      case CMD_ISDAYLIGHT:
/*
         usr.d_clock -= usr.timeZone * 3600.;
         if (usr.f_dayLight == 1) {
            usr.d_clock += 3600.;
         }
*/
         if (Clock_IsDaylightSaving2 (usr.d_clock, usr.timeZone)) {
            printf ("1\n");
         } else {
            printf ("0\n");
         }
         UserFree (&usr);
         return 0;
      case CMD_ISLEAP:
         if (usr.f_isYear) {
            year = usr.d_clock;
         } else {
            usr.d_clock -= usr.timeZone * 3600.;
            if (usr.f_dayLight == 1) {
               usr.d_clock += 3600.;
            }
            totDay = (sInt4) floor (usr.d_clock / SEC_DAY);
            Clock_Epoch2YearDay (totDay, &day, &year);
         }
         if (!ISLEAPYEAR (year)) {
            printf ("0\n");
         } else {
            printf ("1\n");
         }
         UserFree (&usr);
         return 0;
      case CMD_SCAN:
         if (Clock_Scan (&usr.d_clock, usr.str, usr.f_timeAdj) != 0) {
            printf ("Problems scanning '%s'\n", usr.str);
            UserFree (&usr);
            return -1;
         }
         /* adjust d_clock based on zone.. */
         /* timeZone is number of sec to adjust GMT by. */
         /* We are going from Local time to GMT so we + timeZone here. */
         usr.d_clock += usr.timeZone * 3600.;
         if (usr.f_dayLight == 1) {
            usr.d_clock -= 3600.0;
         }
         printf ("%f\n", usr.d_clock);
         UserFree (&usr);
         return 0;
      case CMD_CLICKS:
         printf ("%.0f\n", Clock_Clicks ());
         UserFree (&usr);
         return 0;
      case CMD_SECONDS:
         usr.d_clock = Clock_Seconds ();
         if ((usr.f_timeAdj == 0) || (usr.f_timeAdj == 1)) {
            if (Clock_IsDaylightSaving2 (usr.d_clock, Clock_GetTimeZone ())) {
               usr.d_clock = usr.d_clock - (Clock_GetTimeZone () - 1) * 3600;
            } else {
               usr.d_clock = usr.d_clock - Clock_GetTimeZone () * 3600;
            }
         }
         if (usr.f_timeAdj == 1) {
            usr.d_clock += usr.timeZone * 3600.;
            if (usr.f_dayLight) {
               usr.d_clock -= 3600;
            }
         }
         printf ("%f\n", usr.d_clock);
         UserFree (&usr);
         return 0;
      case CMD_ADD:
         usr.d_clock -= usr.timeZone * 3600.;
         if (usr.f_dayLight == 1) {
            usr.d_clock += 3600.;
         }
         if (usr.amntUnit == 0) {
            /* Seconds */
            usr.d_clock += usr.amnt;
         } else {
            /* Decompose the d_clock into day, month, year, remainder */
            totDay = (sInt4) floor (usr.d_clock / SEC_DAY);
            Clock_Epoch2YearDay (totDay, &day, &year);
            month = Clock_MonthNum (day, year);
            day = day - Clock_NumDay (month, 1, year, 1) + 1;
            d_remain = usr.d_clock - totDay * 3600 * 24.0;
            /* Add the month */
            if (usr.amntUnit == 1) {
               month += usr.amnt;
               while (month > 12) {
                  year++;
                  month -= 12;
               }
               while (month < 1) {
                  year--;
                  month += 12;
               }
            } else {
            /* Add the year.  (usr.amntUnit == 2) */
               year += usr.amnt;
            }
            /* Recompose the date */
            i = Clock_NumDay (month, 1, year, 0);
            if (day > i)
               day = i;
            usr.d_clock = 0;
            Clock_ScanDate (&usr.d_clock, year, month, day);
            usr.d_clock += d_remain;
         }
         printf ("%f\n", usr.d_clock);
         UserFree (&usr);
         return 0;
   }
   printf ("Un-handled command option\n");
   UserFree (&usr);
   return 0;
}

int main_old (int argc, char **argv)
{
   int f_timeAdj;       /* 0 no adjust, 1 adjust as LDT, 2 adjust as LST */   double d_clock;
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
