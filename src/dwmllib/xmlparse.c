/******************************************************************************
 * xmlparse.c
 *
 * DESCRIPTION/PURPOSE
 *  This file contains all code that is common to xmlparse.c used to 
 *  generate the formatted DWMLgen (Digital Weather Markup Language, a type of
 *  XML) products "time-series" and "glance". Two DWMLgenByDay products will 
 *  also be generated, one with a format summarization = "12 hourly", and 
 *  another with a format summarization = "24 hourly". This compiled C code 
 *  will mirror the php coded web service in which requests for NDFD data over 
 *  the internet are sent back in DWML format.
 *         
 * HISTORY
 *  12/2005 Created by Arthur Taylor/Paul Hershberg.
 *   5/2006 Bailing Li, Arthur Taylor, John Schattel: Code Review 1.
 *  
 ******************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "xmlparse.h"
#include "genprobe.h"
#include "grpprobe.h"
#include "clock.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif
#include "myassert.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "myutil.h"
#include "solar.h"

/* Set all choices for the period names for those elements needing them in the 
 * time layouts. 
 */

enum
{ earlyMorning, morning12, afternoon12, earlyMorningMaxT, earlyMorningMinT,
   morning24, afternoon24, MAX_PERIODS
};

typedef struct                /* Denotes structure of the time layouts. */
{
   int period;
   uChar numRows;
   char fmtdStartTime[30];
} layouts;

typedef struct                /* Denotes structure of icon info. */
{
   double validTime;
   char str[500];
   sChar valueType;
} icon_def;

typedef struct                /* Sky cover info used in derivation of icons
                               * and weather. */
{
   double validTime;
   int data;
   sChar valueType;
} sky_def;

typedef struct                /* Wind Speed info used in derivation of icons
                               * & weather. */
{
   double validTime;
   int data;
   sChar valueType;
} WS;

typedef struct                /* Hourly Temp info used in derivation of icons 
                               * & weather. */
{
   double validTime;
   int data;
   sChar valueType;
} TEMP;

typedef struct                /* Weather info used in derivation of icons and 
                               * weather. */
{
   double validTime;
   char str[600];
   sChar valueType;
} WX;

/*****************************************************************************
 * windExtremePhrase() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   This code determines which sky cover phrase should be used and
 *   then assigns an icon to correspond to it.  It follows the 
 *   algorithm developed by Mark Mitchell for use in the forecast
 *   at a glance product on the NWS web site.
 *
 * ARGUMENTS
 *         dayIndex = Indicates which day is being processed. (Input)
 *             phrase = Array containing the short current conditions phrase 
 *                      (i.e rain or partly cloudy) for each day. (Output)
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
 *                      (Input) 
 *        integerTime = Number of seconds since 1970 to when the data is valid.
 *                      Allows the code to know if this data belongs in the current
 *                      period being processed. (Input)
 * integerStartUserTime = Number of seconds since 1970 to when the user 
 *                        established periods being. (Input)  
 *                                                                 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void windExtremePhrase(int f_isDayTime, int f_isNightTime, 
		              int dayIndex, char *baseURL, 
			      double springDoubleDate, 
			      double fallDoubleDate, int *maxWindSpeed, 
			      int *maxWindDirection, int integerTime, 
			      int integerStartUserTime, int *periodMaxTemp, 
			      icon_def *iconInfo, char **phrase)

{
   int WINDY = 25; /* Windy threshold. */
   int BREEZY = 15; /* Breezy threshold. */
   int NORTHEAST = 60; /* NE Wind directions from which BLUSTERY phrase is 
			* created. */
   int NORTHWEST = 300; /* NW Wind directions from which BLUSTERY phrase is 
			 * created. */

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
               (integerStartUserTime <= springDoubleDate && 
	       integerTime >= fallDoubleDate) &&
               (periodMaxTemp[dayIndex] < 32))
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

/*****************************************************************************
 * tempExtremePhrase () -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   This code determines if the day time temperatures is either Hot or Cold 
 *   and then assigns an icon to correspond to it. It follows the algorithm 
 *   developed by Mark Mitchell for use in the forecast at a glance product 
 *   on the NWS web site.
 *
 * ARGUMENTS
 *         dayIndex = Indicates which day is being processed. (Input)
 *             phrase = Array containing the short current conditions phrase 
 *                      (i.e rain or partly cloudy) for each day. (Output)
 *           iconInfo = Array containing the link to a current conditions icon.
 *                      (Output)
 *            baseURL = String value holding the path to the icons.  The
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
 *  6/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void tempExtremePhrase(int f_isDayTime, int *periodMaxTemp, 
		              int dayIndex, char *baseURL,
			      icon_def *iconInfo, char **phrase)
{
   int HOT = 95; /* Temp determining if "Hot" is formatted as weather. */
   int COLD = 32; /* Temp determining if "Cold" is formatted as weather. */

   /* Lets process the case for HOT or COLD day time temperatures. */
   if (periodMaxTemp[dayIndex] != 999)
   {
      if ((periodMaxTemp[dayIndex] > HOT) && f_isDayTime)
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "hot.jpg"); 
         strcpy (phrase[dayIndex], "Hot");
      }

      if ((periodMaxTemp[dayIndex] < COLD) && f_isDayTime)
      {
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "cold.jpg"); 
         strcpy (phrase[dayIndex], "Cold");
      }
   }

   return;

}       

/*****************************************************************************
 * skyPhrase() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   This code determines which sky coverr parameters, double lat, double lon, 
			     int numDays, s phrase should be used and
 *   then assigns an icon to correspond to it.  It follows the 
 *   algorithm developed by Mark Mitchell for use in the forecast
 *   at a glance product on the NWS web site.
 *
 * ARGUMENTS
 *           dayIndex = Indicates which day is being processed. (Input)
 *             phrase = Array containing the short current conditions phrase 
 *                      (i.e rain or partly cloudy) for each day. (Output)
 *           iconInfo = Array containing the link to a current conditions icon.
 *                      (Output)
 *  f_popIsNotAnIssue = Flag denoting if PoP is very low, we won't format 
 *		        the weather values that might be present. (Output)                   
 *          frequency = Describes the two DWMLgenByDay product and they're type
 *                      of summarizations. (Input)            
 *     timeLayoutHour = The time period's hour for the 12 hourly product. Used 
 *                      to determine if it is night or day (should = 6 or 18).
 *                      (Input)                   
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
 *        f_isDayTime = Flag denoting if period is in day time. (Input)
 *      f_isNightTime = Flag denoting if period is in night time. (Input)
 *        f_isDrizzle = Flag denoting if weather is drizzle. (Input)
 *           f_isRain = Flag denoting if weather is rain. (Input)
 *    f_isRainShowers = Flag denoting if weather is rain showers. (Input)
 *          f_isSnow  = Flag denoting if weather is snow. (Input)
 *    f_isSnowShowers =  Flag denoting if weather is snow showers. (Input)
 * f_isFreezingDrizzle = Flag denoting if weather is frz drizzle. (Input)
 *   f_isFreezingRain = Flag denoting if weather is frz rain. (Input)
 *     f_isIcePellets =  Flag denoting if weather is ice pellets. (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void skyPhrase(int *maxSkyCover, int *minSkyCover, int *averageSkyCover, 
		      int dayIndex, int f_isDayTime, int f_isNightTime, 
		      int *maxSkyNum, int *minSkyNum, int *startPositions, 
		      int *endPositions, char *baseURL, icon_def *iconInfo, 
		      char **phrase)

{
   int i;
   int categoryChange = 0; /* The difference between the category of the max and 
			* min sky cover.
			*/
   int maxCategory = 0; /* The max sky cover category. */
   int minCategory = 0; /* The min sky cover category. */
   int avgCategory = 0; /* The average of the max and min sky cover categories. */
   int skyTrend_Periods = 0; /* Constant used to indicate the period before 
			      * which sky trend information (increasing clouds)
			      * will be determined. After this period no trend 
			      * information is provided.
			      */
   int trend_inc_late; /* Used to detect clouds that are increasing late in the 
			* period. */
   int trend_inc_early; /* Used to detect clouds that are increasing early in 
			 * the period. */
   int trend_speed; /* Indicates how much does the sky cover change over the 
		     * period being processed. */
   int trend_dec_late; /* Used to detect clouds that are decreasing late in the 
			* period. */
   int trend_dec_early; /* Used to detect clouds that are decreasing early in 
			 * the period. */
   double catDiff = 0.0; /* Difference between max and min categories / 2. Used
			  * for input to myRound().
			  */

   /* The image arrays. */
   char *daySkyImage[5]; /* Array containing the daylight name of the image used 
			  * at for each sky cover category (i.e. skc.jpg).
			  */
   char *nightSkyImage[5]; /* Array containing the name of the image used 
			    * at night for each sky cover category 
			    * (i.e. nskc.jpg).
			    */
   /* The phrase arrays. */
   char *daySkyPhrase[5]; /* Array containing the word associated with a 
			   * particular sky cover category used during the 
			   * day (i.e. sunny).
			   */
   char *nightSkyPhrase[5]; /* Array containing the word associated with a 
			     * particular sky cover category used during the 
			     * night (i.e. clear).
			     */  
   
   /* Initialize the 5 element daySkyImage array. */
      daySkyImage[0] = (char *) malloc((8) * sizeof(char));
      strcpy (daySkyImage[0], "skc.jpg");
      daySkyImage[1] = (char *) malloc((8) * sizeof(char));
      strcpy (daySkyImage[1], "few.jpg");
      daySkyImage[2] = (char *) malloc((8) * sizeof(char));
      strcpy (daySkyImage[2], "sct.jpg");
      daySkyImage[3] = (char *) malloc((8) * sizeof(char));
      strcpy (daySkyImage[3], "bkn.jpg");
      daySkyImage[4] = (char *) malloc((8) * sizeof(char));
      strcpy (daySkyImage[4], "ovc.jpg");
   
   /* Initialize the 5 element nightSkyImage array. */
      nightSkyImage[0] = (char *) malloc((9) * sizeof(char));
      strcpy (nightSkyImage[0], "nskc.jpg");
      nightSkyImage[1] = (char *) malloc((9) * sizeof(char));
      strcpy (nightSkyImage[1], "nfew.jpg");
      nightSkyImage[2] = (char *) malloc((9) * sizeof(char));
      strcpy (nightSkyImage[2], "nsct.jpg");
      nightSkyImage[3] = (char *) malloc((9) * sizeof(char));
      strcpy (nightSkyImage[3], "nbkn.jpg");
      nightSkyImage[4] = (char *) malloc((9) * sizeof(char));
      strcpy (nightSkyImage[4], "novc.jpg");
   
   /* Initialize th e 5 element daySkyPhrase array. */
      daySkyPhrase[0] = (char *) malloc((6) * sizeof(char));
      strcpy (daySkyPhrase[0], "Sunny");
      daySkyPhrase[1] = (char *) malloc((13) * sizeof(char));
      strcpy (daySkyPhrase[1], "Mostly Sunny");
      daySkyPhrase[2] = (char *) malloc((13) * sizeof(char));
      strcpy (daySkyPhrase[2], "Partly Sunny");
      daySkyPhrase[3] = (char *) malloc((14) * sizeof(char));
      strcpy (daySkyPhrase[3], "Mostly Cloudy");
      daySkyPhrase[4] = (char *) malloc((7) * sizeof(char));
      strcpy (daySkyPhrase[4], "Cloudy");
   
   /* Initialize the 5 element nightSkyPhrase array. */
      nightSkyPhrase[0] = (char *) malloc((6) * sizeof(char));
      strcpy (nightSkyPhrase[0], "Clear");
      nightSkyPhrase[1] = (char *) malloc((13) * sizeof(char));
      strcpy (nightSkyPhrase[1], "Mostly Clear");
      nightSkyPhrase[2] = (char *) malloc((14) * sizeof(char));
      strcpy (nightSkyPhrase[2], "Partly Cloudy");
      nightSkyPhrase[3] = (char *) malloc((14) * sizeof(char));
      strcpy (nightSkyPhrase[3], "Mostly Cloudy");
      nightSkyPhrase[4] = (char *) malloc((7) * sizeof(char));
      strcpy (nightSkyPhrase[4], "Cloudy");

   /* Calculate the change in cloud category 
    * (skyChange never used in John's code??). */

   /* Deterimine Maximum Cloud Categories. */
   if (maxSkyCover[dayIndex] <= 15)
      maxCategory = 0;
   else if (maxSkyCover[dayIndex] <= 39)
      maxCategory = 1;
   else if (maxSkyCover[dayIndex] <= 69)
      maxCategory = 2;
   else if (maxSkyCover[dayIndex] <= 90)
      maxCategory = 3;
   else if (maxSkyCover[dayIndex] <= 100)
      maxCategory = 4;

   /* Deterimine Minimum Cloud Categories. */
   if (minSkyCover[dayIndex] <= 15)
      minCategory = 0;
   else if (minSkyCover[dayIndex] <= 39)
      minCategory = 1;
   else if (minSkyCover[dayIndex] <= 69)
      minCategory = 2;
   else if (minSkyCover[dayIndex] <= 90)
      minCategory = 3;
   else if (minSkyCover[dayIndex] <= 100)
      minCategory = 4;

   /* Calculate the change in cloud categories and the average cloud amount
    * for this day.
    */
   categoryChange = abs(maxCategory - minCategory);
   catDiff = (double)((maxCategory + minCategory) / 2.0);
   avgCategory = (int)myRound(catDiff, 0);
   
   if ((dayIndex > skyTrend_Periods) || (categoryChange < 2))
   {
      if ((averageSkyCover[dayIndex] <= 15) && f_isDayTime)
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "skc.jpg");        
	  strcpy (phrase[dayIndex], "Sunny");
      }
      else if ((averageSkyCover[dayIndex] <= 15) && f_isNightTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nskc.jpg");        
	  strcpy (phrase[dayIndex], "Clear");
      }
      else if ((averageSkyCover[dayIndex] < 40) && f_isDayTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "few.jpg");        
	  strcpy (phrase[dayIndex], "Mostly Sunny");
      }
      else if ((averageSkyCover[dayIndex] < 40) && f_isNightTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nfew.jpg");        
	  strcpy (phrase[dayIndex], "Mostly Clear");
      }
      else if ((averageSkyCover[dayIndex] < 70) && f_isDayTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "sct.jpg");        
      strcpy (phrase[dayIndex], "Partly Cloudy");
      }
      else if ((averageSkyCover[dayIndex] < 70) && f_isNightTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nsct.jpg");        
	  strcpy (phrase[dayIndex], "Partly Cloudy");
      }
      else if ((averageSkyCover[dayIndex] <= 90) && f_isDayTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "bkn.jpg");        
	  strcpy (phrase[dayIndex], "Mostly Cloudy");
      }
      else if ((averageSkyCover[dayIndex] <= 90) && f_isNightTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nbkn.jpg");        
	  strcpy (phrase[dayIndex], "Mostly Cloudy");
      }
      else if ((averageSkyCover[dayIndex] <= 101) && f_isDayTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "ovc.jpg");        
	  strcpy (phrase[dayIndex], "Cloudy");
      }
      else if ((averageSkyCover[dayIndex] <= 101) && f_isNightTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "novc.jpg");        
	  strcpy (phrase[dayIndex], "Cloudy");
      }
   }
   else
   {

      /* Increasing clouds. */
      if ((minSkyNum[dayIndex] < maxSkyNum[dayIndex]) && (minSkyNum[dayIndex] != -999))
      {
         trend_speed = maxSkyNum[dayIndex] - minSkyNum[dayIndex];
	 trend_inc_early = minSkyNum[dayIndex] - startPositions[dayIndex];
	 trend_inc_late = endPositions[dayIndex] - maxSkyNum[dayIndex];

	 if (trend_inc_late < 0)
            trend_inc_late = 0;
	 if (trend_inc_early < 0)
            trend_inc_early = 0;
	 
         /* Clouds increasing over a duration of 4 or more hours. */
	 if (trend_speed >= 4)
	 {
            if (maxCategory > 2)
	    {
	       strcpy (phrase[dayIndex], "Increasing Clouds");
	       if (f_isDayTime)
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          daySkyImage[avgCategory]);
               else if (f_isNightTime)
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          nightSkyImage[avgCategory]);
	    }
	    else
	    {
	       if ((averageSkyCover[dayIndex] <= 15) && f_isDayTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "skc.jpg");
	          strcpy (phrase[dayIndex], "Sunny");
	       }
	       else if ((averageSkyCover[dayIndex] <= 15) && f_isNightTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "nskc.jpg");
	          strcpy (phrase[dayIndex], "Clear");
	       }
	       else if ((averageSkyCover[dayIndex] < 40) && f_isDayTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "few.jpg");
	          strcpy (phrase[dayIndex], "Mostly Sunny");
	       }
	       else if ((averageSkyCover[dayIndex] < 40) && f_isNightTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "nfew.jpg");
	          strcpy (phrase[dayIndex], "Mostly Clear");
	       }
 	       else if ((averageSkyCover[dayIndex] < 70) && f_isDayTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "sct.jpg");
	          strcpy (phrase[dayIndex], "Partly Cloudy");
	       }
	       else if ((averageSkyCover[dayIndex] < 70) && f_isNightTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "nsct.jpg");
	          strcpy (phrase[dayIndex], "Partly Cloudy");
	       }
 	       else if ((averageSkyCover[dayIndex] <= 90) && f_isDayTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "bkn.jpg");
	          strcpy (phrase[dayIndex], "Mostly Cloudy");
	       }
	       else if ((averageSkyCover[dayIndex] <= 90) && f_isNightTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "nbkn.jpg");
	          strcpy (phrase[dayIndex], "Mostly Cloudy");
	       }
 	       else if ((averageSkyCover[dayIndex] <= 101) && f_isDayTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "ovc.jpg");
	          strcpy (phrase[dayIndex], "Cloudy");
	       }
	       else if ((averageSkyCover[dayIndex] <= 101) && f_isNightTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "novc.jpg");
	          strcpy (phrase[dayIndex], "Cloudy");
	       }
	    }
	 }
	 else if (trend_speed < 4)
         {
		 
            /* Clouds increasing over a duration of less than 4 hours. */
            if (trend_inc_early < 4)
            {
               if (f_isDayTime)
               {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
	                  daySkyImage[maxCategory]);
	          strcpy (phrase[dayIndex], daySkyPhrase[maxCategory]);
               }
               else if (f_isNightTime)
               {
                 sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          nightSkyImage[maxCategory]);
	          strcpy (phrase[dayIndex], nightSkyPhrase[maxCategory]);
               }
	    }
            else if (trend_inc_late < 4)
            {
               if (f_isDayTime)
               {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
	                  daySkyImage[avgCategory]);
	          strcpy (phrase[dayIndex], daySkyPhrase[avgCategory]);
               }
	       else if (f_isNightTime)
               {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          nightSkyImage[avgCategory]);
	          strcpy (phrase[dayIndex], nightSkyPhrase[avgCategory]);
               }
	    }
	    else
	    {
	       if (f_isDayTime)
               {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          daySkyImage[avgCategory]);
	          strcpy (phrase[dayIndex], daySkyPhrase[avgCategory]);
               }
               else if (f_isNightTime)
               {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          nightSkyImage[avgCategory]);
	          strcpy (phrase[dayIndex], nightSkyPhrase[avgCategory]);
               }
	    }

            /* AltWords Overrides. */
            if (maxCategory == 4)
	       strcpy (phrase[dayIndex], "Becoming Cloudy");
	       
	 } /* Close "increasing over a duration of less than 4 hours". */

      } /* Close "Increasing Clouds" bracket. */
      else if (maxSkyNum[dayIndex] < minSkyNum[dayIndex])	      
      {
         /* Decreasing Clouds. */
	 trend_speed = minSkyNum[dayIndex] - maxSkyNum[dayIndex]; 
	 trend_dec_early = maxSkyNum[dayIndex] - startPositions[dayIndex]; 
	 trend_dec_late = endPositions[dayIndex] - minSkyNum[dayIndex];

	 if (trend_dec_late < 0)
            trend_dec_late = 0;
         if (trend_dec_early < 0)
            trend_dec_early = 0;

         if (categoryChange >= 3)
         {  
	    /* Decrease of 3 or more categories in sky condition. Use 
	     * clearing/gradual clearing wording.
	     */
            if (trend_dec_early < 4) 
	    {	    
               /* Cloudy/MoCloudy...then clearing early. */
               if (trend_speed < 4) 
	       {
		  if (f_isDayTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		             daySkyImage[3]);
		  else if (f_isNightTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		             nightSkyImage[3]);
			  
	          strcpy (phrase[dayIndex], "Clearing");
	       }
	       else if (trend_speed >= 4)
	       {
	          if (f_isDayTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		             daySkyImage[3]);
		  else if (f_isNightTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		             nightSkyImage[3]);
			  
	          strcpy (phrase[dayIndex], "Gradual Clearing");
	       }
	    }
	    else if (trend_dec_late < 4)
	    {
               /* Cloudy...then clearing late. */
	       if (f_isDayTime)
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          daySkyImage[maxCategory]);
	       else if (f_isNightTime)
	          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          nightSkyImage[maxCategory]);
			  
	       strcpy (phrase[dayIndex], "Clearing Late");
	    }
	    else
	    {
               /* Cloudy...then clearing mid-period. */
               if (trend_speed < 4) 
               {
                  if (f_isDayTime) 
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
			     "bkn.jpg"); 
	          else if (f_isNightTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
			     "nbkn.jpg"); 

	          strcpy (phrase[dayIndex], "Clearing");
	       }
	       else if (trend_speed >= 4)
	       {
                  if (f_isDayTime) 
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
			     "bkn.jpg"); 
	          else if (f_isNightTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
			     "nbkn.jpg"); 

	          strcpy (phrase[dayIndex], "Gradual Clearing");
	       }

	    }

	 }
	 else if (trend_speed >= 4)
         {
            /* Clouds decreasing over a duration of 4 or more hours. */
            if (f_isDayTime)
            {
	       strcpy (phrase[dayIndex], "Decreasing Clouds");
	       sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "bkn.jpg");
	    } 
            else if (f_isNightTime)
            {
	       strcpy (phrase[dayIndex], "Decreasing Clouds");
	       sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "bkn.jpg");
	    } 
	 }
	 else if (trend_speed < 4)
         {
            /* Clouds decreasing over a duration of less than 4 hours. */
            if (trend_dec_early < 4) 
            {         
               if (f_isDayTime)
               { 
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          daySkyImage[minCategory]);
 	          strcpy (phrase[dayIndex], daySkyPhrase[minCategory]);
	       }
	       else if (f_isNightTime)
               { 
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          nightSkyImage[minCategory]);
 	          strcpy (phrase[dayIndex], nightSkyPhrase[minCategory]);
	       }
	    }
	    else
            {
               if (f_isDayTime)
               { 
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          daySkyImage[avgCategory]);
 	          strcpy (phrase[dayIndex], daySkyPhrase[avgCategory]);
	       }
               if (f_isNightTime)
               { 
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          nightSkyImage[avgCategory]);
 	          strcpy (phrase[dayIndex], nightSkyPhrase[avgCategory]);
	       }
	       
	    }
	    
	 }

         /* AltWords Overrides. */
         if ((minCategory == 0) && (f_isDayTime))  
	    strcpy (phrase[dayIndex], "Becoming Sunny");

      }
   }

   /* Free some things. */
   for (i = 0; i < 5; i++)
   {	   
      free(daySkyImage[i]);
      free(nightSkyImage[i]);
      free(daySkyPhrase[i]);
      free(nightSkyPhrase[i]);
   }      

   return;
}   

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
 *         dayIndex = Indicates which day is being processed. (Input)
 *             phrase = Array containing the short current conditions phrase 
 *                      (i.e rain or partly cloudy) for each day. (Output)
 *           iconInfo = Array containing the link to a current conditions icon.
 *                      (Output)
 *  f_popIsNotAnIssue = Flag denoting if PoP is very low, we won't format 
 *		        the weather values that might be present. (Output)                   
 *          frequency = Describes the two DWMLgenByDay product and they're type
 *                      of summarizations. (Input)            
 *     timeLayoutHour = The time period's hour for the 12 hourly product. Used 
 *                      to determine if it is night or day (should = 6 or 18).
 *                      (Input)                   
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
 *                   period being processed. (Input)
 * integerStartUserTime = Number of seconds since 1970 to when the user 
 *                        established periods being. (Input) 
 *        f_isDrizzle = Flag denoting if weather is drizzle. (Input)
 *           f_isRain = Flag denoting if weather is rain. (Input)
 *    f_isRainShowers = Flag denoting if weather is rain showers. (Input)
 *          f_isSnow  = Flag denoting if weather is snow. (Input)
 *    f_isSnowShowers = Flag denoting if weather is snow showers. (Input)
 * f_isFreezingDrizzle = Flag denoting if weather is frz drizzle. (Input)
 *   f_isFreezingRain = Flag denoting if weather is frz rain. (Input)
 *     f_isIcePellets = Flag denoting if weather is ice pellets. (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void generatePhraseAndIcons (int dayIndex, char *frequency, 
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
				    int f_isFreezingRain, icon_def *iconInfo, 
				    char **phrase, int *f_popIsNotAnIssue)

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

   /* Check for the differnt types of weather, and generate the corresponfing
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
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "blizzard.jpg");      
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
      strcpy (phrase[dayIndex], "Sleet");
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "ip.jpg"); 
 
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
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "hi_shwrs.jpg");
	 else
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "shra.jpg");
      }
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 60)
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "hi_nshwrs.jpg");
	 else
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nra.jpg");
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
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "ra.jpg");
      else if (f_isNightTime)
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nra.jpg");

      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Chance Rain");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Rain Likely");        
      else
         strcpy (phrase[dayIndex], "Rain");

      if (strcmp(dominantWeather[4], "HvyRn") == 0)
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
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "drizzle.jpg");
      else if (f_isNightTime)
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "drizzle.jpg");

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
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "sn.jpg");
	 else
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "sn.jpg");
      }
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 60)
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nsn.jpg");
	 else
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nsn.jpg");
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
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "flurries.jpg");      
      else if (f_isNightTime)
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "flurries.jpg");       
  
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
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "sn.jpg");      
      else if (f_isNightTime)
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nsn.jpg");  	   
  
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
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "rasn.jpg");      
      else if (f_isNightTime)
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nrasn.jpg");  
  
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
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "freezingrain.jpg"); 
  
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
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "fdrizzle.jpg"); 
  
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
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "mix.jpg"); 
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
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "fzra.jpg"); 
  
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
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "mix.jpg"); 
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
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "raip.jpg");      
      else if (f_isNightTime)
         sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nraip.jpg");  
  
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
      sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "ip.jpg"); 
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
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "tsra.jpg");
         else
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "scttsra.jpg"); 
      }	      	 
      else if (f_isNightTime)
      {
         if (averageSkyCover[dayIndex] > 60)
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "ntsra.jpg");
         else
            sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nscttsra.jpg"); 
      }	      	 

      if (strcmp(dominantWeather[0], "Chc") == 0 || strcmp(dominantWeather[0], "SChc") == 0)
         strcpy (phrase[dayIndex], "Chance Thunderstorms");
      else if (strcmp(dominantWeather[0], "Lkly") == 0)
         strcpy (phrase[dayIndex], "Thunderstorms Likely");        
      else
         strcpy (phrase[dayIndex], "Thunderstorms");
			 
      if ((strcmp(dominantWeather[4], "DmgW") == 0) || 
         (strcmp(dominantWeather[4], "LgA") == 0) ||
         (strcmp(dominantWeather[4], "TOR") == 0)) 
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

/*****************************************************************************
 * isDominant() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   Routine takes two weather arguments and determines if the 
 *   first one is dominant.
 *
 * ARGUMENTS
 *        arg1 = The first argument, to be tested whether or not it dominates 
 *               the second argument. (Input)
 *        arg2 = The second argument, compared to the first. (Input)
 *     argType = Denotes which weather type we are comparing (coverage, 
 *                intensity, or type). (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static int isDominant(char *arg1, char *arg2, char *argType)

{
   if (strcmp(argType, "coverage") == 0)
   {
      if (strcmp(arg1, "none") == 0)
         return 0;
      else if (strcmp(arg1, "Patchy") == 0)
      {
         if (strcmp("none", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Areas") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Brf") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Inter") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Pds") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Ocnl") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Frq") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Iso") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "SChc") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Sct") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Chc") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0 ||
	    strcmp("Sct", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Num") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0 ||
	    strcmp("Sct", arg2) == 0 || strcmp("Chc", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Lkly") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0 ||
	    strcmp("Sct", arg2) == 0 || strcmp("Chc", arg2) == 0 || 
	    strcmp("Num", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Wide") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0 ||
	    strcmp("Sct", arg2) == 0 || strcmp("Chc", arg2) == 0 || 
	    strcmp("Num", arg2) == 0 || strcmp("Lkly", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Def") == 0)
      {
         if (strcmp("Def", arg2) != 0)
    	    return 1;
	 else
            return 0;
      }
   }
   else if (strcmp(argType, "intensity") == 0)
   {
      if (strcmp(arg1, "none") == 0)
         return 0;
      else if (strcmp(arg1, "--") == 0)
      {
         if (strcmp("none", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "-") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("--", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "m") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("--", arg2) == 0 ||
            strcmp("-", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "+") == 0)
      {
         if (strcmp("+", arg2) != 0)
	     return 1;
	 else
	    return 0;
      }
   }
   else if (strcmp(argType, "type") == 0)
   {
      if (strcmp(arg1, "none") == 0)
         return 0;
      else if (strcmp(arg1, "F") == 0)
      {
         if (strcmp("none", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "BS") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "BD") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "BN") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "H") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "K") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "FR") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "VA") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "L") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "RW") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "R") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "IC") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "IF") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "SW") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "S") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "IP") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "ZF") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "ZY") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0 || strcmp("ZF", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "ZL") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0 || strcmp("ZF", arg2) == 0 ||
	    strcmp("ZY", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "ZR") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0 || strcmp("ZF", arg2) == 0 ||
	    strcmp("ZY", arg2) == 0 || strcmp("ZL", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "T") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0 || strcmp("ZF", arg2) == 0 ||
	    strcmp("ZY", arg2) == 0 || strcmp("ZL", arg2) == 0 ||
	    strcmp("ZR", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "WP") == 0)
      {
         if (strcmp("WP", arg2) != 0)
    	    return 1;
	 else
            return 0;
      }
   }
   return 0;
}

/*****************************************************************************
 * determinePeriodLength() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Determines the period length of an element. Time between the first validTime
 *  and the second validTime is used in the determination.
 *
 * ARGUMENTS
 *        startTime = First valid time of element. (Input)
 *          endTime = Second valid time of element. (Input)
 *    parameterName = Number denoting the NDFD element currently processed. 
 *                    (Output) 
 *          numRows = Number of data rows (values) for an element. (Input)
 *           period = Length between an elements successive validTimes (Input).
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: int period
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static int determinePeriodLength(double startTime, double endTime,
                                 uChar numRows, uChar parameterName)
{
   int period = 0;            /* Length between an elements successive
                               * validTimes. */

   /* If there is just one row of data, guess the period length. */
   if (numRows == 1)
   {
      if (parameterName == NDFD_MAX || parameterName == NDFD_MIN)
         period = 24;
      else if (parameterName == NDFD_POP)
         period = 12;
      else if (parameterName == NDFD_QPF || parameterName == NDFD_SNOW)
         period = 6;
      else
         period = 3;

      return period;
   }

   period = (endTime - startTime) / 3600;

   return period;
}

/*****************************************************************************
 * formatValidTime() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Transform double validTime to character string in form 
 *  (2006-04-13T:00:00:00-00:00), which is the standard form in the formatted
 *  XML.
 *   
 * ARGUMENTS
 *        validTime = Incoming double validTime to be converted. (Input)
 *         timeBuff = Returned time counterpart in character form
 *                    (2006-04-13T:00:00:00-00:00). (Input/Output)
 *    size_timeBuff = Max size of "timeBuff". (Input)
 *                    (Input) 
 *    pntZoneOffset = Number of hours to add tocurrent time to get GMT time. 
 *                    (Input)
 *       f_dayCheck = Flag determining if current point observes Daylight 
 *                    Savings Time. (Input) 
 *        
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or -1)
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static int formatValidTime(double validTime, char *timeBuff,
                           int size_timeBuff, sChar pntZoneOffSet,
                           sChar f_dayCheck)
{

   char zone[7];
   double localTime;          /* validTime with the time zone difference
                               * taken into account. */

   localTime = validTime - (pntZoneOffSet * 3600);

   /* localTime is now in point's local standard time */

   if (f_dayCheck)
   {
      /* Note: a zero is passed to DaylightSavings so it converts from local
       * to local standard time. */

      if (Clock_IsDaylightSaving2(localTime, 0) == 1)
      {
         localTime += 3600;
         pntZoneOffSet--;
      }
   }

   /* Sort by valid time. */

   myAssert(size_timeBuff > 25);
   if (size_timeBuff <= 25)
      return -1;

   /* The '0, 0' is passed in because we already converted to local standard
    * time. */

   Clock_Print2(timeBuff, size_timeBuff, localTime, "%Y-%m-%dT%H:%M:%S", 0, 0);

   /* Change definition of pntZoneOffSet */
   pntZoneOffSet = -1 * pntZoneOffSet;
   if (pntZoneOffSet < 0)
      sprintf(zone, "-%02d:00", -1 * pntZoneOffSet);
   else
      sprintf(zone, "+%02d:00", pntZoneOffSet);
   strcat(timeBuff, zone);
   return 0;
}

/*****************************************************************************
 * computeStartEndTimes() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Determines all the start and end times for all validTime.              
 *
 * ARGUMENTS
 *       startTimes = Character array holding all the start Times an element 
 *                    is valid for. (Output)
 *         endTimes = Character array holding all the end Times and element
 *                    is valid for. (Output)
 *   numPeriodNames = Number period names for one of the seven issuance times.
 *                    (Input)
 *    parameterName = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *          numRows = Number of data rows (values) for an element. (Input)
 *         TZoffset = Number of hours to add to current time to get GMT time. 
 *                    (Input)
 *     f_observeDST = Flag determining if current point observes Daylight 
 *                    Savings Time. (Input)  
 *     periodLength = Length between an elements successive validTimes (Input).
 *        frequency = Set to "boggus" for DWMLgen products, and to "12 hourly" 
 *                    or "24 hourly" for the DWMLgenByDay products.  
 *            match = Pointer to the array of element matches from degrib. 
 *                    (Input) 
 *         numMatch = The number of matches from degrib. (Input)
 *      useEndTimes = Flag denoting if element uses end times in the output XML 
 *                    (Input)
 *   numRowsSkipped = The number of rows skipped when a startTime is chosen on
 *                    the command line argument and a row of data falls between 
 *                    this time and the time of the first forecast period. 
 *                    (Output)
 *  
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void computeStartEndTimes(uChar parameterName, uChar numRows,
                                 int periodLength, sChar TZoffset,
                                 sChar f_observeDST, genMatchType * match,
                                 size_t numMatch, uChar useEndTimes,
                                 char **startTimes, char **endTimes, 
				 char *frequency, uChar f_XML, 
				 double startTime_cml, double currentDoubTime,
				 int *numPopRowsSkippedBeginning)
{

   int i;
   int timeCounter = -1;      /* Counts number of times the start (or end) 
			         times were created using actual data. */
   int deltaSeconds = 0;
   char str1[30];             /* Returned character string holding valid
                               * time. */
   char *pstr;                /* Pointer string used to denote the "T" in the 
                               * validTime string. */
   int oneDay = (24 * 60 * 60); /* # seconds in 24 hours. */
   char temp[3];              /* Temporary string buffer. */
   char temp2[5];             /* Temporary string buffer. */
   int beginningHour;         /* Beginning hour of validTime being processed. 
                               */
   int priorElemCount;        /* Counter used to find elements' location in
                               * match. */
   double Time_doub = 0.0; /* Holds startTime as a double. */
   
   if (f_XML == 1 || f_XML == 2) /* DWMLgen products. */
   {
      switch (parameterName)
      {

         /* For max and min, the period length appears to be 24 hours long.
          * But, in reality the max and min temp only apply to a 12/13 hour
          * period. So we have to reset the start time.  Additionally, max.
          * is valid for a 0700 - 1900 and min on for a 1900 - 0800 local
          * standard time period. */

         case NDFD_MAX:

            /* Loop over matches of the data. */
            priorElemCount = 0;
            for (i = 0; i < numMatch; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName)
               {
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);
                  startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                  if (useEndTimes)
                     endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);

                  /* For daylight saving time points we need to adjust local
                   * time to standard time. */

                  if (f_observeDST)
                  {
                     /* See if it is currently Daylight Savings Time */
                     if (Clock_IsDaylightSaving2(match[i].validTime, TZoffset)
                         == 1)
                     {
                        /* To accommodate daylight saving time move time 
			 * forward.
			 */
                       if (useEndTimes)
                       {
                          pstr = strstr(str1, "T");
                          strncpy(pstr, "T20", 3);
                          strcpy(endTimes[i - priorElemCount], str1);
                       }

                       /* Since the period for the min spans two days, the
                        * start time will need the previous day. */

                       pstr = strstr(str1, "T");
                       strncpy(pstr, "T08", 3);
                       strcpy(startTimes[i - priorElemCount], str1);
                     }
                     else
                     {
                        /* Use standard time definition of Max Temp. */
                        if (useEndTimes)
                        {
                           pstr = strstr(str1, "T");
                           strncpy(pstr, "T19", 3);
                           strcpy(endTimes[i - priorElemCount], str1);
                        }

                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T07", 3);
                        strcpy(startTimes[i - priorElemCount], str1);
                     }
                  }
                  else
                  {
                     if (useEndTimes)
                     {
                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T19", 3);
                        strcpy(endTimes[i - priorElemCount], str1);
                     }

                     pstr = strstr(str1, "T");
                     strncpy(pstr, "T07", 3);
                     strcpy(startTimes[i - priorElemCount], str1);

                  }
               }
               else
                  priorElemCount += 1;
            }
            break;

         case NDFD_MIN:

            /* Loop over matches of the data. */

            priorElemCount = 0;
            for (i = 0; i < numMatch; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName)
               {
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);
                  startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                  if (useEndTimes)
                     endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);

                  /* For daylight saving time points we need to adjust local
                   * time to standard time. */
                  if (f_observeDST)
                  {
                     /* See if it is currently Daylight Savings Time */

                     if (Clock_IsDaylightSaving2(match[i].validTime, TZoffset)
                         == 1)
                     {
			     
                        /* To accommodate daylight saving time move time 
			 * forward.
			 */
                        if (useEndTimes) 
                        {
                           pstr = strstr(str1, "T");
                           strncpy(pstr, "T09", 3);
                           strcpy(endTimes[i - priorElemCount], str1);
                        }

                        /* Since the period for the min spans two days, the
                         * start time will need the previous days date. */

                        formatValidTime((match[i].validTime - oneDay), str1, 30,
                                        TZoffset, f_observeDST);

                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T20", 3);
                        strcpy(startTimes[i - priorElemCount], str1);

                     }
                     else
                     {
                        /* Use standard time definition of Min Temp. */

                        if (useEndTimes)
                        {
                           pstr = strstr(str1, "T");
                           strncpy(pstr, "T08", 3);
                           strcpy(endTimes[i - priorElemCount], str1);
                        }

                        /* Since the period for the min spans two days, the
                         * start time will need the previous day. */

                        formatValidTime((match[i].validTime - oneDay), str1, 30,
                                        TZoffset, f_observeDST);

                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T19", 3);
                        strcpy(startTimes[i - priorElemCount], str1);

                     }
                  }
                  else
                  {
			  
                     /* Use standard time definition of Min Temp. */
		     if (useEndTimes) 
                     {
                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T08", 3);
                        strcpy(endTimes[i - priorElemCount], str1);
                     }

                     /* Since the period for the min spans two days, the start
                      * time will need the previous day. */

                     formatValidTime((match[i].validTime - oneDay), str1, 30,
                                     TZoffset, f_observeDST);

                     pstr = strstr(str1, "T");
                     strncpy(pstr, "T19", 3);
                     strcpy(startTimes[i - priorElemCount], str1);

                  }
               }
               else
                  priorElemCount += 1;
            }
            break;

         case NDFD_POP:
         case NDFD_SNOW:
         case NDFD_QPF:

            /* Loop over matches of the data. */

            priorElemCount = 0;
            for (i = 0; i < numMatch; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName)
               {
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);
		  
                  startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
		  if (useEndTimes)
		  {
                     endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);

                     /* For the NDFD, the valid time is at the end of the valid
                     * period. So end time equal to the "valid time" and calcuate
                     * the start time. */
                     strcpy(endTimes[i - priorElemCount], str1);
		  }

                  temp[0] = str1[11];
                  temp[1] = str1[12];
                  temp[2] = '\0';
                  beginningHour = atoi(temp);
                  beginningHour -= periodLength;

                  /* If the hour is negative, we moved to the previous day so
                   * determine what the new date and time are. */

                  if (beginningHour < 0)
                  {
                     beginningHour += 24;
                     formatValidTime((match[i].validTime - oneDay), str1, 30,
                                     TZoffset, f_observeDST);
                     sprintf(temp, "%d", beginningHour);
                  }

                  /* Now we assemble the start time. Need to make sure we have a 
                   * two digit hour when number is less than 10. */

                  if (beginningHour < 10)
                     sprintf(temp2, "%c%c%1d%c", 'T', '0', beginningHour, '\0');
                  else
                     sprintf(temp2, "%c%2d%c", 'T', beginningHour, '\0');

                  pstr = strstr(str1, "T");
                  strncpy(pstr, temp2, 3);
                  strcpy(startTimes[i - priorElemCount], str1);

               }
               else
                  priorElemCount += 1;
            }

            break;

         default:

            /* Loop over matches of the data. */

            priorElemCount = 0;
            for (i = 0; i < numMatch; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName)
               {

                  str1[0] = '\0';
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);
                  startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                  strcpy(startTimes[i - priorElemCount], str1);
               }
               else
                  priorElemCount += 1;
            }
            break;
      }
   }
   else if (f_XML == 3 || f_XML == 4) /* For DWMLgenByDay Products. */
   {
      if (strcmp (frequency, "24 hourly") == 0)	
      {

	 /* Process the 24 hourly case substituting an hour of "06" into 
          * the data's date and time string. All weather parameters except
          * PoP are summarized starting at 8:00 AM the day the user requested
          * data for through the next day at 8:00 AM.  If the user requested
          * more than one day, this process is repeated for each subsequent day.
	  * 
	  * If the product is "24 hourly" (f_XML = 4), the weather and icon 
	  * elements will use these start and end times (the parameterName was
	  * altered upon entering the generateTimeLayout routine). Only Pop will
	  * use the 12 hourly start and end times. 
	  */
	 switch (parameterName)
         {
            
	    case NDFD_MAX:

               /* Loop over matches of the data. */
               priorElemCount = 0;
               for (i = 0; i < numMatch; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName)
                  {
		     timeCounter ++;
		     
		     /* Build the startTimes. They occur on the current day. */
     		     formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                     f_observeDST);
                     startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                     pstr = strstr(str1, "T");
                     strncpy(pstr, "T06", 3);
                     strcpy(startTimes[i - priorElemCount], str1);
		  
		     /* Build the endTimes. They occur on the next day. */
                     if (useEndTimes)
		     {
		        formatValidTime((match[i].validTime + oneDay), str1, 30,
                                        TZoffset, f_observeDST);
                        endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T06", 3);
                        strcpy(endTimes[i - priorElemCount], str1);
		     }
		  }
		  else
                     priorElemCount ++;
	       }
	       
	       /* Check to see if there is less data than the number of rows
		* to be formatted. If so, we need to fabricate those times. */
               if (timeCounter+1 < numRows)
	       {
	          for (i = timeCounter+1; i < numRows; i++)
	          {
		     Clock_Scan(&Time_doub, startTimes[i-1], 0);
     		     formatValidTime(Time_doub + oneDay, str1, 30, TZoffset,
                                     f_observeDST);
                     startTimes[i] = malloc(strlen(str1) + 1);
                     strcpy(startTimes[i], str1);
		     
                     if (useEndTimes)
		     {
		        Clock_Scan(&Time_doub, endTimes[i-1], 0);
     		        formatValidTime(Time_doub + oneDay, str1, 30, 
					TZoffset, f_observeDST);
                        endTimes[i] = malloc(strlen(str1) + 1);
                        strcpy(endTimes[i], str1);
	             }
		  }
	       }
	 
	       break;

	    case NDFD_MIN:
		  
               /* Loop over matches of the data. */
               priorElemCount = 0;
               for (i = 0; i < numMatch; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName)
                  {
		     timeCounter++;
		     
		     /* Build the endTimes first. They occur on the current day. */
		     if (useEndTimes)
		     {
                        formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                        f_observeDST);
			
                        endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T06", 3);
                        strcpy(endTimes[i - priorElemCount], str1);
		     }
               
		     /* Build the startTimes. They occur on the previous day. */
                     formatValidTime(match[i].validTime - oneDay, str1, 30, TZoffset,
                                     f_observeDST);
                     startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                     pstr = strstr(str1, "T");
                     strncpy(pstr, "T06", 3);
                     strcpy(startTimes[i - priorElemCount], str1);
		  }
		  else
                     priorElemCount++;
	       }
	       
	       /* Check to see if there is less data than the number of rows
		* to be formatted. If so, we need to fabricate those times. */
               if (timeCounter+1 < numRows)
	       {
	          for (i = timeCounter+1; i < numRows; i++)
	          {
		     Clock_Scan(&Time_doub, startTimes[i-1], 0);
     		     formatValidTime(Time_doub + oneDay, str1, 30, TZoffset,
                                     f_observeDST);
                     startTimes[i] = malloc(strlen(str1) + 1);
                     strcpy(startTimes[i], str1);
		     
                     if (useEndTimes)
		     {
		        Clock_Scan(&Time_doub, endTimes[i-1], 0);
     		        formatValidTime(Time_doub + oneDay, str1, 30, 
					TZoffset, f_observeDST);
                        endTimes[i] = malloc(strlen(str1) + 1);
                        strcpy(endTimes[i], str1);
	             }
		  }
	       }
	 	       
	       break;
	 }
      }
      else if (strcmp (frequency, "12 hourly") == 0)
      {

        /* PoP will always be summarized from 0800 to 2000 and 2000 to 0800
	 * (both DWMLgenByDay products; f_XML = 3 or f_XML = 4). If the user 
	 * selects "12 hourly" (f_XML = 3), the other data will be summarized
	 * into these two time periods as well. Max and min temperature will
	 * be placed into the day and night periods respectively. Weather and
	 * Icons will share the start and end times as Pop. (Note, if the 
	 * product is of type f_XML = 4, the only product with 12 hourly 
	 * summarizations will be the Pop element.) 
	 */
	 switch (parameterName)
         {
            
	    case NDFD_POP:
		    
	       deltaSeconds = ((periodLength / 4) * 3600);	  
               /* Loop over matches of the data. */
               priorElemCount = 0;
               for (i = 0; i < numMatch; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName)
                  {
		     if (match[i].validTime - currentDoubTime < deltaSeconds)
		        numPopRowsSkippedBeginning++;
		     timeCounter++;
		     
                     /* For the NDFD, the valid time is at the end of the valid 
                      * period. So end time equals the "valid time". But 
		      * adjustments need to be made to the hour before the 
		      * assignment of the endTimes. 
		      */
                     formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                     f_observeDST);
                     temp[0] = str1[11];
                     temp[1] = str1[12];
                     temp[2] = '\0';
                     beginningHour = atoi(temp);
                     beginningHour -= periodLength;
		     
                     /* If the hour is negative, the endTime uses the current
		      * day, but the starTime uses the previous day so
                      * determine what the new date and time are for 
		      * assignment of the startTime.
		      */
                     if (beginningHour < 0)
		     { 
                        beginningHour = 18;
	                if (useEndTimes)
		        {
		           endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                           pstr = strstr(str1, "T");
                           strncpy(pstr, "T06", 3);
                           strcpy(endTimes[i - priorElemCount], str1);
                        }
                        /* Set up the startTime to contain previous day. */			
                        formatValidTime(match[i].validTime - oneDay, str1, 30, 
				        TZoffset, f_observeDST);
	             }
		     else
		     {
                        beginningHour = 06;
	                if (useEndTimes)
		        {
		           endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                           pstr = strstr(str1, "T");
                           strncpy(pstr, "T18", 3);
                           strcpy(endTimes[i - priorElemCount], str1);
                        }
	             }
		  
                     /*  Build the startTime. */ 
		     startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                     sprintf(temp2, "T%02d%c", beginningHour, '\0');		     
                     pstr = strstr(str1, "T");
                     strncpy(pstr, temp2, 3);
                     strcpy(startTimes[i - priorElemCount], str1);
		  }   
		  else   
		     priorElemCount++;
	       }

	       /* Check to see if there is less data than the number of rows
		* to be formatted. If so, we need to fabricate those times. */
               if (timeCounter+1 < numRows)
	       {
	          for (i = timeCounter+1; i < numRows; i++)
	          {
		     Clock_Scan(&Time_doub, startTimes[i-1], 0);
     		     formatValidTime(Time_doub + (oneDay/2), str1, 30, TZoffset,
                                     f_observeDST);
                     startTimes[i] = malloc(strlen(str1) + 1);
                     strcpy(startTimes[i], str1);
		     
                     if (useEndTimes)
		     {
		        Clock_Scan(&Time_doub, endTimes[i-1], 0);
     		        formatValidTime(Time_doub + (oneDay/2), str1, 30, 
					TZoffset, f_observeDST);
                        endTimes[i] = malloc(strlen(str1) + 1);
                        strcpy(endTimes[i], str1);
	             }
		  }
	       }
	 
	       break;

	    case NDFD_MAX:
	       
               /* Loop over matches of the data. */
               priorElemCount = 0;
               for (i = 0; i < numMatch; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName)
                  {
	             timeCounter++;

		     formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                     f_observeDST);
		     
		     /* Build StartTime. */
		     startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                     pstr = strstr(str1, "T");
                     strncpy(pstr, "T06", 3);
                     strcpy(startTimes[i - priorElemCount], str1);
                     
		     /* Build EndTime. */		     
	             if (useEndTimes)
		     {
		        endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T18", 3);
                        strcpy(endTimes[i - priorElemCount], str1);
                     }
		  }
		  else
	             priorElemCount++;
	       }

	       /* Check to see if there is less data than the number of rows
		* to be formatted. If so, we need to fabricate those times. */
               if (timeCounter+1 < numRows)
	       {
	          for (i = timeCounter+1; i < numRows; i++)
	          {
		     Clock_Scan(&Time_doub, startTimes[i-1], 0);
     		     formatValidTime(Time_doub + oneDay, str1, 30, TZoffset,
                                     f_observeDST);
                     startTimes[i] = malloc(strlen(str1) + 1);
                     strcpy(startTimes[i], str1);
		     
                     if (useEndTimes)
		     {
		        Clock_Scan(&Time_doub, endTimes[i-1], 0);
     		        formatValidTime(Time_doub + oneDay, str1, 30, 
					TZoffset, f_observeDST);
                        endTimes[i] = malloc(strlen(str1) + 1);
                        strcpy(endTimes[i], str1);
	             }
		  }
	       }
	       
	       break;

	    case NDFD_MIN:

	       /* Loop over matches of the data. */
               priorElemCount = 0;
               for (i = 0; i < numMatch; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName)
                  {
	             timeCounter++;

		     formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                     f_observeDST);
		     
		     /* Build EndTime. */		     
	             if (useEndTimes)
		     {
		        endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                        pstr = strstr(str1, "T");
                        strncpy(pstr, "T06", 3);
                        strcpy(endTimes[i - priorElemCount], str1);
                     }
		     
		     /* Build StartTime which will be in the previous day. */	
                     formatValidTime(match[i].validTime - oneDay, str1, 30, 
				     TZoffset, f_observeDST);
		     startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
                     pstr = strstr(str1, "T");
                     strncpy(pstr, "T18", 3);
                     strcpy(startTimes[i - priorElemCount], str1);
		  }
		  else
		     priorElemCount++;
	       }

	       /* Check to see if there is less data than the number of rows
		* to be formatted. If so, we need to fabricate those times. */
               if (timeCounter+1 < numRows)
	       {
	          for (i = timeCounter+1; i < numRows; i++)
	          {
		     Clock_Scan(&Time_doub, startTimes[i-1], 0);
     		     formatValidTime(Time_doub + oneDay, str1, 30, TZoffset,
                                     f_observeDST);
                     startTimes[i] = malloc(strlen(str1) + 1);
                     strcpy(startTimes[i], str1);
		     
                     if (useEndTimes)
		     {
		        Clock_Scan(&Time_doub, endTimes[i-1], 0);
     		        formatValidTime(Time_doub + oneDay, str1, 30, 
					TZoffset, f_observeDST);
                        endTimes[i] = malloc(strlen(str1) + 1);
                        strcpy(endTimes[i], str1);
	             }
		  }
	       }

	       break;
	       
	 } /* Close the switch check. */
      } /* Close the if-else "frequency ==" check. */
   } /* Close the "if f_XML ==" check. */
	       
   return;
}

/*****************************************************************************
 * getColdSeasonTimes () -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Determine the start and end times for the cold season in double form. Used 
 *  to decide if winds are blustery (cold season) or breezy (warm season).
 *
 * ARGUMENTS
 *             match = Pointer to the array of element matches from degrib. (Input) 
 *          numMatch = The number of matches from degrib. (Input)
 *          TZoffset = Number of hours to add to current time to get GMT time. 
 *                     (Input)
 *  springDoubleDate = Double end time of next cold season. (Output)
 *    fallDoubleDate = Double start time of next cold season. (Output)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void getColdSeasonTimes(genMatchType *match, size_t numMatch, 
		               sChar TZoffset, double **springDoubleDate,
			       double **fallDoubleDate)

{
   int i; /* Match structure counter. */
   int springYear_int; /* Integer year in which next cold season ends. */
   int fallYear_int; /* Integer year in which next cold season begins. */   
   char str1[30]; /* String holding formatted valid time. */
   char year[5]; /* String holding formatted year. */
   char fallYear[5]; /* String with year in which next cold season begins. */
   char springYear[5]; /* String with year in which next cold season ends. */
   char springDate[30]; /* Complete string of end time of next cold season. */
   char fallDate[30]; /* Complete string of start time of next cold season. */
   char month[4]; /* String holding formatted month. */
   char time_adj[16]; /* String component holding "T00:00:00-00:00" part. */
   
   /* Based on the current month, we need to know how to calculate the start
    * and end of the cold season (i.e. October 2004 - April 2005). Get this 
    * info using the Wind Speed validTimes.
    */ 
	
   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WS)
      {
         formatValidTime(match[i].validTime, str1, 30, 0, 0);         
         break; 
      }
   }
   
   /* Get the year and the month from the returned string. */
   year[0] = str1[0];
   year[1] = str1[1];
   year[2] = str1[2];
   year[3] = str1[3];
   year[4] = '\0';

   month[0] = str1[5];
   month[1] = str1[6];
   month[2] = '-';
   month[3] ='\0';
   
   /* Craft the hour, minute, second, and Time zone offset. */
   if (TZoffset < 0)
      sprintf (time_adj, "T01:01:01%02d:00", TZoffset);
   else
      sprintf (time_adj, "T01:01:01-%02d:00", TZoffset);

   /* Set year of beginning and end of cold season, depending on current
    * month. 
    */
   if (atoi(month) > 4)
   { 
      strcpy (fallYear, year);
      springYear_int = atoi(year); 
      springYear_int++;
      sprintf (springYear, "%d", springYear_int);
   }
   else
   {
      fallYear_int = atoi(year);
      fallYear_int--;
      sprintf (fallYear, "%d", fallYear_int);
      strcpy (springYear, year);
   }

   /* Now get the double time for the start and end of the cold season. To do
    * this, build the string up, and then send into Clock_Scan() to retrieve 
    * the time in double form. 
    */
   sprintf (springDate, "%s-04-01%s", springYear, time_adj);
   sprintf (fallDate, "%s-10-01%s", fallYear, time_adj);

   /* And in double form... */
   Clock_Scan(*springDoubleDate, springDate, 1);
   Clock_Scan(*fallDoubleDate, fallDate, 1);

   return;
} 

/*****************************************************************************
 * prepareWeatherValuesByDay() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   Prepares and alignes temperature and weather data for the weather 
 *   summarization and generation. Routine get the weather times from the match
 *   structure and transforms to double expressions. Routine also uses the MaxT
 *   and Mint elements to get the "max" temp per period.
 *   
 * ARGUMENTS
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *   frequency = The type of summarization done, if any. Set to "boggus" for 
 *               DWMLgen products, and to "12 hourly" or "24 hourly" for the 
 *               DWMLgenByDay products. (Input) 
 *     numDays = The number of days the validTimes for all the data rows 
 *               (values) consist of. Used for the DWMLgenByDay products 
 *               only. (Input)
 *    numMatch = The number of matches from degrib. (Input)
 *  numRowsSKY = The number of data rows for sky cover. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 *   numRowsWS = The number of data rows for wind speed. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 * numRowsTEMP = The number of data rows for hourly temperatures. These data 
 *               are used to derive icons, if icons are to be formatted. 
 *               (Input)
 *  numRowsPOP = The number of data rows for PoP12hr. (Input)
 *  numRowsMIN = The number of data rows for MinT. (Input)
 *  numRowsMAX = The number of data rows for MaxT.  (Input)
 *   numRowsWX = The number of data rows for weather. These data can be 
 *               formatted if f_wx = 1, and/or used to derive icons if f_icon 
 *               = 1.  (Input)
 *         pnt = The point index. (Input)
 *    TZoffset = Number of hours to add to current time to get GMT time. 
 *               (Input)
 * f_observeDST = Flag determining if current point observes Daylight 
 *                Savings Time. (Input)  
 * weatherDataTimes = In double form, the times of all the weather values.
 *                    (Output)
 * periodMaxTemp = For each forecast period, the "max" temperature occuring in
 *                 the period, based off of the MaxT and MinT elements. If night, 
 *                 the period could have a "max" MinT. (Output)
 * periodStartTimes = The startTimes of each of the forecast periods. (Output)                    * periodEndTimes =   The endTimes of each of the forecast periods. (Output)            
 * springDoubleDate = The end date of next cold season expressed in double form.
 *                    (Output)
 * fallDoubleDate = The start date of next cold season expressed in double form. 
 *                  (Output)
 * timeLayoutHour = The time period's hour for the 12 hourly product. Used to 
 *                  determine if it is night or day in the 
 *                  generatePhraseAndIcons routine (should = 6 or 18). (Output)                 
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void prepareWeatherValuesByDay (genMatchType *match, size_t numMatch, 
		                       sChar TZoffset, sChar f_observeDST, 
				       char *frequency, int numDays, 
				       int numOutputLines, uChar numRowsMIN, 
				       uChar numRowsMAX, uChar f_XML, 
				       uChar numRowsPOP, uChar numRowsWX, 
				       size_t pnt, int f_useMinTempTimes, 
				       double startTime_cml,
				       double *weatherDataTimes,
				       int *periodMaxTemp, 
				       double *periodStartTimes, 
				       double *periodEndTimes,
				       double *springDoubleDate, 
                                       double *fallDoubleDate, 
				       int *timeLayoutHour)
{
	
   int i;                     /* Index through the match structure. */
   int j;                     /* Period Counter. */   	
   int limit;   
   int priorElemCount = 0;        /* Counter used to find elements' location in
                                   * match. */
   int period = 0; /* Length between an elements successive validTimes. */
   int numPopRowsSkippedBeginning = 0;   
   double currentDoubTime;
   char **startTimesMinTemp = NULL; /* Array holding startTimes for MinT. */ 
   char **endTimesMinTemp = NULL;    /* 7-13T18:00:00-06:00 mx:9Array holding the end Times for MinT. */
   char **startTimesMaxTemp = NULL; /* Array holding start Times for MaxT. */ 
   char **endTimesMaxTemp = NULL;    /* Array holding the end Times for MaxT. */
   char **startTimesPop = NULL; /* Array holding start Times for Pop (or 12hr 
				   elements). */ 
   char **endTimesPop = NULL;    /* Array holding the end Times for Pop (or 12hr 
				   elements). */
   char maxTempDay[3]; /* 2-digit day first MaxT falls in. */
   char minTempDay[3]; /* 2-digit day first MinT falls in. */
   char WXtimeStr[30]; /* Temporary string holding formatted time
                        * value of weather. */
   char layoutHour[3]; /* Used to retrieve the timeLayoutHour. */

   /* Get the current time in seconds. */
   currentDoubTime = Clock_Seconds();
   
   /* Based on the current month, we need to know the start and end of the cold
    * season (i.e. October 2004 - April 2005). Retrieve this info in double form.
    */
   getColdSeasonTimes (match, numMatch, TZoffset, &springDoubleDate, 
		       &fallDoubleDate);

   /* Create arrays holding each period's MaxT value (this value could be 
    * a "max" minimum Temp if periods are 12 hours long) and the start and end
    * times of the periods.
    */
   if (strcmp (frequency, "24 hourly") == 0)
   {
      /* Get the MaxT values. */
      for (i = 0; i < numMatch; i++)
      {
         if (match[i].elem.ndfdEnum == NDFD_MAX)
         { 
            if (i < numDays)
            {
               if (match[i].value[pnt].valueType == 0)
                  periodMaxTemp[i] = (int)myRound(match[i].value[pnt].data, 0);
	    }
	 }
      }
      
      /* Get the start and end Times for these periods. */	   
      if (f_useMinTempTimes) /* Use the MinT's start and end times as period 
                              * times. 
                              */
      {
         startTimesMinTemp = (char **) malloc(numRowsMIN * sizeof(char *));
         endTimesMinTemp   = (char **) malloc(numRowsMIN * sizeof(char *));
         computeStartEndTimes (NDFD_MIN, numRowsMIN, 24, TZoffset, f_observeDST,
		               match, numMatch, 1, startTimesMinTemp, 
			       endTimesMinTemp, frequency, f_XML,
			       startTime_cml, currentDoubTime, 
			       &numPopRowsSkippedBeginning);
	 
	 if (numRowsMIN < numDays)
            limit = numRowsMIN;
         else
            limit = numDays;
	 
	 for (i = 0; i < limit; i++)
         {
            Clock_Scan (&periodStartTimes[i], startTimesMinTemp[i], 1);
	    Clock_Scan (&periodEndTimes[i], endTimesMinTemp[i], 1);
         }
	 
      }
      else /* Use the MaxT's start and end times as period times. */
      {
         startTimesMaxTemp = (char **) malloc(numRowsMAX * sizeof(char *));
         endTimesMaxTemp   = (char **) malloc(numRowsMAX * sizeof(char *));
         computeStartEndTimes (NDFD_MAX, numRowsMAX, 24, TZoffset, f_observeDST,
		               match, numMatch, 1, startTimesMaxTemp, 
			       endTimesMaxTemp, frequency, f_XML, 
			       startTime_cml, currentDoubTime, 
			       &numPopRowsSkippedBeginning);
		 
	 if (numRowsMAX < numDays)
            limit = numRowsMAX;
         else
            limit = numDays;
	 
	 for (i = 0; i < limit; i++)
         {
            Clock_Scan (&periodStartTimes[i], startTimesMaxTemp[i], 1);
	    Clock_Scan (&periodEndTimes[i], endTimesMaxTemp[i], 1);
         }
      }
   }
   else if (strcmp (frequency, "12 hourly") == 0)
   { 
	   
   /* We need to know which temperature to start with (max or min)
    * If the end day of the max is different from the min, then start with the max
    * otherwise start with the min since the max is for tomorrow.
    */
      startTimesMaxTemp = (char **) malloc(numRowsMAX * sizeof(char *));
      endTimesMaxTemp = (char **) malloc(numRowsMAX * sizeof(char *));
      computeStartEndTimes (NDFD_MAX, numRowsMAX, 24, TZoffset, f_observeDST,
		            match, numMatch, 1, startTimesMaxTemp, 
			    endTimesMaxTemp, frequency, f_XML, 
			    startTime_cml, currentDoubTime, 
			    &numPopRowsSkippedBeginning);
	   	   
      maxTempDay[0] = endTimesMaxTemp[0][8];
      maxTempDay[1] = endTimesMaxTemp[0][9];
      maxTempDay[2] = '\0';
      
      startTimesMinTemp = (char **) malloc(numRowsMIN * sizeof(char *));
      endTimesMinTemp = (char **) malloc(numRowsMIN * sizeof(char *));
      computeStartEndTimes (NDFD_MIN, numRowsMIN, 24, TZoffset, f_observeDST,
		            match, numMatch, 1, startTimesMinTemp, 
			    endTimesMinTemp, frequency, f_XML,
			    startTime_cml, currentDoubTime, 
			    &numPopRowsSkippedBeginning);
	   	   
      minTempDay[0] = endTimesMinTemp[0][8];
      minTempDay[1] = endTimesMinTemp[0][9];
      minTempDay[2] = '\0';
      
      if (strcmp (maxTempDay, minTempDay) == 0)
      {
	      
         /* Start with Min Temps, skipping periods since MinT's occur every
	  * 24 hours but need to be placed in appropriate, alternating
	  * periods. */ 
	 priorElemCount = 0;
         for (i = 0, j = 0; i < numMatch; i++, j=j+2)
         {
            if (match[i].elem.ndfdEnum == NDFD_MIN)
            { 
               if (i-priorElemCount < numDays)
               {
	          if (match[i].value[pnt].valueType == 0)
                     periodMaxTemp[j-(priorElemCount*2)] = 
		       (int)myRound(match[i].value[pnt].data, 0);
	       }
	    }
	    else
	       priorElemCount++;
         }

         /* Alternate with the Max Temps. */
         for (i = 0, j = 1; i < numMatch; i++, j=j+2)
         {
            if (match[i].elem.ndfdEnum == NDFD_MAX)
            { 
               if (i < numDays)
	       {
	          if (match[i].value[pnt].valueType == 0)	     
                     periodMaxTemp[j] = (int)myRound(match[i].value[pnt].data, 0);
	       }
	    }
         }

      }
      else  /* Start with Max Temps. */
      {	      
         for (i = 0, j = 0; i < numMatch; i++, j=j+2)
         {
 	    if (match[i].elem.ndfdEnum == NDFD_MAX)
            { 
               if (i < numDays)
	       {
                  if (match[i].value[pnt].valueType == 0)
      		       periodMaxTemp[j] = (int)myRound(match[i].value[pnt].data, 0);
               }	   
	    }
         }

         /* Alternate with the Min Temps. */
	 priorElemCount = 0;
         for (i = 0, j = 1; i < numMatch; i++, j=j+2)
         {
 	    if (match[i].elem.ndfdEnum == NDFD_MIN)
            { 
               if (i-priorElemCount < numDays)
	       {
                  if (match[i].value[pnt].valueType == 0)		       
                     periodMaxTemp[j-(priorElemCount*2)] = 
		        (int)myRound(match[i].value[pnt].data, 0);
	       }
	    }
	    else
	       priorElemCount++;
         }

      }
      
      /* Now, get the start and end Times for these periods. Since the periods
       * are 12 hours long now, use POP12hr element's times for period times. */
      startTimesPop = (char **) malloc(numRowsPOP * sizeof(char *));
      endTimesPop   = (char **) malloc(numRowsPOP * sizeof(char *));
      computeStartEndTimes (NDFD_POP, numRowsPOP, 12, TZoffset, f_observeDST,
                            match, numMatch, 1, startTimesPop, 
			    endTimesPop, frequency, f_XML,
			    startTime_cml, currentDoubTime, 
			    &numPopRowsSkippedBeginning);

      /* We need the unchanging time period's hour for use in the
       * generatePhraseAndIcons routine. Grab it here. 
       */
      layoutHour[0] = startTimesPop[0][11];
      layoutHour[1] = startTimesPop[0][12];
      layoutHour[2] = '\0';
      *timeLayoutHour = atoi(layoutHour);

      if (numRowsPOP < numDays*2)
         limit = numRowsPOP;
      else
         limit = numDays*2;
	 
      for (i = 0; i < limit; i++)
      {
	 Clock_Scan (&periodStartTimes[i], startTimesPop[i], 1);
 	 Clock_Scan (&periodEndTimes[i], endTimesPop[i], 1);
      }
   }

   /* Create the double representation of the weather data valid times (this is
    * an element without "end" times).
    */
   priorElemCount = 0;
   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WX)
      {
         weatherDataTimes[i-priorElemCount] = 0.0;
	 WXtimeStr[0] = '\0';
	 
         formatValidTime(match[i].validTime, WXtimeStr, 30, TZoffset,
                         f_observeDST);
         Clock_Scan (&weatherDataTimes[i-priorElemCount], WXtimeStr, 1);
	 
         /* Now we have to account for data that is just in the time
          * period i.e. if data is valid from 4PM - 7PM and time period is
          * from 6AM - 6PM. We shift data by one half the data's period in
          * seconds. */

            if (i - priorElemCount < 1)
               period = determinePeriodLength(match[i].validTime,
                                              match[i + 1].validTime, numRowsWX,
                                              NDFD_WX);
            else
               period = determinePeriodLength(match[i - 1].validTime,
                                              match[i].validTime,
                                              numRowsWX, NDFD_WX);
            weatherDataTimes[i-priorElemCount] = 
		      weatherDataTimes[i-priorElemCount]
		      - (((double)period * 0.5) * 3600);
      }   
      else
         priorElemCount++;
   }
   
   /* Free Max Temp Time arrays. */
   if (strcmp (frequency, "12 hourly") == 0 || !f_useMinTempTimes)
   {
      for (i = 0; i < numRowsMAX; i++)
      {
         free (startTimesMaxTemp[i]);
         free (endTimesMaxTemp[i]);
      }
      free (startTimesMaxTemp);
      free (endTimesMaxTemp);
   }

   /* Free Min Temp Time arrays. */   
   if (strcmp (frequency, "12 hourly") == 0 || f_useMinTempTimes)
   {
      for (i = 0; i < numRowsMIN; i++)
      {
         free (startTimesMinTemp[i]);
         free (endTimesMinTemp[i]);
      }
      free (startTimesMinTemp);
      free (endTimesMinTemp);
   }
   
   /* Free Pop Time arrays (only used for 12 hourly product). */
   if (strcmp (frequency, "12 hourly") == 0)   
   {
      for (i = 0; i < numRowsPOP; i++)
      {
         free (startTimesPop[i]);
         free (endTimesPop[i]);
      }
      free (startTimesPop);
      free (endTimesPop);
   }
     
   return;
}

/******************************************************************************
 * determineNonWeatherIcons() --
 *
 * Paul Hershberg / MDL
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
static void determineNonWeatherIcons
      (int windTimeEqualsWeatherTime, int itIsNightTime, WS *wsInfo,
       int wsIndex, char *baseURL, int numRowsWS, icon_def *iconInfo,
       int wxIndex, int numRowsTEMP, TEMP *tempInfo, int hourlyTempIndex,
       int hourlyTempTimeEqualsWeatherTime)
{

   int strongWind = 25;       /* Wind speed (knots)in which windy icon will
                               * format */
   int hotTemperature = 110;  /* Temp in which "hot" icon will format. */
   int coldTemperature = -40; /* Temp in which "cold" icon will format. */

   /* The time intervals for wind varies through the forecast. However,
    * weather does not.  So we need to syncronize the wind so the right icon
    * appears in the correct weather time interval. */

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
       * value. */

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
    * time so the right icon appears in the correc weather time interval. */

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
       * This should happen every other weather value. */

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

/******************************************************************************
 * determineSkyIcons() --
 *
 * Paul Hershberg / MDL
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
static void determineSkyIcons(int skyCoverTimeEqualsWeatherTime,
                              int itIsNightTime, int skyIndex, int wxIndex,
                              sky_def *skyInfo, icon_def *iconInfo,
                              char *baseURL, int numRowsSKY)
{

   /* The time intervals for sky cover varies through the forecast. However,
    * weather does not.  So we need to syncronize the sky cover so the right
    * icon appears in the correct weather time interval. */

   if (skyCoverTimeEqualsWeatherTime)
   {
      if (itIsNightTime)
      {
         if (skyInfo[skyIndex].data <= 6)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nskc.jpg");
         if (skyInfo[skyIndex].data > 6 && skyInfo[skyIndex].data <= 31)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nfew.jpg");
         if (skyInfo[skyIndex].data > 31 && skyInfo[skyIndex].data <= 69)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nsct.jpg");
         if (skyInfo[skyIndex].data > 69 && skyInfo[skyIndex].data <= 94)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nbkn.jpg");
         if (skyInfo[skyIndex].data > 94)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "novc.jpg");
      }
      else
      {
         if (skyInfo[skyIndex].data <= 6)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "skc.jpg");
         if (skyInfo[skyIndex].data > 6 && skyInfo[skyIndex].data <= 31)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "few.jpg");
         if (skyInfo[skyIndex].data > 31 && skyInfo[skyIndex].data <= 69)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "sct.jpg");
         if (skyInfo[skyIndex].data > 69 && skyInfo[skyIndex].data <= 94)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "bkn.jpg");
         if (skyInfo[skyIndex].data > 94)
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
       * happen every other weather value. */

      if (itIsNightTime && skyIndex > 0)
      {
         if (skyInfo[skyIndex - 1].data <= 6)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nskc.jpg");
         if (skyInfo[skyIndex - 1].data > 6 && skyInfo[skyIndex - 1].data <= 31)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nfew.jpg");
         if (skyInfo[skyIndex - 1].data > 31
             && skyInfo[skyIndex - 1].data <= 69)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nsct.jpg");
         if (skyInfo[skyIndex - 1].data > 69
             && skyInfo[skyIndex - 1].data <= 94)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nbkn.jpg");
         if (skyInfo[skyIndex - 1].data > 94)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "novc.jpg");
      }
      else if (itIsNightTime != 1 && skyIndex > 0)
      {
         if (skyInfo[skyIndex - 1].data <= 6)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "skc.jpg");
         if (skyInfo[skyIndex - 1].data > 6 && skyInfo[skyIndex - 1].data <= 31)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "few.jpg");
         if (skyInfo[skyIndex - 1].data > 31
             && skyInfo[skyIndex - 1].data <= 69)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "sct.jpg");
         if (skyInfo[skyIndex - 1].data > 69
             && skyInfo[skyIndex - 1].data <= 94)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "bkn.jpg");
         if (skyInfo[skyIndex - 1].data > 94)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ovc.jpg");
      }
   }

   return;
}

/******************************************************************************
 * determineWeatherIcons() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   This code creates links for icons portraying various weather conditions.  
 *   These icons have the hightest priority and supercede cloud cover, windy,
 *   cold and hot conditions.  Within the weather conditions group, priority
 *   is estabished by each weather conditions relative appearence in the
 *   processing flow. The highest priority conditions occur later.
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
                
 *                             
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
static void determineWeatherIcons(icon_def *iconInfo, int numGroups,
                                  char **wxType,
                                  int skyCoverTimeEqualsWeatherTime,
                                  int itIsNightTime, sky_def *skyInfo,
                                  char *baseURL, int numRowsSKY, int skyIndex,
                                  int wxIndex, int windTimeEqualsWeatherTime,
                                  WS *wsInfo, int wsIndex, int numRowsWS,
                                  int numRowsTEMP, int hourlyTempIndex,
                                  int hourlyTempTimeEqualsWeatherTime,
                                  TEMP *tempInfo)
{
   /* Initalize flags to '0' so we can selectively turn them on if the
    * conditions are occuring. */

   int f_isFog = 0;
   int f_isFreezingFog = 0;
   int f_isIceFog = 0;
   int f_isSmoke = 0;
   int f_isHaze = 0;
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
      else if (strcmp(wxType[groupIndex], "H") == 0)
         f_isHaze = 1;
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
      else if (strcmp(wxType[groupIndex], "FR") == 0)
         f_noIcon = 1;
      else
         f_noIcon = 1;
   }

   /* Now that we have one or more members of the group processed lets either 
    * create a nonWeather icon if no weather icon is possible or create the
    * appropriate weather icon. */

   if (f_noIcon)
   {

      /* Determine the conditions icon based on sky cover. */

      determineSkyIcons(skyCoverTimeEqualsWeatherTime, itIsNightTime, skyIndex,
                        wxIndex, skyInfo, &(iconInfo[0]), baseURL, numRowsSKY);

      /* Determine the conditions icon based on things like extreme
       * temperatures and strong winds. */

      determineNonWeatherIcons(windTimeEqualsWeatherTime, itIsNightTime,
                               wsInfo, wsIndex, baseURL, numRowsWS,
                               &(iconInfo[0]), wxIndex, numRowsTEMP, tempInfo,
                               hourlyTempIndex,
                               hourlyTempTimeEqualsWeatherTime);
   }

   else

   {
      /* We check for the presence of each weather type noting that order is
       * important -- first things are less important. */

      if (itIsNightTime)

      {
         if (f_isFog || f_isFreezingFog || f_isIceFog)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nfg.jpg");
         if (f_isSmoke)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "fu.jpg");
         if (f_isHaze)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "hazy.jpg");
         if (f_isBlowingDust || f_isBlowingSand)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "du.jpg");
         if (f_isBlowingSnow || f_isBlowingSand)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "blizzard.jpg");
         if (f_isDrizzle || f_isRain)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nra.jpg");

         /* The rain showers icon has a dependancy on sky cover. So, we need
          * to know if there is a corresponding sky cover. value at this
          * time. */

         if (f_isRainShowers && skyCoverTimeEqualsWeatherTime)
         {
            if (skyInfo[skyIndex].data > 60)
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nra.jpg");
            else
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "hi_nshwrs.jpg");
         }

         /* This is the case where there is no sky cover at this time. So, we 
          * use the previous sky cover which is 3 hours earlier. */

         if (f_isRainShowers && skyCoverTimeEqualsWeatherTime != 1 &&
             skyIndex > 0)
         {
            if (skyInfo[skyIndex - 1].data > 60)
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nra.jpg");
            else
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "hi_nshwrs.jpg");
         }

         if (f_isIcePellets)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ip.jpg");
         if (f_isFreezingDrizzle || f_isFreezingRain)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "fzra.jpg");
         if (f_isSnow || f_isSnowShowers)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nsn.jpg");
         if ((f_isRain || f_isRainShowers || f_isDrizzle) &&
             (f_isSnow || f_isSnowShowers))
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nrasn.jpg");
         if ((f_isFreezingRain || f_isFreezingDrizzle) &&
             (f_isSnow || f_isSnowShowers || f_isIcePellets))
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "mix.jpg");
         if ((f_isRain || f_isRainShowers || f_isDrizzle) && f_isIcePellets)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nraip.jpg");
         if ((f_isSnow || f_isSnowShowers) && f_isIcePellets)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ip.jpg");

         /* The thunderstorm icon has a dependancy on sky cover. So, we need
          * to know if there is a corresponding sky cover. value at this
          * time. */

         if (f_isThunderstorm && skyCoverTimeEqualsWeatherTime)
         {
            if (skyInfo[skyIndex].data > 60)
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ntsra.jpg");
            else
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nscttsra.jpg");
         }

         /* This is the case where there is no sky cover at this time. So, we 
          * use the previous sky cover which is 3 hours earlier. */

         if (f_isThunderstorm && skyCoverTimeEqualsWeatherTime != 1 &&
             skyIndex > 0)
         {
            if (skyInfo[skyIndex - 1].data > 60)
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ntsra.jpg");
            else
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "nscttsra.jpg");
         }
      }

      else
         /* It is day time. */
      {
         if (f_isFog || f_isFreezingFog || f_isIceFog)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "fg.jpg");
         if (f_isSmoke)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "fu.jpg");
         if (f_isHaze)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "hazy.jpg");
         if (f_isBlowingDust || f_isBlowingSand)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "du.jpg");
         if (f_isBlowingSnow || f_isBlowingSand)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "blizzard.jpg");
         if (f_isDrizzle || f_isRain)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ra.jpg");

         /* The rain showers icon has a dependancy on sky cover. So, we need
          * to know if there is a corresponding sky cover. value at this
          * time. */

         if (f_isRainShowers && skyCoverTimeEqualsWeatherTime)
         {
            if (skyInfo[skyIndex].data > 60)
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "shra.jpg");
            else
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "hi_shwrs.jpg");
         }

         /* This is the case where there is no sky cover at this time. So, we 
          * use the previous sky cover which is 3 hours earlier. */

         if (f_isRainShowers && skyCoverTimeEqualsWeatherTime != 1 &&
             skyIndex > 0)
         {
            if (skyInfo[skyIndex - 1].data > 60)
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ra.jpg");
            else
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "hi_shwrs.jpg");
         }

         if (f_isIcePellets)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ip.jpg");
         if (f_isFreezingDrizzle || f_isFreezingRain)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "fzra.jpg");
         if (f_isSnow || f_isSnowShowers)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "sn.jpg");
         if ((f_isRain || f_isRainShowers || f_isDrizzle) &&
             (f_isSnow || f_isSnowShowers))
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "rasn.jpg");
         if ((f_isFreezingRain || f_isFreezingDrizzle) &&
             (f_isSnow || f_isSnowShowers || f_isIcePellets))
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "mix.jpg");
         if ((f_isRain || f_isRainShowers || f_isDrizzle) && f_isIcePellets)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "raip.jpg");
         if ((f_isSnow || f_isSnowShowers) && f_isIcePellets)
            sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "ip.jpg");

         /* The thunderstorm icon has a dependancy on sky cover. So, we need
          * to know if there is a corresponding sky cover. value at this
          * time. */

         if (f_isThunderstorm && skyCoverTimeEqualsWeatherTime)
         {
            if (skyInfo[skyIndex].data > 60)
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "tsra.jpg");
            else
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "scttsra.jpg");
         }

         /* This is the case where there is no sky cover at this time. So, we 
          * use the previous sky cover which is 3 hours earlier. */

         if (f_isThunderstorm && skyCoverTimeEqualsWeatherTime != 1 &&
             skyIndex > 0)
         {
            if (skyInfo[skyIndex - 1].data > 60)
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "tsra.jpg");
            else
               sprintf(iconInfo[wxIndex].str, "%s%s", baseURL, "scttsra.jpg");
         }
      }                       /* End of night vs day check. */
   }                          /* End of icon vs no icon possible check. */

   return;
}

/******************************************************************************
 * getTranslatedCoverage() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE  
 *   Get the English translations for the NDFD weather coverage part of the 
 *   "ugly string". For example, NDFD coverage "SChc" is converted to its 
 *   english equivilant "slight chance".
 *
 *   
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather coverage. (Input)
 * transStr = Outgoing string with the translated coverage. (Output)
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
static void getTranslatedCoverage(char *uglyStr, char *transStr)
{

   if (strcmp(uglyStr, "SChc") == 0)
   {
      strcpy(transStr, "slight chance");
      return;
   }

   else if (strcmp(uglyStr, "Chc") == 0)
   {
      strcpy(transStr, "chance");
      return;
   }

   else if (strcmp(uglyStr, "Lkly") == 0)
   {
      strcpy(transStr, "likely");
      return;
   }

   else if (strcmp(uglyStr, "Ocnl") == 0)
   {
      strcpy(transStr, "occasional");
      return;
   }

   else if (strcmp(uglyStr, "Def") == 0)
   {
      strcpy(transStr, "definitely");
      return;
   }

   else if (strcmp(uglyStr, "Iso") == 0)
   {
      strcpy(transStr, "isolated");
      return;
   }

   else if (strcmp(uglyStr, "Sct") == 0)
   {
      strcpy(transStr, "scattered");
      return;
   }

   else if (strcmp(uglyStr, "Num") == 0)
   {
      strcpy(transStr, "numerous");
      return;
   }

   else if (strcmp(uglyStr, "Areas") == 0)
   {
      strcpy(transStr, "areas");
      return;
   }

   else if (strcmp(uglyStr, "Patchy") == 0)
   {
      strcpy(transStr, "patchy");
      return;
   }

   else if (strcmp(uglyStr, "Wide") == 0)
   {
      strcpy(transStr, "widespread");
      return;
   }

   else if (strcmp(uglyStr, "Pds") == 0)
   {
      strcpy(transStr, "periods of");
      return;
   }

   else if (strcmp(uglyStr, "Frq") == 0)
   {
      strcpy(transStr, "frequent");
      return;
   }

   else if (strcmp(uglyStr, "Inter") == 0)
   {
      strcpy(transStr, "intermittent");
      return;
   }

   else if (strcmp(uglyStr, "Brf") == 0)
   {
      strcpy(transStr, "brief");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}

/******************************************************************************
 * getTranslatedType() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE  
 *   Get the English translations for the NDFD weather type part of the 
 *   "ugly string". 
 *      
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather type. (Input)
 * transStr = Outgoing string with the translated type. (Output)
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
static void getTranslatedType(char *uglyStr, char *transStr)
{

   if (strcmp(uglyStr, "ZL") == 0)
   {
      strcpy(transStr, "freezing drizzle");
      return;
   }

   else if (strcmp(uglyStr, "ZR") == 0)
   {
      strcpy(transStr, "freezing rain");
      return;
   }

   else if (strcmp(uglyStr, "SW") == 0)
   {
      strcpy(transStr, "snow showers");
      return;
   }

   else if (strcmp(uglyStr, "BS") == 0)
   {
      strcpy(transStr, "blowing snow");
      return;
   }

   else if (strcmp(uglyStr, "BD") == 0)
   {
      strcpy(transStr, "blowing dust");
      return;
   }

   else if (strcmp(uglyStr, "RW") == 0)
   {
      strcpy(transStr, "rain showers");
      return;
   }

   else if (strcmp(uglyStr, "IP") == 0)
   {
      strcpy(transStr, "ice pellets");
      return;
   }

   else if (strcmp(uglyStr, "FR") == 0)
   {
      strcpy(transStr, "frost");
      return;
   }

   else if (strcmp(uglyStr, "R") == 0)
   {
      strcpy(transStr, "rain");
      return;
   }

   else if (strcmp(uglyStr, "S") == 0)
   {
      strcpy(transStr, "snow");
      return;
   }

   else if (strcmp(uglyStr, "T") == 0)
   {
      strcpy(transStr, "thunderstorms");
      return;
   }

   else if (strcmp(uglyStr, "L") == 0)
   {
      strcpy(transStr, "drizzle");
      return;
   }

   else if (strcmp(uglyStr, "F") == 0)
   {
      strcpy(transStr, "fog");
      return;
   }

   else if (strcmp(uglyStr, "H") == 0)
   {
      strcpy(transStr, "haze");
      return;
   }

   else if (strcmp(uglyStr, "K") == 0)
   {
      strcpy(transStr, "smoke");
      return;
   }

   else if (strcmp(uglyStr, "BN") == 0)
   {
      strcpy(transStr, "blowing sand");
      return;
   }

   else if (strcmp(uglyStr, "IC") == 0)
   {
      strcpy(transStr, "ice crystals");
      return;
   }

   else if (strcmp(uglyStr, "VA") == 0)
   {
      strcpy(transStr, "volcanic ash");
      return;
   }

   else if (strcmp(uglyStr, "WP") == 0)
   {
      strcpy(transStr, "water spouts");
      return;
   }

   else if (strcmp(uglyStr, "ZF") == 0)
   {
      strcpy(transStr, "freezing fog");
      return;
   }

   else if (strcmp(uglyStr, "IF") == 0)
   {
      strcpy(transStr, "ice fog");
      return;
   }

   else if (strcmp(uglyStr, "ZY") == 0)
   {
      strcpy(transStr, "freezing spray");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "");
      return;
   }

   return;
}

/******************************************************************************
 * getTranslatedIntensity() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE  
 *   Get the English translations for the NDFD weather intensity part of the 
 *   "ugly string". 
 *      
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather intensity. (Input)
 * transStr = Outgoing string with the translated intensity. (Output)
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
static void getTranslatedIntensity(char *uglyStr, char *transStr)
{

   if (strcmp(uglyStr, "--") == 0)
   {
      strcpy(transStr, "very light");
      return;
   }

   else if (strcmp(uglyStr, "-") == 0)
   {
      strcpy(transStr, "light");
      return;
   }

   else if (strcmp(uglyStr, "+") == 0)
   {
      strcpy(transStr, "heavy");
      return;
   }

   else if (strcmp(uglyStr, "m") == 0)
   {
      strcpy(transStr, "moderate");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}

/******************************************************************************
 * getTranslatedVisibility() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE  
 *   Get the English translations for the NDFD weather visibility part of the 
 *   "ugly string". 
 *      
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather visibility. (Input)
 * transStr = Outgoing string with the translated visibility. (Output)
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
static void getTranslatedVisibility(char *uglyStr, char *transStr)
{

   if (strcmp(uglyStr, "0SM") == 0)
   {
      strcpy(transStr, "0");
      return;
   }

   else if (strcmp(uglyStr, "1/4SM") == 0)
   {
      strcpy(transStr, "1/4");
      return;
   }

   else if (strcmp(uglyStr, "1/2SM") == 0)
   {
      strcpy(transStr, "1/2");
      return;
   }

   else if (strcmp(uglyStr, "3/4SM") == 0)
   {
      strcpy(transStr, "3/4");
      return;
   }

   else if (strcmp(uglyStr, "1SM") == 0)
   {
      strcpy(transStr, "1");
      return;
   }

   else if (strcmp(uglyStr, "11/2SM") == 0)
   {
      strcpy(transStr, "1 1/2");
      return;
   }

   else if (strcmp(uglyStr, "2SM") == 0)
   {
      strcpy(transStr, "2");
      return;
   }

   else if (strcmp(uglyStr, "21/2SM") == 0)
   {
      strcpy(transStr, "2 1/2");
      return;
   }

   else if (strcmp(uglyStr, "3SM") == 0)
   {
      strcpy(transStr, "3");
      return;
   }

   else if (strcmp(uglyStr, "4SM") == 0)
   {
      strcpy(transStr, "4");
      return;
   }

   else if (strcmp(uglyStr, "5SM") == 0)
   {
      strcpy(transStr, "5");
      return;
   }

   else if (strcmp(uglyStr, "6SM") == 0)
   {
      strcpy(transStr, "6");
      return;
   }

   else if (strcmp(uglyStr, "P6SM") == 0)
   {
      strcpy(transStr, "6+");
      return;
   }
   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}

/******************************************************************************
 * getTranslatedQualifier() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE  
 *   Get the English translations for the NDFD weather qualifier part of the 
 *   "ugly string". 
 *      
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather qualifier. (Input)
 * transStr = Outgoing string with the translated qualifier. (Output)
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
static void getTranslatedQualifier(char *uglyStr, char *transStr)
{
   if (strcmp(uglyStr, "FL") == 0)
   {
      strcpy(transStr, "frequent lightning");
      return;
   }

   else if (strcmp(uglyStr, "HvyRn") == 0)
   {
      strcpy(transStr, "heavy rain");
      return;
   }

   else if (strcmp(uglyStr, "SmA") == 0)
   {
      strcpy(transStr, "small hail");
      return;
   }

   else if (strcmp(uglyStr, "OLA") == 0)
   {
      strcpy(transStr, "outlying areas");
      return;
   }

   else if (strcmp(uglyStr, "GW") == 0)
   {
      strcpy(transStr, "gusty winds");
      return;
   }

   else if (strcmp(uglyStr, "DmgW") == 0)
   {
      strcpy(transStr, "damaging winds");
      return;
   }

   else if (strcmp(uglyStr, "LgA") == 0)
   {
      strcpy(transStr, "large hail");
      return;
   }

   else if (strcmp(uglyStr, "OBO") == 0)
   {
      strcpy(transStr, "on bridges and overpasses");
      return;
   }

   else if (strcmp(uglyStr, "OGA") == 0)
   {
      strcpy(transStr, "on grassy areas");
      return;
   }

   else if (strcmp(uglyStr, "OR") == 0)
   {
      strcpy(transStr, "or");
      return;
   }

   else if (strcmp(uglyStr, "Dry") == 0)
   {
      strcpy(transStr, "dry");
      return;
   }

   else if (strcmp(uglyStr, "Primary") == 0)
   {
      strcpy(transStr, "highest ranking");
      return;
   }

   else if (strcmp(uglyStr, "Mention") == 0)
   {
      strcpy(transStr, "include unconditionally");
      return;
   }

   else if (strcmp(uglyStr, "TOR") == 0)
   {
      strcpy(transStr, "tornado");
      return;
   }

   else if (strcmp(uglyStr, "MX") == 0)
   {
      strcpy(transStr, "mixture");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}

/******************************************************************************
 * genIconLinks() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the "time-series" and "glance" DWML for the Weather 
 *  Conditions Icon element.
 *
 * ARGUMENTS
 *    iconInfo = Structure holding derived Icon links and time info. (Input) 
 *     numRows = The number of data rows formatted into ouput XML. (Input)
 *   layoutKey = The key linking the icons to their valid times 
 *               (ex. k-p3h-n42-1). Its the same as the weather element's 
 *               layoutKey. (Input)
 *  parameters = An xml Node Pointer denoting the parameter node (Output).
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
static void genIconLinks(icon_def *iconInfo, uChar numRows,
		         char *layoutKey, xmlNodePtr parameters)
{

   int index;               /* Counter for icons. */
   xmlNodePtr conditions_icon;  /* An Xml Node Pointer. */
   xmlNodePtr icon_link;      /* An Xml Node Pointer. */

   /* Format the conditions_icon element. */
   conditions_icon = xmlNewChild(parameters, NULL, BAD_CAST "conditions-icon",
                                 NULL);
   xmlNewProp(conditions_icon, BAD_CAST "type", BAD_CAST "forecast-NWS");
   xmlNewProp(conditions_icon, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */
   xmlNewChild(conditions_icon, NULL, BAD_CAST "name", BAD_CAST
               "Conditions Icons");

   /* Loop over all the icon values and format them (there is one for each
    * weather value). */

   for (index = 0; index < numRows; index++)
   {
      /* If the data is missing, so indicate in the XML (nil=true). */
      if (strcmp(iconInfo[index].str, "none") == 0)
      {
         icon_link = xmlNewChild(conditions_icon, NULL, BAD_CAST "icon-link",
                                 NULL);
         xmlNewProp(icon_link, BAD_CAST "xsi:nil", BAD_CAST "true");
      }
      else                    /* Good data, so format it. */
         xmlNewChild(conditions_icon, NULL, BAD_CAST "icon-link",
                     BAD_CAST iconInfo[index].str);
   }
   return;
}

/*****************************************************************************
 * genWeatherValuesByDay() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   Formats applicable "Weather" and derives and formats the "Weather 
 *   Conditions Icons" elements for the DWMLGenByDay products ("12 hourly" and 
 *   "24 hourly" summarizations).
 *
 * ARGUMENTS
 *         pnt = The point index. (Input)
 *   layoutKey = The key linking the icons and weather elements to their valid 
 *               times (ex. k-p3h-n42-1). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *        f_wx = Flag denoting if there is weather data to either format or use
 *               to derive icons. A value = 1 means to format Weather. A value
 *               = 3 means only Icons is to be formatted, but Icons are using 
 *               the time layout for Weather. (Input)
 *    numMatch = The number of matches from degrib. (Input)
 *  numRowsSKY = The number of data rows for sky cover. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 *   numRowsWS = The number of data rows for wind speed. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 * numRowsTEMP = The number of data rows for hourly temperatures. These data 
 *               are used to derive icons, if icons are to be formatted. 
 *               (Input)
 *   numRowsWX = The number of data rows for weather. These data can be 
 *               formatted if f_wx = 1, and/or used to derive icons if f_icon 
 *               = 1.  (Input)
 *  parameters = Xml Node parameter. The formatted weather and icons are child
 *               elements of this node. (Input)
 *    TZoffset = Number of hours to add to current time to get GMT time. 
 *               (Input)
 * f_observeDST = Flag determining if current point observes Daylight 
 *                Savings Time. (Input)  
 *  maxDailyPop =  Array containing the pop values corresponding to a day (24 
 *                 hour format) or 12 hour period (12 hour format).  For 24
 *                 hour format, we use the maximum of the two 12 hour pops 
 *                 that span the day. This variable is used to test if the pop 
 *                 is large enough to justify formatting weather values. (Input)
 * averageSkyCover = Array containing the average Sky Cover values corresponding
 *                   to a day (24 hour format) or 12 hour period (12 hour
 *                   format).  These values are used in deriving the weather 
 *		     and/or icon values. (Input)
 *     maxSkyCover = Array containing the maximum Sky Cover values corresponding
 *                   to a day (24 hour format) or 12 hour period (12 hour
 *                   format).  These values are used in deriving the weather 
 *		     and/or icon values. (Input)
 *     minSkyCover = Array containing the minimum Sky Cover values corresponding
 *                   to a day (24 hour format) or 12 hour period (12 hour
 *                   format).  These values are used in deriving the weather 
 *		     and/or icon values. (Input)
 *       maxSkyNum = The index where the max sky cover was found. Used to 
 *                   determine sky cover trends (i.e. increasing clouds). 
 *                   (Intput)
 *       minSkyNum = The index where the min sky cover was found. Used to 
 *                   determine sky cover trends (i.e. increasing clouds). 
 *                   (Input)
 *  startPositions = The index of where the current forecast period begins.  Used
 *                   to determine sky cover trends (i.e. increasing clouds) for 
 *                   DWMLgenByDay products. (Output)
 *    endPositions = The index of where the current forecast period ends.  Used
 *                   to determine sky cover trends (i.e. increasing clouds) for 
 *                   DWMLgenByDay products. (Output)	 
 *    maxWindSpeed = Array containing the Maximum wind speed values corresponding
 *                   to a day (24 hour format) or 12 hour period (12 hour format).
 *                   These values are used in deriving the weather and/or icon values. 
 *                   (Input)
 * maxWindDirection = Array containing the wind direction values 
 *                    corresponding to a day (24 hour format) or 12 hour period
 *                    (12 hour format). These are not "max" wind direction 
 *                    values, but correspond to the time when the max. wind 
 *                    speed values were found per forecast period.  These values
 *                    are used in deriving the weather and/or icon values. 
 *                    (Input) 
 *     integerTime = Number of seconds since 1970 to when the data is valid.
 *                   Allows the code to know if this data belongs in the current
 *                   period being processed. (Input)
 * integerStartUserTime = Number of seconds since 1970 to when the user 
 *                        established periods being. (Input)                            *         
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void genWeatherValuesByDay(size_t pnt, char *layoutKey, 
		             genMatchType *match, size_t numMatch,
                             uChar f_wx, uChar numRowsWS,
                             uChar numRowsSKY, uChar numRowsPOP,
			     uChar numRowsMAX, uChar numRowsMIN, 
                             uChar numRowsTEMP, uChar numRowsWX,
                             xmlNodePtr parameters, double lat, double lon, 
			     int numDays, sChar TZoffset, sChar f_observeDST,
			     char *frequency, int f_useMinTempTimes, uChar f_XML,
			     int numOutputLines, int *maxDailyPop, 
			     int *averageSkyCover, int *maxSkyCover, 
			     int *minSkyCover, int *maxSkyNum, int *minSkyNum, 
			     int *startPositions, int *endPositions, 
			     int *maxWindSpeed, int *maxWindDirection, 
			     int integerTime, int integerStartUserTime, 
			     double startTime_cml)
{
   double springDoubleDate = 0.0; /* The end date of next cold season expressed
                                     in double form. */
   double fallDoubleDate = 0.0; /* The start date of next cold season expressed
                                   in double form. */
   double *weatherDataTimes = NULL; /* Array holding double startTimes for Wx. */ 
   int *periodMaxTemp = NULL; /* Array holding maximum temp values for 
			         designated forecast periods. */ 
   double *periodStartTimes = NULL; /* Array holding start Times for designated
				       forecast periods. */
   double *periodEndTimes = NULL; /* Array holding end Times for designated
				     forecast periods. */   
   int dayIndex = 0; /* Day containing period being processed. */
   int *isDataAvailable = NULL; /* Array denoting if data exists for that
				   particular day. */
   
   /* Initialize weather triggers. */
   int f_isDrizzle         = 0; /* Flag denoting if weather is drizzle. */
   int f_isRain            = 0; /* Flag denoting if weather is rain. */
   int f_isRainShowers     = 0; /* Flag denoting if weather is rain showers. */
   int f_isSnow            = 0; /* Flag denoting if weather is snow. */
   int f_isSnowShowers     = 0; /* Flag denoting if weather is snow showers. */
   int f_isFreezingDrizzle = 0; /* Flag denoting if weather is frz drizzle. */
   int f_isFreezingRain    = 0; /* Flag denoting if weather is frz rain. */
   int f_isIcePellets      = 0; /* Flag denoting if weather is ice pellets. */
      
   /* Initialize the number of times we see fog today. */
   int fogCount = 0;
   int percentTimeWithFog = 0;
   
   /* Initialize number of rows of data processed for a given day. */
   int numDataRows = 0; 
   
   char *dominantWeather[4]; /* This array stores the weather type, intensity, 
			      * coverage, and qualifier for each day that is 
			      * considered the dominant one. This is the 
			      * summarized weather for the 24/12 hour period.
			      */
   char *tempDom[4]; /* Holds the latest weather information which is then 
		      * compared to the dominantWeather array to see if a new 
		      * dominant weather condition has been found.
		      */
   char *dominantRowsWeather[4][5]; /* Array containing the other 3 weather 
				     * values (out of coverage, intensity,
				     * type, and qualifier) of a row once the 
				     * dominant weather was found.
				     */
   char WxGroups[5][100];    /* An array holding up to 5 weather groups for
                               * any one valid time. */
   char WxValues[5][50];      /* An associative array holding the current
                               * groups type, coverage, intensity, vis, &
                               * qualifier. */
   int numGroups = 0;         /* An index into the weatherGroups array. */
   WX *wxInfo = NULL;         /* Weather data taken from the match array. */
   int priorElemCount = 0;     /* Counter used to find elements' location in
                                 match. */
   char *pstr = NULL;         /* Pointer to "ugly string" delimited by '>'
                               * and '<'. */
   int i;                     /* Index through the match structure. */
   int j;
   char *pstr1 = NULL;        /* Pointer to "ugly string" delimited by '^' to
				 separate the first weather group per 1 row of
				 weather data. */
   char *pstr2 = NULL;        /* Pointer to "ugly string" delimited by '^' to
				 separate (if they exist) the last 4 of a 
				 potential 5 weather groups per 1 row of 
				 weather data. */
   char additive_value[10];   /* String placed in the second and subsequant
                               * weather group to indicate how the data is
                               * combined.  */
   char **wxCoverage = NULL;  /* An individual weather groups coverage. */
   char **wxIntensity = NULL; /* An individual weather groups intensity. */
   char **wxType = NULL;      /* An individual weather groups type. */
   char **wxQualifier = NULL; /* An individual weather groups qualifier. */
   int numValues;             /* An index into each weatherGroups fields (=
                               * 5). */
   int groupIndex;            /* An index into the weatherGroups array. */
   int valueIndex;            /* An index into each weatherGroups fields (=
                               * 5). */
   int wxIndex = 0;           /* Counter for weather. */
   int numDominantTypes = 0;
   xmlNodePtr weather = NULL; /* Xml Node Pointer for node "weather". */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for node "value". */
   xmlNodePtr weather_conditions = NULL;  /* Xml Node Pointer for node
                                           * "weather-conditions". */   
   icon_def *iconInfo = NULL; /* Array holding the icon information. */
   char **phrase; /* Array holding the weather phrase per period. */
   int f_popIsNotAnIssue; /* Flag denoting if PoP is very low, we won't format 
			     the weather values that might be present. */
   int timeLayoutHour = 0; /* The time period's hour for the 12 hourly product.
			      Used to determine if it is night or day in the 
			      generatePhraseAndIcons routine (should = 6 or 
			      18). */
   char transCoverageStr[100];  /* String holding english translation of
                                 * weather coverage. */
   char transTypeStr[100];    /* String holding english translation of
                               * weather coverage. */
   char transIntensityStr[100]; /* String holding english translation of
                                 * weather intensity. */
   char transQualifierStr[100]; /* String holding english translation of
                                 * weather qualifiers. */

   /* Initialize the location where the weather icons are found. */

   char baseURL[] = "http://www.nws.noaa.gov/weather/images/fcicons/";  
   
   /************************************************************************/
   
   /* Firsty, format the weather and display name elements. */

   weather = xmlNewChild(parameters, NULL, BAD_CAST "weather", NULL);
   xmlNewProp(weather, BAD_CAST "time-layout", BAD_CAST layoutKey);

   xmlNewChild(weather, NULL, BAD_CAST "name", BAD_CAST
               "Weather Type, Coverage, and Intensity");
   
   /* Create an array of structures holding the weather element's
    * data info and time info from the match structure. 
    */
      wxInfo = malloc(numRowsWX * sizeof(WX));

      /* Fill Weather Array. */

      priorElemCount = 0;
      for (i = 0; i < numMatch; i++)
      {
         if (match[i].elem.ndfdEnum == NDFD_WX)
         {
            wxInfo[i - priorElemCount].validTime = match[i].validTime;
            if (match[i].value[pnt].valueType != 0 &&
                match[i].value[pnt].valueType != 2)
            {
               strcpy(wxInfo[i - priorElemCount].str, match[i].value[pnt].str);
            }
            wxInfo[i - priorElemCount].valueType =
                  match[i].value[pnt].valueType;
         }
         else
            priorElemCount ++;
      }
      
   /* Prepare to retrieve the weather Data times, the forecast period times, 
    * and the max temperatures in each period. 
    */      
   periodStartTimes = (double *) malloc((numOutputLines) * sizeof(double));
   periodEndTimes = (double *) malloc((numOutputLines) * sizeof(double));
   periodMaxTemp = (int *) malloc((numOutputLines) * sizeof(int));
   
   for (i = 0; i < numOutputLines; i++)
   { 
      periodStartTimes[i] = 9999999999999999999999.0;
      periodEndTimes[i]   = -999.0;
      periodMaxTemp[i]    = 999;
   }
     
   weatherDataTimes = (double *) malloc(numRowsWX * sizeof(double));
   
   prepareWeatherValuesByDay (match, numMatch, TZoffset, f_observeDST, 
		              frequency, numDays, numOutputLines,
			      numRowsMIN, numRowsMAX, f_XML,
			      numRowsPOP, numRowsWX, pnt, 
			      f_useMinTempTimes, startTime_cml, 
			      weatherDataTimes, periodMaxTemp, 
			      periodStartTimes, periodEndTimes,
			      &springDoubleDate, &fallDoubleDate, 
			      &timeLayoutHour);
  
   /* This is a big loop to format each days weather. First, make appropriate
    * allocations. */
   iconInfo = (icon_def *) malloc(numOutputLines * sizeof(icon_def));
   phrase = (char **) malloc(numOutputLines * sizeof(char *));	
   isDataAvailable = (int *) malloc(numOutputLines * sizeof(int));

   /* Allocate the individual elements of the weather phrase. */
   for (i = 0; i < numOutputLines; i++)
      phrase[i] = (char *) malloc(50 * sizeof(char));

   /* See if there are any periods skipped at the beginning of data
    * due to a starTime in the middle of a forecast period. */
/*   for (dayIndex = 0, dayIndex < 2, dayIndex++)
   {
      if (periodStartTimes[dayIndex] == 9999999999999999999999.0 && 
          periodEndTimes[dayIndex] == -999.0)
          numRowsSkipped++;
   }
*/
   /* A big loop to format each days weather. */
   for (dayIndex = 0; dayIndex < numOutputLines; dayIndex++)
   {
    
      /* Initialize the icon for this period to "none". */
      strcpy (iconInfo[dayIndex].str, "none"); 	 
      
      /* We have to know if there is data for a given day so we can check if 
       * the <weather-conditions> should be empty (nil=true) or has data 
       * values.
       */
      isDataAvailable[dayIndex] = 0;
      
      /* Initialize weather triggers for each pass of the loop. */
      f_isDrizzle         = 0;
      f_isRain            = 0;
      f_isRainShowers     = 0;
      f_isSnow            = 0;
      f_isSnowShowers     = 0;
      f_isFreezingDrizzle = 0;
      f_isFreezingRain    = 0;
      f_isIcePellets      = 0;
      
      /* Initialize the number of times we see fog today per each loop. */
      fogCount = 0;
      percentTimeWithFog = 0;
      
      /* Initialized number of rows of data processed for a given day. */
      numDataRows = 0;

      /* Initialize both the dominantWeather (per period) and the day's Weather
       * to lowest possible values. 
       */
      for (i = 0; i < 4; i++) /* 4 wx categories interested in (coverage, 
			       * intensity, type, qualifier). */
      {
         dominantWeather[i] = (char *) malloc(5 * sizeof(char));
         strcpy (dominantWeather[i], "none");
         	      
         for (j = 0; j < 5; j++) /* Up to 5 weather groups per 1 wx string. */
         {
	    dominantRowsWeather[i][j] = (char *) malloc(5 *  sizeof(char));
	    strcpy(dominantRowsWeather[i][j], "none");
	 }
      }

      /* Loop over all of weather's valid times and process any weather groups
       * (up to 5 can exist). 
       */
      for (wxIndex = 0; wxIndex < numRowsWX; wxIndex++)
      {
         memset(WxValues, '\0', 5 * 50);
         memset(WxGroups, '\0', 5 * 100);
         numGroups = 0;
	 
         /* First, determine if this row of weather has valid data. */
	 if (wxInfo[wxIndex].valueType != 2)
         {
	 
            /* Determine if the data is between the user supplied start day and a 
             * calculated end time (based on user provided number of days and the 
             * period length implied by user requested format (make sure the 
	     * weather rows being processed fall with the current period of 
   	     * interest).
             */
	    if (periodStartTimes[dayIndex] <= weatherDataTimes[wxIndex] && 
               weatherDataTimes[wxIndex] < periodEndTimes[dayIndex])	 
            {
               isDataAvailable[dayIndex] = 1;
                 
               /* We found a data row to process, so count it. */
	       numDataRows++;
	    
               /* Lets remove the <> that surround the weather data coming from
                * NDFD (type, intensity, coverge, visibility, and qualifier). */
               pstr = strchr(wxInfo[wxIndex].str, '<');
               while (pstr != NULL)
               {
                  wxInfo[wxIndex].str[pstr - wxInfo[wxIndex].str] = ' ';
                  pstr = strchr(pstr + 1, '<');
               }

               pstr = strchr(wxInfo[wxIndex].str, '>');
               while (pstr != NULL)
               {
                  wxInfo[wxIndex].str[pstr - wxInfo[wxIndex].str] = ' ';
                  pstr = strchr(pstr + 1, '>');
               }
	    
               /* Now put the weather groupings (potentially up to 5 groups per 
	        * ugly string) into an array using the '^' as the delimiter. Fill
	        * the first array elements before the others. 
	        */
               pstr = wxInfo[wxIndex].str;
               groupIndex = 0;
               for (j = 0; pstr[j]; j++)
               {
                  if (pstr[j] != '^')
                     WxGroups[groupIndex][j] = pstr[j];
                  else if (pstr[j] == '^')
                  {
                     WxGroups[groupIndex][j] = '\0';
                     break;
                  }
               }
         
               /* Get the total number of WxGroups for this one row of weather
                * data. 
	        */
               pstr1 = strchr(wxInfo[wxIndex].str, '^');
               while (pstr1 != NULL)
               {
                  numGroups ++;
                  pstr1 = strchr(pstr1 + 1, '^');
               }
      
               /* Continue filling the array of WxGroups. */

               pstr = strchr(wxInfo[wxIndex].str, '^');
               pstr2 = strchr(wxInfo[wxIndex].str, '^');

               for (groupIndex = 1; groupIndex < numGroups + 1; groupIndex++)
               {
                  for (j = 1; pstr[j]; j++)
                  {
                     if (pstr[j] != '^')
                     {
                        WxGroups[groupIndex][j - 1] = pstr[j];
                     }
                     else if (pstr[j] == '^')
                     {
                        WxGroups[groupIndex][j - 1] = '\0';
                        pstr = strchr(pstr + 1, '^');
                        break;
                     }
                  }
               }

               if (numGroups == 0)
               {
                  if (WxGroups[numGroups][1] == 'N'
                      && WxGroups[numGroups][2] == 'o')
                  {
                     WxGroups[numGroups][j - 1] = '\0';
                  }
                  else
                  {
                     WxGroups[numGroups][j] = '\0';
                  }
               }
               else
               {
                  if (pstr2[j - 1] == ':')
                     WxGroups[numGroups][j - 1] = '\0';
               }

               /* If there is more than one group we connect them with an "and". 
                */
               if (numGroups > 0)
                  strcpy(additive_value, "and");

               /* Initialize the temporary dominance array to lowest possible 
	        * values. This array tracks which of the possible 5 groups per one
	        * row of data (one weather ugly string) is the dominant one. */
               for (j = 0; j < 4; j++)
               {
                  tempDom[j] = (char *) malloc(5 *  sizeof(char));
                  strcpy (tempDom[j], "none");
               }
	    
               /* Loop over each group again and process its weather information (i.e.
	        * coverage, intensity, type, and qualifier (visibility is not processed
	        * for the DWMLgenByDay products) all which are denoted by a ":". 
	        */
               wxCoverage = (char **)calloc(numGroups + 1, sizeof(char *));
               wxIntensity = (char **)calloc(numGroups + 1, sizeof(char *));
	       wxType = (char **)calloc(numGroups + 1, sizeof(char *));
               wxQualifier = (char **)calloc(numGroups + 1, sizeof(char *));
	    
               for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
               {
                  numValues = 0;

                  /* Create the associative array holding the weather info
                   * fields. */
                  pstr = WxGroups[groupIndex];
                  valueIndex = 0;

                  for (i = 0; pstr[i]; i++)
                  {
                     if (pstr[i] != ':')
                        WxValues[valueIndex][i] = pstr[i];
                     else if (pstr[i] == ':')
                     {
                        WxValues[valueIndex][i] = '\0';
                        break;
                     }

                  }

                  /* Get the total number of WxValues (should be 5). */
                  pstr1 = strchr(WxGroups[groupIndex], ':');
                  while (pstr1 != NULL)
                  {
                     numValues ++;
                     pstr1 = strchr(pstr1 + 1, ':');
                  }
	       
                  /* Bump this number up by one to account for the Wx Qualifier. 
                   */
                  numValues ++;
       
                  /* Continue filling the array of WxValues */
                  pstr = strchr(WxGroups[groupIndex], ':');
                  for (valueIndex = 1; valueIndex < numValues; valueIndex++)
                  {
                     for (i = 1; pstr[i]; i++)
                     {
                        if (pstr[i] != ':')
                        {
                           WxValues[valueIndex][i - 1] = pstr[i];
                        }

                        else if (pstr[i] == ':')
                        {
                           WxValues[valueIndex][i - 1] = '\0';
                           pstr = strchr(pstr + 1, ':');
                           break;
                        }
                     }
                  }
                  WxValues[4][i - 1] = '\0';

	          /* Set the missing data flag if data is missing for this 
	      	   * particular weather ugly string (weather data row). We 
		   * denote the the whole ugly string is missing by setting
	   	   * each groups dominant weather value (coverage) to "none".
		   */
	          if (wxInfo[wxIndex].valueType == 2)
	          {
                     wxCoverage[groupIndex] = malloc((strlen("none") + 1) *  sizeof(char));
                     strcpy(wxCoverage[groupIndex], "none");
	          }
	          else /* Populate weather value arrays for 'coverage',
		        * 'intensity', 'type', and 'qualifier' (if exists).
		        */
	          {
		       
                     /* Weather Coverage */
                     if (WxValues[0][1] == 'N' && WxValues[0][2] == 'o')
                     {
                        wxCoverage[groupIndex] = (char *) malloc((strlen("none") + 1) *  sizeof(char));
                        strcpy(wxCoverage[groupIndex], "none");
                     }
                     else
                     {
                        strTrim(WxValues[0]);
                        wxCoverage[groupIndex] = (char *) malloc((strlen(WxValues[0]) + 1) *  sizeof(char));
                        strcpy(wxCoverage[groupIndex], WxValues[0]);
                     }

                     /* Weather Type */
                     if (WxValues[1][1] == 'N' && WxValues[1][2] == 'o')
                     {
                        wxType[groupIndex] = (char *) malloc((strlen("none") + 1) *  sizeof(char));
                        strcpy(wxType[groupIndex], "none");
                     }
                     else
                     {
                        strTrim(WxValues[1]);
                        wxType[groupIndex] = (char *) malloc((strlen(WxValues[1]) + 1) *  sizeof(char));
                        strcpy(wxType[groupIndex], WxValues[1]);
                     }

                     /* Weather Intensity */
                        if (WxValues[2][1] == 'N' && WxValues[2][2] == 'o')
                     {
                        wxIntensity[groupIndex] = (char *) malloc((strlen("none") + 1) *  sizeof(char));
                        strcpy(wxIntensity[groupIndex], "none");
                     }
                     else
                     {
                        strTrim(WxValues[2]);
                        wxIntensity[groupIndex] = (char *) malloc((strlen(WxValues[2]) + 1) *  sizeof(char));
                        strcpy(wxIntensity[groupIndex], WxValues[2]);
                     }
	       
                     /* Note, we are not processing the visibility WxValue field, 
	      	      * which is denoted by WxValues[3] element, for the DWMLgenByDay
	   	      * products.
	         	   */
	       
                     /* Weather Qualifier */
                     if (WxValues[4][1] == 'N' && WxValues[4][2] == 'o')
                     {
                        wxQualifier[groupIndex] = (char *) malloc((strlen("none") + 1) *  sizeof(char));
                        strcpy(wxQualifier[groupIndex], "none");
                     }
                     else if (WxValues[4][0] == '\0') /* Check for NULL since
			   			       * qualifers do not have to 
						       * exist. 
						       */
                     {
                        wxQualifier[groupIndex] = (char *) malloc((strlen("none") + 1) *  sizeof(char));
                        strcpy(wxQualifier[groupIndex], "none");
                     }
                     else
                     {
                        strTrim(WxValues[4]);
                        wxQualifier[groupIndex] = (char *) malloc((strlen(WxValues[4]) + 1) *  sizeof(char));
                        strcpy(wxQualifier[groupIndex], WxValues[4]);
                     }
	          }
	       
                  /* Re-initialize the WxValues array for next group iteration. */
                  memset(WxValues, '\0', 5 * 50);

                  /* If coverage is dominant, set new tempDom to the current group. */
	          if (isDominant(wxCoverage[groupIndex], tempDom[0], "coverage"))
	          {
                     /* Copy over the 'coverage' to the temporary array. */
                     tempDom[0] = (char *) realloc(tempDom[0], (strlen(wxCoverage[groupIndex]) + 1) *  sizeof(char));
	             strcpy (tempDom[0], wxCoverage[groupIndex]);

                     /* Copy over the 'intensity' to the temporary array. */
                     tempDom[1] = (char *) realloc(tempDom[1], (strlen(wxIntensity[groupIndex]) + 1) *  sizeof(char));
	   	     strcpy (tempDom[1], wxIntensity[groupIndex]);

                     /* Copy over the 'type' to the temporary array. */
                     tempDom[2] = (char *) realloc(tempDom[2], (strlen(wxType[groupIndex]) + 1) *  sizeof(char));
		     strcpy (tempDom[2], wxType[groupIndex]);

                     /* Copy over the 'qualifier' to the temporary array. */
                     tempDom[3] = (char *) realloc(tempDom[3], (strlen(wxQualifier[groupIndex]) + 1) *  sizeof(char));
		     strcpy (tempDom[3], wxQualifier[groupIndex]);
	          }
	          else if (strcmp(wxCoverage[groupIndex], tempDom[0]) == 0)   
	          {		       
	             /* If coverage is equal, test for dominant intensity. */
                     if (isDominant(wxIntensity[groupIndex], tempDom[1], "intensity"))
	             {
                        /* Copy over the 'coverage' to the temporary array. */
                        tempDom[0] = (char *) realloc(tempDom[0], (strlen(wxCoverage[groupIndex]) + 1) *  sizeof(char));
		        strcpy (tempDom[0], wxCoverage[groupIndex]);

                        /* Copy over the 'intensity' to the temporary array. */
                        tempDom[1] = (char *) realloc(tempDom[1], (strlen(wxIntensity[groupIndex]) + 1) *  sizeof(char));
		        strcpy (tempDom[1], wxIntensity[groupIndex]);
			
                        /* Copy over the 'type' to the temporary array. */
                        tempDom[2] = (char *) realloc(tempDom[2], (strlen(wxType[groupIndex]) + 1) *  sizeof(char));
		        strcpy (tempDom[2], wxType[groupIndex]);

                        /* Copy over the 'qualifier' to the temporary array. */
                        tempDom[3] = (char *) realloc(tempDom[3], (strlen(wxQualifier[groupIndex]) + 1) *  sizeof(char));
	   	        strcpy (tempDom[3], wxQualifier[groupIndex]);
		     }
		     else if (strcmp(wxIntensity[groupIndex], tempDom[1]) == 0)
	             {		       
	                /* If intensity is equal, test for dominant type. */
                        if (isDominant(wxType[groupIndex], tempDom[2], "type"))
	                {
                           /* Copy over the 'coverage' to the temporary array. */
                           tempDom[0] = (char *) realloc(tempDom[0], (strlen(wxCoverage[groupIndex]) + 1) *  sizeof(char));
	   	           strcpy (tempDom[0], wxCoverage[groupIndex]);

                           /* Copy over the 'intensity' to the temporary array. */
                           tempDom[1] = (char *) realloc(tempDom[1], (strlen(wxIntensity[groupIndex]) + 1) *  sizeof(char));
		           strcpy (tempDom[1], wxIntensity[groupIndex]);

                           /* Copy over the 'type' to the temporary array. */
                           tempDom[2] = (char *) realloc(tempDom[2], (strlen(wxType[groupIndex]) + 1) *  sizeof(char));
		           strcpy (tempDom[2], wxType[groupIndex]);

                           /* Copy over the 'qualifier' to the temporary array. */
                           tempDom[3] = (char *) realloc(tempDom[3], (strlen(wxQualifier[groupIndex]) + 1) *  sizeof(char));
		           strcpy (tempDom[3], wxQualifier[groupIndex]);
		        }
	   	     }
	          }
	       } /* We have established the dominant group out of the possible 5 groups 
	          * making up the current ugly string (current row of weather data). 
	          */
	    
               /* If we have fog, calculate the percentage of the time it is occurring.
                * This will be used in determining an icon and the weather phrase.
	        */   
               if (strcmp(tempDom[2], "F") == 0) 
	          fogCount++;

	       /* Compare the dominant group to current dominant weather. */
	       if (isDominant (tempDom[0], dominantWeather[0], "coverage")) 
               { 
                  /* Copy over the 'coverage' to the dominant weather array. */
                  dominantWeather[0] = (char *) realloc(dominantWeather[0], (strlen(tempDom[0]) + 1) *  sizeof(char));
	          strcpy (dominantWeather[0],tempDom[0]);
		  
                  /* Copy over the 'intensity' to the dominant weather array. */
                  dominantWeather[1] = (char *) realloc(dominantWeather[1], (strlen(tempDom[1]) + 1) *  sizeof(char));
	          strcpy (dominantWeather[1], tempDom[1]);

                  /* Copy over the 'type' to the dominant weather array. */
                  dominantWeather[2] = (char *) realloc(dominantWeather[2], (strlen(tempDom[2]) + 1) *  sizeof(char));
	          strcpy (dominantWeather[2], tempDom[2]);

                  /* Copy over the 'qualifier' to the dominant weather array. */
                  dominantWeather[3] = (char *) realloc(dominantWeather[3], (strlen(tempDom[3]) + 1) *  sizeof(char));
	          strcpy (dominantWeather[3], tempDom[3]);
	       
                  for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
	          {
                     /* Save the contents of this row so if it turns out to be the
                      * the dominate row, we will know what all the weather was,
                      * not just the row's dominant member.
	   	      */

		     /* Copy over the 'coverage'. */     
		     dominantRowsWeather[0][groupIndex] = (char *)
			     realloc(dominantRowsWeather[0][groupIndex], (strlen(wxCoverage[groupIndex]) + 1) *  sizeof(char));     
	   	     strcpy (dominantRowsWeather[0][groupIndex], 
		             wxCoverage[groupIndex]);
		     /* Copy over the 'intensity'. */     
		     dominantRowsWeather[1][groupIndex] = (char *)
			     realloc(dominantRowsWeather[1][groupIndex], (strlen(wxIntensity[groupIndex]) + 1) *  sizeof(char));     
		     strcpy (dominantRowsWeather[1][groupIndex], 
		             wxIntensity[groupIndex]);
	   	     /* Copy over the 'type'. */     
	             dominantRowsWeather[2][groupIndex] = (char *)
			     realloc(dominantRowsWeather[2][groupIndex], (strlen(wxType[groupIndex]) + 1) *  sizeof(char));     
	             strcpy (dominantRowsWeather[2][groupIndex], 
		             wxType[groupIndex]);
	             /* Copy over the 'qualifier'. */     
		     dominantRowsWeather[3][groupIndex] = (char *)
			  realloc(dominantRowsWeather[3][groupIndex], (strlen(wxQualifier[groupIndex]) + 1) *  sizeof(char));     
		     strcpy (dominantRowsWeather[3][groupIndex], 
		          wxQualifier[groupIndex]);
	          }
	        
	          numDominantTypes = numGroups;
	       }
	       else if (strcmp(tempDom[0], dominantWeather[0]) == 0)
               {
	          if (isDominant(tempDom[1], dominantWeather[1], "intensity")) 
	          {
                     /* Copy over the 'coverage' to the dominant weather array. */
                     dominantWeather[0] = (char *) realloc(dominantWeather[0], (strlen(tempDom[0]) + 1) *  sizeof(char));
   	             strcpy (dominantWeather[0],tempDom[0]);
		     
                     /* Copy over the 'intensity' to the dominant weather array. */
                     dominantWeather[1] = (char *) realloc(dominantWeather[1], (strlen(tempDom[1]) + 1) *  sizeof(char));
	             strcpy (dominantWeather[1], tempDom[1]);

                     /* Copy over the 'type' to the dominant weather array. */
                     dominantWeather[2] = (char *) realloc(dominantWeather[2], (strlen(tempDom[2]) + 1) *  sizeof(char));
	             strcpy (dominantWeather[2], tempDom[2]);

                     /* Copy over the 'qualifier' to the dominant weather array. */
                     dominantWeather[3] = (char *) realloc(dominantWeather[3], (strlen(tempDom[3]) + 1) *  sizeof(char));
	             strcpy (dominantWeather[3], tempDom[3]);
	       
                     for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
	             {
                        /* Save the contents of this row so if it turns out to be the
                         * the dominate row, we will know what all the weather was,
                         * not just the row's dominant member.
		         */
		   
		        /* Copy over the 'coverage'. */     
		        dominantRowsWeather[0][groupIndex] = (char *)
			        realloc(dominantRowsWeather[0][groupIndex], (strlen(wxCoverage[groupIndex]) + 1) *  sizeof(char));     
		        strcpy (dominantRowsWeather[0][groupIndex], 
		                wxCoverage[groupIndex]);
		        /* Copy over the 'intensity'. */     
		        dominantRowsWeather[1][groupIndex] = (char *)
		      	        realloc(dominantRowsWeather[1][groupIndex], (strlen(wxIntensity[groupIndex]) + 1) *  sizeof(char));     
		        strcpy (dominantRowsWeather[1][groupIndex], 
		                wxIntensity[groupIndex]);
		        /* Copy over the 'type'. */     
		        dominantRowsWeather[2][groupIndex] = (char *)
			        realloc(dominantRowsWeather[2][groupIndex], (strlen(wxType[groupIndex]) + 1) *  sizeof(char));     
		        strcpy (dominantRowsWeather[2][groupIndex], 
		               wxType[groupIndex]);
		        /* Copy over the 'qualifier'. */     
		        dominantRowsWeather[3][groupIndex] = (char *)
		 	        realloc(dominantRowsWeather[3][groupIndex], (strlen(wxQualifier[groupIndex]) + 1) *  sizeof(char));     
		        strcpy (dominantRowsWeather[3][groupIndex], 
		                wxQualifier[groupIndex]);
	             }
	       
	             numDominantTypes = numGroups;  
  
	          }
	          else if (strcmp (tempDom[1], dominantWeather[1]) == 0) 
	          {
	             if (isDominant(tempDom[2], dominantWeather[2], "type")) 
	             {   
                        /* Copy over the 'coverage' to the dominant weather array. */
                        dominantWeather[0] = (char *) realloc(dominantWeather[0], (strlen(tempDom[0]) + 1) *  sizeof(char));
   	                strcpy (dominantWeather[0],tempDom[0]);

                        /* Copy over the 'intensity' to the dominant weather array. */
                        dominantWeather[1] = (char *) realloc(dominantWeather[1], (strlen(tempDom[1]) + 1) *  sizeof(char));
	                strcpy (dominantWeather[1], tempDom[1]);

                        /* Copy over the 'type' to the dominant weather array. */
                        dominantWeather[2] = (char *) realloc(dominantWeather[2], (strlen(tempDom[2]) + 1) *  sizeof(char));
	                strcpy (dominantWeather[2], tempDom[2]);

                        /* Copy over the 'qualifier' to the dominant weather array. */
                        dominantWeather[3] = (char *) realloc(dominantWeather[3], (strlen(tempDom[3]) + 1) *  sizeof(char));
	                strcpy (dominantWeather[3], tempDom[3]);
	       
                        for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
	                {
                           /* Save the contents of this row so if it turns out to be the
                            * the dominate row, we will know what all the weather was,
                            * not just the row's dominant member.
		            */
		   
		           /* Copy over the 'coverage'. */     
		           dominantRowsWeather[0][groupIndex] = (char *)
			           realloc(dominantRowsWeather[0][groupIndex], (strlen(wxCoverage[groupIndex]) + 1) *  sizeof(char));     
		           strcpy (dominantRowsWeather[0][groupIndex], 
		                   wxCoverage[groupIndex]);
		           /* Copy over the 'intensity'. */     
		           dominantRowsWeather[1][groupIndex] = (char *)
		   	        realloc(dominantRowsWeather[1][groupIndex], (strlen(wxIntensity[groupIndex]) + 1) *  sizeof(char));     
		           strcpy (dominantRowsWeather[1][groupIndex], 
		                   wxIntensity[groupIndex]);
		           /* Copy over the 'type'. */     
		           dominantRowsWeather[2][groupIndex] = (char *)
			           realloc(dominantRowsWeather[2][groupIndex], (strlen(wxType[groupIndex]) + 1) *  sizeof(char));     
		           strcpy (dominantRowsWeather[2][groupIndex], 
		                   wxType[groupIndex]);
		           /* Copy over the 'qualifier'. */     
		           dominantRowsWeather[3][groupIndex] = (char *)
			           realloc(dominantRowsWeather[3][groupIndex], (strlen(wxQualifier[groupIndex]) + 1) *  sizeof(char));     
		           strcpy (dominantRowsWeather[3][groupIndex], 
		                   wxQualifier[groupIndex]);
	                }
	       
	                numDominantTypes = numGroups;

		     }
		     else
	             {
                        /* It is possible that two rows will have the exact same 
		         * dominant precedence, but one row may have more 
		         * information (less dominant weather types) that would
                         * warrant making the multiple weather type row trump the 
		         * row that is otherwise equal. So lets look for that case 
		         * and pick the multiple weather type row.
		         */
                        if (numDominantTypes < numGroups)
	                {
                           /* Copy over the 'coverage' to the dominant weather array. */
                           dominantWeather[0] = (char *) realloc(dominantWeather[0], (strlen(tempDom[0]) + 1) *  sizeof(char));
   	                   strcpy (dominantWeather[0],tempDom[0]);

                           dominantWeather[1] = (char *) realloc(dominantWeather[1], (strlen(tempDom[1]) + 1) *  sizeof(char));
	                   strcpy (dominantWeather[1], tempDom[1]);

                           /* Copy over the 'type' to the dominant weather array. */
                           dominantWeather[2] = (char *) realloc(dominantWeather[2], (strlen(tempDom[2]) + 1) *  sizeof(char));
	                   strcpy (dominantWeather[2], tempDom[2]);

                           /* Copy over the 'qualifier' to the dominant weather array. */
                           dominantWeather[3] = (char *) realloc(dominantWeather[3], (strlen(tempDom[3]) + 1) *  sizeof(char));
	                   strcpy (dominantWeather[3], tempDom[3]);
	       
                           for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
	                   {
                              /* Save the contents of this row so if it turns out to be the
                               * the dominate row, we will know what all the weather was,
                               * not just the row's dominant member.
		               */
		   
		              /* Copy over the 'coverage'. */     
		              dominantRowsWeather[0][groupIndex] = (char *)
			              realloc(dominantRowsWeather[0][groupIndex], (strlen(wxCoverage[groupIndex]) + 1) *  sizeof(char));     
		              strcpy (dominantRowsWeather[0][groupIndex], 
		                      wxCoverage[groupIndex]);
		              /* Copy over the 'intensity'. */     
		              dominantRowsWeather[1][groupIndex] = (char *)
		   	           realloc(dominantRowsWeather[1][groupIndex], (strlen(wxIntensity[groupIndex]) + 1) *  sizeof(char));     
		              strcpy (dominantRowsWeather[1][groupIndex], 
		                      wxIntensity[groupIndex]);
		              /* Copy over the 'type'. */     
		              dominantRowsWeather[2][groupIndex] = (char *)
			              realloc(dominantRowsWeather[2][groupIndex], (strlen(wxType[groupIndex]) + 1) *  sizeof(char));     
		              strcpy (dominantRowsWeather[2][groupIndex], 
		                      wxType[groupIndex]);
		              /* Copy over the 'qualifier'. */     
		              dominantRowsWeather[3][groupIndex] = (char *)
			              realloc(dominantRowsWeather[3][groupIndex], (strlen(wxQualifier[groupIndex]) + 1) *  sizeof(char));     
		              strcpy (dominantRowsWeather[3][groupIndex], 
		                      wxQualifier[groupIndex]);
	                   }
	       
	                   numDominantTypes = numGroups;

                        }
		     }
	          }
	       }
	    
	       /* Free up the temp dominance array. */
               for (j = 0; j < 4; j++)
                  free (tempDom[j]);
	    
	       /* Free up the coverage, intensity, type, and qualifiers that can
	        * make up each weather group. 
	        */
               for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
               {
                  free(wxCoverage[groupIndex]);
                  free(wxType[groupIndex]);
                  free(wxIntensity[groupIndex]);
                  free(wxQualifier[groupIndex]);
               }

               free(wxCoverage);
               free(wxType);
               free(wxIntensity);
               free(wxQualifier);
	    	    
               /* Re-initialize the WxGroups arrays. */
               memset(WxGroups, '\0', 5 * 100);	 
	    } /* End of if looking for the data that corresponds to a specific 
	       * day. 
	       */

         } /* End of "if weather row is missing" check. */
	 
      } /* End of for loop over all rows of weather data and checking if in period. */
      
      /* Now we need to look for the occurrance of multiple weather types for
       * this time period.
       */
      for (groupIndex = 0; groupIndex < numDominantTypes + 1; groupIndex++)
      {
         /* Determine which weather types are found on this day. This will
          * allow us to format phrases like "rain and snow".
          */
         if (strcmp(dominantRowsWeather[2][groupIndex], "L") == 0)
            f_isDrizzle = 1;
         if (strcmp(dominantRowsWeather[2][groupIndex], "R") == 0)
            f_isRain = 1;
         if (strcmp(dominantRowsWeather[2][groupIndex], "RW") == 0)
            f_isRainShowers = 1;
         if (strcmp(dominantRowsWeather[2][groupIndex], "IP") == 0)
            f_isIcePellets = 1;
         if (strcmp(dominantRowsWeather[2][groupIndex], "S") == 0)
            f_isSnow = 1;
         if (strcmp(dominantRowsWeather[2][groupIndex], "SW") == 0)
            f_isSnowShowers = 1;
         if (strcmp(dominantRowsWeather[2][groupIndex], "ZL") == 0)
            f_isFreezingDrizzle = 1;
         if (strcmp(dominantRowsWeather[2][groupIndex], "ZR") == 0)
            f_isFreezingRain = 1;
      }
   
      /* Format start of <weather-conditions> element. Then see if there is
       * data and set data as attributes to the <weather-conditions> element. 
       * If not, format the "nil" attribute into the <weather-conditions>
       * element.
       */
      weather_conditions = xmlNewChild(weather, NULL, BAD_CAST
                                       "weather-conditions", NULL);
	 
      /* Did we find data for this day?, prepare to format the XML. */
      if (isDataAvailable[dayIndex])
      {        
         /* Calculate the percentage of time this day had fog.  We will use this
          * to deterine the icon and weather phrase.
          */
         if (fogCount > 0)  
            percentTimeWithFog = fogCount/numDataRows;
         else
            percentTimeWithFog = 0;

         /* Determine the *phrases and icon that goes with the weather 
	  * conditions. Initialize the flag determining if Pop is an issue for
	  * current period. 
	  */
	 f_popIsNotAnIssue = 0;
 	 generatePhraseAndIcons(dayIndex, frequency, timeLayoutHour, 
			        dominantWeather, baseURL, maxDailyPop, 
				averageSkyCover, maxSkyCover, minSkyCover, 
				maxSkyNum, minSkyNum, periodMaxTemp, 
				springDoubleDate, fallDoubleDate, 
				maxWindSpeed, maxWindDirection, integerTime, 
				integerStartUserTime, startPositions, 
				endPositions, f_isDrizzle, f_isRain, 
				f_isRainShowers, f_isIcePellets, f_isSnow, 
				f_isSnowShowers, f_isFreezingDrizzle, 
				f_isFreezingRain, iconInfo, phrase, 
				&f_popIsNotAnIssue);

	 /* Need to get weather phrase by this point. We will insert the 
	  * weather phrase as "weather-summary", an attribute of the 
	  * <weather-conditions> element.  
	  */
	 xmlNewProp(weather_conditions, BAD_CAST "weather-summary", BAD_CAST
	            phrase[dayIndex]);
	 if (strcmp(dominantWeather[0], "none") != 0 && f_popIsNotAnIssue)
         {
            /* Format XML. */
		    	 
            /* Loop over each group and format the <value> element and each of it's 
	     * four attributes (coverage, type, intensity, and qualifier. */
            for (i = 0; i < numDominantTypes + 1; i++)
	    {
               value = xmlNewChild(weather_conditions, NULL, BAD_CAST
                                   "value", NULL);
               getTranslatedCoverage(dominantRowsWeather[0][i], 
			             transCoverageStr);
               getTranslatedType(dominantRowsWeather[2][i], transTypeStr);
               getTranslatedIntensity(dominantRowsWeather[1][i], transIntensityStr);
               getTranslatedQualifier(dominantRowsWeather[3][i], transQualifierStr);

               xmlNewProp(value, BAD_CAST "coverage", BAD_CAST
                          transCoverageStr);
               xmlNewProp(value, BAD_CAST "intensity", BAD_CAST
                          transIntensityStr);

               if (i > 0) /* Groups other than first require
                           * additive attribute. 
			   */
	       {
	          if (strcmp(transQualifierStr, "or") == 0)
                      strcpy(additive_value, "or");
                      xmlNewProp(value, BAD_CAST "additive", BAD_CAST
                                 additive_value);
	       }

               xmlNewProp(value, BAD_CAST "weather-type", BAD_CAST
                          transTypeStr);
               xmlNewProp(value, BAD_CAST "qualifier", BAD_CAST
                          transQualifierStr);

	    }
	 }
      }
      else /* We didn't find data, so format the nil attribute for the <weather-
	    * conditions> element. 
	    */
         xmlNewProp(weather_conditions, BAD_CAST "xsi:nil", BAD_CAST "true");
    
      /* Free the dominantRowsWeather and dominantWeather arrays. */
      for (i = 0; i < 4; i++) 
      {
	 free (dominantWeather[i]);
         for (j = 0; j < 5; j++) /* Up to 5 weather groups per 1 wx string. */
	    free (dominantRowsWeather[i][j]);
      }   
      
   } /* End of all days containing all number of formatted lines and all 
      * periods. 
      */ 

   /* Having saved the appropriate weather icons paths and files names in the
    * structure iconInfo, format the XML to hold the links. 
    */
   genIconLinks(iconInfo, numOutputLines, layoutKey, 
	        parameters);

   /* Free some things. */
   for (i = 0; i < numOutputLines; i++)
      free(phrase[i]);
   free(wxInfo);
   free(iconInfo);
   free(isDataAvailable);
   free(phrase);
   free(periodMaxTemp);
   free(periodStartTimes);
   free(periodEndTimes);
   free(weatherDataTimes);

   return;
} 

/*****************************************************************************
 * genWeatherValues() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   Formats applicable "Weather" and /or derives and formats the "Weather 
 *   Conditions Icons" elements for the DWMLGen products "time-series" and 
 *   "glance".
 *
 * ARGUMENTS
 *         pnt = The point index. (Input)
 *   layoutKey = The key linking the icons and weather elements to their valid 
 *               times (ex. k-p3h-n42-1). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *        f_wx = Flag denoting if there is weather data to either format or use
 *               to derive icons. A value = 1 means to format Weather. A value
 *               = 3 means only Icons is to be formatted, but Icons are using 
 *               the time layout for Weather. (Input)
 *      f_icon = Flag denoting if icons are to be formatted. (Input)
 *    numMatch = The number of matches from degrib. (Input)
 *  numRowsSKY = The number of data rows for sky cover. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 *   numRowsWS = The number of data rows for wind speed. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 * numRowsTEMP = The number of data rows for hourly temperatures. These data 
 *               are used to derive icons, if icons are to be formatted. 
 *               (Input)
 *   numRowsWX = The number of data rows for weather. These data can be 
 *               formatted if f_wx = 1, and/or used to derive icons if f_icon 
 *               = 1.  (Input)
 *  parameters = Xml Node parameter. The formatted weather and icons are child
 *               elements of this node. (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void genWeatherValues(size_t pnt, char *layoutKey, genMatchType * match,
                             size_t numMatch, uChar f_wx, int f_icon,
                             uChar numRowsWS, uChar numRowsSKY,
                             uChar numRowsTEMP, uChar numRowsWX,
                             xmlNodePtr parameters, double lat, double lon)
{

   int i;
   int priorElemCount;        /* Counter used to find elements' location in
                               * match. */
   int itIsNightTime = 0;     /* Denotes wether time is night or day. */
   int wxIndex = 0;           /* Counter for weather. */
   int wsIndex = 0;           /* Counter for wind speed. */
   int skyIndex = 0;          /* Counter for sky cover. */
   int hourlyTempIndex = 0;   /* Counter for hourly temperatures. */
   int foundWeatherValue;     /* Denotes if weather is occuring during a
                               * time. */
   int numGroups = 0;         /* An index into the weatherGroups array. */
   int numValues;             /* An index into each weatherGroups fields (=
                               * 5). */
   int groupIndex;            /* An index into the weatherGroups array. */
   int valueIndex;            /* An index into each weatherGroups fields (=
                               * 5). */
   int skyCoverTimeEqualsWeatherTime = 0; /* Denotes current time has both a
                                           * sky cover and weather value to
                                           * use to derive icons. */
   int windTimeEqualsWeatherTime = 0; /* Denotes current time has both a wind 
                                       * speed and weather value to use to
                                       * derive icons. */
   int hourlyTempTimeEqualsWeatherTime = 0; /* Denotes current time has both
                                             * an hourly temp and weather
                                             * value to use to derive icons. */
   int valueIsMissing = 0;    /* Denotes if current weather is missing value. 
                               */
   char *pstr = NULL;         /* Pointer to "ugly string" delimited by '>'
                               * and '^'. */
   char *pstr1 = NULL;        /* Pointer to "ugly string" delimited by '^'. */
   char *pstr2 = NULL;        /* Pointer to "ugly string" delimited by '^'. */
   char WxGroups[10][100];    /* An array holding up to 5 weather groups for
                               * any one valid time. */
   char WxValues[5][50];      /* An associative array holding the current
                               * groups type, coverage, intensity, vis, &
                               * qualifier. */
   char **wxCoverage = NULL;  /* An individual weather groups coverage. */
   char **wxType = NULL;      /* An individual weather groups type. */
   char **wxIntensity = NULL; /* An individual weather groups intensity. */
   char **wxVisibility = NULL;  /* An individual weather groups visibility. */
   char **wxQualifier = NULL; /* An individual weather groups qualifier. */
   char additive_value[10];   /* String placed in the second and subsequant
                               * weather group to indicate how the data is
                               * combined.  */
   char transCoverageStr[100];  /* String holding english translation of
                                 * weather coverage. */
   char transTypeStr[100];    /* String holding english translation of
                               * weather coverage. */
   char transIntensityStr[100]; /* String holding english translation of
                                 * weather intensity. */
   char transVisibilityStr[100];  /* String holding english translation of
                                   * weather visibility. */
   char transQualifierStr[100]; /* String holding english translation of
                                 * weather qualifiers. */
   xmlNodePtr weather = NULL; /* Xml Node Pointer for node "weather". */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for node "value". */
   xmlNodePtr visibility = NULL;  /* Xml Node Pointer for node "visibility". */
   xmlNodePtr weather_conditions = NULL;  /* Xml Node Pointer for node
                                           * "weather-conditions". */

   WX *wxInfo = NULL;         /* Weather data taken from the match array. */
   WS *wsInfo = NULL;         /* Wind Speed data taken from the match array. */
   sky_def *skyInfo = NULL;   /* Sky coverage data taken from the match
                               * array. */
   TEMP *tempInfo = NULL;     /* Hourly Temp data taken from the match array. 
                               */
   icon_def *iconInfo = NULL; /* Array holding the icon information. */

/* Initialize the location where the weather icons are found. */

   char baseURL[] = "http://www.nws.noaa.gov/weather/images/fcicons/";

/*  Determine if we need to format XML for weather data. */

   if (f_wx == 1 || f_icon)
   {

      /* Firstly, create arrays of structures holding applicable elements'
       * data info and time info from the match structure.  If icons are to
       * be formatted, then wind speed, sky cover, and temperatures must be
       * allocated here as they are used to derive icons. */

      wxInfo = malloc(numRowsWX * sizeof(WX));
      if (f_icon)
      {
         wsInfo = malloc(numRowsWS * sizeof(WS));
         skyInfo = malloc(numRowsSKY * sizeof(sky_def));
         tempInfo = malloc(numRowsTEMP * sizeof(TEMP));
      }

      /* Fill Weather Array. */

      priorElemCount = 0;
      for (i = 0; i < numMatch; i++)
      {
         if (match[i].elem.ndfdEnum == NDFD_WX)
         {
            wxInfo[i - priorElemCount].validTime = match[i].validTime;
            if (match[i].value[pnt].valueType != 0 &&
                match[i].value[pnt].valueType != 2)
            {
               strcpy(wxInfo[i - priorElemCount].str, match[i].value[pnt].str);
            }
            wxInfo[i - priorElemCount].valueType =
                  match[i].value[pnt].valueType;
         }
         else
            priorElemCount += 1;
      }

      if (f_icon)             /* Then Wind Speed, Sky Cover, and Temperature
                               * arrays are * needed. */
      {
         /* Fill Wind Speed Array. */

         priorElemCount = 0;
         for (i = 0; i < numMatch; i++)
         {
            if (match[i].elem.ndfdEnum == NDFD_WS)
            {
               wsInfo[i - priorElemCount].validTime = match[i].validTime;
               wsInfo[i - priorElemCount].data =
                     (int)myRound(match[i].value[pnt].data, 0);
               wsInfo[i - priorElemCount].valueType =
                     match[i].value[pnt].valueType;
            }
            else
               priorElemCount += 1;
         }

         /* Fill Sky Cover Array. */

         priorElemCount = 0;
         for (i = 0; i < numMatch; i++)
         {
            if (match[i].elem.ndfdEnum == NDFD_SKY)
            {
               skyInfo[i - priorElemCount].validTime = match[i].validTime;
               skyInfo[i - priorElemCount].data =
                     (int)myRound(match[i].value[pnt].data, 0);
               skyInfo[i - priorElemCount].valueType =
                     match[i].value[pnt].valueType;
            }
            else
               priorElemCount += 1;
         }

         /* Fill Temperature Array. */

         priorElemCount = 0;
         for (i = 0; i < numMatch; i++)
         {

            if (match[i].elem.ndfdEnum == NDFD_TEMP)
            {
               tempInfo[i - priorElemCount].validTime = match[i].validTime;
               tempInfo[i - priorElemCount].data =
                     (int)myRound(match[i].value[pnt].data, 0);
               tempInfo[i - priorElemCount].valueType =
                     match[i].value[pnt].valueType;
            }
            else
               priorElemCount += 1;
         }
      }

      if (f_wx == 1)
      {
         /* Format the weather and display name elements. */

         weather = xmlNewChild(parameters, NULL, BAD_CAST "weather", NULL);
         xmlNewProp(weather, BAD_CAST "time-layout", BAD_CAST layoutKey);

         /* Format the display name. */

         xmlNewChild(weather, NULL, BAD_CAST "name", BAD_CAST
                     "Weather Type, Coverage, and Intensity");
      }

      if (f_icon)
         iconInfo = malloc(numRowsWX * sizeof(icon_def));

      /* Loop over just the Wx data values now and format them. */

      for (wxIndex = 0; wxIndex < numRowsWX; wxIndex++)
      {
         /* Reset a few things */
	 skyCoverTimeEqualsWeatherTime = 0;
	 windTimeEqualsWeatherTime = 0;
	 hourlyTempTimeEqualsWeatherTime = 0;
         memset(WxValues, '\0', 5 * 50);
         memset(WxGroups, '\0', 10 * 100);
         numGroups = 0;

         if (wxInfo[wxIndex].valueType == 2)
            valueIsMissing = 1;

         if (valueIsMissing != 1)
         {

            /* Determine if this interation is occuring during the day or
             * night. */
            itIsNightTime = isNightPeriod(wxInfo[wxIndex].validTime, lat, lon);

            /* Lets remove the <> that surround the weather data coming from
             * NDFD (type, coverage, intensity, visibility, and qualifier). */
            pstr = strchr(wxInfo[wxIndex].str, '<');
            while (pstr != NULL)
            {
               wxInfo[wxIndex].str[pstr - wxInfo[wxIndex].str] = ' ';
               pstr = strchr(pstr + 1, '<');
            }

            pstr = strchr(wxInfo[wxIndex].str, '>');
            while (pstr != NULL)
            {
               wxInfo[wxIndex].str[pstr - wxInfo[wxIndex].str] = ' ';
               pstr = strchr(pstr + 1, '>');
            }

            /* Now put the weather groupings into an array using the ^ as the 
             * delimiter. Fill the first array elements before the others. */

            pstr = wxInfo[wxIndex].str;
            groupIndex = 0;
            for (i = 0; pstr[i]; i++)
            {
               if (pstr[i] != '^')
                  WxGroups[groupIndex][i] = pstr[i];
               else if (pstr[i] == '^')
               {
                  WxGroups[groupIndex][i] = '\0';
                  break;
               }
            }

            /* Get the total number of WxGroups for this one row of weather
             * data. */

            pstr1 = strchr(wxInfo[wxIndex].str, '^');
            while (pstr1 != NULL)
            {
               numGroups += 1;
               pstr1 = strchr(pstr1 + 1, '^');
            }

            /* Continue filling the array of WxGroups. */

            pstr = strchr(wxInfo[wxIndex].str, '^');
            pstr2 = strchr(wxInfo[wxIndex].str, '^');

            for (groupIndex = 1; groupIndex < numGroups + 1; groupIndex++)
            {
               for (i = 1; pstr[i]; i++)
               {
                  if (pstr[i] != '^')
                  {
                     WxGroups[groupIndex][i - 1] = pstr[i];
                  }
                  else if (pstr[i] == '^')
                  {
                     WxGroups[groupIndex][i - 1] = '\0';
                     pstr = strchr(pstr + 1, '^');
                     break;
                  }
               }
            }

            if (numGroups == 0)
            {
               if (WxGroups[numGroups][1] == 'N'
                   && WxGroups[numGroups][2] == 'o')
               {
                  WxGroups[numGroups][i - 1] = '\0';
               }
               else
               {
                  WxGroups[numGroups][i] = '\0';
               }
            }
            else
            {
               if (pstr2[i - 1] == ':')
                  WxGroups[numGroups][i - 1] = '\0';
            }

            /* If there is more than one group we connect them with an "and". 
             */

            if (numGroups > 0)
               strcpy(additive_value, "and");

            foundWeatherValue = 0;

            /* Determine if the sky cover, temperatures, and wind have the
             * same time as weather.  There are times when weather is every 3
             * hours while wind temp and sky cover are every 6 hours. Only do
             * this if icons are to be derived. */
            if (f_icon)
            {
               if (skyInfo[skyIndex].validTime == wxInfo[wxIndex].validTime)
                  skyCoverTimeEqualsWeatherTime = 1;
               if (wsInfo[wsIndex].validTime == wxInfo[wxIndex].validTime)
                  windTimeEqualsWeatherTime = 1;
               if (tempInfo[hourlyTempIndex].validTime ==
                   wxInfo[wxIndex].validTime)
                  hourlyTempTimeEqualsWeatherTime = 1;
            }

            /* Loop over each group and process its weather information (i.e.
             * type, coverage intensity, visibility, and qualifier, all which
             * are denoted by a ":". */

            wxCoverage = (char **)calloc(numGroups + 1, sizeof(char *));
            wxType = (char **)calloc(numGroups + 1, sizeof(char *));
            wxIntensity = (char **)calloc(numGroups + 1, sizeof(char *));
            wxVisibility = (char **)calloc(numGroups + 1, sizeof(char *));
            wxQualifier = (char **)calloc(numGroups + 1, sizeof(char *));

            for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
            {
               numValues = 0;

               /* Create the associative array holding the weather info
                * fields. */
               pstr = WxGroups[groupIndex];
               valueIndex = 0;

               for (i = 0; pstr[i]; i++)
               {
                  if (pstr[i] != ':')
                     WxValues[valueIndex][i] = pstr[i];
                  else if (pstr[i] == ':')
                  {
                     WxValues[valueIndex][i] = '\0';
                     break;
                  }

               }

               /* Get the total number of WxValues (should be 5, but since we are
		* not dealing with 'visibility', it will be bumped down to 4). */
               pstr1 = strchr(WxGroups[groupIndex], ':');
               while (pstr1 != NULL)
               {
                  numValues += 1;
                  pstr1 = strchr(pstr1 + 1, ':');
               }

               /* Bump this number up by one to account for the Wx Qualifier. 
                */

               numValues += 1;

               /* Continue filling the array of WxValues */

               pstr = strchr(WxGroups[groupIndex], ':');
               for (valueIndex = 1; valueIndex < numValues; valueIndex++)
               {
                  for (i = 1; pstr[i]; i++)
                  {
                     if (pstr[i] != ':')
                     {
                        WxValues[valueIndex][i - 1] = pstr[i];
                     }

                     else if (pstr[i] == ':')
                     {
                        WxValues[valueIndex][i - 1] = '\0';
                        pstr = strchr(pstr + 1, ':');
                        break;
                     }
                  }
               }
               WxValues[4][i - 1] = '\0';

               /* Weather Coverage */

               if (WxValues[0][1] == 'N' && WxValues[0][2] == 'o')
               {
                  wxCoverage[groupIndex] = malloc(strlen("none") + 1);
                  strcpy(wxCoverage[groupIndex], "none");
               }
               else
               {
                  strTrim(WxValues[0]);
                  wxCoverage[groupIndex] = malloc(strlen(WxValues[0]) + 1);
                  strcpy(wxCoverage[groupIndex], WxValues[0]);
               }

               /* Weather Type */

               if (WxValues[1][1] == 'N' && WxValues[1][2] == 'o')
               {
                  wxType[groupIndex] = malloc(strlen("none") + 1);
                  strcpy(wxType[groupIndex], "none");
               }
               else
               {
                  strTrim(WxValues[1]);
                  wxType[groupIndex] = malloc(strlen(WxValues[1]) + 1);
                  strcpy(wxType[groupIndex], WxValues[1]);
               }

               /* Weather Intensity */

               if (WxValues[2][1] == 'N' && WxValues[2][2] == 'o')
               {
                  wxIntensity[groupIndex] = malloc(strlen("none") + 1);
                  strcpy(wxIntensity[groupIndex], "none");
               }
               else
               {
                  strTrim(WxValues[2]);

                  wxIntensity[groupIndex] = malloc(strlen(WxValues[2]) + 1);
                  strcpy(wxIntensity[groupIndex], WxValues[2]);
               }

               /* Weather Visibility */

               if (WxValues[3][1] == 'N' && WxValues[3][2] == 'o')
               {
                  wxVisibility[groupIndex] = malloc(strlen("none") + 1);
                  strcpy(wxVisibility[groupIndex], "none");
               }
               else
               {
                  strTrim(WxValues[3]);
                  wxVisibility[groupIndex] = malloc(strlen(WxValues[3]) + 1);
                  strcpy(wxVisibility[groupIndex], WxValues[3]);
               }

               /* Weather Qualifier */

               if (WxValues[4][1] == 'N' && WxValues[4][2] == 'o')
               {
                  wxQualifier[groupIndex] = malloc(strlen("none") + 1);
                  strcpy(wxQualifier[groupIndex], "none");
               }
               else if (WxValues[4][0] == '\0')
               {
                  wxQualifier[groupIndex] = malloc(strlen("none") + 1);
                  strcpy(wxQualifier[groupIndex], "none");
               }
               else
               {
                  strTrim(WxValues[4]);
                  wxQualifier[groupIndex] = malloc(strlen(WxValues[4]) + 1);
                  strcpy(wxQualifier[groupIndex], WxValues[4]);
               }

               /* Set the no weather occuring flag. */

               if (strcmp(wxType[groupIndex], "none") != 0)
               {
                  foundWeatherValue = 1;

               }

               /* Re-initialize the WxValues array. */

               memset(WxValues, '\0', 5 * 50);

            }                 /* Closing out groupIndex for loop */

            /* If there is data we format it into a weather conditions
             * element. */

            if (foundWeatherValue)
            {
               if (f_wx == 1) /* Format start of weather conditions element,
                               * if applicable. */
               {
                  weather_conditions = xmlNewChild(weather, NULL, BAD_CAST
                                                   "weather-conditions", NULL);

                  /* Loop over each group and format value element. */

                  for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
                  {
                     value = xmlNewChild(weather_conditions, NULL, BAD_CAST
                                         "value", NULL);
                     getTranslatedCoverage(wxCoverage[groupIndex],
                                           transCoverageStr);
                     getTranslatedType(wxType[groupIndex], transTypeStr);
                     getTranslatedIntensity(wxIntensity[groupIndex],
                                            transIntensityStr);
                     getTranslatedVisibility(wxVisibility[groupIndex],
                                             transVisibilityStr);
                     getTranslatedQualifier(wxQualifier[groupIndex],
                                            transQualifierStr);

                     xmlNewProp(value, BAD_CAST "coverage", BAD_CAST
                                transCoverageStr);
                     xmlNewProp(value, BAD_CAST "intensity", BAD_CAST
                                transIntensityStr);

                     if (groupIndex > 0)  /* Groups other than first require
                                           * additive attribute. */
                     {
                        if (strcmp(transQualifierStr, "or") == 0)
                           strcpy(additive_value, "or");
                        xmlNewProp(value, BAD_CAST "additive", BAD_CAST
                                   additive_value);
                     }

                     xmlNewProp(value, BAD_CAST "weather-type", BAD_CAST
                                transTypeStr);
                     xmlNewProp(value, BAD_CAST "qualifier", BAD_CAST
                                transQualifierStr);

                     /* Format visibility as a seperate element (not an
                      * attribute). If no visibility restriction, format a
                      * nil attribute. */

                     if (strcmp(wxVisibility[groupIndex], "none") == 0 ||
                         wxVisibility[groupIndex] == NULL)
                     {
                        visibility = xmlNewChild(value, NULL, BAD_CAST
                                                 "visibility", NULL);
                        xmlNewProp(visibility, BAD_CAST "xsi:nil", BAD_CAST
                                   "true");
                     }
                     else     /* Format the visibility data. */
                     {
                        visibility = xmlNewChild(value, NULL, BAD_CAST
                                                 "visibility",
                                                 BAD_CAST transVisibilityStr);
                        xmlNewProp(visibility, BAD_CAST "units",
                                   BAD_CAST "statute miles");
                     }
                  }
               }

               /* Create and then save the weather icon based on forecast
                * weather types. */

               if (f_icon)
               {
                  determineWeatherIcons(iconInfo, numGroups, wxType,
                                        skyCoverTimeEqualsWeatherTime,
                                        itIsNightTime, skyInfo, baseURL,
                                        numRowsSKY, skyIndex, wxIndex,
                                        windTimeEqualsWeatherTime, wsInfo,
                                        wsIndex, numRowsWS, numRowsTEMP,
                                        hourlyTempIndex,
                                        hourlyTempTimeEqualsWeatherTime,
                                        tempInfo);

                  /* Update the indexes. */

                  if (skyCoverTimeEqualsWeatherTime && skyIndex < numRowsSKY)
                     skyIndex += 1;

                  if (windTimeEqualsWeatherTime && wsIndex < numRowsWS)
                     wsIndex += 1;

                  if (hourlyTempTimeEqualsWeatherTime && hourlyTempIndex <
                      numRowsTEMP)
                     hourlyTempIndex += 1;
               }
            }

            else
               /* No weather occurring, so format empty weather conditions
                * element. */
            {
               if (f_wx == 1)
                  weather_conditions = xmlNewChild(weather, NULL, BAD_CAST
                                                   "weather-conditions", NULL);

               if (f_icon)
               {
                  /* Determine the conditions icon based on sky cover. */

                  determineSkyIcons(skyCoverTimeEqualsWeatherTime,
                                    itIsNightTime, skyIndex, wxIndex, skyInfo,
                                    iconInfo, baseURL, numRowsSKY);

                  /* Determine the conditions icon based on things like
                   * extreme temperatures and strong winds. */

                  determineNonWeatherIcons(windTimeEqualsWeatherTime,
                                           itIsNightTime, wsInfo, wsIndex,
                                           baseURL, numRowsWS, iconInfo,
                                           wxIndex, numRowsTEMP, tempInfo,
                                           hourlyTempIndex,
                                           hourlyTempTimeEqualsWeatherTime);

                  /* Update the indexes. */

                  if (skyCoverTimeEqualsWeatherTime && skyIndex < numRowsSKY)
                     skyIndex += 1;

                  if (windTimeEqualsWeatherTime && wsIndex < numRowsWS)
                     wsIndex += 1;

                  if (hourlyTempTimeEqualsWeatherTime && hourlyTempIndex <
                      numRowsTEMP)
                     hourlyTempIndex += 1;

               }

            }

            /* Re-initialize the WxGroups array and the 5 weather fields. */

            memset(WxGroups, '\0', 10 * 100);

            for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
            {
               free(wxCoverage[groupIndex]);
               free(wxType[groupIndex]);
               free(wxIntensity[groupIndex]);
               free(wxVisibility[groupIndex]);
               free(wxQualifier[groupIndex]);
            }

            free(wxCoverage);
            free(wxType);
            free(wxIntensity);
            free(wxVisibility);
            free(wxQualifier);

         }

         else
            /* Weather is Missing, so format the nil attribute. */
         {
            /* Format start of weather conditions element, if applicable. */

            if (f_wx == 1)
            {
               weather_conditions = xmlNewChild(weather, NULL, BAD_CAST
                                                "weather-conditions", NULL);
               value = xmlNewChild(weather_conditions, NULL, BAD_CAST "value",
                                   NULL);
               xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
            }
            /* No data so indicate there is no icon. */
            if (f_icon)
            {
               strcpy(iconInfo[wxIndex].str, "none");

               /* Update indexes. */

               if (skyCoverTimeEqualsWeatherTime && skyIndex < numRowsSKY)
                  skyIndex += 1;

               if (windTimeEqualsWeatherTime && wsIndex < numRowsWS)
                  wsIndex += 1;

               if (hourlyTempTimeEqualsWeatherTime && hourlyTempIndex <
                   numRowsTEMP)
                  hourlyTempIndex += 1;
            }
         }
      }                       /* End of formatting weather-conditions
                               * element. */

      /* Only incorporate weather if it is needed.  This could be just a
       * weather conditions icon scenario, which is possible in the DWMLgen
       * "time-series" product if user only selects "Weather Conditions
       * Icons". Of course, these icons are based on Weather, it's just that
       * the weather is not formatted to output in this instance. Weather
       * comes before conditions based on sequence in schema. */

      /* Having saved the appropriate weather icons paths and files names in
       * the array iconLinks, format the XML to hold the links. */

      if (f_icon)
      {
         genIconLinks(iconInfo, numRowsWX, layoutKey, parameters);
         free(iconInfo);
      }

      /* Free some things. */
      free(wxInfo);
      if (f_icon)
      {
         free(wsInfo);
         free(skyInfo);
         free(tempInfo);
      }

   }

   return;

}

/******************************************************************************
 * genMaxTempValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Max Temperature element in the DWMLgen and 
 *  DWMLgenByDay products.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Max Temps to their valid times 
 *               (ex. k-p24h-n8-1). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 * f_formatNIL = We may have to create a NIL value for the first max temp if the
 *               request for data is late in the day. The variable f_formatNIL
 *               and firstNIL will have been set to = 1 to denote this case.
 *       f_XML = Flag denoting type of XML product (1 = DWMLgen's "time-series"
 *               product, 2 = DWMLgen's "glance" product, 3 = DWMLgenByDay's 
 *               "12 hourly" product, 4 = DWMLgenByDay's "24 hourly" product.
 *               (Input) 
 *     numRows = Number of rows data is formatted for in the output XML. Used
 *               in DWMLgenByDay's "12 hourly" and "24 hourly" products. 
 *               "numRows" is determined using numDays and is used as an added 
 *               criteria (above and beyond simply having data exist for a 
 *               certain row) in formatting XML for these two products. (Input)
 * numFmtdRows = Number of output lines formatted in DWMLgenByDay products. 
 *               (Input)          
 * startTime_cml = The startTime entered as an option on the command line. 
 *                 (Input)
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

static void genMaxTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                             size_t numMatch, xmlNodePtr parameters,
                             int *f_formatNIL, uChar f_XML, 
			     double startTime_cml, uChar numRows, 
			     int numFmtdRows)
{

   int i;                      /* Element counter thru match structure. */
   int numNils = 0;           /* Denotes diff between number of data rows and 
				 the number that need to be formatted for the 
				 DWMLgenByDay products. */
   int firstNIL = 0;          /* Set to 1 if first Max Temp is set to =
                               * "nil". 
			       */
   int roundedMaxData;        /* Returned rounded data. */
   xmlNodePtr temperature = NULL; /* Xml Node Pointer for <temperature>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <temperature> element. */

   temperature = xmlNewChild(parameters, NULL, BAD_CAST "temperature", NULL);
   xmlNewProp(temperature, BAD_CAST "type", BAD_CAST "maximum");
   xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Fahrenheit");
   xmlNewProp(temperature, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */

   xmlNewChild(temperature, NULL, BAD_CAST "name", BAD_CAST
               "Daily Maximum Temperature");

   /* If DWMLgen product, set numFmtdRows = to numRows since there is no set
    * number of rows we are ultimately formatting.
    */
   if (f_XML == 1 || f_XML == 2)
      numFmtdRows = numRows;

   /* Format the first Max Temp Day as "nil = true" if f_formatNIL = 1
    * (applicable for DWMLgenByDay product's "24 hourly" format). */

   if (f_XML == 4 && *f_formatNIL)
   {
      value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
      xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
      firstNIL = 1;
      *f_formatNIL = 0;
   }

   /* Loop over all the data values and format them. */

   for (i = firstNIL; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_MAX)
      {
         if (i < numFmtdRows) /* Accounts for DWMLgenByDay. */
         {
            if (f_XML == 3 || f_XML == 4)  /* DWMLgenByDay products. */
            {
               if (i < numRows)
               {

                  /* If the data is missing so indicate in the XML (nil=true). */
   
                  if (match[i].value[pnt].valueType == 2)
                  {
                     value = xmlNewChild(temperature, NULL, BAD_CAST "value",
                                         NULL);
                     xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
                  }
                  else if (match[i].value[pnt].valueType == 0) /* Good data. */
                  {
                     roundedMaxData = (int)myRound(match[i].value[pnt].data, 0);
                     sprintf(strBuff, "%d", roundedMaxData);
                     xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST
                                 strBuff);
                  }
               }
            }
            else if (f_XML == 1 || f_XML == 2) /* DWMLgen products. */
            {
               /* If the data is missing, so indicate in the XML (nil=true). */

               if (match[i].value[pnt].valueType == 2)
               {
                  value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
                  xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
               }
               else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                              * data. */
               {
                  roundedMaxData = (int)myRound(match[i].value[pnt].data, 0);
                  sprintf(strBuff, "%d", roundedMaxData);
                  xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST
                              strBuff);
               }
            }
	 }
      }
   }

   /* In certain cases for the DWMLgenByDay products, we'll need to account for 
    * times when there may be less data in the match structure than the amount 
    * of data that needs to be formatted. These "extra" spaces will need to be 
    * formatted with a "nil" attribute. 
    */
   if (f_XML == 3 || f_XML == 4)  /* DWMLgenByDay products. */
   {
      /* Tally up the number of iterations that occurred thru the match 
       * structure and compare to the number of actual data rows to see if there
       * is a difference.
       */
      numNils = numFmtdRows - numRows;
      if (numNils > 0)
      {
         for (i = 0; i < numNils; i++)
	 {
            value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
	 }
      }
   }

   return;
}

/******************************************************************************
 * genMinTempValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Min Temperature element in the DWMLgen and
 *  DWMLgenByDay products.
 *
 * ARGUMENTS
 *          pnt = Current Point index. (Input)
 *    layoutKey = The key linking the Min Temps to their valid times 
 *                (ex. k-p24h-n7-1). (Input)
 *        match = Pointer to the array of element matches from degrib. (Input) 
 *     numMatch = The number of matches from degrib. (Input)
 *   parameters = An xml Node Pointer denoting the <parameters> node to which 
 *                these values will be attached (child node). (Output)
 *        f_XML = Flag denoting type of XML product (1 = DWMLgen's "time-series"
 *                product, 2 = DWMLgen's "glance" product, 3 = DWMLgenByDay's 
 *                "12 hourly" product, 4 = DWMLgenByDay's "24 hourly" product.
 *                (Input) 
 *      numRows = Number of rows NDFD data exists for for MinT element. (Input)
 *   currentDay = Current day's 2 digit date. (Input)
 *  currentHour = Current hour =in 2 digit form. (Input)
 *     TZoffset = Number of hours to add to current time to get GMT time. 
 *                (Input)
 * f_observeDST = Flag determining if current point observes Daylight Savings 
 *                Time. (Input)  
 * numFmtdRows = Number of output lines formatted in DWMLgenByDay products. 
 *                (Input)  
 * startTime_cml = The startTime entered as an option on the command line. 
 *                 (Input)
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
static void genMinTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                             size_t numMatch, xmlNodePtr parameters,
                             uChar f_XML, double startTime_cml, uChar numRows, 
			     char *currentDay, char *currentHour, sChar TZoffset,
                             sChar f_observeDST, int numFmtdRows)
{

   int i;                     /* Element counter thru match structure. */
   int numNils = 0;           /* Denotes diff between number of data rows and 
				 the number that need to be formatted for the 
				 DWMLgenByDay products. */
   int counter = 0;           /* Initialize counter to move thru the min temp 
                               * values. */
   int MinTCounter = 0;       /* Flag denoting first MinT data row. */
   int roundedMinData;        /* Returned rounded data. */
   int priorElemCount = 0;    /* Used to subtract prior elements when looping 
                               * thru matches. */
   xmlNodePtr temperature = NULL; /* Xml Node Pointer for <temperature>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   char str1[30];             /* Temporary string holding formatted time
                               * value. */
   char MinTDay[3];           /* Date (day) of the first Min Temp Data value. 
                               */

   /* Format the <temperature> element. */

   temperature = xmlNewChild(parameters, NULL, BAD_CAST "temperature", NULL);
   xmlNewProp(temperature, BAD_CAST "type", BAD_CAST "minimum");
   xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Fahrenheit");
   xmlNewProp(temperature, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */
   xmlNewChild(temperature, NULL, BAD_CAST "name", BAD_CAST
               "Daily Minimum Temperature");
   
   /* If DWMLgen product, set numFmtdRows = to numRows since there is no set
    * number of rows we are ultimately formatting. IF DWMLgenByDay product, 
    * check to see if the number of data rows is less than the number of rows
    * to be formatted. If so, set numRows = numFmtdRows (will result in a "nil"
    * being formatted).
    */
   if (f_XML == 1 || f_XML == 2)
      numFmtdRows = numRows;

   /* Loop over all the data values and format them. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_MIN)
      {
         if (i - priorElemCount < numFmtdRows) /* Accounts for DWMLgenByDay. */
         {
            if (f_XML == 3 || f_XML == 4)  /* DWMLgenByDay products. */
            {
		 
               /* Early in the morning (midnight - 6AM) the code will want to
                * include the the overnight min temp in the 6AM to 6PM time
                * period, the following code causes the program to correctly
                * skip the first min temp. Only applicable for DWMLgenByDay's
                * "24 hourly" product. */

               if (f_XML == 4 && atoi(currentHour) <= 7 && MinTCounter == 0)
               {
                  /* Get the end time of the first min temp's valid time (i.e. 07
                   * or 19). */
                  formatValidTime(match[i + counter].validTime, str1, 30, TZoffset,
                                  f_observeDST);
   
                  MinTDay[0] = str1[8];
                  MinTDay[1] = str1[9];
                  MinTDay[2] = '\0';

                  if (atoi(MinTDay) == atoi(currentDay))
                     counter = 1;

                  MinTCounter++;
               }

               if (i - priorElemCount < numRows)
               {

                  /* If the data is missing, so indicate in the XML (nil=true).
		   * Also, add a check to make sure the counter does not cause
		   * a bleed over and pick up the next element in the match
		   * structure.
		   */
                  if ((match[i + counter].value[pnt].valueType == 2) || 
		      (match[i + counter].elem.ndfdEnum != NDFD_MIN))
 
                  {
                     value = xmlNewChild(temperature, NULL, BAD_CAST "value",
                                         NULL);
                     xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
                  }
                  else if (match[i + counter].value[pnt].valueType == 0)
                  {
                     roundedMinData =
                           (int)myRound(match[i + counter].value[pnt].data, 0);
                     sprintf(strBuff, "%d", roundedMinData);
                     xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST
                                 strBuff);
                  }
               }
            }
            else if (f_XML == 1 || f_XML == 2) /* DWMLgen products. */
            {
               /* If the data is missing, so indicate in the XML (nil=true). */

               if (match[i].value[pnt].valueType == 2)
               {
                  value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
                  xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
               }
               else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                              * data. */
               {
                  roundedMinData = (int)myRound(match[i].value[pnt].data, 0);
                  sprintf(strBuff, "%d", roundedMinData);
                  xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST
                              strBuff);
               }
	    }
         }
      }
      else
         priorElemCount++;
   }

   /* In certain cases for the DWMLgenByDay products, we'll need to account for 
    * times when there may be less data in the match structure than the amount 
    * of data that needs to be formatted. These "extra" spaces will need to be 
    * formatted with a "nil" attribute. 
    */
   if (f_XML == 3 || f_XML == 4)  /* DWMLgenByDay products. */
   {
      /* Tally up the number of iterations that occurred thru the match 
       * structure and compare to the number of actual data rows to see if there
       * is a difference.
       */
      numNils = numFmtdRows - numRows;
      if (numNils > 0)
      {
         for (i = 0; i < numNils; i++)
	 {
            value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
	 }
      }
   }
   
   return;
}

/******************************************************************************
 * genTempValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Temperature element in the "time-series" DWMLgen 
 *  product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Hourly Temps to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
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
static void genTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                          size_t numMatch, xmlNodePtr parameters)
{
   int i;
   int roundedTempData;       /* Returned rounded data. */
   xmlNodePtr temperature = NULL; /* Xml Node Pointer for <temperature>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   /* Format the <temperature> element. */

   temperature = xmlNewChild(parameters, NULL, BAD_CAST "temperature", NULL);
   xmlNewProp(temperature, BAD_CAST "type", BAD_CAST "hourly");
   xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Fahrenheit");
   xmlNewProp(temperature, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */

   xmlNewChild(temperature, NULL, BAD_CAST "name", BAD_CAST "Temperature");

   /* Loop over all the data values and format them. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_TEMP)
      {
         /* If the data is missing, so indicate in the XML (nil=true). */

         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedTempData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedTempData);
            xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}

/******************************************************************************
 * genDewPointTempValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Dew Point element in the "time-series" DWMLgen 
 *  product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Min Temps to their valid times 
 *               (ex. k-p24h-n7-1). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
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
static void genDewPointTempValues(size_t pnt, char *layoutKey,
                                  genMatchType * match, size_t numMatch,
                                  xmlNodePtr parameters)
{
   int i;
   int roundedTdData;         /* Returned rounded data. */
   xmlNodePtr temperature = NULL; /* Xml Node Pointer for <temperature>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <temperature> element. */

   temperature = xmlNewChild(parameters, NULL, BAD_CAST "temperature", NULL);
   xmlNewProp(temperature, BAD_CAST "type", BAD_CAST "dew point");
   xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Fahrenheit");
   xmlNewProp(temperature, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */

   xmlNewChild(temperature, NULL, BAD_CAST "name", BAD_CAST
               "Dew Point Temperature");

   /* Loop over all the data values and format them. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_TD)
      {

         /* If the data is missing, so indicate in the XML (nil=true). */

         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedTdData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedTdData);
            xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}

/******************************************************************************
 * genAppTempValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Apparent Temperature element in the "time-series" 
 *  DWMLgen product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Apparent Temps to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
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
static void genAppTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                             size_t numMatch, xmlNodePtr parameters)
{
   int i;
   int roundedAtData;         /* Returned rounded data. */
   xmlNodePtr temperature = NULL; /* Xml Node Pointer for <temperature>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <temperature> element. */

   temperature = xmlNewChild(parameters, NULL, BAD_CAST "temperature", NULL);
   xmlNewProp(temperature, BAD_CAST "type", BAD_CAST "apparent");
   xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Fahrenheit");
   xmlNewProp(temperature, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */
   xmlNewChild(temperature, NULL, BAD_CAST "name", BAD_CAST
               "Apparent Temperature");

   /* Loop over all the data values and format them. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_AT)
      {

         /* If the data is missing, so indicate in the XML (nil=true). */

         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedAtData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedAtData);
            xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}

/******************************************************************************
 * genQPFValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the QPF element in the "time-series" DWMLgen product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Hourly Temps to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
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
static void genQPFValues(size_t pnt, char *layoutKey, genMatchType * match,
                         size_t numMatch, xmlNodePtr parameters)
{
   int i;
   float roundedQPFData;      /* Returned rounded data. QPF data rounds to 2
                               * sig- nificant digits. */
   xmlNodePtr precipitation = NULL; /* Xml Node Pointer for <precipitation>
                                     * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <precipitation> element. */

   precipitation = xmlNewChild(parameters, NULL, BAD_CAST "precipitation",
                               NULL);
   xmlNewProp(precipitation, BAD_CAST "type", BAD_CAST "liquid");
   xmlNewProp(precipitation, BAD_CAST "units", BAD_CAST "inches");
   xmlNewProp(precipitation, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */

   xmlNewChild(precipitation, NULL, BAD_CAST "name", BAD_CAST
               "Liquid Precipitation Amount");

   /* Loop over all the data values and format them. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_QPF)
      {

         /* If the data is missing, so indicate in the XML (nil=true). */

         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(precipitation, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedQPFData = (float)myRound(match[i].value[pnt].data, 2);
            sprintf(strBuff, "%2.2f", roundedQPFData);
            xmlNewChild(precipitation, NULL, BAD_CAST "value", BAD_CAST
                        strBuff);
         }
      }
   }
   return;
}

/******************************************************************************
 * genSnowValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Snow Amount element in the "time-series" DWMLgen 
 *  product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Snow Amounts to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
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
static void genSnowValues(size_t pnt, char *layoutKey, genMatchType * match,
                          size_t numMatch, xmlNodePtr parameters)
{
   int i;
   int roundedSnowData;       /* Returned rounded data. */
   xmlNodePtr precipitation = NULL; /* Xml Node Pointer for <precipitation>
                                     * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <precipitation> element. */

   precipitation = xmlNewChild(parameters, NULL, BAD_CAST "precipitation",
                               NULL);
   xmlNewProp(precipitation, BAD_CAST "type", BAD_CAST "snow");
   xmlNewProp(precipitation, BAD_CAST "units", BAD_CAST "inches");
   xmlNewProp(precipitation, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */

   xmlNewChild(precipitation, NULL, BAD_CAST "name", BAD_CAST "Snow Amount");

   /* Loop over all the data values and format them. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_SNOW)
      {

         /* If the data is missing, so indicate in the XML (nil=true). */

         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(precipitation, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedSnowData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedSnowData);
            xmlNewChild(precipitation, NULL, BAD_CAST "value", BAD_CAST
                        strBuff);
         }
      }
   }
   return;
}

/******************************************************************************
 * genPopValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code performs two functions: 
 *            1) Formats the Pop12hr element in the "time-series" DWMLgen 
 *               product. 
 *            2) Collects the Max Pop values per day (12 or 24 hr period) for 
 *               icon determination in the DWMLgenByDay products.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the 12 Hour Pops to their valid times 
 *               (ex. k-p12h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 *     numRows = Number of rows data is formatted for in the output XML. Used
 *               in DWMLgenByDay's "12 hourly" and "24 hourly" products. 
 *               "numRows" is determined using numDays and is used as an added 
 *               criteria (above and beyond simply having data exist for a 
 *               certain row) in formatting XML for these two products. (Input)
 *       f_XML = Flag denoting type of XML product (1 = DWMLgen's "time-series"
 *               product, 2 = DWMLgen's "glance" product, 3 = DWMLgenByDay's 
 *               "12 hourly" product, 4 = DWMLgenByDay's "24 hourly" product.
 *               (Input) 
 * maxDailyPop = Array containing the pop values corresponding to a day (24 
 *               hour format) or 12 hour period (12 hour format). For 24 hour 
 *               format, we use the maximum of the two 12 hour pops that span 
 *		 the day. This variable is used to test if the pop is large 
 *		 enough to justify formatting weather values. (Input)
 * startTime_cml = The startTime entered as an option on the command line. 
 *                 (Input)
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
static void genPopValues(size_t pnt, char *layoutKey, genMatchType * match,
                         size_t numMatch, xmlNodePtr parameters, uChar numRows,
                         uChar f_XML, double startTime_cml, int *maxDailyPop, 
			 int numDays, double currentDoubTime, char *currentHour, 
			 int numPopRowsSkippedBeginning)
{
   int i;
   int numNils = 0;           /* Denotes diff between number of data rows and 
				 the number that need to be formatted for the 
				 DWMLgenByDay products. */
   int numFmtdRows = 0;       /* Number of output lines in DWMLgenByDay products. */
   int numPopRowsSkippedTemp = 0;
   int dayCount = 0;          /* Used to keep count of which day we are
                               * processing (two PoPs per day). */
   int startOverCount = 0;    /* Subset iteration of just PoPs in the match
                               * structure. */
   int roundedPopData = 0;    /* Returned rounded data. */
   xmlNodePtr precipitation = NULL; /* Xml Node Pointer for <precipitation>
                                     * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   int priorElemCount = 0;    /* Used to subtract prior elements when looping 
                               * thru matches. */

   /* Set numFmtdRows from numDays. */
   numFmtdRows = numDays*2;
   
   /* Format the <precipitation> element. */

   precipitation = xmlNewChild(parameters, NULL, BAD_CAST
                               "probability-of-precipitation", NULL);
   xmlNewProp(precipitation, BAD_CAST "type", BAD_CAST "12 hour");
   xmlNewProp(precipitation, BAD_CAST "units", BAD_CAST "percent");
   xmlNewProp(precipitation, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */

   xmlNewChild(precipitation, NULL, BAD_CAST "name", BAD_CAST
               "12 Hourly Probability of Precipitation");
   
   /* If DWMLgen product, set numFmtdRows = to numRows because we have a set
    * number of rows we are ultimately formatting.
    */
   if (f_XML == 1 || f_XML == 2)
      numFmtdRows = numRows;
   else if (f_XML == 3 || f_XML == 4) 
      numFmtdRows = numDays*2;

   /* Loop over all the data values and format them. */
   numPopRowsSkippedTemp = numPopRowsSkippedBeginning;
   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_POP)
      {
         if (i - priorElemCount < numFmtdRows) /* Accounts for DWMLgenByDay. */
         {
            if (numPopRowsSkippedTemp == 0)
            {		    
	       if (f_XML == 3 || f_XML == 4) /* DWMLgenByDay products. */
               {
                  startOverCount++;

                  /* If the data is missing, so indicate in the XML (nil=true). */

                  if (match[i].value[pnt].valueType == 2)
                  {
                     value = xmlNewChild(precipitation, NULL, BAD_CAST "value",
                                         NULL);
                     xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
                  }
                  else if (match[i].value[pnt].valueType == 0) /* Good data. */
                  {
                     roundedPopData = (int)myRound(match[i].value[pnt].data, 0);
                     sprintf(strBuff, "%d", roundedPopData);
                     xmlNewChild(precipitation, NULL, BAD_CAST "value", BAD_CAST
                                 strBuff);
                  }

                  /* Make some adjustments for the next loop interation. */

                  if (f_XML == 4 && match[i].value[pnt].valueType == 0)
                  {
                     /* For 24hr format, get the max PoP to represent the day. */
                     if (roundedPopData > maxDailyPop[dayCount] && dayCount <= numDays)
	             {
                        maxDailyPop[dayCount] = roundedPopData;
                     }
                     /* We change to a new day every other PoP value. */
                     if (startOverCount % 2 == 0)
                        dayCount++;
                  }
                  else if (f_XML == 3 && match[i].value[pnt].valueType == 0)
                  {
                     /* For 12hr format, we use every PoP value. */
                     if (dayCount <= numDays*2)
                     {
                        maxDailyPop[dayCount] = roundedPopData;
                        dayCount++;
	             }
                  }
               }   
               else if (f_XML == 1 || f_XML == 2)
               {
                  /* If the data is missing, so indicate in the XML (nil=true). */

                  if (match[i].value[pnt].valueType == 2)
                  {
                     value = xmlNewChild(precipitation, NULL, BAD_CAST "value", NULL);
                     xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
                  }
                  else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                                 * data. */
                  {
                     roundedPopData = (int)myRound(match[i].value[pnt].data, 0);
                     sprintf(strBuff, "%d", roundedPopData);
                     xmlNewChild(precipitation, NULL, BAD_CAST "value", BAD_CAST
                                 strBuff);
                  }
               }
	    }
	    else
            {
               numPopRowsSkippedTemp--;
	       continue;
	    }   
	 }
      }
      else
         priorElemCount++;
   }

   /* In certain cases for the DWMLgenByDay products, we'll need to account for 
    * times when there may be less data in the match structure than the amount 
    * of data that needs to be formatted. These "extra" spaces will need to be 
    * formatted with a "nil" attribute. 
    */
   if (f_XML == 3 || f_XML == 4)  /* DWMLgenByDay products. */
   {
      /* Tally up the number of iterations that occurred thru the match 
       * structure and compare to the number of actual data rows to see if there
       * is a difference.
       */
      numNils = numFmtdRows - numRows;
      if (numNils > 0)
      {
         for (i = 0; i < numNils; i++)
	 {
            value = xmlNewChild(precipitation, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
	 }
      }
   }
   
   return;
}

/******************************************************************************
 * genWindSpeedValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code performs two functions: 
 *            1) Formats the Wind Speed element in the "time-series" DWMLgen 
 *               product. 
 *            2) Collects the Max Wind Speed values per day for icon 
 *               determination in the DWMLgenByDay products.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Wind Speeds to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 *   startDate = Point specific user supplied start Date (i.e. 2006-04-15). 
 *               (Input)
 * maxWindSpeed = Array containing the Maximum wind speed values corresponding
 *                to a day (24 hour format) or 12 hour period (12 hour format).
 *                These values are used in deriving the weather and/or icon values. 
 *                (Output)
 * valTimeForWindDirMatch = Array with the validTimes that corresponds to the 
 *                          times when the max wind speeds are the highest.  We 
 *                          then collect the wind directions that correspond
 *                          to the same times when the wind speeds are the 
 *			    highest. (Output)
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

static void genWindSpeedValues(size_t pnt, char *layoutKey,
                               genMatchType * match, size_t numMatch,
                               xmlNodePtr parameters, char *startDate,
                               int *maxWindSpeed, int numOutputLines,
                               int timeInterval, sChar TZoffset,
                               sChar f_observeDST, uChar parameterName,
                               uChar numRows, uChar f_XML,
                               double *valTimeForWindDirMatch)
{

   int i;
   int period; /* Length between an elements successive validTimes. */
   int forecastPeriod = 0;
   int priorElemCount = 0;
   int currentDay = 0;        /* Subset iteration of just wind Spds in the
                               * match structure. */
   int integerStartUserTime = 0;
   sChar DST;
   char userStart_year[6];
   char userStart_month[4];
   char userStart_day[3];
   char baseUserTime[30];
   char hourMinSecTZ[16]; /* String component holding "T06:00:00-00:00" part. */
   double baseUserTime_doub = 0.0;
   double WSdoubleTime = 0.0;
   int WSintegerTime = 0;
   int roundedWindSpeedData;  /* Returned rounded data. */
   xmlNodePtr wind_speed = NULL;  /* Xml Node Pointer for <wind-speed>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   char WSstr[30];            /* Temporary string holding formatted time
                               * value of wind speed. */

   /* If the product is of type DWMLgen, format the <wind_speed> element. */

   if (f_XML == 1 || f_XML == 2)
   {
      wind_speed = xmlNewChild(parameters, NULL, BAD_CAST "wind-speed", NULL);
      xmlNewProp(wind_speed, BAD_CAST "type", BAD_CAST "sustained");
      xmlNewProp(wind_speed, BAD_CAST "units", BAD_CAST "knots");
      xmlNewProp(wind_speed, BAD_CAST "time-layout", BAD_CAST layoutKey);

      /* Format the display name. */

      xmlNewChild(wind_speed, NULL, BAD_CAST "name", BAD_CAST "Wind Speed");
   }

   /* If product type is of DWMLgenByDay, parse user supplied start time. */

   if (f_XML == 3 || f_XML == 4)
   {
      userStart_year[0] = startDate[0];
      userStart_year[1] = startDate[1];
      userStart_year[2] = startDate[2];
      userStart_year[3] = startDate[3];
      userStart_year[4] = '-';
      userStart_year[5] = '\0';

      userStart_month[0] = startDate[5];
      userStart_month[1] = startDate[6];
      userStart_month[2] = '-';
      userStart_month[3] = '\0';

      userStart_day[0] = startDate[8];
      userStart_day[1] = startDate[9];
      userStart_day[2] = '\0';

      strcpy(baseUserTime, userStart_year);
      strcat(baseUserTime, userStart_month);
      strcat(baseUserTime, userStart_day);
      
      if (TZoffset < 0)
         sprintf(hourMinSecTZ, "T06:00:00%02d:00", TZoffset);
      else
         sprintf(hourMinSecTZ, "T06:00:00-%02d:00", TZoffset);
      
      strcat(baseUserTime, hourMinSecTZ);   

      Clock_Scan(&baseUserTime_doub, baseUserTime, 0);
      
      /* Before continuing, see if this point observes day light savings time, 
       * and if it is currently in effect. 
       */ 
      if (f_observeDST)
      {
         if (Clock_IsDaylightSaving2(baseUserTime_doub, 0) == 1)
         {
            DST = TZoffset - 1;
	    if  (TZoffset < 0)
               sprintf(hourMinSecTZ, "T06:00:00%02d:00", DST);
	    else
               sprintf(hourMinSecTZ, "T06:00:00-%02d:00", DST);
	    strcat(baseUserTime, hourMinSecTZ);
            Clock_Scan(&baseUserTime_doub, baseUserTime, 1);
	 }            	 
      }

      integerStartUserTime = baseUserTime_doub + (currentDay * timeInterval);
   }
   
   /* Loop over all the Wind Speed values. Format them if the product is of
    * type DWMLgen. Collect them in the maxWindSpeed array if the product is
    * of type DWMLgenByDay. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WS)
      {
         if (f_XML == 1 || f_XML == 2)  /* DWMLgen products. */
         {
            /* If the data is missing, so indicate in the XML (nil=true). */

            if (match[i].value[pnt].valueType == 2)
            {
               value = xmlNewChild(wind_speed, NULL, BAD_CAST "value", NULL);
               xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
            }
            else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                           * data. */
            {
               roundedWindSpeedData = (int)myRound(match[i].value[pnt].data, 0);
               sprintf(strBuff, "%d", roundedWindSpeedData);
               xmlNewChild(wind_speed, NULL, BAD_CAST "value", BAD_CAST
                           strBuff);
            }
         }
         else if ((f_XML == 3 || f_XML == 4) && (currentDay < numOutputLines))
		                            /* We don't format any wind for
                                             * these products, but simply get
                                             * Max values for wind speed and
                                             * wind dir  per summarization 
					     * period for use in icon
                                             * determination. */
         {

            /* Loop over each day if the requested format is 24 hourly, or
             * each 12 hour period if the requested format is 12 hourly. We
             * use a time interval in which the number of seconds in either a
             * 24 or 12 hour period determines the integer start time. 
	     */
            WSintegerTime = 0;
            WSstr[0] = '\0';

            formatValidTime(match[i].validTime, WSstr, 30, TZoffset,
                            f_observeDST);

            Clock_Scan(&WSdoubleTime, WSstr, 0);
            WSintegerTime = WSdoubleTime;

            /* Now we have to account for data that is just in the time
             * period i.e. if data is valid from 4PM - 7PM and time period is
             * from 6AM - 6PM. We shift data by one half the data's period in
             * seconds. 
	     */
            if (i - priorElemCount < 1)
               period = determinePeriodLength(match[i].validTime,
                                              match[i + 1].validTime, numRows,
                                              parameterName);
            else
               period = determinePeriodLength(match[i - 1].validTime,
                                              match[i].validTime,
                                              numRows, parameterName);
            WSintegerTime = WSintegerTime - (((double)period * 0.5) * 3600);

            /* Determine if this time is within the current day being
             * processed. */
            if ((integerStartUserTime <= WSintegerTime) &&
                (WSintegerTime < (integerStartUserTime + timeInterval)))
            {
               /* We need the max windspeed for weather phrase/icon
                * determination later on. Also, collect the valid times in
                * which the max wind speed values fell.  These will be used
                * to get the corresponding wind direction values for these
                * times. */

               if (match[i].value[pnt].valueType == 0)
               {
                  roundedWindSpeedData =
                        (int)myRound(match[i].value[pnt].data, 0);

                  if (roundedWindSpeedData > maxWindSpeed[currentDay])
                  {
                     maxWindSpeed[currentDay] = roundedWindSpeedData;
                     valTimeForWindDirMatch[currentDay] = match[i].validTime;
                  }
               }
            }
            forecastPeriod = ((WSintegerTime - integerStartUserTime) / 3600);

            if (f_XML == 3 && forecastPeriod + period >= 12)
            {
               currentDay++;
               integerStartUserTime = baseUserTime_doub + (currentDay *
                                                           timeInterval);
               forecastPeriod = 0;
            }
            else if (f_XML == 4 && forecastPeriod + period >= 24)
            {
               currentDay++;
               integerStartUserTime = baseUserTime_doub + (currentDay *
                                                           timeInterval);
               forecastPeriod = 0;
            }
         }
      }
      else
         priorElemCount++;
   }
   return;
}

/******************************************************************************
 * genWindDirectionValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Wind Direction element in the "time-series" DWMLgen 
 *  product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Wind Directions to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 * maxWindDirection = Array containing the wind direction values 
 *                    corresponding to a day (24 hour format) or 12 hour period
 *                    (12 hour format). These are not "max" wind direction 
 *                    values, but correspond to the time when the max. wind 
 *                    speed values were found per forecast period.  These values
 *                    are used in deriving the weather and/or icon values. 
 *                    (Output)
 * valTimeForWindDirMatch = Array with the validTimes that corresponds to the 
 *                          times when the max wind speeds are the highest.  We 
 *                          then collect the wind directions that correspond
 *                          to the same times when the wind speeds are the 
 *			    highest. (Input)
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
static void genWindDirectionValues(size_t pnt, char *layoutKey,
                                   genMatchType * match, size_t numMatch,
                                   xmlNodePtr parameters, int *maxWindDirection,
                                   uChar f_XML, int numOutputLines,
				   double *valTimeForWindDirMatch)
{

   int i;
   int priorElemCount = 0;
   int currentDay = 0;        /* Subset iteration of just wind directions in
                               * the match structure. */
   int roundedWindDirectionData;  /* Returned rounded data. */
   xmlNodePtr direction = NULL; /* Xml Node Pointer for <direction> element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* If the product is of type DWMLgen, format the <wind_direction> element. 
    */
   if (f_XML == 1 || f_XML == 2)
   {
      direction = xmlNewChild(parameters, NULL, BAD_CAST "direction", NULL);
      xmlNewProp(direction, BAD_CAST "type", BAD_CAST "wind");
      xmlNewProp(direction, BAD_CAST "units", BAD_CAST "degrees true");
      xmlNewProp(direction, BAD_CAST "time-layout", BAD_CAST layoutKey);

      /* Format the display name. */

      xmlNewChild(direction, NULL, BAD_CAST "name", BAD_CAST "Wind Direction");
   }

   /* Loop over all the Wind Direction data values. Format them if the
    * product is of type DWMLgen. Collect them in the maxWindDirection array
    * if the product is of type DWMLgenByDay. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WD)
      {
         if (f_XML == 1 || f_XML == 2)  /* DWMLgen products. */
         {
            /* If the data is missing, so indicate in the XML (nil=true). */

            if (match[i].value[pnt].valueType == 2)
            {
               value = xmlNewChild(direction, NULL, BAD_CAST "value", NULL);
               xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
            }
            else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                           * data. */
            {
               roundedWindDirectionData =
                     (int)myRound(match[i].value[pnt].data, 0);
               sprintf(strBuff, "%d", roundedWindDirectionData);
               xmlNewChild(direction, NULL, BAD_CAST "value", BAD_CAST strBuff);
            }
         }
         else if ((f_XML == 3 || f_XML == 4) && (currentDay < numOutputLines))
		                            /* We don't format any wind for
                                             * these products, but simply get
                                             * Max. values for wind speed and
                                             * the wind dir that occured when
					     * the max wind speed was, for use 
					     * in icon determination. */
         {
            if (valTimeForWindDirMatch[currentDay] == match[i].validTime)
            {
               maxWindDirection[currentDay] =
                     (int)myRound(match[i].value[pnt].data, 0);
               currentDay++;
            }
         }
      }
      else
         priorElemCount++;
   }
   return;
}

/******************************************************************************
 * genSkyCoverValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Sky Cover element in the "time-series" and "glance" 
 *  DWMLgen products.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Sky Cover values to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 * averageSkyCover = Array containing the average Sky Cover values corresponding
 *                   to a day (24 hour format) or 12 hour period (12 hour
 *                   format).  These values are used in deriving the weather 
 *		     and/or icon values. (Output)
 *  maxSkyCover = Array containing the maximum Sky Cover values corresponding
 *                to a day (24 hour format) or 12 hour period (12 hour
 *                format).  These values are used in deriving the weather 
 *		  and/or icon values. (Output)
 * minSkyCover = Array containing the minimum Sky Cover values corresponding
 *                to a day (24 hour format) or 12 hour period (12 hour
 *                format).  These values are used in deriving the weather 
 *		  and/or icon values. (Output)
 *   maxSkyNum = The index where the max sky cover was found. Used to 
 *               determine sky cover trends (i.e. increasing clouds). (Output)
 *   minSkyNum = The index where the min sky cover was found. Used to 
 *               determine sky cover trends (i.e. increasing clouds). (Output)
 * startPositions = The index of where the current forecast period begins.  Used
 *                  to determine sky cover trends (i.e. increasing clouds) for 
 *                  DWMLgenByDay products. (Output)
 *   endPositions = The index of where the current forecast period ends.  Used
 *                  to determine sky cover trends (i.e. increasing clouds) for 
 *                  DWMLgenByDay products. (Output)
 *    integerTime = Number of seconds since 1970 to when the data is valid.
 *                  Allows the code to know if this data belongs in the current
 *                  period being processed. (Output)
 * integerStartUserTime = Number of seconds since 1970 to when the user 
 *                        established periods being. (Output)
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
static void genSkyCoverValues(size_t pnt, char *layoutKey, genMatchType * match,
                              size_t numMatch, xmlNodePtr parameters,
                              char *startDate, int *maxSkyCover,
                              int *minSkyCover, int *averageSkyCover,
                              int numOutputLines, int timeInterval,
                              sChar TZoffset, sChar f_observeDST,
                              uChar parameterName, uChar numRows, uChar f_XML, 
			      int *maxSkyNum, int *minSkyNum, 
			      int *startPositions, int *endPositions, 
			      int *integerStartUserTime, int *SKYintegerTime,
			      char *currentHour)
{

   int i;
   int numNils = 0;           /* Denotes diff between number of data rows and 
				 the number that need to be formatted for the 
				 DWMLgenByDay products. */
   int firstTime = 1; /* Boolean indicating that we have found the first data value 
                       * in the current period. */

   int period;
   int forecastPeriod = 0;
   int priorElemCount = 0;
   int currentDay = 0;        /* Subset iteration of just wind Spds in the
                               * match structure. */
   char userStart_year[6];
   char userStart_month[4];
   char userStart_day[3];
   sChar DST;
   char baseUserTime[30];
   char hourMinSecTZ[16]; /* String component holding "T06:00:00-{TZ}:00" part. */
   double baseUserTime_doub = 0.0;
   double SKYdoubleTime = 0.0;
   double totalSkyCover = 0.0;
   double numSkyCoverValues = 0.0;
   int roundedSkyCoverData;   /* Returned rounded data. */
   xmlNodePtr cloud_amount = NULL;  /* Xml Node Pointer for <cloud-amount>
                                     * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   char SKYstr[30];           /* Temporary string holding formatted time
                               * value of sky cover. */

   /* If the product is of type DWMLgen, format the <cloud_amount> element. */

   if (f_XML == 1 || f_XML == 2)
   {

      cloud_amount = xmlNewChild(parameters, NULL, BAD_CAST "cloud-amount",
                                 NULL);
      xmlNewProp(cloud_amount, BAD_CAST "type", BAD_CAST "total");
      xmlNewProp(cloud_amount, BAD_CAST "units", BAD_CAST "percent");
      xmlNewProp(cloud_amount, BAD_CAST "time-layout", BAD_CAST layoutKey);

      /* Format the display name. */

      xmlNewChild(cloud_amount, NULL, BAD_CAST "name", BAD_CAST
                  "Cloud Cover Amount");
   }

   /* If product type is of DWMLgenByDay, parse user supplied start time. */

   if (f_XML == 3 || f_XML == 4)
   {
      userStart_year[0] = startDate[0];
      userStart_year[1] = startDate[1];
      userStart_year[2] = startDate[2];
      userStart_year[3] = startDate[3];
      userStart_year[4] = '-';
      userStart_year[5] = '\0';

      userStart_month[0] = startDate[5];
      userStart_month[1] = startDate[6];
      userStart_month[2] = '-';
      userStart_month[3] = '\0';

      userStart_day[0] = startDate[8];
      userStart_day[1] = startDate[9];
      userStart_day[2] = '\0';

      strcpy(baseUserTime, userStart_year);
      strcat(baseUserTime, userStart_month);
      strcat(baseUserTime, userStart_day);
      
      if (TZoffset < 0)
         sprintf(hourMinSecTZ, "T06:00:00%02d:00", TZoffset);
      else
         sprintf(hourMinSecTZ, "T06:00:00-%02d:00", TZoffset);      
      
      strcat(baseUserTime, hourMinSecTZ);
      
      Clock_Scan(&baseUserTime_doub, baseUserTime, 0);

      /* Before continuing, see if this point observes day light savings time, 
       * and if it is currently in effect. 
       */ 
      if (f_observeDST)
      {
         if (Clock_IsDaylightSaving2(baseUserTime_doub, 0) == 1)
         {
            DST = TZoffset - 1;

            if (TZoffset < 0)
	       sprintf(hourMinSecTZ, "T06:00:00%02d:00", DST);
	    else
               sprintf(hourMinSecTZ, "T06:00:00-%02d:00", DST);

            strcat(baseUserTime, hourMinSecTZ);
            Clock_Scan(&baseUserTime_doub, baseUserTime, 1);
	 }            	 
      }

      *integerStartUserTime = baseUserTime_doub + (currentDay * timeInterval);
      
   }

   /* Loop over all the Sky Cover values. Format them if the product is of
    * type DWMLgen. Collect them in the maxSkyCover, minSkyCover, and
    * averageSkyCover arrays if the product is of type DWMLgenByDay. We also 
    * need to collect info such as the startPositions and endPositions of
    * the periods, and the positions where the max and min sky covers are
    * found in each period.
    */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_SKY)
      {
         if (f_XML == 1 || f_XML == 2)  /* DWMLgen products. */
         {
            /* If the data is missing, so indicate in the XML (nil=true). */

            if (match[i].value[pnt].valueType == 2)
            {
               value = xmlNewChild(cloud_amount, NULL, BAD_CAST "value", NULL);
               xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
            }
            else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                           * data. */
            {
               roundedSkyCoverData = (int)myRound(match[i].value[pnt].data, 0);
               sprintf(strBuff, "%d", roundedSkyCoverData);
               xmlNewChild(cloud_amount, NULL, BAD_CAST "value", BAD_CAST
                           strBuff);
            }
         }
         else if ((f_XML == 3 || f_XML == 4) && currentDay < numOutputLines) 
		                            /* We don't format any sky cover
                                             * for these products, but simply
                                             * collect Max, Min, and Avg values
                                             * for sky cover for use in icon
                                             * determination. */
         {

            /* Loop over each day if the requested format is 24 hourly, or
             * each 12 hour period if the requested format is 12 hourly. We
             * use a time interval in which the number of seconds in either a
             * 24 or 12 hour period determines the integer start time. */

            *SKYintegerTime = 0;
            SKYstr[0] = '\0';

            formatValidTime(match[i].validTime, SKYstr, 30, TZoffset,
                            f_observeDST);

            Clock_Scan(&SKYdoubleTime, SKYstr, 0);
            *SKYintegerTime = SKYdoubleTime;

            /* Now we have to account for data that is just in the time
             * period i.e. if data is valid from 4PM-7PM and time period is
             * from 6AM-6PM. We shift data by one half the data's period in
             * seconds. */

            if (i - priorElemCount < 1)
               period = determinePeriodLength(match[i].validTime,
                                              match[i + 1].validTime, numRows,
                                              parameterName);
            else
               period = determinePeriodLength(match[i - 1].validTime,
                                              match[i].validTime,
                                              numRows, parameterName);

            *SKYintegerTime = *SKYintegerTime - (((double)period * 0.5) * 3600);

            /* Determine if this time is within the current day (period) being
             * processed. */

            if ((*integerStartUserTime <= *SKYintegerTime)
                && (*SKYintegerTime < *integerStartUserTime + timeInterval))
            {
	       
               /* We need the max, min, and average sky covers for each
                * forecast period (12 or 24 hours) for weather phrase/icon
                * determination later on. 
		*/
               if (match[i].value[pnt].valueType == 0)
               {
                  roundedSkyCoverData =
                        (int)myRound(match[i].value[pnt].data, 0);

                  /* Get maxSkyCover values and record it's index. */

                  if (roundedSkyCoverData > maxSkyCover[currentDay])
	          {
                     maxSkyCover[currentDay] = roundedSkyCoverData;
	             maxSkyNum[currentDay] = i - priorElemCount;
		  }

                  /* Get minSkyCover values and record it's index. */

                  if (roundedSkyCoverData < minSkyCover[currentDay])
	          {
                     minSkyCover[currentDay] = roundedSkyCoverData;
	             minSkyNum[currentDay] = i - priorElemCount;
	          }
		  
                  /* The cloud phrasing algorithm in skyPhrase() routine needs
		   * to know the start index of each day. So, capture that 
		   * information here. 
		   */
                  if (firstTime)
	          {
	             startPositions[currentDay] = i - priorElemCount;
		     firstTime = 0;
		  }
		  
                  /* Gather data for calculating averageSkyCover values. */
                  totalSkyCover += roundedSkyCoverData;
                  numSkyCoverValues++;
	       }
            }
	    
            forecastPeriod = ((*SKYintegerTime - *integerStartUserTime) / 3600);
    
            /* Calculate the average sky cover for use in phrase algorithm. First,
             * check to see if we need to jump into the next forecast period. 
             */
            if (f_XML == 3)
            {
	       if ((forecastPeriod + period >= 12) ||  
		  (i - priorElemCount == numRows - 1)) /* Accounts for last data row. */
               {
                  if (numSkyCoverValues > 0)
                     averageSkyCover[currentDay] = (int)myRound((totalSkyCover / numSkyCoverValues), 0);
	          else
		     averageSkyCover[currentDay] = 0;

                  totalSkyCover = 0;
                  numSkyCoverValues = 0;
	          endPositions[currentDay] = i - priorElemCount;
	          firstTime = 1;
                  currentDay++;
                  *integerStartUserTime = baseUserTime_doub + (currentDay *
                                                              timeInterval);
                  forecastPeriod = 0;
	       }
            }
            else if (f_XML == 4)
	    {
               if ((forecastPeriod + period >= 24) || 
	          (i - priorElemCount == numRows - 1)) /* Accounts for last data row. */
               {
                  if (numSkyCoverValues > 0)
                     averageSkyCover[currentDay] = (int)myRound((totalSkyCover / numSkyCoverValues), 0);
	          else
		     averageSkyCover[currentDay] = 0;

	          totalSkyCover = 0;
                  numSkyCoverValues = 0;
	          endPositions[currentDay] = i - priorElemCount;
	          firstTime = 1;
                  currentDay++;
                  *integerStartUserTime = baseUserTime_doub + (currentDay *
                                                              timeInterval);
                  forecastPeriod = 0;
	       }
            }
         }
      }
      else
         priorElemCount++;
   }
   
   /* In certain cases for the DWMLgenByDay products, check to make sure any left over
    * periods contain bogus data for the averageSkyCover, start and end positions, and 
    * the min and max sky num arrays.
    */
   if (f_XML == 3 || f_XML == 4)  /* DWMLgenByDay products. */
   {
      /* Tally up the number of iterations that occurred thru the match 
       * structure and compare to the number of actual data rows to see if there
       * is a difference.
       */
      numNils = numOutputLines - currentDay;
      if (numNils > 0)
      {
         for (i = currentDay; i < numOutputLines; i++)
	 {
            averageSkyCover[currentDay] = 0;
	 }
      }
   }
   
   return;
}

/******************************************************************************
 * genRelHumidityValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Relative Humidity element in the "time-series" 
 *  DWMLgen product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Relative Humidities to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
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
static void genRelHumidityValues(size_t pnt, char *layoutKey,
                                 genMatchType * match, size_t numMatch,
                                 xmlNodePtr parameters)
{
   int i;
   int roundedRelHumidityData;  /* Returned rounded data. */
   xmlNodePtr humidity = NULL;  /* Xml Node Pointer for <humidity> element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <humidity> element. */

   humidity = xmlNewChild(parameters, NULL, BAD_CAST "humidity", NULL);
   xmlNewProp(humidity, BAD_CAST "type", BAD_CAST "relative");
   xmlNewProp(humidity, BAD_CAST "units", BAD_CAST "percent");
   xmlNewProp(humidity, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display name. */

   xmlNewChild(humidity, NULL, BAD_CAST "name", BAD_CAST "Relative Humidity");

   /* Loop over all the data values and format them. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_RH)
      {

         /* If the data is missing, so indicate in the XML (nil=true). */

         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(humidity, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedRelHumidityData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedRelHumidityData);
            xmlNewChild(humidity, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}

/******************************************************************************
 * genWaveHeightValues() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the Wave Height element in the "time-series" 
 *  DWMLgen product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Wave Heights to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
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
static void genWaveHeightValues(size_t pnt, char *layoutKey,
                                genMatchType * match, size_t numMatch,
                                xmlNodePtr parameters)
{
   int i;
   int roundedWaveHeightData; /* Returned rounded data. */
   xmlNodePtr water_state = NULL; /* Xml Node Pointer for <water-state>
                                   * element. */
   xmlNodePtr waves = NULL;   /* Xml Node Pointer for <waves> element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <water-state> element. */

   water_state = xmlNewChild(parameters, NULL, BAD_CAST "water-state", NULL);
   xmlNewProp(water_state, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format <waves> element name. */

   waves = xmlNewChild(water_state, NULL, BAD_CAST "waves", NULL);
   xmlNewProp(waves, BAD_CAST "type", BAD_CAST "significant");
   xmlNewProp(waves, BAD_CAST "units", BAD_CAST "feet");

   /* Format the display name. */

   xmlNewChild(waves, NULL, BAD_CAST "name", BAD_CAST "Wave Height");
   /* Loop over all the data values and format them. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WH)
      {

         /* If the data is missing, so indicate in the XML (nil=true). */

         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(waves, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedWaveHeightData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedWaveHeightData);
            xmlNewChild(waves, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}

/*****************************************************************************
 * checkNeedForPeriodName() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code checks to see if a user has requested data with start and end
 *  times that warrent a special period name (TODAY, TONIGHT, etc.). If so, the
 *  period name is determined.
 *
 * ARGUMENTS
 *            index = Index of the startTimes and endTimes arrays. (Input)
 *   numPeriodNames = Number period names for one of the seven issuance times.
 *                    (Input)
 *   timeZoneOffset = Hours to add to local time to get to UTC time. (Input)
 *    parameterName = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *   parsedDataTime = String representation of the data's current startTime 
 *                    being analyzed (ex. 2005-03-30T00:00:00-05:00). (Input)
 * outputPeriodName = A logical (1 = true, 0 = false) used to indicate if a 
 *                    period name is to be used for this row once user selected
 *                    times are taken into account. (Input)
 *     issuanceType = Index (dimension #1) into the period name array that 
 *                    defines the current cycle (ex. morning, afternoon, 
 *                    earlyMorningMinT, etc). (Input)
 *       periodName = Special name used near beginning of time layouts. (Output)
 *       currentDay = Current day's 2 digit date. Used to determine if Max and 
 *                    Min temps should be called "today"/"overnight" or POP 
 *                    value should  be called "overnight". (Input)
 *      currentHour = Today's hour. Used to determine if POP value should be 
 *                    called "overnight" or "later today". (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void checkNeedForPeriodName(int index, uChar * numPeriodNames,
                                   sChar timeZoneOffset, uChar parameterName,
                                   char *parsedDataTime,
                                   uChar * outputPeriodName, uChar issuanceType,
                                   char *periodName, char *currentHour,
                                   char *currentDay, double startTime_cml,
                                   double currentDoubTime,
                                   double firstValidTime, int period)
{

   static char TDay[3];
   uChar whichPeriodName = 3; /* An index (dimension #2 of periodData array)
                               * indicating which period name is to be used
                               * for the row indicated by the index. */
   char *periodData[MAX_PERIODS][4];  /* Array containing the period names
                                       * for each of the different cycles
                                       * (afternoon and morning) and period
                                       * lengths (12 and 24 hours). */
   int numPeriodsClippedBegin = 0;
   int numHoursClippedBegin = 0;
   double startTime_doub = 0.0;

   /* Initialize periodData pointer. */

   periodData[0][0] = "Overnight";
   periodData[0][1] = "Later Today";
   periodData[0][2] = NULL;
   periodData[0][3] = NULL;

   periodData[1][0] = "Today";
   periodData[1][1] = "Tonight";
   periodData[1][2] = "Tomorrow";
   periodData[1][3] = "Tomorrow Night";

   periodData[2][0] = "Tonight";
   periodData[2][1] = "Tomorrow";
   periodData[2][2] = "Tomorrow Night";
   periodData[2][3] = NULL;

   periodData[3][0] = "Later Today";
   periodData[3][1] = NULL;
   periodData[3][2] = NULL;
   periodData[3][3] = NULL;

   periodData[4][0] = "Overnight";
   periodData[4][1] = NULL;
   periodData[4][2] = NULL;
   periodData[4][3] = NULL;

   periodData[5][0] = "Today";
   periodData[5][1] = "Tomorrow";
   periodData[5][2] = NULL;
   periodData[5][3] = NULL;

   periodData[6][0] = "Tonight";
   periodData[6][1] = "Tomorrow Night";
   periodData[6][2] = NULL;
   periodData[6][3] = NULL;

   if (index == 0)
   {
      TDay[0] = parsedDataTime[8];
      TDay[1] = parsedDataTime[9];
      TDay[2] = '\0';
   }

   /* Calculate how many periods were skipped at beginning. */
   Clock_Scan(&startTime_doub, parsedDataTime, 1);
   numHoursClippedBegin = (startTime_doub - currentDoubTime) / 3600;

   /* Now we have to check and see if user supplied a shorter duration for
    * data formatting via the command line arguments startTime and endTime.
    * Then we need to see if period names are still applicable in this
    * shorter interval of time. */
   if (startTime_cml != 0.0 && numHoursClippedBegin >= 12)
   {
      /* We use "12" instead of the period name because MaxT and MinT have
       * periods of 24 hours, but forecast periods of 12 hours. */
      numPeriodsClippedBegin = numHoursClippedBegin / 12;
      if (numPeriodsClippedBegin >= *numPeriodNames)
      {
         *outputPeriodName = 0;
         return;
      }
      else
      {
         *outputPeriodName = 1; /* Tell user they need to use a period name. */
         whichPeriodName = *numPeriodNames - numPeriodsClippedBegin;

         /* Now that we have issuanceType, and whichPeriodName, retrieve the
          * periodName. */
         if (periodData[issuanceType][whichPeriodName] != NULL)
            strcpy(periodName, periodData[issuanceType][whichPeriodName]);
         else
            *outputPeriodName = 0;
         return;
      }
   }
   else
   {
      /* Late in the day, the Max Temp value will be in the tomorrow period
       * so we need to detect that condition and return outputPeriodName =
       * false. */

      if (parameterName == NDFD_MAX)
      {

         /* If the max temp day is not the same as the today's day, then we
          * don't need to label it using "today".  This happens in the
          * evening after about 8:00 PM. */

         if (strcmp(currentDay, TDay) != 0 && index + 1 < *numPeriodNames)
         {
            *outputPeriodName = 1;  /* Tell user they need to use a period
                                     * name. */
            whichPeriodName = index + 1;
         }
         else if (strcmp(currentDay, TDay) == 0 && index < *numPeriodNames)
         {
            *outputPeriodName = 1;  /* Tell user they need to use a period
                                     * name. */
            whichPeriodName = index;
         }
         else if (strcmp(currentDay, TDay) != 0 && index >= *numPeriodNames)
         {
            *outputPeriodName = 0;  /* Tell user they don't use a period
                                     * name. */
            return;
         }
      }

      /* The Min Temp value will be in the tomorrow period so we need to
       * detect that condition and return outputPeriodName = false. */

      else if (parameterName == NDFD_MIN)
      {

         /* If the min temp day is not the same as today's day, then we don't 
          * need to label it using "today".  This happens in the evening
          * after about 8:00 PM. 
	  */

         if (issuanceType == earlyMorningMinT && index == 0)
         {
            *outputPeriodName = 1;  /* Tell user they need to use a period
                                     * name. */
            whichPeriodName = index;
         }
         if (issuanceType == afternoon24)
         {
            if (strcmp(currentDay, TDay) == 0 && index < *numPeriodNames)
            {
               *outputPeriodName = 1; /* Tell user they need to use period
                                       * name. */
               whichPeriodName = index;
            }
            else if (strcmp(currentDay, TDay) == 0 && index != 0 && index + 1 <
                     *numPeriodNames)
            {
               *outputPeriodName = 1; /* Tell user they need to use period
                                       * name. */
               whichPeriodName = index + 1;
            }
            else if (strcmp(currentDay, TDay) != 0 && index < *numPeriodNames)
            {
               *outputPeriodName = 1; /* Tell user they need to use period
                                       * name. */
               whichPeriodName = index + 1;
            }
            else if (strcmp(currentDay, TDay) != 0 && index >= *numPeriodNames)
            {
               *outputPeriodName = 0; /* Tell user they don't need to use a
                                       * period name. */
               return;
            }
         }
      }

      /* Early in the morning, the PoP12 data will be updated and no longer
       * be the OVERNIGHT period.  So we need to detect that condition and
       * return the next period name (LATER TODAY). This will only be
       * applicable for DWMLgenByDay products as the "glance" product of
       * DWMLgen does not output POP. */

      else if (parameterName == NDFD_POP)
      {
         /* If the POP day is not the same as the today's day, then we don't
          * need to label it using "today".  This happens in the evening
          * after about 8:00 PM. */

         if (strcmp(currentDay, TDay) == 0 && atoi(currentHour)
             <= 20 && index < *numPeriodNames)
         {
            *outputPeriodName = 1;  /* Tell user they need to use a period
                                     * name. */
            whichPeriodName = index;
         }
         else if (strcmp(currentDay, TDay) == 0 && atoi(currentHour) <= 20 &&
                  index >= *numPeriodNames)
         {
            *outputPeriodName = 0;  /* Tell user they don't need to use a
                                     * period name. */
            return;
         }
      }
   }

   /* Now that we have issuanceType, and whichPeriodName, retrieve the
    * periodName. */
   if (*outputPeriodName != 0
       && periodData[issuanceType][whichPeriodName] != NULL)
      strcpy(periodName, periodData[issuanceType][whichPeriodName]);

   return;
}

/*****************************************************************************
 * getPeriodInfo() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Retreives the "issuanceType" and "numPeriodNames" for those elements in 
 *  which period names are used. 
 *
 * ARGUMENTS
 *   firstValidTime = First valid time of element. (Input)
 *   numPeriodNames = Number period names for one of the seven issuance times.
 *                    (Input)
 *    parameterName = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *     issuanceType = Index (dimension #1) into the period name array that 
 *                    defines the current cycle (ex. morning, afternoon, 
 *                    earlyMorningMinT, etc). (Input)
 *           period = Length between an elements successive validTimes (Input).
 *        frequency = Set to "boggus" for DWMLgen products, and to "12 hourly" 
 *                    or "24 hourly" for the DWMLgenByDay products.  
 *       currentDay = Current day's 2 digit date. (Input)
 *      currentHour = Current 2-digit hour. (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void getPeriodInfo(uChar parameterName, char *firstValidTime,
                          char *currentHour, char *currentDay,
                          uChar * issuanceType, uChar * numPeriodNames,
                          int period, char *frequency)
{

   /* The period names vary by cycle, so we need to define when each cycle
    * begins */

   static int startAfternoon = 12;  /* Threshold indicating the beginning of
                                     * the evening cycle. */
   static int startMorning = 6; /* Threshold indicating the beginning of
                                 * morning cycle. */
   char firstValidHour[3];    /* Hour of element's first ValidTime. */
   char firstValidDay[3];     /* Day of element's first ValidTime. */

   /* Now we determine when the data begins. */

   firstValidHour[0] = firstValidTime[11];
   firstValidHour[1] = firstValidTime[12];
   firstValidHour[2] = '\0';

   firstValidDay[0] = firstValidTime[8];
   firstValidDay[1] = firstValidTime[9];
   firstValidDay[2] = '\0';

/* Determine if we are to use morning (ex. TODAY) or afternoon (ex. TONIGHT) 
   period names and set the index into the period name array accordingly. */

   if (period == 12)
   {
      if (atoi(firstValidHour) >= startMorning && atoi(firstValidHour) <
          startAfternoon)
         *issuanceType = morning12;
      else
         *issuanceType = afternoon12;
   }
   else if (period == 24)
   {
      if (atoi(firstValidHour) >= startMorning && atoi(firstValidHour) <
          startAfternoon)
         *issuanceType = morning24;
      else
         *issuanceType = afternoon24;
   }

/* In the wee hours of the morning, we need special names for the periods. 
   So we first determine which interface is calling us and then use the 6 AM 
   cutoff for their use.  For the DWMLgen() minT, we need to keep 
   using the early morning period names until the overnight data falls off.
   So we check which day the data is valid for (firstValidDay < todayDay).
   This section if for the DWMLgenByDay product only. */

   if (((strcmp(frequency, "12 hourly") == 0) ||
        (strcmp(frequency, "24 hourly") == 0)) &&
       (atoi(currentHour) < startMorning))
   {
      /* Determine which NDFD parameter we are processing and return TRUE if
       * it is valid for a period of time. */

      switch (parameterName)
      {

         case NDFD_MAX:

            *issuanceType = earlyMorningMaxT;
            break;

         case NDFD_MIN:

            *issuanceType = earlyMorningMinT;
            break;

         case NDFD_POP:

            *issuanceType = earlyMorning;
            break;
      }
   }
   else if (strcmp(frequency, "boggus") == 0)
   {
      /* Determine which NDFD parameter we are processing and return TRUE if
       * it is valid for a period of time. This section is for the DWMLgen
       * product only.  */

      switch (parameterName)
      {

         case NDFD_MAX:

            if (atoi(currentHour) < startMorning)
               *issuanceType = earlyMorningMaxT;
            break;

         case NDFD_MIN:

            if (strcmp(firstValidDay, currentDay) != 0)
               *issuanceType = earlyMorningMinT;
            break;
      }
   }
   /* Determine the Number of Period Names. */

   if (period == 24)
   {
      if (*issuanceType == earlyMorningMaxT || *issuanceType ==
          earlyMorningMinT)
         *numPeriodNames = 1;
      else if (*issuanceType == morning24 || *issuanceType == afternoon24)
         *numPeriodNames = 2;
   }
   if (period == 12)
   {
      if (*issuanceType == earlyMorning)
         *numPeriodNames = 2;
      else if (*issuanceType == morning12)
         *numPeriodNames = 4;
      else if (*issuanceType == afternoon12)
         *numPeriodNames = 3;
   }

   return;
}

/*****************************************************************************
 * useNightPeriodName() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Determines whether it is night time -- and to subsequently use night time
 *  period names.
 *  
 * ARGUMENTS
 *   dataTime = Current element's validTime. (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

int useNightPeriodName(char *dataTime)
{
   char hour[3];              /* 2 digit hour of current validTime. */
   int hr;                    /* 2 digit hour of current validTime in integer 
                               * form. */

   /* Lets parse the validTime information (2004-03-19T12:00:00-05:00). */

   hour[0] = dataTime[11];
   hour[1] = dataTime[12];
   hour[2] = '\0';

   hr = atoi(hour);

   /* Determine if the current hour is in the day */

   if ((hr >= 6) && (hr < 18))
      return 0;
   else                       /* it needs a nighttime label */
      return 1;
}

/*****************************************************************************
 * monthDayYearTime() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Code gets the current time and the first validTime for the MaxT element in 
 *  double form. However, it sets the hours, minutes, and seconds all to zeroes
 *  before the conversion. For example, if the current local time is 
 *  2006-04-30T19:34:22:-05:00, before converting to double form, set the 
 *  current Local time to 2006-04-30T00:00:00:-00:00. Do the same for the first
 *  validTime for the MaxT. This is done for a "same day" check.
 *
 * ARGUMENTS
 *            match = Pointer to the array of element matches from degrib. 
 *                    (Input) 
 *         numMatch = The number of matches from degrib. (Input)
 *         TZoffset = Number of hours to add to current time to get GMT time. 
 * currentLocalTime =  Current Local Time in "2006-04-29T19:34:22-05:00"         
 *                     format. (Input)
 *       currentDay = Current day's 2-digit date. (Input)
 * firstMaxTValidTime_doub_adj = The date of the first MaxT validTime with the 
 *                               hour, minutes, and seconds set to zeroes, 
 *                               translated to double form. 
 *   currentLocalTime_doub_adj = The current date with the hours, minutes, and 
 *                               seconds set to zeroes, translated to double
 *                               form.
 *                             
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void monthDayYearTime(genMatchType * match, size_t numMatch,
                             char *currentLocalTime, char *currentDay,
                             sChar f_observeDST, 
			     double *firstMaxTValidTime_doub_adj,
                             double *currentLocalTime_doub_adj, sChar TZoffset)
{

   int i;
   static char firstMaxTValidTime_char[30]; /* First MaxT's validTime. */
   char maxTYear[6];          /* Year of first MaxT's validTime ("2006-"). */
   char maxTMonth[4];         /* Month of first MaxT's validTime ("04-"). */
   char maxTDay[3];           /* Day of first MaxT's validTime ("30"). */
   char firstMaxTValidTime_char_adj[30];  /* First MaxT's validTime, with
                                           * hours, minutes, and seconds all
                                           * set to zeros. */
   char time_adj[16]; /* String component holding "T00:00:00-00:00" part. */
   char currentYear[6];       /* Year of current Local Time ("2006-"). */
   char currentMonth[4];      /* Month of current Local Time ("04-"). */
   char currentLocalTime_char_adj[30];  /* Current Local Time with hours,
                                         * minutes, and seconds all set to
                                         * zeros. */

   /* Get the first MaxT valid time in character form and set everything but
    * the year, month, and day to = 0. */

   for (i = 0; i < numMatch; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_MAX) /* Set first MaxT validtime. */
      {
         formatValidTime(match[i].validTime, firstMaxTValidTime_char, 30, 
			 TZoffset, f_observeDST);
         break;
      }
   }

   /* Build the first MaxT's validTime, adjusted with hours, minutes, and
    * seconds all set to zeroes. */

   maxTYear[0] = firstMaxTValidTime_char[0];
   maxTYear[1] = firstMaxTValidTime_char[1];
   maxTYear[2] = firstMaxTValidTime_char[2];
   maxTYear[3] = firstMaxTValidTime_char[3];
   maxTYear[4] = '-';
   maxTYear[5] = '\0';

   maxTMonth[0] = firstMaxTValidTime_char[5];
   maxTMonth[1] = firstMaxTValidTime_char[6];
   maxTMonth[2] = '-';
   maxTMonth[3] = '\0';

   maxTDay[0] = firstMaxTValidTime_char[8];
   maxTDay[1] = firstMaxTValidTime_char[9];
   maxTDay[2] = '\0';

   strcpy(firstMaxTValidTime_char_adj, maxTYear);
   strcat(firstMaxTValidTime_char_adj, maxTMonth);
   strcat(firstMaxTValidTime_char_adj, maxTDay);

   if (TZoffset < 0)
      sprintf(time_adj, "T00:00:00%02d:00", TZoffset);
   else
      sprintf(time_adj, "T00:00:00-%02d:00", TZoffset);
   
   strcat(firstMaxTValidTime_char_adj, time_adj);

   /* Get the adjusted MaxT validTime in double form. */
   Clock_Scan(firstMaxTValidTime_doub_adj, firstMaxTValidTime_char_adj, 1);

   /* B`uild the current Local time, adjusted with hours, minutes, and seconds 
    * all set to zeroes. */

   currentYear[0] = currentLocalTime[0];
   currentYear[1] = currentLocalTime[1];
   currentYear[2] = currentLocalTime[2];
   currentYear[3] = currentLocalTime[3];
   currentYear[4] = '-';
   currentYear[5] = '\0';

   currentMonth[0] = currentLocalTime[5];
   currentMonth[1] = currentLocalTime[6];
   currentMonth[2] = '-';
   currentMonth[3] = '\0';

   strcpy(currentLocalTime_char_adj, currentYear);
   strcat(currentLocalTime_char_adj, currentMonth);
   strcat(currentLocalTime_char_adj, currentDay);
   
   if (TZoffset < 0)
      sprintf(time_adj, "T00:00:00%02d:00", TZoffset);
   else
      sprintf(time_adj, "T00:00:00-%02d:00", TZoffset);
   
   strcat(currentLocalTime_char_adj, time_adj);

   /* Get the adjusted current LocalTime in double form. */

   Clock_Scan(currentLocalTime_doub_adj, currentLocalTime_char_adj, 1);

   return;
}

/*****************************************************************************
 * checkNeedForEndTime() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Determines whether or not an element uses an end time <end-valid-time> tag 
 *  in the formatted output XML.
 *  
 * ARGUMENTS
 *   parameterName = Number denoting the NDFD element currently processed. 
 *                   (Input) 
 *  
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static int checkNeedForEndTime(uChar parameterName)
{
   /* Determine which NDFD parameter we are processing and return TRUE if it
    * is valid for a period of time. */

   switch (parameterName)
   {

      case NDFD_MAX:
      case NDFD_MIN:
      case NDFD_POP:
      case NDFD_SNOW:
      case NDFD_QPF:

         return 1;
         break;

      default:
         /* Its a snapshot parameter so no <end-valid-time> tag. */
         return 0;
   }
}

/*****************************************************************************
 * isNewLayout() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Determines whether a new time layout is needed. If so, numCurrenLayouts is
 *  incremented.
 *    
 * ARGUMENTS
 *         newLayout = Contains the current elements layout info (period, 
 *                     numRows, first startTime), in order to determine if this
 *                     is new information or a previous time layout with this 
 *                     information has already been formatted. (Input)
 *    numLayoutSoFar = The total number of time layouts that have been created 
 *                     so far. (Input)
 *  numCurrentLayout = Number of the layout we are currently processing. (Input)
 * f_finalTimeLayout = Flag denoting if this is the last time layout being 
 *                     processed. Used to free up the static 
 *                     timeLayoutDefinitions array. (Input)
 *                  
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static int isNewLayout(layouts newLayout, size_t * numLayoutSoFar,
                       uChar * numCurrentLayout, int f_finalTimeLayout)
{

   int i;
   static layouts *timeLayoutDefinitions; /* An array holding all the past
                                           * time layouts.  They should all
                                           * be unique. */
   /* Check to see if the static array can be freed (the final time layout
    * has been formatted). */
   if (!f_finalTimeLayout)
   {
      /* If this is the first time layout, we just simply add it to array
       * timeLayoutDefintions and return indicating a new time layout. */

      if (*numLayoutSoFar == 1)
      {
         timeLayoutDefinitions = calloc(1, sizeof(layouts));

         timeLayoutDefinitions[0].period = newLayout.period;
         timeLayoutDefinitions[0].numRows = newLayout.numRows;
         strcpy(timeLayoutDefinitions[0].fmtdStartTime,
                newLayout.fmtdStartTime);
         *numCurrentLayout = 1;
      }

      /* If not the first one, check existing layouts for one that
       * corresponds to this data.  If we find one, let the calling program
       * know that a time layout already exists and which one it is. */

      for (i = 0; i < *numLayoutSoFar - 1; i++)
      {
         if ((timeLayoutDefinitions[i].period == newLayout.period) &&
             (timeLayoutDefinitions[i].numRows == newLayout.numRows) &&
             (strcmp(timeLayoutDefinitions[i].fmtdStartTime,
                     newLayout.fmtdStartTime) == 0))
         {
            *numCurrentLayout = i + 1;
            return 0;
         }
      }

      /* Since we didn't find a pre-existing time layout, we create a new one
       * and let calling routine know that we created a new layout. */

      timeLayoutDefinitions = realloc(timeLayoutDefinitions, (*numLayoutSoFar)
                                      * sizeof(layouts));

      timeLayoutDefinitions[*numLayoutSoFar - 1].period = newLayout.period;
      timeLayoutDefinitions[*numLayoutSoFar - 1].numRows = newLayout.numRows;
      strcpy(timeLayoutDefinitions[*numLayoutSoFar - 1].fmtdStartTime,
             newLayout.fmtdStartTime);
      *numCurrentLayout = *numLayoutSoFar;

      return 1;
   }
   else
   {
      free(timeLayoutDefinitions);
      return 0;
   }
}

/*****************************************************************************
 * generateTimeLayout () -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This routine creates the XML time layout for NDFD parameters.
 *              There is one layout for each unique combination of data start
 *              time, period length, and number of data values.  The maximum
 *              temperature time layout would look like the following:
 *
 *      <time-layout time-coordinate="local" summarization="none">
 *          <layout-key>k-p24h-n7-1</layout-key>
 *              <start-valid-time>2004-04-12T08:00:00-04:00</start-valid-time>
 *                  <end-valid-time>2004-04-12T20:00:00-04:00</end-valid-time>
 *              <start-valid-time>2004-04-13T08:00:00-04:00</start-valid-time>
 *                  <end-valid-time>2004-04-13T20:00:00-04:00</end-valid-time>
 *              <start-valid-time>2004-04-14T08:00:00-04:00</start-valid-time>
 *                  <end-valid-time>2004-04-14T20:00:00-04:00</end-valid-time>
 *              <start-valid-time>2004-04-15T08:00:00-04:00</start-valid-time>
 *                  <end-valid-time>2004-04-15T20:00:00-04:00</end-valid-time>
 *              <start-valid-time>2004-04-16T08:00:00-04:00</start-valid-time>
 *                  <end-valid-time>2004-04-16T20:00:00-04:00</end-valid-time>
 *              <start-valid-time>2004-04-17T08:00:00-04:00</start-valid-time>
 *                  <end-valid-time>2004-04-17T20:00:00-04:00</end-valid-time>
 *             <start-valid-time>2004-04-18T08:00:00-04:00</start-valid-time>
 *                  <end-valid-time>2004-04-18T20:00:00-04:00</end-valid-time>
 *      </time-layout>              
 *
 * ARGUMENTS
 *    parameterName = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *          numRows = Number of data rows (values) for an element. (Input)
 *        layoutKey = The key to the time layout is of the form
 *                    k-p{periodLength}h-n{numRows}-{numLayouts}    
 *                    The "k" is for "key".  The "p" is for "period" "h" is for
 *                    "hour" and "n" is for "number" (example:  k-p12h-n6-1).  
 *                    The key is used to relate the layout in the <time-layout>  
 *                    element to the time-layout attribute in the NDFD element 
 *                    element <temperature key-layout="k-p12h-n6-1"> 
 *                    (Input / Output) 
 *   timeCoordinate = The time coordinate that the user wants the time 
 *                    information communicated it.  Currently only local time is 
 *                    supported. (Input)
 *    summarization = The type of temporal summarization being used.
 *                    Currently, no summarization is done in time.
 * f_formatPeriodName = Flag to indicate if period names (i.e. "today") appear 
 *                      in the start valid time tag: 
 *                      <start-valid-time period-name="today"> (Input)
 *   numLayoutSoFar = The total number of time layouts that have been created 
 *                    so far. (Input)
 * numCurrentLayout = Number of the layout we are currently processing. (Input)
 *         TZoffset = Number of hours to add to current time to get GMT time. 
 *                    (Input)
 *     f_observeDST = Flag determining if current point observes Daylight 
 *                    Savings Time. (Input)  
 *       currentDay = Current day's 2 digit date. (Input)
 *      currentHour = Current hour =in 2 digit form. (Input)
 *        frequency = Set to "boggus" for DWMLgen products, and to "12 hourly" 
 *                    or "24 hourly" for the DWMLgenByDay products. (Input)  
 *            match = Pointer to the array of element matches from degrib. 
 *                    (Input) 
 *         numMatch = The number of matches from degrib. (Input)
 *      useEndTimes = Flag denoting if element uses end times in the output XML 
 *             data = An xml Node Pointer denoting the <data> node. (Input)
 *      numFmtdRows = For DWMLgenByDay products, the number of rows formatted 
 *                    is set and not based off of the Match structure. (Input)
 *  
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void generateTimeLayout(uChar numRows, uChar parameterName,
                               char *layoutKey, const char *timeCoordinate,
                               char *summarization, genMatchType * match,
                               size_t numMatch, uChar f_formatPeriodName,
                               sChar TZoffset, sChar f_observeDST,
                               size_t * numLayoutSoFar,
                               uChar * numCurrentLayout, char *currentHour,
                               char *currentDay, char *frequency,
                               xmlNodePtr data, double startTime_cml,
                               double currentDoubTime, int *numFmtdRows,
			       uChar f_XML, int numPopRowsSkippedBeginning)
{

   int i;
   int f_finalTimeLayout = 0; /* Flag denoting if this is the last time
                               * layout being processed. */
   int elemCount = 0;         /* Counter used in determining first validTime. 
                               */
   int period = 0;            /* Length between an elements successive
                               * validTimes. */
   int numPopRowsSkippedTemp = 0;
   double startTime_doub = 0.0; /* Holds startTimes as a double. */
   double firstValidTime = 0.0; /* The validTime of the first match for the
                                 * element being processed. */
   double secondValidTime = 0.0;  /* The validTime of the second match for
                                   * the element being processed. */
   uChar useEndTimes = 0;     /* Flag denoting if element uses end times in
                               * the output XML. */
   char **startTimes = NULL;  /* Character array holding all the start Times. 
                               */
   char **endTimes = NULL;    /* Character array holding all the end Times. */
   xmlNodePtr time_layout = NULL; /* An xml Node Pointer denoting the
                                   * <time-layout> node. */
   xmlNodePtr layout_key = NULL;  /* An xml Node Pointer denoting the
                                   * <layout-key> node. */
   xmlNodePtr startValTime = NULL;  /* An xml Node Pointer denoting the
                                     * <start-valid-time> node. */
   xmlNodePtr endValTime = NULL;  /* An xml Node Pointer denoting the
                                   * <end-valid-time> node. */
   char dayName[40];          /* Contains name of day of week for use in
                               * period names. */
   uChar outputPeriodName;    /* Flag denoting whether element and time needs 
                               * a special period name output in XML. */
   uChar numPeriodNames = 0;  /* Number of period names per 1 of the 7
                               * issuanceTypes. */
   layouts currentTimeLayout; /* Structure containing the current element's
                               * period, first startTime, and numRows. Used
                               * in determining if a new layout is needed. */
   char periodName[30];       /* Name of special period name (i.e.
                               * "Overnight"). */
   uChar issuanceType = MAX_PERIODS;  /* Max number of issuanceTypes. */
   
   /* If DWMLgen product, set numFmtdRows = to numRows. */
   if (f_XML == 1 || f_XML == 2)
      *numFmtdRows = numRows;

   /* Find first validTime per element interested in and start filling in the 
    * time layout array with this current data. Also, to retrieve the period
    * length get the first and second valid time in double form. */
   for (i = 1; i < numMatch; i++)
   {
      if (match[i - 1].elem.ndfdEnum == parameterName && numRows != 1)
      {
         elemCount += 1;
         if (match[i].elem.ndfdEnum != match[i - 1].elem.ndfdEnum)
         {
            formatValidTime(match[i - elemCount].validTime,
                            currentTimeLayout.fmtdStartTime, 30, TZoffset,
                            f_observeDST);
            firstValidTime = match[i - elemCount].validTime;
            secondValidTime = match[i - (elemCount - 1)].validTime;
            break;
         }
      }
      else if (match[i - 1].elem.ndfdEnum == parameterName && numRows == 1)
      {
         formatValidTime(match[i - 1].validTime,
                         currentTimeLayout.fmtdStartTime, 30, TZoffset,
                         f_observeDST);
         firstValidTime = match[i - 1].validTime;
         break;
      }

      if ((i == numMatch - 1) && (match[i - 1].elem.ndfdEnum == parameterName))
      {
         formatValidTime(match[i - elemCount].validTime,
                         currentTimeLayout.fmtdStartTime, 30, TZoffset,
                         f_observeDST);
         firstValidTime = match[i - elemCount].validTime;
         secondValidTime = match[i - (elemCount - 1)].validTime;
      }
   }

   /* Get the period length using either the period name or the valid times. */

   if (parameterName == NDFD_MAX || parameterName == NDFD_MIN)
      period = 24;
   else if (parameterName == NDFD_POP)
      period = 12;
   else                       /* Calculate it */
      period = determinePeriodLength(firstValidTime, secondValidTime, numRows,
                                     parameterName);
   /* Fill the rest of the time layout array with current data. */

   currentTimeLayout.period = period;
   currentTimeLayout.numRows = numRows;

   /* Determine if this layout information has already been formatted. */
   if (isNewLayout(currentTimeLayout, numLayoutSoFar, numCurrentLayout,
                   f_finalTimeLayout) == 1)
   {
      /* Create the new key and bump the number of layouts by one. */
      sprintf(layoutKey, "k-p%dh-n%d-%d", period, *numFmtdRows, 
	      *numLayoutSoFar);
      *numLayoutSoFar += 1;
      
      /* See if we need to format an <end-valid-time> tag . */
      useEndTimes = checkNeedForEndTime(parameterName);

      /* Some parameters like max and min temp don't have valid times that
       * match the real start time.  So make the adjustment. */
      if (*numFmtdRows > numRows) 
      {
         startTimes = (char **)malloc(*numFmtdRows * sizeof(char *));
        if (useEndTimes)
            endTimes = (char **)malloc(*numFmtdRows * sizeof(char *));
	
         computeStartEndTimes(parameterName, *numFmtdRows, period, TZoffset,
                              f_observeDST, match, numMatch, useEndTimes,
                              startTimes, endTimes, frequency, f_XML, 
			      startTime_cml, currentDoubTime, 
			      &numPopRowsSkippedBeginning);
      }
      else
      {
         startTimes = (char **)malloc(numRows * sizeof(char *));
         if (useEndTimes)
            endTimes = (char **)malloc(numRows * sizeof(char *));
         computeStartEndTimes(parameterName, numRows, period, TZoffset,
                              f_observeDST, match, numMatch, useEndTimes,
                              startTimes, endTimes, frequency, f_XML, 
			      startTime_cml, currentDoubTime, 
			      &numPopRowsSkippedBeginning);
      }
      
      /* Format the XML time layout in the output string. */
      time_layout = xmlNewChild(data, NULL, BAD_CAST "time-layout", NULL);
      xmlNewProp(time_layout, BAD_CAST "time-coordinate", BAD_CAST
                 timeCoordinate);
      xmlNewProp(time_layout, BAD_CAST "summarization", BAD_CAST summarization);
      layout_key = xmlNewChild(time_layout, NULL, BAD_CAST "layout-key",
                               BAD_CAST layoutKey);

      /* Before looping throught the valid times determine the period
       * information "issuanceType" and "numPeriodNames". */

      if (f_formatPeriodName && period >= 12)
         getPeriodInfo(parameterName, startTimes[0], currentHour, currentDay,
                       &issuanceType, &numPeriodNames, period, frequency);
      
      /* Now we get the time values for this parameter and format the valid time
       * tags. */
      numPopRowsSkippedTemp = numPopRowsSkippedBeginning;
      for (i = 0; i < *numFmtdRows; i++)
      {
         if (i < *numFmtdRows) /* Accounts for DWMLgenByDay. */
         {
	    if (startTimes[i])
            {
	       if (numPopRowsSkippedTemp == 0)
	       {	       
                  startValTime = xmlNewChild(time_layout, NULL, BAD_CAST
                                             "start-valid-time", BAD_CAST
                                             startTimes[i]);

                  /* We only format period names for parameters with period
                   * greater than 12 hours like (max and min temp, and pop12
                   * etc). */
                  if (f_formatPeriodName && period >= 12)
                  {
                     outputPeriodName = 0;
                     periodName[0] = '\0';
                     Clock_Scan(&startTime_doub, startTimes[i], 1);
		     Clock_Print2(dayName, 30, startTime_doub, "%v", 
			          TZoffset, f_observeDST);

		     /* First see if one of these first special period names is
		      * to be trumped by a holiday name. If so, don't get the 
		      * special period name.
		      */
    	             if (strcmp(dayName, "Sunday") == 0 || 
		         strcmp(dayName, "Monday") == 0 ||
		         strcmp(dayName, "Tuesday") == 0 ||
		         strcmp(dayName, "Wednesday") == 0 ||
		         strcmp(dayName, "Thursday") == 0 ||
		         strcmp(dayName, "Friday") == 0 ||
		         strcmp(dayName, "Saturday") == 0)
                     {
                        checkNeedForPeriodName(i, &numPeriodNames,
                                               TZoffset, parameterName,
                                               startTimes[i],
                                               &outputPeriodName, issuanceType,
                                               periodName, currentHour, 
					       currentDay, startTime_cml, 
					       currentDoubTime, firstValidTime,
					       period);
		     }

                     /* Handle each special period name (up to 3 of them). */
                     if (outputPeriodName)
                     {
                        xmlNewProp(startValTime, BAD_CAST "period-name", BAD_CAST
                                   periodName);
                     }		  
                     else
                      /* We are past special names occurring during the *
                       * first few periods. 
		       */
                     {
                        /* We will use the day of the week for the period name
                         * and add "Night" if it's a night time period. Since we
                         * may have altered the startTimes from the
                         * match.validTimes, we need to send the double version
                         * of the string startTimes into Clock_Print2 routine. 
			 */

                        if (useNightPeriodName(startTimes[i]) == 0)
                        {
                           xmlNewProp(startValTime, BAD_CAST "period-name",
                                      BAD_CAST dayName);
                        }
                        else /* Night time period. Use the "%A" format to insure
			      * that a holiday name isn't placed in a night 
			      * period.
			      */
		        {
			   Clock_Print2(dayName, 30, startTime_doub, "%A", 
					TZoffset, f_observeDST);
                           strcat(dayName, " Night");
                           xmlNewProp(startValTime, BAD_CAST "period-name",
                                      BAD_CAST dayName);
			}
                     }
                  }   
                  /* If this is a parameter needing an <end-valid-time> tag, we
                   * format it. */

                  if (useEndTimes)
                     xmlNewChild(time_layout, NULL, BAD_CAST "end-valid-time",
                                 BAD_CAST endTimes[i]);
	       }
	       else
	       {
                  numPopRowsSkippedTemp--;
	          continue;
               }
            }
            else /* No startTime or the first Pop Rows is skipped. */
            {
               startValTime = xmlNewChild(time_layout, NULL, BAD_CAST
                                          "start-valid-time", BAD_CAST NULL);
               xmlNewProp(startValTime, BAD_CAST "xsi:nil", BAD_CAST "true");
               if (useEndTimes)
               {
                  endValTime = xmlNewChild(time_layout, NULL, BAD_CAST
                                           "end-valid-time", BAD_CAST NULL);
                  xmlNewProp(endValTime, BAD_CAST "xsi:nil", BAD_CAST "true");
               }               
	    }	       
	 }
      }

      /* Free some things. */
      if (*numFmtdRows > numRows) 
      {
         for (i = 0; i < *numFmtdRows; i++)
         {
            free(startTimes[i]);
            if (useEndTimes)
               free(endTimes[i]);
         }
      }
      else
      {
         for (i = 0; i < numRows; i++)
         {
            free(startTimes[i]);
            if (useEndTimes)
               free(endTimes[i]);
         }
      }

      free(startTimes);
      if (useEndTimes)
         free(endTimes);

   }
   else                       /* Not a new key so just return the key name */
   {
      sprintf(layoutKey, "k-p%dh-n%d-%d", period, *numFmtdRows, *numCurrentLayout);
   }
   
   return;

}

/*****************************************************************************
 * formatMetaDWML() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Creates the Meta Data for the formatted XML products (DWMLgen's 
 *  "time-series" and "glance" products and DWMLgenByDay's "12 hourly" and "24
 *  hourly" formatted products.
 *
 * ARGUMENTS
 *       f_XML = Flag denoting type of XML product (1 = DWMLgen's "time-series"
 *               product, 2 = DWMLgen's "glance" product, 3 = DWMLgenByDay's 
 *               "12 hourly" product, 4 = DWMLgenByDay's "24 hourly" product.
 *               (Input) 
 *         doc = An xml Node Pointer denoting the top-level document node. 
 *               (Input)
 *        data = An xml Node Pointer denoting the <data> node. (Input)
 *        dwml = An xml Node Pointer denoting the <dwml> node. (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  1/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void formatMetaDWML(uChar f_XML, xmlDocPtr * doc, xmlNodePtr * data,
                           xmlNodePtr * dwml)
{

   char *prodOrFormat = NULL; /* Denotes the product of format for output
                               * into XML. */
   char *operationalMode = NULL;  /* Denotes the operational mode for output
                                   * into XML. */
   char *productTitle = NULL; /* Denotes the product title for output into
                               * XML. */
   char *moreInfo = NULL;     /* Denotes the URL where more info can be found 
                               * for output into XML. */
   char *prod_center = NULL;  /* Denotes the place of development for output
                               * into XML. */
   char *sub_center = NULL;   /* Denotes the sub-place of development for
                               * output into XML. */
   char currentTime[30];      /* Denotes current UTC time the product is
                               * created. */
   double currentDoubTime;    /* Denotes current UTC time (as a double), the
                               * product is created. */

   /* Local XML Node pointers */

   xmlNodePtr node = NULL;
   xmlNodePtr head = NULL;
   xmlNodePtr production_center = NULL;

   /* Set up the header information depending on product. */

   switch (f_XML)
   {
      case 1:
         prodOrFormat = "time-series";
         productTitle = "NOAA's National Weather Service Forecast Data";
         operationalMode = "developmental";
         moreInfo = "http://www.nws.noaa.gov/forecasts/xml/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         break;
      case 2:
         prodOrFormat = "glance";
         productTitle = "NOAA's National Weather Service Forecast at a Glance";
         operationalMode = "developmental";
         moreInfo = "http://www.nws.noaa.gov/forecasts/xml/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         break;
      case 3:
         prodOrFormat = "dwmlByDay";
         productTitle =
               "NOAA's National Weather Service Forecast by 12 Hour Period";
         operationalMode = "developmental";
         moreInfo = "http://www.nws.noaa.gov/forecasts/xml/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         break;
      case 4:
         prodOrFormat = "dwmlByDay";
         productTitle =
               "NOAA's National Weather Service Forecast by 24 Hour Period";
         operationalMode = "developmental";
         moreInfo = "http://www.nws.noaa.gov/forecasts/xml/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         break;
   }

   /* Get the document creation date. */

   currentDoubTime = Clock_Seconds();
   Clock_Print2(currentTime, 30, currentDoubTime, "%Y-%m-%dT%H:%M:%S", 0, 0);
   strcat(currentTime, "Z");

   /* Create the XML document and format the header data. */

   *doc = xmlNewDoc(BAD_CAST "1.0");
   *dwml = xmlNewNode(NULL, BAD_CAST "dwml");
   xmlNewProp(*dwml, BAD_CAST "version", BAD_CAST "1.0");
   xmlNewProp(*dwml, BAD_CAST "xmlns:xsd", BAD_CAST
              "http://www.w3.org/2001/XMLSchema");
   xmlNewProp(*dwml, BAD_CAST "xmlns:xsi", BAD_CAST
              "http://www.w3.org/2001/XMLSchema-instance");
   xmlNewProp(*dwml, BAD_CAST "xsi:noNamespaceSchemaLocation", BAD_CAST
              "http://www.nws.noaa.gov/forecasts/xml/DWMLgen/schema/DWML.xsd");

   xmlDocSetRootElement(*doc, *dwml);
   head = xmlNewNode(NULL, BAD_CAST "head");
   node = xmlNewChild(head, NULL, BAD_CAST "product", NULL);
   xmlNewProp(node, BAD_CAST "concise-name", BAD_CAST prodOrFormat);
   xmlNewProp(node, BAD_CAST "operational-mode", BAD_CAST operationalMode);
   xmlNewChild(node, NULL, BAD_CAST "title", BAD_CAST productTitle);
   xmlNewChild(node, NULL, BAD_CAST "field", BAD_CAST "meteorological");
   xmlNewChild(node, NULL, BAD_CAST "category", BAD_CAST "forecast");
   node = xmlNewChild(node, NULL, BAD_CAST "creation-date", BAD_CAST
                      currentTime);
   xmlNewProp(node, BAD_CAST "refresh-frequency", BAD_CAST "PT1H");
   node = xmlNewNode(NULL, BAD_CAST "source");
   xmlNewChild(node, NULL, BAD_CAST "more-information", BAD_CAST moreInfo);
   production_center = xmlNewChild(node, NULL, BAD_CAST
                                   "production-center", BAD_CAST prod_center);
   xmlNewChild(production_center, NULL, BAD_CAST "sub-center", BAD_CAST
               sub_center);
   xmlNewChild(node, NULL, BAD_CAST "disclaimer", BAD_CAST
               "http://www.nws.noaa.gov/disclaimer.html");
   xmlNewChild(node, NULL, BAD_CAST "credit", BAD_CAST
               "http://www.weather.gov/");
   xmlNewChild(node, NULL, BAD_CAST "credit-logo", BAD_CAST
               "http://www.weather.gov/images/xml_logo.gif");
   xmlNewChild(node, NULL, BAD_CAST "feedback", BAD_CAST
               "http://www.weather.gov/survey/nws-survey.php?code=xmlsoap");
   xmlAddChild(head, node);
   xmlAddChild(*dwml, head);
   *data = xmlNewNode(NULL, BAD_CAST "data");

   return;
}

/******************************************************************************
 * getNumRows() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Code retrieves the number of rows of data (aka the number of data values) 
 *  for each element retrieved from by degrib from NDFD. These are the elements  
 *  formatted and also any elements used to derive the formatted elements. 
 *
 * ARGUMENTS
 *           f_XML = Flag denoting type of XML product (1 = DWMLgen's 
 *                   "time-series" product, 2 = DWMLgen's "glance" product, 3 = 
 *                   DWMLgenByDay's "12 hourly" product, 4 = DWMLgenByDay's 
 *                   "24 hourly" product. (Input) 
 * numRowsForPoint = Number of rows data is formatted for in the output XML (aka 
 *                   the number of data values). This number is point dependant.
 *                   (Input/Output).
 *           match = Pointer to the array of element matches from degrib. 
 *                   (Input) 
 *        numMatch = The number of matches from degrib. (Input)
 *         numDays = The number of days the validTimes for all the data rows 
 *                   (values) consist of. Used for the DWMLgenByDay products 
 *                   only. (Input)
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

static void getNumRows(uChar * numRowsForPoint, size_t numMatch, 
		       genMatchType * match, int numDays, uChar * wxParameters, 
		       uChar f_XML, sChar * f_icon)
{

   int i;

   /* Initialize numRowsForPoint to zero. */

   for (i = 0; i < (NDFD_MATCHALL + 1); i++)
      numRowsForPoint[i] = 0;
   
   /* Retrieve the number of rows per element for the DWMLgen products. */


      for (i = 0; i < numMatch; i++)
         numRowsForPoint[match[i].elem.ndfdEnum]++;

   /* Since numRows determination is based off of what was actually returned
    * from NDFD, update the weatherParameters array accordingly to reflect
    * this. */
   for (i = 0; i < (NDFD_MATCHALL + 1); i++)
   {
      if (wxParameters[i] >= 1 && numRowsForPoint[i] == 0)
         wxParameters[i] = 0;
   }

   /* Now, check to see that Icons have all the necessary elements retrieved
    * from NDFD to derive them. */
   if ((f_XML == 1 || f_XML == 2) && *f_icon == 1)
   {
      if (numRowsForPoint[NDFD_TEMP] == 0 || numRowsForPoint[NDFD_WS] == 0 ||
          numRowsForPoint[NDFD_SKY] == 0 || numRowsForPoint[NDFD_WX] == 0)
      {
         printf("**************************************\n");
         printf("Cannot format Icons at this time as\n");
         printf("element(s) used to derive are missing.\n");
         printf("**************************************\n");
         *f_icon = 0;
      }
   }
   
   /* Now, check to see that Icons have all the necessary elements retrieved
    * from NDFD to derive them. */
   
   return;

}

/*****************************************************************************
 * prepareDWMLgen() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Flags those elements that are ultimately formatted in the output XML for 
 *  the DWMLgen products. The user chooses the elements for the "time-series" 
 *  product. There are pre-defined elements formatted for the "glance" product.
 *  Routine also sets the flag determining if period names are used in the time
 *  layout generation.
 *
 * ARGUMENTS
 *            f_XML = Flag denoting type of XML product (1 = DWMLgen's 
 *                    "time-series"product, 2 = DWMLgen's "glance" product, 3 
 *                    = DWMLgenByDay's "12 hourly" product, 4 = DWMLgenByDay's 
 *                    "24 hourly" product. (Input) 
 *     wxParameters = Array containing the flags denoting whether a certain 
 *                    element is formatted in the output XML (= 1), or used in 
 *                    deriving another formatted element (= 2). (Input/Output) 
 *    summarization = The type of temporal summarization being used.
 *                    Currently, no summarization is done in time.
 * f_formatPeriodName = Flag to indicate if period names (i.e. "today") appear 
 *                      in the start valid time tag: 
 *                      <start-valid-time period-name="today"> (Input)
 *          varFilter = Shows what NDFD variables are of interest(= 1) or 
 *                      vital(= 2).
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void prepareDWMLgen(uChar f_XML, uChar * f_formatPeriodName,
                           uChar * wxParameters, char *summarization,
                           uChar varFilter[NDFD_MATCHALL + 1],
                           sChar * f_icon)
{

   strcpy(summarization, "none"); /* There is no hourly summariztion for the
                                   * DWMLgen products. */

   /* Flag those elements in the "time-series" product to be formatted in the 
    * output XML. */

   if (f_XML == 1)
   {
      if (varFilter[NDFD_MAX] == 2)
         wxParameters[NDFD_MAX] = 1;
      if (varFilter[NDFD_MIN] == 2)
         wxParameters[NDFD_MIN] = 1;
      if (varFilter[NDFD_POP] == 2)
         wxParameters[NDFD_POP] = 1;
      if (varFilter[NDFD_TEMP] == 2)
         wxParameters[NDFD_TEMP] = 1;
      if (varFilter[NDFD_WD] == 2)
         wxParameters[NDFD_WD] = 1;
      if (varFilter[NDFD_WS] == 2)
         wxParameters[NDFD_WS] = 1;
      if (varFilter[NDFD_TD] == 2)
         wxParameters[NDFD_TD] = 1;
      if (varFilter[NDFD_SKY] == 2)
         wxParameters[NDFD_SKY] = 1;
      if (varFilter[NDFD_QPF] == 2)
         wxParameters[NDFD_QPF] = 1;
      if (varFilter[NDFD_SNOW] == 2)
         wxParameters[NDFD_SNOW] = 1;
      if (varFilter[NDFD_WX] == 2)
         wxParameters[NDFD_WX] = 1;
      if (varFilter[NDFD_WH] == 2)
         wxParameters[NDFD_WH] = 1;
      if (varFilter[NDFD_AT] == 2)
         wxParameters[NDFD_AT] = 1;
      if (varFilter[NDFD_RH] == 2)
         wxParameters[NDFD_RH] = 1;

      /* We need to create a time layout for the icons in the case that only
       * icons is to be formatted. When this occurs, we make Icons use the
       * weather's time layout. Set the wxParameters flag for WX to = 3. */

      if (*f_icon == 1)
      {
         if (wxParameters[NDFD_WX] != 1)
            wxParameters[NDFD_WX] = 3;
      }
   }

   /* For DWMLgen's "glance" product, there are five pre-defined set of NDFD
    * parameters to be formatted. Four of these are maxt, mint, sky, and wx.
    * Two other elements not formatted (temp and wind speed) are used to
    * derive the 5th element formatted: Icons. */

   else if (f_XML == 2)
   {
      *f_formatPeriodName = 1;
      if (varFilter[NDFD_MAX] >= 2)
         wxParameters[NDFD_MAX] = 1;
      if (varFilter[NDFD_MAX] >= 2)
         wxParameters[NDFD_MIN] = 1;
      if (varFilter[NDFD_MAX] >= 2)
         wxParameters[NDFD_SKY] = 1;
      if (varFilter[NDFD_MAX] >= 2)
         wxParameters[NDFD_WX] = 1;
   }

   return;
}

/*****************************************************************************
 * prepareDWMLgenByDay() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Flags those elements that are ultimately formatted in the output XML for 
 *  the DWMLgenByDay products. There are pre-defined elements formatted for the 
 *  "12 hourly and "24 hourly" products. Routine also sets the flag determining 
 *  if period names are used in the time layout generation. Routine also 
 *  calculates numOutputLines, the timeInterval of each period, and sets the 
 *  summarization and format character strings.
 *
 * ARGUMENTS
 *          numDays = The number of days the validTimes for all the data rows 
 *                    (values) consist of. Used for the DWMLgenByDay products only.
 *                    (Input)
 *            f_XML = Flag denoting type of XML product (1 = DWMLgen's 
 *                    "time-series"product, 2 = DWMLgen's "glance" product, 3 
 *                    = DWMLgenByDay's "12 hourly" product, 4 = DWMLgenByDay's 
 *                    "24 hourly" product. (Input) 
 *     wxParameters = Array containing the flags denoting whether a certain 
 *                    element is formatted in the output XML (= 1), or used in 
 *                    deriving another formatted element (= 2). (Input/Output) 
 *    summarization = The type of temporal summarization being used. 
 *                    (Input/Output)
 * f_formatPeriodName = Flag to indicate if period names (i.e. "today") appear 
 *                      in the start valid time tag: 
 *                      <start-valid-time period-name="today"> (Input)
 *            match = Pointer to the array of element matches from degrib. 
 *                    (Input) 
 *         numMatch = The number of matches from degrib. (Input)
 *     timeInterval = Number of seconds in either a 12 hourly format (3600 * 12)
 *                    or 24 hourly format (3600 * 24). (Input/Output)
 *   numOutputLines = The number of data values output.  For the "24 hourly"
 *                    format, numDays equals numOutputLines.  For the 
 *                    "12 hourly" format, numOutputLines is equal to twice 
 *                    the number of days since there are two 12 hour periods. 
 *                    (Input/Output)
 *  lastValidTimeMatch = Time of last valid data value. Used in determining 
 *                       numDays value. (Output)
 * firstValidTimeMatch = Time of first valid data value. Used in determining 
 *                       numDays value. (Output)
 *        startTime = First valid time that user is interested in. (Input)
 *          endTime = Last valid time that user is interested in. (Input)
 *                      
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */

static void prepareDWMLgenByDay(genMatchType *match, size_t numMatch, 
		                uChar f_XML, double *startTime_cml, 
				double *endTime_cml,
				double *firstValidTimeMatch, 
				double *lastValidTimeMatch, int *numDays, 
				char *format, uChar *f_formatPeriodName,
                                uChar * wxParameters, int *timeInterval,
                                int *numOutputLines, char *summarization,
				double currentDoubTime)
{

   int i; /* Counter through the match structure. */
   
   /* Get the validTime of the first match and last match of the match 
    * structure. We'll need these to calculate numDays. 
    */
   *firstValidTimeMatch = match[0].validTime;
   *lastValidTimeMatch = match[0].validTime;
   for (i = 0; i < numMatch; i++)
   {
      if (match[i].validTime < *firstValidTimeMatch)
         *firstValidTimeMatch = match[i].validTime;
      if (match[i].validTime > *lastValidTimeMatch)
         *lastValidTimeMatch = match[i].validTime;
   }
   
   if (*startTime_cml == 0.0 && *endTime_cml == 0.0)	   
      *numDays = ceil(((*lastValidTimeMatch - currentDoubTime) / 3600) / 24);
   else if (*startTime_cml == 0.0 && *endTime_cml != 0.0)
   {	   
      /* Then endTime was entered on the command line argument. See if the time
       * entered occurs after the last valid data in NDFD. If so, simply treat
       * it as if no endTime was entered (set endTime = 0.0).
       */	   
      if (*endTime_cml > *lastValidTimeMatch)
      {
         *endTime_cml = 0.0;
         *numDays = ceil(((*lastValidTimeMatch - currentDoubTime) / 3600) / 24);
      }
      else
      {
         *numDays = (int)myRound(((*endTime_cml - currentDoubTime) / 3600) / 24, 0);
      }

   }
   else if (*startTime_cml != 0.0)
   {
      /* First, see if the startTime entered on the command line occurs before
       * current system time. 
       */
      if (*startTime_cml < currentDoubTime)
      {
         *startTime_cml = 0.0;
         *numDays = ceil(((*lastValidTimeMatch - currentDoubTime) / 3600) / 24);
      }      
      else
      {
         *numDays = ceil(((*lastValidTimeMatch - *firstValidTimeMatch) / 3600) / 24);
      }
   }      

   /* Then startTime was entered as a command line argument from the user.
    * Simply calculate numDays. 
    */
    
   /* DWMLgenByDay, both formats, have pre-defined sets of NDFD parameters. */	   
   if (f_XML == 3)
   {
      *f_formatPeriodName = 1;
      *timeInterval = 3600 * 12;  /* number of seconds in half a day */
      *numOutputLines = *numDays * 2; /* All elements formatted this way in 
					 12 hourly product except MaxT & Mint. */
      strcpy(summarization, "12hourly");
      strcpy(format, "12 hourly");
   }
   else if (f_XML == 4)       /* Note, there is no formatting of period
                               * names. */
   {	    
      *timeInterval = 3600 * 24;  /* number of seconds in a whole day */
      *numOutputLines = *numDays; /* Pop is only element formatted this way
					 in 24 hourly product. */
      strcpy(summarization, "24hourly");
      strcpy(format, "24 hourly");
   }

   /* Flag the below four elements for formatting in the ouput XML (icons
    * will be the fifth element formatted). Set the value to = 1. */

   wxParameters[NDFD_MAX] = 1;
   wxParameters[NDFD_MIN] = 1;
   wxParameters[NDFD_POP] = 1;
   wxParameters[NDFD_WX] = 1;

   /* We need to collect data for the following three elements too. They are
    * not formatted in the output XML, but are used in icon determination.
    * Set these values to = 2. */

   wxParameters[NDFD_SKY] = 2;
   wxParameters[NDFD_WS] = 2;
   wxParameters[NDFD_WD] = 2;

   return;
}

/*****************************************************************************
 * formatLocationInfo() -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Code is used to create all the time layouts (<start-valid-time> and 
 *  <end-valid-time>) tags.
 *  
 * ARGUMENTS
 *          numPnts = Number of points to probe for. (Input) 
 *             pnts = A pointer to the points to probe for (defined in type.h).
 *                    (Input)
 *             data = An xml Node Pointer denoting the <data> node of the 
 *                    formatted XML. (Output)
 *                            
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  5/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
static void formatLocationInfo(size_t numPnts, Point * pnts, xmlNodePtr data)
{

   int j;                     /* Point counter. */
   double lat;                /* Latitude of point. */
   double lon;                /* Longitude of point. */
   xmlNodePtr location = NULL;  /* Local xml Node Pointer denoting the
                                 * <location> node. */
   xmlNodePtr node = NULL;    /* Local xml Node Pointer. */
   char strPointBuff[20];     /* Temporary string holder for point. */
   char strLLBuff[20];        /* Temporary string holder for lat and lon. */

   /* Begin looping through the points to format each point's location,
    * grouping them together. */

   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
      {
         /* Before starting, make sure this point is in at least one of the
          * NDFD Sectors. */

         lat = pnts[j].Y;
         lon = pnts[j].X;

         location = xmlNewNode(NULL, BAD_CAST "location");
         sprintf(strPointBuff, "point%d", (j + 1));
         xmlNewChild(location, NULL, BAD_CAST "location-key", BAD_CAST
                     strPointBuff);
         node = xmlNewChild(location, NULL, BAD_CAST "point", NULL);
         sprintf(strLLBuff, "%2.2f", lat);
         xmlNewProp(node, BAD_CAST "latitude", BAD_CAST strLLBuff);
         sprintf(strLLBuff, "%2.2f", lon);
         xmlNewProp(node, BAD_CAST "longitude", BAD_CAST strLLBuff);
         xmlAddChild(data, location);

      }
   }
   return;
}

/* Print Routine #1. */
static void PrintSameTime(genMatchType * match, size_t pntIndex,
                          int *allElem, sChar pntTimeZone, sChar f_dayCheck)
{
   char timeBuff[30];
   char zone[7];
   size_t i;
   double localTime;

   for (i = 0; i < NDFD_MATCHALL + 1; i++)
   {
      if (allElem[i] != -1)
      {
         localTime = match[allElem[i]].validTime - pntTimeZone * 3600;
         /* localTime is now in local standard time */
         if (f_dayCheck)
         {
            /* Note: A 0 is passed to DaylightSavings so it converts from
             * local to local standard time. */
            if (Clock_IsDaylightSaving2(localTime, 0) == 1)
            {
               localTime += 3600;
               pntTimeZone -= 1;
            }
         }
         /* The 0, 0 is because we already converted to local standard /
          * daylight time. */
         Clock_Print2(timeBuff, 30, localTime, "%Y-%m-%dT%H:%M:%S", 0, 0);
         /* Change definition of pntTimeZone. */
         pntTimeZone = -1 * pntTimeZone;
         if (pntTimeZone < 0)
         {
            sprintf(zone, "-%02d:00", -1 * pntTimeZone);
         }
         else
         {
            sprintf(zone, "+%02d:00", pntTimeZone);
         }
         strcat(timeBuff, zone);
         break;
      }
   }
   /* Check that we have some elements. */
   if (i == NDFD_MATCHALL + 1)
      return;

   printf("%s ", timeBuff);
   if ((allElem[NDFD_TEMP] != -1) &&
       (match[allElem[NDFD_TEMP]].value[pntIndex].valueType != 2))
      printf("tt:%.0f ", match[allElem[NDFD_TEMP]].value[pntIndex].data);
   if ((allElem[NDFD_AT] != -1) &&
       (match[allElem[NDFD_AT]].value[pntIndex].valueType != 2))
      printf("at:%.0f ", match[allElem[NDFD_AT]].value[pntIndex].data);

   if ((allElem[NDFD_MAX] != -1) &&
       (match[allElem[NDFD_MAX]].value[pntIndex].valueType != 2))
   {
      printf("mx:%.0f ", match[allElem[NDFD_MAX]].value[pntIndex].data);
   }
   else if ((allElem[NDFD_MIN] != -1) &&
            (match[allElem[NDFD_MIN]].value[pntIndex].valueType != 2))
   {
      printf("mn:%.0f ", match[allElem[NDFD_MIN]].value[pntIndex].data);
   }
   else
   {
      printf("  :   ");
   }

   if ((allElem[NDFD_TD] != -1) &&
       (match[allElem[NDFD_TD]].value[pntIndex].valueType != 2))
      printf("td:%.0f ", match[allElem[NDFD_TD]].value[pntIndex].data);
   if ((allElem[NDFD_POP] != -1) &&
       (match[allElem[NDFD_POP]].value[pntIndex].valueType != 2))
   {
      printf("po:%.0f ", match[allElem[NDFD_POP]].value[pntIndex].data);
   }
   else
   {
      printf("  :   ");
   }
   if ((allElem[NDFD_RH] != -1) &&
       (match[allElem[NDFD_RH]].value[pntIndex].valueType != 2))
      printf("rh:%.0f ", match[allElem[NDFD_RH]].value[pntIndex].data);
   if ((allElem[NDFD_SKY] != -1) &&
       (match[allElem[NDFD_SKY]].value[pntIndex].valueType != 2))
      printf("sky:%.0f ", match[allElem[NDFD_SKY]].value[pntIndex].data);
   if ((allElem[NDFD_QPF] != -1) &&
       (match[allElem[NDFD_QPF]].value[pntIndex].valueType != 2))
   {
      printf("qp:%5.2f ", match[allElem[NDFD_QPF]].value[pntIndex].data);
   }
   else
   {
      printf("  :      ");
   }
   if ((allElem[NDFD_SNOW] != -1) &&
       (match[allElem[NDFD_SNOW]].value[pntIndex].valueType != 2))
   {
      printf("sn:%5.2f ", match[allElem[NDFD_SNOW]].value[pntIndex].data);
   }
   else
   {
      printf("  :      ");
   }

   if ((allElem[NDFD_WS] != -1) &&
       (match[allElem[NDFD_WS]].value[pntIndex].valueType != 2))
      printf("ws:%2.0f ", match[allElem[NDFD_WS]].value[pntIndex].data);
   if ((allElem[NDFD_WD] != -1) &&
       (match[allElem[NDFD_WD]].value[pntIndex].valueType != 2))
   {
      printf("wd:%.0f ", match[allElem[NDFD_WD]].value[pntIndex].data);
/*
      printf ("wd:%3s ",
              DegToCompass (match[allElem[NDFD_WD]].value[pntIndex].data));
*/
   }
   if ((allElem[NDFD_WH] != -1) &&
       (match[allElem[NDFD_WH]].value[pntIndex].valueType != 2))
      printf("wh:%.0f ", match[allElem[NDFD_WH]].value[pntIndex].data);

   if ((allElem[NDFD_WX] != -1) &&
       (match[allElem[NDFD_WX]].value[pntIndex].valueType != 2))
      printf("\n\t%s ", match[allElem[NDFD_WX]].value[pntIndex].str);

   printf("\n");
}

/* Print Routine #2. */
static void PrintSameDay1(genMatchType * match, size_t pntIndex,
                          collateType * collate, size_t numCollate,
                          sChar pntTimeZone, sChar f_dayCheck)
{
   size_t i;
   size_t j;
   int *dayIndex;
   int numDayIndex = 1;
   sInt4 totDay;              /* # of days since epoch in LST (LDT ignored). */
   sInt4 curTotDay;           /* # of days since epoch in LST (LDT ignored). */

   dayIndex = malloc(numDayIndex * sizeof(int));
   dayIndex[numDayIndex - 1] = 0;

   curTotDay = (sInt4) floor((collate[0].validTime -
                              pntTimeZone * 3600) / SEC_DAY);
   for (i = 1; i < numCollate; i++)
   {
      totDay = (sInt4) floor((collate[i].validTime -
                              pntTimeZone * 3600) / SEC_DAY);
      if (totDay != curTotDay)
      {
         curTotDay = totDay;
         numDayIndex++;
         dayIndex = realloc(dayIndex, numDayIndex * sizeof(int));
         dayIndex[numDayIndex - 1] = i;
      }
   }
   if (dayIndex[numDayIndex - 1] != i)
   {
      numDayIndex++;
      dayIndex = realloc(dayIndex, numDayIndex * sizeof(int));
      dayIndex[numDayIndex - 1] = i;
   }

   for (i = 0; i < numDayIndex - 1; i++)
   {
      for (j = dayIndex[i]; j < dayIndex[i + 1]; j++)
      {
         PrintSameTime(match, pntIndex, collate[j].allElem, pntTimeZone,
                       f_dayCheck);
      }
      printf("--- End of day ---\n");
   }

   free(dayIndex);
}

/******************************************************************************/
/* XMLParse () --
 * 
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *   The driver program that ultimately formats the DWMLgen products 
 *   "time-series" and "glance" and the DWMLgenByDay products (formats)
 *   "12 hourly" and 24 hourly". 
 * 
 * ARGUMENTS 
 *       f_XML = flag for 1 of the 4 DWML products (Input):
 *               1 = DWMLgen's "time-series" product. 
 *               2 = DWMLgen's "glance" product.
 *               3 = DWMLgenByDay's "12 hourly" format product.
 *               4 = DWMLgenByDay's "24 hourly" format product.
 *     numPnts = Number of points to probe for. (Input) 
 *        pnts = A pointer to the points to probe for (defined in type.h).
 *               (Input)
 *     pntInfo = A pointer to each points' timezone and DST info. (defined
 *               in sector.h). (Input)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y. (Input)
 *      labels = Pointer to character array holding Station Names for each 
 *               point. (Input)
 *  numInFiles = Number of input files. (Input)
 *     InFiles = Pointer to character array holding the input files. (Input)
 *  f_fileType = Type of input files. (0=GRIB, 1=Data Cube index file) (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *      f_unit = 0 (GRIB unit), 1 (english), 2 (metric) (Input)
 *    majEarth = Used to override the TDLP major axis of earth. (Input)
 *    minEarth = Used to override the TDLP minor axis of earth. (Input)
 *      f_icon = Flag denoting whether icons are to be derived and formatted.
 *               If this flag is chosen, the other 4 elements' data used to 
 *               derive the icons must be retrieved/derived too (WS, SKY, 
 *               TEMP, WX). (Input)
 * f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *   f_valTime = 0 = false, 1 = f_validStartTime, 2 = f_validEndTime,
 *               3 = both f_validStartTime, and f_validEndTime (Input)
 *   startTime = First valid time that we are interested in. If this variable 
 *               exists as an input, it was set by the user. (Input)
 *     endTime = Last valid time that we are interested in.i If this variable 
 *               exists as an input, it was set by the user. (Input)
 * numNdfdVars = Number of user selected ndfdVars. (Input)
 *    ndfdVars = Pointer to the user selected NDFD variables. (Input)      
 *
 * FILES/DATABASES: None
 *              
 * RETURNS: int
 *         
 * HISTORY
 *   12/2005 Arthur Taylor/Paul Hershberg (MDL): Created.
 *
 * NOTES: The NDFD element list is below.
 *
 * enum { NDFD_MAX, NDFD_MIN, NDFD_POP, NDFD_TEMP, NDFD_WD, NDFD_WS,
 *        NDFD_TD, NDFD_SKY, NDFD_QPF, NDFD_SNOW, NDFD_WX, NDFD_WH,
 *        NDFD_AT, NDFD_RH, NDFD_UNDEF, NDFD_MATCHALL
 *      };
 * 
 ****************************************************************************** 
 */

int XMLParse(uChar f_XML, size_t numPnts, Point * pnts,
             PntSectInfo * pntInfo, sChar f_pntType, char **labels,
             size_t numInFiles, char **inFiles, uChar f_fileType,
             sChar f_interp, sChar f_unit, double majEarth, double minEarth,
             sChar f_icon, sChar f_SimpleVer, sChar f_valTime,
             double startTime, double endTime, size_t numNdfdVars,
             uChar * ndfdVars, char *f_inTypes, char *gribFilter,
             size_t numSector, char **sector, sChar f_ndfdConven)
{

   size_t numElem = 0;        /* Num of elements returned by degrib. */
   genElemDescript *elem = NULL;  /* Structure with info about the element. */
   uChar varFilter[NDFD_MATCHALL + 1];  /* Shows what NDFD variables are of
                                         * interest(= 1) or vital(= 2). */
   size_t numMatch = 0;       /* The number of matches from degrib. */
   genMatchType *match = NULL;  /* Structure of element matches returned from 
                                 * degrib. */
   int i;                     /* Generally used as the element incrementer. */
   size_t j;                  /* Generally used as the point incrementer. */
   size_t k;                  /* Generally used as the time-layout
                               * incrementer. */
   collateType *collate = NULL; /* Used to get elements per each validTime. */
   size_t numCollate = 0;     /* Number of collate members. */
   char *f_pntHasData;        /* Flag used to check if a point has any valid
                               * data. */
   double curTime;            /* Temporary storage for validTimes. */
   int numDays = 0;           /* The number of days the validTimes for all
                               * the data rows (values) consist of. Used for
                               * the DWMLgenByDay products only. */
   int numPopLines = 0; /* Since Pop, in the 24 hourly (f_MXL = 4) product still has 
		         * formatted data every 12 hours, we use this variable and 
		         * set it to numDays *2 when calling generateTimeLayout().
		         */
   int numPopRowsSkippedBeginning = 0;
   uChar weatherParameters[NDFD_MATCHALL + 1];  /* Array containing the flags 
                                                 * denoting whether a certain 
                                                 * element is formatted in
                                                 * the output XML (=1), or
                                                 * used in deriving another
                                                 * formatted element (=2). */
   sChar *TZoffset = NULL;    /* Number of hours to add to current time to
                               * get GMT time. */
   char **startDate = NULL;   /* Point specific user supplied start Date
                               * (i.e. 2006-04-15). */
   int f_atLeastOnePointInSectors = 0;  /* Flag used to denote if at least
                                         * one point is in any of NDFD
                                         * Sectors. */
   uChar **numRowsForPoint;   /* Number of data rows in the Match structure
				 for each element. */
   uChar f_formatPeriodName = 0;  /* Flag to indicate if period names (i.e.
                                   * "today") appear in the start valid time
                                   * tag: <start-valid-time
                                   * period-name="today">. */
   int f_firstPointLoopIteration = 1; /* In the point loop for the time layout
				         generations, this flag denotes the 
					 first time loop is entered. */
   int timeInterval;          /* Number of seconds in either a 12 hourly
                               * format 3600*12 or 24 hourly format 3600*24. */
   int numOutputLines;        /* The number of data values output. For the
                               * "24 hourly" format, numDays equals
                               * numOutputLines. For the "12 hourly" format,
                               * numOutputLines is equal to twice the number
                               * of days since there are two 12 hour period. */
   int integerStartUserTime = 0; /* Number of seconds since 1970 to when the
				  * user established periods begin. */
   int integerTime = 0; /* Number of seconds since 1970 to when the data is 
			  * valid. Allows the code to know if this data belongs 
			  * in the current period being processed. */ 
   const char whichTimeCoordinate[] = "local";  /* Flag indicating which time
                                                 * coordinate we are to format
                                                 * the data in (local is the
                                                 * only coordinate currently
                                                 * supported). */
   char whatSummarization[30];  /* Used to indicate what type of
                                 * summarization is being used. This is
                                 * currently set to none for DWMLgen product
                                 * since there is no summarization of NDFD
                                 * data and to 12 hourly or 24 hourly
                                 * depending on element and format for
                                 * DWMLgenByDay products. */
   char format[30];           /* The two DWMLgenByDay products have two
                               * formats.  The first indicated by
                               * "24 hourly", summarize NDFD data over 24
                               * hour (6 AM to 6 AM) periods. The second,
                               * "12 hourly" summarizes NDFD data in two 12
                               * hour periods for each 24 hour day. (6 AM to
                               * 6 PM) day and (6 PM to 6 AM) night periods. */
   char ***layoutKeys = NULL; /* Array containing the layout keys. */
   char currentDay[3];        /* Current day's 2-digit date. */
   char currentHour[3];       /* Current hour's 2-digit hour. */
   char startDateHr[3];       /* Hour of the first match in the match 
				 structure. */
   sChar f_WxParse;           /* Flag denoting wether to return the weather
                               * strings from NDFD as coded "ugly stings" or
                               * their English equivalent. */
   double firstValidTimeMatch;  /* The very first validTime for all matches.
                                 * Used in determining numDays. */
   double lastValidTimeMatch; /* The very last validTime for all matches.
                               * Used in determining numDays. */
   double firstValidTime_maxt = 0.0;
   double firstValidTime_mint = 0.0;
   double firstValidTime_pop = 0.0;
   int f_formatNIL = 0;       /* Flag denoting that we may have to create a
                               * NIL value for the first max temp if the
                               * request for data is late in the day. Only 
			       * applicable for 24 hourly summarization 
			       * product. */
   int f_useMinTempTimes = 0; /* Flag denoting if we should simply use the
                               * MinT layout key for the MaxT element. Only
                               * used in DWMLgenByDay product. */

   /* Variables that will go into formatTimeLayoutInfo() routine. */

   sChar f_observeDST;        /* Flag determining if current point observes
                               * Daylight Savings Time.  */
   char startDateBuff[30];    /* Temporary string. */
   double currentDoubTime;    /* Current real time in double format.  */
   char currentLocalTime[30]; /* Current Local Time in
                               * "2006-14-29T00:00:00-00:00" format. */
   uChar f_formatNewPointTimeLayouts = 0; /* Flag used fo denote if a new
                                           * time layout is needed due to a
                                           * point existing in a new time
                                           * zone. */
   char layoutKey[20];        /* The key linking the elements to their valid
                               * times (ex. k-p3h-n42-1). */
   size_t numLayoutSoFar = 1; /* Counter denoting the total number of time
                               * layouts that have been created so far. */
   uChar numCurrentLayout = 0;  /* The number of the layout we are currently
                                 * processing.  */
   double firstMaxTValidTime_doub_adj = 0.0;  /* The date of the first MaxT
                                               * validTime with the hour,
                                               * minutes, and seconds set to
                                               * zeroes, translated to double
                                               * form. Used in a DWNLgenByDay
                                               * check. */
   double currentLocalTime_doub_adj = 0.0;  /* The current date with the
                                             * hours, minutes, and seconds
                                             * set to zeroes, translated to
                                             * double form. Used in a
                                             * DWNLgenByDay check. */
   char *pstr;                /* Temporary pointer character string. */

   /* Variables that will go into formatParameterInfo() routine. */

   xmlNodePtr parameters = NULL;  /* An xml Node Pointer denoting the
                                   * <parameters> node in the formatted XML. */
   char pointBuff[20];        /* String holding point number. */
   int *maxDailyPop = NULL;   /* Array containing the pop values
                               * corresponding to a day (24 hour format) or
                               * 12 hour period (12 hour format).  For 24
                               * hour format, we use the maximum of the two
                               * 12 hour pops that span the day. This
                               * variable is used to test if the pop is large
                               * enough to justify formatting weather values. */
   int *maxWindSpeed = NULL;  /* Array containing the Maximum wind speed
                               * values corresponding to a day (24 hour
                               * format) or 12 hour period (12 hour format).
                               * These values are used in deriving the
                               * weather and/or icon values. */
   int *maxWindDirection = NULL;  /* Array containing the wind direction
                                   * values corresponding to a day (24 hour
                                   * format) or 12 hour period (12 hour
                                   * format). These are not "max" wind
                                   * direction values, but correspond to the
                                   * time when the max. wind speed values
                                   * were found per forecast period.  These
                                   * values are used in deriving the weather
                                   * and/or icon values. */
   double *valTimeForWindDirMatch = NULL; /* Array with the validTimes that
                                           * corresponds to the times when
                                           * the max wind speeds are the
                                           * highest.  We then collect the
                                           * wind directions that correspond
                                           * to the same times when the wind
                                           * speeds are the highest. */
   int *maxSkyCover = NULL;   /* Array containing the maximum Sky Cover
                               * values corresponding to a day (24 hour
                               * format) or 12 hour period (12 hour format).
                               * These values are used in deriving the
                               * weather and/or icon values. */
   int *minSkyCover = NULL;   /* Array containing the minimum Sky Cover
                               * values corresponding to a day (24 hour
                               * format) or 12 hour period (12 hour format).
                               * These values are used in deriving the
                               * weather and/or icon values. */
   int *averageSkyCover = NULL; /* Array containing the average Sky Cover
                                 * values corresponding to a day (24 hour
                                 * format) or 12 hour period (12 hour
                                 * format).  These values are used in
                                 * deriving the weather and/or icon values. */
   int *maxSkyNum = NULL; /* Records the index in the match structure where the 
			   * max sky cover was found. Used to determine sky 
			   * cover trends (i.e. increasing clouds) for 
			   * DWMLgenByDay products. */
   int *minSkyNum = NULL; /* Records the index in the match structure where the 
			   * min sky cover was found. Used to determine sky 
			   * cover trends (i.e. increasing clouds) for 
			   * DWMLgenByDay products. */
   int *startPositions = NULL; /* The index of where the current forecast period
			        * begins.  Used to determine sky cover trends 
			        * (i.e. increasing clouds) for DWMLgenByDay 
			        * products. */
   int *endPositions = NULL; /* The index of where the current forecast period
			      * ends.  Used to determine sky cover trends 
			      * (i.e. increasing clouds) for DWMLgenByDay 
			      * products. */
   layouts dummyTimeLayout;   /* Dummy argument for call to isNewLayout() to
                               * free up static array
                               * "timeLayoutDefinitions". */

   /* XML Document pointer */

   xmlDocPtr doc = NULL;      /* An xml Node Pointer denoting the top-level
                               * document node. */

   /* XML Node pointers */

   xmlNodePtr dwml = NULL;    /* An xml Node Pointer denoting the <dwml>
                               * node. */
   xmlNodePtr data = NULL;    /* An xml Node Pointer denoting the <data>
                               * node. */

   /* Set up the varFilter array to show what NDFD variables are of
    * interest(1) or vital(2) to this procedure. */
   memset(varFilter, 0, NDFD_MATCHALL + 1);

   varFilter[NDFD_MAX] = 1;
   varFilter[NDFD_MIN] = 1;
   varFilter[NDFD_POP] = 1;
   varFilter[NDFD_TEMP] = 1;
   varFilter[NDFD_WD] = 1;
   varFilter[NDFD_WS] = 1;
   varFilter[NDFD_TD] = 1;
   varFilter[NDFD_SKY] = 1;
   varFilter[NDFD_QPF] = 1;
   varFilter[NDFD_SNOW] = 1;
   varFilter[NDFD_WX] = 1;
   varFilter[NDFD_WH] = 1;
   varFilter[NDFD_AT] = 1;
   varFilter[NDFD_RH] = 1;

   /* Force genprobe() to return required NDFD element(s). */
   if (f_XML == 2)
   {
      varFilter[NDFD_MAX] = 2;
      varFilter[NDFD_MIN] = 2;
      varFilter[NDFD_TEMP] = 2;
      varFilter[NDFD_WS] = 2;
      varFilter[NDFD_SKY] = 2;
      varFilter[NDFD_WX] = 2;
   }
   else if (f_XML == 1 && f_icon == 1)
   {
      varFilter[NDFD_TEMP] = 2;
      varFilter[NDFD_WS] = 2;
      varFilter[NDFD_SKY] = 2;
      varFilter[NDFD_WX] = 2;
   }
   else if (f_XML == 3 || f_XML == 4)
   {
      varFilter[NDFD_MAX] = 2;
      varFilter[NDFD_MIN] = 2;
      varFilter[NDFD_POP] = 2;
      varFilter[NDFD_WD] = 2;
      varFilter[NDFD_WS] = 2;
      varFilter[NDFD_SKY] = 2;
      varFilter[NDFD_WX] = 2;
   }

   /* Allow user filter of variables to reduce the element list, but include
    * all "vital" (Filter == 2) variables. */
   genElemListInit2(varFilter, numNdfdVars, ndfdVars, &numElem, &elem);

   /* If product is of DWMLgen's time-series, we need to decipher between
    * elements originating on the command line argument and those forced if
    * command line argument -Icon is set to 1 (turned on). */
   if (f_XML == 1 && f_icon == 1)
   {
      varFilter[NDFD_TEMP]--;
      varFilter[NDFD_WS]--;
      varFilter[NDFD_SKY]--;
      varFilter[NDFD_WX]--;
   }

   /* We need to turn the f_icon flag on if all elements are to be formatted
    * by default (when there is no -ndfdfVars command option) AND there was
    * no forcing of elements.  This will ensure Icons are formatted along
    * with all the NDFD elements. */
   if (f_XML == 1 && numNdfdVars == 0 && f_icon == 0)
      f_icon = 1;

   /* Force Icons to be formatted if DWMLgen "glance" product. */
   if (f_XML == 2)
      f_icon = 1;

   /* See if any points are outside the NDFD Sectors. Print error message to
    * standard error if so. If all points selected are outside the NDFD
    * Sectors, exit routine. */
   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
         f_atLeastOnePointInSectors = 1;
      else
         printf("Point #%d is outside of all NDFD Sectors.\n", j + 1);
   }
   if (f_atLeastOnePointInSectors != 1)
   {
      printf("************************************************************\n");
      printf("No point(s) selected are inside the NDFD Sectors. Exiting...\n");
      printf("************************************************************\n");
      return 0;
   }

   /* f_WxParse = 0, is the flag to return WX as ugly weather codes. */

   f_WxParse = 0;

   if (genProbe(numPnts, pnts, f_pntType, numInFiles, inFiles, f_fileType,
                f_interp, f_unit, majEarth, minEarth, f_WxParse,
                f_SimpleVer, numElem, elem, f_valTime, startTime, endTime,
                &numMatch, &match, f_inTypes, gribFilter, numSector,
                sector, f_ndfdConven) != 0)
   {
      for (i = 0; i < numElem; i++)
      {
         genElemFree(elem + i);
      }
      free(elem);
      for (i = 0; i < numMatch; i++)
      {
         genMatchFree(match + i);
      }
      free(match);
      return -1;
   }

   /* If no data is retrived from NDFD (matches = zero), get out. */
   if (numMatch <= 0)
   {
      printf("No data retrieved from NDFD (matches = 0).\n");
      return 0;
   }

   /* Sort the matches by element and then by valid time within each element. 
    */
   qsort(match, numMatch, sizeof(match[0]), matchCompare);

   /* Allocate f_pntNoData. */
   f_pntHasData = calloc(numPnts, sizeof(char));

   /* Colate the matches.     */
   curTime = -1;
   for (i = 0; i < numMatch; i++)
   {
      if (curTime != match[i].validTime)
      {
         j = numCollate;
         numCollate++;
         collate = realloc(collate, numCollate * sizeof(collateType));
         for (k = 0; k < NDFD_MATCHALL + 1; k++)
         {
            collate[numCollate - 1].allElem[k] = -1;
         }
         collate[numCollate - 1].validTime = match[i].validTime;
         curTime = match[i].validTime;
      }

      myAssert(numCollate > 0);
      collate[numCollate - 1].allElem[match[i].elem.ndfdEnum] = i;
      /* update f_pntHasData based on this "match". */
      for (j = 0; j < numPnts; j++)
      {
         if (match[i].value[j].valueType != 2)
            f_pntHasData[j] = 1;
      }
   }
   
   /* Get the first Valid times for MaxT, MinT, and Pop. */
   for (i = 0; i < numMatch; i++)
   { 
      if (match[i].elem.ndfdEnum == NDFD_MAX)
      {
         firstValidTime_maxt = match[i].validTime;
         break;
      }
   }
   for (i = 0; i < numMatch; i++)
   { 
      if (match[i].elem.ndfdEnum == NDFD_MIN)
      {
         firstValidTime_mint = match[i].validTime;
         break;
      }
   }
   for (i = 0; i < numMatch; i++)
   { 
      if (match[i].elem.ndfdEnum == NDFD_POP)
      {
         firstValidTime_pop = match[i].validTime;
         break;
      }
   }

#ifdef PRINT

   /* Loop by point to check if any data at point at all. */
   for (j = 0; j < numPnts; j++)
   {
      /* Check if this point has any data at all */
      if (isPntInASector(pnts[j]))
      {
         if (f_pntHasData[j])
         {
            PrintSameDay1(match, j, collate, numCollate, pntInfo[j].timeZone,
                          pntInfo[j].f_dayLight);
         }
         printf("-----------------\n");
      }
   }
#endif

   /* Get the current system time, in double form. */
   currentDoubTime = Clock_Seconds();

   /*************************************************************************** 
    *                   Start formatting the XML.                             *
    *                                    
    ***********************  HEADER INFO  *************************************/

   /* Create the XML document and format the Meta Data before looping through
    * points. */

   formatMetaDWML(f_XML, &doc, &data, &dwml);

   /* Initialize the weatherParameters array. This denotes those elements
    * that will ultimately be formatted (= 1). Those retrieved from NDFD by
    * degrib but used only to derive other elements are set to = 2. */

   for (i = 0; i < NDFD_MATCHALL + 1; i++)
      weatherParameters[i] = 0;

   /* Prepare some data for DWMLgen's "time-series" product. */

   if ((f_XML == 1) || (f_XML == 2))
      prepareDWMLgen(f_XML, &f_formatPeriodName, &(weatherParameters[0]),
                     whatSummarization, varFilter, &f_icon);

   /* Prepare data for DWMLgenByDay's "12 hourly" & "24 hourly" products. */

   if ((f_XML == 3) || (f_XML == 4))
      prepareDWMLgenByDay(match, numMatch, f_XML, &startTime, &endTime, 
		          &firstValidTimeMatch, &lastValidTimeMatch, &numDays, 
			  format, &f_formatPeriodName, &(weatherParameters[0]), 
			  &timeInterval, &numOutputLines, whatSummarization,
			  currentDoubTime);

   /*************************  LOCATION INFO  ******************************/

   /* Format the Location Information into the XML/DWML. */
   formatLocationInfo(numPnts, pnts, data);

   /* Run through matches to find numRows returned per element for each
    * point. Since this is based off of what was actually returned from NDFD,
    * update the weatherParameters array accordingly to reflect what is
    * actually returned from the NDFD. */

   numRowsForPoint = (uChar **) malloc(numPnts * sizeof(uChar **));
   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
      {
         numRowsForPoint[j] = (uChar *) malloc((NDFD_MATCHALL + 1) *
                                               sizeof(uChar));
         getNumRows(numRowsForPoint[j], numMatch, match, 
	            numDays, &(weatherParameters[0]), f_XML, &f_icon);
      }

   }

#ifdef PRINT
   for (i = 0; i < NDFD_MATCHALL + 1; i++)
      printf("weatherParameters 2 [%d] = %d\n", i, weatherParameters[i]);
   for (j = 0; j < numPnts; j++)
   {
      for (i = 0; i < NDFD_MATCHALL + 1; i++)
      {
         if (isPntInASector(pnts[j]))
            printf("numRowsForPoint[%d][%d] = %d\n",j, i, numRowsForPoint[j][i]);
      }
   }
#endif
   
   /***********************  TIME LAYOUT INFO  *****************************/

   /* Allocate the time zone information, startDates, and the layoutKeys to
    * the number of points. The layout-keys will only be generated once,
    * UNLESS, a point is chosen in a different time zone (which will alter
    * the time-layout info). */

   layoutKeys = (char ***)malloc(numPnts * sizeof(char **));
   TZoffset = (sChar *) malloc(numPnts * sizeof(sChar));
   startDate = (char **)malloc(numPnts * sizeof(char *));

   /* Begin point loop for time-layout generation. */
   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
      {   
         TZoffset[j] = pntInfo[j].timeZone;
         f_observeDST = pntInfo[j].f_dayLight;

         /* If product is of type DWMLgenByDay, find startDates (which depend
          * on point). */

         if (f_XML == 3 || f_XML == 4) 
	 {
            formatValidTime(firstValidTimeMatch, startDateBuff, 30,
                            TZoffset[j], f_observeDST);
	    
	    /* We may have to alter startDate if the hour is late in the day 
	     * and the first forecast period (of which startDate is used in
	     * formulating) is in the next day.
	     */
	    startDateHr[0] = startDateBuff[11];
	    startDateHr[1] = startDateBuff[12];
	    startDateHr[2] = '\0';
	    
            if ((f_XML == 4 && atoi(startDateHr) > 21) || 
	        ((f_XML == 3 && atoi(startDateHr) > 18 && firstValidTimeMatch != firstValidTime_maxt)))
	    {
	       firstValidTimeMatch = firstValidTimeMatch + (3600*24);
               startDateBuff[0] = '\0';	       
               formatValidTime(firstValidTimeMatch, startDateBuff, 30,
                               TZoffset[j], f_observeDST);
	    }
	    
            pstr = strchr(startDateBuff, 'T');
            startDate[j] = (char *)calloc((pstr - startDateBuff) + 1,
                                          sizeof(char));
            strncpy(startDate[j], startDateBuff, pstr - startDateBuff);
	    
         }
         else
         {
            /* If the products of type DWMLgen, simply set startDate[j]
             * to = NULL. */
            startDate[j] = (char *)calloc(1, sizeof(char));
            startDate[j][0] = '\0';
         }

         /* Convert the system time to a formatted local time
          * (i.e. 2006-02-02T17:00:00). 
	  */
         Clock_Print2(currentLocalTime, 30, currentDoubTime,
                      "%Y-%m-%dT%H:%M:%S", TZoffset[j], 1);

         /* If the startTime entered on the command line is before the
          * firstValidTimeMatch, treat it as if there was no startTime
          * entered on the command line, since it didn't effectively shorten
          * the interval of time data was retrieved for. */
         if (startTime < currentDoubTime)
            startTime = 0.0;

         /* Now get the current day's date and hour. */

         currentDay[0] = currentLocalTime[8];
         currentDay[1] = currentLocalTime[9];
         currentDay[2] = '\0';

         currentHour[0] = currentLocalTime[11];
         currentHour[1] = currentLocalTime[12];
         currentHour[2] = '\0';

         /* Check to see if points are in different time zones. We need to
          * compare current points' time zone to all prior * valid points'
          * time zones. */
         if (j > 0)
         {
            for (i = j - 1; i >= 0; i--)
            {
               if (isPntInASector(pnts[i]))
               {
                  if (TZoffset[j] != TZoffset[i])
                  {
                     f_formatNewPointTimeLayouts = 1;
                     break;
                  }
               }     
            }
         }
         layoutKeys[j] = (char **)malloc((NDFD_MATCHALL + 1) * sizeof(char *));

         if (f_firstPointLoopIteration || f_formatNewPointTimeLayouts)
         {
            /* Generate a new set of time-layouts for the point. */

            for (k = 0; k < (NDFD_MATCHALL + 1); k++)
            {
               if (weatherParameters[k] == 1 || weatherParameters[k] == 3)
               {

                  /* For DWMLgen products' "time-series" and "glance". */

                  if (f_XML == 1 || f_XML == 2)
                  {
                     generateTimeLayout(numRowsForPoint[j][k], k, layoutKey,
                                        whichTimeCoordinate,
                                        whatSummarization, match, numMatch,
                                        f_formatPeriodName, TZoffset[j],
                                        f_observeDST, &numLayoutSoFar,
                                        &numCurrentLayout, currentHour,
                                        currentDay, "boggus", data, startTime,
                                        currentDoubTime, &numDays, f_XML, 
					numPopRowsSkippedBeginning);

                     layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                     strcpy(layoutKeys[j][k], layoutKey);

                  }
                  else if (f_XML == 3)
                  {
                     /* For DWMLgenByDay product w/ format == "12 hourly". */

                     if (k == NDFD_MAX || k == NDFD_MIN)
                     {
                        generateTimeLayout(numRowsForPoint[j][k], k, layoutKey,
                                           whichTimeCoordinate,
                                           whatSummarization, match,
                                           numMatch, f_formatPeriodName,
                                           TZoffset[j], f_observeDST,
                                           &numLayoutSoFar, &numCurrentLayout,
                                           currentHour, currentDay, format,
                                           data, startTime, currentDoubTime, 
					   &numDays, f_XML, 
					   numPopRowsSkippedBeginning);
			
                        layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                        strcpy(layoutKeys[j][k], layoutKey);
                     }
                     else
                     {
                        /* The other element's will share Pop's layout. */
                        generateTimeLayout(numRowsForPoint[j][NDFD_POP],
				           NDFD_POP, layoutKey,
                                           whichTimeCoordinate,
                                           whatSummarization, match,
                                           numMatch, f_formatPeriodName,
                                           TZoffset[j], f_observeDST,
                                           &numLayoutSoFar, &numCurrentLayout, 
                                           currentHour, currentDay, format, 
                                           data, startTime, currentDoubTime, 
					   &numOutputLines, f_XML, 
					   numPopRowsSkippedBeginning);

                        layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                        strcpy(layoutKeys[j][k], layoutKey);
                     }
                  }
                  else if (f_XML == 4)
                  {
                     /* For DWMLgenByDay product w/ format == "24 hourly".
                      * Since the product is DWMLgenByDay's "24 hourly"
                      * format, force the weather (and icon) elements so that
                      * their time layout equals that of MaxT and/or MinT
                      * since their periods are = to 24 hours also (the
                      * exception is Pop, since it still has to use a 12
                      * hourly summariztion). 
		      */
                     if (k != NDFD_POP)
                     {

                        /* If the request for MaxT's is late in the day, then
                         * we will format "nil" for first MaxT data value and
                         * simply use the MinT's time layout. Retrieve the
                         * necessary info for this check. 
			 */
                        monthDayYearTime(match, numMatch, currentLocalTime,
                                         currentDay, f_observeDST,
                                         &firstMaxTValidTime_doub_adj,
                                         &currentLocalTime_doub_adj, 
					 TZoffset[j]);

                        if (atoi(currentHour) > 18 &&
                            (currentLocalTime_doub_adj + 86400) ==
                            firstMaxTValidTime_doub_adj)
                        {
                           f_formatNIL = 1;
                           f_useMinTempTimes = 1;

                           generateTimeLayout(numRowsForPoint[j][NDFD_MIN],
					      NDFD_MIN, layoutKey,
                                              whichTimeCoordinate,
                                              whatSummarization, match,
                                              numMatch, f_formatPeriodName,
                                              TZoffset[j], f_observeDST,
                                              &numLayoutSoFar, &numCurrentLayout,
                                              currentHour, currentDay,
                                              format, data, startTime,
                                              currentDoubTime, &numDays, f_XML, 
					      numPopRowsSkippedBeginning);

                           layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                           strcpy(layoutKeys[j][k], layoutKey);
                        }
                        else  /* Use MaxT's own time-layout. */
                        {
                           f_formatNIL = 0;
                           f_useMinTempTimes = 0;

                           generateTimeLayout(numRowsForPoint[j][NDFD_MAX],
					      NDFD_MAX, layoutKey,
                                              whichTimeCoordinate,
                                              whatSummarization, match,
                                              numMatch, f_formatPeriodName,
                                              TZoffset[j], f_observeDST,
                                              &numLayoutSoFar,
                                              &numCurrentLayout,
                                              currentHour, currentDay,
                                              format, data, startTime,
                                              currentDoubTime, &numDays, f_XML, 
					      numPopRowsSkippedBeginning);

                           layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                           strcpy(layoutKeys[j][k], layoutKey);
                        }

                     }
                     else
                     {
                              /* POP gets its own time-layout, using
                               * 12-hourly summarization and format (force
                               * these). */
			numPopLines = numDays*2;
                        generateTimeLayout(numRowsForPoint[j][k], 
					   NDFD_POP, layoutKey, 
					   whichTimeCoordinate, "12hourly",
                                           match, numMatch,
                                           f_formatPeriodName, TZoffset[j],
                                           f_observeDST, &numLayoutSoFar,
                                           &numCurrentLayout, currentHour,
                                           currentDay, "12 hourly", data,
                                           startTime, currentDoubTime, 
					   &numPopLines, f_XML, 
					   numPopRowsSkippedBeginning);

                        layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                        strcpy(layoutKeys[j][k], layoutKey);
                     }

                  }           /* End of "f_XML type" check. */
               }              /* End weatherParameters "if" statement. */
            }                 /* End weatherParameters "k" loop. */
         }
         else
         {
            /* Simply copy the previous valid point's time layout-keys into
             * this point's time layout-keys. */
            for (i = j - 1; i >= 0; i--)
            {
               if (isPntInASector(pnts[i]))
               {
                  for (k = 0; k < (NDFD_MATCHALL + 1); k++)
                  {
                     if (weatherParameters[k] == 1)
                     {
                        layoutKeys[j][k] = malloc(strlen(layoutKeys[i][k]) + 1);
                        strcpy(layoutKeys[j][k], layoutKeys[i][k]);
                     }
                  }
               }
            }
         }  /* End of "if-else" of "format New Point time layout" check. */
	 f_firstPointLoopIteration = 0;
	 
      }  /* End of "is Point in Sector" check. */
   }  /* End "Point Loop" for Time-Layouts. */
   
   /***********************  PARAMETER INFO  *******************************/

   /* Format the Parameter Information into the XML/DWML. */
   /* Begin new Point Loop to format the Data/Weather Parameter values. */
   for (j = 0; j < numPnts; j++)
   {
      /* Check to make sure point is within the NDFD sectors. */
      if (isPntInASector(pnts[j]))
      {
         parameters = xmlNewChild(data, NULL, BAD_CAST "parameters", NULL);
         sprintf(pointBuff, "point%d", (j + 1));
         xmlNewProp(parameters, BAD_CAST "applicable-location", BAD_CAST
                    pointBuff);

         /* Format Maximum Temperature Values, if applicable. */
         if (weatherParameters[NDFD_MAX] == 1)
            genMaxTempValues(j, layoutKeys[j][NDFD_MAX], match, numMatch,
                             parameters, &f_formatNIL, f_XML, startTime,
                             numRowsForPoint[j][NDFD_MAX], numDays);

         /* Format Minimum Temperature Values, if applicable. */
         if (weatherParameters[NDFD_MIN] == 1)
            genMinTempValues(j, layoutKeys[j][NDFD_MIN], match, numMatch,
                             parameters, f_XML, startTime, 
			     numRowsForPoint[j][NDFD_MIN], currentDay, 
			     currentHour, TZoffset[j], pntInfo[j].f_dayLight, 
			     numDays);

         /* Format Hourly Temperature Values, if applicable. */
         if (weatherParameters[NDFD_TEMP] == 1)
         {
            genTempValues(j, layoutKeys[j][NDFD_TEMP], match, numMatch,
                          parameters);
         }
         /* Format Dew Point Temperature Values, if applicable. */
         if (weatherParameters[NDFD_TD] == 1)
            genDewPointTempValues(j, layoutKeys[j][NDFD_TD], match,
                                  numMatch, parameters);

         /* Format Apparent Temperature Values, if applicable. */
         if (weatherParameters[NDFD_AT] == 1)
            genAppTempValues(j, layoutKeys[j][NDFD_AT], match, numMatch,
                             parameters);

         /* Format QPF Values, if applicable. */
         if (weatherParameters[NDFD_QPF] == 1)
            genQPFValues(j, layoutKeys[j][NDFD_QPF], match, numMatch,
                         parameters);

         /* Format Snow Amount Values, if applicable. */
         if (weatherParameters[NDFD_SNOW] == 1)
            genSnowValues(j, layoutKeys[j][NDFD_SNOW], match, numMatch,
                          parameters);

         /* Format PoP12 Values, if applicable. */
         if (weatherParameters[NDFD_POP] == 1)
         {
            /* If product is of DWMLgenByDay type, allocate maxDailyPop array
             * and initialize it. */

            if (f_XML == 3)
            {
               maxDailyPop = malloc((numDays*2) * sizeof(int));
               for (i = 0; i < numDays*2; i++)
                  maxDailyPop[i] = 0;
	    }
	    if (f_XML == 4)
            {
               maxDailyPop = malloc((numDays) * sizeof(int));
               for (i = 0; i < numDays; i++)
                  maxDailyPop[i] = 0;		  
	    }
	    
            genPopValues(j, layoutKeys[j][NDFD_POP], match, numMatch,
                         parameters, numRowsForPoint[j][NDFD_POP], f_XML,
                         startTime, maxDailyPop, numDays, currentDoubTime,
			 currentHour, numPopRowsSkippedBeginning);
	 }
	 
         /* Format Wind Speed Values for DWMLgen products, if applicable.
          * Collect Max Wind Speed values if product is of type DWMLgenByDay. 
          */
         if (weatherParameters[NDFD_WS] == 1 || weatherParameters[NDFD_WS] == 2)
         {
            /* If product is of DWMLgenByDay type, allocate maxWindSpeed
             * array. We need the max wind speed values for each forecast
             * period to derive the weather and icon elements.  Also,
             * allocate the array holding the valid times that correspond to
             * the max wind speeds. These times will be used to collect the
             * wind directions that correspond to the times when the max.
             * wind speeds occurred. */

            if (f_XML == 3 || f_XML == 4)
            {
               maxWindSpeed = malloc((numOutputLines) * sizeof(int));
               valTimeForWindDirMatch = malloc((numOutputLines) * sizeof(double));
               for (i = 0; i < (numOutputLines); i++)
               {
                  maxWindSpeed[i] = -999;
                  valTimeForWindDirMatch[i] = -999;
               }
            }

            genWindSpeedValues(j, layoutKeys[j][NDFD_WS], match, numMatch,
                               parameters, startDate[j], maxWindSpeed,
                               numOutputLines, timeInterval, TZoffset[j],
                               pntInfo[j].f_dayLight, NDFD_WS,
                               numRowsForPoint[j][NDFD_WS], f_XML,
                               valTimeForWindDirMatch);
         }

         /* Format Wind Direction values for DWMLgen products, if applicable.
          * Collect the Wind Direction values that correspond to the times
          * when the maximum Wind Speeds existed if product is of type
          * DWMLgenByDay. */
         if (weatherParameters[NDFD_WD] == 1 || weatherParameters[NDFD_WD] == 2)
         {
		 
            /* If product is of DWMLgenByDay type, allocate maxWindDirection
             * array and initialize. We need these wind direction values for
             * each forecast period to derive the weather and icon elements. */
            if (f_XML == 3 || f_XML == 4)
            {
               maxWindDirection = malloc((numOutputLines) * sizeof(int));
               for (i = 0; i < (numOutputLines); i++)
                  maxWindDirection[i] = -999;
            }
	    
            genWindDirectionValues(j, layoutKeys[j][NDFD_WD], match,
                                   numMatch, parameters, maxWindDirection,
                                   f_XML, numOutputLines,
				   valTimeForWindDirMatch);
         }
	 
         /* Format Sky Cover Values for DWMLgen products, if applicable.
          * Collect Max and Min Sky Cover values for icon determination if
          * product is of type DWMLgenByDay. */
         if (weatherParameters[NDFD_SKY] == 1 || weatherParameters[NDFD_SKY]
             == 2)
         {

            /* If product is of DWMLgenByDay type, allocate the maxSkyCover,
             * minSkyCover, minSkyNum, maxSkyNum, startPositions, endPositions,
	     * and averageSkyCover arrays and initialize them. We need these 
	     * sky values for each forecast period to derive the weather and 
	     * icon elements. */

            if (f_XML == 3 || f_XML == 4)
            {
	       startPositions = malloc(numOutputLines * sizeof(int));
	       endPositions = malloc(numOutputLines * sizeof(int));
               maxSkyCover = malloc(numOutputLines * sizeof(int));
               minSkyCover = malloc(numOutputLines * sizeof(int));
	       maxSkyNum = malloc(numOutputLines * sizeof(int));
               minSkyNum = malloc(numOutputLines * sizeof(int));
               averageSkyCover = malloc(numOutputLines * sizeof(int));
               
               for (i = 0; i < numOutputLines; i++)
               {
		  maxSkyCover[i]    = -999;
                  startPositions[i] = -999;
                  endPositions[i]   = -999;
                  minSkyNum[i]      = -999;
                  maxSkyNum[i]      = +999;  /* Note (+) initialization. */
                  minSkyCover[i]    = +999;  /* Note (+) initialization. */
               }
            }

            genSkyCoverValues(j, layoutKeys[j][NDFD_SKY], match, numMatch,
                              parameters, startDate[j], maxSkyCover,
                              minSkyCover, averageSkyCover, numOutputLines,
                              timeInterval, TZoffset[j], pntInfo[j].f_dayLight,
                              NDFD_SKY, numRowsForPoint[j][NDFD_SKY], f_XML, 
			      maxSkyNum, minSkyNum, startPositions,  
			      endPositions, &integerStartUserTime, 
			      &integerTime, currentHour);
         }

         /* Format Relative Humidity Values, if applicable. */
         if (weatherParameters[NDFD_RH] == 1)
            genRelHumidityValues(j, layoutKeys[j][NDFD_RH], match, numMatch,
                                 parameters);

         /* Format Weather Values and\or Icons, if applicable. We must have
          * at least some rows of weather data to format either. */

         if (numRowsForPoint[j][NDFD_WX] > 0)
         {
            if (f_XML == 1 || f_XML == 2)
               genWeatherValues(j, layoutKeys[j][NDFD_WX], match, numMatch,
                                weatherParameters[NDFD_WX], f_icon,
                                numRowsForPoint[j][NDFD_WS],
                                numRowsForPoint[j][NDFD_SKY],
                                numRowsForPoint[j][NDFD_TEMP],
                                numRowsForPoint[j][NDFD_WX], parameters,
                                pnts[j].Y, pnts[j].X);
	    else
	       genWeatherValuesByDay(j, layoutKeys[j][NDFD_WX], match, numMatch,
                                weatherParameters[NDFD_WX],
                                numRowsForPoint[j][NDFD_WS],
                                numRowsForPoint[j][NDFD_SKY],
				numRowsForPoint[j][NDFD_POP],
				numRowsForPoint[j][NDFD_MAX],
				numRowsForPoint[j][NDFD_MIN],
                                numRowsForPoint[j][NDFD_TEMP],
                                numRowsForPoint[j][NDFD_WX], parameters,
                                pnts[j].Y, pnts[j].X, numDays, TZoffset[j],
                                pntInfo[j].f_dayLight, format, 
				f_useMinTempTimes, f_XML, numOutputLines,
				maxDailyPop, averageSkyCover, maxSkyCover,
				minSkyCover, maxSkyNum, minSkyNum, 
				startPositions, endPositions, maxWindSpeed,
				maxWindDirection, integerTime, 
				integerStartUserTime, startTime);
         }

         /* Format Wave Height Values, if applicable. */

         if (weatherParameters[NDFD_WH] == 1)
            genWaveHeightValues(j, layoutKeys[j][NDFD_WH], match, numMatch,
                                parameters);
	 
         /* Free some things before leaving this iteration of the point loop. */
         if (f_XML == 3 || f_XML == 4)
         {
            free(maxDailyPop);
            free(startPositions);
            free(endPositions);
            free(maxSkyNum);
            free(minSkyNum);     
            free(maxSkyCover);
            free(minSkyCover);
            free(averageSkyCover);
	    free(valTimeForWindDirMatch);
	    free(maxWindDirection);
	    free(maxWindSpeed);
	 }

	 free(numRowsForPoint[j]);
         free(startDate[j]);
	 

         for (k = 0; k < NDFD_MATCHALL + 1; k++)
         {
            if (weatherParameters[k] == 1 || weatherParameters[k] == 3)
               free(layoutKeys[j][k]);
         }


      }                       /* End of "is Point in Sectors" check. */

   }                          /* Close Parameters Point Loop. */

   /* Free layoutKeys array. */
   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
         free(layoutKeys[j]);
   }

   /* Free the static array "timeLayoutDefinitions" by destroying it. */
   dummyTimeLayout.period = 0;
   dummyTimeLayout.numRows = 0;
   dummyTimeLayout.fmtdStartTime[0] = '\0';
   isNewLayout(dummyTimeLayout, 0, 0, 1);

   /* Append <data> node to XML document. */

   xmlAddChild(dwml, data);

   /* Dump XML document to file or stdio (use "-" for stdio). */

   xmlSaveFormatFile("-", doc, 1);

   /* Free the document. */

   xmlFreeDoc(doc);

   /* Free the global variables that may have been allocated by parser. */

   xmlCleanupParser();

   /* This is to debug memory for regression tests. */

   xmlMemoryDump();

   /* Free some more memory. */

   free(layoutKeys);
   free(numRowsForPoint);
   free(TZoffset);
   free(startDate);

   /* Free even some more memory. */

   free(f_pntHasData);
   free(collate);

   for (i = 0; i < numElem; i++)
   {
      genElemFree(elem + i);
   }
   free(elem);

   for (i = 0; i < numMatch; i++)
   {
      genMatchFree(match + i);
   }
   free(match);

   return 0;
}

#ifdef TEST_XML
int main(int argc, char **argv)
{
   int numInFiles;
   char **inFiles;
   int f_XML = 1;             /* version of xml... 1 = DWMLgen's
                               * "time-series" */

   numInFiles = 1;
   inFiles = (char **)malloc(sizeof(char *));
   myAssert(sizeof(char) == 1);
   inFiles[0] = (char *)malloc(strlen("maxt.bin") + 1);
   strcpy(inFiles[0], "maxt.bint");
}
#endif
