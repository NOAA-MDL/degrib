/******************************************************************************
 * hazTimeInfo() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code calls routine that get the time layouts for each individual 
 *  hazard (if it is for one of the summary products). It also calls a 
 *  routine that retrieves the times that correspond to the forecast period 
 *  boundaries.
 *
 * ARGUMENTS
 * multiLayouts = The key linking each point's individual hazards to their 
 *                valid times (ex. k-p3h-n42-3). This array is allocated to the
 *                number of hazards per individual point, since each hazard is 
 *                treated as it's own element if user chose a summary product. 
 *                (Output)
 * timeCoordinate = The time coordinate that the user wants the time 
 *                  information communicated it. Currently only local time is 
 *                  supported. (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *  parameters = An xml Node Pointer denoting the <parameters> node to which 
 *               these values will be attached (child node). (Output)
 * ptsIndivHazs = Array holding information about each point's individual 
 *                hazards. Structure contains hazard info members containing 
 *                startTime, the endTime, the number of consecutive hours the 
 *                hazard exists, the time of a resolution split (1hr res 
 *                changes to 3hr resolution after 3rd day) and the string code.
 *                (Input).
 *  numHazards = Number of active hazards for this point. (Input).
 *  summarization = The type of temporal summarization being used.
 *                  Currently, no summarization is done in time. (Input)
 *       TZoffset = Number of hours to add to current time to get GMT time. 
 *                  (Input)
 *   f_observeDST = Flag determining if current point observes Daylight 
 *                  Savings Time. (Input)  
 *     currentDay = Current day's 2 digit date. (Input)
 *        frequency = Set to "boggus" for DWMLgen products, and to "12 hourly" 
 *                    or "24 hourly" for the DWMLgenByDay products. (Input)  
 *           data = An xml Node Pointer denoting the <data> node. (Input)
 *  startTime_cml = Incoming argument set by user as a double in seconds 
 *                  since 1970 denoting the starting time data was retrieved
 *                  for from NDFD. (Input)
 * currentDoubTime = Current time in double form in sec since 1970. (Input)
 * startPositions = The index of where the current forecast period begins.  Used
 *                  to determine sky cover trends (i.e. increasing clouds) for 
 *                  DWMLgenByDay products. (Output)
 *   endPositions = The index of where the current forecast period ends.  Used
 *                  to determine sky cover trends (i.e. increasing clouds) for 
 *                  DWMLgenByDay products. (Output)
 * numOutputLines = The number of data values output/formatted for each element. 
 *                  (Input)
 *   timeInterval = Number of seconds in either a 12 hourly format (3600 * 12)
 *                  or 24 hourly format (3600 * 24). (Input)
 *        numRowsHZ = Structure containing members:
 *                       total: the total number of rows of data for this element.
 *                     skipBeg: the number of beginning rows not formatted due 
 *                              to a user supplied reduction in time (startTime
 *                              arg is not = 0.0)
 *                     skipEnd: the number of end rows not formatted due to a 
 *                              user supplied reduction in time (endTime arg
 *                              is not = 0.0)
 *               firstUserTime: the first valid time interested per element, 
 *                              taking into consideration any data values 
 *                              (rows) skipped at beginning of time duration.
 *                lastUserTime: the last valid time interested per element, 
 *                              taking into consideration any data values 
 *                              (rows) skipped at end of time duration.
 *          f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product.
 *                     5 = DWMLgen's RTMA "time-series" product.
 *                     6 = DWMLgen's RTMA + NDFD "time-series" product. (Input)
 *    currentHour = Current hour. (Input)
 *      startTime = Incoming argument set by user as a double in seconds 
 *                  since 1970 denoting the starting time data is to be 
 *                  retrieved for from NDFD. (Set to 0.0 if not entered.)
 *                  (Input) 
 *    startNum = First index in match structure an individual point's data 
 *               matches can be found. (Input)
 *      endNum = Last index in match structure an individual point's data
 *               matches can be found. (Input)
 * firstPeriodStartTime = The beginning of the first forecast period (06 hr
 *                        or 18hr) based on the user supplied startTime argument. 
 *                        (Input)
 *    lastPeriodEndTime = The end of the last forecast period (18 hr) based 
 *                        on the startTime & numDays arguments. (Input)
 *    periodTimes = The times bordering each forecast period time. (Input)
 * numPeriodTimes = The number of periodTimes. (Input)
 *   numLayoutSoFar = The total number of time layouts that have been created 
 *                    so far. (Input)
 * numCurrentLayout = Number of the layout we are currently processing. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2008 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void hazTimeInfo(char ***multiLayouts, const char *timeCoordinate, 
                 hazInfo *ptsIndivHazs, int numHazards, char *summarization, 
                 genMatchType *match, size_t numMatch, double TZoffset, 
                 sChar f_observeDST, char *currentHour, char *currentDay, 
                 char *frequency, xmlNodePtr data, double startTime_cml, 
                 double currentDoubTime, uChar f_XML, int startNum, int endNum,
                 double firstPeriodStartTime, double lastPeriodEndTime, 
                 int timeInterval, double **periodTimes, int *numPeriodTimes, 
                 size_t *numLayoutSoFar, uChar *numCurrentLayout)
{
   int i;                 /* Counter thru each hazard for the point. */
   char hazLayoutKey[20]; /* The key linking each individual hazard to it's
                           * valid times (ex. k-p9h-n42-1). */

   /* Get the times bordering each forecast period time. */
   *periodTimes = NULL;
   *numPeriodTimes = 0;
   getPeriodTimes(firstPeriodStartTime, lastPeriodEndTime, timeInterval, 
                  TZoffset, f_observeDST, periodTimes, numPeriodTimes);

   /* Get each individual hazard's time layout. */
   *multiLayouts = (char **)malloc(numHazards * sizeof(char*));
   for (i = 0; i < numHazards; i++)
   {
      if (ptsIndivHazs[i].numConsecRows != 0)
      {
         generateHazTimeLayout(hazLayoutKey, timeCoordinate, ptsIndivHazs[i], 
                               summarization, match, numMatch, 
                               TZoffset, f_observeDST, &numLayoutSoFar, 
                               &numCurrentLayout, currentHour, currentDay, 
                               frequency, data, startTime_cml, currentDoubTime, 
                               &(ptsIndivHazs[i].numOutputLines), f_XML, 
                               startNum, endNum, periodTimes, numPeriodTimes);

         (*multiLayouts)[i] = (char *)malloc((strlen(hazLayoutKey)+1) *
                              sizeof(char));
         strcpy((*multiLayouts)[i], hazLayoutKey);
      }
      else
      {
         (*multiLayouts)[i] = NULL;
      }
   }

   return;
}
