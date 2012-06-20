/*****************************************************************************
 * computeStartEndTimes() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Determines all the start and end times in string form for all validTimes.              
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
 *      numFmtdRows = Number of set rows to format. (Input)
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
 *         TZoffset = Number of hours to add to current time to get GMT time. 
 *                    (Input)
 *     f_observeDST = Flag determining if current point observes Daylight 
 *                    Savings Time. (Input)  
 *     periodLength = Length between an elements successive validTimes (Input).
 *        frequency = Set to "boggus" for DWMLgen products, and to "12hourly" 
 *                    or "24hourly" for the DWMLgenByDay products.  
 *            f_XML = flag for 1 of the 4 DWML products (Input):
 *                    1 = DWMLgen's NDFD "time-series" product. 
 *                    2 = DWMLgen's NDFD "glance" product.
 *                    3 = DWMLgenByDay's "12 hourly" format product.
 *                    4 = DWMLgenByDay's "24 hourly" format product.
 *                    5 = DWMLgen's RTMA "time-series" product. 
 *                    6 = DWMLgen's RTMA & NDFD "time-series" product. 
 *            match = Pointer to the array of element matches from degrib. 
 *                    (Input) 
 *      useEndTimes =Flag denoting if element uses end times in the output XML 
 *                    (Input)
 *    startTime_cml = Incoming argument set by user as a double in seconds 
 *                    since 1970 denoting the starting time data was retrieved
 *                    for from NDFD. (Input)
 *  currentDoubTime = Current time in double form. (Input)
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
 *  3/2007 Paul Hershberg (MDL): -- Added 9 SPC elements 
 *                               -- Added 6 Tropical Wind Threshold elements
 *                               -- Added startNum/endNum arguments.
 *  8/2007 Paul Hershberg (MDL): -- Added 12 Climate Outlook elements 
 * 11/2007 Paul Hershberg (MDL): -- Added 10 RTMA elements 
 *  2/2008 Paul Hershberg (MDL): -- Added special case of RTMA_PRECIPA when 
 *                                  concatenated to NDFD_QPF
 *  6/2008 Paul Hershberg (MDL): -- Added Hazard Element 
 *  8/2009 Paul Hershberg (MDL): Added Lamp Tstm element. 
 *  1/2011 Paul Hershberg (MDL): Added Fire Wx elements.
 *  3/2012 Paul Hershberg (MdL): Added MaxRH/MinRH elements.
 *                                  
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void computeStartEndTimes(uChar parameterName, int numFmtdRows,
                          int periodLength, double TZoffset,
                          sChar f_observeDST, genMatchType * match,
                          uChar useEndTimes, char **startTimes, char **endTimes,
                          char *frequency, uChar f_XML, double startTime_cml, 
			  double currentDoubTime, numRowsInfo numRows, 
                          int startNum, int endNum)
{
   int deltaSeconds = 0;
   int i; /* Counter thru match structure. */
   int timeCounter = -1;      /* Counts number of times the start (or end) 
			       * times were created using actual data. */
   char str1[30];             /* Returned character string holding valid
                               * time. */
   double startTime_validTime = 0.0; /* The newly created startTime in double 
                                      * form. Used in comparison between
                                      * startTime based off the validTime and the 
                                      * startTime based off the refTime for the
                                      * 9 SPC elements. */
   char *pstr;                /* Pointer string used to denote the "T" in the 
                               * validTime string. */
   int oneDay = (24 * 60 * 60); /* # seconds in 24 hours. */
   char temp[3];              /* Temporary string buffer. */
   char temp2[5];             /* Temporary string buffer. */
   int beginningHour;         /* Beginning hour of validTime being processed. 
                               */
   int priorElemCount;        /* Counter used to find elements' location in
                               * match structure. */
   double Time_doub = 0.0;    /* Holds startTime as a double. */
   double startClimatePeriod = 0.0; /* The start of the Climate Outlook
                                       1-monthly and 3-monthly products in secs
                                       since 1970. */

   /* If DWMLgen products. */
   if (f_XML == 1 || f_XML == 2 || f_XML == 5 || f_XML == 6)
   {
      switch (parameterName)
      {
         /* For max and min, the period length appears to be 24 hours long.
          * But, in reality the max and min temp only apply to a 12/13 hour
          * period. So we have to reset the start time.  Additionally, max.
          * is valid for a 0700 - 1900 and min on for a 1900 - 0800 local
          * standard time period. 
	  */
         case NDFD_MAX:
         case NDFD_MAXRH:

            /* Loop over matches of the data. */
            priorElemCount = startNum;
            for (i = startNum; i < endNum; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName && 
	           match[i].validTime >= numRows.firstUserTime &&
		   match[i].validTime <= numRows.lastUserTime)
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
                  priorElemCount++;
            }
            break;

         case NDFD_MIN:
         case NDFD_MINRH:

            /* Loop over matches of the data. */
            priorElemCount = startNum;
            for (i = startNum; i < endNum; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName && 
	           match[i].validTime >= numRows.firstUserTime &&
		   match[i].validTime <= numRows.lastUserTime)
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
                  priorElemCount++;
            }
            break;

         case NDFD_POP:
         case NDFD_SNOW:
         case NDFD_QPF:
         case NDFD_ICEACC:
         case NDFD_INC34:
         case NDFD_INC50:
         case NDFD_INC64:
         case NDFD_CUM34:
         case NDFD_CUM50:
         case NDFD_CUM64:
         case LAMP_TSTMPRB:

            /* Loop over matches of the data. */
            priorElemCount = startNum;
            for (i = startNum; i < endNum; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName && 
	           match[i].validTime >= numRows.firstUserTime &&
		   match[i].validTime <= numRows.lastUserTime)
               {
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);
		  
                  startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
		  if (useEndTimes)
		  {
                     endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);

                     /* For the NDFD, the valid time is at the end of the valid
                      * period. So end time equal to the "valid time" and calcuate
                      * the start time. 
                      */
                     strcpy(endTimes[i - priorElemCount], str1);
		  }

                  temp[0] = str1[11];
                  temp[1] = str1[12];
                  temp[2] = '\0';
                  beginningHour = atoi(temp);
                  beginningHour = beginningHour - periodLength;

                  /* If the hour is negative, we moved to the previous day so
                   * determine what the new date and time are. 
                   */
                  if (beginningHour < 0)
                  {
                     beginningHour += 24;
                     formatValidTime((match[i].validTime - oneDay), str1, 30,
                                     TZoffset, f_observeDST);
                     sprintf(temp, "%d", beginningHour);
                  }

                  /* Now we assemble the start time. Need to make sure we have a 
                   * two digit hour when number is less than 10. 
                   */
                  if (beginningHour < 10)
                     sprintf(temp2, "%c%c%1d%c", 'T', '0', beginningHour, '\0');
                  else
                     sprintf(temp2, "%c%2d%c", 'T', beginningHour, '\0');

                  pstr = strstr(str1, "T");
                  strncpy(pstr, temp2, 3);
                  strcpy(startTimes[i - priorElemCount], str1);

               }
               else
                  priorElemCount++;
            }
            break;

         case NDFD_FWXWINDRH:
         case NDFD_FWXTSTORM:
         case NDFD_CONHAZ:
         case NDFD_PTORN:
         case NDFD_PHAIL:
         case NDFD_PTSTMWIND:
         case NDFD_PXTORN: 
         case NDFD_PXHAIL: 
         case NDFD_PXTSTMWIND:
         case NDFD_PSTORM: 
         case NDFD_PXSTORM:

            /* Loop over matches of the data. */
            priorElemCount = startNum;
            for (i = startNum; i < endNum; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName && 
	           match[i].validTime >= numRows.firstUserTime &&
		   match[i].validTime <= numRows.lastUserTime)
               {
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);		  
                  startTimes[i - priorElemCount] = malloc(strlen(str1)+1);
		  if (useEndTimes)
		  {
                     endTimes[i - priorElemCount] = malloc(strlen(str1)+1);

                     /* For the NDFD, the valid time is at the end of the
                      * valid period. So end time equal to the "valid time"
                      * and calculate the start time. 
                      */
                     strcpy(endTimes[i - priorElemCount], str1);
		  }

                  temp[0] = str1[11];
                  temp[1] = str1[12];
                  temp[2] = '\0';
                  beginningHour = atoi(temp);
                  beginningHour = beginningHour - periodLength;

                  /* If the hour is negative, we move to the previous day
                   * so determine what the new date and time are. 
                   */
                  if (beginningHour < 0)
                  {
                     beginningHour += 24;
                     formatValidTime((match[i].validTime - oneDay), str1, 30,
                                      TZoffset, f_observeDST);
                     sprintf(temp, "%d", beginningHour);
                  }

                  /* Now we assemble the start time. Need to make sure we
                   * have a two digit hour when number is less than 10. 
                   */
                  if (beginningHour < 10)
                     sprintf(temp2, "%c%c%1d%c", 'T', '0', beginningHour, '\0');
                  else
                     sprintf(temp2, "%c%2d%c", 'T', beginningHour, '\0');

                  pstr = strstr(str1, "T");
                  strncpy(pstr, temp2, 3);

                  /* Now, before we assign the startTime based off of the 
                   * validTime, check to see if the base refTime is later than
                   * the newly created startTime. If so, simply use the refTime
                   * as the startTime (this will occur on day 1 of some of these
                   * SPC elements and Fire Wx elements). Get the newly created 
                   * startTime in double form for the comparison.
                   */
                  Clock_Scan(&startTime_validTime, str1, 0);
                  if (startTime_validTime < match[i].refTime)
                     formatValidTime(match[i].refTime, str1, 30, TZoffset,
                                     f_observeDST);
                  strcpy(startTimes[i - priorElemCount], str1);
               }
               else
                  priorElemCount++;
            }
            break;

         case NDFD_TMPABV14D:
         case NDFD_TMPBLW14D:
         case NDFD_PRCPABV14D:
         case NDFD_PRCPBLW14D:

            /* Loop over matches of the data. */
            priorElemCount = startNum;
            for (i = startNum; i < endNum; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName && 
	           match[i].validTime >= numRows.firstUserTime &&
		   match[i].validTime <= numRows.lastUserTime)
               {
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);		  
		  if (useEndTimes)
		  {
                     endTimes[i - priorElemCount] = malloc(strlen(str1)+1);

                     /* For the NDFD, the valid time is at the end of the
                      * valid period. So end time equal to the "valid time"
                      * and calculate the start time. The start time for 
                      * these 8-14 day outlook products will be 6 days
                      * earlier than the endTime (validTime). 
                      */
                     strcpy(endTimes[i - priorElemCount], str1);
		  }
                  startTimes[i - priorElemCount] = malloc(strlen(str1)+1);

                  /* periodLength is in hours. Need to subract # of days in secs. */
                  Time_doub = match[i].validTime - (periodLength*3600);
                  formatValidTime(Time_doub, str1, 30, TZoffset, 
                                  f_observeDST);
                  strcpy(startTimes[i - priorElemCount], str1);
               }
               else
                  priorElemCount++;
            }
            break;

         case NDFD_TMPABV30D:
         case NDFD_TMPBLW30D:
         case NDFD_PRCPABV30D:
         case NDFD_PRCPBLW30D:

            /* Loop over matches of the data. */
            priorElemCount = startNum;
            for (i = startNum; i < endNum; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName && 
	           match[i].validTime >= numRows.firstUserTime &&
		   match[i].validTime <= numRows.lastUserTime)
               {
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);		  
		  if (useEndTimes)
		  {
                     endTimes[i - priorElemCount] = malloc(strlen(str1)+1);

                     /* For the NDFD, the valid time is at the end of the
                      * valid period. So end time equal to the "valid time"
                      * and calculate the start time.
                      *
                      * The start time for the One-monthly outlook products 
                      * will be the beginning of the month (1st) which can
                      * vary between 28 and 31 days earlier than the endTime. 
                      */
                     strcpy(endTimes[i - priorElemCount], str1);
		  }
                  startTimes[i - priorElemCount] = malloc(strlen(str1)+1);

                  /* periodLength is in hours and is varying (between 28-31 days
                   * worth of hours for monthly products, and 88-93 days worth of
                   * hours for the three-monthly products. We need to find 
                   * the exact period for each data value (row of data). 
                   */
                  startClimatePeriod = Clock_AddMonthYear(match[i].validTime, -1, 0);
                  formatValidTime(startClimatePeriod, str1, 30, TZoffset, f_observeDST);
                  strcpy(startTimes[i - priorElemCount], str1);
               }
               else
                  priorElemCount++;
            }
            break;

         case NDFD_TMPABV90D:
         case NDFD_TMPBLW90D:
         case NDFD_PRCPABV90D:
         case NDFD_PRCPBLW90D:

            /* Loop over matches of the data. */
            priorElemCount = startNum;
            for (i = startNum; i < endNum; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName && 
	           match[i].validTime >= numRows.firstUserTime &&
		   match[i].validTime <= numRows.lastUserTime)
               {
                  formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                  f_observeDST);		  
		  if (useEndTimes)
		  {
                     endTimes[i - priorElemCount] = malloc(strlen(str1)+1);

                     /* For the NDFD, the valid time is at the end of the
                      * valid period. So end time equal to the "valid time"
                      * and calculate the start time.
                      * 
                      * The start time for the Three-monthly outlook products 
                      * will be the beginning of the month (1st) three months 
                      * prior. This is not always exactly 90 days prior, but 
                      * the actual number of days in those three months.
                      */
                     strcpy(endTimes[i - priorElemCount], str1);
		  }
                  startTimes[i - priorElemCount] = malloc(strlen(str1)+1);

                  /* periodLength is in hours and is varying (between 28-31 days
                   * worth of hours for monthly products, and 88-93 days worth of
                   * hours for the three-monthly products. We need to find 
                   * the exact period for each data value (row of data). 
                   */
                  startClimatePeriod = Clock_AddMonthYear(match[i].validTime, -3, 0);
                  formatValidTime(startClimatePeriod, str1, 30, TZoffset, f_observeDST);
                  strcpy(startTimes[i - priorElemCount], str1);
               }
               else
                  priorElemCount++;
            }
            break;

         case RTMA_PRECIPA:

            /* Only find endTimes if RTMA Precip Amt is concatenated
             * to the NDFD QPF portion, for consistency sake. 
             */
            if (f_XML == 6) 
            {
               /* Loop over matches of the data. */
               priorElemCount = startNum;
               for (i = startNum; i < endNum; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName && 
	              match[i].validTime >= numRows.firstUserTime &&
		      match[i].validTime <= numRows.lastUserTime)
                  {
                     formatValidTime(match[i].validTime, str1, 30, TZoffset,
                                     f_observeDST);
		  
                     startTimes[i - priorElemCount] = malloc(strlen(str1) + 1);
		     if (useEndTimes)
		     {
                       endTimes[i - priorElemCount] = malloc(strlen(str1) + 1);

                       /* For this RTMA element, the valid time is at the end of the valid
                        * period. So end time equal to the "valid time" and calcuate
                        * the start time by subtracting just one hour. 
                        */
                       strcpy(endTimes[i - priorElemCount], str1);
		     }

                     temp[0] = str1[11];
                     temp[1] = str1[12];
                     temp[2] = '\0';
                     beginningHour = atoi(temp);
                     beginningHour = beginningHour - periodLength;

                     /* If the hour is negative, we moved to the previous day so
                      * determine what the new date and time are. 
                      */
                     if (beginningHour < 0)
                     {
                        beginningHour += 24;
                        formatValidTime((match[i].validTime - oneDay), str1, 30,
                                        TZoffset, f_observeDST);
                        sprintf(temp, "%d", beginningHour);
                     }

                     /* Now we assemble the start time. Need to make sure we have a 
                      * two digit hour when number is less than 10. 
                      */
                     if (beginningHour < 10)
                        sprintf(temp2, "%c%c%1d%c", 'T', '0', beginningHour, '\0');
                     else
                        sprintf(temp2, "%c%2d%c", 'T', beginningHour, '\0');

                     pstr = strstr(str1, "T");
                     strncpy(pstr, temp2, 3);
                     strcpy(startTimes[i - priorElemCount], str1);
                  }
                  else
                    priorElemCount++;
               }
            }
            else /* Just use startTimes (f_XML == 5). */
            {
               priorElemCount = startNum;
               for (i = startNum; i < endNum; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName && 
	             match[i].validTime >= numRows.firstUserTime &&
		     match[i].validTime <= numRows.lastUserTime)
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
            }
            break;

         default: /* All RTMA elements and the Hazard element are here (other
                   * than above RTMA exception for RTMA_PRECIPA). 
                   */

            /* Loop over matches of the data. */
            priorElemCount = startNum;
            for (i = startNum; i < endNum; i++)
            {
               if (match[i].elem.ndfdEnum == parameterName && 
	           match[i].validTime >= numRows.firstUserTime &&
		   match[i].validTime <= numRows.lastUserTime)
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
               priorElemCount = startNum;
               for (i = startNum; i < endNum; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName &&
	              match[i].validTime >= numRows.firstUserTime &&
		      match[i].validTime <= numRows.lastUserTime)
                  {
		     timeCounter++;
		     
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
		* to be formatted. If so, we need to fabricate those times. 
                */
               if (timeCounter+1 < numFmtdRows)
	       {
	          for (i = timeCounter+1; i < numFmtdRows; i++)
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
               priorElemCount = startNum;
               for (i = startNum; i < endNum; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName &&
	              match[i].validTime >= numRows.firstUserTime &&
		      match[i].validTime <= numRows.lastUserTime)
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
               if (timeCounter+1 < numFmtdRows)
	       {
	          for (i = timeCounter+1; i < numFmtdRows; i++)
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
	 * Icons will share the same start and end times as Pop. (Note, if the 
	 * product is of type f_XML = 4, the only product with 12 hourly 
	 * summarizations will be the Pop element.) 
	 */
	 switch (parameterName)
         {
	    case NDFD_POP:
		    
	       deltaSeconds = ((periodLength / 4) * 3600);	  
               /* Loop over matches of the data. */
               priorElemCount = startNum;
               for (i = startNum; i < endNum; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName &&
	              match[i].validTime >= numRows.firstUserTime &&
		      match[i].validTime <= numRows.lastUserTime)
                  {
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
               if (timeCounter+1 < numFmtdRows)
	       {
	          for (i = timeCounter+1; i < numFmtdRows; i++)
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
               priorElemCount = startNum;
               for (i = startNum; i < endNum; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName &&
	              match[i].validTime >= numRows.firstUserTime &&
		      match[i].validTime <= numRows.lastUserTime)
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
               if (timeCounter+1 < numFmtdRows)
	       {
	          for (i = timeCounter+1; i < numFmtdRows; i++)
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
               priorElemCount = startNum;
               for (i = startNum; i < endNum; i++)
               {
                  if (match[i].elem.ndfdEnum == parameterName &&
	              match[i].validTime >= numRows.firstUserTime &&
		      match[i].validTime <= numRows.lastUserTime)
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
		* to be formatted. If so, we need to fabricate those times. 
                */
               if (timeCounter+1 < numFmtdRows)
	       {
	          for (i = timeCounter+1; i < numFmtdRows; i++)
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
