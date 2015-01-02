#include <stdlib.h>
#include <math.h>
#include "cmapf.h"

/*
 * cg2cll.c  - source file for conformal mapping function utility.
 * Written 12/21/94 by
 * Dr. Albion Taylor
 * NOAA / OAR / ARL                  Phone: (301) 713-0295 ext 132
 * Rm. 3151, 1315 East-West Highway  Fax:   (301) 713-0119
 * Silver Spring, MD 20910           E-mail: ADTaylor@arlrisc.ssmc.noaa.gov
 */

void cg2cll(maparam * stcprm,double lat, double longit,
	    double ug,double vg, double * ue,double * vn) {
double along = cperiodic(longit - stcprm->reflon, -180., 180.);
double rot,slong,clong,xpolg,ypolg;
  rot = RADPDEG * (-stcprm->gamma * along);
  slong = sin(rot); clong = cos(rot);
  xpolg = slong * stcprm->crotate + clong * stcprm->srotate;
  ypolg = clong * stcprm->crotate - slong * stcprm->srotate;
  *ue = ypolg * ug - xpolg * vg;
  *vn = ypolg * vg + xpolg * ug;
}

void cg2wll(maparam * stcprm,double lat, double longit,
	    double ug,double vg, double * ue,double * vn) {
double along = cperiodic(longit - stcprm->reflon, -180., 180.);
double rot,slong,clong,xpolg,ypolg;
  if ( lat > 89. ) {
    rot = RADPDEG * ( -stcprm->gamma * along + longit - 180.);
  } else {
    if (lat < -89.) {
      rot = RADPDEG * (-stcprm->gamma * along - longit);
    } else {
      rot = RADPDEG * (-stcprm->gamma * along);
    }
  }
  slong = sin(rot); clong = cos(rot);
  xpolg = slong * stcprm->crotate + clong * stcprm->srotate;
  ypolg = clong * stcprm->crotate - slong * stcprm->srotate;
  *ue = ypolg * ug - xpolg * vg;
  *vn = ypolg * vg + xpolg * ug;
}
