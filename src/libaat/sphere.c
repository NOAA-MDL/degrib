/*****************************************************************************
 * sphere.c
 *
 * DESCRIPTION:
 *    This module contains computations that deal with distances or angles on
 * a sphere.
 *
 * HISTORY
 * 10/2003 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 * The original procedures assumed: 60nm = 1 degree => 2 pi R = 360 * 60 nm
 * => R = 6366.7070 km (a lot of this was in "mercator.c")
 ****************************************************************************/
#include <math.h>
#include "libaat.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PI_360 0.0087266462599716478846
#define PI_180 0.017453292519943295769
#define C180_PI 57.295779513082320877

/*****************************************************************************
 * BearCompute() -- Arthur Taylor / TDL
 *
 * PURPOSE
 *    To compute the bearing from one lat/lon to another in degrees from
 * north, treating the surface as a sphere, rather than as a plane.  Hence
 * no map projection.
 *
 * ARGUMENTS
 * f_radian = Return the answer in radians (Input)
 *     lat1 = (unprojected) latitude of point 1 (degrees) (Input)
 *     lon1 = (unprojected) longitude of point 1 (neg west degrees) (Input)
 *     lat2 = (unprojected) latitude of point 2 (degrees) (Input)
 *     lon2 = (unprojected) longitude of point 2 (neg west degrees) (Input)
 *    theta = Direction from point 1 to point 2 (in degrees from N) (Output)
 *
 * RETURNS: void
 *
 * HISTORY
 *  6/1998 Arthur Taylor (RDC/TDL): Created (originally in mercator.c)
 * 10/2003 AAT (RSIS/MDL): Revisited.
 *  9/2007 AAT (MDL): Added check to see if points are co-located 
 *
 * NOTES
 ****************************************************************************/
void BearCompute(char f_radian, double lat1, double lon1, double lat2,
                 double lon2, double *theta)
{
   if ((lat1 == lat2) && (lon1 == lon2)) {
      *theta = 0;
      return;
   }
   lat1 *= PI_180;
   lon1 *= PI_180;
   lat2 *= PI_180;
   lon2 *= PI_180;
   *theta = M_PI + atan2(cos(lat2) * (cos(lon2) * sin(lon1) -
                                      sin(lon2) * cos(lon1)),
                         (sin(lat1) * cos(lat2) * cos(lon1 - lon2) -
                          cos(lat1) * sin(lat2)));
   if (*theta > 2 * M_PI) {
      *theta = *theta - 2 * M_PI;
   }
   if (!f_radian) {
      *theta = *theta * C180_PI;
   }
}

/*****************************************************************************
 * DistCompute() -- Arthur Taylor / TDL
 *
 * PURPOSE
 *    To compute the distance between two lat/lon points, assuming a spherical
 * earth.  Returned distance is in the units of the given Radius of earth.
 *
 * ARGUMENTS
 *    R = Radius of earth to use. (Input)
 *        (6371.2 km : GRIB2) (6367.47 km: GRIB1)
 *        (60nm = 1 degree => 2 pi R = 360 * 60 nm => R = 6366.7070 km)
 * lat1 = (unprojected) latitude of point 1 (degrees) (Input)
 * lon1 = (unprojected) longitude of point 1 (neg west degrees) (Input)
 * lat2 = (unprojected) latitude of point 2 (degrees) (Input)
 * lon2 = (unprojected) longitude of point 2 (neg west degrees) (Input)
 * dist = Distance from point 1 to point 2 (Output)
 *
 * RETURNS: void
 *
 * HISTORY
 *  6/1998 Arthur Taylor (RDC/TDL): Created
 * 10/2003 AAT (RSIS/MDL): Allowed Radius of earth to be passed in.
 *  4/2007 AAT (MDL): Decided to not do unit conversion here.  Answer is in
 *         same unit as the given Radius.
 *
 * NOTES
 *   Algortihm see:
 * http://en.wikipedia.org/wiki/Haversine_formula
 * http://www.movable-type.co.uk/scripts/GIS-FAQ-5.1.html
 * Haversine Formula (from R.W. Sinnott, "Virtues of the Haversine",
 * Sky and Telescope, vol. 68, no. 2, 1984, p. 159):
 *    dlon = lon2 - lon1
 *    dlat = lat2 - lat1
 *    a = (sin(dlat/2))^2 + cos(lat1) * cos(lat2) * (sin(dlon/2))^2
 *    c = 2 * arcsin(min(1,sqrt(a)))   <Note: (arcsin(1) == 90)>
 *    d = R * c
 * R-Radius of earth, c-great circle distance in radians.
 ****************************************************************************/
