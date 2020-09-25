#ifndef MYMAPF_H
#define MYMAPF_H

#include "cmapf.h"
#include "meta.h"

typedef struct {
  maparam stcprm;
  char f_latlon;
  double lat1;
  double latN;
  double lon1;           /* lon of west corner of grid. */
  double lonN;           /* lon of east corner of grid. */
                         /* lonN and lon1 are set so that:
                          * A) lon1 < lonN,
                          * B) lonN - lon1 <= 360
                          * C) lonN in [0,360), lon1 in [-360,360) */
  double Nx, Ny;
  double Dx, Dy;
  double ratio;         /*  Ratio of Dy / Dx. */
} myMaparam;

void myCxy2ll (myMaparam * map, double x, double y, double *lat, double *lon);

void myCll2xy (myMaparam * map, double lat, double lon, double *x, double *y);

int GDSValid (const gdsType * gds);

void SetMapParamGDS (myMaparam * map, const gdsType *gds);

int computeSubGrid (LatLon *lwlf, int *x1, int *y1, LatLon *uprt, int *x2,
                    int *y2, gdsType *gds, gdsType *newGds);

int DateLineLat (double lon1, double lat1, double lon2, double lat2,
                 double *ans);

#endif
