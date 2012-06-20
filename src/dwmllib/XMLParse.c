/******************************************************************************/
/* XMLParse () --
 * 
 * Paul Hershberg / MDL
 * Linux
 * PURPOSE
 *   The driver program that ultimately formats the DWMLgen products 
 *   "time-series" (RTMA and NDFD elements )and "glance" and the DWMLgenByDay 
 *   products (formats) "12 hourly" and 24 hourly". This compiled C code will 
 *   mirror the php coded web service in which requests for NDFD data over the 
 *   internet are sent back in DWML (Digital Weather Markup Language) format.
 * 
 * ARGUMENTS 
 *       f_XML = flag for 1 of the 6 DWML products:
 *               1 = DWMLgen's "time-series" product. 
 *               2 = DWMLgen's "glance" product.
 *               3 = DWMLgenByDay's "12 hourly" format product.
 *               4 = DWMLgenByDay's "24 hourly" format product.
 *               5 = DWMLgen's RTMA "time-series" product.
 *               6 = DWMLgen's RTMA + NDFD "time-series" product. (Input)
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
 *   9/2007 Paul Hershberg (MDL): Added 12 Climate Outlook Elements.
 *  10/2007 Paul Hershberg (MDL): Removed code that shifted data back by 1/2
 *                                the period length (bug from php code)
 *  11/2007 Paul Hershberg (MDL): Added 10 RTMA Elements.
 *   7/2008 Paul Hershberg (MDL): Accommodates Hazard Element (NDFD_WWA).
 *  10/2008 Paul Hershberg (MDL): Accommodates Hazards in Summary Products.
 *   9/2009 Paul Hershberg (MDL): Accomodates LAMP Tstm Prob.
 *
 * NOTES: The NDFD/RTMA element list is below. This contains all elements
 *        data can be returned for in DWML.
 *
 * enum { NDFD_MAX(0), NDFD_MIN(1), NDFD_POP(2), NDFD_TEMP(3), NDFD_WD(4), 
 *        NDFD_WS(5), NDFD_TD(6), NDFD_SKY(7), NDFD_QPF(8), NDFD_SNOW(9), 
 *        NDFD_WX(10), NDFD_WH(11), NDFD_AT(12), NDFD_RH(13), NDFD_WG(14), 
 *        NDFD_WWA(15), NDFD_INC34(16), NDFD_INC50(17), NDFD_INC64(18), NDFD_CUM34(19), 
 *        NDFD_CUM50(20), NDFD_CUM64(21), NDFD_CONHAZ(22), NDFD_PTORN(23),
 *        NDFD_PHAIL(24), NDFD_PTSTMWIND(25), NDFD_PXTORN(26), NDFD_PXHAIL(27), 
 *        NDFD_PXTSTMWIND(28), NDFD_PSTORM(29), NDFD_PXSTORM(30), 
 *        NDFD_TMPABV14D(31), NDFD_TMPBLW14D(32), NDFD_PRCPABV14D(33), 
 *        NDFD_PRCPBLW14D(34), NDFD_TMPABV30D(35), NDFD_TMPBLW30D(36),
 *        NDFD_PRCPABV30D(37), NDFD_PRCPBLW30D(38), NDFD_TMPABV90D(39), 
 *        NDFD_TMPBLW90D(40), NDFD_PRCPABV90D(41), NDFD_PRCPBLW90D(42), 
 *        LAMP_TSTMPRB(43), RTMA_PRECIPA(44), RTMA_SKY(45), RTMA_TD(46), RTMA_TEMP(47), 
 *        RTMA_UTD(48), RTMA_UTEMP(49), RTMA_UWDIR(50), RTMA_UWSPD(51), 
 *        RTMA_WDIR(52), RTMA_WSPD(53), NDFD_UNDEF(54), NDFD_MATCHALL(55)
 *      };
 *
 * enum { RTMA_NDFD_SKY(56) = NDFD_MATCHALL + 1, RTMA_NDFD_PRECIPA(57), 
 *        RTMA_NDFD_TD(58), RTMA_NDFD_TEMP(59), RTMA_NDFD_WDIR(60), 
 *        RTMA_NDFD_WSPD(61), XML_MAX(62) 
 *      };
 * 
 ****************************************************************************** 
 */
