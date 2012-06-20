#ifndef XMLPARSE_H
#define XMLPARSE_H
#include "genprobe.h"
#include "type.h"
#include "sector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "xmlparse.h"
#include "genprobe.h"
#include "grpprobe.h"
#include "clock.h"
#ifdef MEMWATCH
#include "memwatch/memwatch.h"
#endif
#include "myassert.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "myutil.h"
#include "solar.h"
#define NDFD_DAYS 6

/* Set all choices for the period names for those elements needing them in the 
 * time layouts. 
 */
enum
{ earlyMorning, morning12, afternoon12, earlyMorningMaxT, earlyMorningMinT,
  morning24, afternoon24, MAX_PERIODS
};

/* Set enumerations for the concatenated RTMA + NDFD elements. Append to NDFD 
 * enumeration numbers. RTMA_NDFD_SKY = NDFD_MATCHALL + 1(56), 
 * RTMA_NDFD_PRECIPA(57), RTMA_NDFD_TD(58), RTMA_NDFD_TEMP(59), 
 * RTMA_NDFD_WDIR(60), RTMA_NDFD_WSPD(61), XML_MAX(62) 
 */
enum 
{ RTMA_NDFD_SKY = NDFD_MATCHALL + 1, RTMA_NDFD_PRECIPA, RTMA_NDFD_TD, 
  RTMA_NDFD_TEMP, RTMA_NDFD_WDIR, RTMA_NDFD_WSPD, XML_MAX 
};

typedef struct              /* Denotes structure of the time layouts. */
{
   int period;
   uChar numRows;
   char fmtdStartTime[30];
} layouts;

typedef struct                /* Denotes structure of icon info. */
{
   double validTime;
   char str[500];
   sChar valueType;
} icon_def;

typedef struct                /* Denotes structure of element's Sky cover, 
                               * Temperature, Wind Speed, and Pop info.
			       * Used in derivation of icons and weather. 
			       */
{
   double validTime;
   int data;
   sChar valueType;
} elem_def;

typedef struct                /* Denotes structure of Weather info. Used in 
                               * derivation of icons and weather. 
                               */
{
   double validTime;
   char str[600];
   sChar valueType;
} WX;

typedef struct                /* Denotes structure of Hazard info in match 
                               * structure. */ 
{
   double validTime;
   char str[600];
   sChar valueType;
} HZtype;

typedef struct                /* Array holding info about each hazard. */
{          
   double startHour;
   double endHour;
   int numConsecRows;
   char code[150];
   double valTimeResSplit;
   int numOutputLines;
} hazInfo;

typedef struct                /* Structure with info on the number of rows 
                               * skipped due to a startTime and/or endTime 
                               * effectively shortening the time data was
			       * retrieved for from NDFD. 
			       */
{
   int total; /* Total number of rows interested in. If a user shortens
                 the time period data is retrieved for, this value is 
                 reduced by these "skipped data". */
   int skipBeg; /* If a user shortens the time period data is retrieved
                   for, this value is the number of rows skipped at the
                   beginning. */
   int skipEnd; /* If a user shortens the time period data is retrieved
                   for, this value is the number of rows skipped at the
                   end. */
   double firstUserTime; /* First validTime of an element interested in. 
                            If a user does not shorten the time period
                            data is retrieved for, this value is simply
                            the first validTime for the element returned
                            from the match structure. */
   double lastUserTime;  /* Last validTime of an element interested in. 
                            If a user does not shorten the time period
                            data is retrieved for, this value is simply
                            the last validTime for the element returned
                            from the match structure. */
   char **multiLayouts;  /* In case an element has multiple sub elements.
                          * This is the case for hazards in the summary 
                          * products, in which each hazard is treated as
                          * if it were an individual element.*/
} numRowsInfo;

typedef struct                /* Denotes structure of sector info for use in a 
                               * multiple point call in which points lie in 
                               * different sectors. 
                               */
{
   int startNum;
   int endNum;
   char name[15];
   int enumNum;
} sectInfo;

