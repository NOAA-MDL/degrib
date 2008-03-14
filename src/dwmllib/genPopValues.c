/******************************************************************************
 * genPopValues() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code performs two functions: 
 *            1) Formats the Pop12hr element in the "time-series" DWMLgen 
 *               products (f_XML = 1 or 6). 
 *            2) Collects the Max Pop values per summarization (12 or 24 hr 
 *               period) for icon determination in the DWMLgenByDay products
 *               (f_XML = 3 or f_XML =4).
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *   layoutKey = The key linking the 12 Hour Pops to their valid times 
 *               (ex. k-p12h-n42-3). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
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
 *       f_XML = Flag denoting type of XML product (1 = DWMLgen's "time-series"
 *               product, 2 = DWMLgen's "glance" product, 3 = DWMLgenByDay's 
 *               "12 hourly" product, 4 = DWMLgenByDay's "24 hourly" product, 
 *               5 = DWMLgen's "RTMA time-series" product, 6 = DWMLgen's mix of
 *               "RTMA & NDFD time-series" product.
 *               (Input) 
 * maxDailyPop = Array containing the max pop values corresponding to a day (24 
 *               hour summarization) or 12 hour period (12 hour summarization). 
 *               For 24 hour format, we use the maximum of the two 12 hour pops 
 *		 that span the day. Array's info is used to test if the pop is
 *		 large enough to justify formatting weather values. (Input)
 *    numDays = The number of days the validTimes for all the data rows 
 *              (values) consist of. (Input)
 * currentDoubTime = Current (system) time in double form. (Input)
 *  currentHour = The current hour, based off system time, in 2 digit form. 
 *                (Input)
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
 *   3/2006 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void genPopValues(size_t pnt, char *layoutKey, genMatchType *match,
                  xmlNodePtr parameters, numRowsInfo numRows, uChar f_XML,
		  double startTime_cml, int *maxDailyPop, int *numDays, 
                  double currentDoubTime, char *currentHour, int startNum, 
                  int endNum)
{
   int i;                     /* Counter thru match structure. */
   int numNils = 0;           /* Denotes diff between number of data rows and 
				 the number that need to be formatted for the 
				 DWMLgenByDay products. */
   int numFmtdRows = 0;       /* Number of output lines in DWMLgenByDay products. */
   int dayCount = 0;          /* Used to keep count of which summarization
                               * period being processing (two PoPs per day). */
   int startOverCount = 0;    /* Used to denote if we should increment to the 
                                 next summarization period. */
   int roundedPopData = 0;    /* Returned rounded data. */
   xmlNodePtr precipitation = NULL; /* Xml Node Pointer for <precipitation>
                                     * element. */
   xmlNodePtr value = NULL;   /* Xml Node Pointer for <value> element. */
   char strBuff[30];          /* Temporary string buffer holding rounded
                               * data. */
   int priorElemCount = 0;    /* Used to subtract prior elements when looping 
                               * thru matches. */

   /* Format the <precipitation> element. */
   precipitation = xmlNewChild(parameters, NULL, BAD_CAST
                               "probability-of-precipitation", NULL);
   xmlNewProp(precipitation, BAD_CAST "type", BAD_CAST "12 hour");
   xmlNewProp(precipitation, BAD_CAST "units", BAD_CAST "percent");
   xmlNewProp(precipitation, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(precipitation, NULL, BAD_CAST "name", BAD_CAST
               "12 Hourly Probability of Precipitation");
   
   /* If DWMLgen product, set numFmtdRows = to numRows because we don't have 
    * a set number of rows we are ultimately formatting.
    */
   if (f_XML == 1 || f_XML == 2 || f_XML == 6)
      numFmtdRows = numRows.total-numRows.skipBeg-numRows.skipEnd;
   else if (f_XML == 3 || f_XML == 4) 
      numFmtdRows = (*numDays)*2;

   /* Loop over all the data values and format them. */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_POP && 
	  match[i].validTime >= numRows.firstUserTime &&
          match[i].validTime <= numRows.lastUserTime)
      {  
         /* Accounts for DWMLgenByDay. */
         if ((i - priorElemCount) < (numFmtdRows + startNum)) 
         {
	    if (f_XML == 3 || f_XML == 4) /* DWMLgenByDay products. */
            {
               startOverCount++;

               /* If the data is missing, so indicate in the XML (nil=true). */
               if (match[i].value[pnt].valueType == 2)
               {
                  value = xmlNewChild(precipitation, NULL, BAD_CAST "value",
                                      NULL);
                  xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
               }
               else if (match[i].value[pnt].valueType == 0) /* Good data. */
               {
                  roundedPopData = (int)myRound(match[i].value[pnt].data, 0);
                  sprintf(strBuff, "%d", roundedPopData);
                  xmlNewChild(precipitation, NULL, BAD_CAST "value", BAD_CAST
                              strBuff);
               }

               /* Make some adjustments for the next loop interation. */
               if (f_XML == 4 && match[i].value[pnt].valueType == 0)
               {
                  /* For 24hr summarization, get the max PoP out of the two 
                   * 12hr pops to represent the summarization period. 
		   */
                  if (roundedPopData > maxDailyPop[dayCount] && dayCount <= *numDays)
	          {
                     maxDailyPop[dayCount] = roundedPopData;
                  }
                  /* We change to a new period every other PoP value. */
                  if (startOverCount % 2 == 0)
                     dayCount++;
               }
               else if (f_XML == 3 && match[i].value[pnt].valueType == 0)
               {
                  /* For 12hr summarization, we simply use every PoP value. */
                  if (dayCount <= (*numDays)*2)
                  {
                     maxDailyPop[dayCount] = roundedPopData;
                     dayCount++;
	          }
               }
            }   
            else if (f_XML == 1 || f_XML == 2 || f_XML == 6)
            {
               /* If the data is missing, so indicate in the XML (nil=true). */
               if (match[i].value[pnt].valueType == 2)
               {
                  value = xmlNewChild(precipitation, NULL, BAD_CAST "value", NULL);
                  xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
               }
               else if (match[i].value[pnt].valueType == 0)  /* Format good
                                                              * data. */
               {
                  roundedPopData = (int)myRound(match[i].value[pnt].data, 0);
                  sprintf(strBuff, "%d", roundedPopData);
                  xmlNewChild(precipitation, NULL, BAD_CAST "value", BAD_CAST
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
            value = xmlNewChild(precipitation, NULL, BAD_CAST "value", NULL);
            xmlNewProp(value, BAD_CAST "xsi:nil", BAD_CAST "true");
	 }
      }
   }
   
   return;
}
