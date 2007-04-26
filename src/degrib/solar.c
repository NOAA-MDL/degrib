/*****************************************************************************
 * solar.c
 *
 * DESCRIPTION
 *    This file contains the information needed to compute sunrise/sunset
 * and isNightPeriod()
 *
 * sunlib.C: (comment from George Trojan) The formulas are from:
 *             Explanatory Supplement to the Astronomical Almanac,
 *             Kenneth Seidelmann (ed), University Science Books 1992,
 *             pp 484-487 (eqns 9.311 through 9.33.3)
 *             Assume const Sun semidiameter = 16' and refraction = 34'
 *             Astronomical Almanac, 2001
 *
 * HISTORY
 *   07/16/1998 George Trojan, GSC/TDL: Created sunlib.C
 *   03/04/2004 Joe Lang (MDL): "converted the minimal amount of C++ syntax
 *              into standard C syntax" creating srss.c
 *   5/2006 Arthur Taylor (MDL): Modified from srss.c
 *
 * NOTES
 *****************************************************************************
 */
#include <math.h>
#include <time.h>
#include "solar.h"
#include "clock.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
* Unix time for J2000.0 12h UT
*/
const time_t Epoch = 946728000;
const double LONG_OF_SUN = 280.460;
const double LONG_ABERRATION = 0.9856474;
const double ANOMALY = 357.528;
const double ANOMALY_ABERRATION = 0.9856003;
const double ECLIPTIC_ABERRATION = 0.0000004;
const float SEC_PER_DAY = 86400.0;
const float DAYS_PER_CENTURY = 36524.0;

enum { RISET, SUN_UP, SUN_DOWN };

double radian (double arg)
{
   return (arg * M_PI / 180.0);
}
double degree (double arg)
{
   return (arg * 180.0 / M_PI);
}

/*-------------------------------------------------------------------------*/
/*
* returns UNIX time in UTC
*/
time_t AbsTime (int year, int month, int day, int hour, int min)
{
   time_t now = time (0);
   struct tm *tms = gmtime (&now);

   tms->tm_sec = 0;
   tms->tm_min = min;
   tms->tm_hour = hour;
   tms->tm_mday = day;
   tms->tm_mon = month - 1;
   tms->tm_year = year - 1900;
   /* changed because timezone is not in ANSI C */
/*  return (mktime (tms) - timezone);  */
   return (mktime (tms) - Clock_GetTimeZone ());
}

/*-------------------------------------------------------------------------*/
/*
*  returns number of centuries from J2000.0 12h UT
*/
static double NumCenturies (time_t absTime)
{
   return (difftime (absTime, Epoch) / (SEC_PER_DAY * DAYS_PER_CENTURY));
}

/*-------------------------------------------------------------------------*/
/*
*  Mean longitude of Sun, corrected for aberration, [deg]
*  num_cent: number of centuries from J2000.0 12h UT
*/
static double MeanLonOfSun (double num_cent)
{
   double adjustment = LONG_ABERRATION * DAYS_PER_CENTURY;
   double ml = LONG_OF_SUN + adjustment * num_cent;

   while (ml >= 360)
      ml -= 360;
   while (ml < 0)
      ml += 360;
   return (ml);
}

/*-------------------------------------------------------------------------*/
/*
*  Mean anomaly [deg]
*  num_cent: number of centuries from J2000.0 12h UT
*/
static double MeanAnomaly (double num_cent)
{
   double adjustment = ANOMALY_ABERRATION * DAYS_PER_CENTURY;
   double g = ANOMALY + adjustment * num_cent;

   while (g >= 360)
      g -= 360;
   while (g < 0)
      g += 360;
   return (g);
}

/*-------------------------------------------------------------------------*/
/*
*  Ecliptic longitude [deg]
*  mean_anom: mean anomaly [deg]
*  mean_lon: mean longitude of Sun [deg]
*/
static double EclipticLon (double mean_anom, double mean_lon)
{
   return (mean_lon + 1.915 * sin (radian (mean_anom)) +
           0.020 * sin (radian (2.0 * mean_anom)));
}

