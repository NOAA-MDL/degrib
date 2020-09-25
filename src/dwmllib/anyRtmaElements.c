/*****************************************************************************
 * anyRtmaElements() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Checks to see if this is an RTMA query only (f_XML will be set to 5) or
 *   a mix of both RTMA elements and NDFD elements (f_XML will be set to 6).
 *   This check can only occur if incoming f_XML type is 1 (time-series), 
 *   which can encompass both f_XML = 5 or 6. If query does contain RTMA
 *   elements, we add the necessary input files to array inFiles and increase
 *   numInFiles accordingly. 
 *   
 * ARGUMENTS
 *       f_XML = flag for 1 of the 6 DWML products (Output):
 *               1 = DWMLgen's NDFD "time-series" product. 
 *               2 = DWMLgen's NDFD "glance" product.
 *               3 = DWMLgenByDay's "12 hourly" format product.
 *               4 = DWMLgenByDay's "24 hourly" format product.
 *               5 = DWMLgen's RTMA "time-series" product.
 *               6 = DWMLgen's RTMA + NDFD "time-series" product.
 *  numInFiles = Number of input files. (Output)
 *     InFiles = Pointer to character array holding the input files holding data
 *               to be formatted. (Output)
 * numNdfdVars = Number of NDFD and/or RTMA elements chosen on the command line
 *               arg to format. (Input)
 *    ndfdVars = Array holding the enum numbers of the ndfd/rtma elements chosen
 *               on the command line arg to format. (Input)
 * rtmaDataDir = Directory where RTMA data is located.  If not provided by the
 *               user, userparse.c has set this to the default value (if
 *               possible) (Input)
 *      f_icon = Flag denoting whether NDFD element derived icons are to be
 *               formatted per point. If this flag is chosen, the other 4 NDFD
 *               elements' data used to derive the icons must be
 *               retrieved/derived too (WS, SKY, TEMP, WX). (Input)
 * f_rtmaNdfdTemp = Flag denoting that user queried both NDFD Temp and RTMA 
 *                  Temp. Thus, the two will be conjoined into one element 
 *                  (Output).
 *   f_rtmaNdfdTd = Flag denoting that user queried both NDFD DewPoint & RTMA
 *                  Dew Point. Thus, the two will be conjoined into one element 
 *                  (Output).
 * f_rtmaNdfdWdir = Flag denoting that user queried both NDFD Wind Dir and RTMA
 *                  Wind Dir. Thus, the two will be conjoined into one element 
 *                  (Output).
 * f_rtmaNdfdWspd = Flag denoting that user queried both NDFD Wind Spd and RTMA
 *                  Wind Spd. Thus, the two will be conjoined into one element.
 *   f_rtmaNdfdPrecipa = Flag denoting that user queried both NDFD QPF & RTMA
 *                       Precipitation Amount. Thus, the two will be conjoined
 *                       into one element.
 * f_rtmaNdfdSky = Flag denoting that user queried both NDFD Sky Cover and RTMA
 *                 Sky Cover. Thus, the two will be conjoined into one element
 *                 (Output).
 *      
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  12/2007 Paul Hershberg (MDL): Created.
 *   1/2008 Paul Hershberg (MDL): Added Alaska RTMA directory info
 *   2/2008 Paul Hershberg (MDL): Added flags denoting if query includes the 
 *                                combined RTMA + NDFD concatenaed elements.
 *   3/2008 Paul Hershberg (MDL): Create directories in subsequent routine 
 *                                rtmaFileNames()
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void anyRtmaElements(uChar *f_XML, size_t *numInFiles, char ***inFiles,
                     size_t numNdfdVars, uChar *ndfdVars, char *rtmaDataDir, 
                     int f_icon, size_t numSector, char **sector, 
                     int *f_rtmaNdfdTemp, int *f_rtmaNdfdTd, int *f_rtmaNdfdWdir,
                     int *f_rtmaNdfdWspd,int *f_rtmaNdfdPrecipa,int *f_rtmaNdfdSky)
{
   int i;              /* Loop Counter. */
   int numRtmaElem = 0;   /* Number of rtma elements in query. */
   int f_rtmaTemp = 0; /* Flag denoting if RTMA temp was queried for. */
   int f_rtmaTd  = 0;  /* Flag denoting if RTMA dew point was queried for. */
   int f_rtmaWdir = 0; /* Flag denoting if RTMA wind dir was queried for. */
   int f_rtmaWspd = 0; /* Flag denoting if RTMA wind spd was queried for. */
   int f_rtmaPrecipa = 0; /* Flag denoting if RTMA precip amt was queried 
                           * for. */
   int f_rtmaSky = 0;  /* Flag denoting if RTMA sky cover was queried for. */
   int f_ndfdTemp = 0; /* Flag denoting if NDFD temp was queried for. */
   int f_ndfdTd  = 0;  /* Flag denoting if NDFD dew point was queried for. */
   int f_ndfdWdir = 0; /* Flag denoting if NDFD wind dir was queried for. */
   int f_ndfdWspd = 0; /* Flag denoting if NDFD wind spd was queried for. */
   int f_ndfdQpf = 0;  /* Flag denoting if NDFD precip amt was queried 
                          for. */
   int f_ndfdSky = 0;  /* Flag denoting if NDFD sky cover was queried for. */
   int f_rtmaInConus = 0;  /*Flag denoting if RTMA data in this query was found
                            * for points in the conus sector. */
   int f_rtmaInAK = 0; /*Flag denoting if RTMA data in this query was found
                        * for points in the Alaskan sector. */
   int f_rtmaInPR = 0; /*Flag denoting if RTMA data in this query was found
                        * for points in the Puerto Rican sector. */
   int f_rtmaInGM = 0; /*Flag denoting if RTMA data in this query was found
                        * for points in the Guam sector. */
   int f_rtmaInHW = 0; /*Flag denoting if RTMA data in this query was found
                        * for points in the Hawaii sector. */
