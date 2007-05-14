/******************************************************************************
 * determineNonWeatherIcons() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Determines the Weather Conditions Icon element based on Non Weather infor-
 *   ation available such as windy, hot, and cold conditions.  These icons are 
 *   low on the priority list and so may not appear even if conditions warrent 
 *   them.
 * 
 * ARGUMENTS
 * windTimeEqualsWeatherTime = Flag indicating if the weather and wind times 
 *                             are the same. 0 = no, 1 = yes. (Input)
 *             itIsNightTime = Flag to indicate if it is a night time data 
 *                             item.  If it is night time (= 1, otherwise = 0),
 *                             a night time 
 *                    wsInfo = Structure holding Wind Speed data and time info 
 *                             from the match structure. Used in derivation of
 *                             icons & weather. (Input)
 *                   wsIndex = The counter for wind since it differs from the 
 *                             weather counter (wxIndex) in later times (6 hour 
 *                             interval vice 3 our intervals for weather). 
 *                             (Input)
 *                   baseURL = String value holding the path to the icons.  The
 *                             URL looks like http://www.crh.noaa.gov/weather/
 *                             images/fcicons/. (Input)
 *                 numRowsWS = The number of data rows for wind speed to process 
 *                             and format for this point. (Input)
 *                  iconInfo = Structure holding derived Icon links and time 
 *                             info. (Output)
 *                   wxIndex = The counter for weather and icons since it 
 *                             differs from the sky, wind speed, and temp 
 *                             counters in later times (6 hour interval vice 3
 *                             our intervals for weather). (Input)
 *               numRowsTEMP = The number of data rows for hourly temperature to
 *                             process and format for this point. (Input)
 *                  tempInfo = Structure holding hourly temperature data and 
 *                             time info from the match structure. Used in 
 *                             derivation of icons & weather. (Input)
 *           hourlyTempIndex = The counter for hourly temp since it differs
 *                             from the weather counter (wxIndex) in later 
 *                             times (6 hour interval vice 3 our intervals for
 *                             weather). (Input)
 * hourlyTempTimeEqualsWeatherTime = Flag indicating if the weather and wind
 *                                   times are the same. 0 = no, 1 = yes. 
 *                                   (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2006 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void determineNonWeatherIcons(int windTimeEqualsWeatherTime, int itIsNightTime, 
                              elem_def *wsInfo, int wsIndex, char *baseURL, 
                              int numRowsWS, icon_def *iconInfo, int wxIndex, 
                              int numRowsTEMP, elem_def *tempInfo, 
                              int hourlyTempIndex, 
                              int hourlyTempTimeEqualsWeatherTime)
{
   int strongWind = 30;       /* Wind speed (knots) in which windy icon will
                               * format */
   int hotTemperature = 110;  /* Temp in which "hot" icon will format. */
   int coldTemperature = -40; /* Temp in which "cold" icon will format. */

   /* The time intervals for wind varies through the forecast. However,
    * weather does not.  So we need to syncronize the wind so the right icon
    * appears in the correct weather time interval. 
    */
   if (windTimeEqualsWeatherTime)
   {
      if (itIsNightTime)
      {
         if (wsInfo[wsIndex].data > strongWind)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nwind.jpg");
      }
      else
      {
         if (wsInfo[wsIndex].data > strongWind)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "wind.jpg");
      }
      if (wsIndex < numRowsWS)
         wsIndex += 1;
   }
   else
   {

      /* We are going to assume that if the weather time doesn't match the
       * wind time that we are processing the data where wind is forecast in
       * 6 hour intervals and weather is forecast in 3 hour interval. So, we
       * use the previous wind value for the conditions icon until weather
       * and wind have the same time.  This should happen every other weather
       * value. 
       */
      if (itIsNightTime && wsIndex > 0)
      {
         if (wsInfo[wsIndex - 1].data > strongWind)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nwind.jpg");
      }
      else if (itIsNightTime != 1 && wsIndex > 0)
      {
         if (wsInfo[wsIndex - 1].data > strongWind)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "wind.jpg");
      }
   }

   /* The time intervals for temperatures varies through the forecast.
    * However, weather does not.  So we need to syncronize the temperature
    * time so the right icon appears in the correc weather time interval. 
    */
   if (hourlyTempTimeEqualsWeatherTime)
   {
      if (itIsNightTime != 1)
      {
         if (tempInfo[hourlyTempIndex].data > hotTemperature)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "hot.jpg");
         if (tempInfo[hourlyTempIndex].data < coldTemperature)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "cold.jpg");
      }
      if (hourlyTempIndex < numRowsTEMP)
         hourlyTempIndex += 1;
   }
   else
   {

      /* We are going to assume that if the weather time doesn't match the
       * temperature time that we are processing the data where temperature
       * is forecast in 6 hour intervals and weather is forecast in 3 hour
       * interval.  So, we use the previous temperature value for the
       * conditions icon until weather and temperature have the same time.
       * This should happen every other weather value. 
       */
      if (itIsNightTime != 1 && hourlyTempIndex > 0)
      {
         if (tempInfo[hourlyTempIndex - 1].data > hotTemperature)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "hot.jpg");
         if (tempInfo[hourlyTempIndex - 1].data < coldTemperature)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "cold.jpg");
      }
   }

   return;
}
