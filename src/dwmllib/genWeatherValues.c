/*****************************************************************************
 * genWeatherValues() -- 
 *
 * Paul Hershberg / MDL
 * Linux
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
 *    startNum = First index in match structure an individual point's data 
 *               matches can be found. (Input)
 *      endNum = Last index in match structure an individual point's data
 *               matches can be found. (Input)
 *      f_unit = 0 (GRIB unit), 1 (english), 2 (metric) (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2006 Paul Hershberg (MDL): Created.
 *  7/5/2006 Carl McCalla (MDL): Modified to handle up to five comma delimited 
 *                               hazards/qualifier strings
 *  7/2008 Paul Hershberg (MDL): Initialized valueIsMissing in wx index loop.
 *  4/2009 Paul Hershberg (MDL): Added code to deal with the missing colon 
 *                               delimiter in the ugly wx strings.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void genWeatherValues(size_t pnt, char *layoutKey, genMatchType *match,
                      uChar f_wx, int f_icon, numRowsInfo numRowsWS, 
                      numRowsInfo numRowsSKY, numRowsInfo numRowsTEMP, 
                      numRowsInfo numRowsWX, numRowsInfo numRowsPOP, 
                      xmlNodePtr parameters, double lat, double lon, 
                      int startNum, int endNum, sChar TZoffset, 
                      sChar f_observeDST, sChar f_unit)
{
   int i;                     /* Counter through match structure. */
   int priorElemCount;        /* Counter used to find elements' location in
                               * match. */
   int itIsNightTime = 0;     /* Denotes wether time is night or day. */
   int TZtotal = 0;           /* The total offset from GMT */
   int wxIndex = 0;           /* Counter for weather. */
   int wsIndex = 0;           /* Counter for wind speed. */
   int wqIndex = 0;           /* Counter for wx hazards/qualifiers.  
                               * CMc added on 7/5/06 */
   int indexOfindexes[5];     /* An array holding the # of hazards/qualifiers
			       * found in each wx grp. CMc added on 7/5/06 */
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
   int **anyOrs;              /* Denotes with a "1" if a certain group has any
			       * qualifiers that are = to "OR" in order to place
			       * "or" as the additive_value connecting more than
			       * one weather group.
			       */
   int *POP12SpreadToPOP3 = NULL; /* Array containing the PoP12 values spread 
				   * over all the weather times.
				   */
   char *addrtempstore;       /* Address of the tempstore variable holding each
			       * qualifier.
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
   char tempstore[200];       /* A temporary storage area. CMc added 7/5/06. */
   char *token;               /* Holds each comma delimited wx qualifier. */
   char ***wxQualifier = NULL;/* Char array holding up to 5 weather qualifiers 
			       * for up to 5 wx groups. */ 
                              /*  CMc added on 7/14/06  */
   char additive_value[10];   /* String placed in the second and subsequant
                               * weather groups to indicate how the data is
                               * combined ("and" or "or"). */
   char transCoverageStr[100];  /* String holding english translation of
                                 * weather coverage. */
   char transTypeStr[100];    /* String holding english translation of
                               * weather coverage. */
   char transIntensityStr[100]; /* String holding english translation of
                                 * weather intensity. */
   char transVisibilityStr[100];  /* String holding english translation of
                                   * weather visibility. */
   char transQualifierStr[200]; /* String holding english translation of
                                 * weather qualifiers. */
   char None[] = "<NoCov>:<NoWx>:<NoInten>:<NoVis>:"; /* Used as ugly wx string 
                                                         when ugly wx string 
                                                         consists of <None>. */
   xmlNodePtr weather = NULL; /* Xml Node Pointer for node "weather". */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for node "value". */
   xmlNodePtr visibility = NULL;  /* Xml Node Pointer for node "visibility". */
   xmlNodePtr weather_conditions = NULL;  /* Xml Node Pointer for node
                                           * "weather-conditions". */
   WX *wxInfo = NULL;         /* Weather data taken from the match array. */
   elem_def *wsInfo = NULL;   /* Wind Speed data taken from the match array. */
   elem_def *skyInfo = NULL;   /* Sky coverage data taken from the match
                               * array. */
   elem_def *tempInfo = NULL; /* Hourly Temp data taken from the match array. */ 
   elem_def *popInfo = NULL;  /* Pop12hr data takej from the match array. */ 
   icon_def *iconInfo = NULL; /* Array holding the icon information. */

