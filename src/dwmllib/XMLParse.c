/******************************************************************************/
/* XMLParse () --
 * 
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   The driver program that ultimately formats the DWMLgen products 
 *   "time-series" and "glance" and the DWMLgenByDay products (formats)
 *   "12 hourly" and 24 hourly". This compiled C code will mirror the php coded
 *   web service in which requests for NDFD data over the internet are sent back 
 *   in DWML format.
 * 
 * ARGUMENTS 
 *       f_XML = flag for 1 of the 4 DWML products (Input):
 *               1 = DWMLgen's "time-series" product. 
 *               2 = DWMLgen's "glance" product.
 *               3 = DWMLgenByDay's "12 hourly" format product.
 *               4 = DWMLgenByDay's "24 hourly" format product.
 *     numPnts = Number of points to probe for. (Input) 
 *        pnts = A pointer to the points to probe for (defined in type.h).
 *               (Input)
 *     pntInfo = A pointer to each points' timezone, DST, sector, and 
 *               starting match # and ending match # point's data can be
 *               found in the match structure. (defined in sector.h). 
 *               (Intput)
 *   f_pntType = 0 => pntX, pntY are lat/lon, 1 => they are X,Y. (Input)
 *      labels = Pointer to character array holding Station Names for each 
 *               point. (Input)
 *  numInFiles = Number of input files. (Input)
 *     InFiles = Pointer to character array holding the input files. (Input)
 *  f_fileType = Type of input files. (0=GRIB, 1=Data Cube index file) (Input)
 *    f_interp = true => bi-linear, false => nearest neighbor (Input)
 *      f_unit = 0 (GRIB unit), 1 (english), 2 (metric) (Input)
 *    majEarth = Used to override the TDLP major axis of earth. (Input)
 *    minEarth = Used to override the TDLP minor axis of earth. (Input)
 *      f_icon = Flag denoting whether icons are to be derived and formatted
 *               per point. If this flag is chosen, the other 4 elements' data 
 *               used to derive the icons must be retrieved/derived too (WS, SKY, 
 *               TEMP, WX). (Input)
 * f_SimpleVer = Version of the simple NDFD Weather table to use. (Input)
 *   f_valTime = 0 = false, 1 = f_validStartTime, 2 = f_validEndTime,
 *               3 = both f_validStartTime, and f_validEndTime (Input)
 *   startTime = First valid time that we are interested in. If this variable 
 *               exists as an input, it was set by the user. (Input)
 *     endTime = Last valid time that we are interested in.i If this variable 
 *               exists as an input, it was set by the user. (Input)
 * numNdfdVars = Number of user selected ndfdVars. (Input)
 *    ndfdVars = Pointer to the user selected NDFD variables. (Input)      
 *   numSector = Number of sectors that points were found in. (Input)
 *
 * FILES/DATABASES: None
 *              
 * RETURNS: int
 *         
 * HISTORY
 *  12/2005 Arthur Taylor/Paul Hershberg (MDL): Created.
 *   5/2006 Bailing Li, Arthur Taylor, John Schattel: Code Review 1.
 *  10/2007 Paul Hershberg (MDL): Removed code that shifted data back by 1/2
 *                                the period length (bug from php code)
 *
 * NOTES: The NDFD element list is below.
 *
 * enum { NDFD_MAX, NDFD_MIN, NDFD_POP, NDFD_TEMP, NDFD_WD, NDFD_WS,
 *        NDFD_TD, NDFD_SKY, NDFD_QPF, NDFD_SNOW, NDFD_WX, NDFD_WH,
 *        NDFD_AT, NDFD_RH, NDFD_WG, NDFD_INC34, NDFD_INC50, NDFD_INC64,
 *        NDFD_CUM34, NDFD_CUM50, NDFD_CUM64, NDFD_CONHAZ, NDFD_PTORN,
 *        NDFD_PHAIL, NDFD_PTSTMWIND, NDFD_PXTORN, NDFD_PXHAIL, NDFD_PXTSTMWIND,
 *        NDFD_PSTORM, NDFD_PXSTORM, NDFD_TMPABV14D, NDFD_TMPBLW14D, 
 *        NDFD_PRCPABV14D, NDFD_PRCPBLW14D, NDFD_TMPABV30D, NDFD_TMPBLW30D,
 *        NDFD_PRCPABV30D, NDFD_PRCPBLW30D, NDFD_TMPABV90D, NDFD_TMPBLW90D, 
 *        NDFD_PRCPABV90D, NDFD_PRCPBLW90D, NDFD_UNDEF, NDFD_MATCHALL
 *      };
 * 
 ****************************************************************************** 
 */
