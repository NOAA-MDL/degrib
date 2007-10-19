/******************************************************************************
 * genWindSpeedValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code performs two functions: 
 *            1) Formats the Wind Speed element in the "time-series" (f_XML = 1)
 *               and "glance" (f_XML = 2) DWMLgen products. 
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
 *                     4 = DWMLgenByDay's "24 hourly" format product. (Input)
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
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2006 Paul Hershberg (MDL): Created
 *  10/2007 Paul Hershberg (MDL): Removed code that shifted data back by 1/2
 *                                the period length (bug from php code)
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
                        int startNum, int endNum)
{
   int i; /* Counter thru match structure. */
   int period = 0; /* Length between an elements successive validTimes. */
   int forecastPeriod = 0; /* Used as a counter thru the summarization (forecast) 
                            * period used to determine if current data being 
                            * processed belongs in current forecast period. */ 
   int priorElemCount = 0; /* Used to subtract prior elements when looping 
                            * thru matches. */
   int currentDay = 0;  /* Counter used to denote incrementing thru the 
                         * summarization (forecast) periods.*/      
   double timeUserStartStep = 0; /* Used to denote each forecast period, as
                                  * an incrementer thru all the periods. */
   double WSdoubleTime = 0.0; /* Data's valid Time in double form. */
   int WSintegerTime = 0; /* Data's valid Time in integer form. */
   int roundedWindSpeedData;  /* Returned rounded data. */
   xmlNodePtr wind_speed = NULL;  /* Xml Node Pointer for <wind-speed>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   char WSstr[30];            /* Temporary string holding formatted time
                               * value of wind speed. */
   
   /* Set the first iteration to the incoming user supplied startTime if 
    * product is a summarization.
    */
   if (f_XML == 3 || f_XML == 4)
      timeUserStartStep = timeUserStart;

   /* If the product is of type DWMLgen, format the <wind_speed> element. */
   if (f_XML == 1 || f_XML == 2)
   {
      wind_speed = xmlNewChild(parameters, NULL, BAD_CAST "wind-speed", NULL);
      xmlNewProp(wind_speed, BAD_CAST "type", BAD_CAST "sustained");
      xmlNewProp(wind_speed, BAD_CAST "units", BAD_CAST "knots");
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
         if (f_XML == 1 || f_XML == 2)  /* DWMLgen products. */
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
            WSintegerTime = 0;
            WSstr[0] = '\0';

            formatValidTime(match[i].validTime, WSstr, 30, TZoffset,
                            f_observeDST);
            Clock_Scan(&WSdoubleTime, WSstr, 0);
            WSintegerTime = WSdoubleTime;

            /* Determine if this time is within the current day being
             * processed. */
            if ((timeUserStartStep <= WSintegerTime) &&
                (WSintegerTime < (timeUserStartStep + timeInterval)))
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
                     valTimeForWindDirMatch[currentDay] = match[i].validTime;
                  }
               }
            }
            forecastPeriod = ((WSintegerTime - timeUserStartStep) / 3600);

            if (f_XML == 3 && (forecastPeriod + period) >= 12)
            {
               currentDay++;
               timeUserStartStep = timeUserStart + (currentDay *
                                   timeInterval);
               forecastPeriod = 0;
            }
            else if (f_XML == 4 && (forecastPeriod + period) >= 24)
            {
               currentDay++;
               timeUserStartStep = timeUserStart + (currentDay * timeInterval);
               forecastPeriod = 0;
            }
         }
      }
      else
         priorElemCount++;
   }
   return;
}
