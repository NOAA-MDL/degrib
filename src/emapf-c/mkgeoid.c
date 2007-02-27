#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cmapf.h"

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *A, const char *B);
#endif

GeoidData GData[] = {
#include "geoids.h"
};
int _GeoidCount = sizeof(GData)/sizeof(GData[0]);

int infoGeoids(GeoidData ** geoids) {
*geoids = GData;
return _GeoidCount;
}

int mkGeoid(maparam * stcprm,GspecType t,double arg1,double arg2){
#define ECCEN stcprm->eccen
#define ARAD stcprm->arad
#define BRAD stcprm->brad

  switch(t) {
  case AF:
  /*arg1 = Equatorial radius of Earth, arg2 = flattening,
    i.e. arg2 =  (arad-brad)/arad */
    if (arg1 <= 0.) return 1;
    if ( (arg2 < 0.) || (arg2 >= 1.) ) return 1;
    stcprm->arad = arg1;
    stcprm->brad = arg1 * (1. - arg2) ;
    stcprm->eccen = sqrt ( (2. - arg2 ) * arg2);
    break;
  case AE:
  /*arg1 = Equatorial radius of Earth, arg2 = eccentricity*/
    if (arg1 <= 0.) return 1;
    if (arg2 < 0.  || arg2 >=1. ) return 1;
    stcprm->arad = arg1;
    stcprm->eccen = arg2;
    stcprm->brad = arg1 * sqrt((1. - arg2)*(1. + arg2));
    break;
  case AB:
  /*arg1 = Equatorial radius of Earth, arg2 = Polar Radius*/
    if (arg2 <= 0.) return 1;
    if (arg1 < arg2) return 1;
    stcprm->arad = arg1;
    stcprm->brad = arg2;
    stcprm->eccen = sqrt((arg1 - arg2)/arg1*(arg1 + arg2)/arg1);
    break;
  case BE:
    if (arg1 <= 0.) return 1;
    if (arg2 < 0.  || arg2 >=1. ) return 1;
    stcprm->brad = arg1;
    stcprm->eccen = arg2;
    stcprm->arad = arg1 / sqrt((1. - arg2)*(1. + arg2));
    break;
  case TST:
    if (( BRAD <= 0.) || (ARAD < BRAD)) return 1;
    if (( ECCEN < 0.) || (ECCEN >= 1.) ) return 1;
    if (fabs((ARAD-BRAD)/ARAD*(ARAD+BRAD)/ARAD - ECCEN*ECCEN) > 1.e-9)
       return 1;
    return 0;
  default:
    return 1;
  }
  stcprm->gamma = 10.;
  return 0;
#undef BRAD
#undef ARAD
#undef ECCEN
}

int useGeoid(maparam * stcprm,char * name) {
int k;

  for (k=0; k<sizeof(GData)/sizeof(GData[0]); k++) {
    if (strcasecmp(name,GData[k].name) == 0) {
      mkGeoid(stcprm,GData[k].t,GData[k].arg1,GData[k].arg2);
      return 0;
    }
  }
  return 1;
}

void lsGeoid() {
int k;
  printf("Known Geoids:\n\n");
  for (k=0; k<sizeof(GData)/sizeof(GData[0]); k++) {
    printf ("%s\n",GData[k].name);
  }
}

#ifndef HAVE_STRCASECMP
#include <ctype.h>
int strcasecmp(const char *A, const char *B)  {
   int a,b;
   while (*A != '\0') {
     if (*B == '\0') return 1;
     a = tolower(*A) & 255;
     b = tolower(*B) & 255;
     if (a > b) return 1;
     if (a < b) return -1;
     A++;
     B++;
   }
   if (*B != 0) return -1;
   return 0;
}
#endif
