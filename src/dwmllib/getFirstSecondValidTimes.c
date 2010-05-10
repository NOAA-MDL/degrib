/*****************************************************************************
 * getFirstSecondValidTimes() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Routine finds the first validTime and second validTime (if exists) we're
 *   interested in, for an element. 
 *   
 * ARGUMENTS
 *  *firstValidTime = Returned first validTime of element. (Output)
 * *secondValidtime = Returned second validTime of element. (Output)
 *            match = Pointer to the structure of element matches returned from
 *                    grid probe. (Input)  
 *         numMatch = The total number of matches (all sectors) returned by 
 *                    degrib. (Input)
 *         ndfdEnum = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *         startNum = First index in match structure an individual point's data
 *                    matches can be found. (Input)
 *           endNum = Last index in match structure an individual point's data
 *                    matches can be found. (Input)
 *          numRows = Number of all data rows (values) for an element being 
 *                    formatted. (Input)
 * numRowsSkippedBeg = Number of rows skipped at beginning of time duration if
 *                     user shortened time interval data was chosen for. 
 *                     (Input)
 * numRowsSkippedEnd = Number of rows skipped at end of time duration if user
 *                     shortened time interval data was chosen for. (Input)
 *        
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  11/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void getFirstSecondValidTimes(double *firstValidTime, double *secondValidTime,
		              genMatchType *match, size_t numMatch, 
                              uChar ndfdEnum, int startNum, int endNum, 
                              int numRows, int numRowsSkippedBeg, 
                              int numRowsSkippedEnd)
{
   int i; /* Counter thru match structure. */
   int elemCount = 0; /* Counter tracking different elements thru match 
                       * structure. */

   for (i = (startNum+1); i < (endNum+1); i++)
   {
      if (match[i - 1].elem.ndfdEnum == ndfdEnum)
      {
	 if (numRows-numRowsSkippedBeg-numRowsSkippedEnd != 1)
         {
            elemCount++;
            if (match[i].elem.ndfdEnum == ndfdEnum)

            {
               *firstValidTime = match[(i-elemCount)+numRowsSkippedBeg].validTime;
               *secondValidTime = match[(i - (elemCount - 1))+numRowsSkippedBeg].validTime;
               break;
            }
         }
         else if (numRows-numRowsSkippedBeg-numRowsSkippedEnd == 1)
         {
            *firstValidTime = match[(i - 1) - elemCount].validTime;
            break;
         }

         if ((i == numMatch - 1) && (match[i - 1].elem.ndfdEnum == ndfdEnum))
         {
            *firstValidTime = match[i - elemCount].validTime;
            *secondValidTime = match[i - (elemCount - 1)].validTime;
         }
      }
   }

   return;
}
