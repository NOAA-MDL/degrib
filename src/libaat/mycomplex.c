/*****************************************************************************
 * mycomplex.c
 *
 * DESCRIPTION
 *    This file contains some functions for computing complex numbers.
 *
 * HISTORY
 *  2/2007 Arthur Taylor (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
#include <stdio.h>
#include <math.h>
#include "mycomplex.h"

/* Since _IS_C99_ is set, used here, and then unset in mycomplex.h, we need
 * to also set it here. */
#ifdef __STDC_VERSION__
#if (__STDC_VERSION__ == 199901L)
      /* Is c99, so complex.h should exist */
#define _IS_C99_
#endif
#endif

/*****************************************************************************
 * myCset() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Sets a complex number to the given real and imaginary parts.
 *
 * ARGUMENTS
 * x = The real part. (Input)
 * y = The imaginary part. (Input)
 *
 * RETURNS: myComplex
 *    The resulting complex number
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
myComplex myCset(double x, double y)
{
   myComplex ans;       /* The answer to return */

#ifdef _IS_C99_
   /* http://www.gnu.org/software/libc/manual/html_node/Complex-Numbers.html
    * To construct complex numbers you need a way to indicate the imaginary
    * part of a number. There is no standard notation for an imaginary
    * floating point constant. Instead, complex.h defines two macros that can
    * be used to create complex numbers. */
   /* Macro: const float complex _Complex_I */
   ans = x + _Complex_I * y;
/*
   ans = x;
   __imag__ ans = y;
*/
#else
   ans.x = x;
   ans.y = y;
#endif
   return ans;
}

/*****************************************************************************
 * myCprint() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Prints a complex number.
 *
 * ARGUMENTS
 * z = The complex number to print. (Input)
 *
 * RETURNS: void
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
void myCprint(myComplex z)
{
#ifdef _IS_C99_
   printf("Real = %10.7f, Imag = %10.7f\n", __real__ z, __imag__ z);
#else
   printf("Real = %10.7f, Imag = %10.7f\n", z.x, z.y);
#endif
}

#ifndef _IS_C99_
/*****************************************************************************
 * myCreal (Macro for my_Creal()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Returns the real part of a complex number.
 *
 * ARGUMENTS
 * z = The complex number of interest. (Input)
 *
 * RETURNS: double
 *    Real(z)
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
double my_Creal(myComplex z)
{
   return (z.x);
}

/*****************************************************************************
 * myCimag (Macro for my_Cimag()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Returns the imaginary part of a complex number.
 *
 * ARGUMENTS
 * z = The complex number of interest. (Input)
 *
 * RETURNS: double
 *    Imag(z)
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
double my_Cimag(myComplex z)
{
   return (z.y);
}

/*****************************************************************************
 * myCadd (Macro for my_Cadd()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Adds two complex numbers.
 *
 * ARGUMENTS
 * z1 = The first complex number to add. (Input)
 * z2 = The second complex number to add. (Input)
 *
 * RETURNS: myComplex
 *    z1 + z2.
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
myComplex my_Cadd(myComplex z1, myComplex z2)
{
   z1.x += z2.x;
   z1.y += z2.y;
   return z1;
}

/*****************************************************************************
 * myCsub (Macro for my_Csub()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Subtracts two complex numbers.
 *
 * ARGUMENTS
 * z1 = The complex number to subtract from. (Input)
 * z2 = The complex number to subtract. (Input)
 *
 * RETURNS: myComplex
 *    z1 - z2.
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
myComplex my_Csub(myComplex z1, myComplex z2)
{
   z1.x -= z2.x;
   z1.y -= z2.y;
   return z1;
}

/*****************************************************************************
 * myCmul (Macro for my_Cmul()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Multiplies two complex numbers.
 *
 * ARGUMENTS
 * z1 = The first complex number to multiply. (Input)
 * z2 = The second complex number to multiply. (Input)
 *
 * RETURNS: myComplex
 *    z1 * z2.
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
myComplex my_Cmul(myComplex z1, myComplex z2)
{
   myComplex ans;       /* The answer to return */

   ans.x = z1.x * z2.x - z1.y * z2.y;
   ans.y = z1.x * z2.y + z1.y * z2.x;
   return ans;
}

/*****************************************************************************
 * myCmul_Real (Macro for my_Cmul_Real()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Multiplies a complex times a real number.
 *
 * ARGUMENTS
 * z = The complex number to multiply. (Input)
 * a = The real number to multiply. (Input)
 *
 * RETURNS: myComplex
 *    z * a.
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
myComplex my_Cmul_Real(myComplex z, double a)
{
   z.x *= a;
   z.y *= a;
   return z;
}

/*****************************************************************************
 * myCinv (Macro for my_Cinv()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    The multiplicative inverse of a complex number.
 *
 * ARGUMENTS
 * z = The complex number to take the inverse of. (Input)
 *
 * RETURNS: myComplex
 *    1 / z.
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 *    Don't want to prevent division by zero since C99 didn't.
 ****************************************************************************/
myComplex my_Cinv(myComplex z)
{
   myComplex ans;       /* The answer to return */
   double r2;           /* The magnitude (squared) of z */

   /* Don't want to prevent division by zero since C99 didn't. */
   r2 = (z.x * z.x + z.y * z.y);
   ans.x = z.x / r2;
   ans.y = -1 * z.y / r2;
   return ans;
}

