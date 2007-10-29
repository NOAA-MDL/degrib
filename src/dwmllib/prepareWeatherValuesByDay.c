/*****************************************************************************
 * prepareWeatherValuesByDay() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Prepares and aligns temperature and weather data for the weather 
 *   summarization and generation. Routine get the weather times from the match
 *   structure and transforms to double expressions. Routine also uses the MaxT
 *   and Mint elements to get the "max" temp per period.
 *   
 * ARGUMENTS
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *   frequency = The type of summarization done, if any. Set to "boggus" for 
 *               DWMLgen products, and to "12hourly" or "24hourly" for the 
 *               DWMLgenByDay products. (Input) 
 *     numDays = The number of days the validTimes for all the data rows 
 *               (values) consist of. (Input)
 * numOutputLines = The number of data values output/formatted for each element. 
 *                  (Input)
 *    numMatch = The number of matches from degrib. (Input)
 *   numRowsWS = The number of data rows for wind speed. These data are used to
 *               derive icons, if icons are to be formatted. (Input)
 *  numRowsPOP = The number of data rows for PoP12hr. (Input)
 *  numRowsMIN = The number of data rows for MinT. (Input)
 *  numRowsMAX = The number of data rows for MaxT.  (Input)
 *       f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product. (Input)
 *         pnt = The point index. (Input)
 * f_useMinTempTimes = Flag denoting if we should simply use the MinT 
 *                     layout key for the MaxT element. Only used in 
 *                     DWMLgenByDay products. (Input)
 *   startTime_cml = Incoming argument set by user as a double in seconds 
 *                   since 1970 denoting the starting time data is to be 
 *                   retrieved for from NDFD. (Set to 0.0 if not entered.)
 *                   (Input) 
 *    TZoffset = Number of hours to add to current time to get GMT time. 
 *               (Input)
 * f_observeDST = Flag determining if current point observes Daylight 
 *                Savings Time. (Input)  
 * periodMaxTemp = For each forecast period, the "max" temperature occuring in
 *                 the period, based off of the MaxT and MinT elements. If night, 
 *                 (f_XML = 3) the period could have a "max" MinT. (Output)
 * periodStartTimes = The startTimes of each of the forecast periods. (Output)
 * periodEndTimes =   The endTimes of each of the forecast periods. (Output)            
 * springDoubleDate = The end date of next cold season expressed in double form.
 *                    (Output)
 * fallDoubleDate = The start date of next cold season expressed in double form. 
 *                  (Output)
 * timeLayoutHour = The time period's hour for the 12 hourly product. Used to 
 *                  determine if it is night or day in the 
 *                  generatePhraseAndIcons routine (should = 6 or 18). (Output)                 
 *       startNum = First index in match structure an individual point's data 
 *                  matches can be found. (Input)
 *         endNum = Last index in match structure an individual point's data
 *                  matches can be found. (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void prepareWeatherValuesByDay (genMatchType *match, sChar TZoffset,
		                sChar f_observeDST, char *frequency,
				int *numDays, int *numOutputLines,
				numRowsInfo numRowsWS, 
				numRowsInfo numRowsMIN,
 			        numRowsInfo numRowsMAX, uChar f_XML, 
                                numRowsInfo numRowsPOP, 
				size_t pnt, int f_useMinTempTimes, 
				double startTime_cml, int *periodMaxTemp,
				double *periodStartTimes, 
				double *periodEndTimes,
				double *springDoubleDate, 
                                double *fallDoubleDate, 
				int *timeLayoutHour, int startNum, int endNum)
{
   int i;                     /* Index through the match structure. */
   int j;                     /* Period Counter. */   	
   int limit;   
   int priorElemCount = 0;        /* Counter used to find elements' location in
                                   * match. */
   double currentDoubTime;
   char **startTimesMinTemp = NULL; /* Array holding startTimes for MinT. */ 
   char **endTimesMinTemp = NULL;    /* 7-13T18:00:00-06:00 mx:9Array holding the end Times for MinT. */
   char **startTimesMaxTemp = NULL; /* Array holding start Times for MaxT. */ 
   char **endTimesMaxTemp = NULL;    /* Array holding the end Times for MaxT. */
   char **startTimesPop = NULL; /* Array holding start Times for Pop (or 12hr 
				   elements). */ 
   char **endTimesPop = NULL;    /* Array holding the end Times for Pop (or 12hr 
				   elements). */
   char maxTempDay[3]; /* 2-digit day first MaxT falls in. */
   char minTempDay[3]; /* 2-digit day first MinT falls in. */
   char layoutHour[3]; /* Used to retrieve the timeLayoutHour. */
   int numActualRowsMAX; /* MaxT # of rows interested in taking into accordance 
                          * a user supplied shortened time period. */
   int numActualRowsMIN; /* MinT # of rows interested in taking into accordance 
                          * a user supplied shortened time period. */
   int numActualRowsPOP; /* Pop12 # of rows interested in taking into accordance 
                          * a user supplied shortened time period. */
   
   /* Initialize a few things. */
   numActualRowsMIN = numRowsMIN.total-numRowsMIN.skipBeg-numRowsMIN.skipEnd;   
   numActualRowsMAX = numRowsMAX.total-numRowsMAX.skipBeg-numRowsMAX.skipEnd;   
   numActualRowsPOP = numRowsPOP.total-numRowsPOP.skipBeg-numRowsPOP.skipEnd; 
   
   /* Get the current time in seconds. */
   currentDoubTime = Clock_Seconds();
   
   /* Based on the current month, we need to know the start and end of the cold
    * season (i.e. October 2004 - April 2005). Retrieve this info in double form.
    */
   getColdSeasonTimes (match, numRowsWS, TZoffset, &springDoubleDate,
		       &fallDoubleDate, startNum, endNum);

   /* Create arrays holding each period's MaxT value (this value could be 
    * a "max" minimum Temp if periods are 12 hours long) and the start and end
    * times of the periods.
    */
   if (strcmp (frequency, "24 hourly") == 0)
   {
      /* Get the MaxT values. */
      for (i = startNum; i < endNum; i++)
      {
         if (match[i].elem.ndfdEnum == NDFD_MAX && 
	     match[i].validTime >= numRowsMAX.firstUserTime &&
             match[i].validTime <= numRowsMAX.lastUserTime)
         { 
            if ((i-priorElemCount-startNum) < *numDays)
            {
               if (match[i].value[pnt].valueType == 0)
               {
                  periodMaxTemp[i-priorElemCount-startNum] = 
                                     (int)myRound(match[i].value[pnt].data, 0);
               }
	    }
	 }
         else
            priorElemCount++;
      }

      /* Get the start and end Times for these periods. */	   
      if (f_useMinTempTimes) /* Use the MinT's start and end times as period 
                              * times. 
                              */
      {
         startTimesMinTemp = (char **) malloc(numActualRowsMIN * sizeof(char *));
         endTimesMinTemp   = (char **) malloc(numActualRowsMIN * sizeof(char *));
         computeStartEndTimes (NDFD_MIN, numActualRowsMIN, 24, TZoffset, 
			       f_observeDST, match, 1, startTimesMinTemp, 
                               endTimesMinTemp, frequency, f_XML, startTime_cml,
                               currentDoubTime, numRowsMIN, startNum, endNum);
	 
	 if (numActualRowsMIN < *numDays)
            limit = numActualRowsMIN;
         else
            limit = *numDays;
	 
	 for (i = 0; i < limit; i++)
         {
            Clock_Scan (&periodStartTimes[i], startTimesMinTemp[i], 0);
	    Clock_Scan (&periodEndTimes[i], endTimesMinTemp[i], 0);
         }

         /* See if there is an extra day tacked on the end weather can fall into,
            even if there are less MaxT or MinT "days".
          */
         if (numActualRowsMIN < *numDays)
         {
            for (i = numActualRowsMIN; i < *numDays; i++)
            {
               Clock_Scan(&periodStartTimes[i], startTimesMinTemp[i-1], 0);
               periodStartTimes[i] = periodStartTimes[i] + (24 * 3600);              
               Clock_Scan(&periodEndTimes[i], endTimesMinTemp[i-1], 0);
               periodEndTimes[i] = periodEndTimes[i] + (24 * 3600);
            }
         }
      }
      else /* Use the MaxT's start and end times as period times. */
      {
         startTimesMaxTemp = (char **) malloc(numActualRowsMAX * sizeof(char *));
         endTimesMaxTemp   = (char **) malloc(numActualRowsMAX * sizeof(char *));
         computeStartEndTimes (NDFD_MAX, numActualRowsMAX, 24, TZoffset, f_observeDST,
		               match, 1, startTimesMaxTemp, endTimesMaxTemp,
			       frequency, f_XML, startTime_cml, currentDoubTime, 
                               numRowsMAX, startNum, endNum);
		 
	 if (numActualRowsMAX < *numDays)
            limit = numActualRowsMAX;
         else
            limit = *numDays;
	 
	 for (i = 0; i < limit; i++)
         {
            Clock_Scan (&periodStartTimes[i], startTimesMaxTemp[i], 0);
	    Clock_Scan (&periodEndTimes[i], endTimesMaxTemp[i], 0);
         }
         /* See if there is an extra day tacked on the end weather can fall into,
          * even if there are less MaxT or MinT "days".
          */
         if (numActualRowsMAX < *numDays)
         {
            for (i = numActualRowsMAX; i < *numDays; i++)
            {
               Clock_Scan(&periodStartTimes[i], startTimesMaxTemp[i-1], 0);
               periodStartTimes[i] = periodStartTimes[i] + (24 * 3600);
               Clock_Scan(&periodEndTimes[i], endTimesMaxTemp[i-1], 0);
               periodEndTimes[i] = periodEndTimes[i] + (24 * 3600);
            }
         }

      }
   }
   else if (strcmp (frequency, "12 hourly") == 0)
   { 
	   
   /* We need to know which temperature to start with (max or min)
    * If the end day of the max is different from the min, then start with the max
    * otherwise start with the min since the max is for tomorrow.
    */
      startTimesMaxTemp = (char **) malloc(numActualRowsMAX * sizeof(char *));
      endTimesMaxTemp = (char **) malloc(numActualRowsMAX * sizeof(char *));
      computeStartEndTimes (NDFD_MAX, numActualRowsMAX, 24, TZoffset, f_observeDST,
		            match, 1, startTimesMaxTemp, endTimesMaxTemp, 
                            frequency, f_XML, startTime_cml, currentDoubTime, 
                            numRowsMAX, startNum, endNum);
	   	   
      maxTempDay[0] = endTimesMaxTemp[0][8];
      maxTempDay[1] = endTimesMaxTemp[0][9];
      maxTempDay[2] = '\0';
      
      startTimesMinTemp = (char **) malloc(numActualRowsMIN * sizeof(char *));
      endTimesMinTemp = (char **) malloc(numActualRowsMIN * sizeof(char *));
      computeStartEndTimes (NDFD_MIN, numActualRowsMIN, 24, TZoffset, f_observeDST,
		            match, 1, startTimesMinTemp, endTimesMinTemp, 
                            frequency, f_XML, startTime_cml, currentDoubTime, 
                            numRowsMIN, startNum, endNum);
	   	   
      minTempDay[0] = endTimesMinTemp[0][8];
      minTempDay[1] = endTimesMinTemp[0][9];
      minTempDay[2] = '\0';
      
      if (strcmp (maxTempDay, minTempDay) == 0)
      {
	      
         /* Start with Min Temps, skipping periods since MinT's occur every
	  * 24 hours but need to be placed in appropriate, alternating
	  * periods. 
	  */ 
	 priorElemCount = 0;
         for (i = startNum, j = startNum; i < endNum; i++, j=j+2)
         {
            if (match[i].elem.ndfdEnum == NDFD_MIN && 
	        match[i].validTime >= numRowsMIN.firstUserTime &&
	        match[i].validTime <= numRowsMIN.lastUserTime)
            { 
               if ((i-priorElemCount-startNum) < *numDays)
               {
	          if (match[i].value[pnt].valueType == 0)
                  {
                     periodMaxTemp[j-startNum-(priorElemCount*2)] = 
		        (int)myRound(match[i].value[pnt].data, 0);
                  }
	       }
	    }
	    else
	       priorElemCount++;
         }

         /* Alternate with the Max Temps. */
	 priorElemCount = 0;
         for (i = startNum, j = startNum+1; i < endNum; i++, j=j+2)
         {
            if (match[i].elem.ndfdEnum == NDFD_MAX && 
	        match[i].validTime >= numRowsMAX.firstUserTime &&
	        match[i].validTime <= numRowsMAX.lastUserTime)
            { 
               if (i-priorElemCount-startNum < *numDays)
	       {
	          if (match[i].value[pnt].valueType == 0)
                  {
                     periodMaxTemp[j-priorElemCount-startNum] = 
                                     (int)myRound(match[i].value[pnt].data, 0);
                  }                     
	       }
	    }
            else
               priorElemCount++;
         }
      }
      else  /* Start with Max Temps. */
      {	  
         priorElemCount = 0;    
         for (i = startNum, j = startNum; i < endNum; i++, j=j+2)
         {
 	    if (match[i].elem.ndfdEnum == NDFD_MAX && 
	        match[i].validTime >= numRowsMAX.firstUserTime &&
	        match[i].validTime <= numRowsMAX.lastUserTime)
            { 
               if (i-priorElemCount-startNum < *numDays)
	       {
                  if (match[i].value[pnt].valueType == 0)
                  {
      		       periodMaxTemp[j-priorElemCount-startNum] = 
                                     (int)myRound(match[i].value[pnt].data, 0);
                  }
               }	   
	    }
            else
               priorElemCount++;
         }

         /* Alternate with the Min Temps. */
	 priorElemCount = 0;
         for (i = startNum, j = startNum+1; i < endNum; i++, j=j+2)
         {
 	    if (match[i].elem.ndfdEnum == NDFD_MIN && 
	        match[i].validTime >= numRowsMIN.firstUserTime &&
	        match[i].validTime <= numRowsMIN.lastUserTime)
            { 
               if ((i-priorElemCount-startNum) < *numDays)
	       {
                  if (match[i].value[pnt].valueType == 0)
                  {
                     periodMaxTemp[j-startNum-(priorElemCount*2)] = 
		        (int)myRound(match[i].value[pnt].data, 0);
                  }
	       }
	    }
	    else
	       priorElemCount++;
         }
      }
      
      /* Now, get the start and end Times for these periods. Since the periods
       * are 12 hours long now, use POP12hr element's times for period times. */
      startTimesPop = (char **) malloc(numActualRowsPOP * sizeof(char *));
      endTimesPop   = (char **) malloc(numActualRowsPOP * sizeof(char *));
      computeStartEndTimes (NDFD_POP, numActualRowsPOP, 12, TZoffset, 
		            f_observeDST, match, 1, startTimesPop, 
			    endTimesPop, frequency, f_XML, startTime_cml, 
			    currentDoubTime, numRowsPOP, startNum, endNum);

      /* We need the unchanging time period's hour for use in the
       * generatePhraseAndIcons routine. Grab it here. 
       */
      layoutHour[0] = startTimesPop[0][11];
      layoutHour[1] = startTimesPop[0][12];
      layoutHour[2] = '\0';
      *timeLayoutHour = atoi(layoutHour);

      if (numActualRowsPOP < (*numDays)*2)
         limit = numActualRowsPOP;
      else
         limit = (*numDays)*2;
	 
      for (i = 0; i < limit; i++)
      {
	 Clock_Scan (&periodStartTimes[i], startTimesPop[i], 0);
 	 Clock_Scan (&periodEndTimes[i], endTimesPop[i], 0);
      }

      /* Is there an extra period or two tacked on the end weather can fall into, even
       * if there are less POP12hr periods.
       */
      if (numActualRowsPOP < (*numDays)*2)
      {
         for (i = numActualRowsPOP; i < (*numDays)*2; i++)
         {
            Clock_Scan(&periodStartTimes[i], startTimesPop[i-1], 0);
            periodStartTimes[i] = periodStartTimes[i] + (12 * 3600);
            Clock_Scan(&periodEndTimes[i], endTimesPop[i-1], 0);
            periodEndTimes[i] = periodEndTimes[i] + (12 * 3600);
         }
      }
   }
   
   /* Free Max Temp Time arrays. */
   if (strcmp (frequency, "12 hourly") == 0 || !f_useMinTempTimes)
   {
      for (i = 0; i < numActualRowsMAX; i++)
      {
         free (startTimesMaxTemp[i]);
         free (endTimesMaxTemp[i]);
      }
      free (startTimesMaxTemp);
      free (endTimesMaxTemp);
   }

   /* Free Min Temp Time arrays. */   
   if (strcmp (frequency, "12 hourly") == 0 || f_useMinTempTimes)
   {
      for (i = 0; i < numActualRowsMIN; i++)
      {
         free (startTimesMinTemp[i]);
         free (endTimesMinTemp[i]);
      }
      free (startTimesMinTemp);
      free (endTimesMinTemp);
   }
   
   /* Free Pop Time arrays (only used for 12 hourly product). */
   if (strcmp (frequency, "12 hourly") == 0)   
   {
      for (i = 0; i < numActualRowsPOP; i++)
      {
         free (startTimesPop[i]);
         free (endTimesPop[i]);
      }
      free (startTimesPop);
      free (endTimesPop);
   }
     
   return;
}
