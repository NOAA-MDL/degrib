/******************************************************************************
 * collectHazInfo () --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code gets information on each individual hazard. Each hazard is 
 *  defined as an accumulation of consecutive rows of hazard data. It records 
 *  the startTime, the endTime, the number of consecutive hours the hazard 
 *  exists, the time of a resolution split (1hr res changes to 6hr resolution 
 *  after 3rd day) and the string code.
 *
 * ARGUMENTS
 *         pnt = Current Point index. (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *    numMatch = The number of matches from degrib. (Input)
 *        numRowsHZ = Hazard Data Structure containing members:
 *                       total: the total number of rows of haz data for this 
                                element.
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
 *    startNum = First index in match structure an individual point's data 
 *               matches can be found. (Input)
 *      endNum = Last index in match structure an individual point's data
 *               matches can be found. (Input)
 * ptsIndivHazs = Array holding information about each point's individual 
 *                hazards. Structure contains hazard info members containing 
 *                startTime, the endTime, the number of consecutive hours the 
 *                hazard exists, the time of a resolution split (1hr res 
 *                changes to 3hr resolution after 3rd day) and the string code.
 *                (Output).
 *   numHazards = Number of hazards for each point. (Output).
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2008 Paul Hershberg (MDL): Created
 *  11/2008 Paul Hershberg (MDL): Accommodates multiple hazards. 
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void collectHazInfo(genMatchType *match, size_t numMatch, int startNum, 
                    int endNum, numRowsInfo numRowsHZ, size_t pnt,
                    hazInfo **ptsIndivHazs, int *numHazards)
{
   int i;                /* Counter thru match structure. */
   int j;
   HZtype *noNilorMissData = NULL; /* Hazard data taken from the match array. */
   int numActualRowsHZ; /* Actual number of rows with actual valid hazard 
                         * data. */
   hazInfo **rowHazard = NULL; /* Each row of valid hazard data is broken up 
                                * into component groups, if there are hazards 
                                * occurring simultaneously 
                                * (delimited by "^"). */
   int uglyStrIndex; /* Index thru the raw "ugly" hazard strings. */
   int maxGroups = 0; /* Maximum number of groups any one "ugly" hazard string 
                       * contains. */ 
   int noneOrMissCount = 0; /* Tracks rows with missing or no ("<None>") valid 
                             * hazard data. */
   int numGroups = 0; /* Number of groups each ugly hazard string is split into 
                       * (delimited by "^"). */
   int row; /* Counter thru actual rows with valid hazard data. */
   int group; /* Counter thru each group of each row with valid hazard data. */
   int validHazIndex = 0; /* Counter used when collecting valid hazard data 
                           * from match structure. */
   int hzIndex = -1; /* Index of array containing each point's hazards (and 
                     * subsequent info). */
   int f_firstHazHour = 1; /* Flag denoting if we are at the row where the first 
                            * of the consecutive hazard rows start. */ 
   char *pstr = NULL; /* Char pointer holding hazard string. */ 
   char tempStr[150]; /* Temp string holding hazard string. */
   double timeResSplit = -1.0; /* Time of hazard where there is a resolution 
                                * split from 1hr to 6hr. */  
   double timeSplit = 0.0; /* Used in determining timeResSplit. */
   int f_timeResSplit = 0; /* Denotes when resolution time split was found. */
   int hazRes = 3600; /* Default resolution between consecutive rows of valid 
                       * hazard data. */
   int rowsToSubtract = 1; /* Used to find last row of hazard. */
   int numGroupsCurrRow = 0; /* Number of groups (hazards in ugly string) per 
                              * current row. */
   int numGroupsNextRow = 0; /* Number of groups (hazards in ugly string) per 
                              * next row. */
   int f_combinedCurrHaz = 0; /* Denotes two hazards could be combined in the post 
                               * proccessing. */
   int f_combinedPreviousHaz = 0; /* Denotes during the previous looping, a 
                                   * hazard was combined in the post proccessing. 
                                   */
   int f_currHazClosed = 0; /* Denotes whether current hazard has been closed 
                             * (an end was found). */

   /* Remove any data user might have requested to cut off. */
   numActualRowsHZ = numRowsHZ.total-numRowsHZ.skipBeg-numRowsHZ.skipEnd;

   /* Firstly, create a subset array of structures holding the hazard data from
    * the match structure, (get all hazard ugly strings).
    */
   for (i = startNum; i < endNum; i++)
   {
      if (match[i].elem.ndfdEnum == NDFD_WWA && 
	  match[i].validTime >= numRowsHZ.firstUserTime &&
	  match[i].validTime <= numRowsHZ.lastUserTime)
      {
         if (match[i].value[pnt].valueType != 0) /* "0" denotes double data, 
                                                  * which hazards are not. */
         {
            /* Get the validTime when the resolution switches from 1hr to 
             * 6hr. 
             */
            if (!f_timeResSplit && (i != endNum-1))
            {
               timeSplit = (match[i+1].validTime-match[i].validTime) / 3600;
               if (timeSplit > 5.0)
               {
                  timeResSplit = match[i].validTime;
                  f_timeResSplit = 1;
               }
            }

            /* Remove rows with nil or no data, so each array row contains 
             * valid data. 
             */
            if (match[i].value[pnt].valueType == 2) /* 2 is missing data. */
               noneOrMissCount++;
            else if ((match[i].value[pnt].valueType == 1 && /* 1 = char data. */
               match[i].value[pnt].str[0] == '<' && 
               match[i].value[pnt].str[1] == 'N' && 
               match[i].value[pnt].str[2] == 'o'))
                  noneOrMissCount++;
            else /* Valid hazard data. Collect it. */           
            {
               noNilorMissData = realloc(noNilorMissData, (validHazIndex+1)
                                 * sizeof(HZtype));
               strcpy(noNilorMissData[validHazIndex].str,
                      match[i].value[pnt].str);
               noNilorMissData[validHazIndex].validTime =
                     match[i].validTime;
               validHazIndex++;
            }
         }
      }
   }

   /* Subtract missing or rows with no hazard data. */
   numActualRowsHZ = numActualRowsHZ - noneOrMissCount;

   /* If there are no active hazards for this point, flag it and return. */
   if (numActualRowsHZ == 0)
   {
      *numHazards = 0;
      return;
   }

   /* Loop thru the Hazard ugly strings that contain data and get the maximum 
    * number of groups in all the ugly strings.  
    */
   for (uglyStrIndex = 0; uglyStrIndex < numActualRowsHZ; uglyStrIndex++)
   {
      numGroups = 0;

      /* See if there are multiple hazards (groups) in this ugly hazard string 
       * so we can track each one. Initialize/Reset a few things. 
       */
      strcpy(tempStr, noNilorMissData[uglyStrIndex].str);
      pstr = strtok(tempStr, "^");
      while (pstr != NULL)
      { 
         numGroups++;
         pstr = strtok (NULL, "^");
      }
      if (numGroups > maxGroups)
         maxGroups = numGroups;

   }

   /* Check again to make sure there are no active hazards for this point. */
   if (maxGroups == 0)
   {
      *numHazards = 0;
      return;
   }

   /* Loop thru each hazard ugly string & split each group into it's own array
    * element. 
    */
   rowHazard = (hazInfo **)malloc (numActualRowsHZ * sizeof (hazInfo *));
   for (row = 0; row < numActualRowsHZ; row++)
   {
      rowHazard[row] = (hazInfo *)malloc (maxGroups * sizeof (hazInfo));
      numGroups = 0;
      strcpy(tempStr, noNilorMissData[row].str);
      pstr = strtok(tempStr, "^");
      while (pstr != NULL)
      { 
         numGroups++;
         strcpy (rowHazard[row][numGroups-1].code, pstr);
         rowHazard[row][numGroups-1].startHour = noNilorMissData[row].validTime;
         pstr = strtok (NULL, "^");
      }      
   }

   /* Must set the second dimension of array indivHaz to NULL before realloc.
    * Otherwise it's undefined. 
    */
   *ptsIndivHazs = NULL;
   /* Loop thru each group, per row, and get info on each hazard. */
   for (group = 0; group < maxGroups; group++)
   {
      hazRes = 3600;
      rowsToSubtract = 1;
      f_currHazClosed = 0;
      if (numActualRowsHZ == 1) /* Special case of just one row of hazard(s). */
      {
         hzIndex++;
         (*ptsIndivHazs) = (hazInfo *)realloc((*ptsIndivHazs), (hzIndex+1) * sizeof(hazInfo));
         (*ptsIndivHazs)[hzIndex].valTimeResSplit = timeResSplit;
         (*ptsIndivHazs)[hzIndex].startHour = rowHazard[0][group].startHour;
         (*ptsIndivHazs)[hzIndex].endHour = rowHazard[0][group].startHour;
         strcpy((*ptsIndivHazs)[hzIndex].code, rowHazard[0][group].code);
         (*ptsIndivHazs)[hzIndex].numConsecRows = 1;
         f_currHazClosed = 1;
         f_firstHazHour = 1;
      }
      else /* Loop thru rows of hazard(s). */
      {
         for (row = 1; row < numActualRowsHZ; row++)
         {
            /* Make sure the row processed has less groups than row with maximum number 
             * of groups. 
             */
            numGroupsCurrRow = 0;
            strcpy(tempStr, noNilorMissData[row-1].str);
            pstr = strtok(tempStr, "^");
            while (pstr != NULL)
            {
               numGroupsCurrRow++;
               pstr = strtok (NULL, "^");
            }

            numGroupsNextRow = 0;
            strcpy(tempStr, noNilorMissData[row].str);
            pstr = strtok(tempStr, "^");
            while (pstr != NULL)
            {
               numGroupsNextRow++;
               pstr = strtok (NULL, "^");
            }
            if (numGroupsCurrRow-1 < group || numGroupsNextRow-1 < group)
            {
               rowsToSubtract++;

               if (row != numActualRowsHZ - 1)
               {
                  if (numGroupsCurrRow-1 >= group && numGroupsNextRow-1 < group) 
                  /* We have a new, single hour hazard in Current Row, or current Haz ends. */
                  {
                     if (f_firstHazHour == 0)
                     {
                        (*ptsIndivHazs)[hzIndex].endHour = rowHazard[row-1][group].startHour;
                        f_currHazClosed = 1;
                        f_firstHazHour = 1;
                     }
                     else
                     {
                        hzIndex++;
                        (*ptsIndivHazs) = (hazInfo *)realloc((*ptsIndivHazs), (hzIndex+1) * sizeof(hazInfo));
                        (*ptsIndivHazs)[hzIndex].valTimeResSplit = timeResSplit;
                        (*ptsIndivHazs)[hzIndex].startHour = rowHazard[row-1][group].startHour;
                        (*ptsIndivHazs)[hzIndex].endHour = rowHazard[row-1][group].startHour;
                        strcpy((*ptsIndivHazs)[hzIndex].code, rowHazard[row-1][group].code);
                        (*ptsIndivHazs)[hzIndex].numConsecRows = 1;
                        f_currHazClosed = 1;
                        f_firstHazHour = 1;
                     }
                     continue;
                  }
                  else if (numGroupsNextRow-1 >= group && numGroupsCurrRow-1 < group) /* We have a new hazard in Next Row. */
                  {
                     hzIndex++;
                     (*ptsIndivHazs) = (hazInfo *)realloc((*ptsIndivHazs), (hzIndex+1) * sizeof(hazInfo));
                     (*ptsIndivHazs)[hzIndex].valTimeResSplit = timeResSplit;
                     (*ptsIndivHazs)[hzIndex].startHour = rowHazard[row][group].startHour;
                     (*ptsIndivHazs)[hzIndex].endHour = rowHazard[row][group].startHour;
                     strcpy((*ptsIndivHazs)[hzIndex].code, rowHazard[row][group].code);
                     (*ptsIndivHazs)[hzIndex].numConsecRows = 1;
                     f_firstHazHour = 0;
                     continue;
                  }
                  else
                     continue;
               }
               else /* At end of row loop. */
               {
                  if (numGroupsNextRow-1 >= group && numGroupsCurrRow-1 >= group)
                  {
                     (*ptsIndivHazs)[hzIndex].endHour = rowHazard[numActualRowsHZ-rowsToSubtract][group].startHour;
                     f_firstHazHour = 1;
                     f_currHazClosed = 1;
                  }
                  else if (numGroupsNextRow-1 >= group && numGroupsCurrRow-1 < group) /* New hazard at end of row. */
                  {
                     hzIndex++;
                     (*ptsIndivHazs) = (hazInfo *)realloc((*ptsIndivHazs), (hzIndex+1) * sizeof(hazInfo));
                     (*ptsIndivHazs)[hzIndex].valTimeResSplit = timeResSplit;
                     (*ptsIndivHazs)[hzIndex].startHour = rowHazard[row][group].startHour;
                     (*ptsIndivHazs)[hzIndex].endHour = rowHazard[row][group].startHour;
                     strcpy((*ptsIndivHazs)[hzIndex].code, rowHazard[row][group].code);
                     (*ptsIndivHazs)[hzIndex].numConsecRows = 1;
                  }
                  else if ((numGroupsNextRow-1 < group) && (row == numActualRowsHZ-1) 
                     && (strcmp(noNilorMissData[row].str, noNilorMissData[row-1].str) != 0))
                  {
                     (*ptsIndivHazs)[hzIndex].endHour = rowHazard[numActualRowsHZ-2][group].startHour;
                     f_currHazClosed = 1;
                     f_firstHazHour = 1;
                     break;
                  }                 
                  f_currHazClosed = 1;
                  f_firstHazHour = 1;
                  break;
               }
            }
    
            if (f_firstHazHour)
            {
               hzIndex++;
               rowsToSubtract = 1;            
               (*ptsIndivHazs) = (hazInfo *)realloc((*ptsIndivHazs), (hzIndex+1) * sizeof(hazInfo));
               (*ptsIndivHazs)[hzIndex].valTimeResSplit = timeResSplit;
               (*ptsIndivHazs)[hzIndex].startHour = rowHazard[row-1][group].startHour;
               strcpy((*ptsIndivHazs)[hzIndex].code, rowHazard[row-1][group].code);
               (*ptsIndivHazs)[hzIndex].numConsecRows = 1;
               f_currHazClosed = 0;
               f_firstHazHour = 0;
            } 

            if (f_timeResSplit)
            {
               if (rowHazard[row-1][group].startHour >= timeResSplit)
                  hazRes = 3600*6;
            }

            /* Check to see if hazard exists in next data row, and there is no 
             * gap in the hours of resolution. If so, it is consecutive. 
             */
            if ((strstr(rowHazard[row][group].code, rowHazard[row-1][group].code) != '\0') && 
                (rowHazard[row-1][group].startHour + hazRes == rowHazard[row][group].startHour))
            { 
               (*ptsIndivHazs)[hzIndex].numConsecRows++;
            }
            else /*Hazard closes. */
            {
               (*ptsIndivHazs)[hzIndex].endHour = rowHazard[row-1][group].startHour;
               f_firstHazHour = 1;
               f_currHazClosed = 1;

               if (row == numActualRowsHZ-1) /* Special case, we have a lone hazard in final row of this group. */
               {
                  hzIndex++;
                  (*ptsIndivHazs) = (hazInfo *)realloc((*ptsIndivHazs), (hzIndex+1) * sizeof(hazInfo));
                  (*ptsIndivHazs)[hzIndex].valTimeResSplit = timeResSplit;
                  (*ptsIndivHazs)[hzIndex].startHour = rowHazard[row][group].startHour;
                  (*ptsIndivHazs)[hzIndex].endHour = rowHazard[row][group].startHour;
                  strcpy((*ptsIndivHazs)[hzIndex].code, rowHazard[row][group].code);
                  (*ptsIndivHazs)[hzIndex].numConsecRows = 1;
                  f_currHazClosed = 1;
               }  
            }
         }
      }

      /* At end of row. Go to next group. Close out Hazard if ends here at end of rows. */
      if (!f_currHazClosed) 
      {
         (*ptsIndivHazs)[hzIndex].endHour = rowHazard[numActualRowsHZ-1][group].startHour;
         f_currHazClosed = 1;
         f_firstHazHour = 1;
      }
   }

   *numHazards = hzIndex + 1;

   /* Do we need to post process the ptsIndivHazs array? Check thru the hazards 
    * and see if any can be combined. Reduce the number of hazards accordingly
    * by boggying up those that were combined.
    */
   if (maxGroups > 1)
   {
      for (i = 0; i < *numHazards; i++)
      {
         f_combinedCurrHaz = 0;

         /* If there were hazards that were combined previously, start the loop
          * again to see if this newly combined hazard can further be combined.
          */
         if (f_combinedPreviousHaz)
            i = 0;
         f_combinedPreviousHaz = 0;
          
         for (j = *numHazards-1; j >= i+1; j--)
         {
            if ((*ptsIndivHazs)[j].numConsecRows == 0)
               continue;

            if (strcmp((*ptsIndivHazs)[i].code, (*ptsIndivHazs)[j].code) == 0)
            {
               if ((*ptsIndivHazs)[i].startHour >= (*ptsIndivHazs)[i].valTimeResSplit)
                  hazRes = 3600*6;
               else
                  hazRes = 3600;

               if ((*ptsIndivHazs)[j].endHour + hazRes == (*ptsIndivHazs)[i].startHour)
               {
                  /* Match made, combine hazards. Work off of existing [i] hazard. */
                  (*ptsIndivHazs)[i].startHour = (*ptsIndivHazs)[j].startHour;
                  (*ptsIndivHazs)[i].numConsecRows = (*ptsIndivHazs)[i].numConsecRows + (*ptsIndivHazs)[j].numConsecRows;
                  
                  /* Boggie up this [j] hazard in the ptsIndivHaz array. */
                  (*ptsIndivHazs)[j].numConsecRows = 0;
                  f_combinedCurrHaz = 1;
               }
            }
         }
         if (!f_combinedCurrHaz) /* Go the other way and look to combine. */
         {
            for (j = i+1; j < *numHazards; j++)
            {
               if ((*ptsIndivHazs)[j].numConsecRows == 0)
                  continue;

               if (strcmp((*ptsIndivHazs)[i].code, (*ptsIndivHazs)[j].code) == 0)
               {
                  if ((*ptsIndivHazs)[i].endHour >= (*ptsIndivHazs)[i].valTimeResSplit)
                     hazRes = 3600*6;
                  else
                     hazRes = 3600;

                  if ((*ptsIndivHazs)[i].endHour + hazRes == (*ptsIndivHazs)[j].startHour)
                  {
                     /* Match made, combine hazards.  Work off of existing [i] hazard. */
                     (*ptsIndivHazs)[i].endHour = (*ptsIndivHazs)[j].endHour;
                     (*ptsIndivHazs)[i].numConsecRows = (*ptsIndivHazs)[i].numConsecRows + (*ptsIndivHazs)[j].numConsecRows;
                  
                     /* Boggie up this [j] hazard in the ptsIndivHaz array. */
                     (*ptsIndivHazs)[j].numConsecRows = 0;
                     f_combinedCurrHaz = 1;
                  }
               }
            }
         }
         if (f_combinedCurrHaz)
            f_combinedPreviousHaz = 1;
      }
   }
       
   /* Free some things. */
   for (row = 0; row < numActualRowsHZ; row++)
      free(rowHazard[row]);
   free(rowHazard);
   free(noNilorMissData);
   
   return;
}
