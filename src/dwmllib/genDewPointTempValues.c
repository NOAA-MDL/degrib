/******************************************************************************
 * genDewPointTempValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Dew Point element in the "time-series" DWMLgen 
 *  product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Min Temps to their valid times 
 *               (ex. k-p24h-n7-1). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 *     numRows = Structure containing members: (Input)
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
 *   4/2009 Paul Hershberg (MDL): Put some common sense checks on data. 
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genDewPointTempValues(size_t pnt, char *layoutKey, genMatchType *match, 
                           xmlNodePtr parameters, numRowsInfo numRows, 
                           int startNum, int endNum, sChar f_unit)
{
   int i;
   int roundedTdData;         /* Returned rounded data. */
   xmlNodePtr temperature = NULL; /* Xml Node Pointer for <temperature>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <temperature> element. */
   temperature = xmlNewChild(parameters, NULL, BAD_CAST "temperature", NULL);
   xmlNewProp(temperature, BAD_CAST "type", BAD_CAST "dew point");

   if (f_unit != 2)
      xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Fahrenheit");
   else
      xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Celsius");

   xmlNewProp(temperature, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(temperature, NULL, BAD_CAST "name", BAD_CAST
               "Dew Point Temperature");

   /* Loop over all the data values and format them. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_TD && 
	  match[i].validTime >= numRows.firstUserTime &&
	  match[i].validTime <= numRows.lastUserTime)
      {
         /* If the data is missing, so indicate in the XML (nil=true). 
          * Also, put some common sense checks on the data.
          */
         if (match[i].value[pnt].valueType == 2 || 
             match[i].value[pnt].data > 300 || 
             match[i].value[pnt].data < -300 || 
             match[i].value[pnt].data == '\0')
         {
            value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedTdData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedTdData);
            xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}
