#include <stdlib.h>
#include <math.h>
#include "cmapf.h"
#define REARTH stcprm->arad

/*
 * cpolxy.c  - source file for conformal mapping function utility.
 * Written 12/21/94 by
 * Dr. Albion Taylor
 * NOAA / OAR / ARL                  Phone: (301) 713-0295 ext 132
 * Rm. 3151, 1315 East-West Highway  Fax:   (301) 713-0119
 * Silver Spring, MD 20910           E-mail: ADTaylor@arlrisc.ssmc.noaa.gov
 */
#define GAMMA stcprm->gamma
#define NEARONE .9999999999999

void cpolxy(maparam * stcprm,double x, double y,
		double * enx,double * eny, double * enz) {
double xi0,eta0,xi,eta;
double radial,ymerc;
double slat,clat,temp;
  xi0 = (x - stcprm->x0) * stcprm->gridszeq / REARTH;
  eta0 = (y - stcprm->y0) * stcprm->gridszeq / REARTH;
  xi = xi0 * stcprm->crotate - eta0 * stcprm->srotate;
  eta = eta0 * stcprm->crotate + xi0 * stcprm->srotate;
  radial = 2. * eta - GAMMA * (xi*xi  + eta*eta);
  if ( (temp = GAMMA * radial) >= NEARONE) {
    * enx = * eny = 0.;
    * enz = GAMMA < 0. ? -1. : 1. ;
    return;
  }
  ymerc = - .5 * log1pabovera(- GAMMA,radial);
  cmr2sc(stcprm,ymerc,&slat,&clat);
  *enz = slat;
  temp = clat / sqrt(1. - temp);
  xi = -xi * GAMMA * temp;
  eta = (1. - eta * GAMMA ) *temp;
  *enx = xi * stcprm->crotate + eta * stcprm->srotate;
  *eny = eta * stcprm->crotate - xi * stcprm -> srotate;
}
#undef NEARONE
#undef GAMMA
#undef REARTH

