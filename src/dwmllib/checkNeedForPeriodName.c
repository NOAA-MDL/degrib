/*****************************************************************************
 * checkNeedForPeriodName() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code checks to see if a user has requested data with start and end
 *  times that warrent a special period name (TODAY, TONIGHT, etc.). If so, the
 *  period name is determined.
 *
 * ARGUMENTS
 *            index = Index of the startTimes and endTimes arrays. (Input)
 *   numPeriodNames = Number period names for one of the seven issuance times.
 *                    (Input)
 *   timeZoneOffset = Hours to add to local time to get to UTC time. (Input)
 *    parameterName = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *   parsedDataTime = String representation of the data's current startTime 
 *                    being analyzed (ex. 2005-03-30T00:00:00-05:00). (Input)
 * outputPeriodName = A logical (1 = true, 0 = false) used to indicate if a 
 *                    period name is to be used for this row once user selected
 *                    times are taken into account. (Input)
 *     issuanceType = Index (dimension #1) into the period name array that 
 *                    defines the current cycle (ex. morning, afternoon, 
 *                    earlyMorningMinT, etc). (Input)
 *       periodName = Special name used near beginning of time layouts. (Output)
 *       currentDay = Current day's 2 digit date. Used to determine if Max and 
 *                    Min temps should be called "today"/"overnight" or POP 
 *                    value should  be called "overnight". (Input)
 *    startTime_cml = Incoming argument set by user as a double in seconds 
 *                    since 1970 denoting the starting time data was retrieved
 *                    for from NDFD. (Input)
 *  currentDoubTime = Current time in double form. (Input)
 *   firstValidTime = First valid time of element. (Input)
 *      currentHour = Today's hour. Used to determine if POP value should be 
 *                    called "overnight" or "later today". (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *  2/2008 Paul Hershberg (MDL): Removed "period" as incoming argument to 
 *                               routine.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void checkNeedForPeriodName(int index, uChar * numPeriodNames,
                            sChar timeZoneOffset, uChar parameterName,
                            char *parsedDataTime, uChar * outputPeriodName, 
                            uChar issuanceType, char *periodName, 
                            char *currentHour, char *currentDay, 
                            double startTime_cml, double currentDoubTime,
                            double firstValidTime)
{
   static char TDay[3];       /* String holding 2-dig date. */
   uChar whichPeriodName = 3; /* An index (dimension #2 of periodData array)
                               * indicating which period name is to be used
                               * for the row indicated by the index. */
   char *periodData[MAX_PERIODS][4];  /* Array containing the period names
                                       * for each of the different cycles
                                       * (afternoon and morning) and period
                                       * lengths (12 and 24 hours). */
   double numPeriodsClippedBegin = 0.0; /* Number of forecast periods skipped
                                         * due to a user shortening the time data
                                         * was retrieved for. */
   double numHoursClippedBegin = 0.0;/* Number of hours skipped due to a user
                                      * shortening the time data was retrieved 
                                      * for. */
   double startTime_doub = 0.0; 

   /* Initialize periodData pointer. */
   periodData[0][0] = "Overnight";
   periodData[0][1] = "Later Today";
   periodData[0][2] = NULL;
   periodData[0][3] = NULL;

   periodData[1][0] = "Today";
   periodData[1][1] = "Tonight";
   periodData[1][2] = "Tomorrow";
   periodData[1][3] = "Tomorrow Night";

   periodData[2][0] = "Tonight";
   periodData[2][1] = "Tomorrow";
   periodData[2][2] = "Tomorrow Night";
   periodData[2][3] = NULL;

   periodData[3][0] = "Later Today";
   periodData[3][1] = NULL;
   periodData[3][2] = NULL;
   periodData[3][3] = NULL;

   periodData[4][0] = "Overnight";
   periodData[4][1] = NULL;
   periodData[4][2] = NULL;
   periodData[4][3] = NULL;

   periodData[5][0] = "Today";
   periodData[5][1] = "Tomorrow";
   periodData[5][2] = NULL;
   periodData[5][3] = NULL;

   periodData[6][0] = "Tonight";
   periodData[6][1] = "Tomorrow Night";
   periodData[6][2] = NULL;
   periodData[6][3] = NULL;

   if (index == 0)
   {
      TDay[0] = parsedDataTime[8];
      TDay[1] = parsedDataTime[9];
      TDay[2] = '\0';
   }

   /* Calculate how many periods were skipped at beginning. */
   Clock_Scan(&startTime_doub, parsedDataTime, 1);
   if (parameterName == NDFD_POP)
      numHoursClippedBegin = ((startTime_doub + (0.5*12*3600)) - currentDoubTime) / 3600;
   else
      numHoursClippedBegin = (startTime_doub - currentDoubTime) / 3600;

   /* Now we have to check and see if user supplied a shorter duration for
    * data formatting via the command line arguments startTime and endTime.
    * Then we need to see if period names are still applicable in this
    * shorter interval of time. 
    */
   if ((startTime_cml != 0.0) && numHoursClippedBegin >= 12)
   {
      /* We use "12" instead of the period name because MaxT and MinT have
       * periods of 24 hours, but forecast periods of 12 hours for the 12hr
       * summarization product.
       */
      if (parameterName == NDFD_POP)
      {
         if (numHoursClippedBegin >= 12)
            numPeriodsClippedBegin = floor(numHoursClippedBegin / 12.0);
         else
            numPeriodsClippedBegin = ceil(numHoursClippedBegin / 12.0);
      }
      else
         numPeriodsClippedBegin = myRound((numHoursClippedBegin / 24.0), 0);
 
      if ((int)myRound(numPeriodsClippedBegin, 0) > *numPeriodNames || index+1 > *numPeriodNames)
      {
         *outputPeriodName = 0;
         return;
      }
      else
      {
         *outputPeriodName = 1; /* Tell user they need to use a period name. */
         whichPeriodName = (int)myRound(numPeriodsClippedBegin, 0);

         /* Now that we have issuanceType, and whichPeriodName, retrieve the
          * periodName. 
          */
         if (periodData[issuanceType][whichPeriodName] != NULL)
            strcpy(periodName, periodData[issuanceType][whichPeriodName]);
         else
            *outputPeriodName = 0;

         return;
      }
   }
   else
   {
      /* Late in the day, the Max Temp value will be in the tomorrow period
       * so we need to detect that condition and return outputPeriodName =
       * false. */
      if (parameterName == NDFD_MAX)
      {
         /* If the max temp day is not the same as the today's day, then we  
          * don't need to label it using "today".  This happens in the
          * evening after about 8:00 PM. */
         if (strcmp(currentDay, TDay) != 0 && index + 1 <= *numPeriodNames)
         {
            *outputPeriodName = 1;  /* Tell user they need to use a period
                                     * name. */
            whichPeriodName = index + 1;
         }
         else if (strcmp(currentDay, TDay) == 0 && index < *numPeriodNames)
         {
            *outputPeriodName = 1;  /* Tell user they need to use a period
                                     * name. */
            whichPeriodName = index;
         }
         else if (strcmp(currentDay, TDay) != 0 && index >= *numPeriodNames)
         {
            *outputPeriodName = 0;  /* Tell user they don't use a period
                                     * name. */
            return;
         }
      }

      /* The Min Temp value will be in the tomorrow period so we need to
       * detect that condition and return outputPeriodName = false. 
       */
      else if (parameterName == NDFD_MIN)
      {

         /* If the min temp day is not the same as today's day, then we don't 
          * need to label it using "today".  This happens in the evening
          * after about 8:00 PM. 
	  */
         if (issuanceType == earlyMorningMinT && index == 0)
         {
            *outputPeriodName = 1;  /* Tell user they need to use a period
                                     * name. */
            whichPeriodName = index;
         }
         if (issuanceType == afternoon24)
         {
            if (strcmp(currentDay, TDay) == 0 && index < *numPeriodNames)
            {
               *outputPeriodName = 1; /* Tell user they need to use period
                                       * name. */
               whichPeriodName = index;
            }
            else if (strcmp(currentDay, TDay) == 0 && index != 0 && index + 1 <
                     *numPeriodNames)
            {
               *outputPeriodName = 1; /* Tell user they need to use period
                                       * name. */
               whichPeriodName = index + 1;
            }
            else if (strcmp(currentDay, TDay) != 0 && index < *numPeriodNames)
            {
               *outputPeriodName = 1; /* Tell user they need to use period
                                       * name. */
               whichPeriodName = index + 1;
            }
            else if (strcmp(currentDay, TDay) != 0 && index >= *numPeriodNames)
            {
               *outputPeriodName = 0; /* Tell user they don't need to use a
                                       * period name. */
               return;
            }
         }
      }

      /* Early in the morning, the PoP12 data will be updated and no longer
       * be the OVERNIGHT period.  So we need to detect that condition and
       * return the next period name (LATER TODAY). This will only be
       * applicable for DWMLgenByDay products as the "glance" product of
       * DWMLgen does not output POP. 
       */
      else if (parameterName == NDFD_POP)
      {
         /* If the POP day is not the same as the today's day, then we don't
          * need to label it using "today".  This happens in the evening
          * after about 8:00 PM. 
          */
         if (strcmp(currentDay, TDay) == 0 && atoi(currentHour)
             <= 20 && index < *numPeriodNames)
         {
            *outputPeriodName = 1;  /* Tell user they need to use a period
                                     * name. */
            whichPeriodName = index;
         }
         else if (strcmp(currentDay, TDay) == 0 && atoi(currentHour) <= 20 &&
                  index >= *numPeriodNames)
         {
            *outputPeriodName = 0;  /* Tell user they don't need to use a
                                     * period name. */
            return;
         }
      }
   }

   /* Now that we have issuanceType, and whichPeriodName, retrieve the
    * periodName. 
    */
   if (*outputPeriodName != 0)
   {
       if (periodData[issuanceType][whichPeriodName] == NULL)
       {
          *outputPeriodName = 0;
          return;
       }
       else
          strcpy(periodName, periodData[issuanceType][whichPeriodName]);
   }

   return;
}
