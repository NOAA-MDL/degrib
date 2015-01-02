#include <stdio.h>
#include "cmapf.h"

double cl2ymr(maparam * stcprm,double lat) {
double result,sinlat,coslat;
#define ECC stcprm->eccen
  sinlat = sin(RADPDEG*lat);
/* coslat gives better resolution near the poles.*/
  coslat = fabs(cos(RADPDEG*lat));
  if (coslat > 0.) {
    if (sinlat > 0.) {
      result = .5 * ECC*log((1. - ECC*sinlat) / (1. + ECC*sinlat)) +
                      log((1. + sinlat) / coslat);
    } else {
      result = .5 * ECC*log((1. - ECC*sinlat) / (1. + ECC*sinlat)) -
                      log((1. - sinlat) / coslat);
    }
  } else {
    result = (sinlat > 0.) ? MAXYMERC : -MAXYMERC;
  }
  return result;
}

double ymrcInvScale (maparam * stcprm,double lat) {
double slat,clat;
  slat = sin(RADPDEG * lat);
  clat = cos(RADPDEG * lat);
  clat = clat<0. ? -clat : clat;
  return ymrcInvScaleT (stcprm, slat, clat) ;
}

double ymrcInvScaleT (maparam * stcprm,double slat,double clat) {
  return clat/sqrt((1. - ECC*slat)*(1. + ECC*slat));
}

double cymr2l(maparam * stcprm,double ymerc) {
double sinlat,coslat;
  cmr2sc(stcprm,ymerc,& sinlat,& coslat) ;
  return DEGPRAD*atan2( sinlat,coslat);
}

void cmr2sc(maparam * stcprm,double ymerc,double * sinlat,double * coslat) {
double slat,clat,expy,expmy,adjust=1.;
int k;
  expy=exp(ymerc);
  expmy=exp(-ymerc);
  for(k=0;k<5;k++) {
    clat = 2. / (expy*adjust + expmy/adjust) ;
    slat = .5 * (expy*adjust - expmy/adjust) * clat ;
    adjust=exp( .5*ECC*log((1. + ECC*slat) / (1. - ECC*slat)) );
  }
  *sinlat = slat;*coslat=clat;
}

