/*****************************************************************************
 * blizzardCheck() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   This code checks to see if conditions warrant changing the weather summary
 *   phrase of the day to "Blizzard.
 *
 * ARGUMENTS
 *           blizzCnt = Counter thru blizzard Times array. For each time 
 *                      blizzard conditions are met, the weather time is saved.
 *                      This is the counter thru that array of weather times.
 *                      (Input)
 *   periodStartTimes = Array holding starting times for designated forecast 
 *                      periods. (Input)
 *     periodEndTimes = Array holding ending times for designated forecast 
 *                      periods. (Input)
 *       blizzardTime = If all blizzard conditions are met in the ugly weather 
 *                      string (weather type = "S" or "BS" with a corresponding
 *                      weather intensity = "m" or "+"), the time is recorded 
 *                      in this array. (Input)
 *          numRowsWS = The number of data rows for wind speed. These data are 
 *                      used to figure if blizzard is to be formatted as the 
 *                      weather phrase. Wind speeds must be > 35 concurrent with
 *                      the blizzard (weather) time. (Input)
 *          numRowsWG = The number of data rows for wind gusts. These data are 
 *                      used to figure if blizzard is to be formatted as the 
 *                      weather phrase. Wind gusts must be > 35 concurrent with
 *                      the blizzard (weather) time. (Input)
 *             wsInfo = Wind Speed data taken from the match array. (Input)
 *             wgInfo = Wind Gust data taken from the match array. (Input) 
 *           dayIndex = Indicates which day (summarization period) is being 
 *                      processed. (Input)
 *      f_iconToBlizz = Flag to change the icon link to "blizzard" if the
 *                      weather phrase is "blizzard". (Input)
 *             phrase = Array containing the short current conditions phrase 
 *                      (i.e rain or partly cloudy) for each summarization 
 *                      period. (Output)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void 
 *
 * HISTORY:
 * 6/2007 Paul Hershberg (MDL): Created.
 *  
 * NOTES:
 *
 *****************************************************************************
 */
#include "xmlparse.h"
void blizzardCheck (int blizzCnt, double periodStartTime, double periodEndTime,
                    double *blizzardTime, int numRowsWS, int numRowsWG, 
                    elem_def *wsInfo, elem_def *wgInfo, int dayIndex, 
                    int *f_iconToBlizz, char **phrase)
{
   int i; /* Counter thru blizzardTime array of weather times. */
   int j; /* Counter thru wind speed data. */
   int k; /* Counter thru wind gust data. */
   int wxPeriod = 3; /* Period of weather data. */

   /* Override weather phrase with "Blizzard" if conditions warrant it. */
   for (i = 0; i < blizzCnt; i++)
   {
      /* Check to make sure this instance of blizzard conditions is inside the
       * current summarization period. 
       */
      if (blizzardTime[i] >= periodStartTime && 
          blizzardTime[i] <= periodEndTime)
      {
         for (j = 0; j < numRowsWS; j++)
         {
            for (k = 0; k < numRowsWG; k++)
            {
               /* Make sure conditions are met. */
               if ((wsInfo[j].valueType != 2 && wsInfo[j].data >= 35) ||
                  (wgInfo[k].valueType != 2 && wgInfo[k].data >= 35))
               {
                  /* If so, check to see if either wind speed or wind gust time 
                   * corresponds with the weather time containing blizzard
                   * conditions.
                   */ 
                  if ((wsInfo[j].validTime >= blizzardTime[i]-(0.5*wxPeriod*3600)
                     && wsInfo[j].validTime <= (blizzardTime[i] 
                     + (wxPeriod*3600) + (0.5*wxPeriod*3600))) || 
                     (wgInfo[k].validTime >= blizzardTime[i]-(0.5*wxPeriod*3600)
                     && wgInfo[k].validTime <= (blizzardTime[i] 
                     + (wxPeriod*3600) + (0.5*wxPeriod*3600))))
                  {
                     strcpy (phrase[dayIndex], "Blizzard");
                     *f_iconToBlizz = 1;
                     break;
                  }
               }
            }
            continue;
         }
      }
   }
   return;
}
