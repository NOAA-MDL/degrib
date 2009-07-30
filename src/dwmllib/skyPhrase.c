/*****************************************************************************
 * skyPhrase() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   This code determines the sky phrase that should be used and then assigns 
 *   an icon to correspond to it.  It follows the algorithm developed by Mark 
 *   Mitchell for use in the 12 hourly summarization (f_XML = 3) and 24 hourly
 *   summarization (f_XML = 4) products on the NWS web site.
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
 *     startPositions = The index of where the current forecast period begins.  Used
 *                      to determine sky cover trends (i.e. increasing clouds) for 
 *                      DWMLgenByDay products. (Input)
 *       endPositions = The index of where the current forecast period ends.  Used
 *                      to determine sky cover trends (i.e. increasing clouds) for 
 *                      DWMLgenByDay products. (Input)	
 *        f_isDayTime = Flag denoting if period is in day time. (Input)
 *      f_isNightTime = Flag denoting if period is in night time. (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 * 10/2007 Paul Hershberg (MDL): Fixed bug in which tread speeds were set to 
 *                               a 4, which erroneously was not denoting 4 hours
 *                               but 4 index or category differences. We changed 
 *                               the 4 to 1 index or category difference, which
 *                               signifies a 3 hour difference.
 *  7/2009 Paul Hershberg (MDL): Changed day time error of "Partly Cloudy" to 
 *                               "Partly Sunny".
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void skyPhrase(int *maxSkyCover, int *minSkyCover, int *averageSkyCover, 
	       int dayIndex, int f_isDayTime, int f_isNightTime, int *maxSkyNum,  
	       int *minSkyNum, int *startPositions, int *endPositions,
               char *baseURL, icon_def *iconInfo, char **phrase)
{
   int i; /* Loop counter. */
   int categoryChange = 0; /* The difference between the category of the max and 
			    * min sky cover. */
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
   int trend_speed; /* Indicates how much the sky cover change over the period 
		     * being processed. */
   int trend_dec_late; /* Used to detect clouds that are decreasing late in the 
			* period. */
   int trend_dec_early; /* Used to detect clouds that are decreasing early in 
			 * the period. */
   double catDiff = 0.0; /* Difference between max and min categories / 2. Used
			  * for input to myRound(). */

   /* The image arrays. */
   char *daySkyImage[5]; /* Array containing the daylight name of the image used 
			  * at for each sky cover category (i.e. skc.jpg). */
   char *nightSkyImage[5]; /* Array containing the name of the image used 
			    * at night for each sky cover category 
			    * (i.e. nskc.jpg). */

   /* The phrase arrays. */
   char *daySkyPhrase[5]; /* Array containing the word associated with a 
			   * particular sky cover category used during the 
			   * day (i.e. sunny). */
   char *nightSkyPhrase[5]; /* Array containing the word associated with a 
			     * particular sky cover category used during the 
			     * night (i.e. clear). */  
   
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
   
   /* Initialize the 5 element daySkyPhrase array. */
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
    * (skyChange never used in JohnS's code??). */

   /* Deterimine Maximum Cloud Categories. */
   if (maxSkyCover[dayIndex] <= 5)
      maxCategory = 0;
   else if (maxSkyCover[dayIndex] <= 25)
      maxCategory = 1;
   else if (maxSkyCover[dayIndex] <= 50)
      maxCategory = 2;
   else if (maxSkyCover[dayIndex] <= 87)
      maxCategory = 3;
   else if (maxSkyCover[dayIndex] <= 101)
      maxCategory = 4;

   /* Deterimine Minimum Cloud Categories. */
   if (minSkyCover[dayIndex] <= 5)
      minCategory = 0;
   else if (minSkyCover[dayIndex] <= 25)
      minCategory = 1;
   else if (minSkyCover[dayIndex] <= 50)
      minCategory = 2;
   else if (minSkyCover[dayIndex] <= 87)
      minCategory = 3;
   else if (minSkyCover[dayIndex] <= 101)
      minCategory = 4;

   /* Calculate the change in cloud categories and the average cloud amount
    * for this day.
    */
   categoryChange = abs(maxCategory - minCategory);
   catDiff = (double)((maxCategory + minCategory) / 2.0);
   avgCategory = (int)myRound(catDiff, 0);
   
   if ((dayIndex > skyTrend_Periods) || (categoryChange < 2))
   {
      if ((averageSkyCover[dayIndex] <= 5) && f_isDayTime)
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "skc.jpg");        
	  strcpy (phrase[dayIndex], "Sunny");
      }
      else if ((averageSkyCover[dayIndex] <= 5) && f_isNightTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nskc.jpg");        
	  strcpy (phrase[dayIndex], "Clear");
      }
      else if ((averageSkyCover[dayIndex] <= 25) && f_isDayTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "few.jpg");        
	  strcpy (phrase[dayIndex], "Mostly Sunny");
      }
      else if ((averageSkyCover[dayIndex] <= 25) && f_isNightTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nfew.jpg");        
	  strcpy (phrase[dayIndex], "Mostly Clear");
      }
      else if ((averageSkyCover[dayIndex] <= 50) && f_isDayTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "sct.jpg");        
          strcpy (phrase[dayIndex], "Partly Sunny");
      }
      else if ((averageSkyCover[dayIndex] <= 50) && f_isNightTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "nsct.jpg");        
	  strcpy (phrase[dayIndex], "Partly Cloudy");
      }
      else if ((averageSkyCover[dayIndex] <= 87) && f_isDayTime)     
      {
          sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, "bkn.jpg");        
	  strcpy (phrase[dayIndex], "Mostly Cloudy");
      }
      else if ((averageSkyCover[dayIndex] <= 87) && f_isNightTime)     
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
	 
         /* Clouds increasing over a duration of 3 or more hours (1 index or 
          * category diff). 
          */
	 if (trend_speed >= 1)
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
	       if ((averageSkyCover[dayIndex] <= 5) && f_isDayTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "skc.jpg");
	          strcpy (phrase[dayIndex], "Sunny");
	       }
	       else if ((averageSkyCover[dayIndex] <= 5) && f_isNightTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "nskc.jpg");
	          strcpy (phrase[dayIndex], "Clear");
	       }
	       else if ((averageSkyCover[dayIndex] <= 25) && f_isDayTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "few.jpg");
	          strcpy (phrase[dayIndex], "Mostly Sunny");
	       }
	       else if ((averageSkyCover[dayIndex] <= 25) && f_isNightTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "nfew.jpg");
	          strcpy (phrase[dayIndex], "Mostly Clear");
	       }
 	       else if ((averageSkyCover[dayIndex] <= 50) && f_isDayTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "sct.jpg");
	          strcpy (phrase[dayIndex], "Partly Cloudy");
	       }
	       else if ((averageSkyCover[dayIndex] <= 50) && f_isNightTime)
	       {
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "nsct.jpg");
	          strcpy (phrase[dayIndex], "Partly Cloudy");
	       }
 	       else if ((averageSkyCover[dayIndex] <= 87) && f_isDayTime)
	       { 
                  sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		          "bkn.jpg");
	          strcpy (phrase[dayIndex], "Mostly Cloudy");
	       }
	       else if ((averageSkyCover[dayIndex] <= 87) && f_isNightTime)
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
	 else if (trend_speed < 1)
         {
		 
            /* Clouds increasing over a duration of less than 3 hours (1 index 
             * or category diff). 
             */
            if (trend_inc_early < 1)
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
            else if (trend_inc_late < 1)
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
	       
	 } /* Close "increasing over a duration of less than 3 hours (1 index 
             * or category diff). 
             */

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
            if (trend_dec_early < 1) 
	    {	    
               /* Cloudy/MoCloudy...then clearing early. */
               if (trend_speed < 1) 
	       {
		  if (f_isDayTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		             daySkyImage[3]);
		  else if (f_isNightTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
		             nightSkyImage[3]);
			  
	          strcpy (phrase[dayIndex], "Clearing");
	       }
	       else if (trend_speed >= 1)
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
	    else if (trend_dec_late < 1)
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
               if (trend_speed < 1) 
               {
                  if (f_isDayTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
			     "bkn.jpg");
	          else if (f_isNightTime)
                     sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
			     "nbkn.jpg");

	          strcpy (phrase[dayIndex], "Clearing");
	       }
	       else if (trend_speed >= 1)
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
	 else if (trend_speed >= 1)
         {
            /* Clouds decreasing over a duration of 3 or more hours (1 index or 
             * category diff). 
             */
            if (f_isDayTime)
            {
	       strcpy (phrase[dayIndex], "Decreasing Clouds");
	       sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
                       daySkyImage[avgCategory]);
	    } 
            else if (f_isNightTime)
            {
	       strcpy (phrase[dayIndex], "Decreasing Clouds");
	       sprintf(iconInfo[dayIndex].str, "%s%s", baseURL, 
                       nightSkyImage[avgCategory]);
	    } 
	 }
	 else if (trend_speed < 1)
         {
            /* Clouds decreasing over a duration of less than 3 hours (1 index or 
             * category diff). 
             */
            if (trend_dec_early < 1) 
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
