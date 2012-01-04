/******************************************************************************
 * genWindSpeedValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code performs two functions: 
 *            1) Formats the Wind Speed element in the "time-series" (f_XML = 1
 *               or 6) and "glance" (f_XML = 2) DWMLgen products. 
 *            2) Collects the Max Wind Speed values per day for icon 
 *               determination in the DWMLgenByDay products. It also denotes
 *               the wind direction occurred at the time of max wind speed.
 *
 * ARGUMENTS
  *  timeUserStart = The beginning of the first forecast period (06 hr
 *                  or 18hr) based on the user supplied startTime argument. 
 *                  (Input)
 *    timeUserEnd = The end of the last forecast period (18 hr) based 
 *                  on the startTime & numDays arguments. (Input)
 *            pnt = Current Point index. (Input)
 *      layoutKey = The key linking the Wind Speeds to their valid times 
 *                  (ex. k-p3h-n42-3). (Input)
 *          match = Pointer to the array of element matches from degrib. (Input) 
 *     parameters = An xml Node Pointer denoting the <parameters> node to which 
 *                  these values will be attached (child node). (Output)
 *      startDate = Point specific user supplied start Date (i.e. 2006-04-15). 
 *                  Used for DWMLgenByDay products. (Input)
 *   maxWindSpeed = Array containing the Maximum wind speed values corresponding
 *                  to a day (24 hour format) or 12 hour period (12 hour format).
 *                  These values are used in deriving the weather and/or icon values. 
 *                  Only used for DWMLgenByDay products. (Output)
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
 *                     4 = DWMLgenByDay's "24 hourly" format product.
 *                     5 = DWMLgen's RTMA "time-series" product.
 *                     6 = DWMLgen's RTMA + NDFD "time-series" product. (Input)
 * valTimeForWindDirMatch = Array with the validTimes that corresponds to the 
 *                          times when the max wind speeds are the highest.  We 
 *                          then collect the wind directions that correspond
 *                          to the same times when the wind speeds are the 
 *			    highest. Only used for DWMLgenByDay products. 
 *                          (Output)
 *      startTime = Incoming argument set by user as a double in seconds 
 *                  since 1970 denoting the starting time data is to be 
 *                  retrieved for from NDFD. (Set to 0.0 if not entered.)
 *                  (Input) 
 *       startNum = First index in match structure an individual point's data 
 *                  matches can be found. (Input)
 *         endNum = Last index in match structure an individual point's data
 *                  matches can be found. (Input) 
 *    f_shiftData = Flag used to determine whether we shift data back by 1/2
 *                  it's period to denote the duration of time the data is 
 *                  valid for (Soap Service), or to not shift, and simply use
 *                  the data's endTime (validTime) (JimC's TPEX, FOX, products
 *                  etc) (Input)
 *         f_unit = 0 (GRIB unit), 1 (english), 2 (metric) (Input)
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
 *  11/2007 Paul Hershberg (MDL): Added flag that determines whether to shift data 
 *                                back by 1/2 the period length (Soap Service) or 
 *                                not (JimC's TPEX, SSC, etc). 
 *   2/2008 Paul Hershberg (MDL): Added code to detect if change from DST to 
 *                                Standard Time (or vice versa) occurred sometime 
 *                                in the forecast. 
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genWindSpeedValues(double timeUserStart, double timeUserEnd, size_t pnt, 
                        char *layoutKey, genMatchType * match,
                        xmlNodePtr parameters, char *startDate,
                        int *maxWindSpeed, int *numOutputLines, int timeInterval,
                        sChar TZoffset, sChar f_observeDST, uChar parameterName,
                        numRowsInfo numRows, uChar f_XML,
                        double *valTimeForWindDirMatch, double startTime, 
                        int startNum, int endNum, int f_shiftData, sChar f_unit)
{
   int i; /* Counter thru match structure. */
   int period = 3; /* Length between an elements successive validTimes. */
   int forecastPeriod = 0; /* Used as a counter thru the summarization (forecast) 
                            * period used to determine if current data being 
                            * processed belongs in current forecast period. */ 
   int priorElemCount = 0; /* Used to subtract prior elements when looping 
                            * thru matches. */
   int currentDay = 0;  /* Counter used to denote incrementing thru the 
                         * summarization (forecast) periods.*/      
   double timeUserStartStep = 0; /* Used to denote each forecast period, as
                                  * an incrementer thru all the periods. */
   int WSintegerTime = 0; /* Data's valid Time in integer form. */
   int roundedWindSpeedData;  /* Returned rounded data. */
   xmlNodePtr wind_speed = NULL;  /* Xml Node Pointer for <wind-speed>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   char month[3];             /* String holding formatted month to see if a 
                                 change from standard to daylight savings time
                                 (or vice versa) occurs within forecast period
                                 in question. */
   int interval = timeInterval; /* Used in case DST to Standard time (or vice 
                                 * versa) occurs sometime in the forecast. */
   int f_DSTswitchFound = 0; /* Used to detect of there was a switch from  
                              * Standard to DST in Spring or vice versa in 
                              * Fall. */

   /* Set the first iteration to the incoming user supplied startTime if 
    * product is a summarization.
    */
   if (f_XML == 3 || f_XML == 4)
      timeUserStartStep = timeUserStart;

   /* If the product is of type DWMLgen, format the <wind_speed> element. */
   if (f_XML == 1 || f_XML == 2 || f_XML == 6)
   {
      wind_speed = xmlNewChild(parameters, NULL, BAD_CAST "wind-speed", NULL);
      xmlNewProp(wind_speed, BAD_CAST "type", BAD_CAST "sustained");

      if (f_unit != 2)
         xmlNewProp(wind_speed, BAD_CAST "units", BAD_CAST "knots");
      else
         xmlNewProp(wind_speed, BAD_CAST "units", BAD_CAST "meters/second");

      xmlNewProp(wind_speed, BAD_CAST "time-layout", BAD_CAST layoutKey);

      /* Format the display <name> element. */
      xmlNewChild(wind_speed, NULL, BAD_CAST "name", BAD_CAST "Wind Speed");
   }

   /* Loop over all the Wind Speed values. Format them if the product is of
    * type DWMLgen. Collect them in the maxWindSpeed array if the product is
    * of type DWMLgenByDay. 
    * */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WS && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {
         if (f_XML == 1 || f_XML == 2 || f_XML == 6)  /* DWMLgen products. */
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
         else if ((f_XML == 3 || f_XML == 4) && (currentDay < *numOutputLines))
		                            /* We don't format any wind for
                                             * these products, but simply get
                                             * Max values for wind speed and
                                             * wind dir per summarization 
					     * period for use in icon
                                             * determination. */
         {

            /* Loop over each day if the requested format is 24 hourly, or
             * each 12 hour period if the requested format is 12 hourly. We
             * use a time interval in which the number of seconds in either a
             * 24 or 12 hour period determines the integer start time. 
	     */
            WSintegerTime = match[i].validTime;

            /* Now we have to account for data that is just in the time
             * period i.e. if data is valid from 4PM - 7PM and time period is
             * from 6AM - 6PM. We shift data by one half the data's period in
             * seconds. 
	     */
            if (f_shiftData)
            {
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

	       WSintegerTime = WSintegerTime - (((double)period * 0.5) * 3600);
            }

            /* See if the forecast period time interval is not quite 12 
             * (f_XML = 3) or 24 (f_XML = 4) hours due to a change in DST 
             * occurring in a specific forecast period. In the Spring, 
             * "spring" forward in time resulting in a reduction in 1 hour in 
             * the forecast period. Do the opposite if in Fall.
             */
            if (!f_DSTswitchFound)
            {
               formatValidTime(timeUserStartStep, strBuff, 30, TZoffset, 
                               f_observeDST);
               month[0] = strBuff[5];
               month[1] = strBuff[6];
               month[2] = '\0';

               if (atoi(month) < 6) 
               {
                  if ((Clock_IsDaylightSaving2(timeUserStartStep, TZoffset) != 1) 
                     && (Clock_IsDaylightSaving2(timeUserStartStep + interval, 
                     TZoffset) == 1))
                  {
                     interval = interval - 3600;
                     f_DSTswitchFound = 1; 
                  }
               }
               else
               { 
                  if ((Clock_IsDaylightSaving2(timeUserStartStep, TZoffset) == 1) 
                     && (Clock_IsDaylightSaving2(timeUserStartStep + interval, 
                     TZoffset) != 1))
                  {
                     f_DSTswitchFound = 1; 
                     interval = interval + 3600;
                  }
               }
            }

            /* Determine if this time is within the current day being
             * processed. */
            if ((timeUserStartStep <= WSintegerTime) &&
                (WSintegerTime < (timeUserStartStep + interval)))
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
                     valTimeForWindDirMatch[currentDay] = WSintegerTime;
                  }
               }
            }
            forecastPeriod = ((WSintegerTime - timeUserStartStep) / 3600);

            if (f_XML == 3 && ((forecastPeriod + period) >= (interval/3600)))
            {
               currentDay++;

               /* If there was a change from Standard to DST (or vice versa),
                * detect this and add accordingly to get the time of the 
                * next forecast period. 
                */
               if (f_DSTswitchFound)
               {                  
                  if (atoi(month) < 6) /* Spring. */
                     timeUserStartStep = timeUserStart + 
                                         (currentDay * timeInterval) - 3600;
                  else /* Fall. */
                     timeUserStartStep = timeUserStart + 
                                         (currentDay * timeInterval) + 3600;
               }
               else
                  timeUserStartStep = timeUserStart +
                                      (currentDay * timeInterval);

               interval = timeInterval;
               forecastPeriod = 0;
            }
            else if (f_XML == 4 && ((forecastPeriod + period) >= 
                    (interval/3600)))
            {
               currentDay++;

               /* If there was a change from Standard to DST (or vice versa),
                * detect this and add accordingly to get the time of the 
                * next forecast period. 
                */
               if (f_DSTswitchFound)
               {                  
                  if (atoi(month) < 6) /* Spring. */
                     timeUserStartStep = timeUserStart + 
                                         (currentDay * timeInterval) - 3600;
                  else /* Fall. */
                     timeUserStartStep = timeUserStart + 
                                         (currentDay * timeInterval) + 3600;
               }
               else
                  timeUserStartStep = timeUserStart +
                                      (currentDay * timeInterval);

               interval = timeInterval;
               forecastPeriod = 0;
            }
         }
      }
      else
         priorElemCount++;
   }
   return;
}
