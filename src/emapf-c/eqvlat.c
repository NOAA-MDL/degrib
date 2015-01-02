#include <stdlib.h>
#include "cmapf.h"

/*
 * eqvlat.c  - source file for conformal mapping function utility.
 * Written 12/21/94 by
 * Dr. Albion Taylor
 * NOAA / OAR / ARL                  Phone: (301) 713-0295 ext 132
 * Rm. 3151, 1315 East-West Highway  Fax:   (301) 713-0119
 * Silver Spring, MD 20910           E-mail: ADTaylor@arlrisc.ssmc.noaa.gov
 */
double eqvlat(maparam * stcprm,double lat1,double lat2) {

/*  Written 12/21/94 by Dr. Albion Taylor
 *
 *    This function is provided to assist in finding the tangent latitude
 *    equivalent to the 2-reference latitude specification in the legend
 *    of most lambert conformal maps.  If the map specifies "scale
 *    1:xxxxx true at 40N and 60N", then eqvlat(40.,60.) will return the
 *    equivalent tangent latitude.
 *  INPUTS:
 *    lat1, lat2:  The two latitudes specified in the map legend
 *  RETURNS:
 *    the equivalent tangent latitude
 *  EXAMPLE:  stlmbr(& stcprm, eqvlat(40.,60.), 90.)
 */
/*   Changes Made May 9, 2003 to accomodate the following special
 *   situations:
 *   1. if lat1 == lat2, returned value will be lat1 (reduced to between -90.
 *      and 90.).
 *   2. If either lat1 or lat2 is 90. (or -90.) then 90. (or -90.) will be
 *      returned.  This reflects the fact that, for y fixed (-90. < y < 90.),
 *      as x -> 90. (or -90.), eqvlat(x,y) ->90. (or -90.)  This limiting
 *      case of tangent latitude 90. is a polar stereographic projection,
 *      for which the scale at 90. is a maximum, and therefore greater than the
 *      other latitude y.  Thus, eqvlat(90.,60.) returns 90., although the
 *      scale at 90. will be greater than at 60. For eqvlat(90.,-90.), the
 *      limit process is ambiguous; for the sake of symmetry, such a case
 *      will return 0.
 */

double result,slat1,slat2,ymerc1,ymerc2,s1,s2;


/*First, test whether stcprm is a valid geoid
 */
  if (mkGeoid(stcprm,TST,0.,0.) != 0) return 999.;

  slat1 = sin(RADPDEG * lat1);
  slat2 = sin(RADPDEG * lat2);
/* reorder, slat1 larger */
  if (slat1 < slat2) {
     double temp = slat1;
     slat1 = slat2;
     slat2 = temp;
     temp = lat1;
     lat1 = lat2;
     lat2 = temp;
  }
/*  Take care of special cases first */
  if (slat1 == slat2) return asin(slat1) * DEGPRAD;
  if (slat1 == -slat2 ) return 0.;
  if (slat1 >= 1.) return 90.;
  if (slat2 <= -1.) return -90.;
/********************************************************/

  ymerc1 = cl2ymr(stcprm, lat1);
  ymerc2 = cl2ymr(stcprm, lat2);
  if (ymerc1 > ymerc2 ) {

    s1 = ymrcInvScale(stcprm, lat1);
    s2 = ymrcInvScale(stcprm,lat2);
    result = log(s1/s2) /
           (ymerc2 - ymerc1);
  } else {
    /* We are here because sin(lat1) > sin(lat2) while ymerc1 <= ymerc2.
     * This can only happen through round-off error when sin(lat1) is very
     * clos eo sin(lat2).  We return the common ymerc value,
     */
    return cymr2l(stcprm,.5*(ymerc2+ymerc1));
  }
  /*At this point, result is a sine of latitude.*/
  return DEGPRAD*atan2(result,sqrt((1.-result)*(1.+result)));
}
