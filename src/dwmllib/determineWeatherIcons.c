/******************************************************************************
 * determineWeatherIcons() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   This code creates links for icons portraying various weather conditions.  
 *   These icons have the hightest priority and supercede cloud cover (sky),
 *   windy, cold and hot conditions.  Within the weather conditions group, 
 *   priority is estabished by each weather conditions relative appearence
 *   in the processing flow. The highest priority conditions occur later.
 * 
 * ARGUMENTS
 * windTimeEqualsWeatherTime = Flag indicating if the weather and wind times 
 *                             are the same. 0 = no, 1 = yes. (Input)
 *             itIsNightTime = Flag to indicate if it is a night time data 
 *                             item.  If it is night time (= 1, otherwise = 0),
 *                             a night time 
 *                             icon will be used. (Input)
 *                   skyInfo = Structure holding Sky Cover data and time info 
 *                             from the match structure. Used in derivation of
 *                             icons & weather. (Input)
 *                    wsInfo = Structure holding Wind Speed data and time info 
 *                             from the match structure. Used in derivation of
 *                             icons & weather. (Input)
 *                   wsIndex = The counter for wind since it differs from the 
 *                             weather counter (wxIndex) in later times (6 hour 
 *                             interval vice 3 our intervals for weather). 
 *                             (Input)
 *                  skyIndex = The counter for sky cover since it differs from 
 *                             the weather counter (wxIndex) in later times (6 
 *                             hour interval vice 3 our intervals for weather). 
 *                             (Input)
 *                   baseURL = String value holding the path to the icons.  The
 *                             URL looks like http://www.crh.noaa.gov/weather/
 *                             images/fcicons/. (Input)
 *                 numRowsWS = The number of data rows for wind speed to process 
 *                             and format for this point. (Input)
 *                numRowsSKY = The number of data rows for sky cover to process  
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
 *                 numGroups = Number of weather groups making up one 
 *                             observation of weather data (one row of weather 
 *                             data). There can be up to 5 weather groups at any
 *                             given valid time. (Input)
 *                    wxType = The second field (of 5) in one Group of weather
 *                             data. A pointer to a character array. (Input)
 * skyCoverTimeEqualsWeatherTime = Flag indicating if the weather and sky cover
 *                                 times are the same. 0 = no, 1 = yes. (Input)
 *            POP12ValToPOP3 = Current value of the PoP12 covering the current
 *                             weather times. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY:
 *   3/2006 Paul Hershberg (MDL): Created
 *   9/2006 Paul Hershberg (MDL): Added functionality to add Pops to the icons
 *                                (i.e., ra.jpg --> ra50.jpg)
 *   9/2007 Paul Hershberg (MDL): Changed sky threshold to 50% from 60% to align
 *                                with top of partly cloudy range as this was 
 *                                changed to align with NWSI 10-503.
 *   1/2008 Paul Hershberg (MDL): Added night component .jpg images for nfu.jpg,
 *                                and ndu.jpg.
 *   1/2008 Paul Hershberg (MDL): Removed f_isHaze flag. If Haze occurs as a 
 *                                weather type, there is no longer an icon
 *                                associated with it (defaults to cloud/wind).
 *
 * NOTES:
 ******************************************************************************
 */
