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
 *    numMatch = The number of matches from degrib. (Input)
 *   numRowsWS = The number of data rows for wind speed. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 *  numRowsPOP = The number of data rows for Pop12. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 *  numRowsMAX = The number of data rows for max temps. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 *  numRowsMIN = The number of data rows for min temps. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 *   numRowsWG = The number of data rows for wind gust values. These data are 
 *               used to derive the weather phrase "Blizzard". (Input)
 *   numRowsWX = The number of data rows for weather. These data can be 
 *               formatted if f_wx = 1, and/or used to derive icons if f_icon 
 *               = 1.  (Input)
 *  parameters = Xml Node parameter. The formatted weather and icons are child
 *               elements of this node. (Input)
 *     numDays = The number of days the validTimes for all the data rows 
 *               (values) consist of. (Input)
 *    TZoffset = Number of hours to add to current time to get GMT time. 
 *               (Input)
 * f_observeDST = Flag determining if current point observes Daylight 
 *                Savings Time. (Input)  
 *    frequency = Describes the two DWMLgenByDay product and their type
 *                of summarizations ("12hourly" or "24 hourly"). (Input)   
 * f_useMinTempTimes = Flag denoting if we should simply use the MinT 
 *                     layout key for the MaxT element. Only used in 
 *                     DWMLgenByDay products. (Input)
 *        f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product. (Input)
 * numOutputLines = The number of data values output/formatted for each element. 
 *                  (Input)
 *  maxDailyPop =  Array containing the pop values corresponding to a day (24 
 *                 hour format) or 12 hour period (12 hour format).  For 24
 *                 hour format, we use the maximum of the two 12 hour pops 
 *                 that span the day. This variable is used to test if the pop
 *                 (against a pop threshold) is large enough to justify 
 *                 formatting weather values. (Input)
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
 *   startTime_cml = Incoming argument set by user as a double in seconds 
 *                   since 1970 denoting the starting time data is to be 
 *                   retrieved for from NDFD. (Set to 0.0 if not entered.)
 *                   (Input) 
 *    format_value = Option to turn off the formating of the value child 
 *                   element for weather. (TPEX sets to zero, as they don't 
 *                   format this). (Input)
 *        startNum = First index in match structure an individual point's data 
 *                   matches can be found. (Input)
 *          endNum = Last index in match structure an individual point's data
 *                   matches can be found. (Input)
 *     f_shiftData = Flag used to determine whether we shift data back by 1/2
 *                   it's period to denote the duration of time the data is 
 *                   valid for (Soap Service), or to not shift, and simply use
 *                   the data's endTime (validTime) (JimC's TPEX, FOX, products
 *                   etc) (Input)
 * maxWindSpeedValTimes = Array holding valid Time of max Wind Speed per 
 *                        forecast period (Input). 
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *  4/2007 Paul Hershberg (MDL): Added capability to find Wind Gust validtimes.
 *  6/2007 Paul Hershberg (MDL): Added capability to determine Blizzard Conditions.
 *  7/2007 Paul Hershberg (MDL): Added capability of new T-storm dominance algorithm.
 * 10/2007 Paul Hershberg (MDL): Removed code that shifted data back by 1/2
 *                               the period length (bug from php code)
 * 11/2007 Paul Hershberg (MDL): Added flag that determines whether to shift data 
 *                               back by 1/2 the period length (Soap Service) or 
 *                               not (JimC's TPEX, SSC, etc). 
 *  3/2008 Paul Hershberg (MDL): Added maxWindSpeedValTimes for determination 
 *                               of Cold vs Warm season in windExtremePhrase.c 
 *                               routine.
 *  4/2009 Paul Hershberg (MDL): Added code to deal with the missing colon 
 *                               delimiter in the ugly wx strings.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void genWeatherValuesByDay(size_t pnt, char *layoutKey, 
		           genMatchType *match, size_t numMatch,
                           numRowsInfo numRowsWG, numRowsInfo numRowsWS, 
                           numRowsInfo numRowsPOP, numRowsInfo numRowsMAX, 
                           numRowsInfo numRowsMIN, numRowsInfo numRowsWX, 
                           xmlNodePtr parameters, int *numDays, sChar TZoffset,
			   sChar f_observeDST, char *frequency,
			   int f_useMinTempTimes, uChar f_XML,
			   int *numOutputLines, int *maxDailyPop, 
			   int *averageSkyCover, int *maxSkyCover, 
			   int *minSkyCover, int *maxSkyNum, int *minSkyNum, 
			   int *startPositions, int *endPositions, 
			   int *maxWindSpeed, int *maxWindDirection, 
			   double startTime_cml, int format_value, int startNum,
                           int endNum, int f_shiftData, 
                           double *maxWindSpeedValTimes)
{
   double springDoubleDate = 0.0; /* The end date of next cold season expressed
                                     in double form. */
   double fallDoubleDate = 0.0; /* The start date of next cold season expressed
                                   in double form. */
   int period = 3;      /* Length between an elements successive validTimes. */
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
   int f_isBlowingSnow     = 0; /* Flag denoting if weather is blowing snow. */
     
   /* Initialize the number of times we see fog today. */
   int fogCount = 0;
   int percentTimeWithFog = 0;
   
   /* Initialize # of rows of data processed for a given day. */
   int numDataRows = 0;  /* The # of rows of data processed for a given day. */
   char *dominantWeather[5]; /* This array stores the weather type [0], 
                              * intensity [1], coverage [2], visibility[3] and 
                              * qualifier [4], for each day that is considered 
                              * the dominant one. This is the summarized weather
                              * for the 24/12 hour summarization period. */
   char *tempDom[5]; /* Holds the latest weather information which is then 
		      * compared to the dominantWeather array to see if a new 
		      * dominant weather condition has been found. */
   char *dominantRowsWeather[5][5]; /* Array containing the other 4 weather 
				     * values (out of coverage, intensity,
				     * type, visibility, and qualifier) of a 
				     * row once the dominant weather was found.
				     */
   char WxGroups[5][100];    /* An array holding up to 5 weather groups for
                              * any one valid time. */
   char WxValues[5][50];      /* An associative array holding the current
                               * groups type[1], coverage[0], intensity[2], 
                               * visibility[3], & qualifier[4]. */
   int numGroups = 0;        /* Number of Weather Groups per one row of Weather
                              * data (# of groups in ugly string) */
   WX *wxInfo = NULL;         /* All Weather data taken from the match array. */
   elem_def *wgInfo = NULL;   /* Wind Speed Gust data taken from the match 
                               * array. */
   elem_def *wsInfo = NULL;   /* Wind Speed data taken from the match 
                               * array. */
   int priorElemCount = 0;     /* Counter used to find elements' location in
                                  match. */
   char *pstr = NULL;         /* Pointer to "ugly string" delimited by '>'
                               * and '<'. */
   int i;                     /* Index through the match structure. */
   int j;
   int wqIndex = 0;           /* Counter for wx hazards/qualifiers. */
   int indexOfindexes[5];     /* An array holding the # of hazards/qualifiers
			       * found in each wx grp. */
   char *addrtempstore;       /* Address of the tempstore variable holding each
			       * qualifier.
			       */
   char tempstore[200];       /* A temporary storage area. CMc added 7/5/06. */
   char *token;
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
   char **wxVisibility = NULL; /* An individual weather groups visibility. */
   char ***wxQualifier = NULL; /* An individual weather groups qualifiers.
				* Char array holding up to 5 weather qualifiers 
			        * for up to 5 wx groups. */
   char **Qualifier = NULL;   /* After the multiple qualifiers are concatenated, 
			         this array holds them.*/
   int numValues;             /* An index into each weatherGroups fields (=
                               * 5). */
   int groupIndex;            /* An index into the weatherGroups array. */
   int valueIndex;            /* An index into each weatherGroups fields (=
                               * 5). */
   int wxIndex = 0;           /* Counter thru weather data rows. */
   int numDominantTypes = 0; /* Placeholder for numGroups. */
   xmlNodePtr weather = NULL; /* Xml Node Pointer for node "weather". */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for node "value". */
   xmlNodePtr weather_conditions = NULL;  /* Xml Node Pointer for node
                                           * "weather-conditions". */   
   icon_def *iconInfo = NULL; /* Array holding the icon information. */
   char **phrase; /* Array holding the weather phrase per summarization period. */
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
   char transQualifierStr[200]; /* String holding english translation of
                                 * weather qualifiers. */
   int numActualRowsMAX; /* MaxT # of rows interested in taking into accordance 
                          * a user supplied shortened time period. */
   int numActualRowsMIN; /* MinT # of rows interested in taking into accordance 
                          * a user supplied shortened time period. */
   int numActualRowsWX; /* Wx # of rows interested in taking into accordance 
                         * a user supplied shortened time period. */
   int numActualRowsPOP; /* Pop12 # of rows interested in taking into accordance 
                          * a user supplied shortened time period. */
   int numActualRowsWG; /* # of Wind Gust rows interested in taking into
                         * accordance a user supplied shortened time
                         * period. */
   int numActualRowsWS; /* # of Wind Speed rows interested in taking into
                         * accordance a user supplied shortened time
                         * period. */
   int f_blizzardType = 0; /* Denotes if weather type for "blizzard"
                            * determination was met. */
   int f_blizzardVisibility = 0; /* Denotes if weather visibility for "blizzard" 
                                  * determination was met. */
   int f_blizzardIntensity = 0; /* Denotes if weather intensity for "blizzard" 
                                 * determination was met. */
   double *blizzardTime = NULL; /* If all blizzard conditions are met in the
                                 * ugly weather string, the time is recorded. */
   int blizzCnt = 0; /* Counter thru blizzard Times array. */
   char tempPriorIntensity[10]; /* Temporary holder for intensity due to T-storm 
                                 * dominance check. Holds the prior group's 
                                 * intensity string. */
   char tempCurrentIntensity[10]; /* Temporary holder for intensity due to T-storm 
                                   * dominance check. Holds the current group's 
                                   * intensity string. */
   char None[] = "<NoCov>:<NoWx>:<NoInten>:<NoVis>:"; /* Used as ugly wx string 
                                                         when ugly wx string 
                                                         consists of <None>. */
   /* Initialize the location where the weather icons are found. */
   char baseURL[] = "http://www.nws.noaa.gov/weather/images/fcicons/";

   /************************************************************************/
   /* Initialize the actual number of rows we are working with for the 5
    * elements of interest. 
    */
   numActualRowsMAX = numRowsMAX.total-numRowsMAX.skipBeg-numRowsMAX.skipEnd;
   numActualRowsMIN = numRowsMIN.total-numRowsMIN.skipBeg-numRowsMIN.skipEnd;
   numActualRowsWX = numRowsWX.total-numRowsWX.skipBeg-numRowsWX.skipEnd;
   numActualRowsPOP = numRowsPOP.total-numRowsPOP.skipBeg-numRowsPOP.skipEnd;
   numActualRowsWG = numRowsWG.total-numRowsWG.skipBeg-numRowsWG.skipEnd;
   numActualRowsWS = numRowsWS.total-numRowsWS.skipBeg-numRowsWS.skipEnd;

   /* Firstly, format the weather and display name elements. */
   weather = xmlNewChild(parameters, NULL, BAD_CAST "weather", NULL);
   xmlNewProp(weather, BAD_CAST "time-layout", BAD_CAST layoutKey);

   xmlNewChild(weather, NULL, BAD_CAST "name", BAD_CAST
               "Weather Type, Coverage, and Intensity");
	 
   /* Create an array of structures holding the wind speed gust data
    * and time info from the match structure. These data will be used
    * in blizzard determination. 
    */
   wgInfo = malloc(numActualRowsWG * sizeof(elem_def));

   /* Fill Wind Speed Gust Array (every 3 hours). */
   priorElemCount = 0;
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WG && 
         match[i].validTime >= numRowsWG.firstUserTime &&
         match[i].validTime <= numRowsWG.lastUserTime)
      {
         wgInfo[i -priorElemCount-startNum].validTime = match[i].validTime;

         /* Subtract half the period. */
         if (f_shiftData)
         {
  	    if ((i-priorElemCount-startNum) < 1)
               period = determinePeriodLength(match[i].validTime,
                                              match[i + 1].validTime, 
   					      numActualRowsWG, NDFD_WG);
            else
               period = determinePeriodLength(match[i - 1].validTime,
                                              match[i].validTime,
                                              numActualRowsWG, NDFD_WG);

            wgInfo[i -priorElemCount-startNum].validTime = 
            wgInfo[i -priorElemCount-startNum].validTime - 
                           (((double)period * 0.5) * 3600);
         }
         wgInfo[i-priorElemCount-startNum].data =
                (int)myRound(match[i].value[pnt].data, 0);
         wgInfo[i-priorElemCount-startNum].valueType =
                match[i].value[pnt].valueType;
      }
      else
         priorElemCount++;
   }
		 
   /* Create an array of structures holding the wind speed data
    * and time info from the match structure. These data will be used
    * in blizzard determination.
    */
   wsInfo = malloc(numActualRowsWS * sizeof(elem_def));

  /* Fill Wind Speed Array (every 3 hours). */
   priorElemCount = 0;
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WS && 
         match[i].validTime >= numRowsWS.firstUserTime &&
         match[i].validTime <= numRowsWS.lastUserTime)
      {
         wsInfo[i -priorElemCount-startNum].validTime = match[i].validTime;

         /* Subtract half the period. */
         if (f_shiftData)
         {
  	    if ((i-priorElemCount-startNum) < 1)
               period = determinePeriodLength(match[i].validTime,
                                              match[i + 1].validTime, 
					      numActualRowsWS, NDFD_WS);
            else
               period = determinePeriodLength(match[i - 1].validTime,
                                              match[i].validTime,
                                              numActualRowsWS, NDFD_WS);

            wsInfo[i -priorElemCount-startNum].validTime = 
            wsInfo[i -priorElemCount-startNum].validTime - 
                          (((double)period * 0.5) * 3600);
         }
         wsInfo[i-priorElemCount-startNum].data =
                (int)myRound(match[i].value[pnt].data, 0);
         wsInfo[i-priorElemCount-startNum].valueType =
                match[i].value[pnt].valueType;
      }
      else
         priorElemCount++;
   }
    
   /* Create an array of structures holding the weather element's
    * data info and time info from the match structure. 
    */
   wxInfo = malloc(numActualRowsWX * sizeof(WX));

   /* Fill Weather Array. */
   priorElemCount = 0;
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WX && 
	  match[i].validTime >= numRowsWX.firstUserTime &&
	  match[i].validTime <= numRowsWX.lastUserTime)
      {
         wxInfo[i -priorElemCount-startNum].validTime = match[i].validTime;

         /* Subtract half the period. */
         if (f_shiftData)
         {
  	    if ((i-priorElemCount-startNum) < 1)
               period = determinePeriodLength(match[i].validTime,
                                              match[i + 1].validTime, 
					      numActualRowsWX, NDFD_WX);
            else
               period = determinePeriodLength(match[i - 1].validTime,
                                              match[i].validTime,
                                              numActualRowsWX, NDFD_WX);

            wxInfo[i -priorElemCount-startNum].validTime = 
            wxInfo[i -priorElemCount-startNum].validTime - 
                          (((double)period * 0.5) * 3600);
         }

         if (match[i].value[pnt].valueType != 0 &&
             match[i].value[pnt].valueType != 2)
         {
            strcpy(wxInfo[i-priorElemCount-startNum].str, match[i].value[pnt].str);
         }
         wxInfo[i-priorElemCount-startNum].valueType =
               match[i].value[pnt].valueType;
      }
      else
         priorElemCount++;
   }

   /* Prepare to retrieve the Weather Data times, the forecast period times, 
    * and the max temperatures in each period. 
    */      
   periodStartTimes = (double *) malloc((*numOutputLines) * sizeof(double));
   periodEndTimes = (double *) malloc((*numOutputLines) * sizeof(double));
   periodMaxTemp = (int *) malloc((*numOutputLines) * sizeof(int));
   
   for (i = 0; i < *numOutputLines; i++)
   { 
      periodStartTimes[i] = 9999999999999999999999.0;
      periodEndTimes[i]   = -999.0;
      periodMaxTemp[i]    = 999;
   }
     
   prepareWeatherValuesByDay (match, TZoffset, f_observeDST, frequency, 
		              numDays, numOutputLines, numRowsWS, numRowsMIN,  
			      numRowsMAX, f_XML, numRowsPOP, pnt, 
                              f_useMinTempTimes, startTime_cml, 
                              periodMaxTemp, periodStartTimes, 
                              periodEndTimes, &springDoubleDate, 
                              &fallDoubleDate, &timeLayoutHour, startNum, 
                              endNum);

   /* This is a big loop to format each day's weather. First, make appropriate
    * allocations. 
    */
   iconInfo = (icon_def *) malloc(*numOutputLines * sizeof(icon_def));
   phrase = (char **) malloc(*numOutputLines * sizeof(char *));	
   isDataAvailable = (int *) malloc(*numOutputLines * sizeof(int));

   /* Allocate the individual elements of the weather phrase. */
   for (i = 0; i < *numOutputLines; i++)
      phrase[i] = (char *) malloc(50 * sizeof(char));

   /* A big loop to format each summarization period's weather. */
   for (dayIndex = 0; dayIndex < *numOutputLines; dayIndex++)
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
      f_isBlowingSnow     = 0;
     
      /* Initialize the number of times we see fog today per each loop. */
      fogCount = 0;
      percentTimeWithFog = 0;
      
      /* Initialized number of rows of data processed for a given day. */
      numDataRows = 0;

      /* Initialize both the dominantWeather (per period) and the day's Weather
       * to lowest possible values. 
       */
      for (i = 0; i < 5; i++) /* 5 wx categories interested in (coverage, 
			       * intensity, type, visibility, qualifier). */
      {
         dominantWeather[i] = (char *) malloc(5 * sizeof(char));
         strcpy (dominantWeather[i], "none");
         	      
         for (j = 0; j < 5; j++) /* Up to 5 weather groups per 1 wx string. */
         {
	    dominantRowsWeather[i][j] = (char *) malloc(5 * sizeof(char));
	    strcpy(dominantRowsWeather[i][j], "none");
	 }
      }

      /* Loop over all of weather's valid times and process any weather groups
       * (up to 5 can exist). 
       */
      for (wxIndex = 0; wxIndex < numActualRowsWX; wxIndex++)
      {
         memset(WxValues, '\0', 5 * 50);
         memset(WxGroups, '\0', 5 * 100);
         numGroups = 0;

         /* Flags for Blizzard Determination. */
         f_blizzardType = 0;
         f_blizzardIntensity = 0;
         f_blizzardVisibility = 0;
	 
         /* First, determine if this row of weather has valid data. */
	 if (wxInfo[wxIndex].valueType != 2) 
         {
            /* Determine if the data is between the user supplied start day and a 
             * calculated end time (based on user provided number of days and the 
             * period length implied by user requested format (make sure the 
	     * weather rows being processed fall within the current period of 
   	     * interest).
             */
	    if (periodStartTimes[dayIndex] <= wxInfo[wxIndex].validTime && 
               wxInfo[wxIndex].validTime < periodEndTimes[dayIndex])	 
            {
               isDataAvailable[dayIndex] = 1;

               /* We found a data row to process, so count it. */
	       numDataRows++;

               /* if the "ugly weather string" consists solely of "<None>" 
                * then treat it as if it is "<NoCov>:<NoWx>:<NoInten>:<NoVis>:".
                * Next, check if there are no delimiters (colons) after looking 
                * for <None>. If this is the case, treat current weather row as 
                * non-valid data and continue on to next weather row.
                */
               if (strstr(wxInfo[wxIndex].str, ":") == '\0')
               {
                  if (strstr(wxInfo[wxIndex].str, "<None>") != '\0')
                     strcpy(wxInfo[wxIndex].str, None);
                  else
                     continue;
               }
	    
               /* Lets remove the <> that surround the fields (type, intensity, 
                * coverge, visibility, and qualifier) that make up each weather 
                * group coming from NDFD. 
                */
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
                        WxGroups[groupIndex][j - 1] = pstr[j];
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
                     WxGroups[numGroups][j - 1] = '\0';
                  else
                     WxGroups[numGroups][j] = '\0';
               }
               else
               {
                  if (pstr2[j - 1] == ':')
                     WxGroups[numGroups][j - 1] = '\0';
               }

               /* Initialize the temporary dominance array to lowest possible 
	        * values. This array tracks which of the possible 5 groups per one
	        * row of data (one weather ugly string) is the dominant one. 
                */
               for (j = 0; j < 5; j++)
               {
                  tempDom[j] = (char *) malloc(5 * sizeof(char));
                  strcpy (tempDom[j], "none");
               }
	    
               /* For each group, process its weather information (i.e.
	        * coverage, intensity, type, and qualifier and visibility for 
                * use in "Blizzard" determination) all which are denoted 
                * by a ":". 
	        */
               wxCoverage = (char **)calloc(numGroups + 1, sizeof(char *));
               wxIntensity = (char **)calloc(numGroups + 1, sizeof(char *));
	       wxType = (char **)calloc(numGroups + 1, sizeof(char *));
               wxVisibility = (char **)calloc(numGroups + 1, sizeof(char *));
	       Qualifier = (char **)calloc(numGroups + 1, sizeof(char *));
               wxQualifier = (char ***)calloc(numGroups + 1, sizeof(char **));
	       
	       /* Now that we know numGroups per weather string (up to 5 per 
                * ugly weather string), loop over each group. 
                */
               for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
               {
                  numValues = 0; /* Number of values (up to 5 -- type, 
                                  * intensity, coverage, visibility, and 
                                  * qualifier(s)) per 1 group. */
		  
	          /* Initialize the 5 wxQualifier possibilities to "none". */
	          wxQualifier[groupIndex] = (char **) malloc (5 * sizeof(char *));
		  for (wqIndex = 0; wqIndex < 5; wqIndex++)
	          {
		     wxQualifier[groupIndex][wqIndex] = (char *) malloc 
			        ((strlen("none")+1) * sizeof(char));
	             strcpy(wxQualifier[groupIndex][wqIndex], "none");
	          }

                  /* Create the associative array holding the (up to 5)  
                   * weather info fields. 
                   */
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
                           WxValues[valueIndex][i - 1] = pstr[i];
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
                     wxCoverage[groupIndex] = malloc((strlen("none") + 1) * sizeof(char));
                     strcpy(wxCoverage[groupIndex], "none");
	          }
	          else /* Populate weather value arrays for 'coverage',
		        * 'intensity', 'type', 'visibility' and 'qualifier(s)'
                        * (if they exist). Flag conditions for "Blizzard".
                        */
	          {
		       
                     /* Weather Coverage */
                     if (WxValues[0][1] == 'N' && WxValues[0][2] == 'o')
                     {
                        wxCoverage[groupIndex] = (char *) malloc((strlen("none") + 1) * sizeof(char));
                        strcpy(wxCoverage[groupIndex], "none");
                     }
                     else
                     {
                        strTrim(WxValues[0]);
                        wxCoverage[groupIndex] = (char *) malloc((strlen(WxValues[0]) + 1) * sizeof(char));
                        strcpy(wxCoverage[groupIndex], WxValues[0]);
                     }

                     /* Weather Type */
                     if (WxValues[1][1] == 'N' && WxValues[1][2] == 'o')
                     {
                        wxType[groupIndex] = (char *) malloc((strlen("none") + 1) * sizeof(char));
                        strcpy(wxType[groupIndex], "none");
                     }
                     else
                     {
                        strTrim(WxValues[1]);
                        wxType[groupIndex] = (char *) malloc((strlen(WxValues[1]) + 1) * sizeof(char));
                        strcpy(wxType[groupIndex], WxValues[1]);
                          
                        /* Check for Blizzard condition Type. */
                        if (strcmp (wxType[groupIndex], "S") == 0 || 
                            strcmp (wxType[groupIndex], "BS") == 0)
                           f_blizzardType = 1;
                     }

                     /* Weather Intensity */
                     if (WxValues[2][1] == 'N' && WxValues[2][2] == 'o')
                     {
                        wxIntensity[groupIndex] = (char *) malloc((strlen("none") + 1) * sizeof(char));
                        strcpy(wxIntensity[groupIndex], "none");
                     }
                     else
                     {
                        strTrim(WxValues[2]);
                        wxIntensity[groupIndex] = (char *) malloc((strlen(WxValues[2]) + 1) * sizeof(char));
                        strcpy(wxIntensity[groupIndex], WxValues[2]);
                          
                        /* Check for Blizzard condition Intensity. */
                        if (strcmp (wxIntensity[groupIndex], "+") == 0 || 
                            strcmp (wxIntensity[groupIndex], "m") == 0)
                           f_blizzardIntensity = 1;
                     }
	       
                     /* Note, we are not processing the visibility WxValue field, 
	      	      * which is denoted by WxValues[3] element, for the DWMLgenByDay
	   	      * products. NOTE! -- started processing Visibility for generation of
                      * "Blizzard" as a weather phrase (April 2007).
	              */

                     /* Weather Visibility */
                     if (WxValues[3][1] == 'N' && WxValues[3][2] == 'o')
                     {
                        wxVisibility[groupIndex] = (char *) malloc((strlen("none") + 1) * sizeof(char));
                        strcpy(wxVisibility[groupIndex], "none");
                     }
                     else
                     {
                        strTrim(WxValues[3]);
                        wxVisibility[groupIndex] = (char *) malloc((strlen(WxValues[3]) + 1) * sizeof(char));
                        strcpy(wxVisibility[groupIndex], WxValues[3]);
                          
                        /* Check for Blizzard condition Visibility (as of 6/2007,
                         * visibility is NOT used in Blizzard determination.
                         */
                        if (strcmp (wxVisibility[groupIndex], "1/4SM") == 0)
                           f_blizzardVisibility = 1;
                     }
	       
                     /* Weather Qualifier(s) */
	       
		     /* Declare array of the # of qualifiers associated with each 
		      * wx group. 
		      */
                     wqIndex = indexOfindexes[groupIndex] = 0;
		     
                     /* If they exist, move any weather hazards/qualifiers codes 
	   	      * into the wxQualifier array for later translation. 
		      */
                     if ((WxValues[4][0] != '\0') && 
	                 (WxValues[4][1] != 'N' && WxValues[4][2] != 'o'))
		     {
                        strTrim(WxValues[4]);
                        strcpy(tempstore, WxValues[4]);

                        token = strtok( tempstore, " ," );

                        if( token != NULL ) 
		        {
                           wxQualifier[groupIndex][wqIndex] = (char *) realloc
	                      (wxQualifier[groupIndex][wqIndex], 
			      (strlen(token) + 1) * sizeof(char));
                           strcpy(wxQualifier[groupIndex][wqIndex], token);
                       
                           while (token != NULL) /* Check for up to 5 qualifiers. */
		           {
                              token = strtok( NULL, " ," );

                              if(token != NULL) 
		              {
                                 wqIndex++; 
                                 indexOfindexes[groupIndex] = wqIndex;
			         wxQualifier[groupIndex][wqIndex] = (char *) realloc
	                            (wxQualifier[groupIndex][wqIndex], 
			            (strlen(token) + 1) * sizeof(char));
                                 strcpy(wxQualifier[groupIndex][wqIndex], token);
                              }
                           }
                        }
                     }

		     /* Combine any multiple qualifiers into one statement and place
		      * in transQualifierStr. First, initialize variables in 
		      * preparation for translating these hazards/qualifiers.  
		      */

                     /* Set all characters of tempstore to blanks */
                     memset(tempstore,' ',sizeof(tempstore));

                     /* Set the last character of tempstore to a NULL */
                     memset(tempstore+199,'\0',1);             

                     /* Save the starting address of the tempstore array */
                     addrtempstore = &tempstore[0];            
                     
                     /* Translate the hazards/qualifiers for the current wx 
                      * group into a single translated string.
                      */
                     for (wqIndex = 0; wqIndex <= indexOfindexes[groupIndex]; wqIndex++) 
	             {
                        getTranslatedQualifier(wxQualifier[groupIndex][wqIndex],
                                               transQualifierStr);
                        if (wqIndex == 0) 
                           strcpy (tempstore, transQualifierStr);

                        if(wqIndex != 0 && wqIndex <= indexOfindexes[groupIndex]) 
	                {
                           addrtempstore = &tempstore[0]+strlen(tempstore);
                           strncat(addrtempstore,",",1);
                           addrtempstore += 1;
                           strcat(addrtempstore,transQualifierStr);
                        }
                     }

                     /* Copy complete translated hazards string to transQualifierStr,
		      * which is group dependent, and Qualifier[groupIndex],which 
		      * is not. 
		      */
                     strcpy (transQualifierStr, tempstore);
		     Qualifier[groupIndex] = (char *) malloc 
			        ((strlen(tempstore)+1) * sizeof(char));
                     strcpy (Qualifier[groupIndex], tempstore); 
		  }

                  /* Re-initialize the WxValues array for next group iteration. */
                  memset(WxValues, '\0', 5 * 50);

                  /* See if all "Blizzard" conditions were met. If so, denote the
                   * time these conditions were met. (NOTE, as of 2007, we are 
                   * not using the visibility part of the weather string (<1/4M)
                   * to determine Blizzard). We are assuming Both snow, and blowing 
                   * snow begin at 1/4 mile visibility. Thus, the occurrence of 
                   * snow or blowing snow should be enough to guarantee that 
                   * visibility is 1/4 mile or less.
                   */
                  if (f_blizzardType && f_blizzardIntensity)
/*                  if (f_blizzardType && f_blizzardIntensity && 
                        f_blizzardVisibility)
*/
                  {
                     blizzardTime = (double *) realloc(blizzardTime,
                                    (blizzCnt+1) * sizeof(double));
                     blizzardTime[blizzCnt] = wxInfo[wxIndex].validTime;
                     blizzCnt++;
                  }

                  /***********BEGIN TESTING FOR DOMINANCE OF WEATHER************/

                  /* If coverage is dominant, set new tempDom to the current group. */
	          if (isDominant(wxCoverage[groupIndex], tempDom[0], "coverage"))
	          {
                     /* Copy over the 'coverage' to the temporary array. */
                     tempDom[0] = (char *) realloc(tempDom[0], (strlen(wxCoverage[groupIndex]) + 1) * sizeof(char));
	             strcpy (tempDom[0], wxCoverage[groupIndex]);

                     /* Copy over the 'intensity' to the temporary array. */
                     tempDom[1] = (char *) realloc(tempDom[1], (strlen(wxIntensity[groupIndex]) + 1) * sizeof(char));
	   	     strcpy (tempDom[1], wxIntensity[groupIndex]);

                     /* Copy over the 'type' to the temporary array. */
                     tempDom[2] = (char *) realloc(tempDom[2], (strlen(wxType[groupIndex]) + 1) * sizeof(char));
		     strcpy (tempDom[2], wxType[groupIndex]);

                     /* Copy over the 'visibility' to the temporary array. */
                     tempDom[3] = (char *) realloc(tempDom[3], (strlen(wxVisibility[groupIndex]) + 1) * sizeof(char));
		     strcpy (tempDom[3], wxVisibility[groupIndex]);

                     /* Copy over the 'qualifier' to the temporary array. */
                     tempDom[4] = (char *) realloc(tempDom[4], (strlen(transQualifierStr) + 1) * sizeof(char));
		     strcpy (tempDom[4], transQualifierStr);
	          }
	          else if (strcmp(wxCoverage[groupIndex], tempDom[0]) == 0)   
	          {		       
	             strcpy (tempPriorIntensity, tempDom[1]);
                     strcpy (tempCurrentIntensity, wxIntensity[groupIndex]);

	             /* If coverage is equal, test for dominant intensity. */
                    
                     /* Take care of special case scenario where there is more than 1 group in the ugly weather
                      * string. If the prior group or current group's "type" is thunderstorms ("T") and the 
                      * coverages of the two groups in comparison are equal, set the intensity of the thunderstorm 
                      * group temporarily to "+". This will force thunderstorms to dominate as the weather summary 
                      * phrase of the day over precipitation.
                      */                                
                     if (numGroups > 0)
                     {
                        if (strcmp(wxType[groupIndex], "T") == 0)  
                           strcpy (tempCurrentIntensity, "+");
                        else if (strcmp(tempDom[2], "T") == 0)
                           strcpy (tempPriorIntensity, "+");
                     }
 
                     if (isDominant(tempCurrentIntensity, tempPriorIntensity, "intensity"))
	             {
                        /* Copy over the 'coverage' to the temporary array. */
                        tempDom[0] = (char *) realloc(tempDom[0], (strlen(wxCoverage[groupIndex]) + 1) * sizeof(char));
		        strcpy (tempDom[0], wxCoverage[groupIndex]);
                     
                        /* Copy over the actual 'intensity' to the temporary array (not a "+" if altered above in 
                         * Tstorm priority, as that is temporary).
                         */ 
                        tempDom[1] = (char *) realloc(tempDom[1], (strlen(wxIntensity[groupIndex]) + 1) * sizeof(char));
		        strcpy (tempDom[1], wxIntensity[groupIndex]);

                        /* Copy over the 'type' to the temporary array. */
                        tempDom[2] = (char *) realloc(tempDom[2], (strlen(wxType[groupIndex]) + 1) * sizeof(char));
		        strcpy (tempDom[2], wxType[groupIndex]);

                        /* Copy over the 'visibility' to the temporary array. */
                        tempDom[3] = (char *) realloc(tempDom[3], (strlen(wxVisibility[groupIndex]) + 1) * sizeof(char));
		        strcpy (tempDom[3], wxVisibility[groupIndex]);

                        /* Copy over the 'qualifier' to the temporary array. */
                        tempDom[4] = (char *) realloc(tempDom[4], (strlen(transQualifierStr) + 1) * sizeof(char));
	   	        strcpy (tempDom[4], transQualifierStr);
 		     }
		     else if (strcmp(wxIntensity[groupIndex], tempDom[1]) == 0)
	             {		       
	                /* If intensity is equal, test for dominant type. */
                        if (isDominant(wxType[groupIndex], tempDom[2], "type"))
	                {
                           /* Copy over the 'coverage' to the temporary array. */
                           tempDom[0] = (char *) realloc(tempDom[0], (strlen(wxCoverage[groupIndex]) + 1) * sizeof(char));
	   	           strcpy (tempDom[0], wxCoverage[groupIndex]);

                           /* Copy over the 'intensity' to the temporary array. */
                           tempDom[1] = (char *) realloc(tempDom[1], (strlen(wxIntensity[groupIndex]) + 1) * sizeof(char));
		           strcpy (tempDom[1], wxIntensity[groupIndex]);

                           /* Copy over the 'type' to the temporary array. */
                           tempDom[2] = (char *) realloc(tempDom[2], (strlen(wxType[groupIndex]) + 1) * sizeof(char));
		           strcpy (tempDom[2], wxType[groupIndex]);

                           /* Copy over the 'visibility' to the temporary array. */
                           tempDom[3] = (char *) realloc(tempDom[3], (strlen(wxVisibility[groupIndex]) + 1) * sizeof(char));
		           strcpy (tempDom[3], wxVisibility[groupIndex]);

                           /* Copy over the 'qualifier' to the temporary array. */
                           tempDom[4] = (char *) realloc(tempDom[4], (strlen(transQualifierStr) + 1) * sizeof(char));
		           strcpy (tempDom[4], transQualifierStr);
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
                  dominantWeather[0] = (char *) realloc(dominantWeather[0], (strlen(tempDom[0]) + 1) * sizeof(char));
	          strcpy (dominantWeather[0],tempDom[0]);
		  
                  /* Copy over the 'intensity' to the dominant weather array. */
                  dominantWeather[1] = (char *) realloc(dominantWeather[1], (strlen(tempDom[1]) + 1) * sizeof(char));
	          strcpy (dominantWeather[1], tempDom[1]);

                  /* Copy over the 'type' to the dominant weather array. */
                  dominantWeather[2] = (char *) realloc(dominantWeather[2], (strlen(tempDom[2]) + 1) * sizeof(char));
	          strcpy (dominantWeather[2], tempDom[2]);

                  /* Copy over the 'visibility' to the dominant weather array. */
                  dominantWeather[3] = (char *) realloc(dominantWeather[3], (strlen(tempDom[3]) + 1) * sizeof(char));
	          strcpy (dominantWeather[3], tempDom[3]);

                  /* Copy over the 'qualifier' to the dominant weather array. */
                  dominantWeather[4] = (char *) realloc(dominantWeather[4], (strlen(tempDom[4]) + 1) * sizeof(char));
	          strcpy (dominantWeather[4], tempDom[4]);
	       
                  for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
	          {
                     /* Save the contents of this row so if it turns out to be the
                      * the dominate row, we will know what all the weather was,
                      * not just the row's dominant member.
	   	      */

		     /* Copy over the 'coverage'. */     
		     dominantRowsWeather[0][groupIndex] = (char *)
			     realloc(dominantRowsWeather[0][groupIndex], (strlen(wxCoverage[groupIndex]) + 1) * sizeof(char));     
	   	     strcpy (dominantRowsWeather[0][groupIndex], 
		             wxCoverage[groupIndex]);
		     
		     /* Copy over the 'intensity'. */     
		     dominantRowsWeather[1][groupIndex] = (char *)
			     realloc(dominantRowsWeather[1][groupIndex], (strlen(wxIntensity[groupIndex]) + 1) * sizeof(char));     
		     strcpy (dominantRowsWeather[1][groupIndex], 
		             wxIntensity[groupIndex]);
		     
	   	     /* Copy over the 'type'. */     
	             dominantRowsWeather[2][groupIndex] = (char *)
			     realloc(dominantRowsWeather[2][groupIndex], (strlen(wxType[groupIndex]) + 1) * sizeof(char));     
	             strcpy (dominantRowsWeather[2][groupIndex], 
		             wxType[groupIndex]);

                     /* Copy over the 'visibility'. */
	             dominantRowsWeather[3][groupIndex] = (char *)
			     realloc(dominantRowsWeather[3][groupIndex], (strlen(wxVisibility[groupIndex]) + 1) * sizeof(char));     
	             strcpy (dominantRowsWeather[3][groupIndex], 
		             wxVisibility[groupIndex]);
			     
	             /* Copy over the 'qualifier'. */     
		     dominantRowsWeather[4][groupIndex] = (char *)
			  realloc(dominantRowsWeather[4][groupIndex], (strlen(Qualifier[groupIndex]) + 1) * sizeof(char));     
		     strcpy (dominantRowsWeather[4][groupIndex], 
		          Qualifier[groupIndex]);
	          }
	        
	          numDominantTypes = numGroups;
	       }
	       else if (strcmp(tempDom[0], dominantWeather[0]) == 0)
               {
	          strcpy (tempPriorIntensity, dominantWeather[1]);
                  strcpy (tempCurrentIntensity, tempDom[1]);

	          /* If coverage is equal, test for dominant intensity. */
                    
                  /* Take care of special case scenario where there is more than 1 group in the ugly weather
                   * string. If the prior group or current group's "type" is thunderstorms ("T") and the 
                   * coverages of the two groups in comparison are equal, set the intensity of the thunderstorm 
                   * group temporarily to "+". This will force thunderstorms to dominate as the weather summary 
                   * phrase of the day over precipitation.
                   */                                
                  if (strcmp(tempDom[2], "T") == 0)  
                     strcpy (tempCurrentIntensity, "+");
                  else if (strcmp(dominantWeather[2], "T") == 0)
                     strcpy (tempPriorIntensity, "+");

	          if (isDominant(tempCurrentIntensity, tempPriorIntensity, "intensity")) 
	          {
                     /* Copy over the 'coverage' to the dominant weather array. */
                     dominantWeather[0] = (char *) realloc(dominantWeather[0], (strlen(tempDom[0]) + 1) * sizeof(char));
   	             strcpy (dominantWeather[0],tempDom[0]);
		     
                     /* Copy over the 'intensity' to the dominant weather array. */
                     dominantWeather[1] = (char *) realloc(dominantWeather[1], (strlen(tempDom[1]) + 1) * sizeof(char));
	             strcpy (dominantWeather[1], tempDom[1]);

                     /* Copy over the 'type' to the dominant weather array. */
                     dominantWeather[2] = (char *) realloc(dominantWeather[2], (strlen(tempDom[2]) + 1) * sizeof(char));
	             strcpy (dominantWeather[2], tempDom[2]);

                     /* Copy over the 'visibility' to the dominant weather array. */
                     dominantWeather[3] = (char *) realloc(dominantWeather[3], (strlen(tempDom[3]) + 1) * sizeof(char));
	             strcpy (dominantWeather[3], tempDom[3]);

                     /* Copy over the 'qualifier' to the dominant weather array. */
                     dominantWeather[4] = (char *) realloc(dominantWeather[4], (strlen(tempDom[4]) + 1) * sizeof(char));
	             strcpy (dominantWeather[4], tempDom[4]);
	       
                     for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
	             {
                        /* Save the contents of this row so if it turns out to be the
                         * the dominate row, we will know what all the weather was,
                         * not just the row's dominant member.
		         */
		   
		        /* Copy over the 'coverage'. */     
		        dominantRowsWeather[0][groupIndex] = (char *)
			        realloc(dominantRowsWeather[0][groupIndex], (strlen(wxCoverage[groupIndex]) + 1) * sizeof(char));     
		        strcpy (dominantRowsWeather[0][groupIndex], 
		                wxCoverage[groupIndex]);
		        /* Copy over the 'intensity'. */     
		        dominantRowsWeather[1][groupIndex] = (char *)
		      	        realloc(dominantRowsWeather[1][groupIndex], (strlen(wxIntensity[groupIndex]) + 1) * sizeof(char));     
		        strcpy (dominantRowsWeather[1][groupIndex], 
		                wxIntensity[groupIndex]);
		        /* Copy over the 'type'. */     
		        dominantRowsWeather[2][groupIndex] = (char *)
			        realloc(dominantRowsWeather[2][groupIndex], (strlen(wxType[groupIndex]) + 1) * sizeof(char));     
		        strcpy (dominantRowsWeather[2][groupIndex], 
		               wxType[groupIndex]);

                        /* Copy over the 'visibility'. */
	                dominantRowsWeather[3][groupIndex] = (char *)
			        realloc(dominantRowsWeather[3][groupIndex], (strlen(wxVisibility[groupIndex]) + 1) * sizeof(char));     
	                strcpy (dominantRowsWeather[3][groupIndex], 
		                wxVisibility[groupIndex]);

		        /* Copy over the 'qualifier'. */     
		        dominantRowsWeather[4][groupIndex] = (char *)
		 	        realloc(dominantRowsWeather[4][groupIndex], (strlen(Qualifier[groupIndex]) + 1) * sizeof(char));     
		        strcpy (dominantRowsWeather[4][groupIndex], 
		                Qualifier[groupIndex]);
	             }
	       
	             numDominantTypes = numGroups;  
  
	          }
	          else if (strcmp (tempDom[1], dominantWeather[1]) == 0) 
	          {
	             if (isDominant(tempDom[2], dominantWeather[2], "type")) 
	             {   
                        /* Copy over the 'coverage' to the dominant weather array. */
                        dominantWeather[0] = (char *) realloc(dominantWeather[0], (strlen(tempDom[0]) + 1) * sizeof(char));
   	                strcpy (dominantWeather[0],tempDom[0]);

                        /* Copy over the 'intensity' to the dominant weather array. */
                        dominantWeather[1] = (char *) realloc(dominantWeather[1], (strlen(tempDom[1]) + 1) * sizeof(char));
	                strcpy (dominantWeather[1], tempDom[1]);

                        /* Copy over the 'type' to the dominant weather array. */
                        dominantWeather[2] = (char *) realloc(dominantWeather[2], (strlen(tempDom[2]) + 1) * sizeof(char));
	                strcpy (dominantWeather[2], tempDom[2]);

                        /* Copy over the 'visibility' to the dominant weather array. */
                        dominantWeather[3] = (char *) realloc(dominantWeather[3], (strlen(tempDom[3]) + 1) * sizeof(char));
	                strcpy (dominantWeather[3], tempDom[3]);

                        /* Copy over the 'qualifier' to the dominant weather array. */
                        dominantWeather[4] = (char *) realloc(dominantWeather[4], (strlen(tempDom[4]) + 1) * sizeof(char));
	                strcpy (dominantWeather[4], tempDom[4]);
	       
                        for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
	                {
                          /* Save the contents of this row so if it turns out to be the
                            * the dominate row, we will know what all the weather was,
                            * not just the row's dominant member.
		            */
		   
		           /* Copy over the 'coverage'. */     
		           dominantRowsWeather[0][groupIndex] = (char *)
			           realloc(dominantRowsWeather[0][groupIndex], (strlen(wxCoverage[groupIndex]) + 1) * sizeof(char));     
		           strcpy (dominantRowsWeather[0][groupIndex], 
		                   wxCoverage[groupIndex]);
		           /* Copy over the 'intensity'. */     
		           dominantRowsWeather[1][groupIndex] = (char *)
		   	        realloc(dominantRowsWeather[1][groupIndex], (strlen(wxIntensity[groupIndex]) + 1) * sizeof(char));     
		           strcpy (dominantRowsWeather[1][groupIndex], 
		                   wxIntensity[groupIndex]);
		           /* Copy over the 'type'. */     
		           dominantRowsWeather[2][groupIndex] = (char *)
			           realloc(dominantRowsWeather[2][groupIndex], (strlen(wxType[groupIndex]) + 1) * sizeof(char));     
		           strcpy (dominantRowsWeather[2][groupIndex], 
		                   wxType[groupIndex]);

                           /* Copy over the 'visibility'. */
	                   dominantRowsWeather[3][groupIndex] = (char *)
			           realloc(dominantRowsWeather[3][groupIndex], (strlen(wxVisibility[groupIndex]) + 1) * sizeof(char));     
	                   strcpy (dominantRowsWeather[3][groupIndex], 
		                   wxVisibility[groupIndex]);

		           /* Copy over the 'qualifier'. */     
		           dominantRowsWeather[4][groupIndex] = (char *)
			           realloc(dominantRowsWeather[4][groupIndex], (strlen(Qualifier[groupIndex]) + 1) * sizeof(char));     
		           strcpy (dominantRowsWeather[4][groupIndex], 
		                   Qualifier[groupIndex]);
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
                           dominantWeather[0] = (char *) realloc(dominantWeather[0], (strlen(tempDom[0]) + 1) * sizeof(char));
   	                   strcpy (dominantWeather[0],tempDom[0]);

                           dominantWeather[1] = (char *) realloc(dominantWeather[1], (strlen(tempDom[1]) + 1) * sizeof(char));
	                   strcpy (dominantWeather[1], tempDom[1]);

                           /* Copy over the 'type' to the dominant weather array. */
                           dominantWeather[2] = (char *) realloc(dominantWeather[2], (strlen(tempDom[2]) + 1) * sizeof(char));
	                   strcpy (dominantWeather[2], tempDom[2]);

                           /* Copy over the 'visibility' to the dominant weather array. */
                           dominantWeather[3] = (char *) realloc(dominantWeather[3], (strlen(tempDom[3]) + 1) * sizeof(char));
	                   strcpy (dominantWeather[3], tempDom[3]);

                           /* Copy over the 'qualifier' to the dominant weather array. */
                           dominantWeather[4] = (char *) realloc(dominantWeather[4], (strlen(tempDom[4]) + 1) * sizeof(char));
	                   strcpy (dominantWeather[4], tempDom[4]);
			   
                           for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
	                   {
                            /* Save the contents of this row so if it turns out to be the
                             * the dominate row, we will know what all the weather was,
                             * not just the row's dominant member.
		             */
		   
		              /* Copy over the 'coverage'. */     
		              dominantRowsWeather[0][groupIndex] = (char *)
			              realloc(dominantRowsWeather[0][groupIndex], (strlen(wxCoverage[groupIndex]) + 1) * sizeof(char));     
		              strcpy (dominantRowsWeather[0][groupIndex], 
		                      wxCoverage[groupIndex]);
		              /* Copy over the 'intensity'. */     
		              dominantRowsWeather[1][groupIndex] = (char *)
		   	           realloc(dominantRowsWeather[1][groupIndex], (strlen(wxIntensity[groupIndex]) + 1) * sizeof(char));     
		              strcpy (dominantRowsWeather[1][groupIndex], 
		                      wxIntensity[groupIndex]);

		              /* Copy over the 'type'. */     
		              dominantRowsWeather[2][groupIndex] = (char *)
			              realloc(dominantRowsWeather[2][groupIndex], (strlen(wxType[groupIndex]) + 1) * sizeof(char));     
		              strcpy (dominantRowsWeather[2][groupIndex], 
		                      wxType[groupIndex]);

                              /* Copy over the 'visibility'. */
	                      dominantRowsWeather[3][groupIndex] = (char *)
			              realloc(dominantRowsWeather[3][groupIndex], (strlen(wxVisibility[groupIndex]) + 1) * sizeof(char));     
	                      strcpy (dominantRowsWeather[3][groupIndex], 
		                      wxVisibility[groupIndex]);

		              /* Copy over the 'qualifier'. */     
		              dominantRowsWeather[4][groupIndex] = (char *)
			              realloc(dominantRowsWeather[4][groupIndex], (strlen(Qualifier[groupIndex]) + 1) * sizeof(char));     
		              strcpy (dominantRowsWeather[4][groupIndex], 
		                      Qualifier[groupIndex]);
                           }
	                   numDominantTypes = numGroups;
                        }
		     }
	          }
	       }
	    
	       /* Free up the temp dominance array. */
               for (j = 0; j < 5; j++)
                  free (tempDom[j]);
	    
	       /* Free up the coverage, intensity, type, visibility, 
                * and qualifiers that can make up each weather group. 
	        */
               for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
               {
                  free(wxCoverage[groupIndex]);
                  free(wxType[groupIndex]);
                  free(wxIntensity[groupIndex]);
                  free(wxVisibility[groupIndex]);
		  free(Qualifier[groupIndex]);
		  
	          for (wqIndex = 0; wqIndex < 5; wqIndex++)
                     free(wxQualifier[groupIndex][wqIndex]);
	       
                  free (wxQualifier[groupIndex]);	
               }

               free(wxCoverage);
               free(wxType);
               free(wxIntensity);
               free(wxVisibility);
               free(wxQualifier);
               free(Qualifier);	       
	    	    
               /* Re-initialize the WxGroups arrays. */
               memset(WxGroups, '\0', 5 * 100);	 
	    } /* End of if looking for the data that corresponds to a specific 
	       * day. 
	       */

         } /* End of "if weather row is missing" check. */
	 
      } /* End of for loop over all rows of weather data and checking if in 
	 * summarization period. 
	 */
     
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
         if (strcmp(dominantRowsWeather[2][groupIndex], "BS") == 0)
            f_isBlowingSnow = 1;
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
            percentTimeWithFog = (100*fogCount)/numDataRows;
         else
            percentTimeWithFog = 0;

         /* Determine the phrases and icon that goes with the weather 
	  * conditions. Initialize the flag determining if Pop is an issue for
	  * current period. 
	  */
	 f_popIsNotAnIssue = 0;
 	 generatePhraseAndIcons(dayIndex, frequency, timeLayoutHour, 
			        dominantWeather, baseURL, maxDailyPop, 
				averageSkyCover, maxSkyCover, minSkyCover, 
				maxSkyNum, minSkyNum, periodMaxTemp, 
				springDoubleDate, fallDoubleDate, 
				maxWindSpeed, maxWindDirection, startPositions, 
				endPositions, f_isDrizzle, f_isRain, 
				f_isRainShowers, f_isIcePellets, f_isSnow, 
				f_isSnowShowers, f_isFreezingDrizzle, 
				f_isFreezingRain, f_isBlowingSnow, wgInfo,
                                wsInfo, blizzardTime, blizzCnt, 
                                periodStartTimes[dayIndex], 
                                periodEndTimes[dayIndex], iconInfo, phrase, 
                                &f_popIsNotAnIssue, numActualRowsWS, 
                                numActualRowsWG, percentTimeWithFog, 
                                maxWindSpeedValTimes);

         if (strcmp(phrase[dayIndex],"none") == 0)
         { 
            /* We didn't find weather data and there was no sky information 
             * in this summary period, so format the nil attribute for the 
             * <weather-conditions> element.
             */
            xmlNewProp(weather_conditions, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else
         {   

	    /* Need to get weather phrase by this point. We will insert the 
	     * weather phrase as "weather-summary", an attribute of the 
	     * <weather-conditions> element.  
	     */
	    xmlNewProp(weather_conditions, BAD_CAST "weather-summary", BAD_CAST
	               phrase[dayIndex]);
	    if (strcmp(dominantWeather[0], "none") != 0 && f_popIsNotAnIssue)
            {
               /* Format the XML. */
		    	 
               /* Loop over each group and format the <value> element (if wanted)
                * and each of it's four attributes (coverage, type, intensity, and
                * qualifier(s) (NOTE -- visibility is not formatted, but only used 
                * in generating "Blizzard" as the weather phrase). 
	        */
               if (format_value)
               {
                  for (i = 0; i < numDominantTypes + 1; i++)
	          {
                     value = xmlNewChild(weather_conditions, NULL, BAD_CAST
                                         "value", NULL);
                     getTranslatedCoverage(dominantRowsWeather[0][i], 
		    	                   transCoverageStr);
                     getTranslatedType(dominantRowsWeather[2][i], transTypeStr);
                     getTranslatedIntensity(dominantRowsWeather[1][i], 
                                            transIntensityStr);

                     xmlNewProp(value, BAD_CAST "coverage", BAD_CAST
                                transCoverageStr);
                     xmlNewProp(value, BAD_CAST "intensity", BAD_CAST
                                transIntensityStr);

                     if (i > 0) /* Groups other than first require additive 
                                 * attribute. Check if this attribute is "or" or 
                                 * "and".
			         */
	             {
	                strcpy(additive_value, "and");
		  
		        strcpy (tempstore, dominantRowsWeather[4][i]);
		        token = strtok(tempstore, " ,");
		        if (token != NULL)
	                {
	                   if (strcmp(token, "or") == 0)
		              strcpy(additive_value, "or");
	                   else
		           {
		              while (token != NULL)
		              {
		                 token = strtok (NULL, " ,");
			   
			         if (token != NULL)
		                 {
	                            if (strcmp(token, "or") == 0)
		                    {		   
		                       strcpy(additive_value, "or");
			               break;
			            }
			         }
			      }
	                   }
		        }
		              
                        xmlNewProp(value, BAD_CAST "additive", BAD_CAST
                                   additive_value);

                        /* Set all characters of tempstore to blanks */
                        memset(tempstore,' ',sizeof(tempstore)); 

                        /* Set the last character of tempstore to a NULL */
                        memset(tempstore+199,'\0',1);	       }

                     xmlNewProp(value, BAD_CAST "weather-type", BAD_CAST
                                transTypeStr);
                     xmlNewProp(value, BAD_CAST "qualifier", BAD_CAST
                                dominantRowsWeather[4][i]);

	          }
	       }
            }
         }
      }
      else /* We didn't find data, so format the nil attribute for the <weather-
	    * conditions> element. 
	    */
         xmlNewProp(weather_conditions, BAD_CAST "xsi:nil", BAD_CAST "true");
    
      /* Free the dominantRowsWeather and dominantWeather arrays. */
      for (i = 0; i < 5; i++) 
      {
	 free (dominantWeather[i]);

         for (j = 0; j < 5; j++) /* Up to 5 weather groups per 1 wx string. */
	    free (dominantRowsWeather[i][j]);
      }   
      
   } /* End of dayIndex loop (the loop containing all data for one such
      * summarization (12 or 24 hour) period. 
      */ 

   /* Having saved the appropriate weather icons paths and files names in the
    * structure iconInfo, format the XML to hold the links. 
    */
   genIconLinks(iconInfo, *numOutputLines, layoutKey, 
	        parameters);

   /* Free some things. */
   for (i = 0; i < *numOutputLines; i++)
      free(phrase[i]);
   free(wsInfo);
   free(wgInfo);
   free(wxInfo);
   free(iconInfo);
   free(isDataAvailable);
   free(phrase);
   free(periodMaxTemp);
   free(periodStartTimes);
   free(periodEndTimes);
   if (blizzCnt > 0)
      free(blizzardTime);

   return;
   
}
