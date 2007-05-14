/******************************************************************************
 * roundPopNearestTen() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   This routine rounds POP12hr integers to a nearest value of 10 (i.e. 17 
 *   rounded to nearest 10 = 20).
 *   
 * ARGUMENTS
 *   num = POP12hr Integer between 0 and 100 to be rounded to the nearest 10. 
 *         (Input / Ouput).
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int num
 *
 * HISTORY
 *   9/2006  Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
int roundPopNearestTen(int num) 
{
   
   if (num < 10)
   {
      if (num >= (10 / 2))
         return 10;
      else
	 return 0;
   }
   else
      return (num % 10 >= (10 / 2) 
              ? (floor(num / 10) * 10) + 10
	      : floor(num / 10) * 10);
}
