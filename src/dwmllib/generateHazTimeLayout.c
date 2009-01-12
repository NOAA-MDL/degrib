/*****************************************************************************
 * generateHazTimeLayout () -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This routine creates the XML time layout for the NDFD Hazard parameter, 
 *  when it is called via a summary product. In this case, each individual
 *  point's hazards are treated as a separate element:
 *
 *   <time-layout time-coordinate="local" summarization="12hourly">
 *     <layout-key>k-p6h-n2-6</layout-key>
 *     <start-valid-time>2008-10-02T13:00:00-04:00</start-valid-time>
 *     <end-valid-time>2008-10-02T18:00:00-04:00</end-valid-time>
 *     <start-valid-time>2008-10-02T18:00:00-04:00</start-valid-time>
 *     <end-valid-time>2008-10-02T19:00:00-04:00</end-valid-time>
 *   </time-layout>
 *
 * ARGUMENTS
 *          numRows = Structure containing members: (Input)
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
 *        layoutKey = The key to the time layout is of the form
 *                    k-p{periodLength}h-n{numRows}-{numLayouts}    
 *                    The "k" is for "key".  The "p" is for "period" "h" is for
 *                    "hour" and "n" is for "number" (example:  k-p12h-n6-1).  
 *                    The key is used to relate the layout in the <time-layout>  
 *                    element to the time-layout attribute in the NDFD element 
 *                    element <temperature key-layout="k-p12h-n6-1"> 
 *                    (Input / Output) 
 *   timeCoordinate = The time coordinate that the user wants the time 
 *                    information communicated it. Currently only local time is 
 *                    supported. (Input)
 *    consecHazRows = Array holding information about each individual hazard.
 *                    Each element contains hazard info members containing 
 *                    startTime, the endTime, the number of consecutive hours the 
 *                    hazard exists, the time of a resolution split (1hr res 
 *                    changes to 3hr resolution after 3rd day) and the string code.
 *                    (Output).
 *    summarization = The type of temporal summarization being used.
 *                    Currently, no summarization is done in time.
 * f_formatPeriodName = Flag to indicate if period names (i.e. "today") appear 
 *                      in the start valid time tag: 
 *                      <start-valid-time period-name="today"> (Input)
 *   numLayoutSoFar = The total number of time layouts that have been created 
 *                    so far. (Input)
 * numCurrentLayout = Number of the layout we are currently processing. (Input)
 *         TZoffset = Number of hours to add to current time to get GMT time. 
 *                    (Input)
 *     f_observeDST = Flag determining if current point observes Daylight 
 *                    Savings Time. (Input)  
 *       currentDay = Current day's 2 digit date. (Input)
 *  currentDoubTime = Current time in double form in sec since 1970. (Input)
 *      currentHour = Current hour =in 2 digit form. (Input)
 *        frequency = Set to "boggus" for DWMLgen products, and to "12 hourly" 
 *                    or "24 hourly" for the DWMLgenByDay products. (Input)  
 *            match = Pointer to the array of element matches from degrib. 
 *                    (Input) 
 *         numMatch = The number of matches from degrib. (Input)
 *             data = An xml Node Pointer denoting the <data> node. (Input)
 *    startTime_cml = Incoming argument set by user as a double in seconds 
 *                    since 1970 denoting the starting time data was retrieved
 *                    for from NDFD. (Input)
 *      numFmtdRows = For DWMLgenByDay products, the number of rows to format 
 *                    per each hazard. (Input/Output)
 *            f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's NDFD "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product. (Input)
 *                     5 = DWMLgen's RTMA "time-series" product. 
 *                     6 = DWMLgen's RTMA & NDFD "time-series" product. 
 *         startNum = First index in match structure an individual point's data 
 *                    matches can be found. (Input)
 *           endNum = Last index in match structure an individual point's data
 *                    matches can be found. (Input)
 *      periodTimes = The times bordering each forecast period time. (Output)
 *   numPeriodTimes = The number of periodTimes. (Output)
 *   
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  9/2008 Paul Hershberg (MDL): Created.
 * 11/2008 Paul Hershberg (MDL): Fixed time layout where period was not 
 *                               numTotalHours.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void generateHazTimeLayout(char *layoutKey, const char *timeCoordinate, 
                           hazInfo consecHazRows, char *summarization, 
                           genMatchType *match, size_t numMatch, 
                           double TZoffset, sChar f_observeDST, 
                           size_t **numLayoutSoFar,  uChar **numCurrentLayout, 
                           char *currentHour, char *currentDay, 
                           char *frequency, xmlNodePtr data, 
                           double startTime_cml, double currentDoubTime, 
                           int *numFmtdRows, uChar f_XML, int startNum, 
                           int endNum, double **periodTimes, 
                           int *numPeriodTimes)
{
   int i;                     /* Counter thru the number of start/end times we 
                               * need to format per hazard, depending on how 
                               * many forecast periods the hazard spans. */
   int f_finalTimeLayout = 0; /* Flag denoting if this is the last time
                               * layout being processed. */
   int period = 1;            /* Length between an elements successive
                               * validTimes. */
   double firstValidTime = 0.0; /* The validTime of the first match for the
                                 * element being processed. */
   char **startTimes = NULL;  /* Character array holding the start Times for 
                               * each hazard. */
   char **endTimes = NULL;    /* Character array holding the end Times for each 
                               * hazard. */
   xmlNodePtr time_layout = NULL; /* An xml Node Pointer denoting the
                                   * <time-layout> node. */
   xmlNodePtr layout_key = NULL;  /* An xml Node Pointer denoting the
                                   * <layout-key> node. */
   xmlNodePtr startValTime = NULL;  /* An xml Node Pointer denoting the
                                     * <start-valid-time> node. */
   xmlNodePtr endValTime = NULL;  /* An xml Node Pointer denoting the
                                   * <end-valid-time> node. */
   layouts currentTimeLayout; /* Structure containing the current element's
                               * period, first startTime, and numRows. Used
                               * in determining if a new layout is needed. */
   int numTotalHazHours = 1; /* Total number of consecutive hours hazard exists 
                              * for. Used in layout key as "period". */
   
   /* Set first validTime of this particular hazard, for this point. */
   if (consecHazRows.startHour > consecHazRows.valTimeResSplit && 
       consecHazRows.valTimeResSplit > 0)
      period = 6;
   else
      period = 1;
   firstValidTime =  consecHazRows.startHour - (((double)period) * 3600);

   /* Start filling in the time layout array's with this current data. */
   formatValidTime(firstValidTime, currentTimeLayout.fmtdStartTime, 30, 
		   TZoffset, f_observeDST);

   /* Fill the rest of the time layout array with current data. */
   currentTimeLayout.period = period; /* Set period length to 1 hour. */
   
   /* We need to get the actual number of rows of a hazard's startTimes 
    * and endTimes. The number of rows will be determined by how many 
    * forecast periods the entire accumulated hazard spans. 
    */
   hazStartEndTimes(&startTimes, &endTimes, &numFmtdRows, consecHazRows, 
                    periodTimes, numPeriodTimes, TZoffset, f_observeDST); 

   /* Fill more of the time layout array with current data. */
   currentTimeLayout.numRows = *numFmtdRows;

   /* Determine total number of hours hazards exist for (regardless of 
    * number of periods it spans)
    */
   numTotalHazHours = (int)myRound(((consecHazRows.endHour - 
                                     consecHazRows.startHour) / 3600), 0);

   /* Add one hour since we are dealing with duration. */
   numTotalHazHours++;

   /* Fill the rest of the time layout array with current data. Set period 
    * length to number of consecutive hours hazard exists for to 
    * differentiate it between other hazards. 
    */
   currentTimeLayout.period = numTotalHazHours;

   /* Determine if this layout information has already been formatted. */
   if (isNewLayout(currentTimeLayout, *numLayoutSoFar, *numCurrentLayout,
                   f_finalTimeLayout) == 1)
   {
      /* Create the new key and then bump up the number of layouts by one. */
      sprintf(layoutKey, "k-p%dh-n%d-%d", numTotalHazHours, *numFmtdRows,
              **numLayoutSoFar);
      **numLayoutSoFar += 1;
      
      /* Format the XML time layout in the output string. */
      time_layout = xmlNewChild(data, NULL, BAD_CAST "time-layout", NULL);
      xmlNewProp(time_layout, BAD_CAST "time-coordinate", BAD_CAST
                 timeCoordinate);
      xmlNewProp(time_layout, BAD_CAST "summarization", BAD_CAST summarization);
      layout_key = xmlNewChild(time_layout, NULL, BAD_CAST "layout-key",
                               BAD_CAST layoutKey);

      /* Now we get the time values for this parameter and format the valid time
       * tags. 
       */
      for (i = 0; i < *numFmtdRows; i++)
      {
         if (startTimes[i] != NULL)
         {
            startValTime = xmlNewChild(time_layout, NULL, BAD_CAST
                                       "start-valid-time", BAD_CAST
                                       startTimes[i]);

            xmlNewChild(time_layout, NULL, BAD_CAST "end-valid-time",
                        BAD_CAST endTimes[i]);
         }
         else /* No times. */
         {
            startValTime = xmlNewChild(time_layout, NULL, BAD_CAST
                                       "start-valid-time", BAD_CAST NULL);
            xmlNewProp(startValTime, BAD_CAST "xsi:nil", BAD_CAST "true");
            endValTime = xmlNewChild(time_layout, NULL, BAD_CAST
                                        "end-valid-time", BAD_CAST NULL);
            xmlNewProp(endValTime, BAD_CAST "xsi:nil", BAD_CAST "true");
         }
      }	       
   }
   else /* Not a new key so just return the key name */
   {
      /* Create the new key and then bump up the number of layouts by one. */
      sprintf(layoutKey, "k-p%dh-n%d-%d", numTotalHazHours, *numFmtdRows,
              **numCurrentLayout);
   }

   /* Free some things. */
   for (i = 0; i < *numFmtdRows; i++)
   {
      free(startTimes[i]);
      free(endTimes[i]);
   }

   free(startTimes);
   free(endTimes);
 
   return;
}
