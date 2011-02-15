/*****************************************************************************
 * generateNoHazTimeLayout () -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Dummy up a time layout when there are no active hazards. We'll set 
 *  xsi:nil="true" for one period denoting the entire forecast period 
 *  duration the user chose. Only applicable when user chooses a summary 
 *  product. 
 *
 *   <time-layout time-coordinate="local" summarization="12hourly">
 *     <layout-key>k-p7d-n1-6</layout-key>
 *     <start-valid-time>2008-10-02T06:00:00-04:00</start-valid-time>
 *     <end-valid-time>2008-10-09T18:00:00-04:00</end-valid-time>
 *   </time-layout>
 *
 * ARGUMENTS
 *         tempBuff = Time layout from POP, MaxT, or Mint, we use to dummy up 
 *                    the layout when there are no active hazards. (Input)
 *        layoutKey = The key to the time layout is of the form
 *                    k-p{periodLength}h-n{numRows}-{numLayouts}    
 *                    The "k" is for "key".  The "p" is for "period" "h" is for
 *                    "hour" and "n" is for "number" (example:  k-p12h-n6-1).  
 *                    The key is used to relate the layout in the <time-layout>  
 *                    element to the time-layout attribute in the NDFD element 
 *                    element <temperature key-layout="k-p12h-n6-1"> 
 *                    (Output) 
 *   timeCoordinate = The time coordinate that the user wants the time 
 *                    information communicated it. Currently only local time is 
 *                    supported. (Input)
 *    summarization = The type of temporal summarization being used.
 *                    Currently, no summarization is done in time. (Input)
 *   numLayoutSoFar = The total number of time layouts that have been created 
 *                    so far. (Input/Output)
 * numCurrentLayout = Number of the layout we are currently processing. 
 *                    (Input/Output))
 *         TZoffset = Number of hours to add to current time to get GMT time. 
 *                    (Input)
 *     f_observeDST = Flag determining if current point observes Daylight 
 *                    Savings Time. (Input)  
 *             data = An xml Node Pointer denoting the <data> node. (Input)
 *  firstPeriodStartTime = The time of the beginning of the first forecast 
 *                         period. (Input)   
 *          numDays = Number of days user requested data for. (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  9/2008 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void generateNoHazTimeLayout(char *tempBuff, char *layoutKey, 
                             const char *timeCoordinate, char *summarization,
                             double TZoffset, sChar f_observeDST,
                             size_t *numLayoutSoFar, uChar *numCurrentLayout,
                             double firstPeriodStartTime, int numDays, 
                             xmlNodePtr data)
{
   char tempBuff2[30]; /* Temp string used when f_XML =3 or 4 and there are no 
                        * active hazards. */
   char *pstr = NULL; /* Temp Pointer used when f_XML =3 or 4 and there are no
                        * active hazards. */
   int forecastDuration = 0;  /* Used to denote entire forecast duration. Used 
                               * when f_XML =3 or 4 and there are no active 
                               * hazards. Can be in days or hours. */
   int hours = 0; /* Used to denote entire forecast duration, in hours. Used 
                   * when f_XML = 3 or 4 and there are no active hazards. */
   int f_finalTimeLayout = 0; /* Flag denoting if this is the last time
                               * layout being processed. */
   int f_setToDays = 0; /* Flag denoting whether to denote entire forecast 
                         * period in days instead of hours. */
   double lastPeriodEndTime; /* Ending time of last forecast period. */
   xmlNodePtr time_layout = NULL; /* An xml Node Pointer denoting the
                                   * <time-layout> node. */
   xmlNodePtr layout_key = NULL;  /* An xml Node Pointer denoting the
                                   * <layout-key> node. */
   layouts currentTimeLayout; /* Structure containing the current element's
                               * period, first startTime, and numRows. Used
                               * in determining if a new layout is needed. */

   /* Find the duration of the entire forecast period. Use a preexisting layout
    * to find this. 
    */
   strcpy(tempBuff2, tempBuff);
   pstr = strstr(tempBuff, "p");
   pstr  = strtok(pstr, "h");    
   pstr = pstr+1;
   hours = atoi(pstr);
   pstr = strstr(tempBuff2, "n");
   pstr  = strtok(pstr, "-");    
   pstr = pstr+1;
   forecastDuration = (hours) * atoi(pstr);

   if (forecastDuration >= 24) /* Set  period to days. */
   {
      f_setToDays = 1;
      forecastDuration = (int)(myRound((forecastDuration/24), 0));
   }

   /* Get the startTime and endTime of the entire duration. Start by filling in 
    * the time layout array's with this current data. 
    */
   formatValidTime(firstPeriodStartTime, currentTimeLayout.fmtdStartTime, 30, 
		   TZoffset, f_observeDST);

   /* Fill the rest of the time layout array with current data. */
   currentTimeLayout.period = forecastDuration;
   
   /* Fill more of the time layout array with current data. */
   currentTimeLayout.numRows = 1;

   /* Determine if this layout information has already been formatted. */
   if (isNewLayout(currentTimeLayout, numLayoutSoFar, numCurrentLayout,
                   f_finalTimeLayout) == 1) 
   {
      /* Create the new key and then bump up the number of layouts by one. */
      if (f_setToDays) /* Set to days. */
         sprintf(layoutKey, "k-p%dd-n%s-%d", forecastDuration, "1", 
                 *numLayoutSoFar);
      else /* Leave in hours. */
         sprintf(layoutKey, "k-p%dh-n%s-%d", forecastDuration, "1", 
                 *numLayoutSoFar);

      *numLayoutSoFar += 1;

      /* Format the XML time layout in the output string. */
      time_layout = xmlNewChild(data, NULL, BAD_CAST "time-layout", NULL);
      xmlNewProp(time_layout, BAD_CAST "time-coordinate", BAD_CAST
                 timeCoordinate);
      xmlNewProp(time_layout, BAD_CAST "summarization", BAD_CAST summarization);
      layout_key = xmlNewChild(time_layout, NULL, BAD_CAST "layout-key",
                               BAD_CAST layoutKey);

      /* Now we get the time values for this parameter and format the valid time
       * tags. Since we're dumming it up, We only have one start and one end 
       * time.
       */
      formatValidTime(firstPeriodStartTime, tempBuff2, 30, TZoffset,
                      f_observeDST);
      xmlNewChild(time_layout, NULL, BAD_CAST "start-valid-time", BAD_CAST 
                  tempBuff2);
      lastPeriodEndTime = firstPeriodStartTime + (24 * 3600 * numDays);

      formatValidTime(lastPeriodEndTime, tempBuff2, 30, TZoffset,
                      f_observeDST);
      xmlNewChild(time_layout, NULL, BAD_CAST "end-valid-time", BAD_CAST 
                  tempBuff2);
   }
   else /* Not a new key so just return the key name */
   {
      /* Create the new key and then bump up the number of layouts by one. */
      if (f_setToDays) /* Set to days. */
         sprintf(layoutKey, "k-p%dd-n%s-%d", forecastDuration, "1", 
                 *numCurrentLayout);
      else /* Leave in hours. */
         sprintf(layoutKey, "k-p%dh-n%s-%d", forecastDuration, "1", 
                 *numCurrentLayout);
   }

   return;
}
