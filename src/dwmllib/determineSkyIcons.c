/******************************************************************************
 * determineSkyIcons() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Determines the Weather Conditions Icon element based on Sky Cover. These
 *   icons have a lower priority than weather and so only occur when weather is
 *   not present.
 *   
 * ARGUMENTS
 * skyCoverTimeEqualsWeatherTime = Flag indicating if the weather and sky cover
 *                                 times are the same. 0 = no, 1 = yes. (Input)
 *             itIsNightTime = Flag to indicate if it is a night time data 
 *                             item.  If it is night time (= 1, otherwise = 0),
 *                             a night time 
 *                   skyInfo = Structure holding Sky Cover data and time info 
 *                             from the match structure. Used in derivation of
 *                             icons & weather. (Input)
 *                  skyIndex = The counter for sky cover since it differs from
 *                             the weather counter (wxIndex) in later times (6
 *                             hour interval vice 3 our intervals for weather). 
 *                             (Input)
 *                   baseURL = String value holding the path to the icons.  The
 *                             URL looks like http://www.crh.noaa.gov/weather/
 *                             images/fcicons/. (Input)
 *                  iconInfo = Structure holding derived Icon links and time 
 *                             info. (Output)
 *                   wxIndex = The counter for weather and icons since it 
 *                             differs from the sky, wind speed, and temp 
 *                             counters in later times (6 hour interval vice 3
 *                             our intervals for weather). (Input)
 *                numRowsSKY = The number of data rows for sky cover to process
 *                             and format for this point. (Input)
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
void determineSkyIcons(int skyCoverTimeEqualsWeatherTime, int itIsNightTime, 
                       int skyIndex, int wxIndex, elem_def *skyInfo, 
                       icon_def *iconInfo, char *baseURL, int numRowsSKY)
{

   /* The time intervals for sky cover varies through the forecast. However,
    * weather does not. So we need to syncronize the sky cover so the right
    * icon appears in the correct weather time interval. 
    */
   if (skyCoverTimeEqualsWeatherTime)
   {
      if (itIsNightTime)
      {
         if (skyInfo[skyIndex].data <= 5)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nskc.jpg");
         if (skyInfo[skyIndex].data > 5 && skyInfo[skyIndex].data <= 25)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nfew.jpg");
         if (skyInfo[skyIndex].data > 25 && skyInfo[skyIndex].data <= 50)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nsct.jpg");
         if (skyInfo[skyIndex].data > 50 && skyInfo[skyIndex].data <= 87)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nbkn.jpg");
         if (skyInfo[skyIndex].data > 87)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "novc.jpg");
      }
      else
      {
         if (skyInfo[skyIndex].data <= 5)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "skc.jpg");
         if (skyInfo[skyIndex].data > 5 && skyInfo[skyIndex].data <= 25)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "few.jpg");
         if (skyInfo[skyIndex].data > 25 && skyInfo[skyIndex].data <= 50)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "sct.jpg");
         if (skyInfo[skyIndex].data > 50 && skyInfo[skyIndex].data <= 87)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "bkn.jpg");
         if (skyInfo[skyIndex].data > 87)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ovc.jpg");
      }
      if (skyIndex < numRowsSKY)
         skyIndex += 1;
   }
   else
   {
      /* We are going to assume that if the weather time doesn't match the
       * sky cover time that we are processing the data where sky cover is
       * forecast in 6 hour intervals and weather is forecast in 3 hour
       * interval.  So, we use the previous sky cover for the conditions
       * icons until weather and sky cover have the same time.  This should
       * happen every other weather value. 
       */
      if (itIsNightTime && skyIndex > 0)
      {
         if (skyInfo[skyIndex - 1].data <= 5)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nskc.jpg");
         if (skyInfo[skyIndex - 1].data > 5 && skyInfo[skyIndex - 1].data <= 25)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nfew.jpg");
         if (skyInfo[skyIndex - 1].data > 25
             && skyInfo[skyIndex - 1].data <= 50)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nsct.jpg");
         if (skyInfo[skyIndex - 1].data > 50
             && skyInfo[skyIndex - 1].data <= 87)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nbkn.jpg");
         if (skyInfo[skyIndex - 1].data > 87)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "novc.jpg");
      }
      else if (itIsNightTime != 1 && skyIndex > 0)
      {
         if (skyInfo[skyIndex - 1].data <= 5)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "skc.jpg");
         if (skyInfo[skyIndex - 1].data > 5 && skyInfo[skyIndex - 1].data <= 25)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "few.jpg");
         if (skyInfo[skyIndex - 1].data > 25
             && skyInfo[skyIndex - 1].data <= 50)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "sct.jpg");
         if (skyInfo[skyIndex - 1].data > 50
             && skyInfo[skyIndex - 1].data <= 87)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "bkn.jpg");
         if (skyInfo[skyIndex - 1].data > 87)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ovc.jpg");
      }
   }

   return;
}
