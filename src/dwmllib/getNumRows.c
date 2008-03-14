/******************************************************************************
 * getNumRows() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Code retrieves the number of rows of data (aka the number of data values) 
 *  for each element retrieved by degrib from NDFD. These are the elements  
 *  formatted and also any elements used to derive the formatted elements. Code
 *  also calcuates the number of rows skipped at the beginning and ending of a 
 *  time period if user has shortened the time period by setting startTime and
 *  endTime command line arguments. The first and last valid Times corresponding
 *  to the rows are also found. 
 *
 * ARGUMENTS
 *       f_XML = flag for 1 of the 4 DWML products (Input):
 *               1 = DWMLgen's "time-series" product. 
 *               2 = DWMLgen's "glance" product.
 *               3 = DWMLgenByDay's "12 hourly" format product.
 *               4 = DWMLgenByDay's "24 hourly" format product. (Input)
 *               5 = DWMLgen's RTMA "time-series" product.
 *               6 = DWMLgen's mix of "RTMA & NDFD time-series" product. 
 * numRowsForPoint = Structure with info on the number of rows data is formatted
 *                   for in the output XML (aka the number of data values). This 
 *                   number is point dependant. (Input/Output).
 *   timeUserStart = The beginning of the first forecast period (06 hr
 *                   or 18hr) based on the startTime argument. (Itput)
 *     timeUserEnd = The end of the last forecast period (18 hr) based 
 *                   on the startTime & numDays arguments. (Input)
 *           match = Pointer to the array of element matches from degrib. 
 *                   (Input) 
 *    wxParameters = Array containing the flags denoting whether a certain 
 *                   element is formatted in the output XML (= 1), or used in 
 *                   deriving another formatted element (= 2). (Input) 
 *        numMatch = The number of matches from degrib. (Input)
 *          f_icon = Flag denoting whether icons are to be derived and formatted.
 *                   If this flag is chosen, the other 4 elements' data used to 
 *                   derive the icons must be retrieved/derived too (WS, SKY, 
 *                   TEMP, WX). (Input)
 *        TZoffset = # of hours to add to current time to get GMT time.
 *                   (Input) 
 *    f_observeDST = Flag determining if current point observes Daylight 
 *                   Savings Time. (Input)   
 *       startDate = Point specific startDate that the user supplied 
 *                   startTime falls in (first Valid Match time if startTime
 *                   was not entered). It is the form (i.e. 2006-04-15).
 *                   (Intput) 
 *         numDays = The number of days the validTimes for all the data rows 
 *                   (values) consist of. (Input)
 *       startTime = Incoming argument set by user as a double in seconds 
 *                   since 1970 denoting the starting time data was retrieved
 *                   for from NDFD. (Set to 0.0 if not entered.) (Input) 
 *         endTime = Incoming argument set by user as a double in seconds 
 *                   since 1970 denoting the ending time data was retrieved
 *                   for from NDFD. (Set to 0.0 if not entered.) (Input) 
 *     currentHour = Current hour = in 2 digit form. (Input)
 *  firstValidTime_pop = The very first validTime for POP12hr returned from the
 *                       grid probe for this point. (Input) 
 * firstValidTimeMatch = The very first validTime for all matches returned 
 *                       from the grid probe for this point. (Input) 
 *        numSector = Number of sectors that points were found in. (Input)
 *                pnt = Number of current point being processed. (Input)
 * f_formatIconForPnt = Flag denoting if current point processes and formats
 *                      the Icon element. (Output)
 * f_formatSummarizations = Flag denoting if all 7 elements used in deriving 
 *                          the phrase/icon for the summarization products are
 *                          available. Flag denotes if an element has all missing
 *                          data (does not test for missing data per projection).
 *   pnt_rtmaNdfdTemp = Flag denoting that user queried both NDFD Temp and RTMA 
 *                      Temp. Thus, the two will be conjoined into one element. 
 *                      (Output)
 *     pnt_rtmaNdfdTd = Flag denoting that user queried both NDFD Td and RTMA 
 *                      Td. Thus, the two will be  conjoined into one element. 
 *                      (Output)
 *   pnt_rtmaNdfdWdir = Flag denoting that user queried both NDFD Wind Dir and 
 *                      RTMA Wind Dir. Thus, the two will be conjoined into one 
 *                      element.(Output)
 *   pnt_rtmaNdfdWspd = Flag denoting that user queried both NDFD Wind Spd 
 *                      and RTMA Wind Spd. Thus, the two will be conjoined 
 *                      into one element.(Output)
 * pnt_rtmaNdfdPrecipa = Flag denoting that user queried both NDFD QPF and RTMA 
 *                       Precip Amt. Thus, the two will be conjoined into one 
 *                       element.(Output)
 *    pnt_rtmaNdfdSky = Flag denoting that user queried both NDFD Sky Cover 
 *                      and RTMA Sky Cover. Thus, the two will be conjoined 
 *                      into one element.(Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2006 Paul Hershberg (MDL): Created
 *  10/2007 Paul Hershberg (MDL): Removed code that shifted data back by 1/2
 *                                the period length (bug from php code)
 *  10/2007 Paul Hershberg (MDL): Added if statement that only shortens 
 *                                timeUserStart and timeUserEnd by deltaSecs
 *                                if product is of summary type.
 *   2/2008 Paul Hershberg (MDL): Find number of rows generated for the 
 *                                concatenated RTMA-NDFD elements. 
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getNumRows(numRowsInfo *numRowsForPoint, double *timeUserStart, 
		double *timeUserEnd, size_t numMatch, genMatchType *match, 
                uChar *wxParameters, uChar f_XML, sChar *f_icon, sChar TZoffset, 
                sChar f_observeDST, int startNum, int endNum, char *startDate, 
                int *numDays, double startTime, double endTime, 
                char currentHour[3], double *firstValidTime_pop, 
                double *firstValidTimeMatch, int *f_formatIconForPnt, 
                int *f_formatSummarizations, int pnt, int *pnt_rtmaNdfdTemp, 
                int *pnt_rtmaNdfdTd, int *pnt_rtmaNdfdWdir, 
                int *pnt_rtmaNdfdWspd, int *pnt_rtmaNdfdPrecipa, 
                int *pnt_rtmaNdfdSky, double currentDoubTime)
{
   int i; /* Counter thru match structure. */
   int k; /* Counter thru elements */
   double period = 3; /* Element's periods (initialize to 3 hrs). */
   double timeDataEnd; /* End of time data is valid for (in secs since 1970). */
   double timeDataStart;/* Start of time data is valid for (in secs since 1970). */
   double timeUserStartPerElement; /* The beginning of the first forecast 
                                    * period (06 hr or 18hr) based on the 
                                    * startTime argument, and adjusted per 
                                    * element. */
   double timeUserEndPerElement;/* The end of the last forecast period
                                 * (06 hr or 18hr) based on the startTime 
                                 * argument, and adjusted per element. */
   double firstValidTime = 0.0; /* An element's 1st valid time. Used to
                                 * determine an element's period lenght. */
   double secondValidTime = 0.0; /* An element's 2nd valid time. Used to 
                                  * determine an element's period lenght. */
   double deltaSecs; /* Amount of secs time window is reduced to prevent data
                      * that just barely fits in the time window from getting
                      * in. */
   int f_POPUserStart = 0; /* Used to determine if startDates are different 
                            * between the very first match in the match 
                            * structure and the first POP12hr match. If so, the
                            * first 24 hr forecast period will begin on the 
                            * next day (from current day). */

   /* Initialize numRowsForPoint structure to all zeros. */
   for (k = 0; k < (NDFD_MATCHALL + 1); k++)
   {
      numRowsForPoint[k].total = 0;
      numRowsForPoint[k].skipBeg = 0;
      numRowsForPoint[k].skipEnd = 0;
      numRowsForPoint[k].firstUserTime = 0.0;
      numRowsForPoint[k].lastUserTime = 0.0;
   }
   
   /* Retrieve the total number of rows per element for the DWMLgen/DWMLgenByDay
    * products.
    */
   for (i = startNum; i < endNum; i++)
      numRowsForPoint[match[i].elem.ndfdEnum].total++;

   /* Retrieve the first validTime and last validTime per element amongst all
    * matches for the point.
    */
   for (k = 0; k < (NDFD_MATCHALL + 1); k++)
   {
      for (i = startNum; i < endNum; i++)
      {
	 if (match[i].elem.ndfdEnum == k)
	 {
            numRowsForPoint[k].firstUserTime = match[i].validTime;
	    numRowsForPoint[k].lastUserTime = match[i + (numRowsForPoint[k].total-1)].validTime;
	    break;
	 }
      }
   }

   /* Find the start of the User time interval the summarization is done for, 
    * modified to fit the first day's forecast period. If there is a startTime
    * on a day in the future other than the current day, we use the 06th hour.
    * If there is no startTime entered, the start of the next closest forecast 
    * period is chosen (06th or 18th hr). Routine then finds the end of the user 
    * time interval the summarization is done for, modified to fit the last 
    * day's forecast period (18th hr). Routine is only accessed if product type
    * is one of the summarizations (f_XML = 3 or f_XML = 4). 
    */
   if (f_XML == 3 || f_XML == 4)
   {
      getUserTimes(&timeUserStart, &timeUserEnd, &f_POPUserStart, startDate, 
                   TZoffset, startTime, f_observeDST, numDays, 
                   firstValidTime_pop, f_XML, firstValidTimeMatch);
   }
   else /* For DWMLgen products, simply assign startTime and endTime. */
   {
      *timeUserStart = startTime;
      *timeUserEnd = endTime;
   }
   
   /* Initialize the starting/ending times per element. */
   timeUserStartPerElement = *timeUserStart;
   timeUserEndPerElement = *timeUserEnd;  
   
   /* Adjust the number of rows per element and the first validTime and last 
    * validTime we're interested in. 
    */
   for (k = 0; k < (NDFD_MATCHALL + 1); k++)
   {
      if (wxParameters[k] != 0)
      {
	 if (k == NDFD_MAX || k == NDFD_MIN)
	    period = 12;
	 else
	 {
            getFirstSecondValidTimes(&firstValidTime, &secondValidTime, 
	   		             match, numMatch, k, startNum, endNum, 
		                     numRowsForPoint[k].total, 0, 0);
	    period = determinePeriodLength(firstValidTime, secondValidTime,
	                                   numRowsForPoint[k].total, k);
         }
   	 deltaSecs = (period / 4) * 3600;
	 
         /* Adjust the timeUserStart/timeUserEnd on an element basis, 
	  * if necessary. Tweek the user start and end times to prevent data 
	  * that just barely fits in the time window from getting in. We 
	  * reduce the window by 1/4th of the data's period length at each 
	  * end. We will only do this after 6 AM to allow for the overnight 
	  * minimum temperature to continue to be formatted from midnight to 
	  * 6 AM. Do this for summary products only.
          */
         if (f_XML == 3 || f_XML == 4)
         {	    
            if (atoi(currentHour) >= 6 && (k != NDFD_MIN || k != NDFD_POP))
            {
               timeUserStartPerElement = *timeUserStart + deltaSecs;
               timeUserEndPerElement = *timeUserEnd - deltaSecs;      
            }
            if (k == NDFD_POP)
            {
               if (!f_POPUserStart && startTime == 0.0 &&
               (atoi(currentHour) > 20 || atoi(currentHour) < 6))
               {
                  timeUserStartPerElement = *timeUserStart;
                  timeUserEndPerElement = *timeUserEnd;  
               }
               else
               {
                  timeUserStartPerElement = *timeUserStart + deltaSecs;
                  timeUserEndPerElement = *timeUserEnd - deltaSecs;
               } 
            }
            if (k == NDFD_MIN)
            {
               if (startTime == 0.0  && 
               (atoi(currentHour) > 18 || atoi(currentHour) < 6)) 
               {
                  timeUserStartPerElement = *timeUserStart;
                  timeUserEndPerElement = *timeUserEnd;
               } 
               else
               {
                  timeUserStartPerElement = *timeUserStart + deltaSecs;
                  timeUserEndPerElement = *timeUserEnd - deltaSecs;
               } 
            }
         }

	 /* Loop thru and make the adjustments to the number of rows interested
	  * in per element.
	  */ 
         for (i = startNum; i < endNum; i++)
         {
	    /* Since the data's validTime is the end of the time range the data
	     * is valid for, find the data's starting time.
	     */
	    if (match[i].elem.ndfdEnum == k)
	    {
	       timeDataEnd = match[i].validTime;
               timeDataStart = match[i].validTime - (3600 * period);
      
               /* Filter out RTMA elements with validTimes starting before 24 
                * hrs previous. 
                */
               if (match[i].elem.ndfdEnum == RTMA_TEMP || 
                   match[i].elem.ndfdEnum == RTMA_TD || 
                   match[i].elem.ndfdEnum == RTMA_WSPD || 
                   match[i].elem.ndfdEnum == RTMA_WDIR || 
                   match[i].elem.ndfdEnum == RTMA_PRECIPA || 
                   match[i].elem.ndfdEnum == RTMA_SKY)
               {
                  if (timeDataEnd < currentDoubTime - (25 * 3600))
	             numRowsForPoint[k].skipBeg++;
               }                  

       	       if (*timeUserStart != 0.0) /* Rule out DWMLgen cases where no startTime entered. */
	       {
	          if (match[i].elem.ndfdEnum != NDFD_POP)
                  {
                     if (timeDataEnd < timeUserStartPerElement)
	                numRowsForPoint[k].skipBeg++;
                  }
                  else if (match[i].elem.ndfdEnum == NDFD_POP && f_POPUserStart != 1)
                  {
                     if (timeDataEnd < timeUserStartPerElement)
	                numRowsForPoint[k].skipBeg++;
                  }                  
                  else if (match[i].elem.ndfdEnum == NDFD_POP && f_POPUserStart == 1)
                  {
                     if (timeDataEnd < (timeUserStartPerElement - (12 * 3600)))
	                numRowsForPoint[k].skipBeg++;
                  }      
	       }
       	       if (*timeUserEnd != 0.0) /* Rule out DWMLgen cases where no endTime entered. */	
	       {
	          if (timeDataStart > timeUserEndPerElement)
                     numRowsForPoint[k].skipEnd++;
	       }
	    }
	 }
      }
   }
   
   /* Adjust the first validTime and last validTime interested in, per 
    * element. 
    */
   for (k = 0; k < (NDFD_MATCHALL + 1); k++)
   {
      if ((wxParameters[k] != 0) && 
	  (numRowsForPoint[k].skipBeg != 0 || numRowsForPoint[k].skipEnd != 0))
      {
         for (i = startNum; i < endNum; i++)
	 {
	    if (match[i].elem.ndfdEnum == k)
	    {
               numRowsForPoint[k].firstUserTime = 
	          match[i+numRowsForPoint[k].skipBeg].validTime;
	       numRowsForPoint[k].lastUserTime = 
	          match[i + ((numRowsForPoint[k].total-1) - 
		  numRowsForPoint[k].skipEnd)].validTime;
	       break;
	    }
         }
      }
   }

   /* Since numRows determination is based off of what was actually returned
    * from NDFD, update the weatherParameters array accordingly to reflect
    * this. 
    */
   for (k = 0; k < (NDFD_MATCHALL + 1); k++)
   {
      /* If a concatenated element, check numRows for both the RTMA and NDFD
       * portion. 
       */
      if (k == RTMA_NDFD_TEMP)
      {
         if ((numRowsForPoint[RTMA_TEMP].total-
              numRowsForPoint[RTMA_TEMP].skipBeg- 
              numRowsForPoint[RTMA_TEMP].skipEnd == 0) || 
              (numRowsForPoint[NDFD_TEMP].total-
              numRowsForPoint[NDFD_TEMP].skipBeg- 
              numRowsForPoint[NDFD_TEMP].skipEnd == 0))
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdTemp = 0;
         }
      }
      else if (k == RTMA_NDFD_TD)
      {
         if ((numRowsForPoint[RTMA_TD].total-
              numRowsForPoint[RTMA_TD].skipBeg- 
              numRowsForPoint[RTMA_TD].skipEnd == 0) || 
              (numRowsForPoint[NDFD_TD].total-
              numRowsForPoint[NDFD_TD].skipBeg- 
              numRowsForPoint[NDFD_TD].skipEnd == 0))
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdTd = 0;
         }
      }
      else if (k == RTMA_NDFD_WSPD)
      {
         if ((numRowsForPoint[RTMA_WSPD].total-
              numRowsForPoint[RTMA_WSPD].skipBeg- 
              numRowsForPoint[RTMA_WSPD].skipEnd == 0) || 
              (numRowsForPoint[NDFD_WS].total-
              numRowsForPoint[NDFD_WS].skipBeg- 
              numRowsForPoint[NDFD_WS].skipEnd == 0))
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdWspd = 0;
         }
      }
      else if (k == RTMA_NDFD_WDIR)
      {
         if ((numRowsForPoint[RTMA_WDIR].total-
              numRowsForPoint[RTMA_WDIR].skipBeg- 
              numRowsForPoint[RTMA_WDIR].skipEnd == 0) || 
              (numRowsForPoint[NDFD_WD].total-
              numRowsForPoint[NDFD_WD].skipBeg- 
              numRowsForPoint[NDFD_WD].skipEnd == 0))
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdWdir = 0;
         }
      }
      else if (k == RTMA_NDFD_PRECIPA)
      {
         if ((numRowsForPoint[RTMA_PRECIPA].total-
              numRowsForPoint[RTMA_PRECIPA].skipBeg- 
              numRowsForPoint[RTMA_PRECIPA].skipEnd == 0) || 
              (numRowsForPoint[NDFD_QPF].total-
              numRowsForPoint[NDFD_QPF].skipBeg- 
              numRowsForPoint[NDFD_QPF].skipEnd == 0))
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdPrecipa = 0;
         }
      }
      else if (k == RTMA_NDFD_SKY)
      {
         if ((numRowsForPoint[RTMA_SKY].total-
              numRowsForPoint[RTMA_SKY].skipBeg- 
              numRowsForPoint[RTMA_SKY].skipEnd == 0) || 
              (numRowsForPoint[NDFD_SKY].total-
              numRowsForPoint[NDFD_SKY].skipBeg- 
              numRowsForPoint[NDFD_SKY].skipEnd == 0))
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdSky = 0;
         }
      }
      else
      { 
         if (wxParameters[k] >= 1 && (numRowsForPoint[k].total -
	   	                     numRowsForPoint[k].skipBeg -
			   	     numRowsForPoint[k].skipEnd) == 0)
            wxParameters[k] = 0;
      }
   }

   /* Now, check to see that Icons have all the necessary elements retrieved
    * from NDFD to derive them. 
    */
   if ((f_XML == 1 || f_XML == 2 || f_XML == 6) && *f_icon == 1)
   {
      if ((numRowsForPoint[NDFD_TEMP].total-numRowsForPoint[NDFD_TEMP].skipBeg -
	   numRowsForPoint[NDFD_TEMP].skipEnd) == 0 || 
          (numRowsForPoint[NDFD_WS].total-numRowsForPoint[NDFD_WS].skipBeg -
	   numRowsForPoint[NDFD_WS].skipEnd) == 0 ||
          (numRowsForPoint[NDFD_SKY].total-numRowsForPoint[NDFD_SKY].skipBeg -
	   numRowsForPoint[NDFD_SKY].skipEnd) == 0 || 
	  (numRowsForPoint[NDFD_WX].total-numRowsForPoint[NDFD_WX].skipBeg -
	   numRowsForPoint[NDFD_WX].skipEnd) == 0 ||
	  (numRowsForPoint[NDFD_POP].total-numRowsForPoint[NDFD_POP].skipBeg -
	   numRowsForPoint[NDFD_POP].skipEnd) == 0)
      {
         #ifdef PRINT_DIAG
         printf("**************************************\n");
         printf("Cannot format Icons at this time for\n");
         printf("point %d",(pnt+1)); 
         printf(" as element(s) used to derive\n");
         printf("icons are missing.\n");
         printf("**************************************\n");
         #endif
         *f_formatIconForPnt = 0;
      }
   }
   else if (f_XML == 3 || f_XML == 4)
   {
      /* We need all 7 elements to have data to create weather phrase/icon for the 
       * summarization period. If one is missing, don't format XML. This only checks
       * to see if ALL an elements's data is missing (not individual projections).
       */
      if (wxParameters[NDFD_MAX] == 0 || wxParameters[NDFD_MIN] == 0 ||
          wxParameters[NDFD_POP] == 0 || wxParameters[NDFD_WX] == 0 ||
          wxParameters[NDFD_SKY] == 0 || wxParameters[NDFD_WS] == 0 ||
          wxParameters[NDFD_WD] == 0)
      {
         *f_formatSummarizations = 0;
      }
   }

   /* Deal with the Concatenated RTMA-NDFD Elements. */
   if (f_XML == 6)
   {
      /* The concatenated RTMA + NDFD Temperature element. */
      if (wxParameters[RTMA_NDFD_TEMP] != 0)
      {
         if (numRowsForPoint[RTMA_TEMP].total-numRowsForPoint[RTMA_TEMP].skipBeg- 
                                         numRowsForPoint[RTMA_TEMP].skipEnd != 0)
         {
            numRowsForPoint[RTMA_NDFD_TEMP].total = numRowsForPoint[RTMA_TEMP].total + 
                                                    numRowsForPoint[NDFD_TEMP].total;
            numRowsForPoint[RTMA_NDFD_TEMP].skipBeg = numRowsForPoint[RTMA_TEMP].skipBeg;
            numRowsForPoint[RTMA_NDFD_TEMP].firstUserTime = 
                                            numRowsForPoint[RTMA_TEMP].firstUserTime;
         }
         else
            /* The user supplied startTime cut off all rows of the RTMA data for 
             * the concatenated Temp element. In that case, treat as if the 
             * concatenated element does not exist. 
             */
            *pnt_rtmaNdfdTemp = 0;

         if (numRowsForPoint[NDFD_TEMP].total-numRowsForPoint[NDFD_TEMP].skipBeg-
                                         numRowsForPoint[NDFD_TEMP].skipEnd != 0)
         {
            numRowsForPoint[RTMA_NDFD_TEMP].skipEnd = numRowsForPoint[NDFD_TEMP].skipEnd;
            numRowsForPoint[RTMA_NDFD_TEMP].lastUserTime = 
                                      numRowsForPoint[NDFD_TEMP].lastUserTime;
         }
         else
            /* The user supplied endTime cut off all rows of NDFD data for 
             * the concatenated Temp element. In that case, treat as if the 
             * concatenated element does not exist. 
             */
            *pnt_rtmaNdfdTemp = 0;
      }

      /* The concatenated RTMA + NDFD Dew Point element. */
      if (wxParameters[RTMA_NDFD_TD] != 0)
      {
         if (numRowsForPoint[RTMA_TD].total-numRowsForPoint[RTMA_TD].skipBeg-
                                       numRowsForPoint[RTMA_TD].skipEnd != 0)
         {
            numRowsForPoint[RTMA_NDFD_TD].total = numRowsForPoint[RTMA_TD].total + 
                                                    numRowsForPoint[NDFD_TD].total;
            numRowsForPoint[RTMA_NDFD_TD].skipBeg = numRowsForPoint[RTMA_TD].skipBeg;
            numRowsForPoint[RTMA_NDFD_TD].firstUserTime = 
                                      numRowsForPoint[RTMA_TD].firstUserTime;
         }
         else
            /* The user supplied startTime cut off all rows of RTMA data for 
             * the concatenated Dew Point element. In that case, treat as if
             * the concatenated element does not exist. 
             */
            *pnt_rtmaNdfdTd = 0;

         if (numRowsForPoint[NDFD_TD].total-numRowsForPoint[NDFD_TD].skipBeg-
                                       numRowsForPoint[NDFD_TD].skipEnd != 0)
         {
            numRowsForPoint[RTMA_NDFD_TD].skipEnd = numRowsForPoint[NDFD_TD].skipEnd;
            numRowsForPoint[RTMA_NDFD_TD].lastUserTime = 
                                      numRowsForPoint[NDFD_TD].lastUserTime;
         }
         else
            /* The user supplied endTime cut off all rows of NDFD data for 
             * the concatenated Dew Point element. In that case, treat as if 
             * the concatenated element does not exist. 
             */
            *pnt_rtmaNdfdTd = 0;
      }

      /* The concatenated RTMA + NDFD Wind Speed element. */
      if (wxParameters[RTMA_NDFD_WSPD] != 0)
      {
         if (numRowsForPoint[RTMA_WSPD].total-numRowsForPoint[RTMA_WSPD].skipBeg-
                                         numRowsForPoint[RTMA_WSPD].skipEnd != 0)

         {
            numRowsForPoint[RTMA_NDFD_WSPD].total = numRowsForPoint[RTMA_WSPD].total + 
                                                    numRowsForPoint[NDFD_WS].total;
            numRowsForPoint[RTMA_NDFD_WSPD].skipBeg = numRowsForPoint[RTMA_WSPD].skipBeg;
            numRowsForPoint[RTMA_NDFD_WSPD].firstUserTime = 
                                      numRowsForPoint[RTMA_WSPD].firstUserTime;
         }
         else
            /* The user supplied startTime cut off all rows of RTMA data for 
             * the concatenated Wspd element. In that case, treat as if the 
             * concatenated element does not exist. 
             */
            *pnt_rtmaNdfdWspd = 0;

         if (numRowsForPoint[NDFD_WS].total-numRowsForPoint[NDFD_WS].skipBeg-
                                       numRowsForPoint[NDFD_WS].skipEnd != 0)
         {
            numRowsForPoint[RTMA_NDFD_WSPD].skipEnd = numRowsForPoint[NDFD_WS].skipEnd;
            numRowsForPoint[RTMA_NDFD_WSPD].lastUserTime = 
                                      numRowsForPoint[NDFD_WS].lastUserTime;
         }
         else
            /* The user supplied endTime cut off all rows of NDFD data for 
             * the concatenated Wspd element. In that case, treat as if the 
             * concatenated element does not exist. 
             */
            *pnt_rtmaNdfdWspd = 0;
      }

      /* The concatenated RTMA + NDFD Wind Direction element. */
      if (wxParameters[RTMA_NDFD_WDIR] != 0)
      {
         if (numRowsForPoint[RTMA_WDIR].total-numRowsForPoint[RTMA_WDIR].skipBeg-
                                         numRowsForPoint[RTMA_WDIR].skipEnd != 0)

         {
            numRowsForPoint[RTMA_NDFD_WDIR].total = numRowsForPoint[RTMA_WDIR].total + 
                                                    numRowsForPoint[NDFD_WD].total;
            numRowsForPoint[RTMA_NDFD_WDIR].skipBeg = numRowsForPoint[RTMA_WDIR].skipBeg;
            numRowsForPoint[RTMA_NDFD_WDIR].firstUserTime = 
                                      numRowsForPoint[RTMA_WDIR].firstUserTime;
         }
         else
            /* The user supplied startTime cut off all rows of RTMA data for 
             * the concatenated Wdir element. In that case, treat as if the 
             * concatenated element does not exist. 
             */
            *pnt_rtmaNdfdWdir = 0;

         if (numRowsForPoint[NDFD_WD].total-numRowsForPoint[NDFD_WD].skipBeg-
                                       numRowsForPoint[NDFD_WD].skipEnd != 0)
         {
            numRowsForPoint[RTMA_NDFD_WDIR].skipEnd = numRowsForPoint[NDFD_WD].skipEnd;
            numRowsForPoint[RTMA_NDFD_WDIR].lastUserTime = 
                                      numRowsForPoint[NDFD_WD].lastUserTime;
         }
         else
            /* The user supplied endTime cut off all rows of NDFD data for 
             * the concatenated Wdir element. In that case, treat as if the 
             * concatenated element does not exist. 
             */
            *pnt_rtmaNdfdWdir = 0;
      }

      /* The concatenated RTMA + NDFD Precipitation Amount element. */
      if (wxParameters[RTMA_NDFD_PRECIPA] != 0)
      {
         if (numRowsForPoint[RTMA_PRECIPA].total-numRowsForPoint[RTMA_PRECIPA].skipBeg-
                                            numRowsForPoint[RTMA_PRECIPA].skipEnd != 0)

         {
            numRowsForPoint[RTMA_NDFD_PRECIPA].total = numRowsForPoint[RTMA_PRECIPA].total + 
                                                    numRowsForPoint[NDFD_QPF].total;
            numRowsForPoint[RTMA_NDFD_PRECIPA].skipBeg = numRowsForPoint[RTMA_PRECIPA].skipBeg;
            numRowsForPoint[RTMA_NDFD_PRECIPA].firstUserTime = 
                                      numRowsForPoint[RTMA_PRECIPA].firstUserTime;
         }
         else
            /* The user supplied startTime cut off all rows of RTMA data for 
             * the concatenated Precip Amt element. In that case, treat as if
             * the concatenated element does not exist. 
             */
            *pnt_rtmaNdfdPrecipa = 0;

         if (numRowsForPoint[NDFD_QPF].total-numRowsForPoint[NDFD_QPF].skipBeg-
                                        numRowsForPoint[NDFD_QPF].skipEnd != 0)
         {
            numRowsForPoint[RTMA_NDFD_PRECIPA].skipEnd = numRowsForPoint[NDFD_QPF].skipEnd;
            numRowsForPoint[RTMA_NDFD_PRECIPA].lastUserTime = 
                                      numRowsForPoint[NDFD_QPF].lastUserTime;
         }
         else
            /* The user supplied endTime cut off all rows of NDFD data for 
             * the concatenated Precip Amt element. In that case, treat as if
             * the concatenated element does not exist. 
             */
            *pnt_rtmaNdfdPrecipa = 0;
      }

      /* The concatenated RTMA + NDFD Sky element. */
      if (wxParameters[RTMA_NDFD_SKY] != 0)
      {
         if (numRowsForPoint[RTMA_SKY].total-numRowsForPoint[RTMA_SKY].skipBeg-
                                        numRowsForPoint[RTMA_SKY].skipEnd != 0)

         {
            numRowsForPoint[RTMA_NDFD_SKY].total = numRowsForPoint[RTMA_SKY].total + 
                                                    numRowsForPoint[NDFD_SKY].total;
            numRowsForPoint[RTMA_NDFD_SKY].skipBeg = numRowsForPoint[RTMA_SKY].skipBeg;
            numRowsForPoint[RTMA_NDFD_SKY].firstUserTime = 
                                      numRowsForPoint[RTMA_SKY].firstUserTime;
         }
         else
            /* The user supplied startTime cut off all rows of RTMA data for 
             * the concatenated Sky element. In that case, treat as if the 
             * concatenated element does not exist. 
             */
            *pnt_rtmaNdfdSky = 0;

         if (numRowsForPoint[NDFD_SKY].total-numRowsForPoint[NDFD_SKY].skipBeg-
                                        numRowsForPoint[NDFD_SKY].skipEnd != 0)
         {
            numRowsForPoint[RTMA_NDFD_SKY].skipEnd = numRowsForPoint[NDFD_SKY].skipEnd;
            numRowsForPoint[RTMA_NDFD_SKY].lastUserTime = 
                                      numRowsForPoint[NDFD_SKY].lastUserTime;
         }
         else
            /* The user supplied endTime cut off all rows of NDFD data for 
             * the concatenated Sky element. In that case, treat as if the 
             * concatenated element does not exist. 
             */
            *pnt_rtmaNdfdSky = 0;
      }
   }
   
   return;
}
