#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "libaat.h"

#define PERIOD_YEARS 146097L
#define SEC_DAY 86400L
#define ISLEAPYEAR(y) (((y)%400 == 0) || (((y)%4 == 0) && ((y)%100 != 0)))

static void Clock_Epoch2YearDay(sInt4 totDay, int *Day, sInt4 *Yr)
{
   sInt4 year;          /* Local copy of the year. */

   year = 1970;
   /* Jump to the correct 400 year period of time. */
   if ((totDay <= -PERIOD_YEARS) || (totDay >= PERIOD_YEARS)) {
      year += 400 * (totDay / PERIOD_YEARS);
      totDay -= PERIOD_YEARS * (totDay / PERIOD_YEARS);
   }
   if (totDay >= 0) {
      while (totDay >= 366) {
         if (ISLEAPYEAR(year)) {
            if (totDay >= 1461) {
               year += 4;
               totDay -= 1461;
            } else if (totDay >= 1096) {
               year += 3;
               totDay -= 1096;
            } else if (totDay >= 731) {
               year += 2;
               totDay -= 731;
            } else {
               year++;
               totDay -= 366;
            }
         } else {
            year++;
            totDay -= 365;
         }
      }
      if ((totDay == 365) && (!ISLEAPYEAR(year))) {
         year++;
         totDay -= 365;
      }
   } else {
      while (totDay <= -366) {
         year--;
         if (ISLEAPYEAR(year)) {
            if (totDay <= -1461) {
               year -= 3;
               totDay += 1461;
            } else if (totDay <= -1096) {
               year -= 2;
               totDay += 1096;
            } else if (totDay <= -731) {
               year--;
               totDay += 731;
            } else {
               totDay += 366;
            }
         } else {
            totDay += 365;
         }
      }
      if (totDay < 0) {
         year--;
         if (ISLEAPYEAR(year)) {
            totDay += 366;
         } else {
            totDay += 365;
         }
      }
   }
   *Day = (int)totDay;
   *Yr = year;
}

static int Clock_MonthNum(int day, sInt4 year)
{
   if (day < 31)
      return 1;
   if (ISLEAPYEAR(year))
      day -= 1;
   if (day < 59)
      return 2;
   if (day <= 89)
      return 3;
   if (day == 242)
      return 8;
   return ((day + 64) * 5) / 153 - 1;
}

static int Clock_NumDay(int month, int day, sInt4 year, char f_tot)
{
   if (f_tot == 1) {
      if (month > 2) {
         if (ISLEAPYEAR(year)) {
            return ((month + 1) * 153) / 5 - 63 + day;
         } else {
            return ((month + 1) * 153) / 5 - 64 + day;
         }
      } else {
         return (month - 1) * 31 + day - 1;
      }
   } else {
      if (month == 1) {
         return 31;
      } else if (month != 2) {
         if ((((month - 3) % 5) % 2) == 1) {
            return 30;
         } else {
            return 31;
         }
      } else {
         if (ISLEAPYEAR(year)) {
            return 29;
         } else {
            return 28;
         }
      }
   }
}

void Clock_PrintDate(double clock, sInt4 *year, int *month, int *day,
                     int *hour, int *min, double *sec)
{
   sInt4 totDay;
   sInt4 intSec;

   totDay = (sInt4)floor(clock / SEC_DAY);
   Clock_Epoch2YearDay(totDay, day, year);
   *month = Clock_MonthNum(*day, *year);
   *day = *day - Clock_NumDay(*month, 1, *year, 1) + 1;
   *sec = clock - ((double)totDay) * SEC_DAY;
   intSec = (sInt4)(*sec);
   *hour = (int)((intSec % 86400L) / 3600);
   *min = (int)((intSec % 3600) / 60);
   *sec = (intSec % 60) + (*sec - intSec);
}

void Clock_ScanDate(double *clock, sInt4 year, int mon, int day)
{
   int i;
   sInt4 delt, temp, totDay;

   myAssert((mon >= 1) && (mon <= 12));

   /* Makes sure clock is zero'ed out. */
   *clock = 0;

   if ((mon < 1) || (mon > 12) || (day < 0) || (day > 31))
      return;
   totDay = Clock_NumDay(mon, day, year, 0);
   if (day > totDay)
      return;
   totDay = Clock_NumDay(mon, day, year, 1);
   temp = 1970;
   delt = year - temp;
   if ((delt >= 400) || (delt <= -400)) {
      i = (delt / 400);
      temp += 400 * i;
      totDay += 146097L * i;
   }
   if (temp < year) {
      while (temp < year) {
         if (((temp % 4) == 0) &&
             (((temp % 100) != 0) || ((temp % 400) == 0))) {
            if ((temp + 4) < year) {
               totDay += 1461;
               temp += 4;
            } else if ((temp + 3) < year) {
               totDay += 1096;
               temp += 3;
            } else if ((temp + 2) < year) {
               totDay += 731;
               temp += 2;
            } else {
               totDay += 366;
               temp++;
            }
         } else {
            totDay += 365;
            temp++;
         }
      }
   } else if (temp > year) {
      while (temp > year) {
         temp--;
         if (((temp % 4) == 0) &&
             (((temp % 100) != 0) || ((temp % 400) == 0))) {
            if (year < temp - 3) {
               totDay -= 1461;
               temp -= 3;
            } else if (year < (temp - 2)) {
               totDay -= 1096;
               temp -= 2;
            } else if (year < (temp - 1)) {
               totDay -= 731;
               temp--;
            } else {
               totDay -= 366;
            }
         } else {
            totDay -= 365;
         }
      }
   }
   *clock = *clock + ((double)(totDay)) * 24 * 3600;
}

