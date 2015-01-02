#ifndef COMPLEX_H
#define COMPLEX_H

/* _IS_C99_ is set, used here, and then unset all in this header. */
#undef _IS_C99_
#ifdef __STDC_VERSION__
   #if (__STDC_VERSION__ == 199901L)
      /* Is c99, so complex.h should exist */
      #define _IS_C99_
   #endif
#endif

#ifdef _IS_C99_
  #include <complex.h>
  #ifndef COMPLEX_TYPE
  #define COMPLEX_TYPE
/*  typedef _Complex double myComplex; */
  typedef complex double myComplex;
  #endif
#else
  #ifndef COMPLEX_TYPE
  #define COMPLEX_TYPE
  typedef struct {
     double x, y;
  } myComplex;
  #endif
#endif

myComplex myCset(double x, double y);
void myCprint(myComplex z);

#ifdef _IS_C99_
  #define myCreal creal
  #define myCimag cimag
  #define myCadd(a, b) ((a) + (b))
  #define myCsub(a, b) ((a) - (b))
  #define myCmul(a, b) ((a) * (b))
  #define myCmul_Real(a, b) ((a) * (b))
  #define myCinv(a) (1 / (a))
  #define myCexp cexp
  #define myClog clog
  #define myCsqrt csqrt

#else
  #define myCreal my_Creal
  #define myCimag my_Cimag
  #define myCadd my_Cadd
  #define myCsub my_Csub
  #define myCmul my_Cmul
  #define myCmul_Real my_Cmul_Real
  #define myCinv my_Cinv
  #define myCexp my_Cexp
  #define myClog my_Clog
  #define myCsqrt my_Csqrt

  double my_Creal(myComplex z);
  double my_Cimag(myComplex z);
  myComplex my_Cadd(myComplex z1, myComplex z2);
  myComplex my_Csub(myComplex z1, myComplex z2);
  myComplex my_Cmul(myComplex z1, myComplex z2);
  myComplex my_Cmul_Real(myComplex z, double a);
  myComplex my_Cinv(myComplex z);
  myComplex my_Cexp(myComplex z);
  myComplex my_Clog(myComplex z);
  myComplex my_Csqrt(myComplex z);
#endif
#undef _IS_C99_

/* FYI: C99 implementation found in mingw include <complex.h>
double __MINGW_ATTRIB_CONST creal (double _Complex);
double __MINGW_ATTRIB_CONST cimag (double _Complex);
double __MINGW_ATTRIB_CONST carg (double _Complex);
double __MINGW_ATTRIB_CONST cabs (double _Complex);
double _Complex __MINGW_ATTRIB_CONST conj (double _Complex);
double _Complex  cacos (double _Complex);
double _Complex  casin (double _Complex);
double _Complex  catan (double _Complex);
double _Complex  ccos (double _Complex);
double _Complex  csin (double _Complex);
double _Complex  ctan (double _Complex);
double _Complex  cacosh (double _Complex);
double _Complex  casinh (double _Complex);
double _Complex  catanh (double _Complex);
double _Complex  ccosh (double _Complex);
double _Complex  csinh (double _Complex);
double _Complex  ctanh (double _Complex);
double _Complex  cexp (double _Complex);
double _Complex  clog (double _Complex);
double _Complex  cpow (double _Complex, double _Complex);
double _Complex  csqrt (double _Complex);
double _Complex __MINGW_ATTRIB_CONST cproj (double _Complex);
*/
#endif
