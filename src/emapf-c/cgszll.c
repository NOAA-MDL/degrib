#include <stdlib.h>
#include <math.h>
#include "cmapf.h"

/*
 * cgszll.c  - source file for conformal mapping function utility.
 * Written 12/21/94 by
 * Dr. Albion Taylor
 * NOAA / OAR / ARL                  Phone: (301) 713-0295 ext 132
 * Rm. 3151, 1315 East-West Highway  Fax:   (301) 713-0119
 * Silver Spring, MD 20910           E-mail: ADTaylor@arlrisc.ssmc.noaa.gov
 */
#define NEARONE .9999999999999
#define ECCEN stcprm->eccen
#define GAMMA stcprm->gamma
#if 1==0
static double gszlim(double eccen,double slat,double gamma) ;

double cgszll(maparam * stcprm,double lat,double longit) {
double slat = sin(RADPDEG * lat);
double ymerc;
/*doouble clat;*/
  if (slat <= -NEARONE)
    return
     (GAMMA < -NEARONE ?
      stcprm->gridszeq * gszlim(ECCEN,-slat,-GAMMA) : 0.);

  if (slat >= NEARONE)
    return
     (GAMMA > NEARONE ?
      stcprm->gridszeq * gszlim(ECCEN,slat,GAMMA) : 0.);

/*  clat = cos(RADPDEG * lat);clat = clat<0. ? -clat : clat;*/
  ymerc = cl2ymr(stcprm,lat);
  return stcprm->gridszeq * exp(GAMMA * ymerc) * ymrcInvScale (stcprm,lat);
}

static double gszlim(double eccen,double slat,double gamma) {
/*approximate gridsize near pole in nearly stereographic projections.*/
double ecslat=eccen*slat;
  return exp(.5 * (
          eccen *gamma* log((1.-ecslat)/(1.+ecslat)) +
         (1.+gamma)*log(1. + slat) ) ) /
         sqrt((1. - ecslat)*(1. + ecslat));
}
#else
static double gszlim(maparam * stcprm,double lat) {
/* approximate gridsize near pole in nearly stereographic projections.*/
/* Entry only when sinlat near a pole. (Close to +-1.0)*/
double sinlat = sin(RADPDEG * lat);
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

double cgszll(maparam * stcprm,double lat,double longit) {
double ymerc;
  ymerc = cl2ymr(stcprm,lat);
  if ((ymerc >= MAXYMERC) || (ymerc <= -MAXYMERC)) return gszlim(stcprm,lat);
  return stcprm->gridszeq * exp(GAMMA * ymerc) * ymrcInvScale (stcprm,lat);
}

#endif
  #undef GAMMA
  #undef ECCEN
  #undef NEARONE

