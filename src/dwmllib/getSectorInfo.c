/*****************************************************************************
 * getSectorInfo() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Gets the first and last index in the match structure in which matches correspond 
 *  to a particular sector, for each particular point.
 *   
 * ARGUMENTS
 *             pnts = A pointer to the points to probe for (defined in type.h).
 *                    (Input)
 *          numPnts = Number of points to probe for. (Input) 
 *          pntInfo = A pointer to each points' timezone, DST, sector, and 
 *                    starting match # and ending match # that a point's data 
 *                    can be found in the match structure. (defined in 
 *                    sector.h). (Output)
 *            match = Pointer to the structure of element matches returned from
 *                    grid probe. (Input)  
 *         numMatch = The number of matches from degrib. (Input)
 *          f_nhemi = Denotes at least one match was found in the Northern 
 *                    Hemisphere sector (sector 5). (Input)
 *       f_puertori = Denotes at least one match was found in the Puerto Rico
 *                    sector (sector 1). (Input)
 *          f_conus = Denotes at least one match was found in the Conus sector 
 *                    (sector 0). (Input)
 *         numNhemi = Number of matches in nhemi sector. If this number is 
 *                    equal to numMatch, all matches are found in one sector 
 *                    (this can occur if there is a call for tropical wind 
 *                    thresholds for a call with puertori and conus points. 
 *                    (Input)
 *        numSector = Number of sectors that points were quered for fell in
 *                    (note, nhemi not included in the number, even if matches
 *                    occured in nhemi sector). This does not necessarily equal
 *                    the number of sectors returned in the match structure, as
 *                    that only returns valid data. (Input)
 *           sector = Array of sectors that points were found in (note, nhemi
 *                    not included in array, even if matches occured in nhemi
 *                    sector). (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2007 Paul Hershberg (MDL): Created.
 * 11/2007 Paul Hershberg (MDL): Added code that will format points queried for
 *                               in original call with xsi:nil even if there 
 *                               were zero matches in this point.
 *  2/2008 Paul Hershberg (MDL): Dealt with points occurring in 3+ sectors.
 *  2/2007 Paul Hershberg (MDL): Added the flag f_pntInNhemi.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void getSectorInfo(PntSectInfo *pntInfo, Point *pnts, size_t numPnts,
                   genMatchType *match, size_t numMatch, 
                   size_t numSector, char **sector, int f_nhemi, 
                   int *f_puertori, int *f_conus, int numNhemi)
{
   int priorMatchCount = 0; /* Counter denoting prior matches when tracking how
                             * many fall into a sector. */
   int i; /* Match counter. */
   int j; /* Point Loop counter. */
   int k; /* pntInfo.numSector counter. */
   int m; /* Counter thru temporary deltaSect array. */
   int conus = 0; /* enum number for conus sector. */
   int puertori = 1; /* enum number for puertori sector. */
   int nhemi = 5; /* enum number for nhemi sector. */
   int alaska = 4; /* enum number for alaska sector. */
   int guam = 3; /* enum number for guam sector. */
   int hawaii = 2; /* enum number for hawaii sector. */
   int newSect = 0; /* Denotes the number of sectors skipped if the special case
                     * of there being sector 5 (nhemi) exists. */
   int matchStart = 1; /* Fudge the stating index thru the match structure in 
                        * order to skip the two sectors info has already been 
                        * found for if the special case of there being sector 5 
                        * (nhemi) exists. */
   typedef struct /* Structure containing the starting index a point can find 
                   * it's matches in the match structure, due to a multiple 
                   * point call that has points in different sectors. */
   {
      int startNum;
      int endNum;
      int enumNum;
   } sectorSplit;
 
   sectorSplit *deltaSect = NULL; /* Temporary holder of structure described
                                   * above. */
   sectorSplit conusSect; /* Temporary holder of conus sector information as
                           * the structure described above. */
   sectorSplit puertoriSect; /* Temporary holder of puerto rico sector
                              * information as the structure described above. */
   int numSectInMatch = 0; /* Total number of sectors for matches returned in
                            * match structure. */
   int numSectInPoint = 0; /* Number of sectors the original points queried for
                              fell into. */
   int numSectsInPointNotFoundInMatch = 0; /* Difference between number of
                                              sectors original points queried
                                              for fell into, and number of
                                              sectors matches fell into. */
   int f_sectInPointNotFoundInMatch = 0; /* Denotes if a sector was found in 
                                            the original point's query, and not
                                            in any of the matches. */
   int *sectsInPointNotFoundInMatch = NULL; /* Array of sectors found in 
                                               original point's queried for 
                                               that were not found in 
                                               matches. */
   int *matchSectID = NULL; /* Array containing all sectors matches fell 
                               into. */
   int *pointSectID = NULL; /* Array containing all sectors original queried 
                               points fell into. */
   int f_sectorMatch = 0;   /* Denotes a match between the matches sector and
                               the sector(s) found in the pntInfo array. Used to
                               escape final "for" loop in this routine. */
   int f_pntInNhemi = 0; /* Flag denoting if a point falls in the NHemi sector 
                          * only, but with no matches in the match structure 
                          * existing in the NHemi sector. */

   /* Initialize all points to be able to access entire match structure, as if
    * there were only one point or all points in a multiple call come from one
    * sector. Also, initialize the conus and puertori SectorSPlit arrays.
    */
   conusSect.startNum = 0;
   conusSect.endNum = numMatch;  
   conusSect.enumNum = conus;

   puertoriSect.startNum = 0;
   puertoriSect.endNum = numMatch;  
   puertoriSect.enumNum = puertori;

   for (j = 0; j < numPnts; j++)
   {
      pntInfo[j].startNum = 0;
      pntInfo[j].endNum = numMatch;
   }

   /* Detect condition of multiple sectors in a multiple point call. 
    * If there is only one sector, simply dummy up needed information. 
    * Else, access routine finding out which matches in the match structure 
    * can be accessed per point. 
    *
    * Match structure is already sorted in the sector order: conus (0), nhemi (5),
    * puertori (1), hawaii (2), guam (3), and alaska (4). 

    * The conus (0) and nhemi (5) sectors can be combined as criteria for 
    * determining where in the match structure a conus point can find matches. 
    * The nhemi (5) and peurtori (1) sectors can be combined as criteria for 
    * determining where in the match structure a peurtori point can find matches. 
    */
  
   /* Find how many sectors the matches from match structure fell into. */
   matchSectID = realloc(matchSectID, (numSectInMatch+1) * sizeof(int));
   matchSectID[numSectInMatch] = match[0].f_sector;
   numSectInMatch++;
   for (i = 1; i < numMatch; i++)
   {
      if (match[i-1].f_sector != match[i].f_sector)
      {
         matchSectID = realloc(matchSectID, (numSectInMatch+1) * sizeof(int));
         matchSectID[numSectInMatch] = match[i].f_sector;
         numSectInMatch++;
      }
   }

   if ((numSectInMatch > 1) && (numNhemi != numMatch))
   {
   /* If numNhemi == numMatch or numSectorsInMatch = zero, then we don't go into
    * this loop since all matches are from the same sector. No sector
    * adjustments at all are needed.
    */

      /* If matches exist in the "nhmei" sector, see if there are "conus" or 
       * "peurtori" sectors in this call. 
       */
      if (f_nhemi)
      {
         for (i = 0; i < numMatch; i++)
         { 
            if (match[i].f_sector == conus)
               *f_conus = 1;
            if (match[i].f_sector == puertori)
               *f_puertori = 1;
         }
      }

      /* Find how many sectors the original points queried for were found in. */
      for (i = 0; i < numSector; i++)
      { 
         if (strcmp(sector[i], "conus") == 0)
         {
            pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
            pointSectID[i] = conus;
            numSectInPoint++;
            continue;
         }
         if (strcmp(sector[i], "puertori") == 0)
         {
            pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
            pointSectID[i] = puertori;
            numSectInPoint++;
            continue;
         }
         if (strcmp(sector[i], "alaska" ) == 0)
         {
            pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
            pointSectID[i] = alaska;
            numSectInPoint++;
            continue;
         }         
         if (strcmp(sector[i], "guam") == 0)
         {
            pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
            numSectInPoint++;
            pointSectID[i] = guam;
            continue;
         }
         if (strcmp(sector[i], "hawaii" ) == 0)
         {
            pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
            numSectInPoint++;
            pointSectID[i] = hawaii;
            continue;
         }
      }

      /* "nhemi" sector is not counted in variable numSector. So if f_nhemi = 1, 
       * add it to the total and rename variable. 
       */
      if (f_nhemi)
      {
         pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
         pointSectID[i] = nhemi;
         numSectInPoint++;
      } 
        
      /* If there are matches in all three sectors (nhemi, conus, puertori), it 
       * is a special case. We need to run through the match structure firstly 
       * and get this special case.
       */
      if (f_nhemi && *f_conus && *f_puertori)
      {
         /* Get Conus sector info, which will include nhmei sector info. */
         for (i = 1; i < numMatch; i++)
         {
            priorMatchCount++;
            if (match[i-1].f_sector == nhemi && match[i].f_sector == puertori)
            {
               conusSect.startNum = i-priorMatchCount;
               conusSect.endNum = i-1;  
               conusSect.enumNum = match[i-priorMatchCount].f_sector;
               break;
            }
         }

         /* Get PuertoRi sector info, which will include nhmei sector info. */
         priorMatchCount = 0;
         for (i = 1; i < numMatch; i++)
         {
            if (match[i].f_sector == nhemi || match[i].f_sector == puertori)
               priorMatchCount++;
            if (((match[i-1].f_sector != match[i].f_sector) && 
                (match[i-1].f_sector == puertori)) ||
                (i == numMatch - 1))
            {
               puertoriSect.startNum = i-priorMatchCount;
               puertoriSect.endNum = i-1;  
               puertoriSect.enumNum = match[i-1].f_sector;
               matchStart = i+1; /* Jump into the routine, below, but change the
                                  * starting match structure index to skip the 
                                  * two sectors info has already been found 
                                  * for. */
               break;
            }
         }

         /* Put these 2 sector's information into the deltaSect structure. */
         deltaSect = calloc(2, sizeof(sectorSplit));
         deltaSect[conus].startNum = conusSect.startNum;
         deltaSect[conus].endNum = conusSect.endNum;
         deltaSect[conus].enumNum = conusSect.enumNum;

         deltaSect[puertori].startNum = puertoriSect.startNum;
         deltaSect[puertori].endNum = puertoriSect.endNum;
         deltaSect[puertori].enumNum = puertoriSect.enumNum;

         /* Account for the two sectors already found. */
         newSect = 2;
      }

      if (numMatch != 2)
      {
         priorMatchCount = 0;
         for (i = matchStart; i < numMatch; i++)
         {
            priorMatchCount++;
            if (((match[i-1].f_sector != match[i].f_sector) &&
                !(match[i-1].f_sector == conus && match[i].f_sector == nhemi) &&
                !(match[i-1].f_sector == nhemi && match[i].f_sector == puertori)) ||
                 (i == numMatch - 1))
            {
               deltaSect = realloc(deltaSect, (newSect+1) * sizeof(sectorSplit));
/*             printf (" are we here \n"); */
               deltaSect[newSect].startNum = i-priorMatchCount;
               if (i == (numMatch - 1))
                  deltaSect[newSect].endNum = numMatch-1;
               else           
                  deltaSect[newSect].endNum = i-1;
               if (match[i-priorMatchCount].f_sector == nhemi)
                  deltaSect[newSect].enumNum = match[i-1].f_sector;              
               else
                  deltaSect[newSect].enumNum = match[i-priorMatchCount].f_sector;
               newSect++;
               priorMatchCount = 0;
            }
         }
         /* Check if there was a sector change in final match. */
         if (match[numMatch-2].f_sector != match[numMatch-1].f_sector)
         {
            deltaSect = realloc(deltaSect, (newSect+1) * sizeof(sectorSplit));
            deltaSect[newSect].startNum = numMatch-1;
            deltaSect[newSect].endNum = numMatch-1;
            deltaSect[newSect].enumNum = match[numMatch-1].f_sector;

            /* Update previous deltaSect. */
            deltaSect[newSect-1].endNum = deltaSect[newSect-1].endNum - 1;
            deltaSect[newSect-1].enumNum = match[numMatch-2].f_sector;

            newSect++;
         }

         /* Find any sectors missing from sectors found in match structure but 
          * were found in the original point query. 
          */ 
         if (numSectInMatch < numSectInPoint)
         {
            for (k = 0; k < numSectInPoint; k++)
            {
               f_sectInPointNotFoundInMatch = 0;
               for (m = 0; m < numSectInMatch; m++)
               {
                  if (pointSectID[k] != matchSectID[m])
                     f_sectInPointNotFoundInMatch = 1;
                  else
                  {   
                     f_sectInPointNotFoundInMatch = 0;
                     break;
                  }
               }
               if (f_sectInPointNotFoundInMatch)
               { 
                  numSectsInPointNotFoundInMatch++;
                  sectsInPointNotFoundInMatch = realloc(sectsInPointNotFoundInMatch, 
                                     numSectsInPointNotFoundInMatch * sizeof(int));
                  sectsInPointNotFoundInMatch[numSectsInPointNotFoundInMatch-1] = 
                                              pointSectID[k];
               }
            }

            for (k = 0; k < numSectsInPointNotFoundInMatch; k++)
            {
               deltaSect = realloc(deltaSect, (newSect+1) * sizeof(sectorSplit));

               /* Assign dummy matches of first delta Sector. */
               deltaSect[newSect].startNum = deltaSect[0].startNum;
               deltaSect[newSect].endNum = deltaSect[0].endNum;

               /* Assign the correct enumNum corresponding to sector. */
               deltaSect[newSect].enumNum = sectsInPointNotFoundInMatch[k];

               newSect++;
            }
         }
      }
      else /* numMatch = 2. */
      {
         for (i = 0; i < numMatch; i++)
         {
            deltaSect = realloc(deltaSect, (i+1) * sizeof(sectorSplit));
            deltaSect[i].startNum = i;
            deltaSect[i].endNum = i;
            deltaSect[i].enumNum = match[i].f_sector;
         }

         if (i < numSectInPoint)
         {
            for (k = 0; k < numSectInPoint; k++)
            {
               if (k != deltaSect[0].enumNum && k != deltaSect[1].enumNum)
               {
                  deltaSect = realloc(deltaSect, (i+1) * sizeof(sectorSplit));

                  /* Assign just one match. */
                  deltaSect[i].startNum = 0;
                  deltaSect[i].endNum = 0;
                  deltaSect[i].enumNum = k;
                  i++;
                  continue;
               }
            }
         }
         newSect = newSect + i;
      }
            
      /* Fill in the gathered information from above into each point's pntInfo
       * entry. Match the info in the delaSect array with each point's sector
       * info.
       */
      for (j = 0; j < numPnts; j++)
      {
         f_sectorMatch = 0;
         for (k = 0; k < pntInfo[j].numSector; k++)
         {
            if (pntInfo[j].f_sector[k] == nhemi)
                  f_pntInNhemi = 1;               
            for (m = 0; m < newSect; m++)
            {
               if (deltaSect[m].enumNum == pntInfo[j].f_sector[k])
               {
                  pntInfo[j].startNum = deltaSect[m].startNum;

                  /* "endNum" acts as a total (proxies as numMatch), so add 1.*/
                  pntInfo[j].endNum = deltaSect[m].endNum + 1;
                  f_sectorMatch = 1;
                  break;
               }
            }
            if (f_sectorMatch)
               break;
            else
               continue;
         }
      }

      /* Deal with case of a point being in the NHemi sector only, but with 
       * no matches in the match structure existing in the NHemi sector. If 
       * this case is found, set the pntInfo.startNum and pntInfo.endNum to 
       * the first deltaSect (if not, the startNum and endNum are set 
       * erroneously to the entire match structure).
       */
      if (f_pntInNhemi)
      {
         for (j = 0; j < numPnts; j++)
         {
            for (k = 0; k < pntInfo[j].numSector; k++)
            {
               if (pntInfo[j].numSector == 1 && pntInfo[j].f_sector[k] == nhemi
                  && pntInfo[j].startNum == 0 && pntInfo[j].endNum == numMatch)
               {
                  pntInfo[j].startNum = deltaSect[0].startNum;
                  pntInfo[j].endNum = deltaSect[0].endNum + 1;
               }
            }
         }
      }

      /* Free some things. */
      if (f_sectInPointNotFoundInMatch)
         free(sectsInPointNotFoundInMatch);
      if (deltaSect != NULL)
         free(deltaSect);
      if (pointSectID != NULL)
         free(pointSectID);
   }
   if (matchSectID != NULL)
      free(matchSectID);

   return;
}
