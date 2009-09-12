/******************************************************************************
 * genWindIncCumValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Wind Speed Incremental and Cumulative Probability 
 *  Threshold elements in the "time-series" DWMLgen product.
 *
 * ARGUMENTS
 *           pnt = Current Point index. (Input)
 *     layoutKey = The key linking the Wind Speed Gusts to their valid times 
 *                 (ex. k-p3h-n42-3). (Input)
 * parameterName = Number in NDFD_ENUM denoting the NDFD element currently processed. 
 *                 (Input) 
 *         match = Pointer to the array of element matches from degrib. (Input) 
 * windSpeedType = Character string denoting the type of wind speed to be 
 *                 formatted. (Input)
 * windSpeedName = Character string denoting the name of the wind speed to be 
 *                 formatted. (Input)
 *    parameters = An xml Node Pointer denoting the <parameters> node to which 
 *                 these values will be attached (child node). (Output)
 *       numRows = Structure containing members: (Input)
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
 *   9/2006 Paul Hershberg (MDL): Created
 *   7/2009 Paul Hershberg (MDL): Put some common sense checks on data. 
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genWindIncCumValues(size_t pnt, char *layoutKey, uChar parameterName,
		         genMatchType *match, char *windSpeedType, 
		         char *windSpeedName, xmlNodePtr parameters,
		         numRowsInfo numRows, int startNum, int endNum)
{
   int i;                             /* Index through match structure. */
   int roundedWindICData;             /* Returned rounded probability data. */
   xmlNodePtr wind_speed_prob = NULL; /* Xml Node Pointer for <wind_speed_prob>
                                       * element. */
   xmlNodePtr value = NULL;           /* Xml Node Pointer for <value> element. */
   char strBuff[30];                  /* Temporary string buffer holding rounded
                                       * data. */

   /* Format the <wind_speed_prob> element. */
   wind_speed_prob = xmlNewChild(parameters, NULL, BAD_CAST "wind-speed", NULL);
   xmlNewProp(wind_speed_prob, BAD_CAST "type", BAD_CAST windSpeedType);
   xmlNewProp(wind_speed_prob, BAD_CAST "units", BAD_CAST "percent");
   xmlNewProp(wind_speed_prob, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(wind_speed_prob, NULL, BAD_CAST "name", BAD_CAST
               windSpeedName);

   /* Loop over all the data values and format them. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == parameterName && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {
         /* If the data is missing, so indicate in the XML (nil=true). */
         if (match[i].value[pnt].valueType == 2 ||
             match[i].value[pnt].data > 101 || 
             match[i].value[pnt].data < -1 || 
             match[i].value[pnt].data == '\0')
         {
            value = xmlNewChild(wind_speed_prob, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedWindICData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedWindICData);
            xmlNewChild(wind_speed_prob, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}
