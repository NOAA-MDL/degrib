/*****************************************************************************
 * isNewLayout() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Determines whether a new time layout is needed. If so, numCurrenLayouts is
 *  incremented.
 *    
 * ARGUMENTS
 *         newLayout = Contains the current elements layout info (period, 
 *                     numRows, first startTime), in order to determine if this
 *                     is new information or a previous time layout with this 
 *                     information has already been formatted. (Input)
 *    numLayoutSoFar = The total number of time layouts that have been created 
 *                     so far. (Input)
 *  numCurrentLayout = Number of the layout we are currently processing. (Input)
 * f_finalTimeLayout = Flag denoting if this is the last time layout being 
 *                     processed. Used to free up the static 
 *                     timeLayoutDefinitions array. (Input)
 *                  
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
int isNewLayout(layouts newLayout, size_t * numLayoutSoFar,
                uChar * numCurrentLayout, int f_finalTimeLayout)
{
   int i;
   static layouts *timeLayoutDefinitions; /* An array holding all the past
                                           * time layouts.  They should all
                                           * be unique. */

   /* Check to see if the static array can be freed (the final time layout
    * has been formatted). 
    */
   if (!f_finalTimeLayout)
   {
      /* If this is the first time layout, we just simply add it to array
       * timeLayoutDefintions and return indicating a new time layout. 
       */
      if (*numLayoutSoFar == 1)
      {
         timeLayoutDefinitions = calloc(1, sizeof(layouts));

         timeLayoutDefinitions[0].period = newLayout.period;
         timeLayoutDefinitions[0].numRows = newLayout.numRows;
         strcpy(timeLayoutDefinitions[0].fmtdStartTime,
                newLayout.fmtdStartTime);
         *numCurrentLayout = 1;
      }

      /* If not the first one, check existing layouts for one that
       * corresponds to this data.  If we find one, let the calling program
       * know that a time layout already exists and which one it is. 
       */
      for (i = 0; i < *numLayoutSoFar - 1; i++)
      {
         if ((timeLayoutDefinitions[i].period == newLayout.period) &&
             (timeLayoutDefinitions[i].numRows == newLayout.numRows) &&
             (strcmp(timeLayoutDefinitions[i].fmtdStartTime,
                     newLayout.fmtdStartTime) == 0))
         {
            *numCurrentLayout = i + 1;
            return 0;
         }
      }

      /* Since we didn't find a pre-existing time layout, we create a new one
       * and let calling routine know that we created a new layout. 
       */
      timeLayoutDefinitions = realloc(timeLayoutDefinitions, (*numLayoutSoFar)
                                      * sizeof(layouts));

      timeLayoutDefinitions[*numLayoutSoFar - 1].period = newLayout.period;
      timeLayoutDefinitions[*numLayoutSoFar - 1].numRows = newLayout.numRows;
      strcpy(timeLayoutDefinitions[*numLayoutSoFar - 1].fmtdStartTime,
             newLayout.fmtdStartTime);
      *numCurrentLayout = *numLayoutSoFar;

      return 1;
   }
   else
   {
      free(timeLayoutDefinitions);
      return 0;
   }
}
