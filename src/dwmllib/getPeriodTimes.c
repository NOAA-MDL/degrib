/******************************************************************************
 * getPeriodTimes() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code gets the times that correspond to the forecast period boundaries.
 *
 * ARGUMENTS
 *    firstPeriodStartTime = The beginning of the first forecast period (06 hr
 *                           or 18hr) based on the user supplied startTime argument. 
 *                           (Input)
 *       lastPeriodEndTime = The end of the last forecast period (18 hr) based 
 *                           on the startTime & numDays arguments. (Input)
 *   timeInterval = Number of seconds in either a 12 hourly format (3600 * 12)
 *                  or 24 hourly format (3600 * 24). (Input)
 *       TZoffset = Number of hours to add to current time to get GMT time. 
 *                  (Input)
 *   f_observeDST = Flag determining if current point observes Daylight 
 *                  Savings Time. (Input)  
 *    periodTimes = The times bordering each forecast period time. (Output)
 * numPeriodTimes = The number of periodTimes. (Output)
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
void getPeriodTimes(double firstPeriodStartTime, double lastPeriodEndTime, 
                    int timeInterval, double TZoffset, sChar f_observeDST, 
                    double **periodTimes, int *numPeriodTimes)
{
   int j;  /* Counter thru period times. */
   int i;  /* Counter thru period times. */
   int periodTime = firstPeriodStartTime; /* Each period border time. */
   int f_DSTswitchFound = 0; /* Used to detect of there was a switch from  
                              * Standard to DST in Spring or vice versa in 
                              * Fall. */
   int interval = timeInterval; /* Time interval of forecast period (12 hours 
                                 * or 24 hours) in seconds. */
   char strBuff[30];         /* Temporary string buffer holding rounded
                              * data. */
   char month[3];            /* String holding formatted month to see if a 
                                change from standard to daylight savings time
                                (or vice versa) occurs within forecast period
                                in question. */

   /* Allocate and fill first element of array. */
   *periodTimes = (double *)malloc(1 * sizeof(double));
   (*periodTimes)[0] = firstPeriodStartTime;

   *numPeriodTimes = 1;

   /* Loop thru the forecast periods, getting each time. */
   for (i = periodTime, j = 1; i <= lastPeriodEndTime; j++) 
   {

      /* See if the forecast period time interval is not quite 12 
       * (f_XML = 3) or 24 (f_XML = 4) hours due to a change in DST 
       * occurring in a specific forecast period. In the Spring, 
       * "spring" forward in time resulting in a reduction in 1 hour in 
       * the forecast period. Do the opposite if in Fall.
       */
      if (!f_DSTswitchFound)
      {
         formatValidTime(periodTime, strBuff, 30, TZoffset, f_observeDST);
         month[0] = strBuff[5];
         month[1] = strBuff[6];
         month[2] = '\0';

         if (atoi(month) < 6) 
         {
            if ((Clock_IsDaylightSaving2(periodTime, TZoffset) != 1) 
               && (Clock_IsDaylightSaving2(periodTime + interval, 
               TZoffset) == 1))
            {
               interval = interval - 3600;
               f_DSTswitchFound = 1; 
            }
         }
         else
         { 
            if ((Clock_IsDaylightSaving2(periodTime, TZoffset) == 1) 
               && (Clock_IsDaylightSaving2(periodTime + interval, 
               TZoffset) != 1))
            {
               interval = interval + 3600;
               f_DSTswitchFound = 1;
            } 
         }
      }
      *periodTimes = (double *)realloc(*periodTimes, (j+1) * sizeof(double));
      (*periodTimes)[j] = periodTime + interval;
      periodTime = (*periodTimes)[j];
      *numPeriodTimes = *numPeriodTimes + 1;
      i = (*periodTimes)[j];
   }

   return;
}
