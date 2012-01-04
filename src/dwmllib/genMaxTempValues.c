/******************************************************************************
 * genMaxTempValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Max Temperature element in the DWMLgen and 
 *  DWMLgenByDay products.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the Max Temps to their valid times 
 *               (ex. k-p24h-n8-1). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 * f_formatNIL = We may have to create a NIL value for the first max temp if the
 *               request for data is late in the day. The variable f_formatNIL
 *               and firstNIL will have been set to = 1 to denote this case.
 *       f_XML = Flag denoting type of XML product (1 = DWMLgen's "time-series"
 *               product, 2 = DWMLgen's "glance" product, 3 = DWMLgenByDay's 
 *               "12 hourly" product, 4 = DWMLgenByDay's "24 hourly" product, 
 *               5 = DWMLgen's "RTMA time-series" product, 6 = DWMLgen's mix of 
 *               "RTMA & NDFD time-series" product.
 *               (Input) 
 *     numRows = Number of rows data is formatted for in the output XML. Used
 *               in DWMLgenByDay's "12 hourly" and "24 hourly" products. 
 *               "numRows" is determined using numDays and is used as an added 
 *               criteria (above and beyond simply having data exist for a 
 *               certain row) in formatting XML for these two products. (Input)
 * numFmtdRows = Number of output lines formatted in DWMLgenByDay products. 
 *               (Input)          
 * startTime_cml = The startTime entered as an option on the command line. 
 *                 (Input)
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
void genMaxTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                      xmlNodePtr parameters, int f_formatNIL, uChar f_XML, 
                      double startTime_cml, numRowsInfo numRows, 
                      int numFmtdRows, int startNum, int endNum, sChar f_unit)
{
   int i;                     /* Element counter thru match structure. */
   int numNils = 0;           /* Denotes diff between number of data rows and 
				 the number that need to be formatted for the 
				 DWMLgenByDay products. */
   int priorElemCount = 0;    /* Used to subtract prior elements when looping 
                               * thru matches. */
   int firstNIL = startNum;   /* Set to (startNum+1) if first Max Temp is set
                               * to "nil". 
			       */
   int roundedMaxData;        /* Returned rounded data. */
   xmlNodePtr temperature = NULL; /* Xml Node Pointer for <temperature>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */

   /* Format the <temperature> element. */
   temperature = xmlNewChild(parameters, NULL, BAD_CAST "temperature", NULL);
   xmlNewProp(temperature, BAD_CAST "type", BAD_CAST "maximum");

   if (f_unit != 2)
      xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Fahrenheit");
   else
      xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Celsius");

   xmlNewProp(temperature, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(temperature, NULL, BAD_CAST "name", BAD_CAST
               "Daily Maximum Temperature");

   /* If DWMLgen product, set numFmtdRows = to numRows since there is no set
    * number of rows we are ultimately formatting.
    */
   if (f_XML == 1 || f_XML == 2 || f_XML == 6)
      numFmtdRows = numRows.total-numRows.skipBeg-numRows.skipEnd;

   /* Format the first Max Temp Day as "nil = true" if f_formatNIL = 1
    * (applicable for DWMLgenByDay product's "24 hourly" format). 
    */
   if (f_XML == 4 && f_formatNIL)
   {
      value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
      xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
      firstNIL = startNum + 1;
   }

   /* Loop over all the data values and format them. */
   for (i = firstNIL; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_MAX && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {
         if (i-priorElemCount-startNum < numFmtdRows) /* Accounts for DWMLgenByDay. */
         {
            if (f_XML == 3 || f_XML == 4) /* DWMLgenByDay products. */
            {
               if (i-startNum < numRows.total-numRows.skipBeg-numRows.skipEnd)
               {

                  /* If the data is missing so indicate in the XML (nil=true). 
                   * Also, put some common sense checks on the data. 
                   */   
                  if (match[i].value[pnt].valueType == 2 || 
                      match[i].value[pnt].data > 300 || 
                      match[i].value[pnt].data < -300 || 
                      match[i].value[pnt].data == '\0')
                  {
                     value = xmlNewChild(temperature, NULL, BAD_CAST "value",
                                         NULL);
                     xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
                  }
                  else if (match[i].value[pnt].valueType == 0) /* Good data. */
                  {
                     roundedMaxData = (int)myRound(match[i].value[pnt].data, 0);
                     sprintf(strBuff, "%d", roundedMaxData);
                     xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST
                                 strBuff);
                  }
               }
            }
            else if (f_XML == 1 || f_XML == 2 || f_XML == 6) /* DWMLgen products. */
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
               else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                              * data. */
               {
                  roundedMaxData = (int)myRound(match[i].value[pnt].data, 0);
                  sprintf(strBuff, "%d", roundedMaxData);
                  xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST
                              strBuff);
               }
            }
	 }
      }
      else
         priorElemCount++;
   }

   /* In certain cases for the DWMLgenByDay products, we'll need to account for 
    * times when there may be less data in the match structure than the amount 
    * of data that needs to be formatted. These "extra" spaces will need to be 
    * formatted with a "nil" attribute. 
    */
   if (f_XML == 3 || f_XML == 4)  /* DWMLgenByDay products. */
   {
      /* Tally up the number of iterations that occurred thru the match 
       * structure and compare to the number of actual data rows that need to be
       * formatted to see if there is a difference.
       */
      numNils = numFmtdRows - (numRows.total-numRows.skipBeg-numRows.skipEnd);
      if (numNils > 0)
      {
         for (i = 0; i < numNils; i++)
	 {
            value = xmlNewChild(temperature, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
	 }
      }
   }

   return;
}
