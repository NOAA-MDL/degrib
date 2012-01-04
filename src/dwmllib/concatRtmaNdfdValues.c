/******************************************************************************
 * concatRtmaNdfdalues.c() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the 6 Concatenated RTMA + NDFD elements in the 
 *  DWMLgen RTMA+NDFD time-series product (f_XML = 6). The six elements 
 *  are:
 *      RTMA Temp + NDFD Temp.
 *      RTMA Dew Point + NDFD Dew Point.
 *      RTMA Wind Speed + NDFD Wind Speed
 *      RTMA Wind Direction + NDFD Direction
 *      RTMA Sky Cover + NDFD Sky Cover
 *      RTMA Precipitation Amount + NDFD QPF
 *
 *  Code first formats the RMTA portion of the element, then the NDFD portion
 *  of the element. RTMA uncertainties are not formatted for the concatenated
 *  product for consistancy sake with NDFD. 
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the RTMA elements to their valid times 
 *               (ex. k-p24h-n8-1). (Input)
 *    NDFDname = Number in NDFD_ENUM denoting the NDFD part of the concatenated
 *               element currently processed. (Input) 
 *    RTMAname = Number in NDFD_ENUM denoting the RTMA part of the concatenated
 *               element currently processed. (Input) 
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *        name = Character string denoting the <name> of the element to 
 *               be formatted. (Input)
 *  metElement = Character string denoting the <(element)> tag. (Input)
 *        type = Character string denoting the type attribute of the <(element)>
 *               tag to be formatted. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Input/Output)
 * numRowsRTMA = Number of rows data is formatted for in the output XML for 
 *               the RTMA element. This will be added to numRowsNDFD to get the
 *               total number of rows for the concatenated element. (Input)
 * numRowsNDFD = Number of rows data is formatted for in the output XML for 
 *               the NDFD element. This will be added to numRowsRTMA to get the
 *               total number of rows for the concatenated element. (Input)
 *    startNum = First index in match structure an individual point's data 
 *               matches can be found. (Input)
 *      endNum = Last index in match structure an individual point's data
 *               matches can be found. (Input)
 *       units = Holder of metric or US standard unit. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  2/2008 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void concatRtmaNdfdValues(size_t pnt, char *layoutKey, genMatchType *match, 
                          uChar NDFDname, uChar RTMAname, char *name, 
                          char *metElement, char *type, char *units, 
                          xmlNodePtr parameters, numRowsInfo numRowsNDFD, 
                          numRowsInfo numRowsRTMA, int startNum, int endNum)
{
   int i;
   int roundedIntData;       /* Returned rounded RTMA+NDFD integer data. */
   double roundedDoubleData; /* Returned rounded RTMA+NDFD doulbe data. */
   xmlNodePtr element = NULL; /* Xml Node Pointer for the <(weather-element)>
                               * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer. */

   /* Format the rtma + ndfd concatenated element. */
   element = xmlNewChild(parameters, NULL, BAD_CAST metElement, NULL);
   xmlNewProp(element, BAD_CAST "type", BAD_CAST type);
   xmlNewProp(element, BAD_CAST "units", BAD_CAST units);
   xmlNewProp(element, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(element, NULL, BAD_CAST "name", BAD_CAST name);

   /* Loop over all the RTMA data values first . */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == RTMAname && 
	  match[i].validTime >= numRowsRTMA.firstUserTime &&
	  match[i].validTime <= numRowsRTMA.lastUserTime)
      {
         /* If the data is missing, so indicate in the XML (nil=true). */
         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(element, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "type", BAD_CAST "RTMA");
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            if (match[i].elem.ndfdEnum == RTMA_PRECIPA)
            {
               roundedDoubleData = (float)myRound(match[i].value[pnt].data, 2);
               sprintf(strBuff, "%2.2f", roundedDoubleData);
            }
            else
            {    
               roundedIntData = (int)myRound(match[i].value[pnt].data, 0);
               sprintf(strBuff, "%d", roundedIntData);
            }
            value = xmlNewChild(element, NULL, BAD_CAST "value", BAD_CAST strBuff);
            xmlNewProp(value, BAD_CAST "type", BAD_CAST "RTMA");
         }
      }
   }
  
   /* Loop over all the corresponding NDFD values and concatentate them to the
    * end of the RTMA values.
    */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFDname && 
	  match[i].validTime >= numRowsNDFD.firstUserTime &&
	  match[i].validTime <= numRowsNDFD.lastUserTime)
      {
         /* If the data is missing, so indicate in the XML (nil=true). */
         if (match[i].value[pnt].valueType == 2)
         {
            value = xmlNewChild(element, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "type", BAD_CAST "NDFD");
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
         else if (match[i].value[pnt].valueType == 0) /* Format good data. */
         {
            if (match[i].elem.ndfdEnum == NDFD_QPF)
            {
               roundedDoubleData = (float)myRound(match[i].value[pnt].data, 2);
               sprintf(strBuff, "%2.2f", roundedDoubleData);
            }
            else
            {    
               roundedIntData = (int)myRound(match[i].value[pnt].data, 0);
               sprintf(strBuff, "%d", roundedIntData);
            }
            value = xmlNewChild(element, NULL, BAD_CAST "value", BAD_CAST strBuff);
            xmlNewProp(value, BAD_CAST "type", BAD_CAST "NDFD");
         }
      }
   }

   return;
}
