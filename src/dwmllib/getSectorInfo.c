/*****************************************************************************
 * getSectorInfo() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Gets the first and last index in the match structure in which matches for 
 *  points in this sector can fall between.
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
 *        numSector = Number of sectors that points were found in (note, nhemi
 *                    not included in the number, even if matches occured in
 *                    nhemi sector). (Input)
 *           sector = Array of sectors that points were found in (note, nhemi
 *                    not included in array, even if matches occured in nhemi
 *                    sector). (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2007 Paul Hershberg (MDL): Created.
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
   int puertori = 1; /* enum number for conus sector. */
   int nhemi = 5; /* enum number for conus sector. */
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

   /* Initialize all points to be able to access entire match structure, as if
    * there were only one point or all points in a multiple call come from one
    * sector.
    */
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
    * Structure is already sorted in the sector order: conus (0), nhemi (5),
    * puertori (1), hawaii (2), guam (3), and alaska (4). 

    * The conus (0) and nhemi (5) sectors can be combined as criteria for 
    * determining where in the match structure a conus point can find matches. 
    * The nhemi (5) and peurtori (1) sectors can be combined as criteria for 
    * determining where in the match structure a peurtori point can find matches. 
    */  
   if ((numSector > 1) && (numNhemi != numMatch))
   {
   /* If numNhemi == numMatch or numSector = 0, then we don't go into this loop 
    * since all matches are from the same sector (nhmemi). No sector adjustments 
    * at all are needed.
    */

      /* If matches exist in the "nhmei" sector, see if there are "conus" or 
       * "peurtori" sectors in this call. 
       */
      if (f_nhemi)
      {
         for (i = 0; i < numSector; i++)
         { 
            if (strcmp(sector[i], "conus") == 0)
               *f_conus = 1;
            if (strcmp(sector[i], "puertori") == 0)
               *f_puertori = 1;
         }
      }

      /* If there are matches in all three sectors, it is a special case. We
       * need to run through the match structure firstly and get this special
       * case.
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

      /* Fill in the gathered information from above into each point's pntInfo
       * entry.
       */
      for (j = 0; j < numPnts; j++)
      {
         for (k = 0; k < pntInfo[j].numSector; k++)
         {
            for (m = 0; m < newSect; m++)
            {
               if (deltaSect[m].enumNum == pntInfo[j].f_sector[k])
               {
                  pntInfo[j].startNum = deltaSect[m].startNum;
                  /* "endNum" acts as a total (proxies as numMatch), so add 1.*/
                  pntInfo[j].endNum = deltaSect[m].endNum + 1;
                  break;
               }
            }
            break;
         }
      }
      free(deltaSect);
   }
   return;
}
