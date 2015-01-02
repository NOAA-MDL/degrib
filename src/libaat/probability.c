/*****************************************************************************
 * probability.c
 *
 * DESCRIPTION
 *    This file implements some probabilty functions.
 *
 * HISTORY
 *  4/2007 Arthur A Taylor (MDL): Added to libaat
 *  6/2007 Tom R Shaw (OITC): Fixed MacOSX compile error
 *
 * NOTES
 ****************************************************************************/
#include <stdio.h>
#include <math.h>
#include "libaat.h"

/* In __STRICT_ANSI__, neither erf(), nor erfc() are provided in math.h, so
 * one can either implement them, or put a #undef __STRICT_ANSI__ around the
 * #include <math.h> and assume that the compiler has them.  I chose to
 * provide them. */
#undef _PROVIDE_ERF
#ifdef __STRICT_ANSI__
#define _PROVIDE_ERF
#else
#ifdef NEED_ERF
#define _PROVIDE_ERF
#endif
#endif

/* Added to support MacOSX standard config with  erf(), nor erfc() being
 * provided in math.h. TRS */
#ifdef __APPLE_CC__
#undef _PROVIDE_ERF
#endif

#ifdef _PROVIDE_ERF
/* Prototype for erfc (since erf() and erfc() reference each other */
static double erfc(double x);

/*****************************************************************************
 * err() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To compute the Gaussian error function (see:
 * http://mathworld.wolfram.com/Erf.html).
 * erf(x) = 2/sqrt(pi)*integral(exp(-t^2),t,0,x)
 *        = 1 - erfc(x)
 *    Using the Maclaurin series we get:
 *        = 2/sqrt(pi)*[x - x^3/3 + x^5/5*2! - x^7/7*3! + ...]
 *
 * ARGUMENTS
 * x = The desired value to compute erf(x) of (Input)
 *
 * RETURNS: double
 *
 * HISTORY
 *  1/2004 Steve Strand (see:
 *         http://www.digitalmars.com/archives/cplusplus/3634.html
 *  4/2007 AAT (MDL) Commented.
 *
 * NOTES
 ****************************************************************************/
/* Calculate 12 significant figures.  Can adjust some, but don't choose > 15
 * figures (assuming 52 bit mantissa in a double). */
#define REL_ERROR 1E-12
#ifndef M_2_SQRTPI
#define M_2_SQRTPI 1.12837916709551257390
#endif
static double erf(double x)
{
   double sum = x;      /* The current sum in the Maclaurin series */
   double term = x;     /* The next term in the series. */
   double xsqr;         /* x^2, used to avoid repeated multiplications. */
   int j = 1;           /* The term we're working on */

   /* Use continued fraction when fabs(x) > 2.2 */
   if (fabs(x) > 2.2) {
      return 1.0 - erfc(x);
   }
   xsqr = x * x;
   do {
      term *= xsqr / (double)j;
      sum -= term / (double)(2 * j + 1);
      ++j;
      term *= xsqr / (double)j;
      sum += term / (double)(2 * j + 1);
      ++j;
   } while (fabs(term / sum) > REL_ERROR);
   return M_2_SQRTPI * sum;
}

/*****************************************************************************
 * erfc() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To compute the "complementary" error function (see:
 * http://mathworld.wolfram.com/Erfc.html).
 * erfc(x) = 2/sqrt(pi)*integral(exp(-t^2),t,x,inf)
 *     = 1 - erf(x)
 * (http://functions.wolfram.com/GammaBetaErf/Erfc/10/01/0001/)
 *     = exp(-x^2)/sqrt(pi) * [1/x+ (1/2)/x+ (2/2)/x+ (3/2)/x+ (4/2)/x+ ...]
 * (expression inside [] is a continued fraction so '+' means add to
 *  denominator)
 *
 * Solving the [] with more and more terms reveals:
 * i=1) 1/x,
 * i=2) x/(x^2 + 1/2)),
 * i=3) (1 + x^2)/(x + x^3 + 1/2x),
 * Algorithm claims fraction changes by (numerator:n(i) / denominator:d(i)):
 *     n(i) = n(i-2) * C + n(i-1) * x
 *     d(i) = d(i-2) * C + d(i-1) * x
 *  Where C = .5^(i-3)
 *
 * ARGUMENTS
 * x = The desired value to compute erfc(x) of (Input)
 *
 * RETURNS: double
 *
 * HISTORY
 *  1/2004 Steve Strand (see:
 *         http://www.digitalmars.com/archives/cplusplus/3634.html
 *  4/2007 AAT (MDL) Commented.
 *
 * NOTES
 ****************************************************************************/