typedef struct                /* Denotes structure of sector info for use in a 
                               * multiple point call in which points lie in 
                               * different sectors. 
                               */
{
   uChar Ndfd2Dwml;
   int origNdfdIndex;
} dwmlEnum;

/* Declare XMLParse() interfaces. */
void anyLampElements(size_t *numInFiles, char ***inFiles, size_t numNdfdVars,
                     uChar *ndfdVars, char *lampDataDir);

void anyRtmaElements(uChar *f_XML, size_t *numInFiles, char ***inFiles, 
                     size_t numNdfdVars, uChar *ndfdVars, 
                     char *rtmaDataDir, int f_icon, size_t numSector, 
                     char **sector, int *f_rtmaNdfdTemp, int *f_rtmaNdfdTd, 
                     int *f_rtmaNdfdWdir, int *f_rtmaNdfdWspd,
                     int *f_rtmaNdfdPrecipa,int *f_rtmaNdfdSky);

void blizzardCheck (int blizzCnt, double periodStartTime, double periodEndTime,
                    double *blizzardTime, int numRowsWS, int numRowsWG,
                    elem_def *wsInfo, elem_def *wgInfo, int dayIndex,
                    int *_iconToBlizz, char **phrase);

int checkNeedForEndTime(uChar parameterName, uChar f_XML);

void checkNeedForPeriodName(int index, uChar * numPeriodNames,
                            sChar timeZoneOffset, uChar parameterName,
                            char *parsedDataTime, uChar * outputPeriodName, 
                            uChar issuanceType, char *periodName, 
                            char *currentHour, char *currentDay, 
                            double startTime_cml, double currentDoubTime,
                            double firstValidTime);

void collectHazInfo(genMatchType *match, size_t numMatch, int startNum, 
                    int endNum, numRowsInfo numRowsHZ, size_t pnt, 
                    hazInfo **ptsIndivHazs, int *numHazards);

void computeStartEndTimes(uChar parameterName, int numFmtdRows,
                         int periodLength, double TZoffset,
                          sChar f_observeDST, genMatchType * match,
                          uChar useEndTimes, char **startTimes, char **endTimes,
                          char *frequency, uChar f_XML, double startTime_cml, 
			  double currentDoubTime, numRowsInfo numRows, 
                          int startNum, int endNum);

void concatRtmaNdfdValues(size_t pnt, char *layoutKey, genMatchType *match, 
                          uChar NDFDname, uChar RTMAname, char *name, 
                          char *metElement, char *type, char *units,
                          xmlNodePtr parameters,  numRowsInfo numRowsNDFD, 
                          numRowsInfo numRowsRTMA, int startNum, int endNum);

void determineIconUsingPop(char *iconString, char *wxStrSection, 
		           char *jpgStrSection, int POP12ValToPOP3, 
			   char *baseURL);

void determineNonWeatherIcons(int windTimeEqualsWeatherTime, int itIsNightTime, 
                              elem_def *wsInfo, int wsIndex, char *baseURL, 
                              int numRowsWS, icon_def *iconInfo, int wxIndex, 
                              int numRowsTEMP, elem_def *tempInfo, 
                              int hourlyTempIndex, 
                              int hourlyTempTimeEqualsWeatherTime);

int determinePeriodLength(double startTime, double endTime, uChar numRows, 
                          uChar parameterName);

void determineSkyIcons(int skyCoverTimeEqualsWeatherTime, int itIsNightTime, 
                       int skyIndex, int wxIndex, elem_def *skyInfo, 
                       icon_def *iconInfo, char *baseURL, int numRowsSKY);