/*****************************************************************************
 * myCexp (Macro for my_Cexp()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Computes e raised to a complex number.
 *
 * ARGUMENTS
 * z = The complex number to raise e to. (Input)
 *
 * RETURNS: myComplex
 *    e to the z power
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 *    Don't want to prevent division by zero since C99 didn't.
 ****************************************************************************/
myComplex my_Cexp(myComplex z)
{
   myComplex ans;       /* The answer to return */
   double e_x;          /* The exponential of the real part of z */

   e_x = exp(z.x);
   ans.x = e_x * cos(z.y);
   ans.y = e_x * sin(z.y);
   return ans;
}

/*****************************************************************************
 * myClog (Macro for my_Clog()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Computes the base e logarithm of a complex number.
 *
 * ARGUMENTS
 * z = The complex number to compute the base e logarithm of. (Input)
 *
 * RETURNS: myComplex
 *    log(e) (z)
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
myComplex my_Clog(myComplex z)
{
   myComplex ans;       /* The answer to return */

   ans.x = 0.5 * log(z.x * z.x + z.y * z.y);
   ans.y = atan2(z.y, z.x);
   return ans;
}

/*****************************************************************************
 * myCsqrt (Macro for my_Csqrt()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Computes the square root of a complex number.
 *
 * ARGUMENTS
 * z = The complex number to compute the square root of. (Input)
 *
 * RETURNS: myComplex
 *    sqrt(z)
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
myComplex my_Csqrt(myComplex z)
{
   myComplex ans;       /* The answer to return */
   double r;            /* The magnitude of z */

   r = sqrt(z.x * z.x + z.y * z.y);
   ans.x = sqrt((z.x + r) / 2.0);
   if (z.y >= 0) {
      ans.y = sqrt((r - z.x) / 2.0);
   } else {
      ans.y = -1 * sqrt((r - z.x) / 2.0);
   }
   return ans;
}
#endif

/*****************************************************************************
 * main() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Used to test the implementation of complex.
 *
 * ARGUMENTS
 * argc = The number of command line arguments (ignored). (Input)
 * argv = The command line arguments (ignored). (Input)
 *
 * RETURNS: int
 *    0
 *
 * HISTORY
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
#ifdef DEBUG_MYCOMPLEX
#include <time.h>
int main(int argc, char **argv)
{
   myComplex z1, z2;    /* Temporary variables for testing */
   myComplex z3, z4;    /* Temporary variables for testing */
   myComplex z5, z6;    /* Temporary variables for testing */
   int i;               /* index counter for testing computation time. */

#ifdef _IS_C99_
   printf("Is C99\n");
   printf("sizeof(myComplex) = %d\n", sizeof(myComplex));
#else
   printf("Is not C99\n");
   printf("sizeof(myComplex) = %d\n", sizeof(myComplex));
#endif

   z1 = myCset(0, 1);
   z2 = myCset(1, 1);
   myCprint(z1);
   printf("using Creal Cimag %f %f\n", myCreal(z1), myCimag(z1));
   myCprint(z2);
   printf("using Creal Cimag %f %f\n", myCreal(z2), myCimag(z2));

   z3 = myCadd(z1, z2);
   z3 = myCadd(z3, z2);
   z4 = myCadd(z1, myCmul_Real(z2, 2));
   printf("Testing addition:\n");
   myCprint(z3);
   myCprint(z4);
   z3 = myCsub(z3, myCmul_Real(z2, 2));
   printf("Testing subtraction:\n");
   myCprint(z3);
   myCprint(z1);
   z3 = z4;

   z5 = myCmul(z3, z4);
   z6 = myCmul(z3, myCinv(myCinv(z4)));
   printf("Testing multiplication and inverse:\n");
   myCprint(z5);
   myCprint(z6);

   z3 = myCexp(z5);
   z4 = myCexp(myClog(myCexp(z6)));
   printf("Testing Exponential:\n");
   myCprint(z3);
   myCprint(z4);

   z5 = myCsqrt(z3);
   z6 = myCsqrt(myCsqrt(myCmul(z4, z4)));
   printf("Testing square root:\n");
   myCprint(z5);
   myCprint(z6);

   printf("Testing inv(0+i0):\n");
   z3 = myCset(0, 0);
   myCprint(z3);
   z3 = myCinv(z3);
   myCprint(z3);

   printf("Starting speed test (diff between -ansi and -std=c99)\n");
   printf("Timing info. %f\n", clock() / (double)(CLOCKS_PER_SEC));
   for (i = 0; i < 200000; ++i) {
      z1 = myCset(0, 1);
      z2 = myCset(1, 1);

      z3 = myCadd(z1, z2);
      z3 = myCadd(z3, z2);
      z4 = myCadd(z1, myCmul_Real(z2, 2));
      z3 = myCsub(z3, myCmul_Real(z2, 2));
      z3 = z4;

      z5 = myCmul(z3, z4);
      z6 = myCmul(z3, myCinv(myCinv(z4)));

      z3 = myCexp(z5);
      z4 = myCexp(myClog(myCexp(z6)));

      z5 = myCsqrt(z3);
      z6 = myCsqrt(myCsqrt(myCmul(z4, z4)));

      z3 = myCset(0, 0);
      z3 = myCinv(z3);
   }
   printf("Timing info. %f\n", clock() / (double)(CLOCKS_PER_SEC));

   return 0;
}
#endif
