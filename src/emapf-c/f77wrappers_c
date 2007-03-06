#include "cmapf.h"

typedef float * REAL4;
typedef double * REAL8;
typedef float REAL4v;
typedef double REAL8v;
typedef int INT4v;

#define EQVLAT F77_FUNC(eqvlat,EQVLAT)
REAL4v EQVLAT(maparam * stcprm,REAL4 lat1,REAL4 lat2) {
  return eqvlat(stcprm, * lat1, * lat2) ;
}


#define SETGEOID F77_FUNC(setgeoid,SETGEOID)
int SETGEOID(maparam * stcprm,REAL4 arad,REAL4 brad) {
  return mkGeoid(stcprm, AB, * arad, * brad);
}
int useGeoid (maparam * stcprm,char * name);

#define CGSZLL F77_FUNC(cgszll,CGSZLL)
REAL4v CGSZLL(maparam * stcprm,REAL4 lat,REAL4 longit) {
  return cgszll(stcprm, * lat, * longit) ;
}

#define CGSZXY F77_FUNC(cgszxy,CGSZXY)
REAL4v CGSZXY(maparam * stcprm,REAL4 x,REAL4 y) {
  return cgszxy(stcprm, *x, *y) ;
}

#define STLMBR F77_FUNC(stlmbr,STLMBR)
INT4v STLMBR(maparam * stcprm,REAL4 tnglat,REAL4 reflon) {
  return stlmbr(stcprm,* tnglat,* reflon) ;
}

#define STCM2P F77_FUNC(stcm2p,STCM2P)
INT4v STCM2P(maparam * stcprm,
            REAL4 x1, REAL4 y1, REAL4 xlat1, REAL4 xlong1,
            REAL4 x2, REAL4 y2, REAL4 xlat2, REAL4 xlong2) {
return stcm2p(stcprm,
            * x1, * y1, * xlat1, * xlong1,
            * x2, * y2, * xlat2, * xlong2) ;
}

#define STCM1P F77_FUNC(stcm1p,STCM1P)
INT4v STCM1P(maparam * stcprm,
            REAL4 x1, REAL4 y1, REAL4 xlat1, REAL4 xlong1,
            REAL4 xlatg, REAL4 xlong,REAL4 gridsz, REAL4 orient) {
return stcm1p(stcprm,
            * x1, * y1, * xlat1, * xlong1,
            * xlatg, * xlong,* gridsz, * orient) ;
}

#define CLL2XY F77_FUNC(cll2xy,CLL2XY)
INT4v CLL2XY(maparam * stcprm,REAL4 lat,REAL4 longit, REAL4 x,REAL4 y) {
double xs,ys;
  cll2xy(stcprm, *lat, *longit, &xs, &ys) ;
  *x = xs; *y = ys;
  return 0;
}

#define CCRVLL F77_FUNC(ccrvll,CCRVLL)
INT4v CCRVLL(maparam * stcprm,REAL4 lat, REAL4 longit, REAL4 gx, REAL4 gy) {
double gxs,gys;
  ccrvll(stcprm, * lat, * longit, &gxs, &gys) ;
  *gx = gxs; *gy = gys;
  return 0;
}

#define CXY2LL F77_FUNC(cxy2ll,CXY2LL)
INT4v CXY2LL(maparam * stcprm,REAL4 x, REAL4 y, REAL4 lat, REAL4 longit) {
double lats,longs;
  cxy2ll(stcprm, * x, * y, &lats, &longs) ;
  *lat = lats; *longit = longs;
  return 0;
}

#define CCRVXY  F77_FUNC(ccrvxy,CCRVXY)
INT4v CCRVXY(maparam * stcprm,REAL4 x, REAL4 y, REAL4 gx, REAL4 gy) {
double gxs,gys;
  ccrvxy(stcprm,* x, * y, &gxs, &gys);
  *gx = gxs; *gy = gys;
  return 0;
}

#define CC2GLL F77_FUNC(cc2gll,CC2GLL)
INT4v CC2GLL(maparam * stcprm,REAL4 lat,REAL4 longit,
            REAL4 ue,REAL4 vn, REAL4 ug,REAL4 vg) {
double ugs,vgs;
  cc2gll(stcprm,* lat,* longit, * ue,* vn, &ugs, &vgs) ;
  *ug = ugs;*vg = vgs;
  return 0;
}

