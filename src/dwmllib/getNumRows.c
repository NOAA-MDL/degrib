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
 *   6/2008 Paul Hershberg (MDL): Accommodated hazard (NDFD_WWA) element.
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
                int *f_formatSummarizations, size_t pnt, int *pnt_rtmaNdfdTemp, 
                int *pnt_rtmaNdfdTd, int *pnt_rtmaNdfdWdir, 
                int *pnt_rtmaNdfdWspd, int *pnt_rtmaNdfdPrecipa, 
                int *pnt_rtmaNdfdSky, double currentDoubTime, size_t numElem, 
                genElemDescript *elem)
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
   char tempBuff[30]; /* Temp string. */
   int numRowsPop = 0;
   int numRowsTemp = 0;
   int numRowsWspd = 0;
   int numRowsSky = 0;
   int numRowsWx = 0;
   int f_maxtForSummary = 1;
   int f_mintForSummary = 1;
   int f_popForSummary = 1;
   int f_wdirForSummary = 1;
   int f_wspdForSummary = 1;
   int f_skyForSummary = 1;
   int f_wxForSummary = 1;
   int numRowsNdfdVar = 0;
   int numRowsRtmaVar = 0;
   int numRowsNdfdVarSkipEnd = 0;
   int numRowsNdfdVarLastUserTime = 0.0;
   int numRowsRtmaVarSkipBeg = 0;
   int numRowsRtmaVarFirstUserTime = 0.0;

   /* Initialize some numRowsForPoint structure members to zeros. */
   for (k = 0; k < numElem; k++)
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
   for (k = 0; k < numElem; k++)
   {
      for (i = startNum; i < endNum; i++)
      {
         if (match[i].elem.ndfdEnum == elem[k].ndfdEnum)
         {
            numRowsForPoint[k].total++;
         }
      }
   }

   /* Retrieve the first validTime and last validTime per element amongst all
    * matches for the point.
    */
   for (k = 0; k < numElem; k++)
   {
      for (i = startNum; i < endNum; i++)
      {
	 if (match[i].elem.ndfdEnum == elem[k].ndfdEnum)
	 {
            numRowsForPoint[k].firstUserTime = match[i].validTime;
	    numRowsForPoint[k].lastUserTime = match[i + (numRowsForPoint[k].total-1)].validTime;
            formatValidTime(numRowsForPoint[k].firstUserTime, tempBuff, 30, TZoffset, f_observeDST);
            formatValidTime(numRowsForPoint[k].lastUserTime, tempBuff, 30, TZoffset, f_observeDST);
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
   for (k = 0; k < numElem; k++)
   {
      if (wxParameters[k] != 0)
      {
	 if (elem[k].ndfdEnum == NDFD_MAX || elem[k].ndfdEnum == NDFD_MIN)
	    period = 12;
         else if (elem[k].ndfdEnum == NDFD_WWA)
	    period = 1;
	 else
	 {
            getFirstSecondValidTimes(&firstValidTime, &secondValidTime, 
	   		             match, numMatch, elem[k].ndfdEnum, 
                                     startNum, endNum, numRowsForPoint[k].total, 
                                     0, 0);
	    period = determinePeriodLength(firstValidTime, secondValidTime,
	                                   numRowsForPoint[k].total, 
                                           elem[k].ndfdEnum);
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
            if (atoi(currentHour) >= 6 && (elem[k].ndfdEnum != NDFD_MIN || 
                elem[k].ndfdEnum != NDFD_POP))
            {
               timeUserStartPerElement = *timeUserStart + deltaSecs;
               timeUserEndPerElement = *timeUserEnd - deltaSecs;      
            }
            if (elem[k].ndfdEnum == NDFD_POP)
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
            if (elem[k].ndfdEnum == NDFD_MIN)
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
	    if (match[i].elem.ndfdEnum == elem[k].ndfdEnum)
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
   for (k = 0; k < numElem; k++)
   {
      if ((wxParameters[k] != 0) && 
	  (numRowsForPoint[k].skipBeg != 0 || numRowsForPoint[k].skipEnd != 0))
      {
         for (i = startNum; i < endNum; i++)
	 {
	    if (match[i].elem.ndfdEnum == elem[k].ndfdEnum)
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
    * this. If a concatenated element, check numRows for both the RTMA and NDFD
    * portion. If either is zero, we cannot format the concatenated 
    * RTMA+NDFD element. Also, we need to compute the number of rows for 
    * the concatenated elements by summing up their individual NDFD and RTMA
    * components. Follow the order specified in xmlparse.h for the concatenated 
    * elements (#55 thru #60). 
    */   

  /* If concatenated Rtma + Ndfd Sky Amount was queried for and exists. */
   for (k = 0; k < numElem; k++)
   {
      if (elem[k].ndfdEnum == NDFD_SKY)
      {
          numRowsNdfdVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsNdfdVarSkipEnd = numRowsForPoint[k].skipEnd;
          numRowsNdfdVarLastUserTime = numRowsForPoint[k].lastUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_SKY)
      {
          numRowsRtmaVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsRtmaVarSkipBeg = numRowsForPoint[k].skipBeg;
          numRowsRtmaVarFirstUserTime = numRowsForPoint[k].firstUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_NDFD_SKY)
      {
         if (numRowsRtmaVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdSky = 0;
         }
         else
         {
            numRowsForPoint[k].total =  numRowsNdfdVar + numRowsRtmaVar;
            numRowsForPoint[k].skipBeg = numRowsRtmaVarSkipBeg;
            numRowsForPoint[k].firstUserTime = numRowsRtmaVarFirstUserTime;
         }
         if (numRowsNdfdVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdSky = 0;
         }
         else
         {
            numRowsForPoint[k].skipEnd = numRowsNdfdVarSkipEnd;
            numRowsForPoint[k].lastUserTime = numRowsNdfdVarLastUserTime;
         }
      }
   }

   numRowsNdfdVar = 0;
   numRowsRtmaVar = 0;
   numRowsNdfdVarSkipEnd = 0;
   numRowsNdfdVarLastUserTime = 0.0;
   numRowsRtmaVarSkipBeg = 0;
   numRowsRtmaVarFirstUserTime = 0.0;

   /* If concatenated Rtma + Ndfd Precip Amount was queried for and exists. */
   for (k = 0; k < numElem; k++)
   {
      if (elem[k].ndfdEnum == NDFD_QPF)
      {
         numRowsNdfdVar = numRowsForPoint[k].total-
                          numRowsForPoint[k].skipBeg-
                          numRowsForPoint[k].skipEnd;
         numRowsNdfdVarSkipEnd = numRowsForPoint[k].skipEnd;
         numRowsNdfdVarLastUserTime = numRowsForPoint[k].lastUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_PRECIPA)
      {
         numRowsRtmaVar = numRowsForPoint[k].total-
                          numRowsForPoint[k].skipBeg-
                          numRowsForPoint[k].skipEnd;
         numRowsRtmaVarSkipBeg = numRowsForPoint[k].skipBeg;
         numRowsRtmaVarFirstUserTime = numRowsForPoint[k].firstUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_NDFD_PRECIPA)
      {
         if (numRowsRtmaVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdPrecipa = 0;
         }
         else
         {
            numRowsForPoint[k].total =  numRowsNdfdVar + numRowsRtmaVar;
            numRowsForPoint[k].skipBeg = numRowsRtmaVarSkipBeg;
            numRowsForPoint[k].firstUserTime = numRowsRtmaVarFirstUserTime;
         }
         if (numRowsNdfdVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdPrecipa = 0;
         }
         else
         {
            numRowsForPoint[k].skipEnd = numRowsNdfdVarSkipEnd;
            numRowsForPoint[k].lastUserTime = numRowsNdfdVarLastUserTime;
         }
      }
   }

   numRowsNdfdVar = 0;
   numRowsRtmaVar = 0;
   numRowsNdfdVarSkipEnd = 0;
   numRowsNdfdVarLastUserTime = 0.0;
   numRowsRtmaVarSkipBeg = 0;
   numRowsRtmaVarFirstUserTime = 0.0;
 
   /* If concatenated Rtma + Ndfd Dew Point was queried for and exists. */
   for (k = 0; k < numElem; k++)
   {
      if (elem[k].ndfdEnum == NDFD_TD)
      {
          numRowsNdfdVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsNdfdVarSkipEnd = numRowsForPoint[k].skipEnd;
          numRowsNdfdVarLastUserTime = numRowsForPoint[k].lastUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_TD)
      {
          numRowsRtmaVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsRtmaVarSkipBeg = numRowsForPoint[k].skipBeg;
          numRowsRtmaVarFirstUserTime = numRowsForPoint[k].firstUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_NDFD_TD)
      {
         if (numRowsRtmaVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdTd = 0;
         }
         else
         {
            numRowsForPoint[k].total =  numRowsNdfdVar + numRowsRtmaVar;
            numRowsForPoint[k].skipBeg = numRowsRtmaVarSkipBeg;
            numRowsForPoint[k].firstUserTime = numRowsRtmaVarFirstUserTime;
         }
         if (numRowsNdfdVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdTd = 0;
         }
         else
         {
            numRowsForPoint[k].skipEnd = numRowsNdfdVarSkipEnd;
            numRowsForPoint[k].lastUserTime = numRowsNdfdVarLastUserTime;
         }
      }
   }

   numRowsNdfdVar = 0;
   numRowsRtmaVar = 0;
   numRowsNdfdVarSkipEnd = 0;
   numRowsNdfdVarLastUserTime = 0.0;
   numRowsRtmaVarSkipBeg = 0;
   numRowsRtmaVarFirstUserTime = 0.0;

   /* If concatenated Rtma + Ndfd Temp was queried for and exists. */
   for (k = 0; k < numElem; k++)
   {
      if (elem[k].ndfdEnum == NDFD_TEMP)
      {
          numRowsNdfdVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsNdfdVarSkipEnd = numRowsForPoint[k].skipEnd;
          numRowsNdfdVarLastUserTime = numRowsForPoint[k].lastUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_TEMP)
      {
          numRowsRtmaVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsRtmaVarSkipBeg = numRowsForPoint[k].skipBeg;
          numRowsRtmaVarFirstUserTime = numRowsForPoint[k].firstUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_NDFD_TEMP)
      {
         if (numRowsRtmaVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdTemp = 0;
         }
         else
         {
            numRowsForPoint[k].total = numRowsNdfdVar + numRowsRtmaVar;
            numRowsForPoint[k].skipBeg = numRowsRtmaVarSkipBeg;
            numRowsForPoint[k].firstUserTime = numRowsRtmaVarFirstUserTime;
         }
         if (numRowsNdfdVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdTemp = 0;
         }
         else
         {
            numRowsForPoint[k].skipEnd = numRowsNdfdVarSkipEnd;
            numRowsForPoint[k].lastUserTime = numRowsNdfdVarLastUserTime;
         }
      }
   }

   numRowsNdfdVar = 0;
   numRowsRtmaVar = 0;
   numRowsNdfdVarSkipEnd = 0;
   numRowsNdfdVarLastUserTime = 0.0;
   numRowsRtmaVarSkipBeg = 0;
   numRowsRtmaVarFirstUserTime = 0.0;

   /* If concatenated Rtma + Ndfd Wind Direction was queried for and exists. */
   for (k = 0; k < numElem; k++)
   {
      if (elem[k].ndfdEnum == NDFD_WD)
      {
          numRowsNdfdVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsNdfdVarSkipEnd = numRowsForPoint[k].skipEnd;
          numRowsNdfdVarLastUserTime = numRowsForPoint[k].lastUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_WDIR)
      {
         numRowsRtmaVar = numRowsForPoint[k].total-
                          numRowsForPoint[k].skipBeg-
                          numRowsForPoint[k].skipEnd;
         numRowsRtmaVarSkipBeg = numRowsForPoint[k].skipBeg;
         numRowsRtmaVarFirstUserTime = numRowsForPoint[k].firstUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_NDFD_WDIR)
      {
         if (numRowsRtmaVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdWdir = 0;
         }
         else
         {
            numRowsForPoint[k].total =  numRowsNdfdVar + numRowsRtmaVar;
            numRowsForPoint[k].skipBeg = numRowsRtmaVarSkipBeg;
            numRowsForPoint[k].firstUserTime = numRowsRtmaVarFirstUserTime;
         }
         if (numRowsNdfdVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdWdir = 0;
         }
         else
         {
            numRowsForPoint[k].skipEnd = numRowsNdfdVarSkipEnd;
            numRowsForPoint[k].lastUserTime = numRowsNdfdVarLastUserTime;
         }
      }
   }

   numRowsNdfdVar = 0;
   numRowsRtmaVar = 0;
   numRowsNdfdVarSkipEnd = 0;
   numRowsNdfdVarLastUserTime = 0.0;
   numRowsRtmaVarSkipBeg = 0;
   numRowsRtmaVarFirstUserTime = 0.0;

   /* If concatenated Rtma + Ndfd Wind Speed was queried for and exists. */
   for (k = 0; k < numElem; k++)
   {
      if (elem[k].ndfdEnum == NDFD_WS)
      {
          numRowsNdfdVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsNdfdVarSkipEnd = numRowsForPoint[k].skipEnd;
          numRowsNdfdVarLastUserTime = numRowsForPoint[k].lastUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_WSPD)
      {
          numRowsRtmaVar = numRowsForPoint[k].total-
                           numRowsForPoint[k].skipBeg-
                           numRowsForPoint[k].skipEnd;
          numRowsRtmaVarSkipBeg = numRowsForPoint[k].skipBeg;
          numRowsRtmaVarFirstUserTime = numRowsForPoint[k].firstUserTime;
      }
      if (elem[k].ndfdEnum == RTMA_NDFD_WSPD)
      {
         if (numRowsRtmaVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdWspd = 0;
         }
         else
         {
            numRowsForPoint[k].total =  numRowsNdfdVar + numRowsRtmaVar;
            numRowsForPoint[k].skipBeg = numRowsRtmaVarSkipBeg;
            numRowsForPoint[k].firstUserTime = numRowsRtmaVarFirstUserTime;
         }
         if (numRowsNdfdVar == 0)
         {
            wxParameters[k] = 0;
            *pnt_rtmaNdfdWspd = 0;
         }
         else
         {
            numRowsForPoint[k].skipEnd = numRowsNdfdVarSkipEnd;
            numRowsForPoint[k].lastUserTime = numRowsNdfdVarLastUserTime;
         }
      }
   }

   for (k = 0; k < numElem; k++)
   {
      if (wxParameters[k] >= 1 && (numRowsForPoint[k].total -
                             numRowsForPoint[k].skipBeg -
   	                     numRowsForPoint[k].skipEnd) == 0)
         wxParameters[k] = 0;
   }

   /* Now, check to see that Icons have all the necessary elements retrieved
    * from NDFD to derive them. Icons need Pop, Wspd, Temp, Wdir. SkyCov, and 
    * Weather elements for derivation.
    */
   if ((f_XML == 1 || f_XML == 2 || f_XML == 6) && *f_icon == 1)
   {
      for (k = 0; k < numElem; k++)
      {
         if (elem[k].ndfdEnum == NDFD_POP)
         {
            numRowsPop = numRowsForPoint[k].total-numRowsForPoint[k].skipBeg-
	                 numRowsForPoint[k].skipEnd;
         }
         if (elem[k].ndfdEnum == NDFD_TEMP)
         {
            numRowsTemp = numRowsForPoint[k].total-numRowsForPoint[k].skipBeg-
	                  numRowsForPoint[k].skipEnd;
         }
         if (elem[k].ndfdEnum == NDFD_WS)
         {
            numRowsWspd = numRowsForPoint[k].total-numRowsForPoint[k].skipBeg-
	                  numRowsForPoint[k].skipEnd;
         }
         if (elem[k].ndfdEnum == NDFD_SKY)
         {
            numRowsSky = numRowsForPoint[k].total-numRowsForPoint[k].skipBeg-
	                 numRowsForPoint[k].skipEnd;
         }
         if (elem[k].ndfdEnum == NDFD_WX)
         {
            numRowsWx = numRowsForPoint[k].total-numRowsForPoint[k].skipBeg-
	                numRowsForPoint[k].skipEnd;
         }
      }
      if (numRowsPop == 0 || numRowsTemp == 0 || numRowsWspd == 0 || 
          numRowsSky == 0 || numRowsWx == 0)
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
      /* We need all 8 elements to have data to create weather phrase/icon for the 
       * summarization periods. If one is missing, don't format XML. This only checks
       * to see if ALL an elements's data is missing (not individual projections).
       */
      for (k = 0; k < numElem; k++)
      {
         if (elem[k].ndfdEnum == NDFD_MAX && wxParameters[k] == 0)
            f_maxtForSummary = 0;
         if (elem[k].ndfdEnum == NDFD_MIN && wxParameters[k] == 0)
            f_mintForSummary = 0;
         if (elem[k].ndfdEnum == NDFD_POP && wxParameters[k] == 0)
            f_popForSummary = 0;
         if (elem[k].ndfdEnum == NDFD_WD && wxParameters[k] == 0)
            f_wdirForSummary = 0;
         if (elem[k].ndfdEnum == NDFD_WS && wxParameters[k] == 0)
            f_wspdForSummary = 0;
         if (elem[k].ndfdEnum == NDFD_SKY && wxParameters[k] == 0)
            f_skyForSummary = 0;
         if (elem[k].ndfdEnum == NDFD_WX && wxParameters[k] == 0)
            f_wxForSummary = 0;

/*       Don't let Wind Gust be a show stopper with respect to formatting
         summary weather phrases...it only exists out 52 hours. It is also
         only used in determining weather phrase "Blizzard. 
         if (elem[k].ndfdEnum == NDFD_WG && wxParameters[k] == 0)
            f_wgustForSummary = 0;
*/
      }
      if (!f_maxtForSummary || !f_mintForSummary || !f_popForSummary || 
          !f_wdirForSummary || !f_wspdForSummary || !f_skyForSummary || 
          !f_wxForSummary)
      {
         *f_formatSummarizations = 0;
      }
   }
 
  
   return;
}