/* Following 2 variables were moved to userparse.c 3/18/2008 */
/*   char rtmaSetDir[] = "/www/ndfd/public/database/cube/rtma"; */ /* Default
                       * directory RTMA data is found. Used if -rtmaDir was not
                       * entered as a command line argument. */
/*   char perm; */         /* The permissions on the rtmaSetDir folder */

   /* If numNdfdVars == 0, then -ndfdVars wasn't entered on the command line.
    * Then format all elements (both RTMA & NDFD; set f_XML = 6).
    */
   if (numNdfdVars != 0)
   {
      for (i = 0; i < numNdfdVars; i++)
      {
         if (ndfdVars[i] == NDFD_TEMP)
         {
            f_ndfdTemp = 1;
            continue;
         }
         if (ndfdVars[i] == RTMA_TEMP)
         {
            f_rtmaTemp = 1;
            numRtmaElem++;
            continue;
         }
         if (ndfdVars[i] == NDFD_TD)
         {
            f_ndfdTd = 1;
            continue;
         }
         if (ndfdVars[i] == RTMA_TD)
         {
            f_rtmaTd = 1;
            numRtmaElem++;
            continue;
         }
         if (ndfdVars[i] == NDFD_WD)
         {
            f_ndfdWdir = 1;
            continue;
         }
         if (ndfdVars[i] == RTMA_WDIR)
         {
            f_rtmaWdir = 1;
            numRtmaElem++;
            continue;
         }
         if (ndfdVars[i] == NDFD_WS)
         {
            f_ndfdWspd = 1;
            continue;
         }
         if (ndfdVars[i] == RTMA_WSPD)
         {
            f_rtmaWspd = 1;
            numRtmaElem++;
            continue;
         }
         if (ndfdVars[i] == NDFD_QPF)
         {
            f_ndfdQpf = 1;
            continue;
         }
         if (ndfdVars[i] == RTMA_PRECIPA)
         {
            f_rtmaPrecipa = 1;
            numRtmaElem++;
            continue;
         }
         if (ndfdVars[i] == NDFD_SKY)
         {
            f_ndfdSky = 1;
            continue;
         }
         if (ndfdVars[i] == RTMA_SKY)
         {
            f_rtmaSky = 1;
            numRtmaElem++;
            continue;
         }
      }

      /* We at least have some RTMA elements. See if query is exclusively
       * made up of RTMA elements (f_XML = 5) or a mix (f_XML = 6).
       */
      if ((numNdfdVars == numRtmaElem) && (!f_icon))
         *f_XML = 5;
      else if (numRtmaElem != 0)
         *f_XML = 6;
   }
   else if (numNdfdVars == 0 && !f_icon)
   {
      *f_XML = 6;
      f_rtmaTemp = 1;
      f_rtmaTd = 1;
      f_rtmaWdir = 1;
      f_rtmaWspd = 1;
      f_rtmaPrecipa = 1;
      f_rtmaSky = 1;
      f_ndfdTemp = 1;
      f_ndfdTd = 1;
      f_ndfdWdir = 1;                    
      f_ndfdWspd = 1;
      f_ndfdQpf = 1;
      f_ndfdSky = 1;
   }

   /* If this an exclusive RTMA query, or a query with a mix of both RTMA
    * and NDFD elements, add the RTMA files to the inFiles array (which will
    * already hold NDFD files if the query contains both RTMA and NDFD
    * elements.)
    */
   if (*f_XML != 1) /* At this point, f_XML originally = 1 is = to 5 or 6. */
   {

      /* If rtmaDataDir is not entered as a command line argument (= NULL), but
       * is needed, set it to default value.
       */
/* Moved logic for rtmaSetDir to userparse.c 3/18/2008
      if (rtmaDataDir != NULL)
         strcpy (rtmaSetDir, rtmaDataDir);
      if (myStat (rtmaSetDir, &perm, NULL, NULL) != MYSTAT_ISDIR)
         return;
*/
      /* 3/18/2008 -- added check to see if rtmaDataDir is null, if so, both
       * the user supplied and default values for -rtmaDir are not directories
       * so abort */
      if (rtmaDataDir == NULL)
         return;

      /* See which sector(s), the RTMA data was queried for. */
      for (i = 0; i < numSector; i++)
      {
         if ((strcmp(sector[i], "conus2_5") == 0) || (strcmp(sector[i], "conus5") == 0))
         {
            f_rtmaInConus = 1;
            continue;
         }
         if (strcmp(sector[i], "alaska") == 0)
         {
            f_rtmaInAK = 1;
            continue;
         }
         if (strcmp(sector[i], "puertori") == 0)
         {
            f_rtmaInPR = 1;
            continue;
         }
         if (strcmp(sector[i], "guam") == 0)
         {
            f_rtmaInGM = 1;
            continue;
         }
         if (strcmp(sector[i], "hawaii") == 0)
         {
            f_rtmaInHW = 1;
            continue;
         }
      }

      /**********************RTMA TEMPERATURE*********************************/
      /***********************************************************************/

      /* If RTMA Temperature was in this query, create all data 
       * files/directories for both the Parent Temp and the associated 
       * uncertainty for all sectors in this query. 
       */
      if (f_rtmaTemp)
      {
         if (f_rtmaInConus) 
         {
            rtmaFileNames(numInFiles, inFiles, "/conus/temp_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/conus/utemp_r", rtmaDataDir);
         }
         if (f_rtmaInAK)
         {
            rtmaFileNames(numInFiles, inFiles, "/alaska/temp_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/alaska/utemp_r", rtmaDataDir);
         }
         if (f_rtmaInPR) 
         {
            rtmaFileNames(numInFiles, inFiles, "/puertori/temp_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/puertori/temp_r", rtmaDataDir);
         }
         if (f_rtmaInGM)
         {
            rtmaFileNames(numInFiles, inFiles, "/guam/temp_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/guam/utemp_r", rtmaDataDir);
         }
         if (f_rtmaInHW)
         {
            rtmaFileNames(numInFiles, inFiles, "/hawaii/temp_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/hawaii/utemp_r", rtmaDataDir);
         }
      }

      /**********************RTMA DEW POINTS**********************************/
      /***********************************************************************/

      /* If RTMA Dew Point was in this query, create all data 
       * files/directories for both the Parent Dew Point and the associated 
       * uncertainty for all sectors in this query. 
       */
      if (f_rtmaTd)
      {
         if (f_rtmaInConus) 
         {
            rtmaFileNames(numInFiles, inFiles, "/conus/td_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/conus/utd_r", rtmaDataDir);
         }
         if (f_rtmaInAK)
         {
            rtmaFileNames(numInFiles, inFiles, "/alaska/td_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/alaska/utd_r", rtmaDataDir);
         }
         if (f_rtmaInPR) 
         {
            rtmaFileNames(numInFiles, inFiles, "/puertori/td_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/puertori/utd_r", rtmaDataDir);
         }
         if (f_rtmaInGM)
         {
            rtmaFileNames(numInFiles, inFiles, "/guam/td_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/guam/utd_r", rtmaDataDir);
         }
         if (f_rtmaInHW)
         {
            rtmaFileNames(numInFiles, inFiles, "/hawaii/td_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/hawaii/utd_r", rtmaDataDir);
         }
      }

      /**********************RTMA WIND DIRECTIONS*****************************/
      /***********************************************************************/

      /* If RTMA Wind Direction was in this query, create all data 
       * files/directories for both the Parent Wind Direction and the associated 
       * uncertainty for all sectors in this query. 
       */
      if (f_rtmaWdir)
      {
         if (f_rtmaInConus) 
         {
            rtmaFileNames(numInFiles, inFiles, "/conus/wdir_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/conus/uwdir_r", rtmaDataDir);
         }
         if (f_rtmaInAK)
         {
            rtmaFileNames(numInFiles, inFiles, "/alaska/wdir_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/alaska/uwdir_r", rtmaDataDir);
         }
         if (f_rtmaInPR) 
         {
            rtmaFileNames(numInFiles, inFiles, "/puertori/wdir_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/puertori/uwdir_r", rtmaDataDir);
         }
         if (f_rtmaInGM)
         {
            rtmaFileNames(numInFiles, inFiles, "/guam/wdir_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/guam/uwdir_r", rtmaDataDir);
         }
         if (f_rtmaInHW)
         {
            rtmaFileNames(numInFiles, inFiles, "/hawaii/wdir_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/hawaii/uwdir_r", rtmaDataDir);
         }
      }

      /*************************RTMA WIND SPEEDS******************************/
      /***********************************************************************/

      /* If RTMA Wind Speed was in this query, create all data 
       * files/directories for both the Parent Wind Speed and the associated 
       * uncertainty for all sectors in this query. 
       */
      if (f_rtmaWspd)
      {
         if (f_rtmaInConus) 
         {
            rtmaFileNames(numInFiles, inFiles, "/conus/wspd_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/conus/uwspd_r", rtmaDataDir);
         }
         if (f_rtmaInAK)
         {
            rtmaFileNames(numInFiles, inFiles, "/alaska/wspd_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/alaska/uwspd_r", rtmaDataDir);
         }
         if (f_rtmaInPR) 
         {
            rtmaFileNames(numInFiles, inFiles, "/puertori/wspd_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/puertori/uwspd_r", rtmaDataDir);
         }
         if (f_rtmaInGM)
         {
            rtmaFileNames(numInFiles, inFiles, "/guam/wspd_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/guam/uwspd_r", rtmaDataDir);
         }
         if (f_rtmaInHW)
         {
            rtmaFileNames(numInFiles, inFiles, "/hawaii/wspd_r", rtmaDataDir);
            rtmaFileNames(numInFiles, inFiles, "/hawaii/uwspd_r", rtmaDataDir);
         }
      }

      /**********************RTMA PRECIPITATION AMOUNTS***********************/
      /***********************************************************************/

      /* If RTMA Precipitation Amount was in this query, create all data 
       * files/directories for all sectors in this query. 
       */
      if (f_rtmaPrecipa)
      {
         if (f_rtmaInConus) 
            rtmaFileNames(numInFiles, inFiles, "/conus/precipa_r", rtmaDataDir);
         if (f_rtmaInAK)
            rtmaFileNames(numInFiles, inFiles, "/alaska/precipa_r", rtmaDataDir);
         if (f_rtmaInPR) 
            rtmaFileNames(numInFiles, inFiles, "/puertori/precipa_r", rtmaDataDir);
         if (f_rtmaInGM)
            rtmaFileNames(numInFiles, inFiles, "/guam/precipa_r", rtmaDataDir);
         if (f_rtmaInHW)
            rtmaFileNames(numInFiles, inFiles, "/hawaii/precipa_r", rtmaDataDir);
      }

      /**************************RTMA SKY COVER*******************************/
      /***********************************************************************/

      /* If RTMA Sky Cover was in this query, create all data 
       * files/directories for all sectors in this query. 
       */
      if (f_rtmaSky)
      {
         if (f_rtmaInConus) 
            rtmaFileNames(numInFiles, inFiles, "/conus/sky_r", rtmaDataDir);
         if (f_rtmaInAK)
            rtmaFileNames(numInFiles, inFiles, "/alaska/sky_r", rtmaDataDir);
         if (f_rtmaInPR) 
            rtmaFileNames(numInFiles, inFiles, "/puertori/sky_r", rtmaDataDir);
         if (f_rtmaInGM)
            rtmaFileNames(numInFiles, inFiles, "/guam/sky_r", rtmaDataDir);
         if (f_rtmaInHW)
            rtmaFileNames(numInFiles, inFiles, "/hawaii/sky_r", rtmaDataDir);
      }

      /* Turn on flags if concatenation of RTMA + NDFD element is warranted. */
      if (f_rtmaTemp && f_ndfdTemp)
         *f_rtmaNdfdTemp = 1;
      if (f_rtmaTd && f_ndfdTd)
         *f_rtmaNdfdTd = 1;
      if (f_rtmaWspd && f_ndfdWspd)
         *f_rtmaNdfdWspd = 1;
      if (f_rtmaWdir && f_ndfdWdir)
         *f_rtmaNdfdWdir = 1;
      if (f_rtmaPrecipa && f_ndfdQpf)
         *f_rtmaNdfdPrecipa = 1;
      if (f_rtmaSky && f_ndfdSky)
         *f_rtmaNdfdSky = 1;
   }

   return;
}
