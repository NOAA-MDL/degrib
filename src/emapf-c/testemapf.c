#include <stdlib.h>
#include <stdio.h>
#include "cmapf.h"

main(){
maparam stcprm,stcprm2;
double cnflat,cnflat2;
double reflat=60.,refgrid=381.;

double x,y,xlat,ylong,gsza,gszb;
int rc;
rc = useGeoid(&stcprm,"wgs84");
if ( rc != 0) {
  fprintf(stderr,"useGeoid failed\n");
  return 1;
}
stlmbr(&stcprm,90.,-100.);

if (stcm1p(&stcprm,33.,33., 90.,-100., 60.,-100., 371., 0.)!=0) {
  fprintf(stderr,"stcm1p error\n");
}
printf("gsize %f\n",cgszll(&stcprm,60.,-100.));

{double lat1, ymrc, lat2,sl,cl;
  for (lat1=-55.;lat1<60.;lat1+=55.) {
   ymrc=cl2ymr(&stcprm,lat1);
   lat2=cymr2l(&stcprm,ymrc);
   cmr2sc(&stcprm,ymrc,&sl,&cl);
   printf("ymerc: %f %f %f %f %f\n",lat1,ymrc,lat2,sl,cl);
 }
}

{double lat1=-55.,lon1=-145,x1,y1,lat2,lon2;
  cll2xy(&stcprm,lat1,lon1,&x1,&y1);
  cxy2ll(&stcprm,x1,y1,&lat2,&lon2);
  printf("LLXY (%f,%f) (%f,%f) (%f,%f)\n",lat1,lon1,x1,y1,lat2,lon2);
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

rc = mkGeoid(&stcprm2,AE,6367.470,0.); /*Sphere according to GRIB*/
cnflat2 = eqvlat(&stcprm2,40.,60.);
printf("%f ",cnflat2);
rc = stlmbr(&stcprm2,cnflat2,-90.);
printf("%lf \n",cgszll(&stcprm2,cnflat2,-90.));
printf("%lf %lf \n", cgszll(&stcprm2,40.,-90.),cgszll(&stcprm2,60.,-90.));

printf ( "Polar Stereographic, Gridsize %f at %f N\n",refgrid,reflat);

stlmbr(&stcprm,90.,-90.);stlmbr(&stcprm2,90.,-90.);
stcm1p(&stcprm ,33.,33.,90.,0.,  reflat,-90.,refgrid,0.);
stcm1p(&stcprm2,33.,33.,90.,0.,  reflat,-90.,refgrid,0.);

for (x=1.;x<=33.;x+=1.) {
  y=x;
  cxy2ll(&stcprm, x,y, &xlat,&ylong);
  gsza = cgszxy(&stcprm, x,y);
  gszb = cgszll(&stcprm, xlat,ylong);
  printf(" %2.0f,%2.0f %8.3f %8.3f %8.3f ",x,y,xlat,gsza,gszb);

  cxy2ll(&stcprm2, x,y , &xlat,&ylong);
  gsza = cgszxy(&stcprm2, x,y);
  gszb = cgszll(&stcprm2, xlat,ylong);
  printf(" %8.3f %8.3f %8.3f\n",xlat,gsza,gszb);
}
#ifdef N_HOLD
getc(stdin);
#endif
return 0;
}