#define M_1_SQRTPI 0.56418958354775628695
static double erfc(double x)
{
   double a = 1;        /* Earlier numerator. */
   double b = x;        /* Later numerator. */
   double c = x;        /* Earlier denominator. */
   double d = x * x + 0.5; /* Later denominator. */
   double q1;           /* The earlier aprox. (starts with 1/x (ignored)) */
   double q2 = b / d;   /* The later aprox. (starts with x/(x^2 + 1/2)) */
   double n = 1.0;
   double t;

   /* Use series when fabs(x) < 2.2 */
   if (fabs(x) < 2.2) {
      return 1.0 - erf(x);
   }
   /* continued fraction only valid for x > 0 */
   if (x < 0) {
      return 2.0 - erfc(-x);
   }
   do {
      t = a * n + b * x;
      a = b;
      b = t;
      t = c * n + d * x;
      c = d;
      d = t;
      n += 0.5;
      q1 = q2;
      q2 = b / d;
   } while (fabs(q1 - q2) / q2 > REL_ERROR);
   return M_1_SQRTPI * exp(-x * x) * q2;
}
/* end of #ifdef _PROVIDE_ERF */
#endif

/*****************************************************************************
 * normInv() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To compute the inverse to the normal cumulative distribution.  See:
 * http://home.online.no/~pjacklam/notes/invnorm/
 * http://mathworld.wolfram.com/InverseErf.html
 *
 * Note: http://en.wikipedia.org/wiki/Error_function
 *    States that the error function (erf) is essentially identical to the
 * standard "normal cumulative distribution" function, Phi(x), as they differ
 * only by scaling and translation.  Indeed:
 *    Phi(x) = 1/2 (1 + erf(x/sqrt(2))
 *
 * ARGUMENTS
 * x = The desired value to compute normInv(x) of (Input)
 *
 * RETURNS: double
 *
 * HISTORY
 *  ??     Peter J. Acklam.
 *  4/2007 AAT (MDL) Commented.
 *
 * NOTES
 ****************************************************************************/
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
static double normInv(double p)
{
   static const double A[] = {
      -3.969683028665376e+01, 2.209460984245205e+02, -2.759285104469687e+02,
      1.383577518672690e+02, -3.066479806614716e+01, 2.506628277459239e+00
   };
   static const double B[] = {
      -5.447609879822406e+01, 1.615858368580409e+02, -1.556989798598866e+02,
      6.680131188771972e+01, -1.328068155288572e+01
   };
   static const double C[] = {
      -7.784894002430293e-03, -3.223964580411365e-01, -2.400758277161838e+00,
      -2.549732539343734e+00, 4.374664141464968e+00, 2.938163982698783e+00
   };
   static const double D[] = {
      7.784695709041462e-03, 3.224671290700398e-01, 2.445134137142996e+00,
      3.754408661907416e+00
   };
   static const double p_low = 0.02425;
   static const double p_high = 0.97575; /* 1 - p_low */
   double x;            /* Temporary copy of the answer. */
   double q;            /* temp var 1 during approximation */
   double r;            /* temp var 2 during approximation */
   double e;            /* temp var 1 during refinement */
   double u;            /* temp var 2 during refinement */

   /* Rational approximation for lower region. */
   if ((p > 0) && (p < p_low)) {
      q = sqrt(-2 * log(p));
      x = ((((((C[0] * q + C[1]) * q + C[2]) * q + C[3]) * q + C[4]) *
            q + C[5]) /
           ((((D[0] * q + D[1]) * q + D[2]) * q + D[3]) * q + 1));

      /* Rational approximation for central region. */
   } else if ((p >= p_low) && (p <= p_high)) {
      q = p - 0.5;
      r = q * q;
      x = ((((((A[0] * r + A[1]) * r + A[2]) * r + A[3]) * r + A[4]) *
            r + A[5]) * q /
           (((((B[0] * r + B[1]) * r + B[2]) * r + B[3]) * r + B[4]) * r +
            1));
      /* Rational approximation for upper region. */
   } else if ((p > p_high) && (p < 1)) {
      q = sqrt(-2 * log(1 - p));
      x = -((((((C[0] * q + C[1]) * q + C[2]) * q + C[3]) * q + C[4]) *
             q + C[5]) /
            ((((D[0] * q + D[1]) * q + D[2]) * q + D[3]) * q + 1));
   } else {
      myWarn_Err1Arg("p is out of the valid range\n");
      return 0;
   }
   /* Refine the answer using Halley's rational method (3rd order) */
   e = 0.5 * erfc(-x / sqrt(2)) - p;
   u = e * sqrt(2 * M_PI) * exp(x * x / 2);
   x = x - u / (1 + x * u / 2);
   return x;
}

