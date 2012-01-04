/******************************************************************************
 * genWaveHeightValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Wave Height element in the "time-series" 
 *  DWMLgen product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Wave Heights to their valid times 
 *               (ex. k-p3h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
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
 *      f_unit = 0 (GRIB unit), 1 (english), 2 (metric) (Input)
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
void genWaveHeightValues(size_t pnt, char *layoutKey, genMatchType * match, 
                         xmlNodePtr parameters, numRowsInfo numRows, 
                         int startNum, int endNum, sChar f_unit)
{
   int i;                     /* Element counter thru match structure. */
   int roundedWaveHeightData; /* Returned rounded data. */
   xmlNodePtr water_state = NULL; /* Xml Node Pointer for <water-state>
                                   * element. */
   xmlNodePtr waves = NULL;   /* Xml Node Pointer for <waves> element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <water-state> element. */
   water_state = xmlNewChild(parameters, NULL, BAD_CAST "water-state", NULL);
   xmlNewProp(water_state, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format <waves> element name. */
   waves = xmlNewChild(water_state, NULL, BAD_CAST "waves", NULL);
   xmlNewProp(waves, BAD_CAST "type", BAD_CAST "significant");

   if (f_unit != 2)
      xmlNewProp(waves, BAD_CAST "units", BAD_CAST "feet");
   else
      xmlNewProp(waves, BAD_CAST "units", BAD_CAST "meters");

   /* Format the display <name> element. */
   xmlNewChild(waves, NULL, BAD_CAST "name", BAD_CAST "Wave Height");

   /* Loop over all the data values and format them. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WH && 
	  match[i].validTime >= numRows.firstUserTime &&
	  match[i].validTime <= numRows.lastUserTime)
      {
         /* If the data is missing, so indicate in the XML (nil=true). */
         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(waves, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedWaveHeightData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedWaveHeightData);
            xmlNewChild(waves, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}