void DistCompute(double R, double lat1, double lon1, double lat2, double lon2,
                 double *dist)
{
   double dLat;         /* Change in latitude / 2. */
   double dLon;         /* Change in longitude / 2. */
   double a;            /* The haversine angle */
   double c;            /* Great circle distance in radians. */

   dLon = (lon2 - lon1) * PI_360;
   dLat = (lat2 - lat1) * PI_360;
   a = sin(dLat) * sin(dLat) +
         cos(lat1 * PI_180) * cos(lat2 * PI_180) * sin(dLon) * sin(dLon);
   /* c = (sqrt(a) < 1) ? 2 * asin(sqrt(a)) : 2 * arcsin(1) = 2 * 90; */
   c = (sqrt(a) < 1) ? 2 * asin(sqrt(a)) : 180;
   *dist = R * c;
}

/*****************************************************************************
 * LatLonCompute() -- Arthur Taylor / TDL
 *
 * PURPOSE
 *    Given a lat/lon and a distance and bearing on a sphere, compute the
 * resulting lat/lon.  Assumes the unit of dist is the same as R.
 *
 * ARGUMENTS
 *    R = Radius of earth to use. (Input)
 *        (6371.2 km : GRIB2) (6367.47 km: GRIB1)
 *        (60nm = 1 degree => 2 pi R = 360 * 60 nm => R = 6366.7070 km)
 * lat1 = (unprojected) latitude of point 1 (degrees) (Input)
 * lon1 = (unprojected) longitude of point 1 (neg west degrees) (Input)
 * dist = The distance to travel on the sphere (same unit as R). (Input)
 * bear = The direction from point 1 to travel (in degrees from N) (Input)
 * lat2 = (unprojected) latitude of point 2 (degrees) (Output)
 * lon2 = (unprojected) longitude of point 2 (neg west degrees) (Output)
 *
 * RETURNS: void
 *
 * HISTORY
 * 10/2003 Arthur Taylor (RSIS/MDL): Created
 *  4/2007 AAT (MDL): Decided to not do unit conversion here.
 *
 * NOTES
 ****************************************************************************/
void LatLonCompute(double R, double lat1, double lon1, double dist,
                   double bear, double *lat2, double *lon2)
{
   double A, B, C;

   /* Switch to radians. */
   lat1 *= PI_180;
   lon1 *= PI_180;

   dist = dist / R;
   /* (M_PI - bear) is to get bear oriented correctly. */
   bear = M_PI - bear * PI_180;

   *lat2 = asin(cos(dist) * sin(lat1) -
                sin(dist) * cos(bear) * cos(lat1)) * C180_PI;
   A = cos(dist) * cos(lat1);
   B = sin(dist) * cos(bear) * sin(lat1);
   C = sin(dist) * sin(bear);
   *lon2 = atan2(A * sin(lon1) + B * sin(lon1) + C * cos(lon1),
                 A * cos(lon1) + B * cos(lon1) - C * sin(lon1)) * C180_PI;
}

/* http://www.movable-type.co.uk/scripts/LatLong.html */
/* http://www.movable-type.co.uk/scripts/LatLongVincenty.html */
/* http://www.movable-type.co.uk/scripts/LatLongVincentyDirect.html */

#ifdef DEBUG_SPHERE
int main(int argc, char **argv)
{

}
#endif
