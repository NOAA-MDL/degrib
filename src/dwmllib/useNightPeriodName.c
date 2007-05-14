/*****************************************************************************
 * useNightPeriodName() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Determines whether it is night time -- and to subsequently use night time
 *  period names.
 *  
 * ARGUMENTS
 *   dataTime = Current element's validTime. (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
int useNightPeriodName(char *dataTime)
{
   char hour[3];              /* 2 digit hour of current validTime. */
   int hr;                    /* 2 digit hour of current validTime in integer 
                               * form. */

   /* Lets parse the validTime information (2004-03-19T12:00:00-05:00). */
   hour[0] = dataTime[11];
   hour[1] = dataTime[12];
   hour[2] = '\0';

   hr = atoi(hour);

   /* Determine if the current hour is in the day */
   if ((hr >= 6) && (hr < 18))
      return 0;
   else /* it needs a nighttime label */
      return 1;
}
