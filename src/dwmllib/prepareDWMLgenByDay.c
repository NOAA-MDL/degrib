/*****************************************************************************
 * prepareDWMLgenByDay() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Flags those elements that are ultimately formatted in the output XML for 
 *  the DWMLgenByDay products. There are pre-defined elements formatted for the 
 *  "12 hourly and "24 hourly" products. Routine also sets the flag determining 
 *  if period names are used in the time layout generation. Routine also 
 *  calculates numOutputLines, the timeInterval of each period, and sets the 
 *  summarization and format character strings.
 *
 * ARGUMENTS
 *          numDays = The number of days the validTimes for all the data rows 
 *                    (values) consist of. Used for the DWMLgenByDay products only.
 *                    (Input)
 *            f_XML = Flag denoting type of XML product (1 = DWMLgen's 
 *                    "time-series"product, 2 = DWMLgen's "glance" product, 3 
 *                    = DWMLgenByDay's "12 hourly" product, 4 = DWMLgenByDay's 
 *                    "24 hourly" product. (Input) 
 *     wxParameters = Array containing the flags denoting whether a certain 
 *                    element is formatted in the output XML (= 1), or used in 
 *                    deriving another formatted element (= 2). (Input/Output) 
 *    summarization = The type of temporal summarization being used. 
 *                    (Input/Output)
 * f_formatPeriodName = Flag to indicate if period names (i.e. "today") appear 
 *                      in the start valid time tag: 
 *                      <start-valid-time period-name="today"> (Input)
 *            match = Pointer to the array of element matches from degrib. 
 *                    (Input) 
 *         numMatch = The number of matches from degrib. (Input)
 *     timeInterval = Number of seconds in either a 12 hourly format (3600 * 12)
 *                    or 24 hourly format (3600 * 24). (Input/Output
 *   numOutputLines = The number of data values output.  For the "24 hourly"
 *                    format, numDays equals numOutputLines.  For the 
 *                    "12 hourly" format, numOutputLines is equal to twice 
 *                    the number of days since there are two 12 hour periods. 
 *                    (Input/Output)
 * firstValidTimeMatch = Time of first valid data value for point. Used in 
 *                       determining numDays value. (Output)
 * startTime_cml = The startTime derived from startDate command line option. 
 *                 (Input)
 *   endTime_cml = The endTime derived from numDays command line option. 
 *                 (Input)
 *        format = String denoting format of DWMLgenByDay product ("12 hourly"
 *                 or 24 hourly"). (Input)
 *  currDoubTime = Current time in double form. (Input)
 *       numPnts = Total number of points being processed. (Input)
 *       pntInfo = A pointer to each points' timezone, DST, sector, and 
 *                 starting match # and ending match # point's data can be
 *                 found in the match structure. (defined in sector.h). 
 *                 (Intput)
 *                      
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 * HISTORY:
 *  2/2006 Paul Hershberg (MDL): Created.
 * 11/2007 Paul Hershberg (MDL): Put in "if statement to decipher if current 
 *                               hour is between 5Z and 12Z.
 * 11/2007 Paul Hershberg (MDL): Added code to make startDate work as local 
 *                               time.
 *  6/2008 Paul Hershberg (MDL): Fixed numDays bug.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void prepareDWMLgenByDay(genMatchType *match, uChar f_XML, 
                         double *startTime_cml, double *endTime_cml,
			 double *firstValidTimeMatch, int *numDays, 
                         char *format, uChar *f_formatPeriodName,
                         uChar **wxParameters, int *timeInterval,
                         int *numOutputLines, char *summarization,
			 double currDoubTime, size_t numPnts, 
                         PntSectInfo *pntInfo, char **currentLocalDate, 
                         size_t numElem, genElemDescript *elem, 
                         uChar varFilter[NDFD_MATCHALL + 1])
{
   int i; /* Counter through the match structure. */
   int j; /* Counter through points. */
   int f_startTimeToZero = 0; /* After the point loop and upon exiting routine,
                               * flag denotes whether to turn incoming user
                               * supplied startTime to zero. */
   int f_endTimeToZero = 0; /* After the point loop and upon exiting routine,
                             * flag denotes whether to turn incoming user
                             * supplied endTime to zero. */
   int startNum; /* First index in match structure an individual point's data
                  * matches can be found. */
   int endNum;/* Last index in match structure an individual point's data
               * matches can be found. */
   double lastValidTimeMatch = 0.0; /* Time of last valid data value. Used in 
                                     * determining numDays value. */
   char currUTCHour[3];  /* Current UTC hour in 2-digit form. */
   char currUTCTime[30]; /* Current UTC Time in
                          * "2006-12-29T00:00:00-00:00" format. */
   char currUTCDate[3];  /* Current UTC date in 2-digit form. */

   /* Find the current hour in UTC form. */
   Clock_Print2(currUTCTime, 30, currDoubTime,
                "%Y-%m-%dT%H:%M:%S", 0, 0);

   currUTCHour[0] = currUTCTime[11];
   currUTCHour[1] = currUTCTime[12];
   currUTCHour[2] = '\0';
   currUTCDate[0] = currUTCTime[8];
   currUTCDate[1] = currUTCTime[9];
   currUTCDate[2] = '\0';

   /* Get the validTime of the first match and last match of the match 
    * structure. We'll need these to calculate numDays. 
    */
   for (j = 0; j < numPnts; j++)
   {

/*
      printf ("pntInfo[%d].startNum = %d\n",j,pntInfo[j].startNum);
      printf ("pntInfo[%d].endNum = %d\n",j,pntInfo[j].endNum);
*/
      startNum = pntInfo[j].startNum;
      endNum = pntInfo[j].endNum;

      firstValidTimeMatch[j] = match[startNum].validTime;
      lastValidTimeMatch = match[startNum].validTime;
      for (i = startNum; i < endNum; i++)
      {
         if (match[i].validTime < firstValidTimeMatch[j])
            firstValidTimeMatch[j] = match[i].validTime;
         if (match[i].validTime > lastValidTimeMatch)
            lastValidTimeMatch = match[i].validTime;
      }

      if (*startTime_cml == 0.0 && *endTime_cml == 0.0)
         numDays[j] = ceil(((lastValidTimeMatch - currDoubTime) / 3600) / 24);
      else if (*startTime_cml == 0.0 && *endTime_cml != 0.0)
      {	   
         /* Then endTime was entered on the command line argument. See if the time
          * entered occurs after the last valid data in NDFD. If so, simply treat
          * it as if no endTime was entered (set endTime = 0.0).
          */	   
         if (*endTime_cml < currDoubTime || *endTime_cml > lastValidTimeMatch)
         {
            f_endTimeToZero = 1;
            numDays[j] = ceil(((lastValidTimeMatch - currDoubTime) / 3600) / 24);\
         }
         else
            numDays[j] = (int)myRound(((*endTime_cml - currDoubTime) / 3600) / 24, 0);
      }
      else if (*startTime_cml != 0.0 && *endTime_cml == 0.0)
      {

         /* Then startTime was entered as a command line argument. First, see if 
          * the startTime occurs before current system time. If so, simply treat 
          * it as if no startTime was entered (set startTime = 0.0).
          */
         if (*startTime_cml < currDoubTime || *startTime_cml > lastValidTimeMatch)
         {
            f_startTimeToZero = 1;
            numDays[j] = ceil(((lastValidTimeMatch - currDoubTime) / 3600) / 24);
         }      
         else
         {
            numDays[j] = ceil(((lastValidTimeMatch - *startTime_cml) / 3600) / 24);
         }
      }      
      else if (*startTime_cml != 0.0 && *endTime_cml != 0.0)
      {
         /* Then both startTime and endTime were entered as command line arguments.
          * Simply subtract the times. Rule out erroneous choices for these times. If
          * that is the case, simply use times returned in match structure.
          */
          if (*startTime_cml < currDoubTime || *startTime_cml > lastValidTimeMatch)
          { 
             /* startTime not valid. */
             f_startTimeToZero = 1;
             if (*endTime_cml < currDoubTime || *endTime_cml > lastValidTimeMatch)
             {
                /* endTime is not valid. */
                f_endTimeToZero = 1;
                numDays[j] = ceil(((lastValidTimeMatch - currDoubTime) / 3600) / 24);
             }
             else /* startTime is not valid (before current time), but endTime is valid, 
                     shortening the time period data is retrieved for. */
             {
                if (atoi(currUTCHour) >= 5 && atoi(currUTCHour) < 12)
                /* This if statement is needed because we set the startTime at 5Z on
                 * the date specified by startDate. For example, if startDate is set
                 * on command line to "2007-10-29" then the startTime is converted to
                 * "2007-10-29T01:00:00-04:00". The endTime, based off of command 
                 * line argument "numDays" (ie. numDays = 5) is set to 12Z numDays 
                 * into the future. Here, it would be "2007-11-03T08:00:00-04:00". Thus, 
                 * if the current Time is between these hours, we need to account for 
                 * this with either the floor or ceil command when determining numDays.
                 */
                {
                   if (strcmp(currentLocalDate[j], currUTCDate) != 0)
                      numDays[j] = floor((*endTime_cml - currDoubTime) / (3600 * 24));
                   else
                      numDays[j] = floor((*endTime_cml - currDoubTime) / (3600 * 24));
                }
                else
                {
                   if (strcmp(currentLocalDate[j], currUTCDate) != 0)
                      numDays[j] = floor((*endTime_cml - (currDoubTime-(3600*24))) / (3600 * 24));
                   else
                      numDays[j] = ceil((*endTime_cml - currDoubTime) / (3600 * 24));
                }
             }
          }
          else /* startTime is valid. */
          {
             if (*endTime_cml < currDoubTime || *endTime_cml > lastValidTimeMatch)
             { 
                /* endTime is not valid. */
                f_endTimeToZero = 1;
                numDays[j] = ceil(((lastValidTimeMatch - *startTime_cml) / 3600) / 24);
             }
             else /* Both are valid, shortening the time period data is returned for. */
             {
                numDays[j] = floor((*endTime_cml - *startTime_cml) / (3600 * 24));
             }
          }
      }
 
      /* Assign the number of output lines, if DWMLgenByDay product. */
      if (f_XML == 3)
         /* All elements formatted this way in 12 hourly product except MaxT 
          * and Mint. 
          */
         numOutputLines[j] = numDays[j] * 2;
      else if (f_XML == 4)
         numOutputLines[j] = numDays[j];

      /* Flag the below five elements for formatting in the ouput XML (icons
       * will be the sixth element formatted). Set the value to = 1. 
       */
      for (i = 0; i < numElem; i++)
      {
         if (varFilter[elem[i].ndfdEnum] >= 2)
            wxParameters[j][i] = 1;
      }

      /* We need to collect data for the following four elements too. They are
       * not formatted in the output XML summary products, but are used in icon 
       * determination. Set these values to = 2. 
       */
      for (i = 0; i < numElem; i++)
      {
         if (elem[i].ndfdEnum == NDFD_WD)
            wxParameters[j][i] = 2;
         if (elem[i].ndfdEnum == NDFD_WS)
            wxParameters[j][i] = 2;
         if (elem[i].ndfdEnum == NDFD_SKY)
            wxParameters[j][i] = 2;
         if (elem[i].ndfdEnum == NDFD_WG)
            wxParameters[j][i] = 2;
      }
   } /* End Point Loop. */

   /* DWMLgenByDay, both formats, have pre-defined sets of NDFD parameters. */	   
   if (f_XML == 3)
   {
      *f_formatPeriodName = 1;
      *timeInterval = 3600 * 12;  /* Number of seconds in half a day */
      strcpy(summarization, "12hourly");
      strcpy(format, "12 hourly");
   }
   else if (f_XML == 4)       /* Note, there is no formatting of period
                               * names. */
   {	    
      *timeInterval = 3600 * 24;  /* Number of seconds in a whole day */
      strcpy(summarization, "24hourly");
      strcpy(format, "24 hourly");
   }

   if (f_startTimeToZero)
      *startTime_cml = 0.0;      
   if (f_endTimeToZero)
      *endTime_cml = 0.0; 

   return;
}