/* Initialize the location where the weather icons are found. */
   char baseURL[] = "http://forecast.weather.gov/images/wtf/";

   /* Initialize a few things. */
   int numActualRowsSKY;   
   int numActualRowsTEMP; 
   int numActualRowsPOP; 
   int numActualRowsWX; 
   int numActualRowsWS; 

/************************************************************************/
   
   /* Initialize the actual number of rows we are working with for the 5
    * elements of interest. 
    */
   numActualRowsWS = numRowsWS.total-numRowsWS.skipBeg-numRowsWS.skipEnd;
   numActualRowsSKY = numRowsSKY.total-numRowsSKY.skipBeg-numRowsSKY.skipEnd;
   numActualRowsTEMP = numRowsTEMP.total-numRowsTEMP.skipBeg-numRowsTEMP.skipEnd;
   numActualRowsWX = numRowsWX.total-numRowsWX.skipBeg-numRowsWX.skipEnd;
   numActualRowsPOP = numRowsPOP.total-numRowsPOP.skipBeg-numRowsPOP.skipEnd;
   
/*  Determine if we need to format XML for weather data. */
   if (f_wx == 1 || f_icon)
   {
      /* Firstly, create arrays of structures holding applicable elements'
       * data info and time info from the match structure.  If icons are to
       * be formatted, then wind speed, sky cover, and temperatures must be
       * allocated here as they are used to derive icons. 
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
            wxInfo[i-priorElemCount-startNum].validTime =
                    match[i].validTime;
            if (match[i].value[pnt].valueType != 0 &&
                match[i].value[pnt].valueType != 2)
            {
               strcpy(wxInfo[i-priorElemCount-startNum].str, 
                      match[i].value[pnt].str);
            }
            wxInfo[i-priorElemCount-startNum].valueType =
                  match[i].value[pnt].valueType;
         }
         else
            priorElemCount++;
      }

      if (f_icon) /* Then Wind Speed, Sky Cover, Temperature and POP arrays are 
		   * needed.  Also, allocate room for the Icon Information, and
		   * set up the Pop 12 values that cover each Weather time.
		   */
      {
         wsInfo = malloc(numActualRowsWS * sizeof(elem_def));
         skyInfo = malloc(numActualRowsSKY * sizeof(elem_def));
	 popInfo = malloc(numActualRowsPOP * sizeof(elem_def));
         tempInfo = malloc(numActualRowsTEMP * sizeof(elem_def));
         iconInfo = malloc(numActualRowsWX * sizeof(icon_def));
	 POP12SpreadToPOP3 = (int *) malloc((numActualRowsWX) * sizeof(int));
	 
         /* Fill Wind Speed Array. */
         priorElemCount = 0;
         for (i = startNum; i < endNum; i++)
         {
            if (match[i].elem.ndfdEnum == NDFD_WS && 
	        match[i].validTime >= numRowsWS.firstUserTime &&
	        match[i].validTime <= numRowsWS.lastUserTime)
            {
               wsInfo[i-priorElemCount-startNum].validTime = 
                     match[i].validTime;
               wsInfo[i-priorElemCount-startNum].data =
                     (int)myRound(match[i].value[pnt].data, 0);
               wsInfo[i-priorElemCount-startNum].valueType =
                     match[i].value[pnt].valueType;
            }
            else
               priorElemCount++;
         }

         /* Fill Sky Cover Array. */
         priorElemCount = 0;
         for (i = startNum; i < endNum; i++)
         {
            if (match[i].elem.ndfdEnum == NDFD_SKY && 
	        match[i].validTime >= numRowsSKY.firstUserTime &&
	        match[i].validTime <= numRowsSKY.lastUserTime)
            {
               skyInfo[i-priorElemCount-startNum].validTime = match[i].validTime;
               skyInfo[i-priorElemCount-startNum].data =
                     (int)myRound(match[i].value[pnt].data, 0);
               skyInfo[i-priorElemCount-startNum].valueType =
                     match[i].value[pnt].valueType;
            }
            else
               priorElemCount++;
         }

         /* Fill Temperature Array. */
         priorElemCount = 0;
         for (i = startNum; i < endNum; i++)
         {
            if (match[i].elem.ndfdEnum == NDFD_TEMP && 
	        match[i].validTime >= numRowsTEMP.firstUserTime &&
	        match[i].validTime <= numRowsTEMP.lastUserTime)
            {
               tempInfo[i-priorElemCount-startNum].validTime = match[i].validTime;
               tempInfo[i-priorElemCount-startNum].data =
                     (int)myRound(match[i].value[pnt].data, 0);
               tempInfo[i-priorElemCount-startNum].valueType =
                     match[i].value[pnt].valueType;
            }
            else
               priorElemCount++;
         }

         /* Fill Pop12hr Array. */
         priorElemCount = 0;
         for (i = startNum; i < endNum; i++)
         {
            if (match[i].elem.ndfdEnum == NDFD_POP && 
	        match[i].validTime >= numRowsPOP.firstUserTime &&
	        match[i].validTime <= numRowsPOP.lastUserTime)
            {
               popInfo[i-priorElemCount-startNum].validTime = match[i].validTime;
               popInfo[i-priorElemCount-startNum].data =
                       (int)myRound(match[i].value[pnt].data, 0);
	       popInfo[i-priorElemCount-startNum].valueType =
                       match[i].value[pnt].valueType;
            }
            else
               priorElemCount++;
         }

	 /* Get array holding POP12 values concurrent with weather times. */ 
         spreadPOPsToWxTimes(POP12SpreadToPOP3, wxInfo, numActualRowsWX, popInfo, 
		             numActualRowsPOP);
      }
      
      if (f_wx == 1) /* Format the weather and display name elements. */
      {
         weather = xmlNewChild(parameters, NULL, BAD_CAST "weather", NULL);
         xmlNewProp(weather, BAD_CAST "time-layout", BAD_CAST layoutKey);

         /* Format the display name. */
         xmlNewChild(weather, NULL, BAD_CAST "name", BAD_CAST
                     "Weather Type, Coverage, and Intensity");
      }

      /* Loop over just the Wx data values now and format them. */
      for (wxIndex = 0; wxIndex < numActualRowsWX; wxIndex++)
      {
         /* Initialize/Reset a few things. */
         skyCoverTimeEqualsWeatherTime = 0;
         windTimeEqualsWeatherTime = 0;
         hourlyTempTimeEqualsWeatherTime = 0;
         itIsNightTime = 0;
         memset(WxValues, '\0', 5 * 50);
         memset(WxGroups, '\0', 10 * 100);
         numGroups = 0;
         valueIsMissing = 0;

         if (wxInfo[wxIndex].valueType == 2)
            valueIsMissing = 1;

         /* if the "ugly weather string" consists solely of "<None>" 
          * then treat it as if it is "<NoCov>:<NoWx>:<NoInten>:<NoVis>:".
          * Next, check if there are no delimiters (colons) after looking 
          * for <None>. If this is the case, treat current weather row as 
          * non-valid data. 
          */
         if (strstr(wxInfo[wxIndex].str, ":") == '\0')
         {
            if (strstr(wxInfo[wxIndex].str, "<None>") != '\0')
               strcpy(wxInfo[wxIndex].str, None);
            else
               valueIsMissing = 1;
         }

         if (valueIsMissing != 1)
         {
            /* Determine if this interation is occuring during the day or
             * night. First, calculate the total offset, in hours, from GMT.
             */
            TZtotal = TZoffset;
            if (f_observeDST)
            {
               if (Clock_IsDaylightSaving2(wxInfo[wxIndex].validTime, 0) == 1)
                  TZtotal = TZoffset - 1;
            }

            itIsNightTime = isNightPeriod(wxInfo[wxIndex].validTime, lat, lon, TZtotal);

            /* Lets remove the <> that surround the weather data (type, 
             * coverage, intensity, visibility, and qualifier) coming from
             * NDFD. 
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

            /* Now put the weather groupings into an array using the "^" as the
             * delimiter. Fill the first array elements before the others. 
             */
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
             * data. 
	     */
            pstr1 = strchr(wxInfo[wxIndex].str, '^');
            while (pstr1 != NULL)
            {
               numGroups++;
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

            foundWeatherValue = 0;

            /* Determine if the sky cover, temperatures, and wind have the
             * same time as weather.  There are times when weather is every 3
             * hours while wind temp and sky cover are every 6 hours. Only need
             * to do this if icons are to be derived. 
             */
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

            /* For each group, process its weather information (i.e.
             * type, coverage intensity, visibility, and qualifier(s)), 
	     * all which are denoted by a ":". 
	     */
            wxCoverage = (char **)calloc(numGroups + 1, sizeof(char *));
            wxType = (char **)calloc(numGroups + 1, sizeof(char *));
            wxIntensity = (char **)calloc(numGroups + 1, sizeof(char *));
            wxVisibility = (char **)calloc(numGroups + 1, sizeof(char *));
            wxQualifier = (char ***)calloc(numGroups + 1, sizeof(char **));
            anyOrs = (int **)calloc(numGroups + 1, sizeof(int *));
	    
            /* Loop over each group. */
            for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
            {
	       /* Initialize the number of weather values (coverage, type,
		* intensity, visibility, qualifier(s)) per group. 
		*/
               numValues = 0;
	       
	       /* Initialize the 5 wxQualifier possibilities to "none".
		* Initialize array elements denoting if a qualifier = "OR" 
		* to 0. 
		*/
	       wxQualifier[groupIndex] = (char **) malloc (5 * sizeof(char *));
	       anyOrs[groupIndex] = (int *) malloc (5 * sizeof(int));
	       
	       for (wqIndex = 0; wqIndex < 5; wqIndex++)
	       {
		  wxQualifier[groupIndex][wqIndex] = (char *) malloc 
			     ((strlen("none")+1) * sizeof(char));
	          strcpy(wxQualifier[groupIndex][wqIndex], "none");
		  anyOrs[groupIndex][wqIndex] = 0;
	       }

               /* Create the associative array holding the weather info
                * fields. 
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
                  numValues++;
                  pstr1 = strchr(pstr1 + 1, ':');
               }

               /* Bump this number up by one to account for the Wx Qualifier. 
                */
               numValues ++;

               /* Continue filling the array of WxValues. */
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

               /* Weather Qualifier(s) */
	       
               /* Initialize array of the # of qualifiers associated with each 
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
		  
                  if (token != NULL) 
		  {
                     wxQualifier[groupIndex][wqIndex] = (char *) realloc
	                (wxQualifier[groupIndex][wqIndex], 
			(strlen(token) + 1) * sizeof(char));
                     strcpy(wxQualifier[groupIndex][wqIndex], token);
		     if (strcmp (token, "OR") == 0)
		        anyOrs[groupIndex][wqIndex] = 1;
                       
                     while (token != NULL) /* Check for up to 4 more qualifiers. */
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

		           if (strcmp (token, "OR") == 0)
		              anyOrs[groupIndex][wqIndex] = 1;
                        }
                     }
                  }
               }

               /* Set the no weather occuring flag. */
               if (strcmp(wxType[groupIndex], "none") != 0)
               {
                  foundWeatherValue = 1;
               }

               /* Re-initialize the WxValues array. */
               memset(WxValues, '\0', 5 * 50);

            }  /* Closing out groupIndex for loop */

            /* If there is data we format it into a weather conditions
             * element. 
             */
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
                     getTranslatedVisibility(f_unit, wxVisibility[groupIndex],
                                             transVisibilityStr);

                     /* Initialize variables in preparation for translating the 
                      * hazards/qualifiers.
                      */
                       
                     /* First, set all characters of tempstore to blanks. */
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

                        if (wqIndex != 0 && wqIndex <= indexOfindexes[groupIndex]) 
			{
                           addrtempstore = &tempstore[0]+strlen(tempstore);
                           strncat(addrtempstore,",",1);
                           addrtempstore += 1;
                           strcat(addrtempstore,transQualifierStr);
                        }
                     }

                     /* Copy complete translated hazards string to 
		      * transQualifierStr.
		      */
                     strcpy (transQualifierStr, tempstore); 
		     
                     /* Format the coverage, intensity, qualifier(s), 
		      * visibility, and  intensity. 
		      */
                     xmlNewProp(value, BAD_CAST "coverage", BAD_CAST
                                transCoverageStr);
                     xmlNewProp(value, BAD_CAST "intensity", BAD_CAST
                                transIntensityStr);

                     if (groupIndex > 0)  /* Groups other than first require
                                           * additive attribute. */
                     {
	                strcpy(additive_value, "and");
     	                for (wqIndex = 0; wqIndex < 5; wqIndex++)
			{
			   if (anyOrs[groupIndex][wqIndex] == 1)
                              strcpy(additive_value, "or");
			}
			
                        xmlNewProp(value, BAD_CAST "additive", BAD_CAST
                                   additive_value);
                     }

                     xmlNewProp(value, BAD_CAST "weather-type", BAD_CAST
                                transTypeStr);
                     xmlNewProp(value, BAD_CAST "qualifier", BAD_CAST
                                transQualifierStr);

                     /* Format visibility as a seperate element (not an
                      * attribute). If no visibility restriction, format a
                      * "nil" attribute. 
                      */
                     if (strcmp(wxVisibility[groupIndex], "none") == 0 ||
                         wxVisibility[groupIndex] == NULL)
                     {
                        visibility = xmlNewChild(value, NULL, BAD_CAST
                                                 "visibility", NULL);
                        xmlNewProp(visibility, BAD_CAST "xsi:nil", BAD_CAST
                                   "true");
                     }
                     else /* Format the visibility data. */
                     {
                        visibility = xmlNewChild(value, NULL, BAD_CAST
                                                 "visibility",
                                                 BAD_CAST transVisibilityStr);
                        if (f_unit != 2)
                           xmlNewProp(visibility, BAD_CAST "units",
                                      BAD_CAST "statute miles");
                        else
                           xmlNewProp(visibility, BAD_CAST "units",
                                      BAD_CAST "meters");
                     }
                  }
               }

               /* Create and then save the weather icon based on forecast
                * weather types. 
		*/
               if (f_icon)
               {
                  determineWeatherIcons(iconInfo, numGroups, wxType,
                                        skyCoverTimeEqualsWeatherTime,
                                        itIsNightTime, skyInfo, baseURL,
                                        numActualRowsSKY, skyIndex, wxIndex, 
					windTimeEqualsWeatherTime, wsInfo,
                                        wsIndex, numActualRowsWS, numActualRowsTEMP,
                                        hourlyTempIndex,
                                        hourlyTempTimeEqualsWeatherTime,
                                        tempInfo, POP12SpreadToPOP3[wxIndex]);

                  /* Update the indexes. */
                  if (skyCoverTimeEqualsWeatherTime && skyIndex < numActualRowsSKY)
                     skyIndex += 1;

                  if (windTimeEqualsWeatherTime && wsIndex < numActualRowsWS)
                     wsIndex += 1;

                  if (hourlyTempTimeEqualsWeatherTime && hourlyTempIndex <
                      numActualRowsTEMP)
                     hourlyTempIndex += 1;
               }
            }
            else /* End of if "foundWeather" statement. No weather occurring
                  * so format empty weather conditions element. 
		  */
            {
               if (f_wx == 1)
                  weather_conditions = xmlNewChild(weather, NULL, BAD_CAST
                                                   "weather-conditions", NULL);
               if (f_icon)
               {
                  /* Determine the conditions icon element based on sky. */
                  determineSkyIcons(skyCoverTimeEqualsWeatherTime,
                                    itIsNightTime, skyIndex, wxIndex, skyInfo,
                                    iconInfo, baseURL, numActualRowsSKY);

                  /* Determine the conditions icon element based on things like
                   * extreme temperatures and strong winds. 
                   */
                  determineNonWeatherIcons(windTimeEqualsWeatherTime,
                                           itIsNightTime, wsInfo, wsIndex,
                                           baseURL, numActualRowsWS, iconInfo,
                                           wxIndex, numActualRowsTEMP, tempInfo,
                                           hourlyTempIndex,
                                           hourlyTempTimeEqualsWeatherTime);

                  /* Update the indexes. */
                  if (skyCoverTimeEqualsWeatherTime && skyIndex < numActualRowsSKY)
                     skyIndex++;

                  if (windTimeEqualsWeatherTime && wsIndex < numActualRowsWS)
                     wsIndex++;

                  if (hourlyTempTimeEqualsWeatherTime && hourlyTempIndex <
                      numActualRowsTEMP)
                     hourlyTempIndex++;
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
	       
            /* CMc added the next 4 lines (7/14/06) to free the space
	     * allocated for the wxQualifier array. 
	     */  
	       for (wqIndex = 0; wqIndex < 5; wqIndex++)
                  free(wxQualifier[groupIndex][wqIndex]);
	       
	       free(anyOrs[groupIndex]);
               free (wxQualifier[groupIndex]);	       
	    }	    
            free(wxCoverage);
            free(wxType);
            free(wxIntensity);
            free(wxVisibility);
            free(wxQualifier);
	    free(anyOrs);
         }

         else /* The "else" of the "if (valueIsMissing != 1)" statement. The
               * weather is missing, so format the "nil" attribute. 
               */
         {
            if (f_wx == 1)
            {
               weather_conditions = xmlNewChild(weather, NULL, BAD_CAST
                                                "weather-conditions", NULL);
               value = xmlNewChild(weather_conditions, NULL, BAD_CAST "value",
                                   NULL);
               xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
            }
            /* Since there is no data, indicate there is no icon, if 
             * applicable. 
             */
            if (f_icon)
            {
               strcpy(iconInfo[wxIndex].str, "none");

               /* Update indexes. */
               if (skyCoverTimeEqualsWeatherTime && skyIndex < numActualRowsSKY)
                  skyIndex++;

               if (windTimeEqualsWeatherTime && wsIndex < numActualRowsWS)
                  wsIndex++;

               if (hourlyTempTimeEqualsWeatherTime && hourlyTempIndex <
                   numActualRowsTEMP)
                  hourlyTempIndex++;
            }
         }
      } /* End of formatting weather-conditions element (end of wxIndex "for" 
         * loop). 
         */

      /* Only incorporate/format weather element if it is needed. This could
       * be just a "weather conditions icon" scenario, which is possible in the 
       * DWMLgen "time-series" product if user only selects "Weather Conditions
       * Icons" to format. Of course, these icons are based on Weather, it's
       * just that the weather is not formatted to output in this instance.
       * If both are to be formatted, Weather comes before conditions icons 
       * based on sequence in schema. 
       */

      /* Having saved the appropriate weather icons paths and file names in
       * the array iconLinks, format the XML to hold the links. 
       */

      if (f_icon)
      {
         genIconLinks(iconInfo, numActualRowsWX, layoutKey, parameters);
         free(iconInfo);
      }

      /* Free some things. */
      free(wxInfo);
      if (f_icon)
      {
         free(wsInfo);
         free(skyInfo);
         free(tempInfo);
	 free(popInfo);
	 free(POP12SpreadToPOP3);
      }

   }

   return;
}
