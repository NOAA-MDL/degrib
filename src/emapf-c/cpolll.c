#include <stdlib.h>
#include <math.h>
#include "cmapf.h"
#define GAMMA stcprm->gamma
/*
 * cpolll.c  - source file for conformal mapping function utility.
 * Written 12/21/94 by
 * Dr. Albion Taylor
 * NOAA / OAR / ARL                  Phone: (301) 713-0295 ext 132
 * Rm. 3151, 1315 East-West Highway  Fax:   (301) 713-0119
 * Silver Spring, MD 20910           E-mail: ADTaylor@arlrisc.ssmc.noaa.gov
 */


void cpolll(maparam * stcprm,double lat, double longit,
		double * enx,double * eny, double * enz) {
double fan = -RADPDEG * GAMMA *
		cperiodic(longit - stcprm->reflon,-180.,180.);
double clat = cos(RADPDEG * lat);
double sfan = sin(fan), cfan = cos(fan);
  *enx = clat * (sfan * stcprm->crotate + cfan * stcprm->srotate);
  *eny = clat * (cfan * stcprm->crotate - sfan * stcprm->srotate);
  *enz = sin(RADPDEG * lat);
}
#undef GAMMA
