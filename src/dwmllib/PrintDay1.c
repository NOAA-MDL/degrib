/* Print Routine #2 for diagnostics for XMLParse(). */
#include "xmlparse.h"
void PrintDay1(genMatchType * match, size_t pntIndex, collateType *collate, 
               size_t numCollate, sChar pntTimeZone, sChar f_dayCheck)
{
   size_t i;
   size_t j;
   int *dayIndex;
   int numDayIndex = 1;
   sInt4 totDay;              /* # of days since epoch in LST (LDT ignored). */
   sInt4 curTotDay;           /* # of days since epoch in LST (LDT ignored). */

   dayIndex = malloc(numDayIndex * sizeof(int));
   dayIndex[numDayIndex - 1] = 0;

   curTotDay = (sInt4) floor((collate[0].validTime -
                              pntTimeZone * 3600) / SEC_DAY);
   for (i = 1; i < numCollate; i++)
   {
      totDay = (sInt4) floor((collate[i].validTime -
                              pntTimeZone * 3600) / SEC_DAY);
      if (totDay != curTotDay)
      {
         curTotDay = totDay;
         numDayIndex++;
         dayIndex = realloc(dayIndex, numDayIndex * sizeof(int));
         dayIndex[numDayIndex - 1] = i;
      }
   }
   if (dayIndex[numDayIndex - 1] != i)
   {
      numDayIndex++;
      dayIndex = realloc(dayIndex, numDayIndex * sizeof(int));
      dayIndex[numDayIndex - 1] = i;
   }

   for (i = 0; i < numDayIndex - 1; i++)
   {
      for (j = dayIndex[i]; j < dayIndex[i + 1]; j++)
      {
         PrintTime(match, pntIndex, collate[j].allElem, pntTimeZone,
                   f_dayCheck);
      }
      printf("--- End of day ---\n");
   }

   free(dayIndex);
}
