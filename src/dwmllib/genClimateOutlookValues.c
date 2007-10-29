/******************************************************************************
 * genClimateOutlookValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the 12 Climate Outlook Anomaly Probabilities in the 
 *  DWMLgen time-series product (f_XML = 1).
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Climate Anomalies to their valid times 
 *               (ex. k-p24h-n8-1). (Input)
 * parameterName = Number in NDFD_ENUM denoting the NDFD element currently 
 *                 processed. (Input) 
 *       match = Pointer to the array of element matches from degrib. (Input) 
 * climateOutlookType = Character string denoting the attribute "type" of 
 *                      the climate outlook to be formatted. (Input)
 * climateOutlookName = Character string denoting the element <name> of the 
 *                      climate outlook to be formatted. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Input/Output)
 *     numRows = Number of rows data is formatted for in the output XML. Used
 *               in DWMLgenByDay's "12 hourly" and "24 hourly" products. 
 *               "numRows" is determined using numDays and is used as an added 
 *               criteria (above and beyond simply having data exist for a 
 *               certain row) in formatting XML for these two products. (Input)
 * startTime_cml = The startTime entered as an option on the command line. 
 *                 (Input)
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
 *   8/2007 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genClimateOutlookValues(size_t pnt, char *layoutKey, uChar parameterName, 
                             genMatchType *match, char *climateOutlookType, 
                             char *climateOutlookName, xmlNodePtr parameters, 
                             numRowsInfo numRows, int startNum, int endNum)
{
   int i;
   int roundedClimateData;            /* Returned rounded data. */
   xmlNodePtr climate_anomaly = NULL; /* Xml Node Pointer for <climate-anomaly>
                                       * element. */
   xmlNodePtr climate_period = NULL;  /* Xml Node Pointer for <climate-period>
                                       * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer. */

   /* Format the <climate-anomaly> element. */
   climate_anomaly = xmlNewChild(parameters, NULL, BAD_CAST 
                                 "climate-anomaly", NULL);
   /* Check inside <name> of element to find the climate period length in 
    * question to display as new child element.
    */
   if (strstr(climateOutlookName, "8-14 Day") != '\0')       
      strcpy (strBuff, "weekly");
   else if (strstr(climateOutlookName, "One-Month") != '\0')       
      strcpy (strBuff, "monthly");
   else
      strcpy (strBuff, "seasonal");

   /* Format the climate period element in XML. */
   climate_period = xmlNewChild(climate_anomaly, NULL, BAD_CAST strBuff, NULL);
   xmlNewProp(climate_period, BAD_CAST "type", BAD_CAST climateOutlookType);
   xmlNewProp(climate_period, BAD_CAST "units", BAD_CAST "percent");
   xmlNewProp(climate_period, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(climate_period, NULL, BAD_CAST "name", BAD_CAST 
               climateOutlookName);

   /* Loop over all the data and format the <value> element. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == parameterName && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {
         /* If the data is missing, so indicate in the XML (nil=true). */
         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(climate_period, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedClimateData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedClimateData);
            xmlNewChild(climate_period, NULL, BAD_CAST "value", BAD_CAST strBuff);
         }
      }
   }
   return;
}
