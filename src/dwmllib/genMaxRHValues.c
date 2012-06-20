/******************************************************************************
 * genMaxRHValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Max Relative Humidity element in the DWMLgen 
 *  product.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Max RH values to their valid times 
 *               (ex. k-p24h-n8-1). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 *     numRows = Number of rows data is formatted for in the output XML. Used
 *               in DWMLgenByDay's "12 hourly" and "24 hourly" products. 
 *               "numRows" is determined using numDays and is used as an added 
 *               criteria (above and beyond simply having data exist for a 
 *               certain row) in formatting XML for these two products. (Input)
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
 *   3/2012 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genMaxRHValues(size_t pnt, char *layoutKey, genMatchType * match,
                    xmlNodePtr parameters, numRowsInfo numRows,
                    int startNum, int endNum)
{
   int i;                     /* Element counter thru match structure. */
   int roundedMaxRHData;        /* Returned rounded data. */
   xmlNodePtr humidity = NULL; /* Xml Node Pointer for <humidity>
                                  element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <humidity> element. */
   humidity = xmlNewChild(parameters, NULL, BAD_CAST "humidity", NULL);
   xmlNewProp(humidity, BAD_CAST "type", BAD_CAST "maximum relative");

   xmlNewProp(humidity, BAD_CAST "units", BAD_CAST "percent");

   xmlNewProp(humidity, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(humidity, NULL, BAD_CAST "name", BAD_CAST
               "Daily Maximum Relative Humidity");

   /* Loop over all the data values and format them. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_MAXRH && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {

         /* If the data is missing, so indicate in the XML (nil=true).
          * Also, put some common sense checks on the data.
          */
         if (match[i].value[pnt].valueType == 2 ||
             match[i].value[pnt].data >= 101 ||
             match[i].value[pnt].data < -1 ||
             match[i].value[pnt].data == '\0')
         {
            value = xmlNewChild(humidity, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedMaxRHData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedMaxRHData);
            xmlNewChild(humidity, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}
