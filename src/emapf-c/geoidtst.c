#include <stdlib.h>
#include <stdio.h>
#include "cmapf.h"

int main (int argc,char ** argv) {
maparam stcprm;
int result,nGeoids,k;
GeoidData * gdata;
  result = useGeoid(&stcprm,"dummyGeoid");
  printf("%d\n",result);
  lsGeoid();
  result = useGeoid(&stcprm,"wgs84");
  printf("%d\n",result);
  result = stlmbr(&stcprm,45.,70.);
  printf("%d\n",result);
  printf("%f %f %f \n",stcprm.arad,stcprm.brad,stcprm.eccen);
  stcm2p(&stcprm, 1.,1., 30.,90., 75,1,30.,50.);
  stcm2p(&stcprm, 1.,1., 30.,90., 75,1,30.,50.);
  nGeoids = infoGeoids(&gdata);
  printf("Number of geoids is %d\n",nGeoids );
  for (k=0;k<nGeoids;k++) {
    printf("geoid %d is %s\n",k,gdata[k].name);
  }
#ifdef N_HOLD
  fgetc(stdin);
#endif
  return 0;
}
