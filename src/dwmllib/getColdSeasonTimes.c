/*****************************************************************************
 * getColdSeasonTimes () -- 
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  Determine the start and end times for the cold season in double form. Used 
 *  to decide if winds are blustery (cold season) or breezy (warm season).
 *
 * ARGUMENTS
 *             match = Pointer to the array of element matches from degrib. 
 *         numRowsWS = The number of data rows for Wind Speed.  (Input)
 *          TZoffset = Number of hours to add to current time to get GMT time. 
 *                     (Input)
 *  springDoubleDate = Double; end time of next cold season. (Output)
 *    fallDoubleDate = Double; start time of next cold season. (Output)
 *          startNum = First index in match structure an individual point's data
 *                     matches can be found. (Input)
 *            endNum = Last index in match structure an individual point's data
 *                     matches can be found. (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *  3/2007 Paul Hershberg (MDL): Added startNum and endNum arguments.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void getColdSeasonTimes(genMatchType *match, numRowsInfo numRowsWS,
                        sChar TZoffset, double **springDoubleDate, 
			double **fallDoubleDate, int startNum, int endNum)
{
   int i; /* Match structure counter. */
   int springYear_int; /* Integer year in which next cold season ends. */
   int fallYear_int; /* Integer year in which next cold season begins. */   
   char str1[30]; /* String holding formatted valid time. */
   char year[5]; /* String holding formatted year. */
   char fallYear[5]; /* String with year in which next cold season begins. */
   char springYear[5]; /* String with year in which next cold season ends. */
   char springDate[30]; /* Complete string of end time of next cold season. */
   char fallDate[30]; /* Complete string of start time of next cold season. */
   char month[4]; /* String holding formatted month. */
   char time_adj[16]; /* String component holding "T00:00:00-00:00" part. */
   
   /* Based on the current month, we need to know how to calculate the start
    * and end of the cold season (i.e. October 2004 - April 2005). Get this 
    * info using the Wind Speed validTimes.
    */ 	
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WS && 
	  match[i].validTime >= numRowsWS.firstUserTime &&
          match[i].validTime <= numRowsWS.lastUserTime)
      {
         formatValidTime(match[i].validTime, str1, 30, 0, 0);         
         break; 
      }
   }
   
   /* Get the year and the month from the returned string. */
   year[0] = str1[0];
   year[1] = str1[1];
   year[2] = str1[2];
   year[3] = str1[3];
   year[4] = '\0';

   month[0] = str1[5];
   month[1] = str1[6];
   month[2] = '-';
   month[3] ='\0';
   
   /* Craft the hour, minute, second, and Time zone offset. */
   if (TZoffset < 0)
      sprintf (time_adj, "T01:01:01+%02d:00", -1 * TZoffset);
   else
      sprintf (time_adj, "T01:01:01-%02d:00", TZoffset);

   /* Set year of beginning and end of cold season, depending on current
    * month. 
    */
   if (atoi(month) > 4)
   { 
      strcpy (fallYear, year);
      springYear_int = atoi(year); 
      springYear_int++;
      sprintf (springYear, "%d", springYear_int);
   }
   else
   {
      fallYear_int = atoi(year);
      fallYear_int--;
      sprintf (fallYear, "%d", fallYear_int);
      strcpy (springYear, year);
   }

   /* Now get the double time for the start and end of the cold season. To do
    * this, build the string up, and then send into Clock_Scan() to retrieve 
    * the time in double form. 
    */
   sprintf (springDate, "%s-04-01%s", springYear, time_adj);
   sprintf (fallDate, "%s-10-01%s", fallYear, time_adj);

   /* And in double form... */
   Clock_Scan(*springDoubleDate, springDate, 0);
   Clock_Scan(*fallDoubleDate, fallDate, 0);

   return;
}
