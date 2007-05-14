/******************************************************************************
 * genConvSevereCompValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Convective Hazard Outlooks in the "time-series" 
 *  DWMLgen product for:
 *     Probability of Tornadoes (Day 1)
 *     Probability of Hail (Day 1)
 *     Probability of Damaging Thunderstorm Winds (Day 1)
 *     Probability of Extreme Tornadoes (Day 1)
 *     Probability of Extreme Hail (Day 1)
 *     Probability of Extrmem Thunderstorm Winds (Day 1)
 *     Total Probability of Severe Thunderstorm Winds (Day 2 - Day 3)
 *     Total Probability of Extrmem Severe Thunderstorm Winds (Day 2 - Day 3)
 *
 * ARGUMENTS
 *           pnt = Current Point index. (Input)
 *     layoutKey = The key linking the Wind Speed Gusts to their valid times 
 *                 (ex. k-p3h-n42-3). (Input)
 * parameterName = Number in NDFD_ENUM denoting the NDFD element currently 
 *                 processed. (Input) 
 *         match = Pointer to the array of element matches from degrib. (Input) 
 * severeCompType = Character string denoting the type of severe component to 
 *                  be formatted. (Input)
 * severeCompName = Character string denoting the name of the severe component to 
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
 *   3/2007 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genConvSevereCompValues(size_t pnt, char *layoutKey, uChar parameterName, 
                             genMatchType *match, char *severeCompType, 
                             char *severeCompName, xmlNodePtr parameters, 
                             numRowsInfo numRows, int startNum, int endNum)
{
   int i;                             /* Index through match structure. */
   int roundedProbData;               /* Returned rounded probability data. */
   xmlNodePtr convective_hazard = NULL; /* Xml Node Pointer for 
                                         * <convective-hazard> element. */
   xmlNodePtr severe_component = NULL; /* Xml Node Pointer for 
                                        * <severe-component> element. */
   xmlNodePtr value = NULL;           /* Xml Node Pointer for <value> element. */
   char strBuff[30];                  /* Temporary string buffer holding rounded
                                       * data. */

   /* Format the <convective-hazard> element. */
   convective_hazard = xmlNewChild(parameters, NULL, BAD_CAST 
                                   "convective-hazard", NULL);

  /* Format the <severe-component> element. */
   severe_component = xmlNewChild(convective_hazard, NULL, BAD_CAST 
                                  "severe-component", NULL);
   xmlNewProp(severe_component, BAD_CAST "type", BAD_CAST severeCompType);
   xmlNewProp(severe_component, BAD_CAST "units", BAD_CAST "percent");
   xmlNewProp(severe_component, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(severe_component, NULL, BAD_CAST "name", BAD_CAST
               severeCompName);

   /* Loop over all the data values and format them. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == parameterName && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {
         /* If the data is missing, so indicate in the XML (nil=true). */
         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(severe_component, NULL, BAD_CAST "value", 
                    NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            roundedProbData = (int)myRound(match[i].value[pnt].data, 0);
            sprintf(strBuff, "%d", roundedProbData);
            xmlNewChild(severe_component, NULL, BAD_CAST "value", BAD_CAST 
                        strBuff);
         }
      }
   }
   return;
}