void determineWeatherIcons(icon_def *iconInfo, int numGroups, char **wxType,
                           int skyCoverTimeEqualsWeatherTime, int itIsNightTime, 
                           elem_def *skyInfo, char *baseURL, int numRowsSKY, 
                           int skyIndex, int wxIndex, 
                           int windTimeEqualsWeatherTime, elem_def *wsInfo, 
                           int wsIndex, int numRowsWS, int numRowsTEMP, 
                           int hourlyTempIndex,
                           int hourlyTempTimeEqualsWeatherTime,
                           elem_def *tempInfo, int POP12ValToPOP3);

void dwmlEnumSort(size_t numElem, dwmlEnum **Dwml);

void formatLocationInfo(size_t numPnts, Point * pnts, xmlNodePtr data);

void formatMetaDWML(uChar f_XML, xmlDocPtr * doc, xmlNodePtr * data, 
                    xmlNodePtr * dwml);

void formatMoreWxInfo(size_t numPnts, Point * pnts, xmlNodePtr data);
 
int formatValidTime(double validTime, char *timeBuff, int size_timeBuff, 
                    sChar pntZoneOffSet, sChar f_dayCheck);

void genAppTempValues(size_t pnt, char *layoutKey, genMatchType *match,
                      xmlNodePtr parameters, numRowsInfo numRows, int startNum,
		      int endNum, sChar f_unit);

void genClimateOutlookValues(size_t pnt, char *layoutKey, uChar parameterName, 
                             genMatchType *match, char *climateOutlookType, 
                             char *climateOutlookName, xmlNodePtr parameters, 
                             numRowsInfo numRows, int startNum, int endNum);

void genConvOutlookValues(size_t pnt, char *layoutKey, genMatchType *match, 
                          xmlNodePtr parameters, numRowsInfo numRows, 
                          int startNum, int endNum);

void genConvSevereCompValues(size_t pnt, char *layoutKey, uChar parameterName, 
                             genMatchType *match, char *severeCompType, 
                             char *severeCompName, xmlNodePtr parameters, 
                             numRowsInfo numRows, int startNum, int endNum);

void genDewPointTempValues(size_t pnt, char *layoutKey, genMatchType *match, 
                           xmlNodePtr parameters, numRowsInfo numRows, 
                           int startNum, int endNum, sChar f_unit);

void genFireWxValues(size_t pnt, char *layoutKey, uChar parameterName, 
                     genMatchType *match, char *fireWxType, char *fireWxName, 
                     xmlNodePtr parameters, numRowsInfo numRows, int startNum, 
                     int endNum);

void genHazardSummaryValues(size_t pnt, char **multiLayouts, genMatchType *match,
                            numRowsInfo numRowsHZ, xmlNodePtr parameters,
                            int startNum, int endNum, char *cwaStr, 
                            hazInfo *ptsIndivHazs, int numHazards, 
                            char *layoutKey);

void genHazardValues(size_t pnt, char *layoutKey, genMatchType *match,
                     numRowsInfo numRowsHZ, xmlNodePtr parameters,
                     int startNum, int endNum, char *cwaStr);

void genHazTextURL(char *baseTextURL, char *cwaStr, char *phenomena, 
                   char *significance, char *code, char *hazardTextURL,
                   int *f_formatHazTextURL);

void genIceAccValues(size_t pnt, char *layoutKey, genMatchType *match,
                     xmlNodePtr parameters, numRowsInfo numRows, int startNum,
                     int endNum, sChar f_unit);

void genIconLinks(icon_def *iconInfo, uChar numRows, char *layoutKey, 
                  xmlNodePtr parameters);

void genLampTstmValues(size_t pnt, char *layoutKey, genMatchType *match, 
                       xmlNodePtr parameters, numRowsInfo numRows, 
                       int startNum, int endNum);

void genMaxRHValues(size_t pnt, char *layoutKey, genMatchType * match,
                    xmlNodePtr parameters, numRowsInfo numRows,
                    int startNum, int endNum);

void genMaxTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                      xmlNodePtr parameters, int f_formatNIL, uChar f_XML, 
                      double startTime_cml, numRowsInfo numRows, 
                      int numFmtdRows, int startNum, int endNum, sChar f_unit);

void genMinRHValues(size_t pnt, char *layoutKey, genMatchType * match,
                    xmlNodePtr parameters, numRowsInfo numRows,
                    int startNum, int endNum);

void genMinTempValues(size_t pnt, char *layoutKey, genMatchType *match,
                      xmlNodePtr parameters, uChar f_XML, double startTime_cml,
                      numRowsInfo numRows, char *currentDay, char *currentHour, 
                      sChar TZoffset, sChar f_observeDST, int numFmtdRows, 
                      int startNum, int endNum, sChar f_unit);

void genPopValues(size_t pnt, char *layoutKey, genMatchType *match,
                  xmlNodePtr parameters, numRowsInfo numRows, uChar f_XML,
		  double startTime_cml, int *maxDailyPop, int *numDays, 
                  double currentDoubTime, char *currentHour, int startNum, 
                  int endNum);

void genQPFValues(size_t pnt, char *layoutKey, genMatchType *match,
                  xmlNodePtr parameters, numRowsInfo numRows, int startNum, 
                  int endNum, sChar f_unit);

void genRelHumidityValues(size_t pnt, char *layoutKey, genMatchType * match, 
                           xmlNodePtr parameters, numRowsInfo numRows, 
                           int startNum, int endNum);

void genRtmaValues(size_t pnt, char *layoutKey, uChar parameterName,
                   int errorName, genMatchType *match, char *rtmaName, 
                   char *rtmaElement, char *rtmaType, char *units,
                   xmlNodePtr parameters, numRowsInfo numRows, 
                   int startNum, int endNum);

void genSkyCoverValues(size_t pnt, char *layoutKey, genMatchType * match,
                       xmlNodePtr parameters, char *startDate, int *maxSkyCover,
                       int *minSkyCover, int *averageSkyCover, 
                       int *numOutputLines, int timeInterval, sChar TZoffset, 
                       sChar f_observeDST, uChar parameterName,
                       numRowsInfo numRows, uChar f_XML, int *maxSkyNum,
                       int *minSkyNum, int *startPositions, int *endPositions, 
                       char *currentHour, double timeUserStart, 
                       double startTime, int startNum, int endNum, 
                       int f_shiftData);

void genSnowValues(size_t pnt, char *layoutKey, genMatchType *match,
                   xmlNodePtr parameters, numRowsInfo numRows, int startNum, 
                   int endNum, sChar f_unit);

void genTempValues(size_t pnt, char *layoutKey, genMatchType * match,
                   xmlNodePtr parameters, numRowsInfo numRows, int startNum, 
                   int endNum, sChar f_unit);

void genWaveHeightValues(size_t pnt, char *layoutKey, genMatchType * match, 
                         xmlNodePtr parameters, numRowsInfo numRows, 
                         int startNum, int endNum, sChar f_unit);

void genWeatherValues(size_t pnt, char *layoutKey, genMatchType *match,
                      uChar f_wx, int f_icon, numRowsInfo numRowsWS, 
                      numRowsInfo numRowsSKY, numRowsInfo numRowsTEMP, 
                      numRowsInfo numRowsWX, numRowsInfo numRowsPOP, 
                      xmlNodePtr parameters, double lat, double lon, 
                      int startNum, int endNum, sChar TZoffset, 
                      sChar f_observeDST, sChar f_unit);

