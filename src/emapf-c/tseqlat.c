#include <stdlib.h>
#include <stdio.h>
#include "cmapf.h"
main(){
maparam stcprm;
double cnflat;
int rc;
rc = useGeoid(&stcprm,"wgs84");
if ( rc != 0) {
  fprintf(stderr,"useGeoid failed\n");
  return 1;
}

cnflat = eqvlat(&stcprm,40.,60.);
printf("%f ",cnflat);
rc = stlmbr(&stcprm,cnflat,-90.);
if ( rc != 0) {
  fprintf(stderr,"stlmbr failed\n");
  return 1;
}
printf("%lf \n",cgszll(&stcprm,cnflat,-90.));
printf("%lf %lf \n", cgszll(&stcprm,40.,-90.),cgszll(&stcprm,60.,-90.));
getc(stdin);
return 0;
}
