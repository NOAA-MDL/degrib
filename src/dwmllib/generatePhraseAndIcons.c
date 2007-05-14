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
 *                      coverage [2], and qualifier [3], for each day that is 
 *                      considered the dominant one. This is the summarized 
 *                      weather for the 24 or 12 hour period. (Input)
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
 *     integerTime = Number of seconds since 1970 to when the data is valid.
 *                   Allows the code to know if this data belongs in the current
 *                   period being processed. Info is used in the generation of 
 *                   weather as a spring/fall signifier. (Input)
 * integerStartUserTime = The beginning of the first forecast period (06 hr or 18hr) 
 *                        based on the user supplied startTime argument. (Input)   
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
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  HISTORY:
 *  6/2006 Paul Hershberg (MDL): Created.
 *  9/2006 Paul Hershberg (MDL): Added functionality to add Pops to the icons
 *                               (i.e., ra.jpg --> ra50.jpg)
 *  
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void generatePhraseAndIcons (int dayIndex, char *frequency, 
                             int timeLayoutHour, char *dominantWeather[4],
			     char *baseURL, int *maxDailyPop, 
			     int *averageSkyCover, int *maxSkyCover,
			     int *minSkyCover, int *maxSkyNum, 
			     int *minSkyNum, int *periodMaxTemp, 
			     double springDoubleDate, 
			     double fallDoubleDate,  int *maxWindSpeed, 
			     int *maxWindDirection, int integerTime, 
			     int integerStartUserTime, int *startPositions, 
			     int *endPositions, int f_isDrizzle, 
			     int f_isRain, int f_isRainShowers, 
			     int f_isIcePellets, int f_isSnow, 
			     int f_isSnowShowers, int f_isFreezingDrizzle, 
			     int f_isFreezingRain, int f_isBlowingSnow, 
                             icon_def *iconInfo, char **phrase, 
                             int *f_popIsNotAnIssue)
{
    int f_noIcon = 1; /* Flag used to track if a weather icon is possible. */
    int lowPopThreshold = 20; /* Threshold below which weather values are not 
				 formatted. */
    int lowPopThunder = 10; /* Threshold below which weather values are not
			       formatted. */
    int f_isDayTime = 0; /* Flag denoting if period is in day time. */
    int f_isNightTime = 0; /* Flag denoting if period is in day time. */

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
         /* Day periods are ones divisible by 2. */
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
         /* Night periods are ones divisible by 2. */
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

   /* Check for the differnt types of weather, and generate the corresponding
    * icon links and weather phrases. 
    */
   /* Check for FOG. */   
   if (strcmp(dominantWeather[2], "F") == 0)
   {
      if (f_isDayTime) 
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "fg.jpg"); 
      else if (f_isNightTime) 
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nfg.jpg"); 
      
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
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for DUST. */   
   if (strcmp(dominantWeather[2], "BD") == 0)
   {
      /* Snow Words. */
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "du.jpg");      
      strcpy (phrase[dayIndex], "Blowing Dust");
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1;      
   } 

   /* Check for SAND. */   
   if (strcmp(dominantWeather[2], "BN") == 0)
   {
      /* Snow Words. */
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "du.jpg");      
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
      
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "hazy.jpg");      
      
      /* This type has an icon. */
      f_noIcon = 0;
      
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
      
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "hazy.jpg");      
      
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
     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "smoke.jpg"); 
      
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
         if (averageSkyCover[dayIndex] > 60)
            determineIconUsingPop(iconInfo[dayIndex].str, "hi_shwrs", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
	 else
            determineIconUsingPop(iconInfo[dayIndex].str, "shra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 60)
            determineIconUsingPop(iconInfo[dayIndex].str, "hi_nshwrs", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
	 else
            determineIconUsingPop(iconInfo[dayIndex].str, "nra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }

      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
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

      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Chance Rain");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Rain Likely");        
      else
         strcpy (phrase[dayIndex], "Rain");

      if (strcmp(dominantWeather[3], "HvyRn") == 0)
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
      if (f_isDayTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "drizzle", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
      else if (f_isNightTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "drizzle", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);

      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Chance Drizzle");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Drizzle Likely");        
      else
         strcpy (phrase[dayIndex], "Drizzle");
      
      /* This type has an icon. */
      f_noIcon = 0;
      
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
         if (averageSkyCover[dayIndex] > 60)
            determineIconUsingPop(iconInfo[dayIndex].str, "sn", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
	 else
            determineIconUsingPop(iconInfo[dayIndex].str, "sn", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 60)
            determineIconUsingPop(iconInfo[dayIndex].str, "nsn", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
	 else
            determineIconUsingPop(iconInfo[dayIndex].str, "nsn", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }
  
      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
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
      if (f_isDayTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "flurries", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
      else if (f_isNightTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "flurries", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
  
      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Chance Flurries");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Flurries Likely");        
      else
         strcpy (phrase[dayIndex], "Flurries");
      
      /* This type has an icon. */
      f_noIcon = 0;
      
      /* If weather type was found, then PoP threshold is not an issue. */
      *f_popIsNotAnIssue = 1; 
   }

   /* Check for SNOW. */
   else if (strcmp(dominantWeather[2], "S") == 0 &&
	    maxDailyPop[dayIndex] >= lowPopThreshold)
   {
      if (f_isDayTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "sn", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
      else if (f_isNightTime)
         determineIconUsingPop(iconInfo[dayIndex].str, "nsn", ".jpg", 
			       maxDailyPop[dayIndex], baseURL);
  
      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Chance Snow");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Snow Likely");        
      else
         strcpy (phrase[dayIndex], "Snow");
      
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
  
      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
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
      determineIconUsingPop(iconInfo[dayIndex].str, "freezingrain", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);
  
      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
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
      determineIconUsingPop(iconInfo[dayIndex].str, "fdrizzle", ".jpg", 
			    maxDailyPop[dayIndex], baseURL);
  
      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
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
  
      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
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
  
      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
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
         if (averageSkyCover[dayIndex] > 60)
            determineIconUsingPop(iconInfo[dayIndex].str, "tsra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
         else
            determineIconUsingPop(iconInfo[dayIndex].str, "scttsra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }	      	 
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 60)
            determineIconUsingPop(iconInfo[dayIndex].str, "ntsra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
         else
            determineIconUsingPop(iconInfo[dayIndex].str, "nscttsra", ".jpg", 
			          maxDailyPop[dayIndex], baseURL);
      }	      	 

      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Chance Thunderstorms");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Thunderstorms Likely");        
      else
         strcpy (phrase[dayIndex], "Thunderstorms");
			 
      if ((strcmp(dominantWeather[3], "DmgW") == 0) || 
         (strcmp(dominantWeather[3], "LgA") == 0) ||
         (strcmp(dominantWeather[3], "TOR") == 0)) 
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
      skyPhrase(maxSkyCover, minSkyCover, averageSkyCover, dayIndex, 
		f_isDayTime, f_isNightTime, maxSkyNum, minSkyNum, 
		startPositions, endPositions, baseURL, &(iconInfo[0]), 
		phrase);
      
      tempExtremePhrase(f_isDayTime, periodMaxTemp, dayIndex, baseURL, 
		        &(iconInfo[0]), phrase);

      windExtremePhrase(f_isDayTime, f_isNightTime, dayIndex, baseURL, 
		        springDoubleDate, fallDoubleDate, maxWindSpeed, 
			maxWindDirection, integerTime, integerStartUserTime, 
			periodMaxTemp, &(iconInfo[0]), phrase);
   }
   
   return;
}
