/******************************************************************************
 * XMLmatchCompare() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Compare match A with B. First compares the sector the match was found in,
 *  then compares "NDFDtype" (or element) enum type, then compares the 
 *  validTime. Routine then orders the entire match structure in this manner.
 *
 * ARGUMENTS
 *         A = First NDFD match for comparison. (Input)
 *         B = Second NDFD match for comparison. (Input)
 *                                         
 * FILES/DATABASES: None
 *
 * RETURNS: int (returns a -1, 0, or 1 depending on comparison).
 *
 * HISTORY
 *   5/2007  Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
int XMLmatchCompare (const void *A, const void *B)
{
   const genMatchType *a = (genMatchType *) A;
   const genMatchType *b = (genMatchType *) B;
   double f_sectA = a->f_sector; /* Temp placeholder */
   double f_sectB = b->f_sector; /* Temp placeholder */
   
   /* Get the match structure in the sector enum order
    * 1 - conus2_5, 0 - conus5, 6 - nhemi, 2 - peurtori, 
    * 3 - hawaii, 7 - npacocn, 4 - guam, 5 - alaska, 8 - undefined 
    * for point processing in XMLParse. Take note if 
    * there are any elements from Sector 6 or Sector 7
    * (tropical winds are found in these sectors).
    */

   /* Dummy up sector conus2_5 so it is the first sector. (It comes
    * in as the second enumerated sector.)
    */ 
   if (f_sectA == 1) 
      f_sectA = -1; 
   if (f_sectB == 1) 
      f_sectB = -1; 

   /* Dummy up sector nhemi so it falls in between the two sectors, conus5 and 
    * puertori, that it holds Tropical Wind Threshold data for.  
    */
   if (f_sectA == 6)
      f_sectA = 1.5;
   if (f_sectB == 6)
      f_sectB = 1.5;

   /* Dummy up sector npacocn so it falls in between the two sectors, hawaii and 
    * guam, that it holds Tropical Wind Threshold data for.  
    */
   if (f_sectA == 7)
      f_sectA = 3.5;
   if (f_sectB == 7)
      f_sectB = 3.5;

   /* Sort */
   if (f_sectA < f_sectB)
      return -1;
   if (f_sectA > f_sectB)
      return 1;
   if (a->elem.ndfdEnum < b->elem.ndfdEnum)
      return -1;
   if (a->elem.ndfdEnum > b->elem.ndfdEnum)
      return 1;
   if (a->validTime < b->validTime)
      return -1;
   if (a->validTime > b->validTime)
      return 1;
   return 0;
}
