/******************************************************************************
 * genSkyCoverValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Sky Cover element in the "time-series" and "glance" 
 *  products if product type is DWMLgen (f_XML = 1 or 2). If the product type
 *  is DWMLgenByDay, with formats of "12 hourly" (f_XML = 3) or "24 hourly" 
 *  (f_XML = 4), then the code will fill 7 arrays holding sky information that
 *  will be used in determining the weather/icons.
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
 * numOutputLines = The number of data values output/formatted for each element. 
 *                  (Input)
 *   timeInterval = Number of seconds in either a 12 hourly format (3600 * 12)
 *                  or 24 hourly format (3600 * 24). (Input)
 *       TZoffset = # of hours to add to current time to get GMT time. (Input) 
 *   f_observeDST = Flag determining if current point observes Daylight 
 *                  Savings Time. (Input)  
 *  parameterName = Number denoting the NDFD element currently processed. 
 *                  (Input)
 *        numRows = Structure containing members:
 *                       total: the total number of rows of data for this element.
 *                     skipBeg: the number of beginning rows not formatted due 
 *                              to a user supplied reduction in time (startTime
 *                              arg is not = 0.0)
 *                     skipEnd: the number of end rows not formatted due to a 
 *                              user supplied reduction in time (endTime arg
 *                              is not = 0.0)
 *               firstUserTime: the first valid time interested per element, 
 *                              taking into consideration any data values 
 *                              (rows) skipped at beginning of time duration.
 *                lastUserTime: the last valid time interested per element, 
 *                              taking into consideration any data values 
 *                              (rows) skipped at end of time duration.
 *          f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product. (Input)
 * SKYintegerTime = Number of seconds since 1970 to when each of the data times
 *                  is valid. Allows the code to know if this data belongs in 
 *                  the current period being processed. Info is sent out and 
 *                  used in the generation of weather as a spring/fall 
 *                  signifier. (Output)
 *    currentHour = Current hour. (Input)
 *  timeUserStart = The beginning of the first forecast period (06 hr
 *                  or 18hr) based on the user supplied startTime argument. 
 *                  (Input)
 *      startTime = Incoming argument set by user as a double in seconds 
 *                  since 1970 denoting the starting time data is to be 
 *                  retrieved for from NDFD. (Set to 0.0 if not entered.)
 *                  (Input) 
 *    startNum = First index in match structure an individual point's data 
 *               matches can be found. (Input)
 *      endNum = Last index in match structure an individual point's data
 *               matches can be found. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2006 Paul Hershberg (MDL): Created
 *  10/2007 Paul Hershberg (MDL): Removed code that shifted data back by 1/2
 *                                the period length (bug from php code)
 *  10/2007 Paul Hershberg (MDL): Added back code that shifted data back by 1/2
 *                                the period length. It was erroneously removed.
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genSkyCoverValues(size_t pnt, char *layoutKey, genMatchType * match,
                       xmlNodePtr parameters, char *startDate, int *maxSkyCover,
                       int *minSkyCover, int *averageSkyCover, 
                       int *numOutputLines, int timeInterval, sChar TZoffset, 
                       sChar f_observeDST, uChar parameterName,
                       numRowsInfo numRows, uChar f_XML, int *maxSkyNum,
                       int *minSkyNum, int *startPositions, int *endPositions, 
                       int *SKYintegerTime, char *currentHour, 
                       double timeUserStart, double startTime, int startNum, 
                       int endNum)
{
   int i;                     /* Counter thru match structure. */
   int numNils = 0;           /* Denotes diff between number of data rows and 
				 the number that need to be formatted for the 
				 DWMLgenByDay products. */
   int period = 3;           /* Period of element. */
   int firstTime = 1; /* Flag indicating that we have found the first data value 
                       * in the current period. */
   int forecastPeriod = 0; /* Used as a counter thru the summarization (forecast) 
                            * period used to determine if current data being 
                            * processed belongs in current forecast period. */ 
   int priorElemCount = 0; /* Used to subtract prior elements when looping 
                            * thru matches. */
   int currentDay = 0;      /* Counter used to denote incrementing thru the 
                             * summarization (forecast) periods.*/
   double totalSkyCover = 0.0; /* The total Sky Cover per summarization 
                                * (forecast) period used in determining the
                                * the average sky cover per period. */
   double numSkyCoverValues = 0.0; /* The number of Sky Cover data values
                                    * per summarization (forecast) period.
                                    * Used in determining the average sky 
                                    * cover per period. */
   double timeUserStartStep = 0.0; /* Used to denote each forecast period, as
                                    * an incrementer thru all the periods. */
   int roundedSkyCoverData;   /* Returned rounded data. */
   xmlNodePtr cloud_amount = NULL;  /* Xml Node Pointer for <cloud-amount>
                                     * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   
   /* Set the first iteration to the incoming user supplied startTime if 
    * product is a summarization.
    */
   if (f_XML == 3 || f_XML == 4)
      timeUserStartStep = timeUserStart;

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
   
   /* Loop over all the Sky Cover values. Format them if the product is of
    * type DWMLgen. Collect them in the maxSkyCover, minSkyCover, and
    * averageSkyCover arrays if the product is of type DWMLgenByDay. We also 
    * need to collect info such as the startPositions and endPositions of
    * the periods, and the positions where the max and min sky covers are
    * found in each period.
    */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_SKY && 
	  match[i].validTime >= numRows.firstUserTime &&
	  match[i].validTime <= numRows.lastUserTime)
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
         else if ((f_XML == 3 || f_XML == 4) && currentDay < *numOutputLines) 
		                            /* We don't format any sky cover
                                             * for these products, but simply
                                             * collect Max, Min, and Avg values
                                             * for sky cover for use in icon
                                             * determination. */
         {

            /* Loop over each day if the requested format is 24 hourly, or
             * each 12 hour period if the requested format is 12 hourly. We
             * use a time interval in which the number of seconds in either a
             * 24 or 12 hour period determines the integer start time. 
	     */
            *SKYintegerTime = match[i].validTime;

            /* Now we have to account for data that is just in the time
             * period i.e. if data is valid from 4PM-7PM and time period is
             * from 6AM-6PM. We shift data by one half the data's period in
             * seconds. 
	     */
	    if ((i - (priorElemCount + startNum)) < 1)
               period = determinePeriodLength(match[i].validTime,
                               match[i + 1].validTime, 
			       (numRows.total-numRows.skipBeg-numRows.skipEnd),
                               parameterName);
            else
               period = determinePeriodLength(match[i - 1].validTime,
                               match[i].validTime,
                               (numRows.total-numRows.skipBeg-numRows.skipEnd), 
			       parameterName);

      	       *SKYintegerTime = *SKYintegerTime - (((double)period * 0.5) * 3600);

            /* Determine if this time is within the current day (period) being
             * processed. 
	     */
            if ((timeUserStartStep <= *SKYintegerTime)
                && (*SKYintegerTime < timeUserStartStep + timeInterval))
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
	             maxSkyNum[currentDay] = (i - priorElemCount) - startNum;
		  }

                  /* Get minSkyCover values and record it's index. */
                  if (roundedSkyCoverData < minSkyCover[currentDay])
	          {
                     minSkyCover[currentDay] = roundedSkyCoverData;
	             minSkyNum[currentDay] =  (i - priorElemCount) - startNum;
	          }
		  
                  /* The cloud phrasing algorithm in skyPhrase() routine needs
		   * to know the start index of each day. So, capture that 
		   * information here. 
		   */
                  if (firstTime)
	          {
	             startPositions[currentDay] = (i - priorElemCount) - startNum;
		     firstTime = 0;
		  }
		  
                  /* Gather data for calculating averageSkyCover values. */
                  totalSkyCover += roundedSkyCoverData;
                  numSkyCoverValues++;
	       }
            }
	    
            forecastPeriod = ((*SKYintegerTime - timeUserStartStep) / 3600);
    
            /* Calculate the average sky cover for use in phrase algorithm. First,
             * check to see if we need to jump into the next forecast period. 
             */
            if (f_XML == 3)
            {
	       if ((forecastPeriod + period >= 12) ||  
		  (((i - priorElemCount) - startNum) == 
		  (numRows.total-numRows.skipBeg-numRows.skipEnd) - 1)) 
		   /* Accounts for last data row. */
               {
                  if (numSkyCoverValues > 0)
                     averageSkyCover[currentDay] = (int)myRound((totalSkyCover / numSkyCoverValues), 0);
	          else
		     averageSkyCover[currentDay] = -999;

                  /* Re-initialize some things for next iteration of summary period. */
                  totalSkyCover = 0;
                  numSkyCoverValues = 0;
	          endPositions[currentDay] = (i - priorElemCount) - startNum;
	          firstTime = 1;
                  currentDay++;
                  timeUserStartStep = timeUserStart + (currentDay *
                                      timeInterval);
                  forecastPeriod = 0;
	       }
            }
            else if (f_XML == 4)
	    {
               if ((forecastPeriod + period >= 24) || 
	          (((i - priorElemCount) - startNum) == 
		  (numRows.total-numRows.skipBeg-numRows.skipEnd) - 1))
		  /* Accounts for last data row. */
               {
                  if (numSkyCoverValues > 0)
                     averageSkyCover[currentDay] = (int)myRound((totalSkyCover / numSkyCoverValues), 0);
	          else
		     averageSkyCover[currentDay] = -999;

                  /* Re-initialize some things for next iteration of summary period. */
	          totalSkyCover = 0;
                  numSkyCoverValues = 0;
	          endPositions[currentDay] = (i - priorElemCount) - startNum;
	          firstTime = 1;
                  currentDay++;
                  timeUserStartStep = timeUserStart + (currentDay *
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
      numNils = *numOutputLines - currentDay;
      if (numNils > 0)
      {
         for (i = currentDay; i < *numOutputLines; i++)
	 {
            averageSkyCover[currentDay] = -999;
	 }
      }
   }
   
   return;
}
