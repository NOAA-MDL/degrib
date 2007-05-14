/******************************************************************************
 * spreadPOPsToWxTimes() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Copies POP12 values to an array with indexes corresponding to weather 
 *   times. Specifically, this routine populates an array with POP12 values.
 *   The array has elements valid at times corresponding to the weather times.  
 *   This allows the user to increment through the weather values and 
 *   know what the POP12 value is for the 12 hour period containing a
 *   particular weather value which occur every 3 or 6 hours. 
 * 
 * ARGUMENTS
 *                numRowsPOP = The number of data rows for POP12hr to process  
 *                             for this point. (Input)
 *                 numRowsWX = The number of data rows for weather to process  
 *                             and format for this point. (Input)
 *                   popInfo = Structure holding 12 hourly POP data and 
 *                             time info from the match structure. Used in 
 *                             derivation of icons. (Input)
 *         POP12SpreadToPOP3 = Array containing the PoP12 values spread over 
 *                             all the weather times. (Output)
 *                    wxInfo = Weather data taken from the match structure. 
 *                             (Input)
 *                                         
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2006  Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void spreadPOPsToWxTimes(int *POP12SpreadToPOP3, WX *wxInfo, int numRowsWX,
                         elem_def *popInfo, int numRowsPOP)
{
   int numSec12Hours = 43200; /* Num seconds in 12 hours. */
   int weatherStartIndex = 0; /* Index indicating which weather row is
                               * first convered by a POP12 value.
                               */
   int weatherEndIndex;  /* Index denoting which weather row from the end of 
                          * the array is first convered by a POP12 value.
			  */
   int foundGoodPopRow = 0; /* Denotes wether a Pop row was found with a valid
			     * time that contained a weather valid time.
			     */
   int popStartIndex = 0; /* Index indicating which POP12 value first covers a
                           * weather value's valid time.
                           */
   int popIndex; /* Index thru POP12 element. */
   int popRow; /* Index denoting which row of POP12 array is being processed. */
   int wxRow; /* Index denoting which row of weather array is being processed. */
   int row; /* Index denoting row processed. */
	   
   /*  Loop over all weather valid times and find the first one that is 
    *  contained in the first POP12 valid time.
    */  
   for (popRow = 0; popRow < numRowsPOP; popRow++)
   {
      for (wxRow = 0; wxRow < numRowsWX; wxRow++)
      {
	 if ((wxInfo[wxRow].validTime >= 
             (popInfo[popRow].validTime - numSec12Hours)) &&
	     (wxInfo[wxRow].validTime <= popInfo[popRow].validTime))
         {
	    weatherStartIndex = wxRow;
	    foundGoodPopRow = 1;
	    break;
	 }
      }

      /*  If we found a PoP whose valid time contains one of the weather valid 
       *  times, then we indicate which PoP row it was in and exit the search 
       *  loop. Otherwise, we go to the next PoP valid time and keep looking.
       */
      if (foundGoodPopRow)
      {
         popStartIndex = popRow;
	 break; 
      }
   }
   
   /*  Loop over all weather valid times, starting from the end and find the 
    *  first one that is contained in the last POP12 valid time.
    */
   weatherEndIndex = numRowsWX-1;
   for (row = numRowsWX-1; row > 0; row--)
   {
      if (wxInfo[row].validTime <= popInfo[numRowsPOP-1].validTime)
      {
         weatherEndIndex = row;
         break;
      }
   } 
      
   /*  If by some chance one or more of the weather valid times are not during
    *  a POP12 valid time then set the POP12SpreadToPOP3 value to -1.
    */
   if (weatherStartIndex > 1)
      for (row = 0; row < weatherStartIndex; row++)
         POP12SpreadToPOP3[row] = -1;

   if (weatherEndIndex < numRowsWX-1)
      for (row = numRowsWX-1; row > weatherEndIndex; row--)
         POP12SpreadToPOP3[row] = -1;

   /*  Loop over all the weather valid times and find the POP12 value 
    *  that corresponds to that valid time (POP12 --> POP3 or POP6).
    */
   popIndex = popStartIndex;
   for (row = weatherStartIndex; row <= weatherEndIndex; row++)
   {
      if (wxInfo[row].validTime <= popInfo[popIndex].validTime)
         POP12SpreadToPOP3[row] = popInfo[popIndex].data;
      else
      {
         POP12SpreadToPOP3[row] = popInfo[popIndex+1].data;
	 popIndex++;
      }
   }
   
   return;
}