void genWeatherValuesByDay(size_t pnt, char *layoutKey, 
		           genMatchType *match, size_t numMatch,
                           numRowsInfo numRowsWG, numRowsInfo numRowsWS, 
                           numRowsInfo numRowsPOP, 
                           numRowsInfo numRowsMAX, numRowsInfo numRowsMIN, 
                           numRowsInfo numRowsWX, xmlNodePtr parameters, 
                           int *numDays, sChar TZoffset,
			   sChar f_observeDST, char *frequency,
			   int f_useMinTempTimes, uChar f_XML,
			   int *numOutputLines, int *maxDailyPop, 
			   int *averageSkyCover, int *maxSkyCover, 
			   int *minSkyCover, int *maxSkyNum, int *minSkyNum, 
			   int *startPositions, int *endPositions, 
			   int *maxWindSpeed, int *maxWindDirection, 
			   double startTime_cml, int format_value, int startNum, 
                           int endNum, int f_shiftData, 
                           double *maxWindSpeedValTimes);

void genWindDirectionValues(size_t pnt, char *layoutKey, genMatchType * match,
                            xmlNodePtr parameters, int *maxWindDirection,
                            uChar f_XML, int *numOutputLines,
	                    double *valTimeForWindDirMatch, 
		            numRowsInfo numRows, int startNum, int endNum);

void genWindIncCumValues(size_t pnt, char *layoutKey, uChar parameterName,
		         genMatchType *match, char *windSpeedType, 
		         char *windSpeedName, xmlNodePtr parameters,
		         numRowsInfo numRows, int startNum, int endNum);

void genWindSpeedGustValues(size_t pnt, char *layoutKey,  genMatchType *match, 
                            xmlNodePtr parameters, numRowsInfo numRows, 
                            int startNum, int endNum, sChar f_unit);

void genWindSpeedValues(double timeUserStart, double timeUserEnd, size_t pnt, 
                        char *layoutKey, genMatchType * match,
                        xmlNodePtr parameters, char *startDate,
                        int *maxWindSpeed, int *numOutputLines, int timeInterval,
                        sChar TZoffset, sChar f_observeDST, uChar parameterName,
                        numRowsInfo numRows, uChar f_XML,
                        double *valTimeForWindDirMatch, double startTime, 
                        int startNum, int endNum, int f_shiftData, 
                        sChar f_unit);

void generateConcatTimeLayout(numRowsInfo *numRows, int elemIndex,
                              uChar concatNdfdEnum, char *layoutKey, 
                              const char *timeCoordinate,
                              char *summarization, genMatchType *match,
                              size_t numMatch, uChar f_formatPeriodName,
                              sChar TZoffset, sChar f_observeDST,
                              size_t *numLayoutSoFar,
                              uChar *numCurrentLayout, char *currentHour,
                              char *currentDay, char *frequency,
                              xmlNodePtr data, double startTime_cml,
                              double currentDoubTime,
		      	      uChar f_XML, int startNum, int endNum, 
                              size_t numElem, genElemDescript *elem);

void generateHazTimeLayout(char *layoutKey, const char *timeCoordinate, 
                           hazInfo consecHazRows, char *summarization, 
                           genMatchType *match, size_t numMatch, 
                           double TZoffset, sChar f_observeDST, 
                           size_t **numLayoutSoFar,  uChar **numCurrentLayout, 
                           char *currentHour, char *currentDay, 
                           char *frequency, xmlNodePtr data, 
                           double startTime_cml, double currentDoubTime, 
                           int *numFmtdRows, uChar f_XML, int startNum, 
                           int endNum, double **periodTimes, 
                           int *numPeriodTimes);

void generateNoHazTimeLayout(char *tempBuff, char *layoutKey, 
                             const char *timeCoordinate, char *summarization,
                             double TZoffset, sChar f_observeDST,
                             size_t *numLayoutSoFar, uChar *numCurrentLayout,
                             double firstPeriodStartTime, int numDays,
                             xmlNodePtr data);

