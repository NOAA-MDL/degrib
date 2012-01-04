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
 *        f_npacocn = Denotes at least one match was found in the Northern 
 *                    Pacific sector (sector 6). (Input)
 *       numNpacocn = Number of matches in npacocn sector. If this number is 
 *                    equal to numMatch, all matches are found in one sector 
 *                    (this can occur if there is a call for tropical wind 
 *                    thresholds for a call with hawaii and/or guam points. 
 *                    (Input)
 *          f_nhemi = Denotes at least one match was found in the Northern 
 *                    Hemisphere sector (sector 5). (Input)
 *         numNhemi = Number of matches in nhemi sector. If this number is 
 *                    equal to numMatch, all matches are found in one sector 
 *                    (this can occur if there is a call for tropical wind 
 *                    thresholds for a call with puertori and/or conus points. 
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
 *  2/2008 Paul Hershberg (MDL): Added the flag f_pntInNhemi.
 *  2/2008 Paul Hershberg (MDL): Added code to handle new North Pacific sector.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void getSectorInfo(PntSectInfo *pntInfo, Point *pnts, size_t numPnts,
                   genMatchType *match, size_t numMatch, 
                   size_t numSector, char **sector, int f_conus2_5,
                   int numConus2_5, int f_conus5, int numConus5, int f_nhemi, 
                   int numNhemi, int f_npacocn, int numNpacocn)
{
   int priorMatchCount = 0; /* Counter denoting prior matches when tracking how
                             * many fall into a sector. */
   int i; /* Match counter. */
   int j; /* Point Loop counter. */
   int k; /* pntInfo.numSector counter. */
   int m; /* Counter thru temporary deltaSect array. */
   int conus2_5 = 1; /* enum number for RTMA conus sector. */
   int conus5 = 0; /* enum number for conus sector. */
   int puertori = 2; /* enum number for puertori sector. */
   int nhemi = 6; /* enum number for nhemi sector. */
   int alaska = 5; /* enum number for alaska sector. */
   int guam = 4; /* enum number for guam sector. */
   int hawaii = 3; /* enum number for hawaii sector. */
   int npacocn = 7; /* enum number for north pacific sector. */
   int newSect = 0; /* Denotes the number of sectors skipped if the special case
                     * of there being sector 6 (nhemi) exists. */
   int matchStart = 1; /* Fudge the stating index thru the match structure in 
                        * order to skip the two sectors info has already been 
                        * found for if the special case of there being sector 6 
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
   sectorSplit conusSect; /* Temporary holder of 2_5km and/or 5km res conus sector
                           * information as the structure described above. */
   sectorSplit puertoriSect; /* Temporary holder of puerto rico sector
                              * information as the structure described above. */
   sectorSplit hawaiiSect; /* Temporary holder of hawaii sector information as
                            * the structure described above. */
   sectorSplit guamSect; /* Temporary holder of guam sector information as the 
                          * structure described above. */
/*   sectorSplit alaskaSect; */ /* Temporary holder of alaska sector information as the 
                            * structure described above. */
   int numSectInMatch = 0; /* Total number of sectors for matches returned in
                            * match structure. If in both conus2_5 and conus5, count
                            * as one.
                            */
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
   int f_pntInNpacocn = 0; /* Flag denoting if a point falls in the Npacocn sector 
                            * only, but with no matches in the match structure 
                            * existing in the Npacocn sector. */
   int f_pntInHawaii = 0; /* Flag denoting if a point falls in the Hawaii sector. */
   int f_pntInAlaska = 0; /* Flag denoting if a point falls in the Alaska sector. */
   int f_puertori = 0; /* Flag denoting a match was found in the puertori sector
                        * (sector 2). */
   int f_hawaii = 0; /* Flag denoting a match was found in the hawaii sector
                      * (sector 3). */ 
   int f_guam = 0;   /* Denotes a match was found in the guam sector 
                        (sector 4). */
   int f_alaska = 0;   /* Denotes a match was found in the alaska sector 
                          (sector 5). */
   int f_conus = 0; /* If point falls in either conus2_5 or conus5 sector. */
   int f_conusPuertoriCase = 0; /* Denotes special case where query has info in 
                                 * conus(s), nhemi, and puertori sectors. */
   int f_hawaiiGuamCase = 0; /* Denotes special case where query has info in 
                              * hawaii, npacocn, and guam sectors. */
/*   int f_guamFoundBetweenNpacocnAndAlaska = 0; */
   int f_setToNpacocn = 0;

   /* Initialize all points to be able to access entire match structure, as if
    * there were only one point or all points in a multiple call come from one
    * sector. Also, initialize the conus(s) and puertori SectorSPlit arrays.
    */

/*
   conus2.5kmSect.startNum = 0; 
   conus2.5kmSect.endNum = numMatch;   
   conus2.5kmSect.enumNum = conus2.5; 

   conus5kmSect.startNum = 0;
   conus5kmSect.endNum = numMatch;  
   conus5kmSect.enumNum = conus5;
*/
   conusSect.startNum = 0;
   conusSect.endNum = numMatch;
   conusSect.enumNum = conus5;

   puertoriSect.startNum = 0;
   puertoriSect.endNum = numMatch;  
   puertoriSect.enumNum = puertori;

   hawaiiSect.startNum = 0;
   hawaiiSect.endNum = numMatch;  
   hawaiiSect.enumNum = hawaii;

   guamSect.startNum = 0;
   guamSect.endNum = numMatch;  
   guamSect.enumNum = guam;

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
    * Match structure is already sorted in the sector order (the number in 
    * paranthesis is the original eumeration number set in meta.h, note we need
    * a different order here): conus2_5 (1), conus5 (0), nhemi (6), puertori (2), 
    * hawaii (3), npacocn (7), guam (4), and alaska (5). It needs to be sorted in 
    * this way.
    *
    * The 2 conus sectors (0) & (1) and nhemi (6) sectors can be combined as criteria 
    * for determining where in the match structure a conus point can find matches. 
    * The nhemi (6) and peurtori (2) sectors can be combined as criteria for 
    * determining where in the match structure a peurtori point can find matches. 
    *
    * The hawaii (3) and npacocn (7) sectors can be combined as criteria for 
    * determining where in the match structure a hawaii point can find matches. 
    * The npacocn (7) and guam (4) sectors can be combined as criteria for 
    * determining where in the match structure a guam point can find matches. 
    */
  
   /* Find how many sectors the matches from match structure fell into: */
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

   /* Dummy up case of the 2 conus sectors existing only. If this is case, treat
    * as just one conus sector and leave routine..
    */
   if (f_conus2_5 && f_conus5 && numSectInMatch == 2)
   {
      if (matchSectID != NULL)
         free(matchSectID);
      return;
   }

   /* If numNhemi or numNpacocn == numMatch, or numSectorsInMatch = zero, or the
    * only 2 sectors are conus2_5 and conus5 (which we combined and already left 
    * routine) then we don't go into loop below since all matches are from the same 
    * sector. No sector combinations at all are needed. So return.
    */
   if ((numSectInMatch > 1) && (numNhemi != numMatch) && (numNpacocn != numMatch))
   {
      if (f_conus2_5 || f_conus5)
         f_conus = 1;
      if (f_conus2_5 && f_conus5)
      {

         /* Get Conus(s) sector info */
         for (i = 0; i < numMatch; i++)
         {
            /* Break between conus2_5 and conus5 is final match where conus
             * points can contain data in the match structure.
             */
            if ((match[i].f_sector == conus2_5 || match[i].f_sector == conus5) && (i != numMatch-1))
               priorMatchCount++;
            else if (match[i].f_sector != conus2_5 ||
                     match[i].f_sector != conus5)
            {
               conusSect.startNum = i-priorMatchCount;
               conusSect.endNum = i-1;  
               conusSect.enumNum = match[i-1].f_sector;
               break;
            }
            else if (i == numMatch - 1)
            {
               conusSect.startNum = i-priorMatchCount;
               conusSect.endNum = i;
               conusSect.enumNum = match[i].f_sector;
               break; 
            }
         }
      }

      /* If matches exist in the "nhemi" see if peurtori" sector is in this call. 
       */
      if (f_nhemi)
      {
         for (i = 0; i < numMatch; i++)
         { 
            if (match[i].f_sector == puertori)
            {
               f_puertori = 1;
               break;
            }
         }
      }

      /* If matches exist in the "npacocn" sector, see if there are "hawaii" or 
       * "guam" or "alaska" sectors in this call. 
       */
      if (f_npacocn)
      {
         for (i = 0; i < numMatch; i++)
         { 
            if (match[i].f_sector == hawaii)
               f_hawaii = 1;
            if (match[i].f_sector == guam)
               f_guam = 1;
            if (match[i].f_sector == alaska)
               f_alaska = 1;
         }
      }

      /* Find how many sectors the original points queried for were found in. */
      for (i = 0; i < numSector; i++)
      {
         if (strcmp(sector[i], "conus5") == 0)
         {
            pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
            pointSectID[i] = conus5;
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

      /* "nhemi" and "npacocn and conus2_5 sectors are not counted in variable numSector. So 
       * if f_nhemi = 1 or f_npacocn = 1, add them to the total and rename variable. 
       */
      if (f_nhemi)
      {
         pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
         pointSectID[i] = nhemi;
         numSectInPoint++;
      } 
      if (f_npacocn)
      {
         pointSectID = realloc(pointSectID, (i+1) * sizeof(int));
         pointSectID[i] = npacocn;
         numSectInPoint++;
      }

      /* If there are matches in all 3 sectors (nhemi, conus5 or conus2_5,
       * puertori), it is a special case. We need to run through the match
       * structure firstly and get this special case.
       */
      if (f_nhemi && f_conus && f_puertori)
      {
         /* Get Conus(s) sector info, which will include nhemi sector info. */
         priorMatchCount = 0;
         for (i = 1; i < numMatch; i++)
         {
            priorMatchCount++;
            /* Break between nhemi and puertori is final match where conus 
             * points can contain data in the match structure. 
             */
            if (match[i-1].f_sector == conus5 && match[i].f_sector == nhemi)
            /*  Line above updated Sept 2011, Trop wind Speeds were doubling up when 
                combining conus5 and nhemi sectors. Old line below.... 
            if (match[i-1].f_sector == nhemi && match[i].f_sector == puertori)
             */             
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
               if (i == (numMatch - 1))
               {
                  puertoriSect.startNum = (i-priorMatchCount)+1;
                  puertoriSect.endNum = i;  
                  puertoriSect.enumNum = match[i].f_sector;
               }
               else
               {
                  puertoriSect.startNum = i-priorMatchCount;
                  puertoriSect.endNum = i-1;  
                  puertoriSect.enumNum = match[i-1].f_sector;
               }
               matchStart = i+1; /* Jump into the routine, below, but change the
                                  * starting match structure index to skip the 
                                  * two sectors info has already been found 
                                  * for. */
               break;
            }
         }

         /* Put these 2 sector's information into the deltaSect structure. */
         newSect = 2;
         deltaSect = calloc(newSect, sizeof(sectorSplit));
         deltaSect[0].startNum = conusSect.startNum;
         deltaSect[0].endNum = conusSect.endNum;
         deltaSect[0].enumNum = conusSect.enumNum;

         deltaSect[1].startNum = puertoriSect.startNum;
         deltaSect[1].endNum = puertoriSect.endNum;
         deltaSect[1].enumNum = puertoriSect.enumNum;

         f_conusPuertoriCase = 1;
      }

      /* If there are matches in all three sectors (npacocn, hawaii, guam), it 
       * is a special case. We need to run through the match structure firstly 
       * and get this special case.
       */
      if (f_npacocn && f_hawaii && f_guam)
      {
         /* Get Hawaii sector info, which will include npacocn sector info. */
         priorMatchCount = 0;
         for (i = 1; i < numMatch; i++)
         {
            if (match[i].f_sector == hawaii || match[i].f_sector == npacocn) 
               priorMatchCount++;
            /* Break between npacocn and guam is final match where hawaii 
             * points can contain data in the match structure. 
             */
            if (match[i-1].f_sector == npacocn && match[i].f_sector == guam)
            {
               hawaiiSect.startNum = i-priorMatchCount;
               hawaiiSect.endNum = i-1;  
               hawaiiSect.enumNum = match[i-priorMatchCount].f_sector;
               break;
            }
         }

         /* Get Guam sector info, which will include npacocn sector info. */
         priorMatchCount = 0;
         for (i = 1; i < numMatch; i++)
         {
            if (match[i].f_sector == npacocn || match[i].f_sector == guam)
               priorMatchCount++;
            if (((match[i-1].f_sector != match[i].f_sector) && 
                (match[i-1].f_sector == guam)) ||
                (i == numMatch - 1))
            {
               if (i == (numMatch - 1))
               {
                  guamSect.startNum = (i-priorMatchCount)+1;
                  guamSect.endNum = i;  
                  guamSect.enumNum = match[i].f_sector;
               }
               else
               {
                  guamSect.startNum = i-priorMatchCount;
                  guamSect.endNum = i-1;  
                  guamSect.enumNum = match[i-1].f_sector;
               }
 
               break;
            }
         }

         /* Put these 2 sector's information into the deltaSect structure.
          * Account for the two sectors already found, if applicable. 
          */
         if (f_conusPuertoriCase)
         {
            newSect = 4;
            deltaSect = realloc(deltaSect, newSect * sizeof(sectorSplit));         
            deltaSect[2].startNum = hawaiiSect.startNum;
            deltaSect[2].endNum = hawaiiSect.endNum;
            deltaSect[2].enumNum = hawaiiSect.enumNum;
   
            deltaSect[3].startNum = guamSect.startNum;
            deltaSect[3].endNum = guamSect.endNum;
            deltaSect[3].enumNum = guamSect.enumNum;
         }
         else
         {
            newSect = 2;
            deltaSect = calloc(newSect, sizeof(sectorSplit)); 
            deltaSect[0].startNum = hawaiiSect.startNum;
            deltaSect[0].endNum = hawaiiSect.endNum;
            deltaSect[0].enumNum = hawaiiSect.enumNum;
   
            deltaSect[1].startNum = guamSect.startNum;
            deltaSect[1].endNum = guamSect.endNum;
            deltaSect[1].enumNum = guamSect.enumNum;
         }
         f_hawaiiGuamCase = 1;
      }

#ifdef ALASKAHASTPCWINDS
      /* If there are matches in the sectors npacocn and alaska, it is a 
       * special case. We need to run through the match structure
       * and get this special case.
       */
      if (f_npacocn && f_alaska)
      {
         /* Get Alaska sector info, which may include guam sector info, and 
          * will include npacocn sector info. 
          */
         priorMatchCount = 0;
         for (i = 1; i < numMatch; i++)
         {
            if (match[i].f_sector == npacocn || match[i].f_sector == guam || match[i].f_sector == alaska) 
               priorMatchCount++;
            if (match[i].f_sector == guam && !f_guamFoundBetweenNpacocnAndAlaska)
               f_guamFoundBetweenNpacocnAndAlaska = 1;

            /* Alaskan matches, if they occur, will be at the very end of the 
             * match structure. 
             */
            if (match[i-1].f_sector == alaska && i == (numMatch-1))
            {
               alaskaSect.startNum = (i-priorMatchCount)+1;
               alaskaSect.endNum = i;  
               alaskaSect.enumNum = match[i].f_sector;
               break;
            }
         }

         /* Put this sector's information into the deltaSect structure.
          * Account for the two (or four) sectors already found, if applicable. 
          */
         if (f_conusPuertoriCase && f_hawaiiGuamCase)
         {
            newSect = 5;
            deltaSect = realloc(deltaSect, newSect * sizeof(sectorSplit));         
            deltaSect[4].startNum = alaskaSect.startNum;
            deltaSect[4].endNum = alaskaSect.endNum;
            deltaSect[4].enumNum = alaskaSect.enumNum;
         }
         else if ((f_conusPuertoriCase && !f_hawaiiGuamCase) || (!f_conusPuertoriCase && f_hawaiiGuamCase))
         {
            newSect = 3;
            deltaSect = realloc(deltaSect, newSect * sizeof(sectorSplit)); 
            deltaSect[2].startNum = alaskaSect.startNum;
            deltaSect[2].endNum = alaskaSect.endNum;
            deltaSect[2].enumNum = alaskaSect.enumNum;
         }
         else
         {
            newSect = 1;
            deltaSect = calloc(newSect, sizeof(sectorSplit)); 
            deltaSect[0].startNum = alaskaSect.startNum;
            deltaSect[0].endNum = alaskaSect.endNum;
            deltaSect[0].enumNum = alaskaSect.enumNum;
         }
      }
      /* Will replace Conditional statement below. */
      if (((match[i-1].f_sector != match[i].f_sector) &&
          !(match[i-1].f_sector == conus5 && match[i].f_sector == nhemi) &&
          !(match[i-1].f_sector == nhemi && match[i].f_sector == puertori) &&
          !(match[i-1].f_sector == hawaii && match[i].f_sector == npacocn) && 
          !(match[i-1].f_sector == npacocn && match[i].f_sector == guam) &&
          !(match[i-1].f_sector == npacocn && match[i].f_sector == alaska) &&
          !(match[i-1].f_sector == guam && match[i].f_sector == alaska && 
          f_guamFoundBetweenNpacocnAndAlaska)) || (i == numMatch - 1))
#endif

      if (numMatch != 2)
      {
         priorMatchCount = 0;
         for (i = matchStart; i < numMatch; i++)
         {
            priorMatchCount++;
            /* Skip over special cases of combined sectors. */
            if (((match[i-1].f_sector != match[i].f_sector) &&
                !(match[i-1].f_sector == conus2_5 && match[i].f_sector == conus5) &&
/* Commented out this line as nhemi and conus 5, when combined, the Tropical Wind 
   Speed data was doubling up. Sept 2011. This goes along with commented out line above.
                !(match[i-1].f_sector == conus5 && match[i].f_sector == nhemi) &&
*/
                !(match[i-1].f_sector == nhemi && match[i].f_sector == puertori) &&
                !(match[i-1].f_sector == hawaii && match[i].f_sector == npacocn) && 
                !(match[i-1].f_sector == npacocn && match[i].f_sector == guam)) ||
                (i == numMatch - 1))
            {
               deltaSect = realloc(deltaSect, (newSect+1) * sizeof(sectorSplit));
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
          * were found in the original point query. This is only applicable if
          * there are points with "normal" elements (maxt, pop12) that are found
          * in sectors other than the N Pacific or Nhemi sectors. If the query 
          * only contains matches for the Tropical Wind Threshold elements, matches
          * will only be found in N Pacific and/or Nhemi sectors. Thus, don't go
          * into this if statement below.
          */
         if (!(f_nhemi && f_npacocn && !f_conus && !f_puertori && !f_hawaii && 
              !f_guam && !f_alaska))
         {
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
            if (pntInfo[j].f_sector[k] == npacocn)
                  f_pntInNpacocn = 1;   
            if (pntInfo[j].f_sector[k] == hawaii)
                  f_pntInHawaii = 1;               
            if (pntInfo[j].f_sector[k] == alaska)
                  f_pntInAlaska = 1;

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
                  for (m = 0; m < newSect; m++)
                  {
                     if (deltaSect[m].enumNum == conus5)
                     {
                        pntInfo[j].startNum = deltaSect[m].startNum;
                        pntInfo[j].endNum = deltaSect[m].endNum + 1;
                     }
                  }
               }
            }
         }
      }

      /* Deal with case of a point being in the Npacocn sector only, but with 
       * no matches in the match structure existing in the Npacocn sector. If 
       * this case is found, set the pntInfo.startNum and pntInfo.endNum to 
       * the hawaii and guam sector info (if not, the startNum and endNum
       * are set erroneously to the entire match structure).
       */
      if (f_pntInNpacocn)
      {
         for (j = 0; j < numPnts; j++)
         {
            for (k = 0; k < pntInfo[j].numSector; k++)
            {
               if (pntInfo[j].numSector == 1 && pntInfo[j].f_sector[k] == npacocn
                  && pntInfo[j].startNum == 0 && pntInfo[j].endNum == numMatch)
               {
                  priorMatchCount = 0;
                  for (i = 1; i < numMatch; i++)
                  {
                     if (match[i].f_sector == npacocn)
                        priorMatchCount++;
                     if (((match[i-1].f_sector != match[i].f_sector) && 
                         (match[i-1].f_sector == npacocn)) ||
                         (i == (numMatch-1)))
                     {
                        if (i == (numMatch - 1))
                        {
                           pntInfo[j].startNum = (i-priorMatchCount)+1;
                           pntInfo[j].endNum = i+1;  
                        }
                        else
                        {
                           pntInfo[j].startNum = i-priorMatchCount;
                           pntInfo[j].endNum = i;  
                        }
                        break;
                     }
                  }
               }
            }
         }
      }

      /* Deal with case of a point being in the Alaska sector only, but with 
       * no matches in the match structure existing in the Alaska sector. If 
       * this case is found, set the pntInfo.startNum and pntInfo.endNum to 
       * the npacocn sector info or conus sector info (if not, the startNum and
       * endNum are set erroneously to the entire match structure).
       */
      if (f_pntInAlaska)
      {
         for (j = 0; j < numPnts; j++)
         {
            for (k = 0; k < pntInfo[j].numSector; k++)
            {
               if (pntInfo[j].numSector == 1 && pntInfo[j].f_sector[k] == alaska
                  && pntInfo[j].startNum == 0 && pntInfo[j].endNum == numMatch)
               {
                  priorMatchCount = 0;
                  for (i = 1; i < numMatch; i++)
                  {
                     if (match[i].f_sector == npacocn)
                        priorMatchCount++;
                     if (((match[i-1].f_sector != match[i].f_sector) && 
                         (match[i-1].f_sector == npacocn)) ||
                         (i == (numMatch-1)))
                     {
                        f_setToNpacocn = 1;
                        if (i == (numMatch - 1))
                        {
                           pntInfo[j].startNum = (i-priorMatchCount)+1;
                           pntInfo[j].endNum = i+1;  
                        }
                        else
                        {
                           pntInfo[j].startNum = i-priorMatchCount;
                           pntInfo[j].endNum = i;  
                        }
                        break;
                     }
                  }
                  if (!f_setToNpacocn) /* Set to first deltaSect. */
                  {
                     pntInfo[j].startNum = deltaSect[0].startNum;
                     pntInfo[j].endNum = deltaSect[0].endNum + 1;
                  }
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
