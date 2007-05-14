/*****************************************************************************
 * tempExtremePhrase () -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   This code determines if the day time temperatures is either Hot or Cold 
 *   and then assigns an icon to correspond to it. It follows the algorithm 
 *   developed by Mark Mitchell for use in the 12 hourly (f_XML = 3) and 24
 *   hourly (f_XML = 4) summarization products on the NWS web site.
 *
 * ARGUMENTS
 *           dayIndex = Indicates which day (summarization period) is being 
 *                      processed. (Input)
 *             phrase = Array containing the short current conditions phrase 
 *                      (i.e rain or partly cloudy) for each summarization 
 *                      period. (Output)
 *           iconInfo = Array containing the links to current conditions icons.
 *                      (Output)
 *            baseURL = String value holding the path to the icons. The
 *                      URL looks like http://www.crh.noaa.gov/weather/
 *                      images/fcicons/. (Input)
 *        f_isDayTime = Flag denoting if period is in day time. (Input)
 *      periodMaxTemp = For each forecast period, the "max" temperature occuring in
 *                      the period, based off of the MaxT and MinT elements. If night, 
 *                      the period could have a "max" MinT. (Input)
 *                      
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 * HISTORY:
 *  6/2006 Paul Hershberg (MDL): Created.
 *  2/2007 Paul Hershberg (MDL): Upped the HOT temp to 97F from 95F.
 *                               Lowered the COLD temp from 32F to 10F.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void tempExtremePhrase(int f_isDayTime, int *periodMaxTemp, int dayIndex, 
                       char *baseURL, icon_def *iconInfo, char **phrase)
{
   int HOT = 97; /* Temp determining if "Hot" is formatted as weather. */
   int COLD = 10; /* Temp determining if "Cold" is formatted as weather. */

   /* Lets process the case for HOT or COLD day time temperatures. */
   if (periodMaxTemp[dayIndex] != 999)
   {
      if ((periodMaxTemp[dayIndex] >= HOT) && f_isDayTime)
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "hot.jpg"); 
         strcpy (phrase[dayIndex], "Hot");
      }

      if ((periodMaxTemp[dayIndex] <= COLD) && f_isDayTime)
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "cold.jpg"); 
         strcpy (phrase[dayIndex], "Cold");
      }
   }
   return;
}       
