#include <stdlib.h>
#include <stdio.h>
#include "cmapf.h"

main (int argc,char ** argv) {
  maparam stcprm;
  int result;
  result = useGeoid(&stcprm,"dummyGeoid");
  printf("%d\n",result);
  lsGeoid();
  result = useGeoid(&stcprm,"wgs84");
  printf("%d\n",result);
  result = stlmbr(&stcprm,45.,70.);
  printf("%d\n",result);
  printf("%f %f %f \n",stcprm.arad,stcprm.brad,stcprm.eccen);
  stcm2p(&stcprm, 1.,1., 30.,90., 75,1,30.,50.);
  fgetc(stdin);
  stcm2p(&stcprm, 1.,1., 30.,90., 75,1,30.,50.);
}