void generatePhraseAndIcons (int dayIndex, char *frequency, 
                             int timeLayoutHour, char *dominantWeather[4],
			     char *baseURL, int *maxDailyPop, 
			     int *averageSkyCover, int *maxSkyCover,
			     int *minSkyCover, int *maxSkyNum, 
			     int *minSkyNum, int *periodMaxTemp, 
			     double springDoubleDate, 
			     double fallDoubleDate,  int *maxWindSpeed, 
			     int *maxWindDirection, int *startPositions, 
			     int *endPositions, int f_isDrizzle, 
			     int f_isRain, int f_isRainShowers, 
			     int f_isIcePellets, int f_isSnow, 
			     int f_isSnowShowers, int f_isFreezingDrizzle, 
			     int f_isFreezingRain, int f_isBlowingSnow, 
                             elem_def *wgInfo, elem_def *wsInfo, 
                             double *blizzardTime, int blizzCnt, 
                             double periodStartTime, double periodEndTime, 
                             icon_def *iconInfo, char **phrase, 
                             int *f_popIsNotAnIssue, int numRowsWS, 
                             int numRowsWG, int percentTimeWithFog, 
                             double *maxWindSpeedValTimes);

void generateTimeLayout(numRowsInfo numRows, uChar parameterEnum,
                        char *layoutKey, const char *timeCoordinate,
                        char *summarization, genMatchType * match,
                        size_t numMatch, uChar f_formatPeriodName,
                        double TZoffset, sChar f_observeDST,
                        size_t * numLayoutSoFar,
                        uChar * numCurrentLayout, char *currentHour,
                        char *currentDay, char *frequency,
                        xmlNodePtr data, double startTime_cml,
                        double currentDoubTime, int *numFmtdRows,
			uChar f_XML, int startNum, int endNum);

void getColdSeasonTimes(genMatchType *match, numRowsInfo numRowsWS,
                        sChar TZoffset, double **springDoubleDate, 
			double **fallDoubleDate, int startNum, int endNum);

void getElemIndexes(int *ndfdMaxIndex, int *ndfdMinIndex, int *ndfdPopIndex, 
                    int *ndfdWwaIndex, int *ndfdTempIndex, int *ndfdTdIndex, 
                    int *ndfdQpfIndex, int *ndfdWspdIndex, int *ndfdWdirIndex, 
                    int *ndfdSkyIndex, int *ndfdWgustIndex, 
                    int *rtmaPrecipaIndex, int *rtmaSkyIndex, int *rtmaTdIndex,
                    int *rtmaTempIndex, int *rtmaWdirIndex, int *rtmaWspdIndex, 
                    int *rtmaNdfdSkyIndex, int *rtmaNdfdPrecipaIndex,
                    int *rtmaNdfdTdIndex, int *rtmaNdfdTempIndex,
                    int *rtmaNdfdWdirIndex, int *rtmaNdfdWspdIndex, uChar f_XML, 
                    size_t numElem, genElemDescript *elem);

void getFirstSecondValidTimes(double *firstValidTime, double *secondValidTime,
		              genMatchType *match, size_t numMatch, 
                              uChar parameterName, int startNum, int endNum, 
                              int numRows, int numRowsSkippedBeg, 
                              int numRowsSkippedEnd);

void  getHazPhenAndIcon(char *uglyStr, char *significance, char *transStr,
                        int *f_icon, char *iconStr);

void getNumRows(numRowsInfo *numRowsForPoint, double *timeUserStart, 
		double *timeUserEnd, size_t numMatch, genMatchType *match, 
                uChar *wxParameters, uChar f_XML, sChar *f_icon, sChar TZoffset, 
                sChar f_observeDST, int startNum, int endNum, char *startDate, 
                int *numDays, double startTime, double endTime, 
                char currentHour[3], double *firstValidTime_pop, 
                double *firstValidTimeMatch, int *f_formatIconForPnt, 
                int *f_formatSummarizations, size_t pnt, int *pnt_rtmaNdfdTemp, 
                int *pnt_rtmaNdfdTd, int *pnt_rtmaNdfdWdir, 
                int *pnt_rtmaNdfdWspd, int *pnt_rtmaNdfdPrecipa, 
                int *pnt_rtmaNdfdSky, double currentDoubTime, size_t numElem, 
                genElemDescript *elem);

