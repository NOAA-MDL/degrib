/*****************************************************************************
 * formatValidTime() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Transform double validTime to character string in form 
 *  (2006-04-13T:00:00:00-05:00), which is the standard form in the formatted
 *  XML.
 *   
 * ARGUMENTS
 *        validTime = Incoming double validTime (elements endTime) to be 
 *                    converted. (Input)
 *         timeBuff = Returned time counterpart in character form
 *                    (2006-04-13T:00:00:00-05:00). (Output)
 *    size_timeBuff = Max size of "timeBuff". (Input)
 *    pntZoneOffset = Number of hours to add to current time to get GMT time. 
 *                    (Input)
 *       f_dayCheck = Flag determining if current point observes Daylight 
 *                    Savings Time. (Input) 
 *        
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
int formatValidTime(double validTime, char *timeBuff, int size_timeBuff, 
                    sChar pntZoneOffSet, sChar f_dayCheck)
{
   char zone[7]; /* Time Zone. */
   double localTime; /* validTime with the time zone difference taken into 
                      * account. */

   localTime = validTime - (pntZoneOffSet * 3600);

   /* localTime is now in point's local standard time */
   if (f_dayCheck)
   {
      /* Note: a zero is passed to DaylightSavings so it converts from local
       * to local standard time. 
       */
      if (Clock_IsDaylightSaving2(localTime, 0) == 1)
      {
         localTime += 3600;
         pntZoneOffSet--;
      }
   }

   /* Sort by valid time. */
   myAssert(size_timeBuff > 25);
   if (size_timeBuff <= 25)
      return -1;

   /* The '0, 0' is passed in because we already converted to local standard
    * time. 
    */
   Clock_Print2(timeBuff, size_timeBuff, localTime, "%Y-%m-%dT%H:%M:%S", 0, 0);

   /* Change definition of pntZoneOffSet */
   pntZoneOffSet = -1 * pntZoneOffSet;
   if (pntZoneOffSet < 0)
      sprintf(zone, "-%02d:00", -1 * pntZoneOffSet);
   else
      sprintf(zone, "+%02d:00", pntZoneOffSet);
   strcat(timeBuff, zone);
   return 0;
}
