#include <stdlib.h>
#include <math.h>
#include "cmapf.h"
#define REARTH stcprm->arad
#define NEARONE .9999999999999
#define ECCEN stcprm->eccen
#define GAMMA stcprm->gamma
/*
 * cgszxy.c  - source file for conformal mapping function utility.
 * Written 12/21/94 by
 * Dr. Albion Taylor
 * NOAA / OAR / ARL                  Phone: (301) 713-0295 ext 132
 * Rm. 3151, 1315 East-West Highway  Fax:   (301) 713-0119
 * Silver Spring, MD 20910           E-mail: ADTaylor@arlrisc.ssmc.noaa.gov
 */
#if 1==0
static double gszlim(double eccen,double slat,double gamma) {
/*approximate gridsize near pole in nearly stereographic projections.*/
double ecslat=eccen*slat;
  return exp(.5 * (
          eccen *gamma* log((1.-ecslat)/(1.+ecslat)) +
         (1.+gamma)*log(1. + slat) ) ) /
         sqrt(1. - ecslat*ecslat);
}

double cgszxy(maparam * stcprm,double x,double y) {
double xi,eta,xi0,eta0,ymerc,slat,clat;
double radial;
  xi0 = (x - stcprm->x0) * stcprm->gridszeq / REARTH;
  eta0 = (y - stcprm->y0) * stcprm->gridszeq / REARTH;
  xi = xi0 * stcprm->crotate - eta0 * stcprm->srotate;
  eta = xi0 * stcprm->srotate + eta0 * stcprm->crotate;
  radial = 2. * eta - GAMMA * (xi*xi + eta*eta);
  if ((NEARONE - GAMMA * radial) <= 0.) {
    if ( GAMMA < -NEARONE ) return
      stcprm->gridszeq * gszlim(ECCEN,-1.,-GAMMA);
    else {
      if (GAMMA > NEARONE) return
        stcprm->gridszeq * gszlim(ECCEN,1.,GAMMA);
      else return 0;
    }
  }
  ymerc = - .5 * log1pabovera(- GAMMA,radial);
/*If y mercator > 17, we are within 1.0e-5 degrees (10^-5 km or 1cm.) of pole.*/
  if (ymerc >  17.)
     return GAMMA > NEARONE ?
             stcprm->gridszeq*gszlim(ECCEN,1.,GAMMA) : 0. ;
  if (ymerc <  -17.)
     return GAMMA < -NEARONE ?
             stcprm->gridszeq*gszlim(ECCEN,-1,-GAMMA) : 0. ;
  cmr2sc(stcprm,ymerc,&slat,&clat);
  return stcprm->gridszeq * exp(GAMMA * ymerc) * clat /
       sqrt((1. - ECCEN*slat)*(1. + ECCEN * slat));
#else
static double gszlim(maparam * stcprm,double sinlat) {
/* approximate gridsize near pole in nearly stereographic projections.*/
/* Entry only when sinlat near a pole. (Close to +-1.0)*/
double ecslat=ECCEN*sinlat;
/* basic principle is to factor out (1. - sinlat)^((1-gamma)/2) near the
 * North Pole, where it is zero if gamma<1, but one if gamma==1,
 * and a similar expression near the South Pole.
 */
  if (sinlat > 0.) {
    return (GAMMA > NEARONE) ? stcprm->gridszeq * exp(.5 * (
         ECCEN *GAMMA* log((1.-ecslat)/(1.+ecslat)) +
         (1. + GAMMA)*log(1. + sinlat) ) ) /
         sqrt((1. - ecslat)*(1. + ecslat)) : 0.;
  } else {
    return (GAMMA < -NEARONE) ? stcprm->gridszeq * exp(.5 * (
         ECCEN *GAMMA* log((1.-ecslat)/(1.+ecslat)) +
         (1. - GAMMA)*log(1. - sinlat) ) ) /
         sqrt((1. - ecslat)*(1. + ecslat)): 0.;
  }
}

double cgszxy(maparam * stcprm,double x,double y) {
double xi,eta,xi0,eta0,ymerc,slat,clat;
double radial;
  xi0 = (x - stcprm->x0) * stcprm->gridszeq / REARTH;
  eta0 = (y - stcprm->y0) * stcprm->gridszeq / REARTH;
  xi = xi0 * stcprm->crotate - eta0 * stcprm->srotate;
  eta = xi0 * stcprm->srotate + eta0 * stcprm->crotate;
  radial = 2. * eta - GAMMA * (xi*xi + eta*eta);
  ymerc = - .5 * log1pabovera(- GAMMA,radial);
  cmr2sc(stcprm,ymerc,&slat,&clat);
/*If y mercator > 17, we are within 1.0e-5 degrees (10^-5 km or 1 cm) of pole.*/
  if ((ymerc >= MAXYMERC) || (ymerc <= -MAXYMERC)) return gszlim(stcprm,slat);
  return stcprm->gridszeq * exp(- GAMMA * ymerc) * ymrcInvScaleT(stcprm,slat,clat);
/*  * clat /
       sqrt((1. - ECCEN*slat)*(1. + ECCEN * slat)); */
#endif
#undef GAMMA
#undef ECCEN
#undef NEARONE
#undef REARTH
}