/*****************************************************************************
 * probNormDist() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To compute the normal cumulative distribution.  See:
 * http://mathworld.wolfram.com/NormalDistribution.html (equation 3)
 * Returns the probabilty that a standard normal distribution assumes a value
 * in the interval of [0,x] (assuming a mean of 0, and standard dev of 1)
 *    Phi(x) = 1/2 (1 + erf(x/sqrt(2))
 *
 * ARGUMENTS
 * x = The desired value to the normal distribution of (Input)
 *
 * RETURNS: double
 *
 * HISTORY
 *  4/2007 AAT (MDL) Commented.
 *
 * NOTES
 ****************************************************************************/
double probNormDist(double x)
{
   return (erf(x / sqrt(2)) / 2.);
}

/*****************************************************************************
 * probNormBound() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To compute "right" so that between left and right, there is "prob"
 * probability based on the normal distribution.
 *
 * ARGUMENTS
 * prob = The desired probability amount [0.0, 0.5] (Input)
 * left = The left edge of the area in question (Input)
 *
 * RETURNS: double
 *
 * HISTORY
 *  4/2007 AAT (MDL) Commented.
 *
 * NOTES
 ****************************************************************************/
double probNormBound(double prob, double left)
{
   double lftProb;      /* The amount of prob to associate from 0 to left */

   if ((left < 0) || (prob > .5) || (prob < 0)) {
      myWarn_Err1Arg("need: left >= 0, 0 <= prob <= .5\n");
      return .5;
   }
   lftProb = probNormDist(left);
   if (prob + lftProb > .5) {
      myWarn_Err1Arg("'left' is too far right for requested prob.\n");
      return .5;
   }
   prob += lftProb;
   return normInv(prob + .5);
}

/*****************************************************************************
 * main() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   To test the probability module functions.
 *
 * ARGUMENTS
 * argc = The number of arguments on the command line. (Input)
 * argv = The arguments on the command line. (Input)
 *
 * RETURNS: int
 *
 * HISTORY
 *  4/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
#ifdef DEBUG_PROB
#include <string.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
   double prob;
   double left;
   double x;
   double ans;

   if ((argc != 3) && (argc != 4)) {
      printf("usage %s <normBound> <prob> <left>\n", argv[0]);
      printf("usage %s <normDist> <x>\n", argv[0]);
      return 0;
   }
   if (strcmp("normBound", argv[1]) == 0) {
      if (argc != 4) {
         printf("usage %s <normBound> <prob> <left>\n", argv[0]);
         return 0;
      }
      prob = atof(argv[2]);
      left = atof(argv[3]);
      printf("prob = %f, left = %f\n", prob, left);
      ans = probNormBound(prob, left);
      printf("Answer = %f\n", ans);
   } else if (strcmp("normDist", argv[1]) == 0) {
      if (argc != 3) {
         printf("usage %s <normDist> <x>\n", argv[0]);
         return 0;
      }
      x = atof(argv[2]);
      ans = probNormDist(x);
      printf("Answer = %f\n", ans);
   } else {
      printf("usage %s <normBound> <prob> <left>\n", argv[0]);
      printf("usage %s <normDist> <x>\n", argv[0]);
      return 0;
   }
   return 0;
}
#endif
