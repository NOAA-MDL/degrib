/******************************************************************************
 * genWindDirectionValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Wind Direction element in the "time-series" and 
 *  "glance" DWMLgen products. It also fills an array of wind direction values
 *  that occurred at the same time of max wind speed values (per summarization
 *  period)
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Wind Directions to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 * maxWindDirection = Array containing the wind direction values 
 *                    corresponding to a day (24 hour format) or 12 hour period
 *                    (12 hour format). These are not "max" wind direction 
 *                    values, but correspond to the time when the max wind 
 *                    speed values were found per forecast period.  These values
 *                    are used in deriving the weather and/or icon values. Only
 *                    used for DWMLgenByDay products. (Output)
 *                    (Output)
 *       f_XML = Flag for 1 of the 4 DWML products:
 *                    1 = DWMLgen's "time-series" product. 
 *                    2 = DWMLgen's "glance" product.
 *                    3 = DWMLgenByDay's "12 hourly" format product.
 *                    4 = DWMLgenByDay's "24 hourly" format product.
 *                    5 = DWMLgen's RTMA "time-series" product.
 *                    6 = DWMLgen's RTMA + NDFD "time-series" product. (Input)
 * numOutputLines = The number of data values output/formatted for each element. 
 *                  (Input)
 * valTimeForWindDirMatch = Array with the validTimes that corresponds to the 
 *                          times when the max wind speeds are the highest.  We 
 *                          then collect the wind directions that correspond
 *                          to the same times when the wind speeds are the 
 *			    highest. (Input)
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
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2006 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genWindDirectionValues(size_t pnt, char *layoutKey, genMatchType * match,
                            xmlNodePtr parameters, int *maxWindDirection,
                            uChar f_XML, int *numOutputLines,
	                    double *valTimeForWindDirMatch, 
		            numRowsInfo numRows, int startNum, int endNum)
{
   int i; /* Counter thru match structure. */
   int priorElemCount = 0; /* Used to subtract prior elements when looping 
                            * thru matches. */
   int currentDay = 0;  /* Counter used to denote incrementing thru the 
                         * summarization (forecast) periods.*/ 
   int roundedWindDirectionData;  /* Returned rounded data. */
   xmlNodePtr direction = NULL; /* Xml Node Pointer for <direction> element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* If first forecast period is empty (from finding the max wind speed), 
    * assign missing data value. 
    */
   if (f_XML == 3 || f_XML == 4)
   {
      if (valTimeForWindDirMatch[0] == -999.0)
         currentDay = currentDay + 1;
   }
      
   /* If the product is of type DWMLgen, format the <wind_direction> element. 
    */
   if (f_XML == 1 || f_XML == 2|| f_XML == 6)
   {
      direction = xmlNewChild(parameters, NULL, BAD_CAST "direction", NULL);
      xmlNewProp(direction, BAD_CAST "type", BAD_CAST "wind");
      xmlNewProp(direction, BAD_CAST "units", BAD_CAST "degrees true");
      xmlNewProp(direction, BAD_CAST "time-layout", BAD_CAST layoutKey);

      /* Format the display name. */

      xmlNewChild(direction, NULL, BAD_CAST "name", BAD_CAST "Wind Direction");
   }

   /* Loop over all the Wind Direction data values. Format them if the
    * product is of type DWMLgen. Collect them in the maxWindDirection array
    * if the product is of type DWMLgenByDay. 
    */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WD && 
	  match[i].validTime >= numRows.firstUserTime &&
	  match[i].validTime <= numRows.lastUserTime)
      {
         if (f_XML == 1 || f_XML == 2|| f_XML == 6)  /* DWMLgen products. */
         {
            /* If the data is missing, so indicate in the XML (nil=true). */
            if (match[i].value[pnt].valueType == 2)
            {
               value = xmlNewChild(direction, NULL, BAD_CAST "value", NULL);
               xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
            }
            else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                           * data. */
            {
               roundedWindDirectionData =
                     (int)myRound(match[i].value[pnt].data, 0);
               sprintf(strBuff, "%d", roundedWindDirectionData);
               xmlNewChild(direction, NULL, BAD_CAST "value", BAD_CAST strBuff);
            }
         }
         else if ((f_XML == 3 || f_XML == 4) && (currentDay < *numOutputLines))
		                            /* We don't format any wind for
                                             * these products, but simply get
                                             * Max. values for wind speed and
                                             * the wind dir that occured
					     * during the max speed time; for
					     * use in icon determination. */
         {
            if (valTimeForWindDirMatch[currentDay] == match[i].validTime)
            {
               maxWindDirection[currentDay] =
                     (int)myRound(match[i].value[pnt].data, 0);
               currentDay++;
            }
         }
      }
      else
         priorElemCount++;
   }
   return;
}