void getPeriodInfo(uChar parameterEnum, char *firstValidTime, char *currentHour, 
                   char *currentDay, uChar * issuanceType, 
                   uChar * numPeriodNames, int period, char *frequency);

void getPeriodTimes(double firstPeriodStartTime, double lastPeriodEndTime, 
                    int timeInterval, double TZoffset, sChar f_observeDST, 
                    double **periodTimes, int *numPeriodTimes);

void getSectorInfo(PntSectInfo *pntInfo, Point *pnts, size_t numPnts,
                   genMatchType *match, size_t numMatch, 
                   size_t numSector, char **sector, int f_conus2_5, 
                   int numConus2_5, int f_conus5, int numConus5, int f_nhemi, 
                   int numNhemi, int f_npacocn, int numNpacocn);

void getStartDates(char **startDate, uChar f_XML, double startTime, 
		   double firstValidTimeMatch, double firstValidTime_maxt,
                   sChar TZoffset, sChar f_observeDST, size_t point);

void getTranslatedCoverage(char *uglyStr, char *transStr);

void getTranslatedFireWx(int fireWxCat, char *transStr);

void getTranslatedHzSig(char *uglyStr, char *transStr);

void getTranslatedIntensity(char *uglyStr, char *transStr);

void getTranslatedQualifier(char *uglyStr, char *transStr);

void getTranslatedRisk(int convHazCat, char *transStr);

void getTranslatedType(char *uglyStr, char *transStr);

void getTranslatedVisibility(sChar f_unit, char *uglyStr, char *transStr);

void getUserTimes(double **timeUserStart, double **timeUserEnd, 
                  int *f_POPUserStart, char *startDate, sChar TZ, 
                  double startTime, sChar f_observeDST, int *numDays, 
                  double *firstValidTime_pop, sChar f_XML, 
                  double *firstValidTimeMatch);

void hazStartEndTimes(char ***startTimes, char ***endTimes, int **numFmtdRows, 
                      hazInfo consecHazRows, double **periodTimes, 
                      int *nmPeriodTimes, double TZoffset, sChar f_observeDST);

void hazTimeInfo(char ***multiLayouts, const char *timeCoordinate, 
                 hazInfo *ptsIndivHazs, int numHazards, char *summarization, 
                 genMatchType *match, size_t numMatch, double TZoffset, 
                 sChar f_observeDST, char *currentHour, char *currentDay, 
                 char *frequency, xmlNodePtr data, double startTime_cml, 
                 double currentDoubTime, uChar f_XML, int startNum, int endNum,
                 double firstPeriodStartTime, double lastPeriodEndTime, 
                 int timeInterval, double **periodTimes, int *numPeriodTimes, 
                 size_t *numLayoutSoFar, uChar *numCurrentLayout);

int isDominant(char *arg1, char *arg2, char *argType);

int isNewLayout(layouts newLayout, size_t * numLayoutSoFar,
                uChar * numCurrentLayout, int f_finalTimeLayout);

void monthDayYearTime(genMatchType * match, size_t numMatch,
                      char *currentLocalTime, char *currentDay,
                      sChar f_observeDST, double *firstMaxTValidTime_doub_adj,
                      double *currentLocalTime_doub_adj, sChar TZoffset,
		      int startNum, int endNum, numRowsInfo numRows);

void prepareDWMLgen(uChar f_XML, uChar * f_formatPeriodName,
                    uChar ***wxParameters, size_t numPnts, char *summarization,
                    uChar varFilter[NDFD_MATCHALL + 1], sChar * f_icon, 
                    size_t *numElem, genElemDescript **elem);

