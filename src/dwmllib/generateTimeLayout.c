/*****************************************************************************
 * generateTimeLayout () -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This routine creates the XML time layout for NDFD parameters. There is
 *  one layout for each unique combination of data start time, period length, 
 *  and number of data values. The maximum temperature time layout would look 
 *  like the following:
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
 *    parameterEnum = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *          numRows = Structure containing members: (Input)
 *                    total: Total number of rows data is formatted for in the 
 *                           output XML. Used in DWMLgenByDay's "12 hourly" and 
 *                           "24 hourly" products. "numRows" is determined 
 *                           using numDays and is used as an added criteria
 *                           (above and beyond simply having data exist for a 
 *                           certain row) in formatting XML for these two 
 *                           products. (Input)
 *                  skipBeg: the number of beginning rows not formatted due 
 *                           to a user supplied reduction in time (startTime
 *                           arg is not = 0.0)
 *                  skipEnd: the number of end rows not formatted due to a 
 *                           user supplied reduction in time (endTime arg
 *                           is not = 0.0)
 *            firstUserTime: the first valid time interested per element, 
 *                           taking into consideration any data values 
 *                           (rows) skipped at beginning of time duration.
 *             lastUserTime: the last valid time interested per element, 
 *                           taking into consideration any data values 
 *                           (rows) skipped at end of time duration.
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
 *    startTime_cml = Incoming argument set by user as a double in seconds 
 *                    since 1970 denoting the starting time data was retrieved
 *                    for from NDFD. (Input)
 *      numFmtdRows = For DWMLgenByDay products, the number of rows formatted 
 *                    is set and not based off of the Match structure. (Input)
 *            f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's NDFD "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product. (Input)
 *                     5 = DWMLgen's RTMA "time-series" product. 
 *                     6 = DWMLgen's RTMA & NDFD "time-series" product. 
 *         startNum = First index in match structure an individual point's data 
 *                    matches can be found. (Input)
 *           endNum = Last index in match structure an individual point's data
 *                    matches can be found. (Input)
 *   
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *  2/2008 Paul Hershberg (MDL): Removed "period" as argument to 
 *                               checkNeedForPeriodName() routine.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void generateTimeLayout(numRowsInfo numRows, uChar parameterEnum,
                        char *layoutKey, const char *timeCoordinate,
                        char *summarization, genMatchType * match,
                        size_t numMatch, uChar f_formatPeriodName,
                        double TZoffset, sChar f_observeDST,
                        size_t * numLayoutSoFar,
                        uChar * numCurrentLayout, char *currentHour,
                        char *currentDay, char *frequency,
                        xmlNodePtr data, double startTime_cml,
                        double currentDoubTime, int *numFmtdRows,
			uChar f_XML, int startNum, int endNum)
{
   int i;                     /* Counter thru match structure. */
   int f_finalTimeLayout = 0; /* Flag denoting if this is the last time
                               * layout being processed. */
   int period = 0;            /* Length between an elements successive
                               * validTimes. */
   int periodClimate = 0;      /* Length between an elements successive 
                                * validTimes in days or months (for climate
                                * outlook products). */
   int numActualRows; /* (numRows - those skipped) due to user shortening time
                       * data was retrieved for. */
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
   
   /* Set the number of actual rows. */
   numActualRows = numRows.total-numRows.skipBeg-numRows.skipEnd;

   /* If DWMLgen product, set numFmtdRows = to numRows. */
   if (f_XML == 1 || f_XML == 2 || f_XML == 5 || f_XML == 6)
      *numFmtdRows = numActualRows;

   /* Find first and second validTime per element (if exists) interested in. */
   getFirstSecondValidTimes(&firstValidTime, &secondValidTime, match, numMatch, 
		            parameterEnum, startNum, endNum, numRows.total, 
                            numRows.skipBeg, numRows.skipEnd);

   /* Start filling in the time layout array's  with this current data. */
   formatValidTime(firstValidTime, currentTimeLayout.fmtdStartTime, 30, 
		   TZoffset, f_observeDST);

   /* Get the period length in hours using either the period name or the valid 
    * times. 
    */
   if (parameterEnum == NDFD_MAX || parameterEnum == NDFD_MIN)
      period = 24;
   else if (parameterEnum == NDFD_POP)
      period = 12;

   /* Force the LAMP Tstorm Probabilities to 2 hrs (since the first 6 hours are 
    * only one hours apart, and determinePeriodLength routine will format this
    * 1hr period).
    */
   else if (parameterEnum == LAMP_TSTMPRB)
      period = 2;
   else if (parameterEnum == RTMA_TEMP || parameterEnum == RTMA_TD || 
            parameterEnum == RTMA_WSPD || parameterEnum == RTMA_WDIR || 
            parameterEnum == RTMA_PRECIPA || parameterEnum == RTMA_SKY ||
            parameterEnum == NDFD_WWA)
      period = 1;
   else /* Calculate it */
      period = determinePeriodLength(firstValidTime, secondValidTime, 
		                     numActualRows, parameterEnum);

   /* Fill the rest of the time layout array with current data. */
   currentTimeLayout.period = period;
   currentTimeLayout.numRows = numActualRows;

   /* Determine if this layout information has already been formatted. */
   if (isNewLayout(currentTimeLayout, numLayoutSoFar, numCurrentLayout,
                   f_finalTimeLayout) == 1)
   {
      /* Create the new key and then bump up the number of layouts by one.
       * We'll use days and not hours as period unit for climate products with
       * a period between 48 hours and 672 hours (28 days). Use months for
       * those climate products with period > 672 hours (28 days).
       */
      if (period >= 672)
      {
         periodClimate = (int)(myRound((period/24/30), 0));
         sprintf(layoutKey, "k-p%dm-n%d-%d", periodClimate, *numFmtdRows, 
	         *numLayoutSoFar);
      }
      else if (period > 48)
      {
         periodClimate = (int)(myRound((period/24), 0));
         sprintf(layoutKey, "k-p%dd-n%d-%d", periodClimate, *numFmtdRows, 
	         *numLayoutSoFar);
      }
      else if (period == 1 && parameterEnum == NDFD_WWA)
      {
         sprintf(layoutKey, "k-p%sh-n%d-%d", "1", *numFmtdRows, 
	         *numLayoutSoFar);
      }
      else
         sprintf(layoutKey, "k-p%dh-n%d-%d", period, *numFmtdRows, 
	         *numLayoutSoFar);

      *numLayoutSoFar += 1;
      
      /* See if we need to format an <end-valid-time> tag . */
      useEndTimes = checkNeedForEndTime(parameterEnum, f_XML);

      /* Some parameters like max and min temp don't have valid times that
       * match the real start time. So make the adjustment. 
       */
      if (*numFmtdRows > numActualRows) /* For summary products with a set 
                                         * number of rows to format. */
      {
         startTimes = (char **)malloc(*numFmtdRows * sizeof(char *));
         if (useEndTimes)
             endTimes = (char **)malloc(*numFmtdRows * sizeof(char *));
         computeStartEndTimes(parameterEnum, *numFmtdRows, period, TZoffset,
                              f_observeDST, match, useEndTimes, startTimes, 
                              endTimes, frequency, f_XML, startTime_cml, 
                              currentDoubTime, numRows, startNum, endNum);
      }
      else
      {
         startTimes = (char **)malloc(numActualRows* sizeof(char *));
         if (useEndTimes)
            endTimes = (char **)malloc(numActualRows * sizeof(char *));
         computeStartEndTimes(parameterEnum, *numFmtdRows, period, TZoffset,
                              f_observeDST, match, useEndTimes, startTimes, 
                              endTimes, frequency, f_XML, startTime_cml, 
                              currentDoubTime, numRows, startNum, endNum);
      }

      /* Format the XML time layout in the output string. */
      time_layout = xmlNewChild(data, NULL, BAD_CAST "time-layout", NULL);
      xmlNewProp(time_layout, BAD_CAST "time-coordinate", BAD_CAST
                 timeCoordinate);
      xmlNewProp(time_layout, BAD_CAST "summarization", BAD_CAST summarization);
      layout_key = xmlNewChild(time_layout, NULL, BAD_CAST "layout-key",
                               BAD_CAST layoutKey);

      /* Before looping throught the valid times determine the period
       * information "issuanceType" and "numPeriodNames". 
       */
      if (f_formatPeriodName && period >= 12)
         getPeriodInfo(parameterEnum, startTimes[0], currentHour, currentDay,
                       &issuanceType, &numPeriodNames, period, frequency);
      
      /* Now we get the time values for this parameter and format the valid time
       * tags. 
       */
      for (i = 0; i < *numFmtdRows; i++)
      {
         if (i < *numFmtdRows) /* Accounts for DWMLgenByDay. */
         {

	    if (startTimes[i])
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
                                            TZoffset, parameterEnum,
                                            startTimes[i],
                                            &outputPeriodName, issuanceType,
                                            periodName, currentHour, 
					    currentDay, startTime_cml, 
					    currentDoubTime, firstValidTime);
		  }

                  /* Handle each special period name (up to 3 of them). */
                  if (outputPeriodName)
                  {
                     xmlNewProp(startValTime, BAD_CAST "period-name", BAD_CAST
                                periodName);
                  }		  
                  else
                   /* We are past special names occurring during the 
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
                * foodNameTest, periodName)rmat it. 
		*/
               if (useEndTimes)
                  xmlNewChild(time_layout, NULL, BAD_CAST "end-valid-time",
                              BAD_CAST endTimes[i]);
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
      if (*numFmtdRows > numActualRows) 
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
         for (i = 0; i < numActualRows; i++)
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
   else /* Not a new key so just return the key name */
   {
      if (period >= 672)
      {
         periodClimate = (int)(myRound((period/24/30), 0));
         sprintf(layoutKey, "k-p%dm-n%d-%d", periodClimate, *numFmtdRows, 
	         *numCurrentLayout);
      }
      else if (period > 48)
      {
         periodClimate = (int)(myRound((period/24), 0));
         sprintf(layoutKey, "k-p%dd-n%d-%d", periodClimate, *numFmtdRows, 
	         *numCurrentLayout);
      }
      else if (period == 1 && parameterEnum == NDFD_WWA)
      {
         sprintf(layoutKey, "k-p%sh-n%d-%d", "1", *numFmtdRows, 
	         *numCurrentLayout);
      }
      else
         sprintf(layoutKey, "k-p%dh-n%d-%d", period, *numFmtdRows, 
	         *numCurrentLayout);
   }
   
   return;
}