#include "xmlparse.h"
void determineWeatherIcons(icon_def *iconInfo, int numGroups, char **wxType,
                           int skyCoverTimeEqualsWeatherTime, int itIsNightTime, 
                           elem_def *skyInfo, char *baseURL, int numRowsSKY, 
                           int skyIndex, int wxIndex, 
                           int windTimeEqualsWeatherTime, elem_def *wsInfo, 
                           int wsIndex, int numRowsWS, int numRowsTEMP, 
                           int hourlyTempIndex,
                           int hourlyTempTimeEqualsWeatherTime,
                           elem_def *tempInfo, int POP12ValToPOP3) 
{
   /* Initialize flags to '0' so we can selectively turn them on if the
    * conditions are occuring. 
    */
   int f_isFog = 0;
   int f_isFreezingFog = 0;
   int f_isIceFog = 0;
   int f_isSmoke = 0;
   int f_isBlowingDust = 0;
   int f_isBlowingSnow = 0;
   int f_isDrizzle = 0;
   int f_isRain = 0;
   int f_isRainShowers = 0;
   int f_isSnow = 0;
   int f_isSnowShowers = 0;
   int f_isFreezingDrizzle = 0;
   int f_isFreezingRain = 0;
   int f_isIcePellets = 0;
   int f_isThunderstorm = 0;
   int f_isBlowingSand = 0;
   int f_noIcon = 0;
   int groupIndex;

   /* Determine what types we are dealing with in this time period. */
   for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
   {
      if (strcmp(wxType[groupIndex], "F") == 0)
         f_isFog = 1;
      else if (strcmp(wxType[groupIndex], "K") == 0)
         f_isSmoke = 1;
      else if (strcmp(wxType[groupIndex], "BD") == 0)
         f_isBlowingDust = 1;
      else if (strcmp(wxType[groupIndex], "BS") == 0)
         f_isBlowingSnow = 1;
      else if (strcmp(wxType[groupIndex], "L") == 0)
         f_isDrizzle = 1;
      else if (strcmp(wxType[groupIndex], "R") == 0)
         f_isRain = 1;
      else if (strcmp(wxType[groupIndex], "RW") == 0)
         f_isRainShowers = 1;
      else if (strcmp(wxType[groupIndex], "IP") == 0)
     	 f_isIcePellets = 1;
      else if (strcmp(wxType[groupIndex], "S") == 0)
         f_isSnow = 1;
      else if (strcmp(wxType[groupIndex], "SW") == 0)
         f_isSnowShowers = 1;
      else if (strcmp(wxType[groupIndex], "ZL") == 0)
         f_isFreezingDrizzle = 1;
      else if (strcmp(wxType[groupIndex], "ZR") == 0)
         f_isFreezingRain = 1;
      else if (strcmp(wxType[groupIndex], "T") == 0)
         f_isThunderstorm = 1;
      else if (strcmp(wxType[groupIndex], "BN") == 0)
         f_isBlowingSand = 1;
      else if (strcmp(wxType[groupIndex], "IC") == 0)
         f_noIcon = 1;
      else if (strcmp(wxType[groupIndex], "VA") == 0)
         f_noIcon = 1;
      else if (strcmp(wxType[groupIndex], "WP") == 0)
         f_noIcon = 1;
      else if (strcmp(wxType[groupIndex], "ZF") == 0)
         f_isFreezingFog = 1;
      else if (strcmp(wxType[groupIndex], "IF") == 0)
         f_isIceFog = 1;
      else if (strcmp(wxType[groupIndex], "ZY") == 0)
         f_noIcon = 1;
      else if (strcmp(wxType[groupIndex], "H") == 0)
         f_noIcon = 1;
      else if (strcmp(wxType[groupIndex], "FR") == 0)
         f_noIcon = 1;
      else
         f_noIcon = 1;
   }

   /* Now that we have one or more members of the group processed lets either 
    * create a nonWeather icon if no weather icon is possible or create the
    * appropriate weather icon. 
    */
   if (f_noIcon)
   {
      /* Determine the conditions icon element based on sky cover. */
      determineSkyIcons(skyCoverTimeEqualsWeatherTime, itIsNightTime, skyIndex,
                        wxIndex, skyInfo, &(iconInfo[0]), baseURL, numRowsSKY);

      /* Determine the conditions icon element based on things like extreme
       * temperatures and strong winds. 
       */
      determineNonWeatherIcons(windTimeEqualsWeatherTime, itIsNightTime,
                               wsInfo, wsIndex, baseURL, numRowsWS,
                               &(iconInfo[0]), wxIndex, numRowsTEMP, tempInfo,
                               hourlyTempIndex,
                               hourlyTempTimeEqualsWeatherTime);
   }
   else
   {
      /* We check for the presence of each weather type noting that order is
       * important -- first things are less important. 
       */
      if (itIsNightTime)
      {
         if (f_isFog || f_isFreezingFog || f_isIceFog)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nfg.jpg");
         if (f_isSmoke)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nfu.jpg");
         if (f_isBlowingDust || f_isBlowingSand)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ndu.jpg");
         if (f_isBlowingSnow || f_isBlowingSand)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "blizzard.jpg");
         if (f_isDrizzle || f_isRain)
            determineIconUsingPop(iconInfo[wxIndex].str, "nra", ".jpg", 
			          POP12ValToPOP3, baseURL);

         /* The rain showers icon has a dependancy on sky cover. So, we need
          * to know if there is a corresponding sky cover. value at this
          * time. 
	  */
         if (f_isRainShowers && skyCoverTimeEqualsWeatherTime)
         {
            if (skyInfo[skyIndex].data > 50)
	       determineIconUsingPop(iconInfo[wxIndex].str, "nra", ".jpg", 
			             POP12ValToPOP3, baseURL);       
            else
	       determineIconUsingPop(iconInfo[wxIndex].str, "hi_nshwrs", ".jpg", 
			             POP12ValToPOP3, baseURL);  		    
         }

         /* This is the case where there is no sky cover at this time. So, we 
          * use the previous sky cover which is 3 hours earlier. 
	  */
         if (f_isRainShowers && skyCoverTimeEqualsWeatherTime != 1 &&
             skyIndex > 0)
         {
            if (skyInfo[skyIndex - 1].data > 50)
       	       determineIconUsingPop(iconInfo[wxIndex].str, "nra", ".jpg", 
			             POP12ValToPOP3, baseURL);  
            else
               determineIconUsingPop(iconInfo[wxIndex].str, "hi_nshwrs", ".jpg", 
			             POP12ValToPOP3, baseURL);  
         }

         if (f_isIcePellets)
            determineIconUsingPop(iconInfo[wxIndex].str, "ip", ".jpg", 
			          POP12ValToPOP3, baseURL);  
         if (f_isFreezingDrizzle || f_isFreezingRain)
            determineIconUsingPop(iconInfo[wxIndex].str, "fzra", ".jpg", 
			          POP12ValToPOP3, baseURL);  
         if (f_isSnow || f_isSnowShowers)
            determineIconUsingPop(iconInfo[wxIndex].str, "nsn", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if ((f_isRain || f_isRainShowers || f_isDrizzle) &&
             (f_isSnow || f_isSnowShowers))
            determineIconUsingPop(iconInfo[wxIndex].str, "nrasn", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if ((f_isFreezingRain || f_isFreezingDrizzle) &&
             (f_isSnow || f_isSnowShowers || f_isIcePellets))
            determineIconUsingPop(iconInfo[wxIndex].str, "mix", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if ((f_isRain || f_isRainShowers || f_isDrizzle) && f_isIcePellets)
            determineIconUsingPop(iconInfo[wxIndex].str, "nraip", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if ((f_isSnow || f_isSnowShowers) && f_isIcePellets)
            determineIconUsingPop(iconInfo[wxIndex].str, "ip", ".jpg", 
			          POP12ValToPOP3, baseURL);

         /* The thunderstorm icon has a dependancy on sky cover. So, we need
          * to know if there is a corresponding sky cover. value at this
          * time. 
	  */
         if (f_isThunderstorm && skyCoverTimeEqualsWeatherTime)
         {
            if (skyInfo[skyIndex].data > 50)
               determineIconUsingPop(iconInfo[wxIndex].str, "ntsra", ".jpg", 
			             POP12ValToPOP3, baseURL);
            else
               determineIconUsingPop(iconInfo[wxIndex].str, "nscttsra", ".jpg", 
			             POP12ValToPOP3, baseURL);
         }

         /* This is the case where there is no sky cover at this time. So, we 
          * use the previous sky cover which is 3 hours earlier. */

         if (f_isThunderstorm && skyCoverTimeEqualsWeatherTime != 1 &&
             skyIndex > 0)
         {
            if (skyInfo[skyIndex - 1].data > 50)
               determineIconUsingPop(iconInfo[wxIndex].str, "ntsra", ".jpg", 
			             POP12ValToPOP3, baseURL);
            else
               determineIconUsingPop(iconInfo[wxIndex].str, "nscttsra", ".jpg", 
			             POP12ValToPOP3, baseURL);
         }
      }

      else /* It is day time. */
      {
         if (f_isFog || f_isFreezingFog || f_isIceFog)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "fg.jpg");
         if (f_isSmoke)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "fu.jpg");
         if (f_isBlowingDust || f_isBlowingSand)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "du.jpg");
         if (f_isBlowingSnow || f_isBlowingSand)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "blizzard.jpg");
         if (f_isDrizzle || f_isRain)
            determineIconUsingPop(iconInfo[wxIndex].str, "ra", ".jpg", 
			          POP12ValToPOP3, baseURL);

         /* The rain showers icon has a dependancy on sky cover. So, we need
          * to know if there is a corresponding sky cover. value at this
          * time. 
	  */
         if (f_isRainShowers && skyCoverTimeEqualsWeatherTime)
         {
            if (skyInfo[skyIndex].data > 50)
               determineIconUsingPop(iconInfo[wxIndex].str, "shra", ".jpg", 
			             POP12ValToPOP3, baseURL);
            else
               determineIconUsingPop(iconInfo[wxIndex].str, "hi_shwrs", ".jpg", 
			             POP12ValToPOP3, baseURL);
         }

         /* This is the case where there is no sky cover at this time. So, we 
          * use the previous sky cover which is 3 hours earlier. 
	  */
         if (f_isRainShowers && skyCoverTimeEqualsWeatherTime != 1 &&
             skyIndex > 0)
         {
            if (skyInfo[skyIndex - 1].data > 50)
               determineIconUsingPop(iconInfo[wxIndex].str, "ra", ".jpg", 
			             POP12ValToPOP3, baseURL);
            else
               determineIconUsingPop(iconInfo[wxIndex].str, "hi_shwrs", ".jpg", 
			             POP12ValToPOP3, baseURL);
         }

         if (f_isIcePellets)
            determineIconUsingPop(iconInfo[wxIndex].str, "ip", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if (f_isFreezingDrizzle || f_isFreezingRain)
            determineIconUsingPop(iconInfo[wxIndex].str, "fzra", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if (f_isSnow || f_isSnowShowers)
            determineIconUsingPop(iconInfo[wxIndex].str, "sn", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if ((f_isRain || f_isRainShowers || f_isDrizzle) &&
             (f_isSnow || f_isSnowShowers))
            determineIconUsingPop(iconInfo[wxIndex].str, "rasn", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if ((f_isFreezingRain || f_isFreezingDrizzle) &&
             (f_isSnow || f_isSnowShowers || f_isIcePellets))
            determineIconUsingPop(iconInfo[wxIndex].str, "mix", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if ((f_isRain || f_isRainShowers || f_isDrizzle) && f_isIcePellets)
            determineIconUsingPop(iconInfo[wxIndex].str, "raip", ".jpg", 
			          POP12ValToPOP3, baseURL);
         if ((f_isSnow || f_isSnowShowers) && f_isIcePellets)
            determineIconUsingPop(iconInfo[wxIndex].str, "ip", ".jpg", 
			          POP12ValToPOP3, baseURL);

         /* The thunderstorm icon has a dependancy on sky cover. So, we need
          * to know if there is a corresponding sky cover. value at this
          * time. 
	  */
         if (f_isThunderstorm && skyCoverTimeEqualsWeatherTime)
         {
            if (skyInfo[skyIndex].data > 50)
               determineIconUsingPop(iconInfo[wxIndex].str, "tsra", ".jpg", 
			             POP12ValToPOP3, baseURL);
            else
               determineIconUsingPop(iconInfo[wxIndex].str, "scttsra", ".jpg", 
			             POP12ValToPOP3, baseURL);
         }

         /* This is the case where there is no sky cover at this time. So, we 
          * use the previous sky cover which is 3 hours earlier. */

         if (f_isThunderstorm && skyCoverTimeEqualsWeatherTime != 1 &&
             skyIndex > 0)
         {
            if (skyInfo[skyIndex - 1].data > 50)
               determineIconUsingPop(iconInfo[wxIndex].str, "tsra", ".jpg", 
			             POP12ValToPOP3, baseURL);
            else
               determineIconUsingPop(iconInfo[wxIndex].str, "scttsra", ".jpg", 
			             POP12ValToPOP3, baseURL);
         }
      } /* End of night vs day check. */

   } /* End of icon vs no icon possible check. */
   
   return;
}