void prepareDWMLgenByDay(genMatchType *match, uChar f_XML, 
                         double *startTime_cml, double *endTime_cml,
			 double *firstValidTimeMatch, int *numDays, 
                         char *format, uChar *f_formatPeriodName,
                         uChar **wxParameters, int *timeInterval,
                         int *numOutputLines, char *summarization,
			 double currDoubTime, size_t numPnts, 
                         PntSectInfo *pntInfo, char **currentLocalDate, 
                         size_t numElem, genElemDescript *elem, 
                         uChar varFilter[NDFD_MATCHALL+1]);

void prepareVarFilter(sChar f_XML, sChar *f_icon, size_t numNdfdVars, 
                      uChar *ndfdVars, uChar varFilter[NDFD_MATCHALL+1], 
                      size_t *numElem, genElemDescript **elem); 

void prepareWeatherValuesByDay (genMatchType *match, sChar TZoffset,
		                sChar f_observeDST, char *frequency,
				int *numDays, int *numOutputLines,
				numRowsInfo numRowsWS, 
				numRowsInfo numRowsMIN,
 			        numRowsInfo numRowsMAX, uChar f_XML, 
                                numRowsInfo numRowsPOP, 
				size_t pnt, int f_useMinTempTimes, 
				double startTime_cml,
				int *periodMaxTemp, 
				double *periodStartTimes, 
				double *periodEndTimes,
				double *springDoubleDate, 
                                double *fallDoubleDate, 
				int *timeLayoutHour, int startNum, int endNum);

void PrintDay1(genMatchType * match, size_t pntIndex, collateType *collate, 
               size_t numCollate, sChar pntTimeZone, sChar f_dayCheck);

void PrintTime(genMatchType * match, size_t pntIndex, int *allElem, 
               sChar pntTimeZone, sChar f_dayCheck);

int roundPopNearestTen(int num);

void rtmaFileNames(size_t *numInFiles, char ***inFiles, char *directoryTail, 
                   char *rtmaSetDir);

void setVarFilter(sChar f_XML, sChar *f_icon, size_t numNdfdVars, 
                  const uChar *ndfdVars, uChar varFilter[NDFD_MATCHALL+1]);

void skyPhrase(int *maxSkyCover, int *minSkyCover, int *averageSkyCover, 
	       int dayIndex, int f_isDayTime, int f_isNightTime, int *maxSkyNum,  
	       int *minSkyNum, int *startPositions, int *endPositions,
               char *baseURL, icon_def *iconInfo, char **phrase);

void spreadPOPsToWxTimes(int *POP12SpreadToPOP3, WX *wxInfo, int numRowsWX,
                         elem_def *popInfo, int numRowsPOP);

void tempExtremePhrase(int f_isDayTime, int *periodMaxTemp, int dayIndex, 
                       char *baseURL, icon_def *iconInfo, char **phrase);

int useNightPeriodName(char *dataTime);

void windExtremePhrase(int f_isDayTime, int f_isNightTime, int dayIndex, 
                       char *baseURL, double springDoubleDate, 
		       double fallDoubleDate, int *maxWindSpeed, 
	               int *maxWindDirection, int *periodMaxTemp, 
		       icon_def *iconInfo, char **phrase, 
                       double *maxWindSpeedValTimes);

int XMLmatchCompare(const void *A, const void *B);

int XMLParse(uChar f_XML, size_t numPnts, Point * pnts,
             PntSectInfo * pntInfo, sChar f_pntType, char **labels,
             size_t *numInFiles, char ***inFiles, uChar f_fileType,
             sChar f_interp, sChar f_unit, double majEarth, double minEarth,
             sChar f_icon, sChar f_SimpleVer, sChar f_SimpleWWA, sChar f_valTime,
             double startTime, double endTime, size_t numNdfdVars, 
             uChar *ndfdVars, char *f_inTypes, char *gribFilter, 
             size_t numSector, char **sector, sChar f_ndfdConven, 
             char *rtmaDataDir, sChar f_avgInterp, char *lampDataDir);

#endif
