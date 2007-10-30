/******************************************************************************
 * getUserTimes() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Routine finds start of the User time interval the summarization is done for, 
 *   modified to fit the first day's forecast period. If there is a startTime
 *   on a day in the future other than the current day, we use the 06th hour. If 
 *   there is no startTime entered, the start of the next closest forecast period
 *   is chosen (06th or 18th hr). Routine then finds the end of the user time 
 *   interval the summarization is done for, modified to fit the last day's 
 *   forecast period (18th hr). Routine is only accessed if product type is one of 
 *   the summarizations (f_XML = 3 or f_XML = 4).
 *    
 * ARGUMENTS
 *     timeUserStart = The beginning of the first forecast period (06 hr
 *                     or 18hr) based on the startDate command line argument. 
 *                     (Output)
 *       timeUserEnd = The end of the last forecast period (18 hr) based 
 *                     on the startDate & numDays arguments. (Output)
 *    f_POPUserStart = Flag used to denote if the first forecast period occurs
 *                     on the next day than current day. Only used if 24 hr 
 *                     summarization (f_XML = 4). (Output)
 *           numDays = The number of days the validTimes for all the data rows 
 *                     (values) consist of. (Input)
 *         startDate = Point specific user supplied Date that the startTime 
 *                     falls in. It is the form (i.e. 2006-04-15). (Input) 
 *             f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product. (Input)
 *         startTime = Incoming argument set by user as a double in seconds 
 *                     since 1970 denoting the starting time data was retrieved
 *                     for from NDFD. (Input)
 * firstValidTimeMatch = The very first validTime for all matches returned 
 *                       from the gird probe for current point. (Input) 
 *  firstValidTime_pop = The very first validTime for POP12hr returned from the
 *                       grid probe for current point. (Input) 
 *                TZ = # of hours to add to current time to get GMT time.
 *                     (Input) 
 *      f_observeDST = Flag determining if current point observes Daylight 
 *                     Savings Time. (Input)  
 *               
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   10/2006 Paul Hershberg (MDL): Created
 *   10/2007 Paul Hershberg (MDL): Removed code that shifted data back by 1/2
 *                                 the period length (bug from php code)
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getUserTimes(double **timeUserStart, double **timeUserEnd, 
                  int *f_POPUserStart, char *startDate, sChar TZ, 
                  double startTime, sChar f_observeDST, int *numDays, 
                  double *firstValidTime_pop, sChar f_XML,
                  double *firstValidTimeMatch)
{
   sChar DST; /* Temporary storage for time zone offset if daylight savings is
                 effect. */
   char hourMinSecTZ[16]; /* String component holding "T06:00:00-00:00" part of 
                             time string. */
   int oneDay = (24 * 60 * 60); /* # seconds in 24 hours. */
   int realEndOfDay = 0; /* The end of the first day's forecast period (18 hr)
			    based on the the startTime argument. */
   char userStart_year[6]; /* Year of startDate, which is based off of incoming
			      argument startTime. */
   char userStart_month[4]; /* Month of startDate, which is based off of incoming
			       argument startTime. */
   char userStart_day[3]; /* Day of startDate, which is based off of incoming
			     argument startTime. */
   char firstMatch_year[6]; /* Year based off of the very first match in the 
                               match structure. */
   char firstMatch_month[4]; /* Month based off of the very first match in the
                                match structure. */
   char firstMatch_day[3]; /* Day based off of the very first match in the match
                              structure. */
   char firstPOP_year[6]; /* Year based off of the first POP12hr match in the 
                             match structure. */
   char firstPOP_month[4]; /* Month based off of the first POP12hr match in the 
                              match structure. */
   char firstPOP_day[3]; /* Day based off of the first POP12hr match in the 
                            match structure. */
   char POPstr[30];       /* Returned character string holding first valid 
			     time for POP12hr. */
   char POPstrLess1Day[30]; /* Returned character string holding the time that
                               is one day prior to date of first valid POP12hr
                               time. Used when there is no startTime entered, 
                               the summarization is 12 hourly, and the next
                               closest forecast cycle (summaraization period) is
                               18th hour. */
   char firstMatchStr[30]; /* Returned character string holding first valid 
			      match for any element in the match structure. 
                              Used  */
   char startDateAddDayStr[30]; /* Returned character string holding a valid
                                   time with startDate of very first match 
                                   + 1 day. Used if there is no startTime 
                                   entered as an argument and summarization is
                                   24 hours. */
   char temp[3];            /* Temporary string buffer. */
   int beginningHour;       /* Beginning hour of a validTime processed. */ 
   char base06UserTime[30]; /* Year, Month, and Day part of a 06hr time 
                               string. */
   char base18UserTime[30]; /* Year, Month, and Day part of a 18hr time 
                               string. */
   char firstMatchStartDate[30]; /* Year, Month, and Day of first match in match
                                  * structure. */
   char firstPOPStartDate[30]; /* Year, Month, and Day of first POP12hr match in 
                                * match structure. */
   double startUserTime_doub = 0.0; /* Double time (what we're after in this
                                       routine) representing the start of the 
                                       next summarization period (forecast 
                                       cycle).  */
   double endUserTime_doub = 0.0; /* Double time (what we're after in this
                                     routine) representing the end of the 
                                     last summarization period forecast 
                                     cycle).*/
   char startDateBuff[30];    /* Temporary string. */

   /* For DWMLgenByDay products, parse user supplied start time, if supplied. */

   /* Choose the default reference time to be the 06th hour on the current date
    * denoted by current startDate. We will alter this if necessary.
    */     
   userStart_year[0] = startDate[0];
   userStart_year[1] = startDate[1];
   userStart_year[2] = startDate[2];
   userStart_year[3] = startDate[3];
   userStart_year[4] = '-';
   userStart_year[5] = '\0';

   userStart_month[0] = startDate[5];
   userStart_month[1] = startDate[6];
   userStart_month[2] = '-';
   userStart_month[3] = '\0';

   userStart_day[0] = startDate[8];
   userStart_day[1] = startDate[9];
   userStart_day[2] = '\0';
      
   strcpy(base06UserTime, userStart_year);
   strcat(base06UserTime, userStart_month);
   strcat(base06UserTime, userStart_day);

   if (TZ < 0)
      sprintf(hourMinSecTZ, "T06:00:00+%02d:00", -1 * TZ);
   else
      sprintf(hourMinSecTZ, "T06:00:00-%02d:00", TZ); 
     
   strcat(base06UserTime, hourMinSecTZ);          
   Clock_Scan(&startUserTime_doub, base06UserTime, 0);
     
   /* Before continuing, see if this point observes day light savings time, 
    * and if it is currently in effect. 
    */ 
   if (f_observeDST)
   {
      if (Clock_IsDaylightSaving2(startUserTime_doub, 0) == 1)
      {
         DST = TZ - 1;
        if (DST < 0)
            sprintf(hourMinSecTZ, "T06:00:00+%02d:00", -1 * DST);
        else
            sprintf(hourMinSecTZ, "T06:00:00-%02d:00", DST);
   
         strcat(base06UserTime, hourMinSecTZ);
         Clock_Scan(&startUserTime_doub, base06UserTime, 1);
      }            	 
   }

   /* Get the 18th hour on this date in order to find the endTime. */
   strcpy(base18UserTime, userStart_year);
   strcat(base18UserTime, userStart_month);
   strcat(base18UserTime, userStart_day);

   if (TZ < 0)
      sprintf(hourMinSecTZ, "T18:00:00+%02d:00", -1 * TZ);
   else
      sprintf(hourMinSecTZ, "T18:00:00-%02d:00", TZ); 
     
   strcat(base18UserTime, hourMinSecTZ);
   Clock_Scan(&endUserTime_doub, base18UserTime, 0);
     
   /* Before continuing, see if this point observes day light savings time, 
    * and if it is currently in effect. 
    */ 
   if (f_observeDST)
   {
      if (Clock_IsDaylightSaving2(endUserTime_doub, 0) == 1)
      {
         DST = TZ - 1;
         if (DST < 0)
            sprintf(hourMinSecTZ, "T18:00:00+%02d:00", -1 * DST);
         else
            sprintf(hourMinSecTZ, "T18:00:00-%02d:00", DST);
   
         strcat(base18UserTime, hourMinSecTZ);
         Clock_Scan(&endUserTime_doub, base18UserTime, 1);
      }            	 
   }
  
   /* Now determine the next forecast period (06 or 18 hr) using the first
    * valid Time for the POP12hr element. Only do this if there was no startTime
    * given on the command line argument. If there is a startTime, always begin
    * the next closest forecast period to begin on the 06th hour of the date
    * denoted on startDate. Also, if the summarization is 24 hourly, the base 
    * time will be the 06th hour of the next 24 hourly forecast.
    */
   formatValidTime(*firstValidTime_pop, POPstr, 30, TZ, f_observeDST);
   temp[0] = POPstr[11];
   temp[1] = POPstr[12];
   temp[2] = '\0';
   beginningHour = atoi(temp) - 12;

   /* This statement checks if the next summarization period begins on the
    * 18th hour. This can only occur if the summarization is 12 hourly and 
    * there is no startTime (startDate) entered.
    */
   if (beginningHour < 0 && f_XML == 3)
   {
      formatValidTime(*firstValidTime_pop - oneDay, POPstrLess1Day, 30, TZ, f_observeDST); 
      userStart_year[0] = POPstrLess1Day[0];
      userStart_year[1] = POPstrLess1Day[1];
      userStart_year[2] = POPstrLess1Day[2];
      userStart_year[3] = POPstrLess1Day[3];
      userStart_year[4] = '-';
      userStart_year[5] = '\0';

      userStart_month[0] = POPstrLess1Day[5];
      userStart_month[1] = POPstrLess1Day[6];
      userStart_month[2] = '-';
      userStart_month[3] = '\0';

      userStart_day[0] = POPstrLess1Day[8];
      userStart_day[1] = POPstrLess1Day[9];
      userStart_day[2] = '\0';
      
      strcpy(base18UserTime, userStart_year);
      strcat(base18UserTime, userStart_month);
      strcat(base18UserTime, userStart_day);
       
      if (TZ < 0)
         sprintf(hourMinSecTZ, "T18:00:00+%02d:00", -1 * TZ);
      else
         sprintf(hourMinSecTZ, "T18:00:00-%02d:00", TZ); 
      
      strcat(base18UserTime, hourMinSecTZ);
      Clock_Scan(&startUserTime_doub, base18UserTime, 0);
      
      /* Before continuing, see if this point observes day light savings time, 
       * and if it is currently in effect. 
       */ 
      if (f_observeDST)
      {
         if (Clock_IsDaylightSaving2(startUserTime_doub, 0) == 1)
         {
            DST = TZ - 1;
            if (DST < 0)
	       sprintf(hourMinSecTZ, "T18:00:00+%02d:00", -1 * DST);
	    else
               sprintf(hourMinSecTZ, "T18:00:00-%02d:00", DST);

            strcat(base18UserTime, hourMinSecTZ);
            Clock_Scan(&startUserTime_doub, base18UserTime, 1);
	 }            	 
      }
   }
   
   /* Check to see if first summarization for this 24 hourly case begins on the
    * current day, or begins on the next day. 
    */
   else if (startTime == 0.0 && f_XML == 4)
   {
      formatValidTime(*firstValidTimeMatch, firstMatchStr, 30, TZ, f_observeDST); 
      
      firstMatch_year[0] = firstMatchStr[0];
      firstMatch_year[1] = firstMatchStr[1];
      firstMatch_year[2] = firstMatchStr[2];
      firstMatch_year[3] = firstMatchStr[3];
      firstMatch_year[4] = '-';
      firstMatch_year[5] = '\0';

      firstMatch_month[0] = firstMatchStr[5];
      firstMatch_month[1] = firstMatchStr[6];
      firstMatch_month[2] = '-';
      firstMatch_month[3] = '\0';

      firstMatch_day[0] = firstMatchStr[8];
      firstMatch_day[1] = firstMatchStr[9];
      firstMatch_day[2] = '\0';
      
      strcpy(firstMatchStartDate, firstMatch_year);
      strcat(firstMatchStartDate, firstMatch_month);
      strcat(firstMatchStartDate, firstMatch_day);

      formatValidTime(*firstValidTime_pop, POPstr, 30, TZ, f_observeDST); 
      
      firstPOP_year[0] = POPstr[0];
      firstPOP_year[1] = POPstr[1];
      firstPOP_year[2] = POPstr[2];
      firstPOP_year[3] = POPstr[3];
      firstPOP_year[4] = '-';
      firstPOP_year[5] = '\0';

      firstPOP_month[0] = POPstr[5];
      firstPOP_month[1] = POPstr[6];
      firstPOP_month[2] = '-';
      firstPOP_month[3] = '\0';

      firstPOP_day[0] = POPstr[8];
      firstPOP_day[1] = POPstr[9]; 
      firstPOP_day[2] = '\0';
      
      strcpy(firstPOPStartDate, firstPOP_year);
      strcat(firstPOPStartDate, firstPOP_month);
      strcat(firstPOPStartDate, firstPOP_day);

      /* See if the startDates are different between the very first match in
       * match structure and the first POP12hr match. If so, the first 24 hr
       * forecast period will begin on the next day (from current day).
       * Use f_POPUserStart flag to denote this.
       */
      if (strcmp(firstPOPStartDate, firstMatchStartDate) != 0)
      {
         *f_POPUserStart = 1;
         formatValidTime(*firstValidTimeMatch + oneDay, startDateAddDayStr, 30, TZ, f_observeDST); 

         userStart_year[0] = startDateAddDayStr[0];
         userStart_year[1] = startDateAddDayStr[1];
         userStart_year[2] = startDateAddDayStr[2];
         userStart_year[3] = startDateAddDayStr[3];
         userStart_year[4] = '-';
         userStart_year[5] = '\0';

         userStart_month[0] = startDateAddDayStr[5];
         userStart_month[1] = startDateAddDayStr[6];
         userStart_month[2] = '-';
         userStart_month[3] = '\0';

         userStart_day[0] = startDateAddDayStr[8];
         userStart_day[1] = startDateAddDayStr[9];
         userStart_day[2] = '\0';
      
         strcpy(base06UserTime, userStart_year);
         strcat(base06UserTime, userStart_month);
         strcat(base06UserTime, userStart_day);

         if (TZ < 0)
            sprintf(hourMinSecTZ, "T06:00:00+%02d:00", -1 * TZ);
         else
            sprintf(hourMinSecTZ, "T06:00:00-%02d:00", TZ); 
     
         strcat(base06UserTime, hourMinSecTZ);          
         Clock_Scan(&startUserTime_doub, base06UserTime, 0);
     
         /* Before continuing, see if this point observes day light savings time, 
          * and if it is currently in effect. 
          */ 
         if (f_observeDST)
         {
            if (Clock_IsDaylightSaving2(startUserTime_doub, 0) == 1)
            {
               DST = TZ - 1;
               if (DST < 0)
                  sprintf(hourMinSecTZ, "T06:00:00+%02d:00", -1 * DST);
	       else
                  sprintf(hourMinSecTZ, "T06:00:00-%02d:00", DST);
   
               strcat(base06UserTime, hourMinSecTZ);
               Clock_Scan(&startUserTime_doub, base06UserTime, 1);
	    }            	 
         }

         /* Get the 18th hour on this date in order to find the endTime. */
         strcpy(base18UserTime, userStart_year);
         strcat(base18UserTime, userStart_month);
         strcat(base18UserTime, userStart_day);

         if (TZ < 0)
            sprintf(hourMinSecTZ, "T18:00:00+%02d:00", -1 * TZ);
         else
            sprintf(hourMinSecTZ, "T18:00:00-%02d:00", TZ); 
     
         strcat(base18UserTime, hourMinSecTZ);          
         Clock_Scan(&endUserTime_doub, base18UserTime, 0);
     
         /* Before continuing, see if this point observes day light savings time, 
          * and if it is currently in effect. 
          */ 
         if (f_observeDST)
         {
            if (Clock_IsDaylightSaving2(endUserTime_doub, 0) == 1)
            {
               DST = TZ - 1;
               if (DST < 0)
                  sprintf(hourMinSecTZ, "T18:00:00+%02d:00", -1 * DST);
	       else
                  sprintf(hourMinSecTZ, "T18:00:00-%02d:00", DST);
   
               strcat(base18UserTime, hourMinSecTZ);
               Clock_Scan(&endUserTime_doub, base18UserTime, 1);
	    }            	 
         }
      }
   }
 
   formatValidTime (startUserTime_doub, startDateBuff, 30,
                    TZ, f_observeDST);
   formatValidTime (endUserTime_doub, startDateBuff, 30,
                    TZ, f_observeDST);

   /* Assign Beginning Time. */ 
   **timeUserStart = startUserTime_doub;

   /* Now for End Time. */
   realEndOfDay = endUserTime_doub;
   **timeUserEnd = realEndOfDay + (24 * 3600 * (*numDays));
   
   return;
}
