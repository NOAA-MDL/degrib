/*****************************************************************************
 * monthDayYearTime() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Code gets the current time and the first validTime for the MaxT element in 
 *  double form. However, it sets the hours, minutes, and seconds all to zeroes
 *  before the conversion. For example, if the current local time is 
 *  2006-04-30T19:34:22:-05:00, before converting to double form, set the 
 *  current Local time to 2006-04-30T00:00:00:-00:00. Do the same for the first
 *  validTime for the MaxT. This is done for a "same day" check.
 *
 * ARGUMENTS
 *            match = Pointer to the array of element matches from degrib. 
 *                    (Input) 
 *         numMatch = The number of matches from degrib. (Input)
 *         TZoffset = Number of hours to add to current time to get GMT time. 
 * currentLocalTime =  Current Local Time in "2006-04-29T19:34:22-05:00"         
 *                     format. (Input)
 *       currentDay = Current day's 2-digit date. (Input)
 * firstMaxTValidTime_doub_adj = The date of the first MaxT validTime with the 
 *                               hour, minutes, and seconds set to zeroes, 
 *                               translated to double form. 
 *   currentLocalTime_doub_adj = The current date with the hours, minutes, and 
 *                               seconds set to zeroes, translated to double
 *                               form.
 *                    startNum = First index in match structure an individual 
 *                               point's data matches can be found. (Input)
 *                      endNum = Last index in match structure an individual 
 *                               point's data matches can be found. (Input)
 *                     numRows = Structure containing members: (Input)
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
 *                             
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void monthDayYearTime(genMatchType * match, size_t numMatch,
                      char *currentLocalTime, char *currentDay,
                      sChar f_observeDST, double *firstMaxTValidTime_doub_adj,
                      double *currentLocalTime_doub_adj, sChar TZoffset,
		      int startNum, int endNum, numRowsInfo numRows)
{
   int i;
   static char firstMaxTValidTime_char[30]; /* First MaxT's validTime. */
   char maxTYear[6];          /* Year of first MaxT's validTime ("2006-"). */
   char maxTMonth[4];         /* Month of first MaxT's validTime ("04-"). */
   char maxTDay[3];           /* Day of first MaxT's validTime ("30"). */
   char firstMaxTValidTime_char_adj[30];  /* First MaxT's validTime, with
                                           * hours, minutes, and seconds all
                                           * set to zeros. */
   char time_adj[16]; /* String component holding "T00:00:00-00:00" part. */
   char currentYear[6];       /* Year of current Local Time ("2006-"). */
   char currentMonth[4];      /* Month of current Local Time ("04-"). */
   char currentLocalTime_char_adj[30];  /* Current Local Time with hours,
                                         * minutes, and seconds all set to
                                         * zeros. */

   /* Get the first MaxT valid time in character form and set everything but
    * the year, month, and day to = 0. 
    */
   for (i = startNum; i < endNum; i++)
   {  

      /* Set first MaxT validtime. */
      if (match[i].elem.ndfdEnum == NDFD_MAX && 
	  match[i].validTime >= numRows.firstUserTime &&
	  match[i].validTime <= numRows.lastUserTime) 
      {
         formatValidTime(match[i].validTime, firstMaxTValidTime_char, 30, 
			 TZoffset, f_observeDST);
         break;
      }
   }

   /* Build the first MaxT's validTime, adjusted with hours, minutes, and
    * seconds all set to zeroes. 
    */
   maxTYear[0] = firstMaxTValidTime_char[0];
   maxTYear[1] = firstMaxTValidTime_char[1];
   maxTYear[2] = firstMaxTValidTime_char[2];
   maxTYear[3] = firstMaxTValidTime_char[3];
   maxTYear[4] = '-';
   maxTYear[5] = '\0';

   maxTMonth[0] = firstMaxTValidTime_char[5];
   maxTMonth[1] = firstMaxTValidTime_char[6];
   maxTMonth[2] = '-';
   maxTMonth[3] = '\0';

   maxTDay[0] = firstMaxTValidTime_char[8];
   maxTDay[1] = firstMaxTValidTime_char[9];
   maxTDay[2] = '\0';

   strcpy(firstMaxTValidTime_char_adj, maxTYear);
   strcat(firstMaxTValidTime_char_adj, maxTMonth);
   strcat(firstMaxTValidTime_char_adj, maxTDay);

   if (TZoffset < 0)
      sprintf(time_adj, "T00:00:00+%02d:00", -1 * TZoffset);
   else
      sprintf(time_adj, "T00:00:00-%02d:00", TZoffset);
   
   strcat(firstMaxTValidTime_char_adj, time_adj);

   /* Get the adjusted MaxT validTime in double form. */
   Clock_Scan(firstMaxTValidTime_doub_adj, firstMaxTValidTime_char_adj, 1);

   /* Build the current Local time, adjusted with hours, minutes, and seconds 
    * all set to zeroes. 
    */
   currentYear[0] = currentLocalTime[0];
   currentYear[1] = currentLocalTime[1];
   currentYear[2] = currentLocalTime[2];
   currentYear[3] = currentLocalTime[3];
   currentYear[4] = '-';
   currentYear[5] = '\0';

   currentMonth[0] = currentLocalTime[5];
   currentMonth[1] = currentLocalTime[6];
   currentMonth[2] = '-';
   currentMonth[3] = '\0';

   strcpy(currentLocalTime_char_adj, currentYear);
   strcat(currentLocalTime_char_adj, currentMonth);
   strcat(currentLocalTime_char_adj, currentDay);
   
   if (TZoffset < 0)
      sprintf(time_adj, "T00:00:00+%02d:00", -1 * TZoffset);
   else
      sprintf(time_adj, "T00:00:00-%02d:00", TZoffset);
   
   strcat(currentLocalTime_char_adj, time_adj);

   /* Get the adjusted current LocalTime in double form. */
   Clock_Scan(currentLocalTime_doub_adj, currentLocalTime_char_adj, 1);

   return;
}