#include "xmlparse.h"
int XMLParse(uChar f_XML, size_t numPnts, Point * pnts,
             PntSectInfo * pntInfo, sChar f_pntType, char **labels,
             size_t *numInFiles, char ***inFiles, uChar f_fileType,
             sChar f_interp, sChar f_unit, double majEarth, double minEarth,
             sChar f_icon, sChar f_SimpleVer, sChar f_SimpleWWA, sChar f_valTime,
             double startTime, double endTime, size_t numNdfdVars, 
             uChar *ndfdVars, char *f_inTypes, char *gribFilter, 
             size_t numSector, char **sector, sChar f_ndfdConven, 
             char *rtmaDataDir, sChar f_avgInterp, char *lampDataDir)
{
   size_t numElem = 0;        /* Num of elements returned by genProbe (those 
                               * formatted plus those used in deriving 
                               * formatted elements). */
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
   int startNum = 0;    /* First index in match structure an individual point's
                         * data matches can be found. */
   int endNum = 0;      /* Last index in match structure an individual point's
                         * data matches can be found. */
   int numPopLines = 0; /* Since Pop, in the 24 hourly (f_MXL = 4) product still has 
		         * formatted data every 12 hours, we use this variable and 
		         * set it to numDays *2 when calling generateTimeLayout().
		         */
   int format_value = 1; /* Option to turn off the formating of the value child
                          * element for weather. (TPEX sets to zero, as they 
                          * don't format this). */
   uChar **weatherParameters = NULL; /* Array containing the flags denoting 
                                      * whether a certain element is to be 
                                      * formatted in the output XML (= 1), or
                                      * used in deriving another formatted 
                                      * element (= 2) or if the weather elements 
                                      * time layout is to be used by Icons (= 3)
                                      * (occurs when wx element not chosen but 
                                      * Icons are (-Icon 1). */ 
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
                     *        (rows) skipped at end of time duration.
                     * multiLayouts: Used in deriving individual hazards 
                     *               different layouts. */
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
   int f_conus2_5 = 0;        /* Denotes a match was found in the 2.5 km res
                                 conus sector (sector 0). */
   int numConus2_5 = 0;       /* Number of matches in conus2.5 sector. */
   int f_conus5 = 0;          /* Denotes a match was found in the 5 km res 
                                 conus sector (sector 0). */
   int numConus5 = 0;         /* Number of matches in conus5 sector. */
   int f_npacocn = 0;         /* Denotes a match was found in the npacocn sector
                               * (sector 6). */
   int numNpacocn = 0;        /* Number of matches in npacocn sector. If this 
                                 number is equal to numMatch, all matches are
                                 found in one sector (this can occur if there
                                 is a call for tropical wind thresholds for
                                 a call with hawaii and/or guam points. */
   int f_nhemi = 0;           /* Denotes a match was found in the nhemi sector
                               * (sector 5). */
   int numNhemi = 0;          /* Number of matches in nhemi sector. If this 
                                 number is equal to numMatch, all matches are
                                 found in one sector (this can occur if there
                                 is a call for tropical wind thresholds for
                                 a call with puertori and/or conus points. */
   int f_shiftData = 0; /* Flag used to determine whether we shift data back by
                           1/2 it's period to denote the duration of time the 
                           data is valid for (Soap Service), or to not shift, 
                           and simply use the data's endTime (validTime) 
                           (JimC's TPEX, FOX, products etc). */
   int f_rtmaNdfdTemp = 0; /* Flag denoting that user queried both NDFD 
                            * Temp and RTMA Temp. Thus, the two will be 
                            * conjoined into one element. */
   int *pnt_rtmaNdfdTemp = NULL; /* Point specific f_rtmaNdfdTemp */
   int f_rtmaNdfdTd = 0; /* Flag denoting that user queried both NDFD 
                          * Td and RTMA Td. Thus, the two will be 
                          * conjoined into one element. */
   int *pnt_rtmaNdfdTd = NULL; /* Point specific f_rtmaNdfdTd */
   int f_rtmaNdfdWdir = 0; /* Flag denoting that user queried both NDFD 
                            * Wind Dir and RTMA Wind Dir. Thus, the two 
                            * will be conjoined into one element. */
   int *pnt_rtmaNdfdWdir = NULL; /* Point specific f_rtmaNdfdWdir */
   int f_rtmaNdfdWspd = 0; /* Flag denoting that user queried both NDFD 
                            * Wind Spd and RTMA Wind Spd. Thus, the two 
                            * will be conjoined into one element. */
   int *pnt_rtmaNdfdWspd = NULL; /* Point specific f_rtmaNdfdWspd */
   int f_rtmaNdfdPrecipa = 0; /* Flag denoting that user queried both NDFD 
                               * QPF and RTMA Precip Amt. Thus, the two 
                               * will be conjoined into one element. */
   int *pnt_rtmaNdfdPrecipa = NULL; /* Point specific f_rtmaNdfdPrecipa */
   int f_rtmaNdfdSky = 0; /* Flag denoting that user queried both NDFD 
                           * Sky Cover and RTMA Sky Cover. Thus, the two
                           * will be conjoined into one element. */
   int *pnt_rtmaNdfdSky = NULL; /* Point specific f_rtmaNdfdSky */
   hazInfo **indivHaz = NULL; /* Array holding info about each hazard in the
                               * summary products. */
   int *numHazards = NULL; /* The number of hazards existing, per point. */
   double **periodTimes = NULL; /* The times bordering each forecast period. */
   int *numPeriodTimes = NULL;
   char tempBuff[30]; /* Temp string. */
   char units[40]; /* Holder for units (metric vs U.S. Standard). */
   int ndfdMaxIndex = -1; /* Index in elem array holding NDFD_MAXT. */
   int ndfdMinIndex = -1; /* Index in elem array holding NDFD_MINT. */ 
   int ndfdPopIndex = -1; /* Index in elem array holding NDFD_POP. */ 
   int ndfdWwaIndex = -1; /* Index in elem array holding NDFD_WWA. */
   int ndfdTempIndex = -1; /* Index in elem array holding NDFD_TEMP. */
   int ndfdTdIndex = -1; /* Index in elem array holding NDFD_TD. */
   int ndfdQpfIndex = -1; /* Index in elem array holding NDFD_QPF. */
   int ndfdSkyIndex = -1; /* Index in elem array holding NDFD_SKY. */
   int ndfdWdirIndex = -1; /* Index in elem array holding NDFD_WDIR. */
   int ndfdWspdIndex = -1; /* Index in elem array holding NDFD_WSPD. */
   int ndfdWgustIndex = -1; /* Index in elem array holding NDFD_WGUST. */
   int ndfdWgustOrWspdIndex = -1; /* Index in elem array holding either Wgust
                                   * if available, else Wspd. */ 
   int rtmaPrecipaIndex = -1; /* Index in elem array holding RTMA_PRECIPA. */
   int rtmaSkyIndex = -1; /* Index in elem array holding RTMA_SKY. */
   int rtmaTdIndex = -1; /* Index in elem array holding RTMA_TD. */ 
   int rtmaTempIndex = -1; /* Index in elem array holding RTMA_TEMP. */
   int rtmaWdirIndex = -1; /* Index in elem array holding RTMA_WDIR. */
   int rtmaWspdIndex = -1; /* Index in elem array holding RTMA_WSPD. */
   int rtmaNdfdSkyIndex = -1; /* Index in elem array holding RTMA_NDFD_SKY. */
   int rtmaNdfdPrecipaIndex = -1; /* Index in elem array holding 
                                   * RTMA_NDFD_PRECIPA. */
   int rtmaNdfdTdIndex = -1; /* Index in elem array holding RTMA_NDFD_TD. */
   int rtmaNdfdTempIndex = -1; /* Index in elem array holding RTMA_NDFD_TEMP. */
   int rtmaNdfdWdirIndex = -1; /* Index in elem array holding RTMA_NDFD_WDIR. */
   int rtmaNdfdWspdIndex = -1; /* Index in elem array holding RTMA_NDFD_WSPD. */
   int f_allTempsFormatted = 0; /* Flag denoting Temps were formatted for 
                                 * point. */
   int f_allTdsFormatted = 0; /* Flag denoting Td's were formatted for point. */
   int f_allQpfsFormatted = 0; /* Flag denoting QPF's were formatted for 
                                * point. */
   int f_allWspdsFormatted = 0; /* Flag denoting Wspd's were formatted for 
                                 * point. */
   int f_allWdirsFormatted = 0; /* Flag denoting Wdir's were formatted for 
                                 * point. */
   int f_allSkysFormatted = 0; /* Flag denoting Sky's were formatted for 
                                * point. */
   int numHazardsAllPts = 0; /* Used for summary products. If there are no 
                              * hazards at all in a list of points, then we 
                              * can copy a previous points time layouts (if 
                              * in same time zone). But if there are active 
                              * hazards, we can't do this copy. Set here to +1
                              * so time-series and glance products are not 
                              * effected. Re-initialize to 0 for summary 
                              * products. */

   /* Denote the order needed in array NDFD2DWML of the index of the enumeration. 
    * e.g. NDFD2DWML[NDFD_POP] = NDFD2DWML[2] = 8TH Position needed in 
    * formatting. 
    */
   static uChar NDFD2DWML[] = { 0, 1, 8, 2, 44, 40, 3, 45, 5, 6, 7, 49, 4, 46,
   41, 48, 33, 34, 35, 36, 37, 38, 39, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
   21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 42, 43, 51, 52, 53, 54, 55, 56, 57,
   58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69
   };

/*   static int DWML2NDFD[] = { 0, 1, 3, 6, 12, 8, 9, 2, 22, 23, 24, 25, 26, 27, 
     28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 16, 17, 18, 
     19, 20, 21, 5, 14, 4, 7, 13, 10, 15, 11, 43, 44, 45, 46, 47, 48, 49, 50, 
     51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62
     }; 
*/
  
   dwmlEnum *Dwml;  /* Subset of NDFD2DWML above that holds the DWML 
                     * enumerations (the order the elements need to be 
                     * formatted in, dictated by schema). Array holds 
                     * just those elemetns queried for. */

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

   /* Check to see if this is an RTMA query only (f_XML will be set to 5) or
    * a mix of both RTMA elements and NDFD elements (f_XML will be set to 6).
    * This check can only occur if incoming f_XML type is 1 (time-series), 
    * which can encompass both f_XML = 5 or 6. If query has RTMA elements, 
    * upate the inFiles array and numInFiles accordingly.
    */
   if (f_XML == 1) 
   {
      anyRtmaElements(&f_XML, numInFiles, inFiles, numNdfdVars, ndfdVars,
                      rtmaDataDir, f_icon, numSector, sector, &f_rtmaNdfdTemp,
                      &f_rtmaNdfdTd, &f_rtmaNdfdWdir, &f_rtmaNdfdWspd, 
                      &f_rtmaNdfdPrecipa, &f_rtmaNdfdSky);

      /* Check to see if there were any LAMP elements. */
      anyLampElements(numInFiles, inFiles, numNdfdVars, ndfdVars,
                      lampDataDir);
   }

   /* Prepare the variable filter array to show what NDFD variables are of
    * interest(1) or vital(2) to this procedure. Also, manipulate the varFilter
    * array depending on type of XML queried.
    */
   prepareVarFilter(f_XML, &f_icon, numNdfdVars, ndfdVars, varFilter, &numElem,
                    &elem);

   /* See if any points are outside the NDFD Sectors. Print error message to
    * standard error if so. If all points selected are outside the NDFD
    * Sectors, exit routine. 
    */
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
         {
            endTime = endTime + (24 * 3600);
         }
      }
   }

   /*********************** GRAB THE NDFD DATA AND SORT. *********************/
   /**************************************************************************/
   /* f_WxParse = 0, is the flag to return WX as ugly weather codes. */
   f_WxParse = 0;
   if (genProbe(numPnts, pnts, f_pntType, *numInFiles, *inFiles, f_fileType,
                f_interp, f_unit, majEarth, minEarth, f_WxParse,
                f_SimpleVer, f_SimpleWWA, numElem, elem, f_valTime, startTime, endTime,
                f_XML, &numMatch, &match, f_inTypes, gribFilter, numSector,
                sector, f_ndfdConven, f_avgInterp) != 0)
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
      #ifdef PRINT_DIAG
      printf("No data retrieved from NDFD (matches = 0).\n");
      #endif

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
   
   /* Sort the matches by sector, element, and then by valid time. */
   qsort(match, numMatch, sizeof(match[0]), XMLmatchCompare);

   /* Allocate f_pntNoData. */
   f_pntHasData = calloc(numPnts, sizeof(char));

   /* Collate the matches. While accessing match structure, get infomation on
    * whether there are any matches in the "nhemi" and "npacocn" sectors.
    */
   curTime = -1;
   for (i = 0; i < numMatch; i++)
   {
      if (match[i].f_sector == 0) /* Enumerated conus5 sector # equals 0. */
         numConus5++;
      if (match[i].f_sector == 1) /* Enumerated conus2.5 sector # equals 1. */
         numConus2_5++;
      if (match[i].f_sector == 6) /* Enumerated nhemi sector # equals 6. */
         numNhemi++;
      if (match[i].f_sector == 7) /* Enumerated npacocn sector # equals 7. */
         numNpacocn++;
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
      printf ("Pnt = %d --------------------------------------\n",j);
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

   /**************** DEAL WITH POINTS IN DIFFERENT SECTORS. ******************/
   /**************************************************************************/

   /* Gather up sector information for times when a multiple point call has
    * points in different sectors. Place this info in the Point's informational
    * structure "pntInfo". 
    */
   if (numConus2_5 > 0)
      f_conus2_5 = 1;
   if (numConus5 > 0)
      f_conus5 = 1;
   if (numNhemi > 0)
      f_nhemi = 1;
    if (numNpacocn > 0)
      f_npacocn = 1;
   getSectorInfo(pntInfo, pnts, numPnts, match, numMatch, numSector, sector,
                 f_conus2_5, numConus2_5, f_conus5, numConus5, f_nhemi, numNhemi, 
                 f_npacocn, numNpacocn);

#ifdef PRINT_DIAG
   for (i = 0; i < numPnts; i++)
   {
      printf ("pntInfo[%d].numSector = %d\n",i,pntInfo[i].numSector);
      for (j = 0;j < pntInfo[i].numSector;j++)
         printf ("pntInfo[%d].f_sector[%d] = %d \n",i,j,pntInfo[i].f_sector[j]);
      printf ("pntInfo[%d].startNum, endNum = %d, %d\n",i,pntInfo[i].startNum,pntInfo[i].endNum);
   }
#endif

   /*************************************************************************** 
    *                    START FORMATTING THE XML/DWML                        *
    *                                                                         * 
    ************************* FORMAT HEADER INFO ******************************/

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
   currentDay = (char **)malloc(numPnts * sizeof(char *));
   currentHour = (char **)malloc(numPnts * sizeof(char *));

   for (j = 0; j < numPnts; j++)
   {
      firstValidTimeMatch[j] = 0.0;
      firstValidTime_pop[j] = 0.0;
      firstValidTime_maxt[j] = 0.0;
      numDays[j] = 0;
 
      /* weatherParameters array denotes those elements that will ultimately
       * be formatted (set to 1). Those returned in genProbe via the match 
       * structure but used only to derive other elements are set to 2.
       * Initialize to 0. 
       */
      weatherParameters[j] = (uChar *) malloc(numElem * sizeof(uChar));
      for (i = 0; i < numElem; i++)
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
   }
 
   /* Prepare some data for DWMLgen's "time-series" product. */
   if ((f_XML == 1) || (f_XML == 2) || (f_XML == 5) || (f_XML == 6))
      prepareDWMLgen(f_XML, &f_formatPeriodName, &weatherParameters, numPnts,
                     whatSummarization, varFilter, &f_icon, &numElem, &elem);

   /* Prepare data for DWMLgenByDay's "12 hourly" & "24 hourly" products. */
   if ((f_XML == 3) || (f_XML == 4))
      prepareDWMLgenByDay(match, f_XML, &startTime, &endTime, 
		          firstValidTimeMatch, numDays, format, 
			  &f_formatPeriodName, weatherParameters, 
			  &timeInterval, numOutputLines, whatSummarization,
			  currentDoubTime, numPnts, pntInfo, currentDay, 
                          numElem, elem, varFilter);

   /************************* FORMAT DATA INFO *******************************/
   /**************************************************************************/ 

   /* First format the <LOCATION> ELEMENT in the XML/DWML. */
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
   pnt_rtmaNdfdTemp = (int *)malloc(numPnts * sizeof(int));
   pnt_rtmaNdfdTd = (int *)malloc(numPnts * sizeof(int)); 
   pnt_rtmaNdfdWdir = (int *)malloc(numPnts * sizeof(int)); 
   pnt_rtmaNdfdWspd = (int *)malloc(numPnts * sizeof(int)); 
   pnt_rtmaNdfdPrecipa = (int *)malloc(numPnts * sizeof(int)); 
   pnt_rtmaNdfdSky = (int *)malloc(numPnts * sizeof(int));

   /* See if some elements were queried for. If so, denote the index they 
    * occur in in the elem array. 
    */
   getElemIndexes(&ndfdMaxIndex, &ndfdMinIndex, &ndfdPopIndex, &ndfdWwaIndex, 
                  &ndfdTempIndex, &ndfdTdIndex, &ndfdQpfIndex, &ndfdWspdIndex, 
                  &ndfdWdirIndex, &ndfdSkyIndex, &ndfdWgustIndex, 
                  &rtmaPrecipaIndex, &rtmaSkyIndex, &rtmaTdIndex, 
                  &rtmaTempIndex, &rtmaWdirIndex, &rtmaWspdIndex, 
                  &rtmaNdfdSkyIndex, &rtmaNdfdPrecipaIndex, &rtmaNdfdTdIndex, 
                  &rtmaNdfdTempIndex, &rtmaNdfdWdirIndex, &rtmaNdfdWspdIndex, 
                  f_XML, numElem, elem);

   if (f_XML == 3 || f_XML == 4)
   {
      indivHaz = (hazInfo **)malloc(numPnts * sizeof(hazInfo *));
      numHazards = (int *)calloc(numPnts, sizeof(int));
   }

   for (j = 0; j < numPnts; j++)
   {
      /* Initialize a few things. */
      if (f_icon == 1)
         f_formatIconForPnt[j] = 1;
      else
         f_formatIconForPnt[j] = 0;
 
      f_formatSummarizations[j] = 1;
      
      pnt_rtmaNdfdTemp[j] = f_rtmaNdfdTemp; 
      pnt_rtmaNdfdTd[j] = f_rtmaNdfdTd; 
      pnt_rtmaNdfdWspd[j] = f_rtmaNdfdWspd; 
      pnt_rtmaNdfdWdir[j] = f_rtmaNdfdWdir; 
      pnt_rtmaNdfdPrecipa[j] = f_rtmaNdfdPrecipa; 
      pnt_rtmaNdfdSky[j] = f_rtmaNdfdSky; 

      if (isPntInASector(pnts[j]))
      {
	 /* Open up each point's # of Rows to an element allocation. */
         numRowsForPoint[j] = (numRowsInfo *) malloc(numElem *
                                               sizeof(numRowsInfo));

	 /* Fill/Get the startDate array. */
         getStartDates(startDate, f_XML, startTime, firstValidTimeMatch[j], 
		       firstValidTime_maxt[j], pntInfo[j].timeZone, 
		       pntInfo[j].f_dayLight, j);

         getNumRows(numRowsForPoint[j], &timeUserStart[j], &timeUserEnd[j], 
                    numMatch, match, weatherParameters[j], f_XML, &f_icon,
                    pntInfo[j].timeZone, pntInfo[j].f_dayLight,
                    pntInfo[j].startNum, pntInfo[j].endNum, startDate[j], 
                    &numDays[j], startTime, endTime, currentHour[j], 
		    &firstValidTime_pop[j], &firstValidTimeMatch[j], 
                    &f_formatIconForPnt[j], &f_formatSummarizations[j], j, 
                    &pnt_rtmaNdfdTemp[j], &pnt_rtmaNdfdTd[j], 
                    &pnt_rtmaNdfdWdir[j], &pnt_rtmaNdfdWspd[j], 
                    &pnt_rtmaNdfdPrecipa[j], &pnt_rtmaNdfdSky[j], 
                    currentDoubTime, numElem, elem);

         /* Gather up needed Hazard Info if a Summary Product was chosen. */
         if ((f_XML == 3 || f_XML == 4) && ndfdWwaIndex >= 0)
         {
            if (numRowsForPoint[j][ndfdWwaIndex].total != 0)
            {
               collectHazInfo(match, numMatch, pntInfo[j].startNum, 
                              pntInfo[j].endNum, numRowsForPoint[j][ndfdWwaIndex],
                              j, &(indivHaz[j]), &numHazards[j]);

               /* Count up total number of hazards if a summary product. */
               numHazardsAllPts = numHazardsAllPts + numHazards[j];
            }
         }
      }
   }

   /**********FORMAT <MOREWEATHERINFORMATION> ELEMENT IN XML/DWML*************/
   /**************************************************************************/
   formatMoreWxInfo(numPnts, pnts, data);
   
   /***************FORMAT <TIME-LAYOUT> ELEMENT IN XML/DWML*******************/

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
         layoutKeys[j] = (char **)malloc(numElem * sizeof(char *));

         if (f_firstPointLoopIteration || f_formatNewPointTimeLayouts || 
            ((f_XML == 3 || f_XML == 4) && numHazardsAllPts > 0))
         {
            /* Generate a new set of time-layouts for the point. */
            for (k = 0; k < numElem; k++)
            {
               /* Pass on the RTMA errors. They'll share the time-layouts of 
                * the corresponding parent element. Also, pass on the 
                * individual elements of the concatenated RTMA + NDFD elements.
                */
               if ((elem[k].ndfdEnum == RTMA_UTEMP) || 
                   (elem[k].ndfdEnum == RTMA_UTD) || 
                   (elem[k].ndfdEnum == RTMA_UWSPD) || 
                   (elem[k].ndfdEnum == RTMA_UWDIR) || 
                   (elem[k].ndfdEnum == NDFD_TEMP && pnt_rtmaNdfdTemp[j]) ||
                   (elem[k].ndfdEnum == RTMA_TEMP && pnt_rtmaNdfdTemp[j]) || 
                   (elem[k].ndfdEnum == NDFD_TD && pnt_rtmaNdfdTd[j]) || 
                   (elem[k].ndfdEnum == RTMA_TD && pnt_rtmaNdfdTd[j]) || 
                   (elem[k].ndfdEnum == NDFD_WS && pnt_rtmaNdfdWspd[j]) || 
                   (elem[k].ndfdEnum == RTMA_WSPD && pnt_rtmaNdfdWspd[j]) || 
                   (elem[k].ndfdEnum == NDFD_WD && pnt_rtmaNdfdWdir[j]) || 
                   (elem[k].ndfdEnum == RTMA_WDIR && pnt_rtmaNdfdWdir[j]) || 
                   (elem[k].ndfdEnum == NDFD_QPF && pnt_rtmaNdfdPrecipa[j]) || 
                   (elem[k].ndfdEnum == RTMA_PRECIPA && pnt_rtmaNdfdPrecipa[j]) || 
                   (elem[k].ndfdEnum == NDFD_SKY && pnt_rtmaNdfdSky[j]) || 
                   (elem[k].ndfdEnum == RTMA_SKY && pnt_rtmaNdfdSky[j]))
               {
                  continue;
               }
               else
               {
                  if (weatherParameters[j][k] == 1 || weatherParameters[j][k] == 3)
                  {
                     /* For DWMLgen "time-series" and "glance" products.  */
                     if ((f_XML == 1 || f_XML == 2 || f_XML == 5 || f_XML == 6) 
                          && numRowsForPoint[j][k].total != 0)
                     {
                        /* One of the concatenated elements? */
                        if (elem[k].ndfdEnum == RTMA_NDFD_TEMP || 
                            elem[k].ndfdEnum == RTMA_NDFD_TD || 
                            elem[k].ndfdEnum == RTMA_NDFD_WSPD || 
                            elem[k].ndfdEnum == RTMA_NDFD_WDIR || 
                            elem[k].ndfdEnum == RTMA_NDFD_PRECIPA || 
                            elem[k].ndfdEnum == RTMA_NDFD_SKY)
                        {
                           generateConcatTimeLayout(numRowsForPoint[j], k, 
                                                    elem[k].ndfdEnum, layoutKey,
                                                    whichTimeCoordinate,
                                                    whatSummarization, match, 
                                                    numMatch, f_formatPeriodName, 
                                                    TZoffset[j], f_observeDST, 
                                                    &numLayoutSoFar, 
                                                    &numCurrentLayout, 
                                                    currentHour[j], currentDay[j], 
                                                    "boggus", data, startTime,
                                                    currentDoubTime, f_XML, 
                                                    startNum, endNum, numElem, 
                                                    elem);
                        }
                        else
                        { 
                           generateTimeLayout(numRowsForPoint[j][k], 
                                              elem[k].ndfdEnum, layoutKey,
                                              whichTimeCoordinate,
                                              whatSummarization, match, numMatch,
                                              f_formatPeriodName, TZoffset[j],
                                              f_observeDST, &numLayoutSoFar,
                                              &numCurrentLayout, currentHour[j],
                                              currentDay[j], "boggus", data, 
                                              startTime, currentDoubTime, 
                                              &numDays[j], f_XML, 
                                              startNum, endNum);
                        }
                        layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                        strcpy(layoutKeys[j][k], layoutKey);
                     }

                     /* For DWMLgenByDay product w/ format == "12 hourly". */
                     else if (f_XML == 3 && numRowsForPoint[j][k].total != 0)
                     {
                        if (elem[k].ndfdEnum == NDFD_MAX || 
                            elem[k].ndfdEnum == NDFD_MIN)
                        {
                           generateTimeLayout(numRowsForPoint[j][k], 
                                              elem[k].ndfdEnum, layoutKey,
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
                        else if ((weatherParameters[j][k] == 1 || 
                                 weatherParameters[j][k] == 3) && 
                                 (elem[k].ndfdEnum != NDFD_WWA))
                        /* POP, Wx, and Icons. */
                        {
                           /* The other element's (Wx and Icons) will share 
		   	    * Pop's layout.
			    */
                           if (ndfdPopIndex >= 0)
                           { 
                              if (numRowsForPoint[j][ndfdPopIndex].total != 0)
                              {
                                 generateTimeLayout(numRowsForPoint[j][ndfdPopIndex],
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
                        }

                        /* Check to see if this is a case when no active hazards
                         * occurring. Then we need to create a time-layout for
                         * this occurrence. We'll need to use the POP, MAXT, or
                         * MINT time layout.
                         */
                        if (elem[k].ndfdEnum == NDFD_WWA && numHazards[j] == 0)
                        {
                           /* First, see if we can use the POP time layout to 
                            * create the new "entire" forecast period duration 
                            * layout. If not, use MAXT or MINT time layout.
                            */
                           if (ndfdPopIndex >= 0)
                           {
                              if (numRowsForPoint[j][ndfdPopIndex].total != 0)
                                 strcpy (tempBuff, layoutKeys[j][ndfdPopIndex]);
                           }
                           else if (f_useMinTempTimes[j] && ndfdMinIndex > 0)
                           {
                              if (numRowsForPoint[j][ndfdMinIndex].total != 0)
                                 strcpy (tempBuff, layoutKeys[j][ndfdMinIndex]);
                           }
                           else if (!f_useMinTempTimes[j]) 
                           {
                              if (ndfdMaxIndex >= 0)
                              {
                                 if (numRowsForPoint[j][ndfdMaxIndex].total != 0)
                                    strcpy (tempBuff, layoutKeys[j][ndfdMaxIndex]);
                              }                     
                              else if (ndfdMinIndex >= 0)
                              {
                                 if (numRowsForPoint[j][ndfdMinIndex].total != 0)
                                    strcpy (tempBuff, layoutKeys[j][ndfdMinIndex]);
                              } 
                           }
                           generateNoHazTimeLayout(tempBuff, layoutKey, 
                                                   whichTimeCoordinate, 
                                                   whatSummarization, 
                                                   TZoffset[j], f_observeDST,
                                                   &numLayoutSoFar, 
                                                   &numCurrentLayout,
                                                   timeUserStart[j], 
                                                   numDays[j], data);

                           layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                           strcpy(layoutKeys[j][k], layoutKey);
                        }
                     }

                     /* For DWMLgenByDay product w/ format == "24 hourly".
                      * Since the product is DWMLgenByDay's "24 hourly"
                      * format, force the weather (and icon) elements so that
                      * their time layout equals that of MaxT and/or MinT
                      * since their periods are = to 24 hours also (the
                      * exception is Pop, since it still has to use a 12
                      * hourly summariztion). 
		      */
                     else if (f_XML == 4 && numRowsForPoint[j][k].total != 0)
                     {
                        if (elem[k].ndfdEnum != NDFD_POP && 
                            elem[k].ndfdEnum != NDFD_WWA)
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
					 numRowsForPoint[j][ndfdMaxIndex]);

                           if ((atoi(currentHour[j]) > 18) && (currentLocalTime_doub_adj
                               + 86400 == firstMaxTValidTime_doub_adj) && 
                               (firstValidTime_maxt[j] <= firstValidTime_pop[j])) 
                           {  
                              if (ndfdMinIndex >= 0)
                              {
                                 if (numRowsForPoint[j][ndfdMinIndex].total != 0)
                                 {
                                    f_formatNIL[j] = 1;
                                    f_useMinTempTimes[j] = 1;

                                    generateTimeLayout(numRowsForPoint[j][ndfdMinIndex],
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
                              }
                           }
                           else  /* Use MaxT's own time-layout, but check to 
                                  * make sure it is available first. If MaxT is
                                  * not available, then use MinT's time layout.
                                  */
                           {
                              if (ndfdMaxIndex >= 0)
                              {
                                 if (numRowsForPoint[j][ndfdMaxIndex].total != 0)
                                 {
                                    f_formatNIL[j] = 0;
                                    f_useMinTempTimes[j] = 0;

                                    generateTimeLayout(numRowsForPoint[j][ndfdMaxIndex],
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
                              else if (ndfdMinIndex >= 0)
                              {
                                 if (numRowsForPoint[j][ndfdMinIndex].total != 0)
                                 {
                                    f_formatNIL[j] = 1;
                                    f_useMinTempTimes[j] = 1;

                                    generateTimeLayout(numRowsForPoint[j][ndfdMinIndex],
					            NDFD_MIN, layoutKey,
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
                           }
                        }
                        else if ((weatherParameters[j][k] == 1 || 
                                 weatherParameters[j][k] == 3) && 
                                 (elem[k].ndfdEnum != NDFD_WWA))
                        {
                           /* POP gets its own time-layout, using
                            * 12-hourly summarization and format (force
                            * these), even though f_XML == 4. 
			    */
                           if (ndfdPopIndex >= 0)
                           {
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
                        }

                        /* Check to see if this is a case when no active hazards
                         * occurring. Then we need to create a time-layout for
                         * this occurrence. We'll need to use the POP, MAXT, or
                         * MINT time layout.
                         */
                        if (elem[k].ndfdEnum == NDFD_WWA && numHazards[j] == 0)
                        {
                           /* First, see if we can use the POP time layout to 
                            * create the new "entire" forecast period duration 
                            * layout. If not, use MAXT or MINT time layout.
                            */
                           if (ndfdPopIndex >= 0)
                           {
                              if (numRowsForPoint[j][ndfdPopIndex].total != 0)
                                 strcpy (tempBuff, layoutKeys[j][ndfdPopIndex]);
                           }
                           else if (f_useMinTempTimes[j] && ndfdMinIndex >= 0)
                           {
                              if (numRowsForPoint[j][ndfdMinIndex].total != 0)
                                 strcpy (tempBuff, layoutKeys[j][ndfdMinIndex]);
                           }
                           else if (!f_useMinTempTimes[j]) 
                           {
                              if (ndfdMaxIndex >= 0)
                              {
                                 if (numRowsForPoint[j][ndfdMaxIndex].total != 0)
                                    strcpy (tempBuff, layoutKeys[j][ndfdMaxIndex]);
                              }                     
                              else if (ndfdMinIndex >= 0)
                              {
                                 if (numRowsForPoint[j][ndfdMinIndex].total != 0)
                                    strcpy (tempBuff, layoutKeys[j][ndfdMinIndex]);
                              } 
                           }

                           generateNoHazTimeLayout(tempBuff, layoutKey, 
                                                   whichTimeCoordinate, 
                                                   whatSummarization, 
                                                   TZoffset[j], f_observeDST,
                                                   &numLayoutSoFar, 
                                                   &numCurrentLayout,
                                                   timeUserStart[j], 
                                                   numDays[j], data);

                           layoutKeys[j][k] = malloc(strlen(layoutKey) + 1);
                           strcpy(layoutKeys[j][k], layoutKey);
                        }
                     } /* End of "f_XML type" check. */
                  } /* End weatherParameters "if" statement. */
               } /* End passing RTMA errors & individual concatenated elements. */
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
                  for (k = 0; k < numElem; k++)
                  {
                     if ((elem[k].ndfdEnum == RTMA_UTEMP) || (elem[k].ndfdEnum == RTMA_UTD) || (elem[k].ndfdEnum == RTMA_UWSPD) || 
                         (elem[k].ndfdEnum ==RTMA_UWDIR) || (elem[k].ndfdEnum ==NDFD_TEMP && pnt_rtmaNdfdTemp[j]) ||
                         (elem[k].ndfdEnum == RTMA_TEMP && pnt_rtmaNdfdTemp[j]) || 
                         (elem[k].ndfdEnum == NDFD_TD && pnt_rtmaNdfdTd[j]) || 
                         (elem[k].ndfdEnum == RTMA_TD && pnt_rtmaNdfdTd[j]) || 
                         (elem[k].ndfdEnum == NDFD_WS && pnt_rtmaNdfdWspd[j]) || 
                         (elem[k].ndfdEnum == RTMA_WSPD && pnt_rtmaNdfdWspd[j]) || 
                         (elem[k].ndfdEnum == NDFD_WD && pnt_rtmaNdfdWdir[j]) || 
                         (elem[k].ndfdEnum == RTMA_WDIR && pnt_rtmaNdfdWdir[j]) || 
                         (elem[k].ndfdEnum == NDFD_QPF && pnt_rtmaNdfdPrecipa[j]) || 
                         (elem[k].ndfdEnum == RTMA_PRECIPA && pnt_rtmaNdfdPrecipa[j]) || 
                         (elem[k].ndfdEnum == NDFD_SKY && pnt_rtmaNdfdSky[j]) || 
                         (elem[k].ndfdEnum == RTMA_SKY && pnt_rtmaNdfdSky[j]))
                     {
                        continue;
                     }
                     else
                     {
                        if (weatherParameters[j][k] == 1 || weatherParameters[j][k] == 3)
                        {
                           if (f_XML == 3 || f_XML == 4)
                           {
                              if (elem[k].ndfdEnum == NDFD_WWA && numHazards[i] != 0)
                                 continue;
                              else
                              {
                                 layoutKeys[j][k] = malloc(strlen(layoutKeys[i][k]) + 1);
                                 strcpy(layoutKeys[j][k], layoutKeys[i][k]);
                              }
                           }
                           else
                           {
                              layoutKeys[j][k] = malloc(strlen(layoutKeys[i][k]) + 1);
                              strcpy(layoutKeys[j][k], layoutKeys[i][k]);
                           }
                        }
                     }
                  }
               }
            }
         }  /* End of "if-else" of "format New Point time layout" check. */
	 f_firstPointLoopIteration = 0;
      }  /* End of "is Point in Sector" check. */
   }  /* End "Point Loop" for Time-Layouts. */

   /* If the product is a summary, a point's individual hazards can have 
    * differing time layouts. They can also differ from previous points' time 
    * layouts for the individual hazards. Account for this, and, retrieve the 
    * times that border each forecast period. Only applicable when there are 
    * active hazards. 
    */
   if ((f_XML == 3 || f_XML == 4) && ndfdWwaIndex >= 0)
   {
      periodTimes = (double **)malloc(numPnts * sizeof(double *));
      numPeriodTimes = (int *)malloc(numPnts * sizeof(int));
      for (j = 0; j < numPnts; j++)
      {
         if (numHazards[j] != 0) /* Point contains active hazards. */
         {
            hazTimeInfo(&(numRowsForPoint[j][ndfdWwaIndex].multiLayouts), 
                        whichTimeCoordinate, indivHaz[j], numHazards[j], 
                        whatSummarization, match, numMatch, pntInfo[j].timeZone, 
                        pntInfo[j].f_dayLight, currentHour[j], currentDay[j], 
                        format, data, startTime, currentDoubTime, f_XML, startNum, 
                        endNum, timeUserStart[j], timeUserEnd[j], timeInterval,
                        &(periodTimes[j]), &(numPeriodTimes[j]), &numLayoutSoFar,
                        &numCurrentLayout);
         }
      }
   }

   /************** FORMAT PARAMETER <ELEMENT> IN XML/DWML ********************/
   /**************************************************************************/

   /* Firstly, we need to get a new order of enumerations, strictly for the way
    * the DWML elements need to be formatted. We'll call these DWML 
    * enumerations. We need to collect them for the elements queried for. 
    * Collect this subset from the NDFD2DWML[] array.
    */
   Dwml = (dwmlEnum *) malloc(numElem * sizeof(dwmlEnum));

   for (k = 0; k < numElem; k++)
   {
      Dwml[k].Ndfd2Dwml = NDFD2DWML[elem[k].ndfdEnum];
      Dwml[k].origNdfdIndex = k;
   }

   /* Don't use qsort, use a simple Bubble Sort for the new order. */
   dwmlEnumSort(numElem, &Dwml);

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
         for (k = 0; k < numElem; k++)
         {
            /************************MAXIMUM TEMPS***************************/

             /* Format Maximum Temperature Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_MAX] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genMaxTempValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], match, 
                                parameters, f_formatNIL[j], f_XML, startTime,
                                numRowsForPoint[j][Dwml[k].origNdfdIndex],      
                                numDays[j], pntInfo[j].startNum, 
                                pntInfo[j].endNum, f_unit);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_MAX] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;
         
            /************************MINIMUM TEMPS***************************/
   
            /* Format Minimum Temperature Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_MIN] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genMinTempValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], match, 
                                parameters, f_XML, startTime, 
                                numRowsForPoint[j][Dwml[k].origNdfdIndex],
	   		        currentDay[j], currentHour[j], TZoffset[j], 
                                pntInfo[j].f_dayLight, numDays[j], 
                                pntInfo[j].startNum, pntInfo[j].endNum, 
                                f_unit);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_MIN] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;
         
            /************************HOURLY TEMPS*****************************/

            /* Format Hourly RTMA Temperature Values + NDFD Temperature values,
             * if applicable. Concatenate the two together.
             */
            if ((!f_allTempsFormatted) && (ndfdTempIndex >= 0 || 
                 rtmaTempIndex >= 0 || rtmaNdfdTempIndex >= 0))
            {
               if (f_XML == 5 || f_XML == 6)
               {
                  if (pnt_rtmaNdfdTemp[j] && rtmaNdfdTempIndex >= 0 && 
                      ndfdTempIndex >= 0 && rtmaTempIndex >= 0)
                  {
                     if (weatherParameters[j][ndfdTempIndex] == 1 && 
                         weatherParameters[j][rtmaNdfdTempIndex] == 1 && 
                         weatherParameters[j][rtmaTempIndex] == 1)
                     {
                        if (f_unit != 2)
                           strcpy (units, "Fahrenheit");
                        else
                           strcpy (units, "Celsius");

                        concatRtmaNdfdValues(j, layoutKeys[j][rtmaNdfdTempIndex], 
                                          match, NDFD_TEMP, RTMA_TEMP, 
                                          "Temperature", "temperature", 
                                          "hourly", units, parameters, 
                                          numRowsForPoint[j][ndfdTempIndex], 
                                          numRowsForPoint[j][rtmaTempIndex], 
                                          pntInfo[j].startNum, 
                                          pntInfo[j].endNum);
                        f_allTempsFormatted = 1;
                     }
                  }

                  /* Format the Real Time Mesoscale Analyses for Hourly Temperature, if  
                   * applicable. 
	           */
                  else if (!pnt_rtmaNdfdTemp[j] && rtmaTempIndex >= 0 && 
                           ndfdTempIndex < 0)
                  {
                     if (weatherParameters[j][rtmaTempIndex] == 1)
                     {
                        if (f_unit != 2) 
                           strcpy (units, "Fahrenheit");
                        else 
                          strcpy (units, "Celsius");

                        genRtmaValues(j, layoutKeys[j][rtmaTempIndex], 
                                      RTMA_TEMP, RTMA_UTEMP, match, 
                                      "RTMA Temperature", "temperature", 
                                      "rtma-hourly", units, parameters, 
                                      numRowsForPoint[j][rtmaTempIndex], 
                                      pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allTempsFormatted = 1;
                     }
                  }   
                  else if (!pnt_rtmaNdfdTemp[j] && rtmaNdfdTempIndex >= 0 && 
                           ndfdTempIndex >= 0 && rtmaTempIndex >= 0)
                  {
                     /* This is the specific case where query was for NDFD 
                      * element and RTMA part of that element, but the RTMA 
                      * part doesn't exist (occurs in Puerto Rico). 
                      */
                     if (weatherParameters[j][ndfdTempIndex] == 1)
                     {
                        genTempValues(j, layoutKeys[j][ndfdTempIndex], match, 
                                   parameters, numRowsForPoint[j][ndfdTempIndex], 
                                   pntInfo[j].startNum, pntInfo[j].endNum, 
                                   f_unit);
                        f_allTempsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdTempIndex] == 0)
                     {
                        f_allTempsFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdTemp[j] && rtmaNdfdTempIndex < 0 && 
                           ndfdTempIndex >= 0 && rtmaTempIndex < 0)
                  {
                     if (weatherParameters[j][ndfdTempIndex] == 1)
                     {
                        genTempValues(j, layoutKeys[j][ndfdTempIndex], match, 
                                   parameters, numRowsForPoint[j][ndfdTempIndex], 
                                   pntInfo[j].startNum, pntInfo[j].endNum, 
                                   f_unit);
                        f_allTempsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdTempIndex] == 0)
                     {
                        f_allTempsFormatted = 1;
                     }
                  }
               }
               else if (f_XML == 1 && ndfdTempIndex >= 0)
               {
                  /* Format Hourly NDFD Temperature Values, if applicable. */
                  if (weatherParameters[j][ndfdTempIndex] == 1)
                  { 
                     genTempValues(j, layoutKeys[j][ndfdTempIndex], match, 
                                   parameters, numRowsForPoint[j][ndfdTempIndex],
                                   pntInfo[j].startNum, pntInfo[j].endNum, 
                                   f_unit);
                     f_allTempsFormatted = 1;
                  }
                  else if (weatherParameters[j][ndfdTempIndex] == 0)
                  {
                     f_allTempsFormatted = 1;
                  }
               }
            }

            /************************DEW POINT TEMPS*****************************/

            /* Format Hourly RTMA + NDFD Dew Point Temperature values, if
             * applicable. Concatenate the two together.
             */
            if ((!f_allTdsFormatted) && (ndfdTdIndex >= 0 || rtmaTdIndex >= 0
                 || rtmaNdfdTdIndex >= 0))
            {
               if (f_XML == 5 || f_XML == 6)
               {
                  if (pnt_rtmaNdfdTd[j] && rtmaNdfdTdIndex >= 0 && 
                      ndfdTdIndex >= 0 && rtmaTdIndex >= 0)
                  {
                     if (weatherParameters[j][ndfdTdIndex] == 1 && 
                         weatherParameters[j][rtmaNdfdTdIndex] == 1 && 
                         weatherParameters[j][rtmaTdIndex] == 1)
                     {
                        if (f_unit != 2)
                           strcpy (units, "Fahrenheit");
                        else
                          strcpy (units, "Celsius");

                        concatRtmaNdfdValues(j, layoutKeys[j][rtmaNdfdTdIndex], 
                                       match, NDFD_TD, RTMA_TD, 
                                       "Dew Point Temperature", "temperature", 
                                       "dew point", units, parameters, 
                                       numRowsForPoint[j][ndfdTdIndex],
                                       numRowsForPoint[j][rtmaTdIndex], 
                                       pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allTdsFormatted = 1;
                     }
                  }

                  /* Format the Real Time Mesoscale Analyses for Dew Point, if
                   * applicable. 
	           */
                  else if (!pnt_rtmaNdfdTd[j] && rtmaTdIndex >= 0 && 
                           ndfdTdIndex < 0)
                  {
                     if (weatherParameters[j][rtmaTdIndex] == 1)
                     {
                        if (f_unit != 2)
                           strcpy (units, "Fahrenheit");
                        else
                          strcpy (units, "Celsius");

                        genRtmaValues(j, layoutKeys[j][rtmaTdIndex], RTMA_TD, 
                              RTMA_UTD, match, "RTMA Dew Point Temperature", 
                              "temperature", "rtma-dew point", units, 
                              parameters, numRowsForPoint[j][rtmaTdIndex], 
                              pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allTdsFormatted = 1;
                     }
                  }   
                  else if (!pnt_rtmaNdfdTd[j] && rtmaNdfdTdIndex >= 0 && 
                           ndfdTdIndex >= 0 && rtmaTdIndex >= 0)
                  {
                     /* This is the specific case where query was for NDFD 
                      * element and RTMA part of that element, but the RTMA 
                      * part doesn't exist (occurs in Puerto Rico). 
                      */
                     if (weatherParameters[j][ndfdTdIndex] == 1)
                     {
                        genDewPointTempValues(j, layoutKeys[j][ndfdTdIndex], 
                                              match, parameters, 
                                              numRowsForPoint[j][ndfdTdIndex],
                                              pntInfo[j].startNum, 
                                              pntInfo[j].endNum, f_unit);
                        f_allTdsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdTdIndex] == 0)
                     {
                        f_allTdsFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdTd[j] && rtmaNdfdTdIndex < 0 && 
                           ndfdTdIndex >= 0 && rtmaTdIndex < 0)
                  {
                     if (weatherParameters[j][ndfdTdIndex] == 1)
                     {
                        genDewPointTempValues(j, layoutKeys[j][ndfdTdIndex], 
                                              match, parameters, 
                                              numRowsForPoint[j][ndfdTdIndex],
                                              pntInfo[j].startNum, 
                                              pntInfo[j].endNum, f_unit);
                        f_allTdsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdTdIndex] == 0)
                     {
                        f_allTdsFormatted = 1;
                     }
                  }
               }
               else if (f_XML == 1 && ndfdTdIndex >= 0)
               {
                  /* Format Dew Point Temperature Values, if applicable. */
                  if (weatherParameters[j][ndfdTdIndex] == 1)
                  { 
                     genDewPointTempValues(j, layoutKeys[j][ndfdTdIndex], 
                                           match, parameters, 
                                           numRowsForPoint[j][ndfdTdIndex],
                                           pntInfo[j].startNum, 
                                           pntInfo[j].endNum, f_unit);
                     f_allTdsFormatted = 1;
                  }
                  else if (weatherParameters[j][ndfdTdIndex] == 0)
                  {
                     f_allTdsFormatted = 1;
                  }
               }
            }

            /* Format Apparent Temperature Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_AT] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genAppTempValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], match, 
                                parameters, 
                                numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                pntInfo[j].startNum, pntInfo[j].endNum, f_unit);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_AT] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /******************QPF AND PRECIP AMOUNTS*************************/

            /* Format Hourly RTMA + NDFD Precipitation Amount values, if
             * applicable. Concatenate the two together.
             */
            if ((!f_allQpfsFormatted) && (ndfdQpfIndex >= 0 || 
                 rtmaPrecipaIndex >= 0 || rtmaNdfdPrecipaIndex >= 0))
            {
               if (f_XML == 5 || f_XML == 6)
               {
                  if (pnt_rtmaNdfdPrecipa[j] && rtmaNdfdPrecipaIndex >= 0 && 
                      ndfdQpfIndex >= 0 && rtmaPrecipaIndex >= 0)
                  {
                     if (weatherParameters[j][ndfdQpfIndex] == 1 && 
                         weatherParameters[j][rtmaNdfdPrecipaIndex] == 1 && 
                         weatherParameters[j][rtmaPrecipaIndex] == 1)
                     {
                        if (f_unit != 2)
                           strcpy (units, "inches");
                        else
                           strcpy (units, "centimeters");

                        concatRtmaNdfdValues(j, layoutKeys[j][rtmaNdfdPrecipaIndex], 
                                             match, NDFD_QPF, RTMA_PRECIPA, 
                                             "Liquid Precipitation Amount",
                                             "precipitation", "liquid", units,
                                             parameters, 
                                             numRowsForPoint[j][ndfdQpfIndex],
                                             numRowsForPoint[j][rtmaPrecipaIndex], 
                                             pntInfo[j].startNum, 
                                             pntInfo[j].endNum);
                        f_allQpfsFormatted = 1;
                     }
                  }

                  /* Format the Real Time Mesoscale Analyses for Precipitation 
                   * Amount, if applicable. 
	           */
                  else if (!pnt_rtmaNdfdPrecipa[j] && rtmaPrecipaIndex >= 0 && 
                           ndfdQpfIndex < 0)
                  {
                     if (weatherParameters[j][rtmaPrecipaIndex] == 1)
                     {
                        if (f_unit != 2)
                           strcpy (units, "inches");
                        else
                          strcpy (units, "centimeters");

                        genRtmaValues(j, layoutKeys[j][rtmaPrecipaIndex], 
                             RTMA_PRECIPA, -1, match, 
                             "RTMA Liquid Precipitation Amount", 
                             "precipitation", "rtma-liquid", units, 
                             parameters, numRowsForPoint[j][rtmaPrecipaIndex], 
                             pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allQpfsFormatted = 1;
                     }
                  }   
                  else if (!pnt_rtmaNdfdPrecipa[j] && rtmaNdfdPrecipaIndex >= 0 && 
                           ndfdQpfIndex >= 0 && rtmaPrecipaIndex >= 0)
                  {
                     /* This is the specific case where query was for NDFD 
                      * element and RTMA part of that element, but the RTMA 
                      * part doesn't exist (occurs in Puerto Rico). 
                      */
                     if (weatherParameters[j][ndfdQpfIndex] == 1)
                     { 
                        genQPFValues(j, layoutKeys[j][ndfdQpfIndex], match, 
                                     parameters, numRowsForPoint[j][ndfdQpfIndex], 
                                     pntInfo[j].startNum, pntInfo[j].endNum, 
                                     f_unit);
                        f_allQpfsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdQpfIndex] == 0)
                     {
                        f_allQpfsFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdPrecipa[j] && rtmaNdfdPrecipaIndex < 0 && 
                           ndfdQpfIndex >= 0 && rtmaPrecipaIndex < 0)
                  {
                     if (weatherParameters[j][ndfdQpfIndex] == 1)
                     { 
                        genQPFValues(j, layoutKeys[j][ndfdQpfIndex], match, 
                               parameters, numRowsForPoint[j][ndfdQpfIndex], 
                               pntInfo[j].startNum, pntInfo[j].endNum, f_unit);
                        f_allQpfsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdQpfIndex] == 0)
                     {
                        f_allQpfsFormatted = 1;
                     }
                  }
               }
               else if (f_XML == 1 && ndfdQpfIndex >= 0)
               {
                  /* Format NDFD QPF Values, if applicable. */
                  if (weatherParameters[j][ndfdQpfIndex] == 1)
                  { 
                     genQPFValues(j, layoutKeys[j][ndfdQpfIndex], match, 
                            parameters, numRowsForPoint[j][ndfdQpfIndex], 
                            pntInfo[j].startNum, pntInfo[j].endNum, f_unit);
                     f_allQpfsFormatted = 1;
                  }
                  else if (weatherParameters[j][ndfdQpfIndex] == 0)
                  {
                     f_allQpfsFormatted = 1;
                  }
               }
            }

            /* Format Snow Amount Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_SNOW] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genSnowValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], match, 
                             parameters,
                             numRowsForPoint[j][Dwml[k].origNdfdIndex],
                             pntInfo[j].startNum, pntInfo[j].endNum, f_unit);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_SNOW] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Ice Accumulation Amount Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_ICEACC] &&
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genIceAccValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], match,
                               parameters,
                               numRowsForPoint[j][Dwml[k].origNdfdIndex],
                               pntInfo[j].startNum, pntInfo[j].endNum, f_unit);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_ICEACC] &&
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /************************POP12*****************************/
            /* Format PoP12 Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_POP] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               /* If product is of DWMLgenByDay type, allocate maxDailyPop
                * array and initialize it. 
                */
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
               genPopValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], match, 
                         parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                         f_XML, startTime, maxDailyPop, &numDays[j], 
                         currentDoubTime, currentHour[j], pntInfo[j].startNum, 
                         pntInfo[j].endNum);
               continue;
	    }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_POP] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /*******************2 FIRE WEATHER OUTLOOK ELEMENTS***************/
            /*****************************************************************/
            /* Format the Fire Wx Outlook due to wind and relative humidity for
             * DWMLgen time-series product, for days 1-7, if applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_FWXWINDRH] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genFireWxValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                         NDFD_FWXWINDRH, match, 
                         "risk from wind and relative humidity",
                         "Fire Weather Outlook from Wind and Relative Humidity", 
                         parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                         pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_FWXWINDRH] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format the Fire Wx Outlook due to dry thunderstorms for
             * DWMLgen time-series product, for days 1-3, if applicable.
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_FWXTSTORM] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genFireWxValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                         NDFD_FWXTSTORM, match, 
                         "risk from dry thunderstorms",
                         "Fire Weather Outlook from Dry Thunderstorms", 
                         parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                         pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_FWXTSTORM] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /*****************9 SPC CONVECTIVE HAZARDS ELEMENTS***************/
            /*****************************************************************/
            /* Format the Categorical Convective Hazard Outlook for DWMLgen 
             * time-series product, for days 1-3, if applicable.    	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_CONHAZ] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genConvOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                             match, parameters, 
                             numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                             pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_CONHAZ] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format the Probability of Tornadoes Convective Hazard for DWMLgen 
             * time-series product, for Day 1, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PTORN] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genConvSevereCompValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                    NDFD_PTORN, match, "tornadoes",
                                    "Probability of Tornadoes", parameters, 
                                    numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                    pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PTORN] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format the Probability of Hail Convective Hazard for DWMLgen 
             * time-series product, for Day 1, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PHAIL] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genConvSevereCompValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                    NDFD_PHAIL, match, "hail", 
                                    "Probability of Hail", parameters, 
                                    numRowsForPoint[j][Dwml[k].origNdfdIndex],
                                    pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PHAIL] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format the Probability of Damaging Thunderstorm Winds Convective 
             * Hazard for DWMLgen time-series product, for Day 1, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PTSTMWIND] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genConvSevereCompValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                  NDFD_PTSTMWIND,  match, 
                                  "damaging thunderstorm winds", 
                                  "Probability of Damaging Thunderstorm Winds", 
                                  parameters, 
                                  numRowsForPoint[j][Dwml[k].origNdfdIndex],
                                  pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PTSTMWIND] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format the Probability of Extreme Tornadoes Convective Hazard
             * for DWMLgen time-series product, for Day 1, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PXTORN] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genConvSevereCompValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                       NDFD_PXTORN, match, "extreme tornadoes", 
                                       "Probability of Extreme Tornadoes", 
                                       parameters, 
                                       numRowsForPoint[j][Dwml[k].origNdfdIndex],
                                       pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PXTORN] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format the Probability of Extreme Hail Convective Hazard for
             * DWMLgen time-series product, for Day 1, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PXHAIL] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genConvSevereCompValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                       NDFD_PXHAIL, match, "extreme hail", 
                                       "Probability of Extreme Hail",
                                       parameters, 
                                       numRowsForPoint[j][Dwml[k].origNdfdIndex],
                                       pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PXHAIL] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format the Probability of Extreme Thunderstorm Winds Convective 
             * Hazard for DWMLgen time-series product, for Day 1, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PXTSTMWIND] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genConvSevereCompValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                    NDFD_PXTSTMWIND, match, 
                                    "extreme thunderstorm winds", 
                                    "Probability of Extreme Thunderstorm Winds", 
                                    parameters, 
                                    numRowsForPoint[j][Dwml[k].origNdfdIndex],
                                    pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PXTSTMWIND] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;
  
            /* Format the Total Probability of Severe Thunderstorms Convective 
             * Hazard for DWMLgen time-series product, for Days 2-3, if 
             * applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PSTORM] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genConvSevereCompValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                   NDFD_PSTORM, match, "severe thunderstorms", 
                                   "Total Probability of Severe Thunderstorms", 
                                   parameters, 
                                   numRowsForPoint[j][Dwml[k].origNdfdIndex],
                                   pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PSTORM] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format the Total Probability of Extreme Severe Thunderstorms 
             * Convective Hazard for DWMLgen time-series product, for Days 2-3,
             * if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PXSTORM] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genConvSevereCompValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                           NDFD_PXSTORM, match, "extreme severe thunderstorms", 
                           "Total Probability of Extreme Severe Thunderstorms", 
                           parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex],
                           pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PXSTORM] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /*****************12 CLIMATE ANOMALY PROBABILITIES ***************/
            /*****************************************************************/
            /* Format 8-14 Average Temperature, Above Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPABV14D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                     NDFD_TMPABV14D, match, "average temperature above normal",
                     "Probability of 8-14 Day Average Temperature Above Normal",
                     parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                     pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPABV14D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format 8-14 Average Temperature, Below Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPBLW14D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                    NDFD_TMPBLW14D, match, "average temperature below normal", 
                    "Probability of 8-14 Day Average Temperature Below Normal",
                    parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                    pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPBLW14D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format 8-14 Average Precipitation, Above Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPABV14D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                  NDFD_PRCPABV14D, match, "average precipitation above normal", 
                  "Probability of 8-14 Day Average Precipitation Above Normal",
                  parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                  pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPABV14D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format 8-14 Average Precipitation, Below Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPBLW14D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                  NDFD_PRCPBLW14D, match, "average precipitation below normal",
                  "Probability of 8-14 Day Average Precipitation Below Normal",
                  parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                  pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPBLW14D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Monthly Average Temperature, Above Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPABV30D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                   NDFD_TMPABV30D, match, "average temperature above normal", 
                   "Probability of One-Month Average Temperature Above Normal",
                   parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                   pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPABV30D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Monthly Average Temperature, Below Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPBLW30D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                   NDFD_TMPBLW30D, match, "average temperature below normal", 
                   "Probability of One-Month Average Temperature Below Normal",
                   parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                   pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPBLW30D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Monthly Average Precipitation, Above Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPABV30D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                 NDFD_PRCPABV30D, match, "average precipitation above normal",
                 "Probability of One-Month Average Precipitation Above Normal",
                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                 pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPABV30D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Monthly Average Precipitation, Below Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPBLW30D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                 NDFD_PRCPBLW30D, match, "average precipitation below normal", 
                 "Probability of One-Month Average Precipitation Below Normal",
                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                 pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPBLW30D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format 3-Monthly Average Temperature, Above Normal Values, if
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPABV90D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                 NDFD_TMPABV90D, match, "average temperature above normal", 
                 "Probability of Three-Month Average Temperature Above Normal",
                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                 pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPABV90D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format 3-Monthly Average Temperature, Below Normal Values, if
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPBLW90D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                 NDFD_TMPBLW90D, match, "average temperature below normal", 
                 "Probability of Three-Month Average Temperature Below Normal",
                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                 pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_TMPBLW90D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format 3-Monthly Average Precipitation, Above Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPABV90D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
               NDFD_PRCPABV90D, match, "average precipitation above normal", 
               "Probability of Three-Month Average Precipitation Above Normal",
               parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
               pntInfo[j].startNum, pntInfo[j].endNum);
              continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPABV90D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format 3-Monthly Average Precipitation, Below Normal Values, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPBLW90D] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genClimateOutlookValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
               NDFD_PRCPBLW90D, match, "average precipitation below normal", 
               "Probability of Three-Month Average Precipitation Below Normal",
               parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
               pntInfo[j].startNum, pntInfo[j].endNum);
              continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_PRCPBLW90D] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /*****************6 TROPICAL WIND THRESHOLD PROBABILITIES ***********/
            /********************************************************************/
            /* Format Incremental Probability of 34 Knt Wind Values for DWMLgen 
	     * product, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_INC34] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genWindIncCumValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                         NDFD_INC34, match, "incremental34", 
	 "Probability of a Tropical Cyclone Wind Speed above 34 Knots (Incremental)",
	                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex],
                         pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_INC34] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Incremental Probability of 50 Knt Wind Values for DWMLgen 
	     * product, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_INC50] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genWindIncCumValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                         NDFD_INC50, match, "incremental50", 
	 "Probability of a Tropical Cyclone Wind Speed above 50 Knots (Incremental)",
	                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex],
                         pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_INC50] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Incremental Probability of 64 Knt Wind Values for DWMLgen 
	     * product, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_INC64] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genWindIncCumValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                         NDFD_INC64, match, "incremental64", 
	 "Probability of a Tropical Cyclone Wind Speed above 64 Knots (Incremental)",
	                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex],
                         pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_INC64] && 
                    weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Cumulative Probability of 34 Knt Wind Values for DWMLgen 
	     * product, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_CUM34] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genWindIncCumValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                         NDFD_CUM34, match, "cumulative34", 
	 "Probability of a Tropical Cyclone Wind Speed above 34 Knots (Cumulative)",
                         parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex],
                         pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_CUM34] && 
                    weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Cumulative Probability of 50 Knt Wind Values for DWMLgen 
	     * product, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_CUM50] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genWindIncCumValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                         NDFD_CUM50, match, "cumulative50", 
	 "Probability of a Tropical Cyclone Wind Speed above 50 Knots (Cumulative)", 
	                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex],
                         pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_CUM50] && 
                    weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Cumulative Probability of 64 Knt Wind Values for DWMLgen 
	     * product, if applicable. 
	     */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_CUM64] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
                genWindIncCumValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                         NDFD_CUM64, match, "cumulative64", 
	 "Probability of a Tropical Cyclone Wind Speed above 64 Knots (Cumulative)", 
	                 parameters, numRowsForPoint[j][Dwml[k].origNdfdIndex],
                         pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_CUM64] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /************************WIND SPEEDS******************************/

            /* Format RTMA + NDFD Wind Speed values, if applicable. 
             * Concatenate the two together. f_XML = 2 (Glance Produce ) does 
             * not need to access anything (no formatting or collecting Wspd 
             * data.
             */
            if ((!f_allWspdsFormatted) && (ndfdWspdIndex >= 0 || 
                 rtmaWspdIndex >= 0 || rtmaNdfdWspdIndex >= 0) && f_XML != 2)
            {
                if (f_XML == 5 || f_XML == 6)
               {
                  if (pnt_rtmaNdfdWspd[j] && rtmaNdfdWspdIndex >= 0 && 
                      ndfdWspdIndex >= 0 && rtmaWspdIndex >= 0)
                  {
                     if (weatherParameters[j][ndfdWspdIndex] == 1 && 
                         weatherParameters[j][rtmaNdfdWspdIndex] == 1 && 
                         weatherParameters[j][rtmaWspdIndex] == 1)
                     {
                        if (f_unit != 2)
                           strcpy (units, "knots");
                        else
                          strcpy (units, "meters/second");

                        concatRtmaNdfdValues(j, layoutKeys[j][rtmaNdfdWspdIndex], 
                                 match, NDFD_WS, RTMA_WSPD, "Wind Speed", 
                                 "wind-speed", "sustained", units, parameters,
                                 numRowsForPoint[j][ndfdWspdIndex],
                                 numRowsForPoint[j][rtmaWspdIndex], 
                                 pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allWspdsFormatted = 1;
                     }
                  }

                  /* Format the Real Time Mesoscale Analyses for Wind Speed, if
                   * applicable. 
       	           */
                  else if (!pnt_rtmaNdfdWspd[j] && rtmaWspdIndex >= 0 && 
                           ndfdWspdIndex < 0)
                  {
                     if (weatherParameters[j][rtmaWspdIndex] == 1)
                     {
                        if (f_unit != 2) 
                           strcpy (units, "knots"); 
                        else 
                          strcpy (units, "meters/second");

                        genRtmaValues(j, layoutKeys[j][rtmaWspdIndex], 
                             RTMA_WSPD, RTMA_UWSPD, match, "RTMA Wind Speed", 
                             "wind-speed", "rtma-sustained", units, 
                             parameters, numRowsForPoint[j][rtmaWspdIndex], 
                             pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allWspdsFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdWspd[j] && rtmaNdfdWspdIndex >= 0 && 
                           ndfdWspdIndex >= 0 && rtmaWspdIndex >= 0)
                  {
                     /* This is the specific case where query was for NDFD 
                      * element and RTMA part of that element, but the RTMA 
                      * part doesn't exist (occurs in Puerto Rico). 
                      */
                     if (weatherParameters[j][ndfdWspdIndex] == 1)
                     {
                        genWindSpeedValues(timeUserStart[j], timeUserEnd[j], j,
	   		       layoutKeys[j][ndfdWspdIndex], match, parameters,
                               startDate[j], maxWindSpeed, &numOutputLines[j],
                               timeInterval, TZoffset[j], pntInfo[j].f_dayLight,
                               NDFD_WS, numRowsForPoint[j][ndfdWspdIndex], 
                               f_XML, valTimeForWindDirMatch, startTime, 
                               pntInfo[j].startNum, pntInfo[j].endNum, 
                               f_shiftData, f_unit);
                        f_allWspdsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdWspdIndex] == 0)
                     {
                        f_allWspdsFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdWspd[j] && rtmaNdfdWspdIndex < 0 && 
                           ndfdWspdIndex >= 0 && rtmaWspdIndex < 0)
                  {
                     if (weatherParameters[j][ndfdWspdIndex] == 1)
                     {
                        genWindSpeedValues(timeUserStart[j], timeUserEnd[j], j,
	   		       layoutKeys[j][ndfdWspdIndex], match, parameters,
                               startDate[j], maxWindSpeed, &numOutputLines[j],
                               timeInterval, TZoffset[j], pntInfo[j].f_dayLight,
                               NDFD_WS, numRowsForPoint[j][ndfdWspdIndex], 
                               f_XML, valTimeForWindDirMatch, startTime, 
                               pntInfo[j].startNum, pntInfo[j].endNum, 
                               f_shiftData, f_unit);
                        f_allWspdsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdWspdIndex] == 0)
                     {
                        f_allWspdsFormatted = 1;
                     }
                  }
               }

               /* Format NDFD Wind Speed Values for DWMLgen products, if 
                * applicable. Collect Max Wind Speed values if product is of 
                * type DWMLgenByDay. 
                */
               else /* if f_XML = 1, 3, or 4 */
               {
                  /* If product is of DWMLgenByDay type, allocate maxWindSpeed
                   * array. We need the max wind speed values for each forecast
                   * period to derive the weather and icon elements.  Also,
                   * allocate the array holding the valid times that correspond
                   * to the max wind speeds. These times will be used to collect
                   * the wind directions that correspond to the times when the 
                   * max wind speeds occurred. 
	           */
                  if (ndfdWspdIndex >= 0)
                  {
                     if ((f_XML == 3 || f_XML == 4) && 
                          weatherParameters[j][ndfdWspdIndex] == 2)
                     {
                        maxWindSpeed = malloc((numOutputLines[j]) 
                                       * sizeof(int));
                        valTimeForWindDirMatch = malloc((numOutputLines[j])
                                                 * sizeof(double));
                        for (i = 0; i < (numOutputLines[j]); i++)
                        {
                           maxWindSpeed[i] = -999;
                           valTimeForWindDirMatch[i] = -999;
                        }
                     }
                     if (weatherParameters[j][ndfdWspdIndex] == 1 || 
                         weatherParameters[j][ndfdWspdIndex] == 2)
                     {
                        genWindSpeedValues(timeUserStart[j], timeUserEnd[j], j,
	   		       layoutKeys[j][ndfdWspdIndex], match, parameters,
                               startDate[j], maxWindSpeed, &numOutputLines[j],
                               timeInterval, TZoffset[j], pntInfo[j].f_dayLight,
                               NDFD_WS, numRowsForPoint[j][ndfdWspdIndex], 
                               f_XML, valTimeForWindDirMatch, startTime, 
                               pntInfo[j].startNum, pntInfo[j].endNum, 
                               f_shiftData, f_unit);
                        f_allWspdsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdWspdIndex] == 0)
                     {
                        f_allWspdsFormatted = 1;
                     }
                  }
               }
            }

            /* Format NDFD Wind Speed Gust Values for DWMLgen products, if 
             * applicable. 
             */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WG] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genWindSpeedGustValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                      match, parameters, 
                                      numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                      pntInfo[j].startNum, pntInfo[j].endNum, 
                                      f_unit);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WG] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /************************WIND DIRECTIONS**************************/

            /* Format RTMA + NDFD Wind Direcion values, if applicable. 
             * Concatenate the two together.
             */
            if ((!f_allWdirsFormatted) && (ndfdWdirIndex >= 0 || 
                 rtmaWdirIndex >= 0 || rtmaNdfdWdirIndex >= 0))
            {
               if (f_XML == 5 || f_XML == 6)
               {
                  if (pnt_rtmaNdfdWdir[j] && rtmaNdfdWdirIndex >= 0 && 
                      ndfdWdirIndex >= 0 && rtmaWdirIndex >= 0)
                  {
                     if (weatherParameters[j][ndfdWdirIndex] == 1 && 
                         weatherParameters[j][rtmaNdfdWdirIndex] == 1 && 
                         weatherParameters[j][rtmaWdirIndex] == 1)
                     {
                        concatRtmaNdfdValues(j, layoutKeys[j][rtmaNdfdWdirIndex], 
                                 match, NDFD_WD, RTMA_WDIR, "Wind Direction", 
                                 "direction", "wind", "degrees true", 
                                 parameters, numRowsForPoint[j][ndfdWdirIndex],
                                 numRowsForPoint[j][rtmaWdirIndex], 
                                 pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allWdirsFormatted = 1;
                     }
                  }

                  /* Format the Real Time Mesoscale Analyses for Wind Direction,
                   * if applicable. 
	           */
                  else if (!pnt_rtmaNdfdWdir[j] && rtmaWdirIndex >= 0 && 
                            ndfdWdirIndex < 0)
                  {
                     if (weatherParameters[j][rtmaWdirIndex] == 1)
                     {
                        genRtmaValues(j, layoutKeys[j][rtmaWdirIndex], 
                             RTMA_WDIR, RTMA_UWDIR, match, 
                             "RTMA Wind Direction", "direction", "rtma-wind", 
                             "degrees true", parameters, 
                             numRowsForPoint[j][rtmaWdirIndex], 
                             pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allWdirsFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdWdir[j] && rtmaNdfdWdirIndex >= 0 && 
                           ndfdWdirIndex >= 0 && rtmaWdirIndex >= 0)
                  {
                     /* This is the specific case where query was for NDFD 
                      * element and RTMA part of that element, but the RTMA 
                      * part doesn't exist (occurs in Puerto Rico).
                      */
                     if (weatherParameters[j][ndfdWdirIndex] == 1)
                     {
                        genWindDirectionValues(j, layoutKeys[j][ndfdWdirIndex], 
                                   match, parameters, maxWindDirection, f_XML, 
                                   &numOutputLines[j], valTimeForWindDirMatch,
				   numRowsForPoint[j][ndfdWdirIndex], 
                                   pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allWdirsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdWdirIndex] == 0)
                     {
                        f_allWdirsFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdWdir[j] && rtmaNdfdWdirIndex < 0 && 
                           ndfdWdirIndex >= 0 && rtmaWdirIndex < 0)
                  {
                     if (weatherParameters[j][ndfdWdirIndex] == 1)
                     {
                        genWindDirectionValues(j, layoutKeys[j][ndfdWdirIndex], 
                                   match, parameters, maxWindDirection, f_XML, 
                                   &numOutputLines[j], valTimeForWindDirMatch,
				   numRowsForPoint[j][ndfdWdirIndex], 
                                   pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allWdirsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdWdirIndex] == 0)
                     {
                        f_allWdirsFormatted = 1;
                     }
                  }
               }

               /* Format NDFD Wind Dir values for DWMLgen products, if 
                * applicable. Collect the Wind Dir values that correspond to 
                * the times when the maximum Wind Speeds existed if product is 
                * of type DWMLgenByDay. 
                */
               else /* if f_XML = 1, 3, or 4 */
               {
                  /* If product is of DWMLgenByDay type, allocate 
                   * maxWindDirection array and initialize. We need these 
                   * wind direction values for each forecast period to derive 
                   * the weather and icon elements. 
                   */
                  if (ndfdWdirIndex >= 0)
                  {
                     if (f_XML == 3 || f_XML == 4)
                     {
                        maxWindDirection = malloc((numOutputLines[j]) 
                                                   * sizeof(int));
                        for (i = 0; i < (numOutputLines[j]); i++)
                           maxWindDirection[i] = -999;
                     }
                     if (weatherParameters[j][ndfdWdirIndex] == 1 || 
                         weatherParameters[j][ndfdWdirIndex] == 2)
                     {
                        genWindDirectionValues(j, layoutKeys[j][ndfdWdirIndex], 
                                   match, parameters, maxWindDirection, f_XML, 
                                   &numOutputLines[j], valTimeForWindDirMatch,
				   numRowsForPoint[j][ndfdWdirIndex],
                                   pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allWdirsFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdWdirIndex] == 0)
                     {
                        f_allWdirsFormatted = 1;
                     }                  
                  }
               }
            }
 
            /****************************SKY COVER****************************/

            /* Format RTMA + NDFD Sky Cover values, if applicable. Concatenate
             * the two together.
             */

            if ((!f_allSkysFormatted) && (ndfdSkyIndex >= 0 || rtmaSkyIndex >= 0 
                 || rtmaNdfdSkyIndex >= 0))
            {
               if (f_XML == 5 || f_XML == 6) 
               {
                  if (pnt_rtmaNdfdSky[j] && rtmaNdfdSkyIndex >= 0 && 
                      ndfdSkyIndex >= 0 && rtmaSkyIndex >= 0)
                  {
                     if (weatherParameters[j][ndfdSkyIndex] == 1 && 
                         weatherParameters[j][rtmaNdfdSkyIndex] == 1 && 
                         weatherParameters[j][rtmaSkyIndex] == 1)
                     {
                        concatRtmaNdfdValues(j, layoutKeys[j][rtmaNdfdSkyIndex], 
                                 match, NDFD_SKY, RTMA_SKY, 
                                 "Cloud Cover Amount", "cloud-amount", "total", 
                                 "percent", parameters,
                                 numRowsForPoint[j][ndfdSkyIndex],
                                 numRowsForPoint[j][rtmaSkyIndex], 
                                 pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allSkysFormatted = 1;
                     }
                  }
                  /* Format the Real Time Mesoscale Analyses for Sky Cover Amount, 
                   * if applicable. 
	           */
                  else if (!pnt_rtmaNdfdSky[j] && rtmaSkyIndex >= 0 && 
                            ndfdSkyIndex < 0)
                  {
                     if (weatherParameters[j][rtmaSkyIndex] == 1)
                     {
                        genRtmaValues(j, layoutKeys[j][rtmaSkyIndex], RTMA_SKY, 
                             -1, match, "RTMA Cloud Cover Amount", "cloud-amount", 
                             "rtma-total", "percent", parameters, 
                             numRowsForPoint[j][rtmaSkyIndex], 
                             pntInfo[j].startNum, pntInfo[j].endNum);
                        f_allSkysFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdSky[j] && rtmaNdfdSkyIndex >= 0 && 
                           ndfdSkyIndex >= 0 && rtmaSkyIndex >= 0)
                  {
                     /* This is the specific case where query was for NDFD 
                      * element and RTMA part of that element, but the RTMA 
                      * part doesn't exist (occurs in Puerto Rico. 
                      */
                     if (weatherParameters[j][ndfdSkyIndex] == 1)
                     {
                        genSkyCoverValues(j, layoutKeys[j][ndfdSkyIndex], match, 
                              parameters, startDate[j], maxSkyCover, 
                              minSkyCover, averageSkyCover, &numOutputLines[j], 
                              timeInterval, TZoffset[j], pntInfo[j].f_dayLight, 
                              NDFD_SKY, numRowsForPoint[j][ndfdSkyIndex], 
                              f_XML, maxSkyNum, minSkyNum, startPositions, 
                              endPositions, currentHour[j], timeUserStart[j], 
                              startTime, pntInfo[j].startNum, pntInfo[j].endNum, 
                              f_shiftData);
                        f_allSkysFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdSkyIndex] == 0)
                     {
                        f_allSkysFormatted = 1;
                     }
                  }
                  else if (!pnt_rtmaNdfdSky[j] && rtmaNdfdSkyIndex < 0 && 
                           ndfdSkyIndex >= 0 && rtmaSkyIndex < 0)
                  {
                     if (weatherParameters[j][ndfdSkyIndex] == 1)
                     {
                        genSkyCoverValues(j, layoutKeys[j][ndfdSkyIndex], match, 
                              parameters, startDate[j], maxSkyCover, 
                              minSkyCover, averageSkyCover, &numOutputLines[j], 
                              timeInterval, TZoffset[j], pntInfo[j].f_dayLight, 
                              NDFD_SKY, numRowsForPoint[j][ndfdSkyIndex], 
                              f_XML, maxSkyNum, minSkyNum, startPositions, 
                              endPositions, currentHour[j], timeUserStart[j], 
                              startTime, pntInfo[j].startNum, pntInfo[j].endNum, 
                              f_shiftData);
                        f_allSkysFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdSkyIndex] == 0)
                     {
                        f_allSkysFormatted = 1;
                     }
                  }
               }

               /* Format NDFD Sky Cover Values for DWMLgen products, if 
                * applicable. Collect Max and Min Sky Cover values for icon 
                * determination if product is of type DWMLgenByDay. 
                */
               else /* if f_XML = 1, 2, 3, 4. */
               {
                  /* If product is of DWMLgenByDay type, allocate the 
                   * maxSkyCover, minSkyCover, minSkyNum, maxSkyNum, 
                   * startPositions, endPositions, and averageSkyCover arrays 
                   * and initialize them. We need these sky values for each 
                   * forecast period to derive the weather and icon elements. 
                   */
                  if (ndfdSkyIndex >= 0)
                  {
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
                     if (weatherParameters[j][ndfdSkyIndex] == 1 || 
                         weatherParameters[j][ndfdSkyIndex] == 2)
                     {
                        genSkyCoverValues(j, layoutKeys[j][ndfdSkyIndex], match, 
                              parameters, startDate[j], maxSkyCover, 
                              minSkyCover, averageSkyCover, &numOutputLines[j], 
                              timeInterval, TZoffset[j], pntInfo[j].f_dayLight, 
                              NDFD_SKY, numRowsForPoint[j][ndfdSkyIndex], 
                              f_XML, maxSkyNum, minSkyNum, startPositions, 
                              endPositions, currentHour[j], timeUserStart[j], 
                              startTime, pntInfo[j].startNum, pntInfo[j].endNum, 
                              f_shiftData);
                        f_allSkysFormatted = 1;
                     }
                     else if (weatherParameters[j][ndfdSkyIndex] == 0)
                     {
                        f_allSkysFormatted = 1;
                     }
                  }
               }
            }

            /* Format Relative Humidity Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_RH] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genRelHumidityValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                    match, parameters, 
                                    numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                    pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_RH] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Maximum Relative Humidity Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_MAXRH] &&
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genMaxRHValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], match,
                              parameters, 
                              numRowsForPoint[j][Dwml[k].origNdfdIndex],
                              pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_MAXRH] &&
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

            /* Format Minimum Relative Humidity Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_MINRH] &&
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genMinRHValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], match,
                              parameters,
                              numRowsForPoint[j][Dwml[k].origNdfdIndex],
                              pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_MINRH] &&
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue; 
 
            /************************WEATHER AND HAZARD GENERATION************/

            /* Format Hazards and Weather Values (and\or Icons), if applicable. 
             * We must have at least some rows of weather data to format weather 
             * and icons. 
             */
            if (f_XML == 1 || f_XML == 2 || f_XML == 6)
            {
               /**************************** WEATHER *************************/
               if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WX] && 
                  (weatherParameters[j][Dwml[k].origNdfdIndex] == 1 || 
                   weatherParameters[j][Dwml[k].origNdfdIndex] == 3))
               {
                  if (f_formatIconForPnt[j])
                  {
                     genWeatherValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                   match,
                                   weatherParameters[j][Dwml[k].origNdfdIndex],
                                   f_formatIconForPnt[j], 
                                   numRowsForPoint[j][ndfdWspdIndex],
                                   numRowsForPoint[j][ndfdSkyIndex],
                                   numRowsForPoint[j][ndfdTempIndex],
                                   numRowsForPoint[j][Dwml[k].origNdfdIndex], 
	                           numRowsForPoint[j][ndfdPopIndex], parameters,
                                   pnts[j].Y, pnts[j].X, pntInfo[j].startNum, 
                                   pntInfo[j].endNum, TZoffset[j], 
                                   pntInfo[j].f_dayLight, f_unit);
                     continue;
                  }
                  else /* Dummy up the elements needed for icons, as they're 
                        * not needed if just weather is formatted. 
                        */
                  {
                     genWeatherValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                   match,
                                   weatherParameters[j][Dwml[k].origNdfdIndex],
                                   f_formatIconForPnt[j], 
                                   numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                   numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                   numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                   numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                   numRowsForPoint[j][Dwml[k].origNdfdIndex],
                                   parameters, pnts[j].Y, pnts[j].X, 
                                   pntInfo[j].startNum, 
                                   pntInfo[j].endNum, TZoffset[j], 
                                   pntInfo[j].f_dayLight, f_unit);
                     continue;
                  }
               }
               else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WX] && 
                        weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
                  continue;

               /**************************** HAZARDS *************************/

               if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WWA] && 
                   weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
               {
                  genHazardValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                               match, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                               parameters, pntInfo[j].startNum, 
                               pntInfo[j].endNum, pntInfo[j].cwa);
                  continue;
               }
            }
            else if (f_XML == 3 || f_XML == 4)
            /* Summarized Products. */
            {
               if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WX] && 
                   f_formatSummarizations[j])
               {
	       /**************************** WEATHER **************************/
                  /* Use gust if availabale. If not, use Wspd in it's place. 
                   * Don't let the availability of Gust shut off weather phrase
                   * and icon generation. It only exist 52 hours out and is only
                   * used in Blizzard determination.
                   */
                  if (ndfdWgustIndex < 0)
                     ndfdWgustOrWspdIndex = ndfdWspdIndex;
                  else
                     ndfdWgustOrWspdIndex = ndfdWgustIndex;

	          genWeatherValuesByDay(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                     match, numMatch,
	                             numRowsForPoint[j][ndfdWgustOrWspdIndex],
                                     numRowsForPoint[j][ndfdWspdIndex],
			             numRowsForPoint[j][ndfdPopIndex],
			             numRowsForPoint[j][ndfdMaxIndex],
			             numRowsForPoint[j][ndfdMinIndex],
                                     numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                     parameters, &numDays[j], TZoffset[j], 
                                     pntInfo[j].f_dayLight,
                                     format, f_useMinTempTimes[j], f_XML, 
                                     &numOutputLines[j], maxDailyPop, 
                                     averageSkyCover, maxSkyCover, minSkyCover, 
                                     maxSkyNum, minSkyNum, startPositions, 
                                     endPositions, maxWindSpeed,
			             maxWindDirection, startTime, format_value, 
                                     pntInfo[j].startNum, pntInfo[j].endNum,
                                     f_shiftData, valTimeForWindDirMatch);
                  continue;
               }
               else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WX] && 
                        !f_formatSummarizations[j])
               {
                  #ifdef PRINT_DIAG
                  printf ("************************************************\n");
                  printf ("Can't format weather summaries and icons for point\n");
                  printf ("#%d as all elements needed to derive these are not\n",(j+1));
                  printf ("available.\n");
                  printf ("************************************************\n");
                  #endif
               }

               /**************************** HAZARDS *************************/

               if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WWA] && 
                   weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
               {
                  genHazardSummaryValues(j, 
                        numRowsForPoint[j][Dwml[k].origNdfdIndex].multiLayouts,
                        match, numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                        parameters, pntInfo[j].startNum, pntInfo[j].endNum, 
                        pntInfo[j].cwa, indivHaz[j], numHazards[j], 
                        layoutKeys[j][Dwml[k].origNdfdIndex]);
                  continue;
               }
               else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WWA] && 
                        weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
                  continue;
            }

            /* Format Wave Height Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WH] &&
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genWaveHeightValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                                   match, parameters,
                                   numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                                   pntInfo[j].startNum, pntInfo[j].endNum, 
                                   f_unit);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[NDFD_WH] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;
   
            /* Format Lamp Thunderstorm Probability Values, if applicable. */
            if (Dwml[k].Ndfd2Dwml == NDFD2DWML[LAMP_TSTMPRB] && 
                weatherParameters[j][Dwml[k].origNdfdIndex] == 1)
            {
               genLampTstmValues(j, layoutKeys[j][Dwml[k].origNdfdIndex], 
                              match, parameters,
                              numRowsForPoint[j][Dwml[k].origNdfdIndex], 
                              pntInfo[j].startNum, pntInfo[j].endNum);
               continue;
            }
            else if (Dwml[k].Ndfd2Dwml == NDFD2DWML[LAMP_TSTMPRB] && 
                     weatherParameters[j][Dwml[k].origNdfdIndex] == 0)
               continue;

         } /* End of numElem loop. */

         /* Free some things before leaving this iteration of the point loop. */
         for (k = 0; k < numElem; k++)
         {
            if (f_XML == 3 || f_XML == 4)
            {
               if (elem[k].ndfdEnum == NDFD_POP && weatherParameters[j][k] == 1)
               {
                  free(maxDailyPop);
                  free (layoutKeys[j][k]);
                  continue;
               }
               if ((elem[k].ndfdEnum == NDFD_WD) && (weatherParameters[j][k] == 1
                    || weatherParameters[j][k] == 2))
               {
	          free(maxWindDirection);
               }              
               if ((elem[k].ndfdEnum == NDFD_WS) && (weatherParameters[j][k] == 1
                    || weatherParameters[j][k] == 2))
               {
                  free(valTimeForWindDirMatch);
	          free(maxWindSpeed);
               }
               if ((elem[k].ndfdEnum == NDFD_SKY) && (weatherParameters[j][k] == 1
                    || weatherParameters[j][k] == 2))
               {
                  free(startPositions);
                  free(endPositions);
                  free(maxSkyNum);
                  free(minSkyNum);     
                  free(maxSkyCover);
                  free(minSkyCover);
                  free(averageSkyCover);
               }

               if (elem[k].ndfdEnum == NDFD_WWA && weatherParameters[j][k] == 1
                   && numHazards[j] != 0)
               {
                  for (i = 0; i < numHazards[j]; i++)
                  {
                     if (numRowsForPoint[j][k].multiLayouts[i] != NULL)
                        free(numRowsForPoint[j][k].multiLayouts[i]);
                  }
                  free(numRowsForPoint[j][k].multiLayouts);
                  continue;
               }
            }
         
            /* Free layoutKeys element array, if allocated. */
            if ((elem[k].ndfdEnum == RTMA_UTEMP) || 
                (elem[k].ndfdEnum == RTMA_UTD) || 
                (elem[k].ndfdEnum == RTMA_UWSPD) ||
                (elem[k].ndfdEnum == RTMA_UWDIR) || 
                (elem[k].ndfdEnum == NDFD_TEMP && pnt_rtmaNdfdTemp[j]) ||
                (elem[k].ndfdEnum == RTMA_TEMP && pnt_rtmaNdfdTemp[j]) || 
                (elem[k].ndfdEnum == NDFD_TD && pnt_rtmaNdfdTd[j]) || 
                (elem[k].ndfdEnum == RTMA_TD && pnt_rtmaNdfdTd[j]) || 
                (elem[k].ndfdEnum == NDFD_WS && pnt_rtmaNdfdWspd[j]) || 
                (elem[k].ndfdEnum == RTMA_WSPD && pnt_rtmaNdfdWspd[j]) || 
                (elem[k].ndfdEnum == NDFD_WD && pnt_rtmaNdfdWdir[j]) || 
                (elem[k].ndfdEnum == RTMA_WDIR && pnt_rtmaNdfdWdir[j]) || 
                (elem[k].ndfdEnum == NDFD_QPF && pnt_rtmaNdfdPrecipa[j]) || 
                (elem[k].ndfdEnum == RTMA_PRECIPA && pnt_rtmaNdfdPrecipa[j]) || 
                (elem[k].ndfdEnum == NDFD_SKY && pnt_rtmaNdfdSky[j]) || 
                (elem[k].ndfdEnum == RTMA_SKY && pnt_rtmaNdfdSky[j]) || 
                (elem[k].ndfdEnum == RTMA_NDFD_TEMP && !pnt_rtmaNdfdTemp[j]) || 
                (elem[k].ndfdEnum == RTMA_NDFD_TD && !pnt_rtmaNdfdTd[j]) || 
                (elem[k].ndfdEnum == RTMA_NDFD_WSPD && !pnt_rtmaNdfdWspd[j]) || 
                (elem[k].ndfdEnum == RTMA_NDFD_WDIR && !pnt_rtmaNdfdWdir[j]) || 
                (elem[k].ndfdEnum == RTMA_NDFD_SKY && !pnt_rtmaNdfdSky[j]) || 
                (elem[k].ndfdEnum == RTMA_NDFD_PRECIPA && !pnt_rtmaNdfdPrecipa[j]))
            {
               continue;
            }            
            else if (weatherParameters[j][k] == 1 || 
                     weatherParameters[j][k] == 3)
            {
               free(layoutKeys[j][k]);
               continue;
            }
         }
         if ((f_XML == 3 || f_XML == 4) && (numHazards[j] != 0))
         {
            free(periodTimes[j]); 
            free(indivHaz[j]);
         }

	 free(numRowsForPoint[j]);
         free(weatherParameters[j]);
         free(startDate[j]);
         free(currentDay[j]);
         free(currentHour[j]);
  
         /* Reset the element flags for those elements that could be 
          * concatenated.
          */
         f_allTempsFormatted = 0;
         f_allTdsFormatted = 0;
         f_allQpfsFormatted = 0;
         f_allWspdsFormatted = 0;
         f_allWdirsFormatted = 0;
         f_allSkysFormatted = 0;
 
      }  /* End of "is Point in Sectors" check. */

   }  /* Close Parameters Point Loop. */

   /* Free layoutKeys point array. */
   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
      {
         free(layoutKeys[j]);
      }
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
   if (f_XML == 3 || f_XML == 4)
   {
      free(indivHaz);
      free(numHazards);
      free(periodTimes);
      free(numPeriodTimes);
   } 
   free(numRowsForPoint);
   free(weatherParameters);
   free(TZoffset);
   free(startDate);         
   free(currentHour);
   free(currentDay);
   free(timeUserStart);	 
   free(timeUserEnd);
   free(f_formatNIL);
   free (f_useMinTempTimes);
   free(firstValidTimeMatch);
   free(firstValidTime_pop);
   free(firstValidTime_maxt);
   free(numDays);
   free(numOutputLines);
   free(f_formatIconForPnt);
   free(f_formatSummarizations);
   free (pnt_rtmaNdfdTemp); 
   free (pnt_rtmaNdfdTd); 
   free (pnt_rtmaNdfdWspd); 
   free (pnt_rtmaNdfdWdir); 
   free (pnt_rtmaNdfdSky); 
   free (pnt_rtmaNdfdPrecipa);
   free(Dwml);

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
