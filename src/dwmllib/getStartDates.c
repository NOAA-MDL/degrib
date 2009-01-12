/******************************************************************************
 * getStartDates() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Routine finds the char startDate (in form "2006-14-29") by using the given
 *   double startTime (if entered as a command line argument) or the time of the
 *   first valid Data if startTime was not entered (startTime set to 0.0).
 *   
 * ARGUMENTS
 *         startDate = Point specific user supplied start Date that the 
 *                     startTime falls in (first Valid Match time if startTime
 *                     was not entered). It is in the form (i.e. 2006-04-15).
 *                     (Output) 
 *             f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product.
 *                     5 = DWMLgen's RTMA "time-series" product. 
 *                     6 = DWMLgen's RTMA + NDFD "time-series" product. (Input)
 *         startTime = Incoming argument set by user as a double in seconds 
 *                     since 1970 denoting the start time data is to be
 *                     retrieved for (set to 0.0 if not supplied). (Input)
 * firstValidTimeMatch = The very first validTime for all matches returned 
 *                       from the grid probe for this point. (Input) 
 * firstValidTime_maxt = Valid time of first MaxT returned from the grid probe
 *                       for this point. (Input) 
 *            TZoffset = # of hours to add to current time to get GMT time.
 *                       (Input) 
 *        f_observeDST = Flag determining if current point observes Daylight 
 *                       Savings Time. (Input)  
 *               point = Current point being processed. (Input)
 *               
 * FILES/DATABASES: None
 *
 * RETURNS: void (char ** startDate is variable altered)
 *
 * HISTORY
 *   10/2006  Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getStartDates(char **startDate, uChar f_XML, double startTime, 
		   double firstValidTimeMatch, double firstValidTime_maxt,
                   sChar TZoffset, sChar f_observeDST, size_t point)
{
   char *pstr;                /* Temporary pointer character string. */
   char startDateBuff[30];    /* Returned temporary string. */
   char startDateHr[3];       /* Hour of the first match in the match 
				 structure. */

   /* If product is of type DWMLgenByDay. */  
   if (f_XML == 3 || f_XML == 4)
   {
      /* If the startTime argument was not entered as command line argument. */ 
      if (startTime == 0.0)
      {
         formatValidTime(firstValidTimeMatch, startDateBuff, 30,
                         TZoffset, f_observeDST);
	    
         startDateHr[0] = startDateBuff[11];
         startDateHr[1] = startDateBuff[12];
         startDateHr[2] = '\0';

         pstr = strchr(startDateBuff, 'T');
         startDate[point] = (char *)calloc((pstr - startDateBuff) + 1,
                                            sizeof(char));
         strncpy(startDate[point], startDateBuff, pstr - startDateBuff);
      }
      else if (startTime != 0.0)
      {
         /* If startTime was entered as an argument... since startTime was 
	  * already altered before going into the grid probe, simply use it.
	  * Note, simply send in zero's for the TZoffset and f_observeDST
	  * variables, as we are strictly dealing with GMT time.  
	  */ 
         formatValidTime(startTime, startDateBuff, 30, 0, 0);
	 
         pstr = strchr(startDateBuff, 'T');
         startDate[point] = (char *)calloc((pstr - startDateBuff) + 1,
                                            sizeof(char));
         strncpy(startDate[point], startDateBuff, pstr - startDateBuff);
      }
   }
   else
   {

      /* If the product's of type DWMLgen, simply set startDate to = NULL. */
      startDate[point] = (char *)calloc(1, sizeof(char));
      startDate[point][0] = '\0';
   }
   
   return;
}