#include "xmlparse.h"
int XMLParse(uChar f_XML, size_t numPnts, Point * pnts,
             PntSectInfo * pntInfo, sChar f_pntType, char **labels,
             size_t numInFiles, char **inFiles, uChar f_fileType,
             sChar f_interp, sChar f_unit, double majEarth, double minEarth,
             sChar f_icon, sChar f_SimpleVer, sChar f_valTime,
             double startTime, double endTime, size_t numNdfdVars,
             uChar * ndfdVars, char *f_inTypes, char *gribFilter,
             size_t numSector, char **sector, sChar f_ndfdConven)
{
   size_t numElem = 0;        /* Num of elements returned by degrib. */
   genElemDescript *elem = NULL;  /* Structure with info about the element. */
   uChar varFilter[NDFD_MATCHALL + 1];  /* Shows what NDFD variables are of
                                         * interest(= 1) or vital(= 2). */
   size_t numMatch = 0;       /* The number of matches from degrib. */
   genMatchType *match = NULL;  /* Structure of element matches returned from 
                                 * degrib. */
   int i;                     /* Generally used as the element incrementer. */
   size_t j;                  /* Generally used as the point incrementer. */
   size_t k;                  /* Generally used as the time-layout
                               * incrementer. */
   collateType *collate = NULL; /* Used to get elements per each validTime. */
   size_t numCollate = 0;     /* Number of collate members. */
   char *f_pntHasData;        /* Flag used to check if a point has any valid
                               * data. */
   double curTime;            /* Temporary storage for validTimes. */
   int *numDays = NULL; /* The number of days the validTimes for all the data
                         * rows (values) consist of, per point. Used for the
                         * DWMLgenByDay products only. */
   int startNum;        /* First index in match structure an individual point's
                         * data matches can be found. */
   int endNum;          /* Last index in match structure an individual point's
                         * data matches can be found. */
   int numPopLines = 0; /* Since Pop, in the 24 hourly (f_MXL = 4) product still has 
		         * formatted data every 12 hours, we use this variable and 
		         * set it to numDays *2 when calling generateTimeLayout().
		         */
   int format_value = 1; /* Option to turn off the formating of the value child
                          * element for weather. (TPEX sets to zero, as they 
                          * don't format this). */
   uChar **weatherParameters = NULL;            /* Array containing the flags
                                                 * denoting whether a certain 
                                                 * element is formatted in
                                                 * the output XML (=1), or
                                                 * used in deriving another
                                                 * formatted element (=2). */
   int *f_formatIconForPnt = NULL; /* Determines wether there is enough data to 
                                      icons for the particular point being 
                                      processed. */
   int *f_formatSummarizations = NULL; /*  Flag denoting if all 7 elements used 
                                           in deriving the phrase/icon for the 
                                           summarization products (f_XML = 3 or 
                                           4) are available. Flag denotes if an
                                           element has all missing data (does 
                                           not test for missing data per 
                                           projection). */
   sChar *TZoffset = NULL;    /* Number of hours to add to current time to
                               * get GMT time. */
   char **startDate = NULL;   /* Point specific user supplied start Date
                               * (i.e. 2006-04-15). */
   int f_atLeastOnePointInSectors = 0;  /* Flag used to denote if at least
                                         * one point is in any of NDFD
                                         * Sectors. */
   numRowsInfo **numRowsForPoint; /* Structure containing members:
                     * total: Total number of rows data is formatted for in the 
                     *      output XML. Used in DWMLgenByDay's "12 hourly" and 
                     *      "24 hourly" products. "numRows" is determined 
                     *      using numDays and is used as an added criteria
                     *      (above and beyond simply having data exist for a 
                     *       certain row) in formatting XML for these two 
                     *       products. (Input)
                     * skipBeg: the number of beginning rows not formatted due 
                     *       to a user supplied reduction in time (startTime
                     *       arg is not = 0.0)
                     * skipEnd: the number of end rows not formatted due to a 
                     *       user supplied reduction in time (endTime arg
                     *       is not = 0.0)
                     * firstUserTime: the first valid time interested per element, 
                     *       taking into consideration any data values 
                     *       (rows) skipped at beginning of time duration.
                     * lastUserTime: the last valid time interested per element, 
                     *        taking into consideration any data values 
                     *        (rows) skipped at end of time duration. */
   uChar f_formatPeriodName = 0;  /* Flag to indicate if period names (i.e.
                                   * "today") appear in the start valid time
                                   * tag: <start-valid-time
                                   * period-name="today">. */
   int f_firstPointLoopIteration = 1; /* In the point loop for the time layout
				         generations, this flag denotes the 
					 first time loop is entered. */
   int timeInterval;          /* Number of seconds in either a 12 hourly
                               * format 3600*12 or 24 hourly format 3600*24. */
   int *numOutputLines = NULL;  /* The number of data values output, per point.
                                 * For the "24 hourly" format, numDays equals
                                 * numOutputLines. For the "12 hourly" format,
                                 * numOutputLines is equal to twice the number
                                 * of days since there are two 12 hour period. */
   int integerTime = 0; /* Number of seconds since 1970 to when the data is 
			 * valid. Allows the code to know if this data belongs 
			 * in the current period being processed. */ 
   const char whichTimeCoordinate[] = "local";  /* Flag indicating which time
                                                 * coordinate we are to format
                                                 * the data in (local is the
                                                 * only coordinate currently
                                                 * supported). */
   char whatSummarization[30];  /* Used to indicate what type of
                                 * summarization is being used. This is
                                 * currently set to none for DWMLgen product
                                 * since there is no summarization of NDFD
                                 * data and to 12 hourly or 24 hourly
                                 * depending on element and format for
                                 * DWMLgenByDay products. */
   char format[30];           /* The two DWMLgenByDay products have two
                               * formats.  The first indicated by
                               * "24 hourly", summarize NDFD data over 24
                               * hour (6 AM to 6 AM) periods. The second,
                               * "12 hourly" summarizes NDFD data in two 12
                               * hour periods for each 24 hour day. (6 AM to
                               * 6 PM) day and (6 PM to 6 AM) night periods. */
   char ***layoutKeys = NULL; /* Array containing the layout keys. */
   char **currentDay = NULL;  /* Current day's 2-digit date. */
   char **currentHour = NULL; /* Current hour's 2-digit hour. */
   sChar f_WxParse;           /* Flag denoting wether to return the weather
                               * strings from NDFD as coded "ugly stings" or
                               * their English equivalent. */
   double *timeUserStart = NULL; /* User supplied startTime, if available. Defaults
				    to first validTime if argument startTime not 
				    entered. */
   double *timeUserEnd = NULL;  /* User supplied endTime, if available. Defaults
				   to last validTime if argument endTime not 
				   entered. */
   double *firstValidTimeMatch = NULL; /* The very first validTime for all 
                                        * matches, per point. Used in 
                                        * determining numDays. */
   double *firstValidTime_maxt = NULL; /* Valid time of first MaxT, per point. */
   double *firstValidTime_pop = NULL; /* Valid time of first Pop12Hr, per point */
   int *f_formatNIL = NULL;   /* Flag denoting that we may have to create a
                               * NIL value for the first max temp if the
                               * request for data is late in the day. Only 
			       * applicable for 24 hourly summarization 
			       * product. */
   int *f_useMinTempTimes = NULL; /* Flag denoting if we should simply use the
                                   * MinT layout key for the MaxT element. Only
                                   * used in DWMLgenByDay product. */

   /* Variables that will go into formatTimeLayoutInfo() routine. */

   sChar f_observeDST;        /* Flag determining if current point observes
                               * Daylight Savings Time.  */
   double currentDoubTime;    /* Current real time in double format.  */
   char currentLocalTime[30]; /* Current Local Time in
                               * "2006-14-29T00:00:00-00:00" format. */
   uChar f_formatNewPointTimeLayouts = 0; /* Flag used fo denote if a new
                                           * time layout is needed due to a
                                           * point existing in a new time
                                           * zone. */
   char layoutKey[20];        /* The key linking the elements to their valid
                               * times (ex. k-p3h-n42-1). */
   size_t numLayoutSoFar = 1; /* Counter denoting the total number of time
                               * layouts that have been created so far. */
   uChar numCurrentLayout = 0;  /* The number of the layout we are currently
                                 * processing.  */
   double firstMaxTValidTime_doub_adj = 0.0;  /* The date of the first MaxT
                                               * validTime with the hour,
                                               * minutes, and seconds set to
                                               * zeroes, translated to double
                                               * form. Used in a DWNLgenByDay
                                               * check. */
   double currentLocalTime_doub_adj = 0.0;  /* The current date with the
                                             * hours, minutes, and seconds
                                             * set to zeroes, translated to
                                             * double form. Used in a
                                             * DWNLgenByDay check. */

   /* Variables that will go into formatParameterInfo() routine. */

   xmlNodePtr parameters = NULL;  /* An xml Node Pointer denoting the
                                   * <parameters> node in the formatted XML. */
   char pointBuff[20];        /* String holding point number. */
   int *maxDailyPop = NULL;   /* Array containing the pop values
                               * corresponding to a day (24 hour format) or
                               * 12 hour period (12 hour format).  For 24
                               * hour format, we use the maximum of the two
                               * 12 hour pops that span the day. This
                               * variable is used to test if the pop is large
                               * enough to justify formatting weather values. */
   int *maxWindSpeed = NULL;  /* Array containing the Maximum wind speed
                               * values corresponding to a day (24 hour
                               * format) or 12 hour period (12 hour format).
                               * These values are used in deriving the
                               * weather and/or icon values. */
   int *maxWindDirection = NULL;  /* Array containing the wind direction
                                   * values corresponding to a day (24 hour
                                   * format) or 12 hour period (12 hour
                                   * format). These are not "max" wind
                                   * direction values, but correspond to the
                                   * time when the max. wind speed values
                                   * were found per forecast period.  These
                                   * values are used in deriving the weather
                                   * and/or icon values. */
   double *valTimeForWindDirMatch = NULL; /* Array with the validTimes that
                                           * corresponds to the times when
                                           * the max wind speeds are the
                                           * highest.  We then collect the
                                           * wind directions that correspond
                                           * to the same times when the wind
                                           * speeds are the highest. */
   int *maxSkyCover = NULL;   /* Array containing the maximum Sky Cover
                               * values corresponding to a day (24 hour
                               * format) or 12 hour period (12 hour format).
                               * These values are used in deriving the
                               * weather and/or icon values. */
   int *minSkyCover = NULL;   /* Array containing the minimum Sky Cover
                               * values corresponding to a day (24 hour
                               * format) or 12 hour period (12 hour format).
                               * These values are used in deriving the
                               * weather and/or icon values. */
   int *averageSkyCover = NULL; /* Array containing the average Sky Cover
                                 * values corresponding to a day (24 hour
                                 * format) or 12 hour period (12 hour
                                 * format).  These values are used in
                                 * deriving the weather and/or icon values. */
   int *maxSkyNum = NULL; /* Records the index in the match structure where the 
			   * max sky cover was found. Used to determine sky 
			   * cover trends (i.e. increasing clouds) for 
			   * DWMLgenByDay products. */
   int *minSkyNum = NULL; /* Records the index in the match structure where the 
			   * min sky cover was found. Used to determine sky 
			   * cover trends (i.e. increasing clouds) for 
			   * DWMLgenByDay products. */
   int *startPositions = NULL; /* The index of where the current forecast period
			        * begins.  Used to determine sky cover trends 
			        * (i.e. increasing clouds) for DWMLgenByDay 
			        * products. */
   int *endPositions = NULL; /* The index of where the current forecast period
			      * ends.  Used to determine sky cover trends 
			      * (i.e. increasing clouds) for DWMLgenByDay 
			      * products. */
   layouts dummyTimeLayout;   /* Dummy argument for call to isNewLayout() to
                               * free up static array
                               * "timeLayoutDefinitions". */
   int f_puertori = 0;        /* Denotes a match was found in the puertori sector
                               * (sector 1). */
   int f_conus = 0;           /* Denotes a match was found in the conus sector
                               * (sector 0). */
   int f_nhemi = 0;           /* Denotes a match was found in the nhemi sector
                               * (sector 5). */
   int numNhemi = 0;          /* Number of matches in nhemi sector. If this 
                                 number is equal to numMatch, all matches are
                                 found in one sector (this can occur if there
                                 is a call for tropical wind thresholds for
                                 a call with puertori and conus points. */

   /* XML Document pointer */
   xmlDocPtr doc = NULL;      /* An xml Node Pointer denoting the top-level
                               * document node. */

   /* XML Node pointers */
   xmlNodePtr dwml = NULL;    /* An xml Node Pointer denoting the <dwml>
                               * node. */
   xmlNodePtr data = NULL;    /* An xml Node Pointer denoting the <data>
                               * node. */

   /* First, get the current system time, in double form. */
   currentDoubTime = Clock_Seconds();

   /* Set up the varFilter array to show what NDFD variables are of
    * interest(1) or vital(2) to this procedure. 
    */
   setVarFilter(f_XML, f_icon, &(varFilter[0]));

   /* Allow user filter of variables to reduce the element list, but include
    * all "vital" (Filter == 2) variables. 
    */
   genElemListInit2(varFilter, numNdfdVars, ndfdVars, &numElem, &elem);

   /* If product is of DWMLgen's time-series, we need to decipher between
    * elements originating on the command line argument and those forced if
    * command line argument -Icon is set to 1 (turned on). 
    */
   if (f_XML == 1 && f_icon == 1)
   {
      varFilter[NDFD_TEMP]--;
      varFilter[NDFD_WS]--;
      varFilter[NDFD_SKY]--;
      varFilter[NDFD_WX]--;
      varFilter[NDFD_POP]--;
   }

   /* We need to turn the f_icon flag on if all elements are to be formatted
    * by default (when there is no -ndfdfVars command option) AND there was
    * no forcing of elements.  This will ensure Icons are formatted along
    * with all the NDFD elements. 
    */
   if (f_XML == 1 && numNdfdVars == 0 && f_icon == 0)
      f_icon = 1;

   /* Force Icons to be formatted if DWMLgen "glance" product. */
   if (f_XML == 2)
      f_icon = 1;

   /* See if any points are outside the NDFD Sectors. Print error message to
    * standard error if so. If all points selected are outside the NDFD
    * Sectors, exit routine. */
   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
         f_atLeastOnePointInSectors = 1;
      else
      {
         #ifdef PRINT_DIAG
         printf("Point #%d is outside of all NDFD Sectors.\n", j + 1);
         #endif
      }
   }
   if (f_atLeastOnePointInSectors != 1)
   {
      #ifdef PRINT_DIAG
      printf("************************************************************\n");
      printf("No point(s) selected are inside the NDFD Sectors. Exiting...\n");
      printf("************************************************************\n");
      return 0;
      #endif
   }

   /* If the product is one of the summarizations (f_XML = 3 or 4) and a 
    * startTime and/or endTime command line argument is entered, we need to 
    * shorten the time window the grid probe returns data for so not all data
    * is returned. The time frame needs to start at 05 hours on the date of 
    * the startTime and end 12 hours into the next day containing the endTime.    
    */

   /* If the adjusted startTime and endTime are just 12 hours apart (i.e., user
    * chose a startTime and an endTime on the same calander day), bump the 
    * endTime up by 24 hours to at minimum format 1 days summarization. Add a 
    * logical time check too. 
    */
   if (f_XML == 3 || f_XML == 4)
   {
      if (endTime != 0.0 && startTime != 0.0)
      {
         if (endTime - startTime < 0)
         {
            #ifdef PRINT_DIAG
            printf ("Command line argument endTime starts before startTime\n");
            printf ("exiting.... \n");
            return 0;
            #endif
         }
	 if (endTime - startTime == (12 * 3600))
            endTime = endTime + (24 * 3600);
      }
   }

   /* f_WxParse = 0, is the flag to return WX as ugly weather codes. */
   f_WxParse = 0;
   if (genProbe(numPnts, pnts, f_pntType, numInFiles, inFiles, f_fileType,
                f_interp, f_unit, majEarth, minEarth, f_WxParse,
                f_SimpleVer, numElem, elem, f_valTime, startTime, endTime,
                &numMatch, &match, f_inTypes, gribFilter, numSector,
                sector, f_ndfdConven) != 0)
   {
      for (i = 0; i < numElem; i++)
      {
         genElemFree(elem + i);
      }
      free(elem);
      for (i = 0; i < numMatch; i++)
      {
         genMatchFree(match + i);
      }
      free(match);
      return -1;
   }

   /* If no data is retrieved from NDFD (matches = zero), get out. */
   if (numMatch <= 0)
   {
/*      #ifdef PRINT_DIAG */
      printf("No data retrieved from NDFD (matches = 0).\n");
      return 0;
/*      #endif */
   }

   /* Sort the matches by sector, element, and then by valid time. */
   qsort(match, numMatch, sizeof(match[0]), XMLmatchCompare);

   /* Allocate f_pntNoData. */
   f_pntHasData = calloc(numPnts, sizeof(char));

   /* Collate the matches. While accessing match structure, get infomation on
    * any matches in the "nhemi" structure.
    */
   curTime = -1;
   for (i = 0; i < numMatch; i++)
   {
      if (match[i].f_sector == 5) /* Enumerated nhemi sector # equals 5. */
         numNhemi++;
      if (curTime != match[i].validTime)
      {
         j = numCollate;
         numCollate++;
         collate = realloc(collate, numCollate * sizeof(collateType));
         for (k = 0; k < NDFD_MATCHALL + 1; k++)
         {
            collate[numCollate - 1].allElem[k] = -1;
         }
         collate[numCollate - 1].validTime = match[i].validTime;
         curTime = match[i].validTime;
      }

      myAssert(numCollate > 0);
      collate[numCollate - 1].allElem[match[i].elem.ndfdEnum] = i;
      /* update f_pntHasData based on this "match". */
      for (j = 0; j < numPnts; j++)
      {
         if (match[i].value[j].valueType != 2)
            f_pntHasData[j] = 1;
      }
   }