/*-------------------------------------------------------------------------*/
/*
*  Obliquity of ecliptic [deg]
*  num_cent: number of centuries from J2000.0 12h UT
*/
static double OblOfEcliptic (double num_cent)
{
   double adjustment = ECLIPTIC_ABERRATION * DAYS_PER_CENTURY;

   return (23.439 - adjustment * num_cent);
}

/*-------------------------------------------------------------------------*/
/*
*  Right ascension
*  obl_ecl: obliquity of ecliptic [deg]
*  ecl_lon: ecliptic longitude [deg]
*/
static double Ascension (double ecl_lon, double obl_ecl)
{
   double t;

   ecl_lon = radian (ecl_lon);
   obl_ecl = radian (obl_ecl);
   t = tan (obl_ecl / 2.) * tan (obl_ecl / 2.);
   return (degree (ecl_lon - (t * sin (2. * ecl_lon)) +
                   (0.5 * t * t * sin (4. * ecl_lon))));
}

/*-------------------------------------------------------------------------*/
/*
*  Equation of time [deg]
*  mean_lon: mean_longitude [deg]
*  alpha: ascension [deg]
*/
static double EqnOfTime (double mean_lon, double alpha)
{
   return (mean_lon - alpha);
}

/*-------------------------------------------------------------------------*/
/*
*  declination [deg]
*  obl_ecl: obliquity of ecliptic [deg]
*  ecl_lon: ecliptic longitude [deg]
*/
static double Declination (double obl_ecl, double ecl_lon)
{
   return (degree (asin (sin (radian (obl_ecl)) * sin (radian (ecl_lon)))));
}

/*-------------------------------------------------------------------------*/
/*
*  Calculates distance of sun from earth in astronomical units
*  mean_anom: mean anomaly [deg]
*/
static double DistanceFromSun (double mean_anom)
{
   return (1.00014 - (0.01671 * cos (radian (mean_anom))) -
           (0.00014 * cos (radian (2. * mean_anom))));
}

/*-------------------------------------------------------------------------*/
/*
*  Hour angle: solves the equation wr to omega [deg]
*     cos(theta) = sin(delta)*sin(fi) + cos(delta)*cos(fi)*cos(omega)
*     where:
*        theta:  zenith angle
*        delta:  declination
*        fi:     latitude
*        omega:  hour angle
*  decl: declination [deg]
*  lat: latitude [deg]
*  h: correction for refraction + semidiameter [deg]
*/
static double HourAngle (double decl, double lat, double h)
{
   double tmp;

   h = radian (h);
   decl = radian (decl);
   lat = radian (lat);
   tmp = sin (h) / (cos (decl) * cos (lat)) - tan (decl) * tan (lat);
   if (tmp >= 1.0)
      return (0.0);
   else if (tmp <= -1.0)
      return (180);
   else
      return (degree (acos (tmp)));
}

/*-------------------------------------------------------------------------*/
/*
*  Calculates semidiameter of sun based on distance of sun from earth.
*  dist: distance of sun from earth [au]
*/
static double SemidiameterOfSun (double dist)
{
   return (0.2666 / dist);
}

/*-------------------------------------------------------------------------*/
/*  Calculate the sunrise or sunset time
*  T: Number of Centuries since Jan 1, 2001, 12Z
   lat: Station latitude [deg]
   lon: Station longitude [deg]
   sunrise: 0 for sunset, 1 for sunrise
   zone: adjustment to UTC in hours to obtain local time
*/
double CalcTime (double T, double lat, double lon, int sunrise, double UT)
{
   int n;
/*
 * precision: 6 s
 */
   static const double eps = 6.0 / 3600.0;
   double UT1;

/* 10 iterations max or until when the consecutive values differ < 6 s
*/
   for (n = 0; n < 10; n++) {
      double L = MeanLonOfSun (T);
      double G = MeanAnomaly (T);
      double lambda = EclipticLon (G, L);
      double epsilon = OblOfEcliptic (T);
      double alpha = Ascension (lambda, epsilon);
      double E = EqnOfTime (L, alpha);
      double delta = Declination (epsilon, lambda);
      double R = DistanceFromSun (G);
      double SD = SemidiameterOfSun (R);
/*
*  Assume 34 arcmin for refraction
*/
      double h = -(34. / 60. + SD);
      double t = HourAngle (delta, lat, h);
      if (fabs (t) == 0 || fabs (t) == 180)
         return (t);
      if (sunrise == 0) {
         t = -t;
      }
      UT1 = 12.0 - (E + lon + t) / 15.0;
      T += (UT1 - UT) / (DAYS_PER_CENTURY * 24.0);
      if (fabs (UT1 - UT) < eps)
         return (UT1);
      UT = UT1;
   }
   return (UT1);
}

