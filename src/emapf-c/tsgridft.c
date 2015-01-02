#include <stdlib.h>
#include <stdio.h>
#include "cmapf.h"
main(){
maparam stcprm,sphere;
double RadEarth,x0,y0,x1,y1;
double reflat=60.,gsize=381.;
double lat0,long0,lat1,long1;
  if (useGeoid(&stcprm,"wgs84") != 0) {
    printf("Geoid wgs84 not found\n");
    exit(1);
  }
  stlmbr(&stcprm,90.,-90.); /*Polar Stereographic projection.*/
  stcm1p(&stcprm, 33.,33., 90.,-90., reflat,-90., gsize, 0.);
  cll2xy(&stcprm, 90.,-90., &x0, &y0);
  cll2xy(&stcprm, reflat,-90., &x1, &y1);
  RadEarth = sqrt((x1-x0)*(x1 - x0) + (y1-y0)*(y1-y0))*gsize/cos(RADPDEG*reflat);
  printf ("Radius = %f km.\n",RadEarth);
  mkGeoid(&sphere,AF,RadEarth,0.);
  stlmbr(&sphere,90.,-90.); /*Polar Stereographic projection.*/
  stcm1p(&sphere, 33.,33., 90.,-90., reflat,-90., gsize, 0.);
  for (y0=33.;y0>=0.;y0 -= 1.) {
    cxy2ll(&stcprm, 33.,y0, &lat0,&long0);
    cxy2ll(&sphere, 33.,y0, &lat1,&long1);
    printf("%5.2f %10.6f %10.6f %7.4f\n",y0,lat0,lat1,60.*(lat1-lat0));
  }
#ifdef N_HOLD
  fgetc(stdin);
#endif
  return 0;
}