#ifdef PRINT_DIAG
   /* Loop by point to check if any data at point at all. */
   for (j = 0; j < numPnts; j++)
   {
      /* Check if this point has any data at all */
      if (isPntInASector(pnts[j]))
      {
         if (f_pntHasData[j])
         {
            PrintDay1(match, j, collate, numCollate, pntInfo[j].timeZone,
                          pntInfo[j].f_dayLight);
         }
         printf("-----------------\n");
      }
   }
#endif

   /* Gather up sector information for times when a multiple point call has two
    * points in different sectors. Place this info in the Point's informational
    * structure "pntInfo". 
    */
   if (numNhemi > 0)
      f_nhemi = 1;
   getSectorInfo(pntInfo, pnts, numPnts, match, numMatch, numSector, sector,
                 f_nhemi, &f_puertori, &f_conus, numNhemi);

   /*************************************************************************** 
    *                   Start formatting the XML.                             *
    *                                                                         * 
    ***********************  HEADER INFO  *************************************/

   /* Create the XML document and format the Meta Data before looping through
    * points. 
    */
   formatMetaDWML(f_XML, &doc, &data, &dwml);

   /* Allocate, and then initialize some things in a point loop. */
   weatherParameters = (uChar **) malloc(numPnts * sizeof(uChar *));
   firstValidTimeMatch = (double *) malloc(numPnts * sizeof(double));
   firstValidTime_pop = (double *) malloc(numPnts * sizeof(double));
   firstValidTime_maxt = (double *) malloc(numPnts * sizeof(double));
   numDays = (int *) malloc(numPnts * sizeof(int));
   numOutputLines = (int *) malloc(numPnts * sizeof(int));

   for (j = 0; j < numPnts; j++)
   {
      firstValidTimeMatch[j] = 0.0;
      firstValidTime_pop[j] = 0.0;
      firstValidTime_maxt[j] = 0.0;
      numDays[j] = 0;
 
      /* weatherParameters array denotes those elements that will ultimately
       * be formatted (set to 1). Those retrieved from NDFD by degrib but used
       * only to derive other elements are set to 2.
       */
      weatherParameters[j] = (uChar *) malloc((NDFD_MATCHALL + 1) *
                                               sizeof(uChar));
      for (i = 0; i < NDFD_MATCHALL + 1; i++)
         weatherParameters[j][i] = 0;

      /* Get each point's first Valid times for MaxT and Pop. */
      for (i = pntInfo[j].startNum; i < pntInfo[j].endNum; i++)
      { 
         if (match[i].elem.ndfdEnum == NDFD_MAX)
         {
            firstValidTime_maxt[j] = match[i].validTime;
            break;
         }
      }
      for (i = pntInfo[j].startNum; i < pntInfo[j].endNum; i++)
      { 
         if (match[i].elem.ndfdEnum == NDFD_POP)
         {
            firstValidTime_pop[j] = match[i].validTime;
            break;
         }
      }
   }

   /* Prepare some data for DWMLgen's "time-series" product. */
   if ((f_XML == 1) || (f_XML == 2))
      prepareDWMLgen(f_XML, &f_formatPeriodName, weatherParameters,
                     whatSummarization, varFilter, &f_icon, numPnts);

   /* Prepare data for DWMLgenByDay's "12 hourly" & "24 hourly" products. */
   if ((f_XML == 3) || (f_XML == 4))
      prepareDWMLgenByDay(match, f_XML, &startTime, &endTime, 
		          firstValidTimeMatch, numDays, format, 
			  &f_formatPeriodName, weatherParameters, 
			  &timeInterval, numOutputLines, whatSummarization,
			  currentDoubTime, numPnts, pntInfo);

   /*************************  DATA INFO  ******************************/

   /* Format the <LOCATION> ELEMENT in the XML/DWML. */
   formatLocationInfo(numPnts, pnts, data);

   /* Run through matches to find numRows returned per element for each
    * point. Since this is based off of what was actually returned from NDFD,
    * update the weatherParameters array accordingly to reflect what is
    * actually returned from the NDFD. 
    */
   numRowsForPoint = (numRowsInfo **)malloc(numPnts * sizeof(numRowsInfo *));
   startDate = (char **)malloc(numPnts * sizeof(char *));
   timeUserStart = (double *)malloc(numPnts * sizeof(double));
   timeUserEnd = (double *)malloc(numPnts * sizeof(double));
   f_formatNIL = (int *)malloc(numPnts * sizeof(int));
   f_useMinTempTimes = (int *)malloc(numPnts * sizeof(int));
   f_formatIconForPnt = (int *)malloc(numPnts * sizeof(int));
   f_formatSummarizations = (int *)malloc(numPnts * sizeof(int));
   currentDay = (char **)malloc(numPnts * sizeof(char *));
   currentHour = (char **)malloc(numPnts * sizeof(char *));

   for (j = 0; j < numPnts; j++)
   {
      /* Initialize a few things. */
      if (f_icon == 1)
         f_formatIconForPnt[j] = 1;
      else
         f_formatIconForPnt[j] = 0;
 
      f_formatSummarizations[j] = 1;

      if (isPntInASector(pnts[j]))
      {
	 /* Open up each point's # of Rows to an element allocation. */
         numRowsForPoint[j] = (numRowsInfo *) malloc((NDFD_MATCHALL + 1) *
                                               sizeof(numRowsInfo));

	 /* Fill/Get the startDate array. */
         getStartDates(startDate, f_XML, startTime, firstValidTimeMatch[j], 
		       firstValidTime_maxt[j], pntInfo[j].timeZone, 
		       pntInfo[j].f_dayLight, j);

         /* Convert the system time to a formatted local time
          * (i.e. 2006-02-02T17:00:00-05:00) to get the current Hour and Day
          * for the point location. 
          */
         Clock_Print2(currentLocalTime, 30, currentDoubTime,
                      "%Y-%m-%dT%H:%M:%S", pntInfo[j].timeZone, 1);

         /* Now get the current day's date and hour. */
         currentDay[j] = (char *) malloc(3 * sizeof(char));
         currentDay[j][0] = currentLocalTime[8];
         currentDay[j][1] = currentLocalTime[9];
         currentDay[j][2] = '\0';

         currentHour[j] = (char *) malloc(3 * sizeof(char));
         currentHour[j][0] = currentLocalTime[11];
         currentHour[j][1] = currentLocalTime[12];
         currentHour[j][2] = '\0';
 
         getNumRows(numRowsForPoint[j], &timeUserStart[j], &timeUserEnd[j], 
                    numMatch, match, weatherParameters[j], f_XML, &f_icon,
                    pntInfo[j].timeZone, pntInfo[j].f_dayLight,
                    pntInfo[j].startNum, pntInfo[j].endNum, startDate[j], 
                    &numDays[j], startTime, endTime, currentHour[j], 
		    &firstValidTime_pop[j], &firstValidTimeMatch[j], 
                    &f_formatIconForPnt[j], &f_formatSummarizations[j], j);
      }
   }

