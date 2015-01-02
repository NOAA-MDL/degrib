#include <stdlib.h>
#include <math.h>
#include "cmapf.h"
#define REARTH stcprm->arad
#define ARAD stcprm->arad
#define BRAD stcprm->brad
#define ECCEN stcprm->eccen
#define GAMMA stcprm->gamma

/*
 * stlmbr.c  - source file for conformal mapping function utility.
 * Written 12/21/94 by
 * Dr. Albion Taylor
 * NOAA / OAR / ARL                  Phone: (301) 713-0295 ext 132
 * Rm. 3151, 1315 East-West Highway  Fax:   (301) 713-0119
 * Silver Spring, MD 20910           E-mail: ADTaylor@arlrisc.ssmc.noaa.gov
 */

static void cnllxy(maparam * stcprm,double lat,double longit,
		double * xi,double * eta) ;

int stlmbr(maparam * stcprm,double tnglat,double reflon) {
double xi,eta;
/*First check for consistency of geoid: */
  if (mkGeoid(stcprm,TST,0.,0.) != 0) return 1;

  stcprm->reflon = reflon;
  GAMMA = sin(tnglat * RADPDEG);
  stcprm->x0 = stcprm->y0 = stcprm->srotate = 0;
  stcprm->crotate = 1.;
  stcprm->gridszeq = REARTH;
  cnllxy(stcprm, 89.,reflon, &xi, &eta);
  stcprm->npwarn = 2. * eta - GAMMA * (eta * eta);
  cnllxy(stcprm, -89.,reflon, &xi, &eta);
  stcprm->spwarn = 2. * eta - GAMMA * (eta * eta);
  return 0;
}

void cll2xy(maparam * stcprm,double lat,double longit,
		double * x,double * y) {
double xi,eta;
  cnllxy(stcprm,lat,longit,&xi,&eta);
  *x = stcprm->x0 + REARTH / stcprm->gridszeq  *
     (stcprm->crotate * xi + stcprm->srotate * eta);
  *y = stcprm->y0 + REARTH / stcprm->gridszeq  *
     (stcprm->crotate * eta - stcprm->srotate * xi);
}

static void cnllxy(maparam * stcprm,double lat,double longit,
		double * xi,double * eta) {
#define FSM 1.e-2
/*#define NEARONE .9999999999999
  double slat;
*/
double gdlong,sndgam,cdgam,mercy,gmercy,rhog1;
double dlong=RADPDEG * cperiodic(longit - stcprm->reflon,-180.,180.);
  gdlong = GAMMA * dlong;
  if ((gdlong < -FSM) || (gdlong > FSM)) {
    sndgam = sin(gdlong) /GAMMA;
    cdgam = (1. - cos(gdlong)) / GAMMA / GAMMA;
  } else {
    gdlong *= gdlong;
    sndgam = dlong * (1. - 1./6. * gdlong *
		     (1. - 1./20. * gdlong *
		     (1. - 1./42. * gdlong )));
    cdgam = dlong * dlong * (1. - 1./12. * gdlong *
			    (1. - 1./30. * gdlong *
                            (1. - 1./56. * gdlong )));
  }
#if 1==0
  slat = sin(RADPDEG * lat);
  if ((slat >=NEARONE) || (slat <= -NEARONE)) {
    *eta = 1./GAMMA;
    *xi = 0.;
    return ;
  }
#endif
  mercy =  cl2ymr(stcprm,lat);
  gmercy = mercy * GAMMA;
  if ( (gmercy < -FSM) || (gmercy > FSM)){
    rhog1 = (1. - exp(- gmercy) ) / GAMMA;
    if (GAMMA > 0.) {
      if (mercy >= MAXYMERC) rhog1 = 1. / GAMMA;
    } else if (GAMMA < 0.) {
      if (mercy <= -MAXYMERC) rhog1 = 1. / GAMMA;
    }
  } else {
    rhog1 = mercy * (1. - 1./2. * gmercy *
		    (1. - 1./3. * gmercy *
		    (1. - 1./4. * gmercy)));
  }
  *eta = rhog1 + (1. - GAMMA * rhog1) * GAMMA *
			cdgam;
  *xi = (1. - GAMMA * rhog1 ) * sndgam;
#undef FSM
#undef REARTH
#undef GAMMA
#undef ECCEN
#undef BRAD
#undef ARAD
}

