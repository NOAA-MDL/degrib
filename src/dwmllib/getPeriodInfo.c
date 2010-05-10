/*****************************************************************************
 * getPeriodInfo() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Retreives the "issuanceType" and "numPeriodNames" for those elements in 
 *  which period names are used. 
 *
 * ARGUMENTS
 *   firstValidTime = First valid time of element. (Input)
 *   numPeriodNames = Number period names for one of the seven issuance times.
 *                    (Input)
 *    parameterEnum = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *     issuanceType = Index (dimension #1) into the period name array that 
 *                    defines the current cycle (ex. morning, afternoon, 
 *                    earlyMorningMinT, etc). (Input)
 *           period = Length between an elements successive validTimes (Input).
 *        frequency = Set to "boggus" for DWMLgen products, and to "12 hourly" 
 *                    or "24 hourly" for the DWMLgenByDay products.  
 *       currentDay = Current day's 2 digit date. (Input)
 *      currentHour = Current 2-digit hour. (Input)
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
void getPeriodInfo(uChar parameterEnum, char *firstValidTime, char *currentHour, 
                   char *currentDay, uChar * issuanceType, 
                   uChar * numPeriodNames, int period, char *frequency)
{

   /* The period names vary by cycle, so we need to define when each cycle
    * begins 
    */
   static int startAfternoon = 12;  /* Threshold indicating the beginning of
                                     * the evening cycle. */
   static int startMorning = 6; /* Threshold indicating the beginning of
                                 * morning cycle. */
   char firstValidHour[3];    /* Hour of element's first ValidTime. */
   char firstValidDay[3];     /* Day of element's first ValidTime. */

   /* Now we determine when the data begins. */
   firstValidHour[0] = firstValidTime[11];
   firstValidHour[1] = firstValidTime[12];
   firstValidHour[2] = '\0';

   firstValidDay[0] = firstValidTime[8];
   firstValidDay[1] = firstValidTime[9];
   firstValidDay[2] = '\0';

   /* Determine if we are to use morning (ex. TODAY) or afternoon (ex. TONIGHT) 
    * period names and set the index into the period name array accordingly. 
    */
   if (period == 12)
   {
      if (atoi(firstValidHour) >= startMorning && atoi(firstValidHour) <
          startAfternoon)
         *issuanceType = morning12;
      else
         *issuanceType = afternoon12;
   }
   else if (period == 24)
   {
      if (atoi(firstValidHour) >= startMorning && atoi(firstValidHour) <
          startAfternoon)
         *issuanceType = morning24;
      else
         *issuanceType = afternoon24;
   }

   /* In the wee hours of the morning, we need special names for the periods. 
    * So we first determine which interface is calling us and then use the 6 AM 
    * cutoff for their use. For the DWMLgen() minT, we need to keep 
    * using the early morning period names until the overnight data falls off.
    * So we check which day the data is valid for (firstValidDay < todayDay).
    * This section if for the DWMLgenByDay product only. 
    */
   if (((strcmp(frequency, "12 hourly") == 0) || (strcmp(frequency, "24 hourly") == 0)) &&
       (((atoi(currentHour) < startMorning && strcmp(firstValidHour, "06") != 0) || (atoi(currentHour) < startMorning+3 && strcmp(firstValidDay, currentDay) != 0))))
   {

      /* Determine which NDFD parameter we are processing and return TRUE if
       * it is valid for a period of time. 
       */
      switch (parameterEnum)
      {

         case NDFD_MAX:

            *issuanceType = earlyMorningMaxT;
            break;

         case NDFD_MIN:

            *issuanceType = earlyMorningMinT;
            break;

         case NDFD_POP:

            *issuanceType = earlyMorning;
            break;
      }
   }
   else if (strcmp(frequency, "boggus") == 0)
   {

      /* Determine which NDFD parameter we are processing and return TRUE if
       * it is valid for a period of time. This section is for the DWMLgen
       * product only.  
       */
      switch (parameterEnum)
      {

         case NDFD_MAX:

            if (atoi(currentHour) < startMorning)
               *issuanceType = earlyMorningMaxT;
            break;

         case NDFD_MIN:

            if (strcmp(firstValidDay, currentDay) != 0)
               *issuanceType = earlyMorningMinT;
            break;
      }
   }

   /* Determine the Number of Period Names. */
   if (period == 24)
   {
      if (*issuanceType == earlyMorningMaxT || *issuanceType ==
          earlyMorningMinT)
         *numPeriodNames = 1;
      else if (*issuanceType == afternoon24)
         *numPeriodNames = 2;
      else if ((*issuanceType == morning24) && (strcmp(firstValidDay, currentDay) != 0))
         *numPeriodNames = 1;
      else
         *numPeriodNames = 2;
   }
   if (period == 12)
   {
      if (*issuanceType == earlyMorning)
         *numPeriodNames = 2;
      else if (*issuanceType == morning12)
         *numPeriodNames = 4;
      else if (*issuanceType == afternoon12)
         *numPeriodNames = 3;
   }

   return;
}
