/*****************************************************************************
 * generateConcatTimeLayout () -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This routine creates the XML time layouts for the 6 Concatenated RTMA + NDFD
 *  elements in the DWMLgen RTMA+NDFD time-series product (f_XML = 6). The six 
 *  elements are:
 *
 *      RTMA Temp + NDFD Temp.
 *      RTMA Dew Point + NDFD Dew Point.
 *      RTMA Wind Speed + NDFD Wind Speed
 *      RTMA Wind Direction + NDFD Direction
 *      RTMA Sky Cover + NDFD Sky Cover
 *      RTMA Precipitation Amount + NDFD QPF
 *
 *  Code first formats the RTMA Time Layout, then the NDFD portion of the
 *  Time Layout. 
 *
 * ARGUMENTS
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
 *       concatNdfdEnum = Enumerated number of the concatenated RTMA+NDFD element
 *                        (Input).
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
 *  2/2008 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void generateConcatTimeLayout(numRowsInfo *numRows, int elemIndex, 
                              uChar concatNdfdEnum, char *layoutKey, 
                              const char *timeCoordinate,
                              char *summarization, genMatchType *match,
                              size_t numMatch, uChar f_formatPeriodName,
                              sChar TZoffset, sChar f_observeDST,
                              size_t *numLayoutSoFar,
                              uChar *numCurrentLayout, char *currentHour,
                              char *currentDay, char *frequency,
                              xmlNodePtr data, double startTime_cml,
                              double currentDoubTime,
		      	      uChar f_XML, int startNum, int endNum, 
                              size_t numElem, genElemDescript *elem)
{
   int i;                     /* Counter thru match structure. */
   int j;                     /* Counter */
   int k;                     /* Counter. */
   int m;                     /* Counter thru start and endTimes. */
   int f_finalTimeLayout = 0; /* Flag denoting if this is the last time
                               * layout being processed. */
   int period = 1;            /* Length between an elements successive
                               * validTimes. */
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
   int ndfdElemIndex = concatNdfdEnum; /* The enumerated number of the NDFD 
                                          portion of the concatenated element. */
   int rtmaElemIndex = concatNdfdEnum; /* The enumerated number of the RTMA 
                                          portion of the concatenated element. */
   int concatNumRows = numRows[elemIndex].total;
   int elemNumRows = 0; /* Variable used when accessing the time layout loop 
                           twice (once for the RTMA portion and once for the NDFD 
                           portion). */

   /* Which of the 6 concatenated elements do we have? Set the number of actual
    * rows and associated enumerated element numbers. 
    */
   if (concatNdfdEnum == RTMA_NDFD_TEMP)
   {
      /* Find the number of the parent elements. */
      for (k = 0; k < numElem; k++)
      {
         if (elem[k].ndfdEnum == NDFD_TEMP)
            ndfdElemIndex = k;
         if (elem[k].ndfdEnum == RTMA_TEMP)
            rtmaElemIndex = k;
      }
   }
   else if (concatNdfdEnum == RTMA_NDFD_TD)
   {
      for (k = 0; k < numElem; k++)
      {
         if (elem[k].ndfdEnum == NDFD_TD)
            ndfdElemIndex = k;
         if (elem[k].ndfdEnum == RTMA_TD)
            rtmaElemIndex = k;
      }
   }
   else if (concatNdfdEnum == RTMA_NDFD_WSPD)
   {
      for (k = 0; k < numElem; k++)
      {
         if (elem[k].ndfdEnum == NDFD_WS)
            ndfdElemIndex = k;
         if (elem[k].ndfdEnum == RTMA_WSPD)
            rtmaElemIndex = k;
      }
   }
   else if (concatNdfdEnum == RTMA_NDFD_WDIR)
   {
      for (k = 0; k < numElem; k++)
      {
         if (elem[k].ndfdEnum == NDFD_WD)
            ndfdElemIndex = k;
         if (elem[k].ndfdEnum == RTMA_WDIR)
            rtmaElemIndex = k;
      }
   }
   else if (concatNdfdEnum == RTMA_NDFD_PRECIPA)
   {
      for (k = 0; k < numElem; k++)
      {
         if (elem[k].ndfdEnum == NDFD_QPF)
            ndfdElemIndex = k;
         if (elem[k].ndfdEnum == RTMA_PRECIPA)
            rtmaElemIndex = k;
      }
   }
   else if (concatNdfdEnum == RTMA_NDFD_SKY)
   {
      for (k = 0; k < numElem; k++)
      {
         if (elem[k].ndfdEnum == NDFD_SKY)
         {
            ndfdElemIndex = k;
         }
         if (elem[k].ndfdEnum == RTMA_SKY)
         {
            rtmaElemIndex = k;
         }
      }
   }
   /* Find first and second validTime per element (if exists) interested in. 
    * This will involve the RTMA element since RTMA times start before NDFD
    * times.
    */
   getFirstSecondValidTimes(&firstValidTime, &secondValidTime, match, 
		            numMatch, elem[rtmaElemIndex].ndfdEnum, startNum, 
                            endNum, numRows[rtmaElemIndex].total, 
                            numRows[rtmaElemIndex].skipBeg, 
                            numRows[rtmaElemIndex].skipEnd);

   /* Start filling in the time layout array's with this current data. */
   formatValidTime(firstValidTime, currentTimeLayout.fmtdStartTime, 30, 
		   TZoffset, f_observeDST);

   /* Fill the rest of the time layout array with current data. */
   currentTimeLayout.period = period;
   currentTimeLayout.numRows = concatNumRows;

   /* Determine if this layout information has already been formatted. */
   if (isNewLayout(currentTimeLayout, numLayoutSoFar, numCurrentLayout,
                   f_finalTimeLayout) == 1)
   {
      /* Create the new key and then bump up the number of layouts by one. */
      sprintf(layoutKey, "k-p%dh-n%d-%d", period, concatNumRows,
	      *numLayoutSoFar);

      *numLayoutSoFar += 1;

      /* Format the XML time layout in the output string. */
      time_layout = xmlNewChild(data, NULL, BAD_CAST "time-layout", NULL);
      xmlNewProp(time_layout, BAD_CAST "time-coordinate", BAD_CAST
                 timeCoordinate);
      xmlNewProp(time_layout, BAD_CAST "summarization", BAD_CAST summarization);
      layout_key = xmlNewChild(time_layout, NULL, BAD_CAST "layout-key",
                               BAD_CAST layoutKey);

      /* Loop thru the two elements making up the concatenated element and get
       * each time layout. Start with the RTMA portion, and then the NDFD 
       * portion. 
       */
      for (i = rtmaElemIndex, j = 0; j < 2; j++)
      {
         /* See if we need to format an <end-valid-time> tag . */
         useEndTimes = checkNeedForEndTime(elem[i].ndfdEnum, f_XML);

         /* Gather the startTimes and/or endTimes. */
         elemNumRows = numRows[i].total - numRows[i].skipBeg -
                                          numRows[i].skipEnd;

         startTimes = (char **)malloc(elemNumRows * sizeof(char *));
         if (useEndTimes)
            endTimes = (char **)malloc(elemNumRows * sizeof(char *));
         computeStartEndTimes(elem[i].ndfdEnum, elemNumRows, period, TZoffset,
                              f_observeDST, match, useEndTimes, startTimes, 
                              endTimes, frequency, f_XML, startTime_cml, 
                              currentDoubTime, numRows[i], startNum, endNum);

         /* Before looping through the valid times determine the period
          * information "issuanceType" and "numPeriodNames". 
          */
         if (f_formatPeriodName && period >= 12)
            getPeriodInfo(elem[i].ndfdEnum, startTimes[0], currentHour, 
                          currentDay, &issuanceType, &numPeriodNames, period, 
                          frequency);
      
         /* Now we get the time values for the RTMA portion of this combined 
          * parameter and format the valid time tags. 
          */
         for (m = 0; m < elemNumRows; m++)
         {
	    if (startTimes[m])
            {
               startValTime = xmlNewChild(time_layout, NULL, BAD_CAST
                                          "start-valid-time", BAD_CAST
                                          startTimes[m]);

               /* We only format period names for parameters with period
                * greater than 12 hours like (max and min temp, and pop12
                * etc). 
                */
               if (f_formatPeriodName && period >= 12)
               {
                  outputPeriodName = 0;
                  periodName[0] = '\0';
                  Clock_Scan(&startTime_doub, startTimes[m], 1);
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
                     checkNeedForPeriodName(m, &numPeriodNames, TZoffset, 
                                            elem[i].ndfdEnum, startTimes[m],
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
                     if (useNightPeriodName(startTimes[m]) == 0)
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
                * format it. 
	        */
               if (useEndTimes)
                  xmlNewChild(time_layout, NULL, BAD_CAST "end-valid-time",
                              BAD_CAST endTimes[m]);

               /* Free up the individual startTime and endTime. */
               free(startTimes[m]);
               if (useEndTimes)
                  free(endTimes[m]);
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
         /* Set up some things for the second trip thru the loop. This will be 
          * for the NDFD portion of the concatenated element. Find first and 
          * second validTime for the NDFD Parent Element. Also, change the 
          * index to the NDFD Parent Element.
          */
         i = ndfdElemIndex;
         getFirstSecondValidTimes(&firstValidTime, &secondValidTime, match, 
		                  numMatch, elem[i].ndfdEnum, startNum, endNum,
                                  numRows[i].total, numRows[i].skipBeg, 
                                  numRows[i].skipEnd);

         /* Change the period to that of the parent Element for second and last 
          * iteration of loop. 
          */
         period = determinePeriodLength(firstValidTime, secondValidTime, 
		                      (numRows[i].total-numRows[i].skipBeg-numRows[i].skipEnd), 
                                      elem[i].ndfdEnum); 

         /* Free startTime & endTime arrays before next iteration of loop. */
         free(startTimes);
         if (useEndTimes)
            free(endTimes);   
      }
   }
   else /* Not a new key so just return the key name */
   {
      sprintf(layoutKey, "k-p%dh-n%d-%d", period, concatNumRows, 
	      *numCurrentLayout);
   }
   
   return;
}
