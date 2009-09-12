/******************************************************************************
 * hazStartEndTimes() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code gets the startTimes and endTimes for each individual hazard, if
 *  it is a summary product. It also retrieves the times that correspond to
 *  the forecast period boundaries.
 *
 * ARGUMENTS
 *    startTimes = The number of startTimes for each hazard. This is 
 *                 determined by how many forecast periods the entire 
 *                 accumulated hazard spans. (Output)
 *      endTimes = The number of endTimes for each hazard. This is 
 *                 determined by how many forecast periods the entire 
 *                 accumulated hazard spans. (Output)
 *   numFmtdRows = For DWMLgenByDay products, the number of rows to format 
 *                 per each hazard. This number depends on how many forecast 
 *                 periods the entire accumulated hazard spans.(Output)
 * consecHazRows = Array holding information about each individual hazard.
 *                 Each element contains hazard info members containing 
 *                 startTime, the endTime, the number of consecutive hours the 
 *                 hazard exists, the time of a resolution split (1hr res 
 *                 changes to 3hr resolution after 3rd day) and the string code.
 *                 (Input). 
 *   periodTimes = The times bordering each forecast period time. (Input)
 * numPeriodTimes = The number of periodTimes. (Input)
 *      TZoffset = Number of hours to add to current time to get GMT time. 
 *                 (Input)
 *  f_observeDST = Flag determining if current point observes Daylight 
 *                 Savings Time. (Input)  
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2008 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void hazStartEndTimes(char ***startTimes, char ***endTimes, int **numFmtdRows, 
                      hazInfo consecHazRows, double **periodTimes, 
                      int *numPeriodTimes, double TZoffset, sChar f_observeDST)
{
   int i;  /* Counter thru each hazard for the point. */
   int period = 1; /* Default period for each hazard hour, until 3 days out. */
   double hazStartTimeToCompareStart = 0.0; /* Beginning of a hazards first 
                                             * valid hour. */
   double hazStartTimeToCompareMid = 0.0; /* Midpoint of a hazards first 
                                           * valid hour. */
   double hazStartTimeToCompareEnd = 0.0; /* End of a hazards first valid
                                           * hour. */
   double hazEndTimeToCompareMid = 0.0; /* Midpoint of a hazards last valid 
                                         * hour. */
   double hazEndTimeToCompareEnd = 0.0; /* End of a hazards last valid hour. */
   int timeIndex = -1; /* Index tracking how many forecast periods the 
                        * accumulated hazard fall into. */
   char tempBuff[30]; /* Temp string. */
   int f_hazContinuesToNextPeriod = 0; /* Denotes when an accumulated hazard 
                                        * falls into the next forecast 
                                        * period. */
   int f_entireHazIsFound = 0; /* Denotes a hazard has ended. */

   /* Initilalize a few things. */
   *startTimes = NULL;
   *endTimes = NULL;
   **numFmtdRows = 0;

   /* Set resolution. */
   if (consecHazRows.startHour > consecHazRows.valTimeResSplit && 
       consecHazRows.valTimeResSplit > 0)
      period = 6;
   else
      period = 1;

   hazStartTimeToCompareStart = consecHazRows.startHour;
   hazStartTimeToCompareMid = consecHazRows.startHour + (((double)period * 0.5) * 3600);
   hazStartTimeToCompareEnd = consecHazRows.startHour + (((double)period) * 3600);

   if (consecHazRows.endHour > consecHazRows.valTimeResSplit && 
       consecHazRows.valTimeResSplit > 0)
      period = 6;
   else
      period = 1;

   hazEndTimeToCompareMid = consecHazRows.endHour + (((double)period * 0.5) * 3600);
   hazEndTimeToCompareEnd = consecHazRows.endHour + (((double)period) * 3600);

   for (i = 1; i < *numPeriodTimes; i++) /* Search thru periods until we find 
                                          * which one this particular
                                          * accumulated hazard is in. */
   {
      /* Determine if this time to compare is within the current forecast
       * being processed. 
       */
      if ((((*periodTimes)[i-1] <= hazStartTimeToCompareMid) && 
           (hazStartTimeToCompareMid < (*periodTimes)[i])) || 
           (f_hazContinuesToNextPeriod))
      {
         **numFmtdRows += 1;
         timeIndex++;

         /***************************** STARTTIME(S) *************************/
         *startTimes = (char **)realloc(*startTimes, (timeIndex+1) * 
                                        sizeof(char *));

         /* If so, first see if hazard hour straddles period boundary 
          * beginning. If so, the  period boundary start is a startTime of the 
          * hazard. 
          */
         if ((hazStartTimeToCompareStart <= (*periodTimes)[i-1] && 
             (*periodTimes)[i-1] < hazStartTimeToCompareEnd) || 
             (f_hazContinuesToNextPeriod))
         {
            formatValidTime((*periodTimes)[i-1], tempBuff, 30, TZoffset, f_observeDST);
            (*startTimes)[timeIndex] = (char *)malloc((strlen(tempBuff)+1) * sizeof(char));
            strcpy ((*startTimes)[timeIndex], tempBuff);
         }
         else /* Hazard row begins somewhere in middle of period. This hazard row is the 
               * startTime of the entire hazard. This possibility can only occur 
               * once, in the first period hazard begins in.
               */
         {
            formatValidTime(hazStartTimeToCompareStart, tempBuff, 30, TZoffset, f_observeDST);
            (*startTimes)[timeIndex] = (char *)malloc((strlen(tempBuff)+1) * sizeof(char));
            strcpy ((*startTimes)[timeIndex], tempBuff);
         }

         /******************************** ENDTIME(S) ************************/
         /* Does the final hour of the accumulated hazard occur in 
          * subsequent periods? if so, then the current period boundary 
          * ending becomes an intermediate endTime. We'll need to go into
          * subsequent periods. Note this.
          */
         *endTimes = (char **)realloc(*endTimes, (timeIndex+1) * sizeof(char *));
         if (hazEndTimeToCompareMid >= (*periodTimes)[i])
         {
            formatValidTime((*periodTimes)[i], tempBuff, 30, TZoffset, f_observeDST);
            (*endTimes)[timeIndex] = (char *)malloc((strlen(tempBuff)+1) * sizeof(char));
            strcpy ((*endTimes)[timeIndex], tempBuff);
            f_hazContinuesToNextPeriod = 1;
         }
         else /* Final endTime of accumulated hazard is in this period. Use the 
               * final endTime as endTime of accumulated hazard. We can leave 
               * routine after getting it. 
               */              
         {
            formatValidTime(hazEndTimeToCompareEnd, tempBuff, 30, TZoffset, f_observeDST);
            (*endTimes)[timeIndex] = (char *)malloc((strlen(tempBuff)+1) * sizeof(char));
            strcpy ((*endTimes)[timeIndex], tempBuff);
            f_entireHazIsFound = 1;
            f_hazContinuesToNextPeriod = 0;
         }
      }

      /* Check to see if accumulated hazard in question continues into 
       * subsequent periods. If not, we have all necessary info on this hazard 
       * and can leave routine. 
       */
      if (f_hazContinuesToNextPeriod)
         continue;
      else if (f_entireHazIsFound)
         break;
   }

   return;
}
