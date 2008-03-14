/*****************************************************************************
 * windExtremePhrase() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   This code determines which wind phrase should be used and then
 *   assigns an icon to correspond to it. It follows the algorithm 
 *   developed by Mark Mitchell for use in the 12 hourly (f_XML = 3) and 
 *   24 hourly (f_XML = 4) summarization products on the NWS web site.
 *
 * ARGUMENTS
 *           dayIndex = Indicates which day (summarization period) is being 
 *                      processed. (Input)
 *             phrase = Array containing the short current conditions phrase 
 *                      (i.e rain or partly cloudy) for each summarization 
 *                      period. (Output)
 *           iconInfo = Array containing the link to a current conditions icon.
 *            baseURL = String value holding the path to the icons.  The
 *                      URL looks like http://www.crh.noaa.gov/weather/
 *                      images/fcicons/. (Input) 
 *   springDoubleDate = The end date of next cold season expressed in double form.
 *                      (Input)
 *     fallDoubleDate = The start date of next cold season expressed in double 
 *                      form. (Input)
 *        f_isDayTime = Flag denoting if period is in day time. (Input)
 *      f_isNightTime = Flag denoting if period is in night time. (Input)
 *       maxWindSpeed = Array containing the Maximum wind speed values corresponding
 *                      to a day (24 hour format) or 12 hour period (12 hour format).
 *                      These values are used in deriving the weather and/or icon values. 
 *                      (Input)
 *   maxWindDirection = Array containing the wind direction values 
 *                      corresponding to a day (24 hour format) or 12 hour period
 *                      (12 hour format). These are not "max" wind direction 
 *                      values, but correspond to the time when the max. wind 
 *                      speed values were found per forecast period.  These values
 *                      are used in deriving the weather and/or icon values. 
 *      periodMaxTemp = For each forecast period, the "max" temperature occuring in
 *                      the period, based off of the MaxT and MinT elements. If night, 
 *                      the period could have a "max" MinT. (Input) 
 * maxWindSpeedValTimes = Array holding valid Time of max Wind Speed per 
 *                        forecast period (Input). 
 *                                                                
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 * HISTORY:
 *  6/2006 Paul Hershberg (MDL): Created.
 *  3/2007 Paul Hershberg (MDL): Upped the wind threshold from 25 to 30mph.
 *                               Upped the breezy threshold from 15 to 20mph.
 *  3/2008 Paul Hershberg (MDL): Added maxWindSpeedValTimes for determination 
 *                               of Cold vs Warm season.
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void windExtremePhrase(int f_isDayTime, int f_isNightTime, int dayIndex, 
                       char *baseURL, double springDoubleDate, 
		       double fallDoubleDate, int *maxWindSpeed, 
		       int *maxWindDirection, int *periodMaxTemp, 
		       icon_def *iconInfo, char **phrase, 
                       double *maxWindSpeedValTimes)
{
   int WINDY = 30; /* Windy threshold. */
   int BREEZY = 20; /* Breezy threshold. */
   int NORTHEAST = 60; /* NE Wind directions from which BLUSTERY phrase is 
			* created. */
   int NORTHWEST = 300; /* NW Wind directions from which BLUSTERY phrase is 
			 * created. */
   int COLD = 32; /* Temp determining if "Blustery" is formatted as weather.
                     This is different temp for when just "Cold" is  the
                     dominant weather. For that, the temp = 10 deg F. */

   /* Lets determine if wind speeds support a WINDY, BREEZY, or BLUSTERY 
    * phrase. 
    */
   if (maxWindSpeed[dayIndex] != -999)
   {
      if (maxWindSpeed[dayIndex] >= WINDY)
      {
         strcpy (phrase[dayIndex], "Windy");
      }      
      else if ((maxWindSpeed[dayIndex] >= BREEZY) &&
               (maxWindDirection[dayIndex] <= NORTHEAST || 
   	       maxWindDirection[dayIndex] >= NORTHWEST) &&
               (maxWindSpeedValTimes[dayIndex] <= springDoubleDate && 
	       maxWindSpeedValTimes[dayIndex] >= fallDoubleDate) &&
               (periodMaxTemp[dayIndex] < COLD))
      {
         strcpy (phrase[dayIndex], "Blustery");
      }
      else if (maxWindSpeed[dayIndex] >= BREEZY)
      { 
         strcpy (phrase[dayIndex], "Breezy");
      }

      /* Set the Windy Icon. */
      if ((maxWindSpeed[dayIndex] >= BREEZY) && f_isDayTime)  
      { 
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "wind.jpg"); 
      }
      else if ((maxWindSpeed[dayIndex] >= BREEZY) && f_isNightTime)
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nwind.jpg"); 
      }
      
   }

   return;
}
