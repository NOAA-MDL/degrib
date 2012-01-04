/******************************************************************************
 * genMinTempValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code formats the Min Temperature element in the DWMLgen and
 *  DWMLgenByDay products.
 *
 * ARGUMENTS
 *          pnt = Current Point index. (Input)
 *    layoutKey = The key linking the Min Temps to their valid times 
 *                (ex. k-p24h-n7-1). (Input)
 *        match = Pointer to the array of element matches from degrib. (Input) 
 *   parameters = An xml Node Pointer denoting the <parameters> node to which 
 *                these values will be attached (child node). (Output)
 *        f_XML = Flag denoting type of XML product (1 = DWMLgen's "time-series"
 *                product, 2 = DWMLgen's "glance" product, 3 = DWMLgenByDay's 
 *                "12 hourly" product, 4 = DWMLgenByDay's "24 hourly" product, 
 *                5 = DWMLgen's "RTMA time-series" product, 6 = DWMLgen's mix 
 *                of "RTMA & NDFD time-series" product.
 *                (Input) 
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
 *                           (Input)
 *   currentDay = Current day's 2 digit date. (Input)
 *  currentHour = Current hour = in 2 digit form. (Input)
 *     TZoffset = Number of hours to add to current time to get GMT time. 
 *                (Input)
 * f_observeDST = Flag determining if current point observes Daylight Savings 
 *                Time. (Input)  
 *  numFmtdRows = Number of output lines formatted in DWMLgenByDay products. 
 *                (Input)  
 * startTime_cml = The startTime entered as an option on the command line. 
 *                 (Input)
 *     startNum = First index in match structure an individual point's data 
 *                matches can be found. (Input)
 *       endNum = Last index in match structure an individual point's data
 *                matches can be found. (Input)
 *       f_unit = 0 (GRIB unit), 1 (english), 2 (metric) (Input)
 *                
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
void genMinTempValues(size_t pnt, char *layoutKey, genMatchType *match,
                      xmlNodePtr parameters, uChar f_XML, double startTime_cml,
                      numRowsInfo numRows, char *currentDay, char *currentHour, 
                      sChar TZoffset, sChar f_observeDST, int numFmtdRows, 
                      int startNum, int endNum, sChar f_unit)
{
   int i;                     /* Element counter thru match structure. */
   int numNils = 0;           /* Denotes diff between number of data rows and 
				 the number that need to be formatted for the 
				 DWMLgenByDay products. */
   int counter = 0;           /* Initialize counter to move thru the min temp 
                               * values. */
   int MinTCounter = 0;       /* Flag denoting first MinT data row. */
   int roundedMinData;        /* Returned rounded data. */
   int priorElemCount = 0;    /* Used to subtract prior elements when looping 
                               * thru matches. */
   xmlNodePtr temperature = NULL; /* Xml Node Pointer for <temperature>
                                   * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   char str1[30];             /* Temporary string holding formatted time
                               * value. */
   char MinTDay[3];           /* Date (day) of the first Min Temp Data value. 
                               */
   char startTimeDay[3];      /* Date (day) of the startTime_cml argument, in 
				 local time. */

   /* Format the <temperature> element. */
   temperature = xmlNewChild(parameters, NULL, BAD_CAST "temperature", NULL);
   xmlNewProp(temperature, BAD_CAST "type", BAD_CAST "minimum");

   if (f_unit != 2)
      xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Fahrenheit");
   else
      xmlNewProp(temperature, BAD_CAST "units", BAD_CAST "Celsius");

   xmlNewProp(temperature, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(temperature, NULL, BAD_CAST "name", BAD_CAST
               "Daily Minimum Temperature");
   
   /* If DWMLgen product, set numFmtdRows = to numRows since there is no set
    * number of rows we are ultimately formatting. IF DWMLgenByDay product, 
    * check to see if the number of data rows in match structure is less than 
    * the number of rows to be formatted. If so, set numRows = numFmtdRows 
    * (will result in a "nil" being formatted).
    */
   if (f_XML == 1 || f_XML == 2 || f_XML == 6)
      numFmtdRows = numRows.total-numRows.skipBeg-numRows.skipEnd;

   /* Loop over all the data values and format them. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_MIN && 
	  match[i].validTime >= numRows.firstUserTime &&
	  match[i].validTime <= numRows.lastUserTime)
      {
         if (i-priorElemCount-startNum < numFmtdRows) /* For DWMLgenByDay. */
         {
            if (f_XML == 3 || f_XML == 4)  /* DWMLgenByDay products. */
            {
		 
               /* Early in the morning (midnight - 6AM) the code will want to
                * include the overnight min temp in the 6AM to 6PM time
                * period. If it is not, the following code causes the program to
                * correctly skip the first min temp (by adjusting "counter". Only 
		* applicable for DWMLgenByDay's "24 hourly" product. */
               if (f_XML == 4 && atoi(currentHour) <= 7 && MinTCounter == 0)
               {
                  /* Get the end time of the first min temp's valid time (i.e. 07
                   * or 19). "counter" here firstly == 0. */
                  formatValidTime(match[i + counter].validTime, str1, 30, TZoffset,
                                  f_observeDST);
   
                  MinTDay[0] = str1[8];
                  MinTDay[1] = str1[9];
                  MinTDay[2] = '\0';
                 
		  if (startTime_cml == 0.0)
		  {
		     if (atoi(MinTDay) == atoi(currentDay))
                        counter = 1;
		  }
		  else
                  {
                     formatValidTime(startTime_cml, str1, 30, TZoffset, 
				     f_observeDST);
                     startTimeDay[0] = str1[8];
                     startTimeDay[1] = str1[9];
                     startTimeDay[2] = '\0';
		     
		     if (atoi(MinTDay) == atoi(startTimeDay))
		        counter = 1;
		  }

                  MinTCounter++;
               }

               if (i-priorElemCount-startNum < 
                  numRows.total-numRows.skipBeg-numRows.skipEnd)
               {
                  /* If the data is missing, so indicate in the XML (nil=true).
		   * Also, add a check to make sure the counter does not cause
		   * a bleed over and pick up the next element in the match
		   * structure. Also, put some common sense checks on the data.
                   */ 
                  if ((match[i + counter].value[pnt].valueType == 2) || 
		      (match[i + counter].elem.ndfdEnum != NDFD_MIN) ||
                      (match[i + counter].value[pnt].data > 300) || 
                      (match[i + counter].value[pnt].data < -300) || 
                      (match[i].value[pnt].data == '\0'))
                  {
                     value = xmlNewChild(temperature, NULL, BAD_CAST "value",
                                         NULL);
                     xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
                  }
                  else if (match[i + counter].value[pnt].valueType == 0)
                  {
                     roundedMinData =
                           (int)myRound(match[i + counter].value[pnt].data, 0);
                     sprintf(strBuff, "%d", roundedMinData);
                     xmlNewChild(temperature, NULL, BAD_CAST "value", BAD_CAST
                                 strBuff);
                  }
               }
            } 
            /* DWMLgen products: */
            else if (f_XML == 1 || f_XML == 2 || f_XML == 6)
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
                  roundedMinData = (int)myRound(match[i].value[pnt].data, 0);
                  sprintf(strBuff, "%d", roundedMinData);
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
       * structure and compare to the number of actual data rows to see if there
       * is a difference.
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