/*-------------------------------------------------------------------------*/
/*
*  calculates sunrise and sunset [sidereal days]
*  year, month, day: date
*  lat: latitude [deg]
*  lon: longitude [deg]
*  riset: returned time [hour LT (local time)]
*  sunrise: 1 for sunrise, 0 for sunset
*  zone: number of hours from UTC
*/
int RiseSet (int year, int month, int day, double lat, double lon,
             double *riset, int sunrise, int zone)
{
   double UT = 12.0 - lon / 15.0; /* start value */
   int min = 0;
   double T = NumCenturies (AbsTime (year, month, day, (int) (UT), min));
   double LT;
/*   Calculate the sunrise or sunset */
   double UT1 = CalcTime (T, lat, lon, sunrise, UT);
/* if no sunrise or sunset, exit this routine
*/

   if (fabs (UT1) == 0)
      return (SUN_DOWN);
/*  We need to consider the possibility that the sun set before it rose today.
 *  This can happen at high latitudes, where the sunset may set close to midnight,
 *  which is shifted into the next day because the selection of time zone doesn't
 *  coincide well with the position of the sun.  In other words, we need to consider
 *  the possibility that the sunset associated with yesterday, actually occurred
 *  today.
*/
   else if (fabs (UT1) == 180) {
      T = T - (1.0 / DAYS_PER_CENTURY);
      UT1 = CalcTime (T, lat, lon, sunrise, UT);
/* if the sun is up all day or down all day as a result of the adjustment to LT,
   exit this routine
*/
      if (fabs (UT1) == 0)
         return (SUN_DOWN);
      else if (fabs (UT1) == 180)
         return (SUN_UP);
/* Otherwise, compute the sunrise/sunset time in local time coordinates
*/
      LT = UT1 + zone;
/*  The sunrise or sunset doesn't occur in this calendar day as a result of
    of the adjustment to LT
*/
      if (LT < 24)
         return (SUN_UP);
      else {
         *riset = LT - 24;
         return (RISET);
      }
   }

/*    Now check to determine if we've calculated a sunrise/sunset for the
      correct day in local time.  If not adjust the day and try again.
*/
   LT = UT1 + zone;

   if (LT > 24) {
      T = T - (1.0 / DAYS_PER_CENTURY);
      UT1 = CalcTime (T, lat, lon, sunrise, UT);
/* if the sun is up all day or down all day as a result of the adjustment to LT,
   exit this routine
*/
      if (fabs (UT1) == 0)
         return (SUN_DOWN);
      else if (fabs (UT1) == 180)
         return (SUN_UP);

/*  Otherwise, compute the sunrise/sunset time in local time coordinates
*/
      LT = UT1 + zone;

/*  The sunrise or sunset doesn't occur in this calendar day as a result of
    of the adjustment to LT
*/
      if (LT < 24) {
         if (sunrise == 0)
            return (SUN_DOWN);
         else
            return (SUN_UP);
      }
   } else if (LT < 0) {
      T = T + (1.0 / DAYS_PER_CENTURY);
      UT1 = CalcTime (T, lat, lon, sunrise, UT);
      LT = UT1 + zone;
/*  The sunrise or sunset doesn't occur in this calendar day as a result
    of the adjustment to LT
*/
      if (LT > 0) {
         if (sunrise == 0)
            return (SUN_DOWN);
         else
            return (SUN_UP);
      }
   }
   LT = UT1 + zone;
   while (LT >= 24.0)
      LT -= 24.0;
   while (LT < 0.0)
      LT += 24.0;
   *riset = LT;
   return (RISET);
}

