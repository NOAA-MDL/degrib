#!/bin/make

# Set by calling makefile CC='$(CC)'
# Set by calling makefile CFLAGS='$(CFLAGS)'
AR=ar
RANLIB=ranlib
ARFLAGS=ruv
LIBNAME=dwml
LIBA=lib$(LIBNAME).a


C_OBJS=$(LIBA)(PrintDay1.o PrintTime.o XMLParse.o XMLmatchCompare.o anyLampElements.o anyRtmaElements.o blizzardCheck.o checkNeedForEndTime.o checkNeedForPeriodName.o collectHazInfo.o computeStartEndTimes.o concatRtmaNdfdValues.o determineIconUsingPop.o determineNonWeatherIcons.o determinePeriodLength.o determineSkyIcons.o determineWeatherIcons.o dwmlEnumSort.o formatLocationInfo.o formatMetaDWML.o formatMoreWxInfo.o formatValidTime.o genAppTempValues.o genClimateOutlookValues.o genConvOutlookValues.o genConvSevereCompValues.o genDewPointTempValues.o genFireWxValues.o genHazTextURL.o genHazardSummaryValues.o genHazardValues.o genIceAccValues.o genIconLinks.o genLampTstmValues.o genMaxRHValues.o genMaxTempValues.o genMinRHValues.o genMinTempValues.o genPopValues.o genQPFValues.o genRelHumidityValues.o genRtmaValues.o genSkyCoverValues.o genSnowValues.o genTempValues.o genWaveHeightValues.o genWeatherValues.o genWeatherValuesByDay.o genWindDirectionValues.o genWindIncCumValues.o genWindSpeedGustValues.o genWindSpeedValues.o generateConcatTimeLayout.o generateHazTimeLayout.o generateNoHazTimeLayout.o generatePhraseAndIcons.o generateTimeLayout.o getColdSeasonTimes.o getElemIndexes.o getFirstSecondValidTimes.o getHazPhenAndIcon.o getNumRows.o getPeriodInfo.o getPeriodTimes.o getSectorInfo.o getStartDates.o getTranslatedCoverage.o getTranslatedFireWx.o getTranslatedHzSig.o getTranslatedIntensity.o getTranslatedQualifier.o getTranslatedRisk.o getTranslatedType.o getTranslatedVisibility.o getUserTimes.o hazTimeInfo.o hazStartEndTimes.o isDominant.o isNewLayout.o monthDayYearTime.o prepareDWMLgen.o prepareDWMLgenByDay.o prepareVarFilter.o prepareWeatherValuesByDay.o roundPopNearestTen.o rtmaFileNames.o setVarFilter.o skyPhrase.o spreadPOPSToWxTimes.o tempExtremePhrase.o useNightPeriodName.o windExtremePhrase.o)

H_SOURCES=xmlparse.h

all: $(LIBA)

$(LIBA): $(C_OBJS) $(H_SOURCES)
	$(RANLIB) $(LIBA)

install:

clean:
	rm -f *.o *.a

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.o.a:
	$(AR) $(ARFLAGS) $@ $<

