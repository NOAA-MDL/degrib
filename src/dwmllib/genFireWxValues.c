/******************************************************************************
 * genFireWxValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the 2 Fire Weather Elements in the "time-series" 
 *  DWMLgen product:
 *     Fire Weather Outlook due to Wind and Relative Humidity (Valid Days 1-7)
 *     Fire Weather Outlook due to Dry Thunderstorms (Valid Days 1-3)
 *
 * ARGUMENTS
 *           pnt = Current Point index. (Input)
 *     layoutKey = The key linking the Wind Speed Gusts to their valid times 
 *                 (ex. k-p3h-n42-3). (Input)
 * parameterName = Number in NDFD_ENUM denoting the NDFD element currently 
 *                 processed. (Input) 
 *         match = Pointer to the array of element matches from degrib. (Input) 
 * fireWxType    = Character string denoting the type of fire weather to 
 *                  be formatted (due to wind/rel hum or dry lightning). (Input)
 * fireWxName    = Character string denoting the name of the fire weather to 
 *                  be formatted. (Input)
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
 *   1/2011 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genFireWxValues(size_t pnt, char *layoutKey, uChar parameterName, 
                     genMatchType *match, char *fireWxType, char *fireWxName, 
                     xmlNodePtr parameters, numRowsInfo numRows, int startNum, 
                     int endNum)
{
   int i;                             /* Index through match structure. */
   int fireWxData;                    /* Returned integer categorical data. */
   xmlNodePtr fire_wx = NULL;         /* Xml Node Pointer for 
                                       * <fire-wx> element. */
   xmlNodePtr value = NULL;           /* Xml Node Pointer for <value> element. */
   char transFireWxStr[200];    /* String holding english translation of the
                                 * Fire Wx Outlook. */
                  
   /* Format the <fire-wx> element. */
   fire_wx = xmlNewChild(parameters, NULL, BAD_CAST "fire-weather", NULL);

  /* Format the <fire-wx> attributes. */
   xmlNewProp(fire_wx, BAD_CAST "type", BAD_CAST fireWxType);
   xmlNewProp(fire_wx, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(fire_wx, NULL, BAD_CAST "name", BAD_CAST fireWxName);

   /* Loop over all the data <value> elements and format them. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == parameterName && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {
         /* If the data is missing, so indicate in the XML (nil=true). */
         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(fire_wx, NULL, BAD_CAST "value", 
                    NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            fireWxData = (int)match[i].value[pnt].data;
            getTranslatedFireWx(fireWxData, transFireWxStr);
            xmlNewChild(fire_wx, NULL, BAD_CAST "value", BAD_CAST
                        transFireWxStr);
         }
      }
   }
   return;
}