#define CG2CLL F77_FUNC(cg2cll,CG2CLL)
INT4v CG2CLL(maparam * stcprm,REAL4 lat,REAL4 longit,
            REAL4 ug,REAL4 vg, REAL4 ue,REAL4 vn) {
double ues,vns;
  cg2cll(stcprm,* lat,* longit, * ug,* vg, &ues, &vns);
  *ue = ues; *vn = vns;
  return 0;
}

#define CC2GXY F77_FUNC(cc2gxy,CC2GXY)
INT4v CC2GXY(maparam * stcprm,REAL4 x,REAL4 y,
            REAL4 ue,REAL4 vn, REAL4 ug,REAL4 vg) {
double ugs,vgs;
  cc2gxy(stcprm,* x,* y, * ue,* vn, &ugs, &vgs) ;
  *ug = ugs, *vg = vgs;
  return 0;
}

#define CG2CXY F77_FUNC(cg2cxy,CG2CXY)
INT4v CG2CXY(maparam * stcprm,REAL4 x,REAL4 y,
            REAL4 ug,REAL4 vg, REAL4 ue,REAL4 vn) {
double ues,vns;
  cg2cxy(stcprm,* x,* y, * ug,* vg, &ues, &vns) ;
  *ue = ues, *vn = vns;
  return 0;
}

#define CW2GLL F77_FUNC(cw2gll,CW2GLL)
INT4v CW2GLL(maparam * stcprm,REAL4 lat,REAL4 longit,
            REAL4 ue,REAL4 vn, REAL4 ug,REAL4 vg) {
double ugs,vgs;
  cw2gll(stcprm,* lat,* longit, * ue,* vn, &ugs, &vgs) ;
  *ug = ugs, *vg = vgs;
  return 0;
}

#define CG2WLL F77_FUNC(cg2wll,CG2WLL)
INT4v CG2WLL(maparam * stcprm,REAL4 lat,REAL4 longit,
            REAL4 ug,REAL4 vg, REAL4 ue,REAL4 vn) {
double ues,vns;
  cg2wll(stcprm,* lat,* longit, * ug,* vg, &ues, &vns) ;
  *ue = ues, *vn = vns;
  return 0;
}

#define CW2GXY F77_FUNC(cw2gxy,CW2GXY)
INT4v CW2GXY(maparam * stcprm,REAL4 x,REAL4 y,
            REAL4 ue,REAL4 vn, REAL4 ug,REAL4 vg) {
double ugs,vgs;
  cw2gxy(stcprm,* x,* y, * ue,* vn, &ugs, &vgs) ;
  *ug = ugs, *vg = vgs;
  return 0;
}

#define CG2WXY F77_FUNC(cg2wxy,CG2WXY)
INT4v CG2WXY(maparam * stcprm,REAL4 x,REAL4 y,
            REAL4 ug,REAL4 vg, REAL4 ue,REAL4 vn) {
double ues,vns;
  cg2wxy(stcprm,* x,* y, * ug,* vg, &ues, &vns) ;
  *ue = ues, *vn = vns;
  return 0;
}

#define CPOLLL F77_FUNC(cpolll,CPOLLL)
INT4v CPOLLL(maparam * stcprm,REAL4 lat, REAL4 longit,
                REAL4 enx,REAL4 eny, REAL4 enz) {
double enxs,enys,enzs;
  cpolll(stcprm,* lat, * longit, & enxs,&enys, &enzs) ;
  *enx = enxs; *eny = enys; *enz = enzs;
  return 0;
}

#define CPOLXY F77_FUNC(cpolxy,CPOLXY)
INT4v CPOLXY(maparam * stcprm,REAL4 x, REAL4 y,
                REAL4 enx,REAL4 eny, REAL4 enz) {
double enxs,enys,enzs;
  cpolxy(stcprm,* x, * y, & enxs,&enys, &enzs) ;
  *enx = enxs; *eny = enys; *enz = enzs;
  return 0;
}
