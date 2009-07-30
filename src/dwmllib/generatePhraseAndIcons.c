/*****************************************************************************
 * generatePhraseAndIcons() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   This code creates a phrase to describe the current weather 
 *   conditions.  If weather is present, then the weather (i.e. fog)
 *   is formatted as the phrase. If no weather is occurring, then
 *   sky conditions (i.e. partly cloudy) as well as extremes of heat,
 *   cold, and wind speed are formatted.  Once the weather condition 
 *   is determined, a link to an complementary icon is formatted.
 *
 * ARGUMENTS
 *           dayIndex = Indicates which day (summarization period) is being 
 *                      processed. (Input)
 *             phrase = Array containing the short current conditions phrase 
 *                      (i.e rain or partly cloudy) for each summarization 
 *                      period. (Output)
 *           iconInfo = Array containing the link to a current conditions icon.
 *                      (Output)
 *  f_popIsNotAnIssue = Flag denoting if PoP is very low, we won't format 
 *		        the weather values that might be present. (Output)                   
 *          frequency = Describes the two DWMLgenByDay product and they're type
 *                      of summarizations. (Input)            
 *     timeLayoutHour = The time period's hour for the 12 hourly product. Used 
 *                      to determine if it is night or day (should = 6 or 18).
 *                      (Input)                   
 *    dominantWeather = This array stores the weather type [0], intensity [1], 
 *                      coverage [2], visibility[3] and qualifier [4], for each 
 *                      day that is considered the dominant one. This is the 
 *                      summarized weather for the 24/12 hour summarization 
 *                      period. (Input)
 *            baseURL = String value holding the path to the icons.  The
 *                      URL looks like http://www.crh.noaa.gov/weather/
 *                      images/fcicons/. (Input) 
 *        maxDailyPop = Array containing the pop values corresponding to a day (24 
 *                      hour format) or 12 hour period (12 hour format).  For 24
 *                      hour format, we use the maximum of the two 12 hour pops 
 *                      that span the day. This variable is used to test if the pop 
 *                      is large enough to justify formatting weather values. (Input)
 *    averageSkyCover = Array containing the average Sky Cover values corresponding
 *                      to a day (24 hour format) or 12 hour period (12 hour
 *                      format).  These values are used in deriving the weather 
 *		        and/or icon values. (Input)
 *        maxSkyCover = Array containing the maximum Sky Cover values corresponding
 *                      to a day (24 hour format) or 12 hour period (12 hour
 *                      format).  These values are used in deriving the weather 
 *		        and/or icon values. (Input)
 *        minSkyCover = Array containing the minimum Sky Cover values corresponding
 *                      to a day (24 hour format) or 12 hour period (12 hour
 *                      format).  These values are used in deriving the weather 
 *		        and/or icon values. (Input)
 *          maxSkyNum = Array of indexes where the max sky cover was found. Used to 
 *                      determine sky cover trends (i.e. increasing clouds). 
 *                      (Input)
 *          minSkyNum = Array of indexes where the min sky cover was found. Used to 
 *                      determine sky cover trends (i.e. increasing clouds). 
 *                      (Input)
 *      periodMaxTemp = For each forecast period, the "max" temperature occuring in
 *                      the period, based off of the MaxT and MinT elements. If night, 
 *                      the period could have a "max" MinT. (Input)
 *   springDoubleDate = The end date of next cold season expressed in double form.
 *                      (Input)
 *     fallDoubleDate = The start date of next cold season expressed in double 
 *                      form. (Input)
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
 *                      (Input)
 *  startPositions = The index of where the current forecast period begins.  Used
 *                   to determine sky cover trends (i.e. increasing clouds) for 
 *                   DWMLgenByDay products. (Input)
 *    endPositions = The index of where the current forecast period ends.  Used
 *                   to determine sky cover trends (i.e. increasing clouds) for 
 *                   DWMLgenByDay products. (Input)	 
 *        f_isDrizzle = Flag denoting if weather is drizzle. (Input)
 *           f_isRain = Flag denoting if weather is rain. (Input)
 *    f_isRainShowers = Flag denoting if weather is rain showers. (Input)
 *          f_isSnow  = Flag denoting if weather is snow. (Input)
 *    f_isSnowShowers = Flag denoting if weather is snow showers. (Input)
 * f_isFreezingDrizzle = Flag denoting if weather is frz drizzle. (Input)
 *   f_isFreezingRain = Flag denoting if weather is frz rain. (Input)
 *     f_isIcePellets = Flag denoting if weather is ice pellets. (Input)
 *    f_isBlowingSnow = Flag denoting if weather is blowing snow. (Input)
 *           blizzCnt = Counter thru blizzard Times array. For each time 
 *                      blizzard conditions are met, the weather time is saved.
 *                      This is the counter thru that array of weather times.
 *                      (Input)
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
 * maxWindSpeedValTimes = Array holding valid Time of max Wind Speed per 
 *                        forecast period (Input). 
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 * HISTORY:
 *  6/2006 Paul Hershberg (MDL): Created.
 *  9/2006 Paul Hershberg (MDL): Added functionality to add Pops to the icons
 *                               (i.e., ra.jpg --> ra50.jpg)
 *  6/2007 Paul Hershberg (MDL): Added a blizzard check for formatting 
 *                               "blizzard" as the weather phrase.  
 *  9/2007 Paul Hershberg (MDL): Changed sky threshold to 50% from 60% to align
 *                               with top of partly cloudy range as this was 
 *                               changed to align with NWSI 10-503.
 * 12/2007 Paul Hershberg (MDL): Changed "freezingrain" to "fzra" for icons.
 *  1/2008 Paul Hershberg (MDL): Removed PoP from icon URL for flurries, 
 *                               freezing drizzle, and drizzle. 
 *  1/2008 Paul Hershberg (MDL): Made freezing drizzle Icon URL dependant on 
 *                               POP. Also, changed the icon from fdrizzle.jpg 
 *                               to fzra.jpg, so it is treated like freezing 
 *                               rain. 
 *  1/2008 Paul Hershberg (MDL): For Flurries and Drizzle only, added criteria
 *                               that for flurries.jpg or drizzle.jpg to be 
 *                               formatted as the icon, coverage must be > SChc.
 *                               If coverage == SChc, icon defaults to 
 *                               cloud/wind/temp icon.
 *  1/2008 Paul Hershberg (MDL): For coverages...split SChc and Chc up when 
 *                               determining coverage. Previously, these two 
 *                               coverages were combined to format only "Chance".                                
 *  1/2008 Paul Hershberg (MDL): Changed to a load based URL: 
 *                               http://forecast.weather.gov/images/wtf/ from 
 *                               http://www.nws.noaa.gov/weather/images/fcicons/
 *                               This involved adding some additional night
 *                               time images (nfu.jpg, ndu.jpg). Also, haze.jpg
 *                               no longer exists, so haze icon defaults to a 
 *                               cloud/wind/temp icon. Also, drizzle.jpg and 
 *                               flurries.jpg no longer exist. They were changed
 *                               to use ra/nra.jpg and sn/nsn.jpg respectively,
 *                               with both not utilizing the POP # as part of the 
 *                               URL.
 *  2/2008 Paul Hershberg (MDL): Mapped the Wx Coverages to the Wx Types for 
 *                               "Sleet", "Snow/Sleet", and "Wintry Mix". 
 *  3/2008 Paul Hershberg (MDL): Added maxWindSpeedValTimes for determination 
 *                               of Cold vs Warm season in windExtremePhrase.c 
 *                               routine.
 *  7/2009 Paul Hershberg (MDL): Added weather summary phrase for 
 *                               "Isolated Thunderstorms" 
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void generatePhraseAndIcons (int dayIndex, char *frequency, 
                             int timeLayoutHour, char *dominantWeather[5],
			     char *baseURL, int *maxDailyPop, 
			     int *averageSkyCover, int *maxSkyCover,
			     int *minSkyCover, int *maxSkyNum, 
			     int *minSkyNum, int *periodMaxTemp, 
			     double springDoubleDate, 
			     double fallDoubleDate,  int *maxWindSpeed, 
			     int *maxWindDirection, int *startPositions,
			     int *endPositions, int f_isDrizzle, 
			     int f_isRain, int f_isRainShowers, 
			     int f_isIcePellets, int f_isSnow, 
			     int f_isSnowShowers, int f_isFreezingDrizzle, 
			     int f_isFreezingRain, int f_isBlowingSnow, 
                             elem_def *wgInfo, elem_def *wsInfo,
                             double *blizzardTime, int blizzCnt, 
                             double periodStartTime, double periodEndTime, 
                             icon_def *iconInfo, char **phrase, 
                             int *f_popIsNotAnIssue, int numRowsWS, 
                             int numRowsWG, int percentTimeWithFog, 
                             double *maxWindSpeedValTimes)
{
   int f_noIcon = 1; /* Flag used to track if a weather icon is possible. */
   int lowPopThreshold = 20; /* Threshold below which weather values are not 
				formatted. */
   int lowPopThunder = 10; /* Threshold below which weather values are not
			      formatted. */
   int f_isDayTime = 0; /* Flag denoting if period is in day time. */
   int f_isNightTime = 0; /* Flag denoting if period is in day time. */
   int f_iconToBlizz = 0;

   /* If we have two 12-hour periods, we need to deterine which periods 
    * correspond to the day and which correspond to night time.  Then we use 
    * the correct icon.
    */
   if (strcmp(frequency, "12 hourly") == 0)
   {
      /* If we have two 12-hour periods lets make one of them correspond to 
       * night and we will display night icons. 
       */
      if (timeLayoutHour == 6)
      {
         /* Day periods are those divisible by 2. */
	 if (dayIndex % 2 == 0)
         {
            f_isDayTime = 1;
	    f_isNightTime = 0;
	 }
	 else
         {
            f_isDayTime = 0;
	    f_isNightTime = 1;
	 }
      }
      else if (timeLayoutHour == 18)
      {
         /* Night periods are those divisible by 2. */
	 if (dayIndex % 2 == 0)
         {
            f_isDayTime = 0;
	    f_isNightTime = 1;
	 }
	 else
         {
            f_isDayTime = 1;
	    f_isNightTime = 0;
	 }
      }
      else
         printf ("ERROR: period hour is not 6 or 18. \n");
   }
   else if (strcmp (frequency, "24 hourly") == 0)
   {
      /* With only one 24 hour period, we will use all day icons. */
      f_isDayTime = 1;
      f_isNightTime = 0;
   }
   else
      printf ("ERROR: format is not 12 hourly or 24 hourly. \n");

   /* Check for the different types of weather, and generate the corresponding
    * icon links and weather phrases. 
    */

   /* Check for FOG. */   
   if (strcmp(dominantWeather[2], "F") == 0 && (percentTimeWithFog >= 50))
   {
      if (f_isDayTime)
      {
         if (averageSkyCover[dayIndex] > 50) 
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "fg.jpg");
         else
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "sctfg.jpg");
      }             
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 50)
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nfg.jpg");
         else
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nbknfg.jpg");
      }         
      
      /* This type has an icon. */
      f_noIcon = 0;
  
      if (strcmp(dominantWeather[1], "+") == 0) 
         strcpy (phrase[dayIndex], "Dense Fog");
      else
      {
         if (strcmp(dominantWeather[0], "Patchy") == 0)
            strcpy (phrase[dayIndex], "Patchy Fog");
	 else if (strcmp(dominantWeather[0], "Areas") == 0)
           strcpy (phrase[dayIndex], "Areas Fog");
	 else
           strcpy (phrase[dayIndex], "Fog");
      }
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;
   }      

   /* Check for BLOWING SNOW. */   
   if (strcmp(dominantWeather[2], "BS") == 0)
   {
      /* Snow Words. */
      determineIconUsingPop(iconInfo[dayIndex].str, "blizzard", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);

      strcpy (phrase[dayIndex], "Blowing Snow");

      /* Since dominant weather type is "BS", check to see if all conditions
       * needed to determine "Blizzard" occurred simaltaneously some time in the 
       * summarization period. "Blizzard" will then become weather phrase.
       */
      if (blizzCnt > 0)
      {
         f_iconToBlizz = 0;
         blizzardCheck(blizzCnt, periodStartTime, periodEndTime, 
                       blizzardTime, numRowsWS, numRowsWG, wsInfo, 
                       wgInfo, dayIndex, &f_iconToBlizz, phrase);
      }
 
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for DUST. */   
   if (strcmp(dominantWeather[2], "BD") == 0)
   {
      /* Dust Words. */
     if (f_isDayTime) 
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "du.jpg");      
     else if (f_isNightTime)
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "ndu.jpg");

     strcpy (phrase[dayIndex], "Blowing Dust");
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for SAND. */   
   if (strcmp(dominantWeather[2], "BN") == 0)
   {
      /* Sand Words. */
     if (f_isDayTime) 
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "du.jpg");      
     else if (f_isNightTime)
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "ndu.jpg");

     strcpy (phrase[dayIndex], "Blowing Sand");
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for HAZE. */   
   if (strcmp(dominantWeather[2], "H") == 0)
   {
      if (strcmp(dominantWeather[0], "Patchy") == 0)
         strcpy (phrase[dayIndex], "Patchy Haze");
      else if (strcmp(dominantWeather[0], "Areas") == 0)
         strcpy (phrase[dayIndex], "Areas Haze");
      else
         strcpy (phrase[dayIndex], "Haze");
      
      /* This type has NO icon. */
      f_noIcon = 1;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for ICE CRYSTALS. */   
   if (strcmp(dominantWeather[2], "IC") == 0)
   {
      if (strcmp(dominantWeather[0], "Patchy") == 0)
         strcpy (phrase[dayIndex], "Patchy Ice Crystals");
      else if (strcmp(dominantWeather[0], "Areas") == 0)
         strcpy (phrase[dayIndex], "Areas Ice Crystals");
      else
         strcpy (phrase[dayIndex], "Ice Crystals");
      
      /* This type has NO icon. */
      f_noIcon = 1;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for ICE FOG. */   
   if (strcmp(dominantWeather[2], "IF") == 0)
   {
     if (f_isDayTime) 
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "fg.jpg"); 
     else if (f_isNightTime) 
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nfg.jpg"); 
      
     if (strcmp(dominantWeather[0], "Patchy") == 0)
        strcpy (phrase[dayIndex], "Patchy Ice Fog");
     else if (strcmp(dominantWeather[0], "Areas") == 0)
        strcpy (phrase[dayIndex], "Areas Ice Fog");
     else
        strcpy (phrase[dayIndex], "Ice Fog");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for FREEZING FOG. */   
   if (strcmp(dominantWeather[2], "ZF") == 0)
   {
     if (f_isDayTime) 
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "fg.jpg"); 
     else if (f_isNightTime) 
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nfg.jpg"); 
      
     if (strcmp(dominantWeather[0], "Patchy") == 0)
        strcpy (phrase[dayIndex], "Patchy Freezing Fog");
     else if (strcmp(dominantWeather[0], "Areas") == 0)
        strcpy (phrase[dayIndex], "Areas Freezing Fog");
     else
        strcpy (phrase[dayIndex], "Freezing Fog");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for BLOWING SAND. */   
   if (strcmp(dominantWeather[2], "ZY") == 0)
   {
      strcpy (phrase[dayIndex], "Freezing Spray");

      /* This type has NO icon. */
      f_noIcon = 1;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for SMOKE. */   
   if (strcmp(dominantWeather[2], "K") == 0)
   {
     if (f_isDayTime) 
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "fu.jpg"); 
     else if (f_isNightTime) 
        sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nfu.jpg"); 

     if (strcmp(dominantWeather[0], "Patchy") == 0)
        strcpy (phrase[dayIndex], "Patchy Smoke");
     else if (strcmp(dominantWeather[0], "Areas") == 0)
        strcpy (phrase[dayIndex], "Areas Smoke");
     else
        strcpy (phrase[dayIndex], "Smoke");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for FROST. */   
   if (strcmp(dominantWeather[2], "FR") == 0)
   {
     if (strcmp(dominantWeather[0], "Patchy") == 0)
        strcpy (phrase[dayIndex], "Patchy Frost");
     else if (strcmp(dominantWeather[0], "Areas") == 0)
        strcpy (phrase[dayIndex], "Areas Frost");
     else
        strcpy (phrase[dayIndex], "Frost");

      /* This type has NO icon. */
      f_noIcon = 1;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for VOLCANIC ASH. */   
   if (strcmp(dominantWeather[2], "VA") == 0)
   {
     if (strcmp(dominantWeather[0], "Patchy") == 0)
        strcpy (phrase[dayIndex], "Patchy Ash");
     else if (strcmp(dominantWeather[0], "Areas") == 0)
        strcpy (phrase[dayIndex], "Areas Ash");
     else
        strcpy (phrase[dayIndex], "Volcanic Ash");

      /* This type has NO icon. */
      f_noIcon = 1;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for SLEET. */   
   if (strcmp(dominantWeather[2], "IP") == 0 && maxDailyPop[dayIndex] >= 
       lowPopThreshold)
   {
      determineIconUsingPop(iconInfo[dayIndex].str, "ip", ".jpg",
	                    maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Sleet");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Sleet");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Sleet Likely");        
      else
         strcpy (phrase[dayIndex], "Sleet");
 
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   }

   /* Check for RAIN SHOWERS. */   
   else if (strcmp(dominantWeather[2], "RW") == 0 && maxDailyPop[dayIndex] >= 
       lowPopThreshold)
   {
      if (f_isDayTime)
      {
         if (averageSkyCover[dayIndex] > 50)
            determineIconUsingPop(iconInfo[dayIndex].str, "hi_shwrs", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
	 else
            determineIconUsingPop(iconInfo[dayIndex].str, "shra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 50)
            determineIconUsingPop(iconInfo[dayIndex].str, "hi_nshwrs", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
	 else
            determineIconUsingPop(iconInfo[dayIndex].str, "nra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Rain Showers");
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Rain Showers");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Rain Showers Likely");        
      else
         strcpy (phrase[dayIndex], "Rain Showers");         
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }

   /* Check for RAIN. */   
   else if (strcmp(dominantWeather[2], "R") == 0 && maxDailyPop[dayIndex] >= 
       lowPopThreshold)
   {

      if (f_isDayTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "ra", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
      else if (f_isNightTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "nra", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Rain");
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Rain");         
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Rain Likely");        
      else
         strcpy (phrase[dayIndex], "Rain");

      /* Overide phrase if Qualifer justifies it. */
      if ((strstr(dominantWeather[4], "heavy rain") != '\0'))       
         strcpy (phrase[dayIndex], "Heavy Rain");	
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }         

   /* Check for DRIZZLE. */
   else if (strcmp(dominantWeather[2], "L") == 0 && maxDailyPop[dayIndex] >= 
       lowPopThreshold)
   {

      /* Icon URL is not based on POP. If coverage is Slight Chance, do not
       * format drizzle for the icon, but default to a cloud/wind/temp phrase.
       */
      if  ((f_isDayTime) && (strcmp(dominantWeather[0], "SChc") != 0))
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "ra.jpg");
         f_noIcon = 0;
      }
      else if ((f_isNightTime) && (strcmp(dominantWeather[0], "SChc") != 0))
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nra.jpg");
         f_noIcon = 0;
      }
      else  /* Denote "no icon" to default to cloud/wind/temp phrase. */ 
         f_noIcon = 1;

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Drizzle");
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Drizzle");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Drizzle Likely");        
      else
         strcpy (phrase[dayIndex], "Drizzle");
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }

   /* Check for SNOW SHOWERS. */
   else if ((strcmp(dominantWeather[2], "SW") == 0) && 
            (strcmp(dominantWeather[1], "--") != 0) &&
	    (maxDailyPop[dayIndex] >= lowPopThreshold))

   {
      if (f_isDayTime)
      {
         if (averageSkyCover[dayIndex] > 50)
            determineIconUsingPop(iconInfo[dayIndex].str, "sn", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
	 else
            determineIconUsingPop(iconInfo[dayIndex].str, "sn", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 50)
            determineIconUsingPop(iconInfo[dayIndex].str, "nsn", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
	 else
            determineIconUsingPop(iconInfo[dayIndex].str, "nsn", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Snow Showers");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Snow Showers");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Snow Showers Likely");        
      else
         strcpy (phrase[dayIndex], "Snow Showers");
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }

   /* Check for FLURRIES. */
   else if ((strcmp(dominantWeather[2], "S") == 0 || 
            strcmp(dominantWeather[2], "SW") == 0) &&
            (strcmp(dominantWeather[1], "--") == 0) &&
	    (maxDailyPop[dayIndex] >= lowPopThreshold))
   {

      /* Icon URL is not based on POP. If coverage is Slight Chance, do not
       * format flurries for the icon, but default to a cloud/wind/temp phrase.
       */
      if ((f_isDayTime) && (strcmp(dominantWeather[0], "SChc") != 0))
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "sn.jpg");
         f_noIcon = 0;
      }
      else if ((f_isNightTime) && (strcmp(dominantWeather[0], "SChc") != 0))
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nsn.jpg");
         f_noIcon = 0;
      }
      else  /* Denote "no icon" to default to cloud/wind/temp phrase. */ 
         f_noIcon = 1;

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Flurries");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Flurries");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Flurries Likely");        
      else
         strcpy (phrase[dayIndex], "Flurries");
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }

   /* Check for SNOW/BLIZZARD. */
   else if (strcmp(dominantWeather[2], "S") == 0 &&
	    maxDailyPop[dayIndex] >= lowPopThreshold)
   {
      if (f_isDayTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "sn", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
      else if (f_isNightTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "nsn", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Snow"); 
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Snow");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Snow Likely");        
      else
         strcpy (phrase[dayIndex], "Snow");

      /* Since dominant weather type is "S", check to see if all conditions
       * needed to determine "Blizzard" occurred simaltaneously some time in the 
       * summarization period. "Blizzard" will then become weather phrase.
       */
      if (blizzCnt > 0)
      {
         f_iconToBlizz = 0;
         blizzardCheck(blizzCnt, periodStartTime, periodEndTime, 
                       blizzardTime, numRowsWS, numRowsWG, wsInfo, 
                       wgInfo, dayIndex, &f_iconToBlizz, phrase);
      } 
      if (f_iconToBlizz) /* Make icon "blizzard" too. */
         determineIconUsingPop(iconInfo[dayIndex].str, "blizzard", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
         
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }
  
   /* Check for a mixture of RAIN and SNOW. */
   if ((f_isRain || f_isRainShowers) && (f_isSnow || f_isSnowShowers) && 
       (maxDailyPop[dayIndex] >= lowPopThreshold))
   {
      if (f_isDayTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "rasn", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
      else if (f_isNightTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "nrasn", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Rain/Snow");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Rain/Snow");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Rain/Snow Likely");        
      else
         strcpy (phrase[dayIndex], "Rain/Snow");
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }

   /* Check for FREEZING RAIN. */
   if (strcmp(dominantWeather[2], "ZR") == 0 &&
      maxDailyPop[dayIndex] >= lowPopThreshold)
   {
      determineIconUsingPop(iconInfo[dayIndex].str, "fzra", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Freezing Rain");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Freezing Rain");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Freezing Rain Likely");        
      else
         strcpy (phrase[dayIndex], "Freezing Rain");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }
      
   /* Check for FREEZING DRIZZLE. */
   else if (strcmp(dominantWeather[2], "ZL") == 0 &&
      maxDailyPop[dayIndex] >= lowPopThreshold)
   {

      /* Icon URL is based on POP. */
      determineIconUsingPop(iconInfo[dayIndex].str, "fzra", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Freezing Drizzle"); 
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Freezing Drizzle");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Freezing Drizzle Likely");        
      else
         strcpy (phrase[dayIndex], "Freezing Drizzle");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }
      
   /* Check for a combination resulting in a WINTRY MIX. */
   if ((f_isFreezingDrizzle || f_isFreezingRain) && 
      (f_isSnow || f_isSnowShowers) && 
      (maxDailyPop[dayIndex] >=lowPopThreshold))
   {
      determineIconUsingPop(iconInfo[dayIndex].str, "mix", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Wintry Mix");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Wintry Mix");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Wintry Mix Likely");        
      else
         strcpy (phrase[dayIndex], "Wintry Mix");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }
      
   /* Check for RAIN & FREEZING RAIN mixture. */
   else if ((f_isRain || f_isRainShowers) && 
           (f_isFreezingDrizzle || f_isFreezingRain) && 
	   (maxDailyPop[dayIndex] >= lowPopThreshold))
   {
      determineIconUsingPop(iconInfo[dayIndex].str, "fzra", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Rain/Freezing Rain");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Rain/Freezing Rain");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Rain/Freezing Rain Likely");        
      else
         strcpy (phrase[dayIndex], "Rain/Freezing Rain");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }
      
   /* Check for a combination resulting in a WINTRY MIX. */
   else if ((f_isIcePellets) && (f_isFreezingDrizzle || f_isFreezingRain) &&
            (maxDailyPop[dayIndex] >= lowPopThreshold))
   {
      determineIconUsingPop(iconInfo[dayIndex].str, "mix", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Wintry Mix");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Wintry Mix");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Wintry Mix Likely");        
      else
         strcpy (phrase[dayIndex], "Wintry Mix");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }
      
   /* Check for a RAIN & SLEET mixture. */     
   else if ((f_isIcePellets) && (f_isRain || f_isRainShowers) && 
	   (maxDailyPop[dayIndex] >= lowPopThreshold))
   {
      if (f_isDayTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "raip", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
      else if (f_isNightTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "nraip", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Rain/Sleet");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Rain/Sleet");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Rain/Sleet Likely");        
      else
         strcpy (phrase[dayIndex], "Rain/Sleet");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }
      
   /* Check for a SNOW & SLEET mixture. */
   else if ((f_isIcePellets) && (f_isSnow || f_isSnowShowers) && 
	   (maxDailyPop[dayIndex] >= lowPopThreshold))
   {
      determineIconUsingPop(iconInfo[dayIndex].str, "ip", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Snow/Sleet");  
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Snow/Sleet");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Snow/Sleet Likely");        
      else
         strcpy (phrase[dayIndex], "Snow/Sleet");

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }

   /* Check for THUNDERSTORMS. */
   if (strcmp(dominantWeather[2], "T") == 0 && maxDailyPop[dayIndex] >= 
       lowPopThunder)
   {
      if (f_isDayTime)
      {
         if (averageSkyCover[dayIndex] > 50)
            determineIconUsingPop(iconInfo[dayIndex].str, "tsra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
         else
            determineIconUsingPop(iconInfo[dayIndex].str, "scttsra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }	      	 
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 50)
            determineIconUsingPop(iconInfo[dayIndex].str, "ntsra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
         else
            determineIconUsingPop(iconInfo[dayIndex].str, "nscttsra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }

      if (strcmp(dominantWeather[0], "Iso") == 0)
         strcpy (phrase[dayIndex], "Isolated Thunderstorms");	
      else if (strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Slight Chance Thunderstorms");	      	 
      else if (strcmp(dominantWeather[0], "Chc") == 0)
         strcpy (phrase[dayIndex], "Chance Thunderstorms");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Thunderstorms Likely");        
      else
         strcpy (phrase[dayIndex], "Thunderstorms");
      
      /* Override the thunderstorm phrase to "Severe Tstms" if the wx qualifier
       * justifies it (we have already translated the ugly string qualifier, so
       * use regular English terms). Also, override the thunderstorm phrase to 
       * "Severe Tstms" if the wx intensity justifies it. 
       */
      if ((strstr(dominantWeather[4], "damaging winds") != '\0') || 
          (strstr(dominantWeather[4], "large hail") != '\0') || 
          (strstr(dominantWeather[4], "tornado") != '\0') ||
          (strcmp(dominantWeather[1], "+") == 0))
             strcpy (phrase[dayIndex], "Severe Tstms");            

      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;
   }

   /* Check for WATER SPOUTS. */
   if (strcmp(dominantWeather[2], "WP") == 0)
   {
      strcpy (phrase[dayIndex], "Water Spouts");         	   
      
      /* This type has NO icon. */
      f_noIcon = 1;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;
   }

/*****************************************************************************/

   /* If weather type does not support an icon, see if there is a sky condition,
    * or extreme temperature, or extreme wind icon we can provide.
    */
   if (f_noIcon)
   {
      if (averageSkyCover[dayIndex] < 0)
      { 
          strcpy (iconInfo[dayIndex].str, "none");
          strcpy (phrase[dayIndex], "none");
      }
      else
      {
         skyPhrase(maxSkyCover, minSkyCover, averageSkyCover, dayIndex, 
	   	   f_isDayTime, f_isNightTime, maxSkyNum, minSkyNum, 
		   startPositions, endPositions, baseURL, &(iconInfo[0]), 
		   phrase);
      }

      tempExtremePhrase(f_isDayTime, periodMaxTemp, dayIndex, baseURL, 
		        &(iconInfo[0]), phrase);

      windExtremePhrase(f_isDayTime, f_isNightTime, dayIndex, baseURL, 
		        springDoubleDate, fallDoubleDate, maxWindSpeed, 
			maxWindDirection, periodMaxTemp, &(iconInfo[0]), 
                        phrase, maxWindSpeedValTimes);
   }
   
   return;
}
