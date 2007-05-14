/* Print Routine #1 for diagnostics for XMLParse(). */
#include "xmlparse.h"
void PrintTime(genMatchType * match, size_t pntIndex, int *allElem, 
               sChar pntTimeZone, sChar f_dayCheck)
{
   char timeBuff[30];
   char zone[7];
   size_t i;
   double localTime;

   for (i = 0; i < NDFD_MATCHALL + 1; i++)
   {
      if (allElem[i] != -1)
      {
         localTime = match[allElem[i]].validTime - pntTimeZone * 3600;
         /* localTime is now in local standard time */
         if (f_dayCheck)
         {
            /* Note: A 0 is passed to DaylightSavings so it converts from
             * local to local standard time. */
            if (Clock_IsDaylightSaving2(localTime, 0) == 1)
            {
               localTime += 3600;
               pntTimeZone -= 1;
            }
         }
         /* The 0, 0 is because we already converted to local standard /
          * daylight time. */
         Clock_Print2(timeBuff, 30, localTime, "%Y-%m-%dT%H:%M:%S", 0, 0);
         /* Change definition of pntTimeZone. */
         pntTimeZone = -1 * pntTimeZone;
         if (pntTimeZone < 0)
         {
            sprintf(zone, "-%02d:00", -1 * pntTimeZone);
         }
         else
         {
            sprintf(zone, "+%02d:00", pntTimeZone);
         }
         strcat(timeBuff, zone);
         break;
      }
   }
   /* Check that we have some elements. */
   if (i == NDFD_MATCHALL + 1)
      return;

   printf("%s ", timeBuff);
   if ((allElem[NDFD_TEMP] != -1) &&
       (match[allElem[NDFD_TEMP]].value[pntIndex].valueType != 2))
      {
         printf("tt:%.0f ", match[allElem[NDFD_TEMP]].value[pntIndex].data);
         printf ("match[%d].f_sector = %d\n",allElem[NDFD_TEMP],match[allElem[NDFD_TEMP]].f_sector);
      }
   if ((allElem[NDFD_AT] != -1) &&
       (match[allElem[NDFD_AT]].value[pntIndex].valueType != 2))
      {
         printf("at:%.0f ", match[allElem[NDFD_AT]].value[pntIndex].data);
         printf ("match[%d].f_sector = %d\n",allElem[NDFD_AT],match[allElem[NDFD_AT]].f_sector);
      }

   if ((allElem[NDFD_MAX] != -1) &&
       (match[allElem[NDFD_MAX]].value[pntIndex].valueType != 2))
   {
      printf("mx:%.0f ", match[allElem[NDFD_MAX]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_MAX],match[allElem[NDFD_MAX]].f_sector);
   }
   else if ((allElem[NDFD_MIN] != -1) &&
            (match[allElem[NDFD_MIN]].value[pntIndex].valueType != 2))
   {
      printf("mn:%.0f ", match[allElem[NDFD_MIN]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_MIN],match[allElem[NDFD_MIN]].f_sector);
   }
   else
   {
      printf("  :   ");
   }

   if ((allElem[NDFD_TD] != -1) &&
       (match[allElem[NDFD_TD]].value[pntIndex].valueType != 2))
   {
      printf("td:%.0f ", match[allElem[NDFD_TD]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_TD],match[allElem[NDFD_TD]].f_sector);
   }
   if ((allElem[NDFD_POP] != -1) &&
       (match[allElem[NDFD_POP]].value[pntIndex].valueType != 2))
   {
      printf("po:%.0f ", match[allElem[NDFD_POP]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_POP],match[allElem[NDFD_POP]].f_sector);
   }
   else
   {
      printf("  :   ");
   }
   if ((allElem[NDFD_RH] != -1) &&
       (match[allElem[NDFD_RH]].value[pntIndex].valueType != 2))
   {
      printf("rh:%.0f ", match[allElem[NDFD_RH]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_RH],match[allElem[NDFD_RH]].f_sector);
   }
   if ((allElem[NDFD_CUM34] != -1) &&
       (match[allElem[NDFD_CUM34]].value[pntIndex].valueType != 2))
   {
      printf("tcs34c:%.0f ", match[allElem[NDFD_CUM34]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_CUM34],match[allElem[NDFD_CUM34]].f_sector);
   }
   if ((allElem[NDFD_CUM50] != -1) &&
       (match[allElem[NDFD_CUM50]].value[pntIndex].valueType != 2))
   {
      printf("tcs50c:%.0f ", match[allElem[NDFD_CUM50]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_CUM50],match[allElem[NDFD_CUM50]].f_sector);
   }
   if ((allElem[NDFD_CUM64] != -1) &&
       (match[allElem[NDFD_CUM64]].value[pntIndex].valueType != 2))
   {
      printf("tcs64c:%.0f ", match[allElem[NDFD_CUM64]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_CUM64],match[allElem[NDFD_CUM64]].f_sector);
   }
   if ((allElem[NDFD_INC34] != -1) &&
       (match[allElem[NDFD_INC34]].value[pntIndex].valueType != 2))
   {
      printf("tcs34i:%.0f ", match[allElem[NDFD_INC34]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_INC34],match[allElem[NDFD_INC34]].f_sector);
   }
   if ((allElem[NDFD_INC50] != -1) &&
       (match[allElem[NDFD_INC50]].value[pntIndex].valueType != 2))
   {
      printf("tcs50i:%.0f ", match[allElem[NDFD_INC50]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_INC50],match[allElem[NDFD_INC50]].f_sector);
   }
   if ((allElem[NDFD_INC64] != -1) &&
       (match[allElem[NDFD_INC64]].value[pntIndex].valueType != 2))
   {
      printf("tcs64i:%.0f ", match[allElem[NDFD_INC64]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_INC64],match[allElem[NDFD_INC64]].f_sector);
   }
   if ((allElem[NDFD_SKY] != -1) &&
       (match[allElem[NDFD_SKY]].value[pntIndex].valueType != 2))
   {
      printf("sky:%.0f ", match[allElem[NDFD_SKY]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_SKY],match[allElem[NDFD_SKY]].f_sector);
   }
   if ((allElem[NDFD_QPF] != -1) &&
       (match[allElem[NDFD_QPF]].value[pntIndex].valueType != 2))
   {
      printf("qp:%5.2f ", match[allElem[NDFD_QPF]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_QPF],match[allElem[NDFD_QPF]].f_sector);
   }
   else
   {
      printf("  :      ");
   }
   if ((allElem[NDFD_SNOW] != -1) &&
       (match[allElem[NDFD_SNOW]].value[pntIndex].valueType != 2))
   {
      printf("sn:%5.2f ", match[allElem[NDFD_SNOW]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_SNOW],match[allElem[NDFD_SNOW]].f_sector);
   }
   else
   {
      printf("  :      ");
   }
   if ((allElem[NDFD_WG] != -1) &&
       (match[allElem[NDFD_WG]].value[pntIndex].valueType != 2))
      {
         printf("wg:%2.0f ", match[allElem[NDFD_WG]].value[pntIndex].data);
         printf ("match[%d].f_sector = %d\n",allElem[NDFD_WG],match[allElem[NDFD_WG]].f_sector);
      } 

   if ((allElem[NDFD_WS] != -1) &&
       (match[allElem[NDFD_WS]].value[pntIndex].valueType != 2))
      {
         printf("ws:%2.0f ", match[allElem[NDFD_WS]].value[pntIndex].data);
         printf ("match[%d].f_sector = %d\n",allElem[NDFD_WS],match[allElem[NDFD_WS]].f_sector);
      } 
   if ((allElem[NDFD_CONHAZ] != -1) &&
       (match[allElem[NDFD_CONHAZ]].value[pntIndex].valueType != 2))
   {
      printf("convHaz:%.0f ", match[allElem[NDFD_CONHAZ]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_CONHAZ],match[allElem[NDFD_CONHAZ]].f_sector);
   } 
   if ((allElem[NDFD_PTORN] != -1) &&
       (match[allElem[NDFD_PTORN]].value[pntIndex].valueType != 2))
   {
      printf("ptorn:%.0f ", match[allElem[NDFD_PTORN]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PTORN],match[allElem[NDFD_PTORN]].f_sector);
   } 
   if ((allElem[NDFD_PHAIL] != -1) &&
       (match[allElem[NDFD_PHAIL]].value[pntIndex].valueType != 2))
   {
      printf("phail:%.0f ", match[allElem[NDFD_PHAIL]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PHAIL],match[allElem[NDFD_PHAIL]].f_sector);
   } 
   if ((allElem[NDFD_PTSTMWIND] != -1) &&
       (match[allElem[NDFD_PTSTMWIND]].value[pntIndex].valueType != 2))
   {
      printf("ptstmwind:%.0f ", match[allElem[NDFD_PTSTMWIND]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PTSTMWIND],match[allElem[NDFD_PTSTMWIND]].f_sector);
   } 
   if ((allElem[NDFD_PXTORN] != -1) &&
       (match[allElem[NDFD_PXTORN]].value[pntIndex].valueType != 2))
   {
      printf("pxtorn:%.0f ", match[allElem[NDFD_PXTORN]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PXTORN],match[allElem[NDFD_PXTORN]].f_sector);
   }   
   if ((allElem[NDFD_PXHAIL] != -1) &&
       (match[allElem[NDFD_PXHAIL]].value[pntIndex].valueType != 2))
   {
      printf("pxhail:%.0f ", match[allElem[NDFD_PXHAIL]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PXHAIL],match[allElem[NDFD_PXHAIL]].f_sector);
   }  
   if ((allElem[NDFD_PXTSTMWIND] != -1) &&
       (match[allElem[NDFD_PXTSTMWIND]].value[pntIndex].valueType != 2))
   {
      printf("pxtstmwind:%.0f ", match[allElem[NDFD_PXTSTMWIND]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PXTSTMWIND],match[allElem[NDFD_PXTSTMWIND]].f_sector);
   }  
   if ((allElem[NDFD_PSTORM] != -1) &&
       (match[allElem[NDFD_PSTORM]].value[pntIndex].valueType != 2))
   {
      printf("pstorm:%.0f ", match[allElem[NDFD_PSTORM]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PSTORM],match[allElem[NDFD_PSTORM]].f_sector);
   }  
   if ((allElem[NDFD_PXSTORM] != -1) &&
       (match[allElem[NDFD_PXSTORM]].value[pntIndex].valueType != 2))
   {
      printf("pxstorm:%.0f ", match[allElem[NDFD_PXSTORM]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PXSTORM],match[allElem[NDFD_PXSTORM]].f_sector);
   }  
   if ((allElem[NDFD_WD] != -1) &&
       (match[allElem[NDFD_WD]].value[pntIndex].valueType != 2))
   {
      printf("wd:%.0f ", match[allElem[NDFD_WD]].value[pntIndex].data);
/*      printf ("wd:%3s ",
              DegToCompass (match[allElem[NDFD_WD]].value[pntIndex].data));
*/
   }
   if ((allElem[NDFD_WH] != -1) &&
       (match[allElem[NDFD_WH]].value[pntIndex].valueType != 2))
   {
       printf("wh:%.0f ", match[allElem[NDFD_WH]].value[pntIndex].data);
       printf ("match[%d].f_sector = %d\n",allElem[NDFD_WH],match[allElem[NDFD_WH]].f_sector);
   }
   if ((allElem[NDFD_WX] != -1) &&
       (match[allElem[NDFD_WX]].value[pntIndex].valueType != 2))
   {
      printf("wx \n\t%s ", match[allElem[NDFD_WX]].value[pntIndex].str);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_WX],match[allElem[NDFD_WX]].f_sector);
   }
   printf("\n");
}