int Clock_PrintBuffDate(double clock, char *buffer, size_t len)
{
   sInt4 year;
   int month;
   int day;
   int hour;
   int min;
   double sec;

   Clock_PrintDate(clock, &year, &month, &day, &hour, &min, &sec);
   if (len >= 14) {
      sprintf(buffer, "%04ld%02d%02d%02d%02d%02.0f", year, month, day, hour,
              min, sec);
   } else if (len == 12) {
      sprintf(buffer, "%04ld%02d%02d%02d%02d", year, month, day, hour, min);
   } else if (len == 10) {
      sprintf(buffer, "%04ld%02d%02d%02d", year, month, day, hour);
   } else if (len == 8) {
      sprintf(buffer, "%04ld%02d%02d", year, month, day);
   } else if (len == 6) {
      sprintf(buffer, "%04ld%02d", year, month);
   } else if (len == 4) {
      sprintf(buffer, "%04ld", year);
   } else {
      myWarn_Err2Arg("Invalid date buffer length '%d'\n", len);
      return -1;
   }
   return 0;
}

int Clock_ScanBuffDate(double *clock, char *buffer)
{
   int buffLen = strlen(buffer);
   sInt4 year;
   int mon = 1;
   int day = 1;
   int hour = 0;
   int min = 0;
   int sec = 0;
   char c_temp;

   *clock = 0;
   if ((buffLen != 4) && (buffLen != 6) && (buffLen != 8) &&
       (buffLen != 10) && (buffLen != 12) && (buffLen != 14)) {
      return 1;
   }
   c_temp = buffer[4];
   buffer[4] = '\0';
   year = atoi(buffer);
   buffer[4] = c_temp;
   if (buffLen > 4) {
      c_temp = buffer[6];
      buffer[6] = '\0';
      mon = atoi(buffer + 4);
      buffer[6] = c_temp;
   }
   if (buffLen > 6) {
      c_temp = buffer[8];
      buffer[8] = '\0';
      day = atoi(buffer + 6);
      buffer[8] = c_temp;
   }
   if (buffLen > 8) {
      c_temp = buffer[10];
      buffer[10] = '\0';
      hour = atoi(buffer + 8);
      buffer[10] = c_temp;
   }
   if (buffLen > 10) {
      c_temp = buffer[12];
      buffer[12] = '\0';
      min = atoi(buffer + 10);
      buffer[12] = c_temp;
   }
   if (buffLen > 12) {
      c_temp = buffer[14];
      buffer[14] = '\0';
      sec = atoi(buffer + 12);
      buffer[14] = c_temp;
   }
   Clock_ScanDate(clock, year, mon, day);
   *clock = *clock + sec + min * 60 + hour * 3600;
   return 0;
}

/*****************************************************************************
 * Clock_ScanMonth() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Scans a string looking for a month word.  Assumes string is all caps.
 *
 * ARGUMENTS
 * ptr = The character string to scan. (Input)
 *
 * RETURNS: int
 * Returns the month number read, or -1 if no month word seen.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *   6/2004 AAT (MDL): Updated.
 *
 * NOTES
 *****************************************************************************
 */
int Clock_ScanMonth(char *ptr)
{
   switch (*ptr) {
      case 'A':
         if ((strcmp(ptr, "APR") == 0) || (strcmp(ptr, "APRIL") == 0))
            return 4;
         else if ((strcmp(ptr, "AUG") == 0) || (strcmp(ptr, "AUGUST") == 0))
            return 8;
         return -1;
      case 'D':
         if ((strcmp(ptr, "DEC") == 0) || (strcmp(ptr, "DECEMBER") == 0))
            return 12;
         return -1;
      case 'F':
         if ((strcmp(ptr, "FEB") == 0) || (strcmp(ptr, "FEBRUARY") == 0))
            return 2;
         return -1;
      case 'J':
         if ((strcmp(ptr, "JAN") == 0) || (strcmp(ptr, "JANUARY") == 0))
            return 1;
         else if ((strcmp(ptr, "JUN") == 0) || (strcmp(ptr, "JUNE") == 0))
            return 6;
         else if ((strcmp(ptr, "JUL") == 0) || (strcmp(ptr, "JULY") == 0))
            return 7;
         return -1;
      case 'M':
         if ((strcmp(ptr, "MAR") == 0) || (strcmp(ptr, "MARCH") == 0))
            return 3;
         else if (strcmp(ptr, "MAY") == 0)
            return 5;
         return -1;
      case 'N':
         if ((strcmp(ptr, "NOV") == 0) || (strcmp(ptr, "NOVEMBER") == 0))
            return 11;
         return -1;
      case 'O':
         if ((strcmp(ptr, "OCT") == 0) || (strcmp(ptr, "OCTOBER") == 0))
            return 10;
         return -1;
      case 'S':
         if ((strcmp(ptr, "SEP") == 0) || (strcmp(ptr, "SEPTEMBER") == 0))
            return 9;
         return -1;
   }
   return -1;
}

/*****************************************************************************
 * Clock_PrintMonth3() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *
 * ARGUMENTS
 *
 * RETURNS: void
 *
 * HISTORY
 *
 * NOTES
 ****************************************************************************/
void Clock_PrintMonth3(int mon, char *buffer, int buffLen)
{
   static char *MonthName[] = {
      "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT",
      "NOV", "DEC"
   };
   myAssert((mon > 0) && (mon < 13));
   myAssert(buffLen > 3);
   strcpy(buffer, MonthName[mon - 1]);
}
