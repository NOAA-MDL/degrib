#include <stdlib.h>
#include <stdio.h>
#include "cmapf.h"
main(){
maparam stcprm;
double lat1=40., lat2=60., cnflat;
double gsz1,gsz2;
int rc,k;
char * geoid[]={"wgs84","Clarke1866","sphereGrib2"};
  for (k=0;k<sizeof(geoid)/sizeof(geoid[0]);k++) {
    rc = useGeoid(&stcprm,geoid[k]);
    if ( rc != 0) {
      printf("Geoid %s not found.\n",geoid[k]);
      continue;
    }
    printf("Geoid %s.  Reference latitudes %8.4f and %8.4f.\n",
                      geoid[k],lat1,lat2);
    cnflat = eqvlat(&stcprm,lat1,lat2);
    printf("Tangent Latitude = %8.4f.\n",cnflat);
    rc = stlmbr(&stcprm,cnflat,-90.);
    if ( rc != 0) {
      fprintf(stderr,"stlmbr failed\n");
      return 1;
    }
    gsz1=cgszll(&stcprm,lat1,-90.);gsz2=cgszll(&stcprm,lat2,-90.);
    printf("At %8.4f, gridsize = %10.4f, while at %8.4f, gridsize = %10.4f.\n",
                     lat1,gsz1,lat2,gsz2);
    printf("Difference < 1.0e-5? .. ");
    if (fabs(gsz1-gsz2) < 1.e-5) {
      printf("yes.\n");
    } else {
      printf("no.\n");
    }
    printf("At %8.4f, gridsize = %10.4f \n--------\n",
                     cnflat,cgszll(&stcprm,cnflat,-90.));
    printf("Tangent Latitude = %8.4f.\n",(lat1+lat2)/2.);
    rc = stlmbr(&stcprm,(lat1+lat2)/2.,-90.);
    if ( rc != 0) {
      fprintf(stderr,"stlmbr failed\n");
      return 1;
    }
    gsz1=cgszll(&stcprm,lat1,-90.);gsz2=cgszll(&stcprm,lat2,-90.);
    printf("At %8.4f, gridsize = %10.4f, while at %8.4f, gridsize = %10.4f.\n",
                     lat1,gsz1,lat2,gsz2);
    printf("Difference = %8.5f\n",gsz1-gsz2);
    printf("At %8.4f, gridsize = %10.4f \n--------\n",
                     cnflat,cgszll(&stcprm,cnflat,-90.));
  }
#ifdef N_HOLD
  getc(stdin);
#endif
  return 0;
}