#ifdef PRINT_DIAG
   for (j = 0; j < numPnts; j++)
   {
      for (i = 0; i < NDFD_MATCHALL + 1; i++)
      {
         if (isPntInASector(pnts[j]))
            printf("numRowsForPoint[%d][%d].total check = %d\n",j, i, 
	            numRowsForPoint[j][i].total);
      }
   }
   for (j = 0; j < numPnts; j++)
   {
      for (i = 0; i < NDFD_MATCHALL + 1; i++)
      {
         if (isPntInASector(pnts[j]))
            printf("weatherParameters [%d][%d] = %d\n", j, i, 
                    weatherParameters[j][i]);
      }
   }
#endif

   /*******Format the <MOREWEATHERINFORMATION> ELEMENT in the XML/DWML********/
   formatMoreWxInfo(numPnts, pnts, data);
   
   /***********Format the <TIME-LAYOUT> ELEMENT in the XML/DWML***************/

   /* Allocate the time zone information and the layoutKeys to the number
    * of points. The layout-keys will only be generated once, UNLESS,
    * a point is chosen in a different time zone (which will alter the
    * time-layout info). 
    */
   layoutKeys = (char ***)malloc(numPnts * sizeof(char **));
   TZoffset = (sChar *) malloc(numPnts * sizeof(sChar));

   /* Begin point loop for time-layout generation. */
   for (j = 0; j < numPnts; j++)
   { 
      f_formatNIL[j] = 0;
      f_useMinTempTimes[j] = 0;
      if (isPntInASector(pnts[j]))
      {   
         TZoffset[j] = pntInfo[j].timeZone;
         f_observeDST = pntInfo[j].f_dayLight;
         startNum = pntInfo[j].startNum;
         endNum = pntInfo[j].endNum;
       
         /* Convert the system time to a formatted local time
          * (i.e. 2006-02-02T17:00:00-05:00).
	 */
         Clock_Print2(currentLocalTime, 30, currentDoubTime,
                      "%Y-%m-%dT%H:%M:%S",TZoffset[j], 1);
	 
         /* Check to see if points are in different time zones. We need to
          * compare current points' time zone to all prior valid points'
          * time zones. 
	  */
         if (j > 0)
         {
            for (i = j - 1; i >= 0; i--)
            {
               if (isPntInASector(pnts[i]))
               {
                  if (TZoffset[j] != TZoffset[i])
                  {
                     f_formatNewPointTimeLayouts = 1;
                     break;
                  }
               }     
            }
         }
         layoutKeys[j] = (char **)malloc((NDFD_MATCHALL + 1) * sizeof(char *));

         if (f_firstPointLoopIteration || f_formatNewPointTimeLayouts)
         {
            /* Generate a new set of time-layouts for the point. */
            for (k = 0; k < (NDFD_MATCHALL + 1); k++)
            {
               if (weatherParameters[j][k] == 1 || weatherParameters[j][k] == 3)
               {
                  /* For DWMLgen products' "time-series" and "glance". */
                  if ((f_XML == 1 || f_XML == 2) && 
                       numRowsForPoint[j][k].total != 0)
                  {
                     generateTimeLayout(numRowsForPoint[j][k], k, layoutKey,
                                        whichTimeCoordinate,
                                        whatSummarization, match, numMatch,
                                        f_formatPeriodName, TZoffset[j],
                                        f_observeDST, &numLayoutSoFar,
                                        &numCurrentLayout, currentHour[j],
                                        currentDay[j], "boggus", data, startTime,
                                        currentDoubTime, &numDays[j], f_XML, 
                                        startNum, endNum);

                     layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                     strcpy(layoutKeys[j][k], layoutKey);
                  }
                  else if (f_XML == 3 && numRowsForPoint[j][k].total != 0)
                  {
                     /* For DWMLgenByDay product w/ format == "12 hourly". */
                     if (k == NDFD_MAX || k == NDFD_MIN)
                     {
                        generateTimeLayout(numRowsForPoint[j][k], k, layoutKey,
                                           whichTimeCoordinate,
                                           whatSummarization, match,
                                           numMatch, f_formatPeriodName,
                                           TZoffset[j], f_observeDST,
                                           &numLayoutSoFar, &numCurrentLayout,
                                           currentHour[j], currentDay[j], format,
                                           data, startTime, currentDoubTime, 
					   &numDays[j], f_XML, startNum, endNum);
			
                        layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                        strcpy(layoutKeys[j][k], layoutKey);
                     }
                     else
                     {
                        /* The other element's (Wx and Icons) will share 
			 * Pop's layout.
			 */
                        generateTimeLayout(numRowsForPoint[j][NDFD_POP],
				           NDFD_POP, layoutKey,
                                           whichTimeCoordinate,
                                           whatSummarization, match,
                                           numMatch, f_formatPeriodName,
                                           TZoffset[j], f_observeDST,
                                           &numLayoutSoFar, &numCurrentLayout, 
                                           currentHour[j], currentDay[j], format, 
                                           data, startTime, currentDoubTime, 
					   &numOutputLines[j], f_XML, startNum, 
                                           endNum);

                        layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                        strcpy(layoutKeys[j][k], layoutKey);
                     }
                  }
                  else if (f_XML == 4 && numRowsForPoint[j][k].total != 0)
                  {
                     /* For DWMLgenByDay product w/ format == "24 hourly".
                      * Since the product is DWMLgenByDay's "24 hourly"
                      * format, force the weather (and icon) elements so that
                      * their time layout equals that of MaxT and/or MinT
                      * since their periods are = to 24 hours also (the
                      * exception is Pop, since it still has to use a 12
                      * hourly summariztion). 
		      */
                     if (k != NDFD_POP)
                     {
                        /* If the request for MaxT's is late in the day, then
                         * we will format "nil" for first MaxT data value and
                         * simply use the MinT's time layout. Retrieve the
                         * necessary info for this check. 
			 */
                        monthDayYearTime(match, numMatch, currentLocalTime,
                                         currentDay[j], f_observeDST,
                                         &firstMaxTValidTime_doub_adj,
                                         &currentLocalTime_doub_adj, 
					 TZoffset[j], startNum, endNum,
					 numRowsForPoint[j][NDFD_MAX]);

                        if (atoi(currentHour[j]) > 18 && currentLocalTime_doub_adj
                            + 86400 == firstMaxTValidTime_doub_adj) 
                        {
                           f_formatNIL[j] = 1;
                           f_useMinTempTimes[j] = 1;

                           generateTimeLayout(numRowsForPoint[j][NDFD_MIN],
					      NDFD_MIN, layoutKey,
                                              whichTimeCoordinate,
                                              whatSummarization, match,
                                              numMatch, f_formatPeriodName,
                                              TZoffset[j], f_observeDST,
                                              &numLayoutSoFar, &numCurrentLayout,
                                              currentHour[j], currentDay[j],
                                              format, data, startTime,
                                              currentDoubTime, &numDays[j], f_XML, 
                                              startNum, endNum);

                           layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                           strcpy(layoutKeys[j][k], layoutKey);
                        }
                        else  /* Use MaxT's own time-layout. */
                        {
                           f_formatNIL[j] = 0;
                           f_useMinTempTimes[j] = 0;

                           generateTimeLayout(numRowsForPoint[j][NDFD_MAX],
					      NDFD_MAX, layoutKey,
                                              whichTimeCoordinate,
                                              whatSummarization, match,
                                              numMatch, f_formatPeriodName,
                                              TZoffset[j], f_observeDST,
                                              &numLayoutSoFar,
                                              &numCurrentLayout,
                                              currentHour[j], currentDay[j],
                                              format, data, startTime,
                                              currentDoubTime, &numDays[j], f_XML, 
                                              startNum, endNum);

                           layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                           strcpy(layoutKeys[j][k], layoutKey);
                        }

                     }
                     else
                     {
                        /* POP gets its own time-layout, using
                         * 12-hourly summarization and format (force
                         * these), even though f_XML == 4. 
			 */
			numPopLines = numDays[j] * 2;
                        generateTimeLayout(numRowsForPoint[j][k], 
					   NDFD_POP, layoutKey, 
					   whichTimeCoordinate, "12hourly",
                                           match, numMatch,
                                           f_formatPeriodName, TZoffset[j],
                                           f_observeDST, &numLayoutSoFar,
                                           &numCurrentLayout, currentHour[j],
                                           currentDay[j], "12 hourly", data,
                                           startTime, currentDoubTime, 
					   &numPopLines, f_XML, startNum, 
                                           endNum);

                        layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                        strcpy(layoutKeys[j][k], layoutKey);
                     }

                  } /* End of "f_XML type" check. */
               } /* End weatherParameters "if" statement. */
            } /* End weatherParameters "k" loop. */
         }
         else
         {
            /* Simply copy the previous valid point's time layout-keys into
             * this point's time layout-keys. 
	     */
            for (i = j - 1; i >= 0; i--)
            {
               if (isPntInASector(pnts[i]))
               {
                  for (k = 0; k < (NDFD_MATCHALL + 1); k++)
                  {
                     if (weatherParameters[j][k] == 1)
                     {
                        layoutKeys[j][k] = malloc(strlen(layoutKeys[i][k]) + 1);
                        strcpy(layoutKeys[j][k], layoutKeys[i][k]);
                     }
                  }
               }
            }
         }  /* End of "if-else" of "format New Point time layout" check. */
	 f_firstPointLoopIteration = 0;
	 
      }  /* End of "is Point in Sector" check. */
   }  /* End "Point Loop" for Time-Layouts. */

   /*********************** PARAMETER <ELEMENT> *****************************/

   /* Format the Parameter Information into the XML/DWML. */
   /* Begin new Point Loop to format the Data/Weather Parameter values. The 
    * order of elements below matches the order in the DWML schema in the way
    * the elements are to be formatted. 
    */
   for (j = 0; j < numPnts; j++)
   {

      /* Check to make sure point is within the NDFD sectors. */
      if (isPntInASector(pnts[j]))
      {
         parameters = xmlNewChild(data, NULL, BAD_CAST "parameters", NULL);
         sprintf(pointBuff, "point%d", (j + 1));
         xmlNewProp(parameters, BAD_CAST "applicable-location", BAD_CAST
                    pointBuff);

         /* Format Maximum Temperature Values, if applicable. */
         if (weatherParameters[j][NDFD_MAX] == 1)
            genMaxTempValues(j, layoutKeys[j][NDFD_MAX], match, parameters,
                             f_formatNIL[j], f_XML, startTime,
                             numRowsForPoint[j][NDFD_MAX], numDays[j], 
                             pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Minimum Temperature Values, if applicable. */
         if (weatherParameters[j][NDFD_MIN] == 1)
            genMinTempValues(j, layoutKeys[j][NDFD_MIN], match, parameters, 
                             f_XML, startTime, numRowsForPoint[j][NDFD_MIN],
			     currentDay[j], currentHour[j], TZoffset[j], 
                             pntInfo[j].f_dayLight, numDays[j], 
                             pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Hourly Temperature Values, if applicable. */
         if (weatherParameters[j][NDFD_TEMP] == 1)
            genTempValues(j, layoutKeys[j][NDFD_TEMP], match, parameters,
                          numRowsForPoint[j][NDFD_TEMP], pntInfo[j].startNum, 
                          pntInfo[j].endNum);

         /* Format Dew Point Temperature Values, if applicable. */
         if (weatherParameters[j][NDFD_TD] == 1)
            genDewPointTempValues(j, layoutKeys[j][NDFD_TD], match,
                                  parameters, numRowsForPoint[j][NDFD_TD], 
                                  pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Apparent Temperature Values, if applicable. */
         if (weatherParameters[j][NDFD_AT] == 1)
            genAppTempValues(j, layoutKeys[j][NDFD_AT], match, parameters,
                             numRowsForPoint[j][NDFD_AT], pntInfo[j].startNum, 
                             pntInfo[j].endNum);

         /* Format QPF Values, if applicable. */
         if (weatherParameters[j][NDFD_QPF] == 1)
            genQPFValues(j, layoutKeys[j][NDFD_QPF], match, parameters, 
                         numRowsForPoint[j][NDFD_QPF], pntInfo[j].startNum, 
                         pntInfo[j].endNum);

         /* Format Snow Amount Values, if applicable. */
         if (weatherParameters[j][NDFD_SNOW] == 1)
            genSnowValues(j, layoutKeys[j][NDFD_SNOW], match, parameters,
                         numRowsForPoint[j][NDFD_SNOW], pntInfo[j].startNum, 
                         pntInfo[j].endNum);

         /* Format PoP12 Values, if applicable. */
         if (weatherParameters[j][NDFD_POP] == 1)
         {
            /* If product is of DWMLgenByDay type, allocate maxDailyPop array
             * and initialize it. */

            if (f_XML == 3)
            {
               maxDailyPop = malloc((numDays[j] * 2) * sizeof(int));
               for (i = 0; i < numDays[j] * 2; i++) 
                  maxDailyPop[i] = 0;
	    }
	    if (f_XML == 4)
            {
               maxDailyPop = malloc((numDays[j]) * sizeof(int));
               for (i = 0; i < numDays[j]; i++)
                  maxDailyPop[i] = 0;		  
	    }
            genPopValues(j, layoutKeys[j][NDFD_POP], match, parameters, 
                         numRowsForPoint[j][NDFD_POP], f_XML, startTime,
                         maxDailyPop, &numDays[j], currentDoubTime, currentHour[j],
			 pntInfo[j].startNum, pntInfo[j].endNum);
	 }

         /****************************RTMA ELEMENTS***************************/
         /********************************************************************/
         /* Format the Real Time Mesoscale Analyses elements for the rtma 
          * product, if applicable. 
	  */




         /*****************9 SPC CONVECTIVE HAZARDS ELEMENTS******************/
         /********************************************************************/
         /* Format the Categorical Convective Hazard Outlook for DWMLgen 
          * time-series product, for days 1-3, if applicable. 
	  */
         if (weatherParameters[j][NDFD_CONHAZ] == 1)
            genConvOutlookValues(j, layoutKeys[j][NDFD_CONHAZ], match,
                                 parameters, numRowsForPoint[j][NDFD_CONHAZ], 
                                 pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format the Probability of Tornadoes Convective Hazard for DWMLgen 
          * time-series product, for Day 1, if applicable. 
	  */
         if (weatherParameters[j][NDFD_PTORN] == 1)
            genConvSevereCompValues(j, layoutKeys[j][NDFD_PTORN], NDFD_PTORN, 
                                    match, "tornadoes", 
                                    "Probability of Tornadoes", parameters, 
                                    numRowsForPoint[j][NDFD_PTORN], 
                                    pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format the Probability of Hail Convective Hazard for DWMLgen 
          * time-series product, for Day 1, if applicable. 
	  */
         if (weatherParameters[j][NDFD_PHAIL] == 1)
            genConvSevereCompValues(j, layoutKeys[j][NDFD_PHAIL], NDFD_PHAIL, 
                                    match, "hail", "Probability of Hail", 
                                    parameters, numRowsForPoint[j][NDFD_PHAIL],
                                    pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format the Probability of Damaging Thunderstorm Winds Convective 
          * Hazard for DWMLgen time-series product, for Day 1, if applicable. 
	  */
         if (weatherParameters[j][NDFD_PTSTMWIND] == 1)
            genConvSevereCompValues(j, layoutKeys[j][NDFD_PTSTMWIND], NDFD_PTSTMWIND, 
                                    match, "damaging thunderstorm winds", 
                                    "Probability of Damaging Thunderstorm Winds", 
                                    parameters, numRowsForPoint[j][NDFD_PTSTMWIND],
                                    pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format the Probability of Extreme Tornadoes Convective Hazard
          * for DWMLgen time-series product, for Day 1, if applicable. 
	  */
         if (weatherParameters[j][NDFD_PXTORN] == 1)
            genConvSevereCompValues(j, layoutKeys[j][NDFD_PXTORN], NDFD_PXTORN, 
                                    match, "extreme tornadoes", 
                                    "Probability of Extreme Tornadoes", 
                                    parameters, numRowsForPoint[j][NDFD_PXTORN],
                                    pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format the Probability of Extreme Hail Convective Hazard for
          * DWMLgen time-series product, for Day 1, if applicable. 
	  */
         if (weatherParameters[j][NDFD_PXHAIL] == 1)
            genConvSevereCompValues(j, layoutKeys[j][NDFD_PXHAIL], NDFD_PXHAIL, 
                                    match, "extreme hail", 
                                    "Probability of Extreme Hail", 
                                    parameters, numRowsForPoint[j][NDFD_PXHAIL],
                                    pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format the Probability of Extreme Thunderstorm Winds Convective 
          * Hazard for DWMLgen time-series product, for Day 1, if applicable. 
	  */
         if (weatherParameters[j][NDFD_PXTSTMWIND] == 1)
            genConvSevereCompValues(j, layoutKeys[j][NDFD_PXTSTMWIND], NDFD_PXTSTMWIND, 
                                    match, "extreme thunderstorm winds", 
                                    "Probability of Extreme Thunderstorm Winds", 
                                    parameters, numRowsForPoint[j][NDFD_PXTSTMWIND],
                                    pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format the Total Probability of Severe Thunderstorms Convective 
          * Hazard for DWMLgen time-series product, for Days 2-3, if applicable. 
	  */
         if (weatherParameters[j][NDFD_PSTORM] == 1)
            genConvSevereCompValues(j, layoutKeys[j][NDFD_PSTORM], NDFD_PSTORM, 
                                    match, "severe thunderstorms", 
                                    "Total Probability of Severe Thunderstorms", 
                                    parameters, numRowsForPoint[j][NDFD_PSTORM],
                                    pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format the Total Probability of Extreme Severe Thunderstorms 
          * Convective Hazard for DWMLgen time-series product, for Days 2-3,
          * if applicable. 
	  */
         if (weatherParameters[j][NDFD_PXSTORM] == 1)
            genConvSevereCompValues(j, layoutKeys[j][NDFD_PXSTORM], NDFD_PXSTORM, 
                                    match, "extreme severe thunderstorms", 
                                    "Total Probability of Extreme Severe Thunderstorms", 
                                    parameters, numRowsForPoint[j][NDFD_PXSTORM],
                                    pntInfo[j].startNum, pntInfo[j].endNum);

         /*****************12 CLIMATE ANOMALY PROBABILITIES ******************/
         /********************************************************************/
         /* Format 8-14 Average Temperature, Above Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_TMPABV14D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_TMPABV14D], 
                         NDFD_TMPABV14D, match, "average temperature above normal",
                         "Probability of 8-14 Day Average Temperature Above Normal",
                         parameters, numRowsForPoint[j][NDFD_TMPABV14D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format 8-14 Average Temperature, Below Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_TMPBLW14D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_TMPBLW14D], 
                         NDFD_TMPBLW14D, match, "average temperature below normal", 
                         "Probability of 8-14 Day Average Temperature Below Normal",
                         parameters, numRowsForPoint[j][NDFD_TMPBLW14D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format 8-14 Average Precipitation, Above Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_PRCPABV14D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_PRCPABV14D], 
                         NDFD_PRCPABV14D, match, "average precipitation above normal", 
                         "Probability of 8-14 Day Average Precipitation Above Normal",
                         parameters, numRowsForPoint[j][NDFD_PRCPABV14D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format 8-14 Average Precipitation, Below Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_PRCPBLW14D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_PRCPBLW14D], 
                         NDFD_PRCPBLW14D, match, "average precipitation below normal", 
                         "Probability of 8-14 Day Average Precipitation Below Normal",
                         parameters, numRowsForPoint[j][NDFD_PRCPBLW14D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Monthly Average Temperature, Above Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_TMPABV30D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_TMPABV30D], 
                         NDFD_TMPABV30D, match, "average temperature above normal", 
                         "Probability of One-Month Average Temperature Above Normal",
                         parameters, numRowsForPoint[j][NDFD_TMPABV30D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Monthly Average Temperature, Below Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_TMPBLW30D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_TMPBLW30D],
                         NDFD_TMPBLW30D, match, "average temperature below normal", 
                         "Probability of One-Month Average Temperature Below Normal",
                         parameters, numRowsForPoint[j][NDFD_TMPBLW30D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Monthly Average Precipitation, Above Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_PRCPABV30D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_PRCPABV30D], 
                         NDFD_PRCPABV30D, match, "average precipitation above normal", 
                         "Probability of One-Month Average Precipitation Above Normal",
                         parameters, numRowsForPoint[j][NDFD_PRCPABV30D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Monthly Average Precipitation, Below Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_PRCPBLW30D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_PRCPBLW30D],
                         NDFD_PRCPBLW30D, match, "average precipitation below normal", 
                         "Probability of One-Month Average Precipitation Below Normal",
                         parameters, numRowsForPoint[j][NDFD_PRCPBLW30D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format 3-Monthly Average Temperature, Above Normal Values, if
          * applicable. 
          */
         if (weatherParameters[j][NDFD_TMPABV90D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_TMPABV90D], 
                         NDFD_TMPABV90D, match, "average temperature above normal", 
                         "Probability of Three-Month Average Temperature Above Normal",
                         parameters, numRowsForPoint[j][NDFD_TMPABV90D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format 3-Monthly Average Temperature, Below Normal Values, if
          * applicable. 
          */
         if (weatherParameters[j][NDFD_TMPBLW90D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_TMPBLW90D], 
                         NDFD_TMPBLW90D, match, "average temperature below normal", 
                         "Probability of Three-Month Average Temperature Below Normal",
                         parameters, numRowsForPoint[j][NDFD_TMPBLW90D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format 3-Monthly Average Precipitation, Above Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_PRCPABV90D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_PRCPABV90D], 
                         NDFD_PRCPABV90D, match, "average precipitation above normal", 
                         "Probability of Three-Month Average Precipitation Above Normal",
                         parameters, numRowsForPoint[j][NDFD_PRCPABV90D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format 3-Monthly Average Precipitation, Below Normal Values, if 
          * applicable. 
          */
         if (weatherParameters[j][NDFD_PRCPBLW90D] == 1)
            genClimateOutlookValues(j, layoutKeys[j][NDFD_PRCPBLW90D], 
                         NDFD_PRCPBLW90D, match, "average precipitation below normal", 
                         "Probability of Three-Month Average Precipitation Below Normal",
                         parameters, numRowsForPoint[j][NDFD_PRCPBLW90D], 
                         pntInfo[j].startNum, pntInfo[j].endNum);

         /*****************6 TROPICAL WIND THRESHOLD PROBABILITIES ***********/
         /********************************************************************/
         /* Format Incremental Probability of 34 Knt Wind Values for DWMLgen 
	  * product, if applicable. 
	  */
         if (weatherParameters[j][NDFD_INC34] == 1)
            genWindIncCumValues(j, layoutKeys[j][NDFD_INC34], NDFD_INC34, 
	                        match, "incremental34", 
	 "Probability of a Tropical Cyclone Wind Speed above 34 Knots (Incremental)",
	                        parameters, numRowsForPoint[j][NDFD_INC34], 
                                pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Incremental Probability of 50 Knt Wind Values for DWMLgen 
	  * product, if applicable. 
	  */
         if (weatherParameters[j][NDFD_INC50] == 1)
            genWindIncCumValues(j, layoutKeys[j][NDFD_INC50], NDFD_INC50,
			        match, "incremental50", 
	 "Probability of a Tropical Cyclone Wind Speed above 50 Knots (Incremental)",
	                        parameters, numRowsForPoint[j][NDFD_INC50],
                                pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Incremental Probability of 64 Knt Wind Values for DWMLgen 
	  * product, if applicable. 
	  */
         if (weatherParameters[j][NDFD_INC64] == 1)
            genWindIncCumValues(j, layoutKeys[j][NDFD_INC64], NDFD_INC64,
			        match, "incremental64", 
	 "Probability of a Tropical Cyclone Wind Speed above 64 Knots (Incremental)",
	                        parameters, numRowsForPoint[j][NDFD_INC64],
                                pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Cumulative Probability of 34 Knt Wind Values for DWMLgen 
	  * product, if applicable. 
	  */
         if (weatherParameters[j][NDFD_CUM34] == 1)
            genWindIncCumValues(j, layoutKeys[j][NDFD_CUM34], NDFD_CUM34,
			        match, "cumulative34", 
	 "Probability of a Tropical Cyclone Wind Speed above 34 Knots (Cumulative)",
	                        parameters, numRowsForPoint[j][NDFD_CUM34],
                                pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Cumulative Probability of 50 Knt Wind Values for DWMLgen 
	  * product, if applicable. 
	  */
         if (weatherParameters[j][NDFD_CUM50] == 1)
            genWindIncCumValues(j, layoutKeys[j][NDFD_CUM50], NDFD_CUM50,
			        match, "cumulative50", 
	 "Probability of a Tropical Cyclone Wind Speed above 50 Knots (Cumulative)", 
	                        parameters, numRowsForPoint[j][NDFD_CUM50],
                                pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Cumulative Probability of 64 Knt Wind Values for DWMLgen 
	  * product, if applicable. 
	  */
         if (weatherParameters[j][NDFD_CUM64] == 1)
            genWindIncCumValues(j, layoutKeys[j][NDFD_CUM64], NDFD_CUM64,
			        match, "cumulative64", 
	 "Probability of a Tropical Cyclone Wind Speed above 64 Knots (Cumulative)", 
	                        parameters, numRowsForPoint[j][NDFD_CUM64],
                                pntInfo[j].startNum, pntInfo[j].endNum);

         /********************************************************************/
         /* Format Wind Speed Values for DWMLgen products, if applicable.
          * Collect Max Wind Speed values if product is of type DWMLgenByDay. 
          */
         if (weatherParameters[j][NDFD_WS] == 1 || weatherParameters[j][NDFD_WS]
             == 2)
         {
            /* If product is of DWMLgenByDay type, allocate maxWindSpeed
             * array. We need the max wind speed values for each forecast
             * period to derive the weather and icon elements.  Also,
             * allocate the array holding the valid times that correspond to
             * the max wind speeds. These times will be used to collect the
             * wind directions that correspond to the times when the max
             * wind speeds occurred. 
	     */
            if (f_XML == 3 || f_XML == 4)
            {
               maxWindSpeed = malloc((numOutputLines[j]) * sizeof(int));
               valTimeForWindDirMatch = malloc((numOutputLines[j]) * sizeof(double));
               for (i = 0; i < (numOutputLines[j]); i++)
               {
                  maxWindSpeed[i] = -999;
                  valTimeForWindDirMatch[i] = -999;
               }
            }
            genWindSpeedValues(timeUserStart[j], timeUserEnd[j], j, 
			       layoutKeys[j][NDFD_WS], match, parameters, 
                               startDate[j], maxWindSpeed, &numOutputLines[j],
                               timeInterval, TZoffset[j], pntInfo[j].f_dayLight,
                               NDFD_WS, numRowsForPoint[j][NDFD_WS], f_XML,
                               valTimeForWindDirMatch, startTime, 
                               pntInfo[j].startNum, pntInfo[j].endNum);
         }

         /* Format Wind Speed Gust Values for DWMLgen products, if applicable. */
         if (weatherParameters[j][NDFD_WG] == 1)
            genWindSpeedGustValues(j, layoutKeys[j][NDFD_WG], match, 
                                   parameters, numRowsForPoint[j][NDFD_WG], 
                                   pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Wind Direction values for DWMLgen products, if applicable.
          * Collect the Wind Direction values that correspond to the times
          * when the maximum Wind Speeds existed if product is of type
          * DWMLgenByDay. */
         if (weatherParameters[j][NDFD_WD] == 1 || 
             weatherParameters[j][NDFD_WD] == 2)
         {		 
            /* If product is of DWMLgenByDay type, allocate maxWindDirection
             * array and initialize. We need these wind direction values for
             * each forecast period to derive the weather and icon elements. */
            if (f_XML == 3 || f_XML == 4)
            {
               maxWindDirection = malloc((numOutputLines[j]) * sizeof(int));
               for (i = 0; i < (numOutputLines[j]); i++)
                  maxWindDirection[i] = -999;
            }
            genWindDirectionValues(j, layoutKeys[j][NDFD_WD], match,
                                   parameters, maxWindDirection, f_XML, 
                                   &numOutputLines[j], valTimeForWindDirMatch,
				   numRowsForPoint[j][NDFD_WD], 
                                   pntInfo[j].startNum, pntInfo[j].endNum);
         }

         /* Format Sky Cover Values for DWMLgen products, if applicable.
          * Collect Max and Min Sky Cover values for icon determination if
          * product is of type DWMLgenByDay. */
         if (weatherParameters[j][NDFD_SKY] == 1 ||
             weatherParameters[j][NDFD_SKY] == 2)
         {

            /* If product is of DWMLgenByDay type, allocate the maxSkyCover,
             * minSkyCover, minSkyNum, maxSkyNum, startPositions, e
	     * ndPositions,
	     * and averageSkyCover arrays and initialize them. We need these 
	     * sky values for each forecast period to derive the weather and 
	     * icon elements. */

            if (f_XML == 3 || f_XML == 4)
            {
	       startPositions = malloc(numOutputLines[j] * sizeof(int));
	       endPositions = malloc(numOutputLines[j] * sizeof(int));
               maxSkyCover = malloc(numOutputLines[j] * sizeof(int));
               minSkyCover = malloc(numOutputLines[j] * sizeof(int));
	       maxSkyNum = malloc(numOutputLines[j] * sizeof(int));
               minSkyNum = malloc(numOutputLines[j] * sizeof(int));
               averageSkyCover = malloc(numOutputLines[j] * sizeof(int));
               
               for (i = 0; i < numOutputLines[j]; i++)
               {
		  maxSkyCover[i]    = -999;
                  startPositions[i] = -999;
                  endPositions[i]   = -999;
                  minSkyNum[i]      = -999;
                  maxSkyNum[i]      = +999;  /* Note (+) initialization. */
                  minSkyCover[i]    = +999;  /* Note (+) initialization. */
               }
            }

            genSkyCoverValues(j, layoutKeys[j][NDFD_SKY], match, parameters,
                              startDate[j], maxSkyCover, minSkyCover, 
                              averageSkyCover, &numOutputLines[j], timeInterval, 
                              TZoffset[j], pntInfo[j].f_dayLight, NDFD_SKY,
                              numRowsForPoint[j][NDFD_SKY], f_XML, maxSkyNum, 
			      minSkyNum, startPositions, endPositions,
			      &integerTime, currentHour[j], timeUserStart[j], 
			      startTime, pntInfo[j].startNum, pntInfo[j].endNum);
         }

         /* Format Relative Humidity Values, if applicable. */
         if (weatherParameters[j][NDFD_RH] == 1)
            genRelHumidityValues(j, layoutKeys[j][NDFD_RH], match, parameters, 
                                 numRowsForPoint[j][NDFD_RH], 
                                 pntInfo[j].startNum, pntInfo[j].endNum);

         /* Format Weather Values and\or Icons, if applicable. We must have
          * at least some rows of weather data to format either. 
          */
         if ((f_XML == 1 || f_XML == 2) && weatherParameters[NDFD_WX]) 
         {
            genWeatherValues(j, layoutKeys[j][NDFD_WX], match,
                             weatherParameters[j][NDFD_WX],
                             f_formatIconForPnt[j], 
                             numRowsForPoint[j][NDFD_WS],
                             numRowsForPoint[j][NDFD_SKY],
                             numRowsForPoint[j][NDFD_TEMP],
                             numRowsForPoint[j][NDFD_WX], 
	                     numRowsForPoint[j][NDFD_POP], parameters,
                             pnts[j].Y, pnts[j].X, pntInfo[j].startNum, 
                             pntInfo[j].endNum, TZoffset[j], 
                             pntInfo[j].f_dayLight);
         }
	 else if (f_XML == 3 || f_XML == 4)
         { 
            if (f_formatSummarizations[j])
            {
	       genWeatherValuesByDay(j, layoutKeys[j][NDFD_WX], match, numMatch,
	                             numRowsForPoint[j][NDFD_WG],
                                     numRowsForPoint[j][NDFD_WS],
			             numRowsForPoint[j][NDFD_POP],
			             numRowsForPoint[j][NDFD_MAX],
			             numRowsForPoint[j][NDFD_MIN],
                                     numRowsForPoint[j][NDFD_WX], parameters,
                                     &numDays[j], TZoffset[j], pntInfo[j].f_dayLight,
                                     format, f_useMinTempTimes[j], f_XML, 
                                     &numOutputLines[j], maxDailyPop, averageSkyCover, 
                                     maxSkyCover, minSkyCover, maxSkyNum, minSkyNum, 
			             startPositions, endPositions, maxWindSpeed,
			             maxWindDirection, integerTime, 
			             timeUserStart[j], startTime, format_value, 
                                     pntInfo[j].startNum, pntInfo[j].endNum);
            }
            else
            {
               #ifdef PRINT_DIAG
               printf ("******************************************************\n");
               printf ("Can't format weather summaries and icons for point\n");
               printf ("#%d as all elements needed to derive these are not\n",(j+1));
               printf ("available.\n");
               printf ("******************************************************\n");
               #endif
            }
         }

         /* Format Wave Height Values, if applicable. */
         if (weatherParameters[j][NDFD_WH] == 1)
            genWaveHeightValues(j, layoutKeys[j][NDFD_WH], match, parameters,
                                numRowsForPoint[j][NDFD_WH], 
                                pntInfo[j].startNum, pntInfo[j].endNum);

         /* Free some things before leaving this iteration of the point loop. */
         if (f_XML == 3 || f_XML == 4)
         {
            if (weatherParameters[j][NDFD_POP] == 1)
               free(maxDailyPop);
            if (weatherParameters[j][NDFD_SKY] == 1 ||
                weatherParameters[j][NDFD_SKY] == 2)
            {
               free(startPositions);
               free(endPositions);
               free(maxSkyNum);
               free(minSkyNum);     
               free(maxSkyCover);
               free(minSkyCover);
               free(averageSkyCover);
            }
	    if (weatherParameters[j][NDFD_WS] == 1 || 
                weatherParameters[j][NDFD_WS] == 2)
            {
               free(valTimeForWindDirMatch);
	       free(maxWindSpeed);
            }
            if (weatherParameters[j][NDFD_WD] == 1 || 
                weatherParameters[j][NDFD_WD] == 2)
	       free(maxWindDirection);
	 }

	 free(numRowsForPoint[j]);
         free(weatherParameters[j]);
         free(startDate[j]);
         free(currentDay[j]);
         free(currentHour[j]);

         for (k = 0; k < NDFD_MATCHALL + 1; k++)
         {
            if (weatherParameters[j][k] == 1 || weatherParameters[j][k] == 3)
               free(layoutKeys[j][k]);
         }

      }                       /* End of "is Point in Sectors" check. */

   }                          /* Close Parameters Point Loop. */

   /* Free layoutKeys array. */
   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
         free(layoutKeys[j]);
   }

   /* Free the static array "timeLayoutDefinitions" by destroying it. */
   dummyTimeLayout.period = 0;
   dummyTimeLayout.numRows = 0;
   dummyTimeLayout.fmtdStartTime[0] = '\0';
   isNewLayout(dummyTimeLayout, 0, 0, 1);

   /* Append <data> node to XML document. */
   xmlAddChild(dwml, data);

   /* Dump XML document to file or stdio (use "-" for stdio). */
   xmlSaveFormatFile("-", doc, 1);

   /* Free the document. */
   xmlFreeDoc(doc);

   /* Free the global variables that may have been allocated by parser. */
   xmlCleanupParser();

   /* This is to debug memory for regression tests. */
   xmlMemoryDump();

   /* Free some more memory. */
   free(layoutKeys);
   free(numRowsForPoint);
   free(weatherParameters);
   free(TZoffset);
   free(startDate);         
   free(currentHour);
   free(currentDay);
   free(timeUserStart);	 
   free(timeUserEnd);
   free(f_formatNIL);
   free(firstValidTimeMatch);
   free(firstValidTime_pop);
   free(firstValidTime_maxt);
   free(numDays);
   free(numOutputLines);
   free(f_formatIconForPnt);
   free(f_formatSummarizations);

   /* Free even some more memory. */
   free(f_pntHasData);
   free(collate);

   for (i = 0; i < numElem; i++)
   {
      genElemFree(elem + i);
   }
   free(elem);

   for (i = 0; i < numMatch; i++)
   {
      genMatchFree(match + i);
   }
   free(match);

   return 0;
}

#ifdef TEST_XML
int main(int argc, char **argv)
{
   int numInFiles;
   char **inFiles;
   int f_XML = 1;             /* version of xml... 1 = DWMLgen's
                               * "time-series" */

   numInFiles = 1;
   inFiles = (char **)malloc(sizeof(char *));
   myAssert(sizeof(char) == 1);
   inFiles[0] = (char *)malloc(strlen("maxt.bin") + 1);
   strcpy(inFiles[0], "maxt.bint");
}
#endif