/*-------------------------------------------------------------------------*/
/* Return's the time of the rising or setting sun (f_sunrise = 1 for sunrise)
 * in UTC on the date specified by year/mon/day in UTC at location lat/lon */
/* returns 1 if sun always down.
 * returns 2 if sun always up.
 * returns 0 if time is valid. */
int sunTime (sInt4 year, int mon, int day, double lat, double lon,
             int f_sunrise, int *hh, int *mm, int *ss)
{
   double riset;
   int rc;

   /* num_off_UTC = 0 is number of hours offset from UTC. */
   rc = RiseSet (year, mon, day, lat, lon, &riset, f_sunrise, 0);

   switch (rc) {
      case SUN_DOWN:
         *hh = 0;
         *mm = 0;
         *ss = 0;
         return 1;
      case SUN_UP:
         *hh = 0;
         *mm = 0;
         *ss = 0;
         return 2;
   }
   *ss = (int) (riset * 3600);
   *hh = *ss / 3600;
   *mm = (*ss - *hh * 3600) / 60;
   *ss = *ss % 60;
   /* *ss = *ss - *hh * 3600 - *mm * 60; */
   if (*hh == 24)
      *hh = 0;
   return 0;
}

/*-------------------------------------------------------------------------*/
/* Returns true if the time (in UTC) at lat/lon is during the night.
 * returns false if the time (in UTC) at lat/lon is during the day.
 * zone (EST is 4) = amount to shift from UTC to LST
 */
#define ISDARK 1
#define ISLIGHT 0
int isNightPeriod (double time, double lat, double lon, int zone)
{
   sInt4 year;
   int mon;
   int day;
   int hour;
   int min;
   double sec;
   double rise;
   double set;
   double now;
   int rc;

   time = time - zone * 3600;
   Clock_PrintDate (time, &year, &mon, &day, &hour, &min, &sec);
   now = hour + min / 60. + sec / 3600.;

   /* sunrise is 1 so we get sunrise */
   /* num_off_UTC = 0 is number of hours offset from UTC. */
   rc = RiseSet (year, mon, day, lat, lon, &rise, 1, -1 * zone);
   switch (rc) {
      case SUN_DOWN:
         return ISDARK;
      case SUN_UP:
         return ISLIGHT;
   }

   /* sunrise is 0 so we get sunset */
   /* num_off_UTC = 0 is number of hours offset from UTC. */
   rc = RiseSet (year, mon, day, lat, lon, &set, 0, -1 * zone);
   switch (rc) {
      case SUN_DOWN:
         return ISDARK;
      case SUN_UP:
         return ISLIGHT;
   }

   if (rise < set) {
      if (now < rise)
         return ISDARK;
      else if (now < set)
         return ISLIGHT;
      else
         return ISDARK;
   } else {
      if (now < set)
         return ISLIGHT;
      else if (now < rise)
         return ISDARK;
      else
         return ISLIGHT;
   }
}

#ifdef DEBUG_SOLAR
int main (int argc, char **argv)
{
   double time;
   double lat;
   double lon;
   sInt4 year;
   int month;
   int day;
   int hour;
   int min;
   double sec;
   int i;
   char f_isNight;

   lat = 38;
   lon = -78;
   time = Clock_Seconds();

   for (i = 0; i < 100; ++i) {
      Clock_PrintDate (time, &year, &month, &day, &hour, &min, &sec);
      f_isNight = isNightPeriod (time, lat, lon, 4);
      if (f_isNight) {
         printf ("Night: %d/%d/%d %d:%d\n", month, day, year, hour, min);
      } else {
         printf ("Day: %d/%d/%d %d:%d\n", month, day, year, hour, min);
      }
      /* Add 1 hour to time */
      time = time + 3600;
   }
}
#endif

#undef ISDARK
#undef ISLIGHT
