/******************************************************************************
 * genRtmaValues.c() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the 10 Real Time Mesoscale Analyses elements in the 
 *  DWMLgen RTMA time-series product (f_XML = 5) or DWMLgen RTMA + NDFD 
 *  time-series product (f_XML == 6).
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the RTMA elements to their valid times 
 *               (ex. k-p24h-n8-1). (Input)
 * parameterName = Number in NDFD_ENUM denoting the RTMA element currently 
 *                 processed. (Input) 
 *   errorName = 1 if RTMA has corresponding uncertainty. 0 if not. (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    rtmaName = Character string denoting the <name> of the rtma element to 
 *               be formatted. (Input)
 * rtmaElement = Character string denoting the <(element)> tag. (Input)
 *    rtmaType = Character string denoting the type attribute of the <(element)>
 *               tag to be formatted. (Input)
 *   rtmaUnits = Character string denoting the units attribute of the <(element)>
 *               tag to be formatted. (Input)
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
 *       units = Holder for metric or U.S. Standard units. (Input)
 *
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  11/2007 Paul Hershberg (MDL): Created
 *   1/2008 Paul Hershberg (MDL): Altered to display Temp and Dewpoint's errors
 *                                to the 10th's precision.
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genRtmaValues(size_t pnt, char *layoutKey, uChar parameterName,
                   int errorName, genMatchType *match, char *rtmaName, 
                   char *rtmaElement, char *rtmaType, char *units, 
                   xmlNodePtr parameters, numRowsInfo numRows, 
                   int startNum, int endNum)
{
   int i;
   int j;
   int roundedRtmaData;       /* Returned rounded RTMA data. */
   int f_uncertaintyAttributeWasSet = 0;
   double roundedPrecipaData; /* Returned rounded RTMA Precipa Amnt data. */ 
   xmlNodePtr element = NULL; /* Xml Node Pointer for the <(weather-element)>
                               * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   xmlNodePtr uncertainty = NULL; /* Xml Node Pointer for <uncertainty> 
                                   * element. */
   xmlNodePtr error = NULL;   /* Xml Node Pointer for <error> element. */
   xmlNodePtr valueWithUncertainty = NULL; /* Xml Node Pointer for 
                                            * <valueWithUncertainty> element. */
   char strBuff[30];          /* Temporary string buffer. */
   double errVal;          /* Error value in Kelvin. */
   double rtmaParentTime; /* Time of parent RTMA elements. Used to match up with 
                             corresponding error. */

   /* Format the rtma element. */
   element = xmlNewChild(parameters, NULL, BAD_CAST rtmaElement, NULL);
   xmlNewProp(element, BAD_CAST "type", BAD_CAST rtmaType);
   xmlNewProp(element, BAD_CAST "units", BAD_CAST units);
   xmlNewProp(element, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(element, NULL, BAD_CAST "name", BAD_CAST rtmaName);

   /* Loop over all the data and format the <value> element. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == parameterName && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {
         /* Find time of parent element to match with corresponding error. */
         rtmaParentTime = match[i].validTime;

         if (errorName > 0) /* RTMA elements wdir_r, wspd_r, temp_r, & td_r. */
         {
            valueWithUncertainty = xmlNewChild(element, NULL, BAD_CAST 
                                               "valueWithUncertainty", NULL);
            f_uncertaintyAttributeWasSet = 0;

            /* If the <value> data is missing, so indicate in the XML 
             * (nil=true). 
             */
            if (match[i].value[pnt].valueType == 2)
            {
               xmlNewProp(valueWithUncertainty, BAD_CAST "xsi:nil", BAD_CAST "true");
            }
            else if (match[i].value[pnt].valueType == 0) /* Format good data. */
            {
               xmlNewProp(valueWithUncertainty, BAD_CAST "type", BAD_CAST "RTMA");
               roundedRtmaData = (int)myRound(match[i].value[pnt].data, 0);
               sprintf(strBuff, "%d", roundedRtmaData);
               value = xmlNewChild(valueWithUncertainty, NULL, BAD_CAST 
                                   "value", BAD_CAST strBuff);

               uncertainty = xmlNewChild(valueWithUncertainty, NULL, BAD_CAST
                                         "uncertainty", NULL);

               /* Find the corresponding error. */ 
               for (j = startNum; j < endNum; j++) 
               {
                  if (match[j].elem.ndfdEnum == errorName && 
	              match[j].validTime == rtmaParentTime)
                  {
                     f_uncertaintyAttributeWasSet = 1;

                     if (match[j].value[pnt].valueType == 2)
                     {
                        xmlNewProp(uncertainty, BAD_CAST "type", BAD_CAST
                                   "analysis error");
                        xmlNewProp(uncertainty, BAD_CAST "xsi:nil", BAD_CAST "true" );
                     }

                     /* Format good data. */
                     else if (match[j].value[pnt].valueType == 0)
                     {
                        xmlNewProp(uncertainty, BAD_CAST "type", BAD_CAST 
                                   "analysis error");
                        /* Convert 2 Temp error values from Kelvin to 
                         * Farhenheit. The error values will also be displayed
                         * to the 10th's precision.
                         */
                        if (match[j].elem.ndfdEnum == RTMA_UTEMP || 
                            match[j].elem.ndfdEnum == RTMA_UTD)
                        {
                           errVal = match[j].value[pnt].data;
                           errVal = errVal*1.8;

                           sprintf(strBuff, "%2.1f", errVal);
                           error = xmlNewChild(uncertainty, NULL, BAD_CAST 
                                               "error", BAD_CAST strBuff);
                        }
                        else
                        {
                           roundedRtmaData = 
                                     (int)myRound(match[j].value[pnt].data, 0);
                           sprintf(strBuff, "%d", roundedRtmaData);
                           error = xmlNewChild(uncertainty, NULL, BAD_CAST 
                                               "error", BAD_CAST strBuff);
                        }

                        xmlNewProp(error, BAD_CAST "qualifier", BAD_CAST "+/-");
                     }
                     break;
                  }
               }
               if (!f_uncertaintyAttributeWasSet)
               {
                  xmlNewProp(uncertainty, BAD_CAST "type", BAD_CAST
                             "analysis error");
                  xmlNewProp(uncertainty, BAD_CAST "xsi:nil", BAD_CAST "true" );                           } 
            }
         }
         else /* precipa_r and sky_r don't have associated uncertainties. */
         {
            /* If the <value> data is missing, so indicate in the XML 
             * (nil=true). 
             */
            if (match[i].value[pnt].valueType == 2)
            {
               value = xmlNewChild(element, NULL, BAD_CAST "value", NULL);
               xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
            }
            else if (match[i].value[pnt].valueType == 0) /* Format good data. */
            {
               /* Format precipitation amount in decimal form. */
               if (match[i].elem.ndfdEnum == RTMA_PRECIPA)
               {
                  roundedPrecipaData = (float)myRound(match[i].value[pnt].data, 2);
                  sprintf(strBuff, "%2.2f", roundedPrecipaData);
               }
               else /* Format sky amount in normal integer form. */
               {
                  roundedRtmaData = (int)myRound(match[i].value[pnt].data, 0);
                  sprintf(strBuff, "%d", roundedRtmaData);
               }
               value = xmlNewChild(element, NULL, BAD_CAST "value", BAD_CAST 
                                   strBuff);
            }
         }
      }
   }
   return;
}
