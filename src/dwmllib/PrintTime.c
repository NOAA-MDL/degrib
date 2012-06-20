/* Print Routine #1 for diagnostics for XMLParse(). */
#include "xmlparse.h"
void PrintTime(genMatchType * match, size_t pntIndex, int *allElem, 
               sChar pntTimeZone, sChar f_dayCheck)
{
   char timeBuff[30];
   char zone[7];
   size_t i;
   double localTime;

   for (i = 0; i < NDFD_MATCHALL; i++)
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
   if ((allElem[NDFD_TMPABV14D] != -1) &&
       (match[allElem[NDFD_TMPABV14D]].value[pntIndex].valueType != 2))
   {
      printf("tmpabv14d:%.0f ", match[allElem[NDFD_TMPABV14D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_TMPABV14D],match[allElem[NDFD_TMPABV14D]].f_sector);
   }
   if ((allElem[NDFD_TMPBLW14D] != -1) &&
       (match[allElem[NDFD_TMPBLW14D]].value[pntIndex].valueType != 2))
   {
      printf("tmpblw14d:%.0f ", match[allElem[NDFD_TMPBLW14D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_TMPBLW14D],match[allElem[NDFD_TMPBLW14D]].f_sector);
   }
   if ((allElem[NDFD_PRCPABV14D] != -1) &&
       (match[allElem[NDFD_PRCPABV14D]].value[pntIndex].valueType != 2))
   {
      printf("prcpabv14d:%.0f ", match[allElem[NDFD_PRCPABV14D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PRCPABV14D],match[allElem[NDFD_PRCPABV14D]].f_sector);
   }
   if ((allElem[NDFD_PRCPBLW14D] != -1) &&
       (match[allElem[NDFD_PRCPBLW14D]].value[pntIndex].valueType != 2))
   {
      printf("prcpblw14d:%.0f ", match[allElem[NDFD_PRCPBLW14D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PRCPBLW14D],match[allElem[NDFD_PRCPBLW14D]].f_sector);
   }
   if ((allElem[NDFD_TMPABV30D] != -1) &&
       (match[allElem[NDFD_TMPABV30D]].value[pntIndex].valueType != 2))
   {
      printf("tmpabv30d:%.0f ", match[allElem[NDFD_TMPABV30D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_TMPABV30D],match[allElem[NDFD_TMPABV30D]].f_sector);
   }
   if ((allElem[NDFD_TMPBLW30D] != -1) &&
       (match[allElem[NDFD_TMPBLW30D]].value[pntIndex].valueType != 2))
   {
      printf("tmpblw30d:%.0f ", match[allElem[NDFD_TMPBLW30D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_TMPBLW30D],match[allElem[NDFD_TMPBLW30D]].f_sector);
   }
   if ((allElem[NDFD_PRCPABV30D] != -1) &&
       (match[allElem[NDFD_PRCPABV30D]].value[pntIndex].valueType != 2))
   {
      printf("prcpabv30d:%.0f ", match[allElem[NDFD_PRCPABV30D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PRCPABV30D],match[allElem[NDFD_PRCPABV30D]].f_sector);
   }
   if ((allElem[NDFD_PRCPBLW30D] != -1) &&
       (match[allElem[NDFD_PRCPBLW30D]].value[pntIndex].valueType != 2))
   {
      printf("prcpblw30d:%.0f ", match[allElem[NDFD_PRCPBLW30D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PRCPBLW30D],match[allElem[NDFD_PRCPBLW30D]].f_sector);
   }
   if ((allElem[NDFD_TMPABV90D] != -1) &&
       (match[allElem[NDFD_TMPABV90D]].value[pntIndex].valueType != 2))
   {
      printf("tmpabv90d:%.0f ", match[allElem[NDFD_TMPABV90D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_TMPABV90D],match[allElem[NDFD_TMPABV90D]].f_sector);
   }
   if ((allElem[NDFD_TMPBLW90D] != -1) &&
       (match[allElem[NDFD_TMPBLW90D]].value[pntIndex].valueType != 2))
   {
      printf("tmpblw90d:%.0f ", match[allElem[NDFD_TMPBLW90D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_TMPBLW90D],match[allElem[NDFD_TMPBLW90D]].f_sector);
   }
   if ((allElem[NDFD_PRCPABV90D] != -1) &&
       (match[allElem[NDFD_PRCPABV90D]].value[pntIndex].valueType != 2))
   {
      printf("prcpabv90d:%.0f ", match[allElem[NDFD_PRCPABV90D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PRCPABV90D],match[allElem[NDFD_PRCPABV90D]].f_sector);
   }
   if ((allElem[NDFD_PRCPBLW90D] != -1) &&
       (match[allElem[NDFD_PRCPBLW90D]].value[pntIndex].valueType != 2))
   {
      printf("prcpblw90d:%.0f ", match[allElem[NDFD_PRCPBLW90D]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_PRCPBLW90D],match[allElem[NDFD_PRCPBLW90D]].f_sector);
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

   /* RTMA Elements. */
   if ((allElem[RTMA_TEMP] != -1) &&
       (match[allElem[RTMA_TEMP]].value[pntIndex].valueType != 2))
   {
      printf("rtma_temp:%.0f ", match[allElem[RTMA_TEMP]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_TEMP],match[allElem[RTMA_TEMP]].f_sector);
   }
   if ((allElem[RTMA_UTEMP] != -1) &&
       (match[allElem[RTMA_UTEMP]].value[pntIndex].valueType != 2))
   {
      printf("rtma_utemp:%.0f ", match[allElem[RTMA_UTEMP]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_UTEMP],match[allElem[RTMA_UTEMP]].f_sector);
   }
   if ((allElem[RTMA_TD] != -1) &&
       (match[allElem[RTMA_TD]].value[pntIndex].valueType != 2))
   {
      printf("rtma_td:%.0f ", match[allElem[RTMA_TD]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_TD],match[allElem[RTMA_TD]].f_sector);
   }
   if ((allElem[RTMA_UTD] != -1) &&
       (match[allElem[RTMA_UTD]].value[pntIndex].valueType != 2))
   {
      printf("rtma_utd:%.0f ", match[allElem[RTMA_UTD]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_UTD],match[allElem[RTMA_UTD]].f_sector);
   }
   if ((allElem[RTMA_WDIR] != -1) &&
       (match[allElem[RTMA_WDIR]].value[pntIndex].valueType != 2))
   {
      printf("rtma_wdir:%.0f ", match[allElem[RTMA_WDIR]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_WDIR],match[allElem[RTMA_WDIR]].f_sector);
   }
   if ((allElem[RTMA_UWDIR] != -1) &&
       (match[allElem[RTMA_UWDIR]].value[pntIndex].valueType != 2))
   {
      printf("rtma_uwdir:%.0f ", match[allElem[RTMA_UWDIR]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_UWDIR],match[allElem[RTMA_UWDIR]].f_sector);
   }
   if ((allElem[RTMA_WSPD] != -1) &&
       (match[allElem[RTMA_WSPD]].value[pntIndex].valueType != 2))
   {
      printf("rtma_wspd:%.0f ", match[allElem[RTMA_WSPD]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_WSPD],match[allElem[RTMA_WSPD]].f_sector);
   }
   if ((allElem[RTMA_UWSPD] != -1) &&
       (match[allElem[RTMA_UWSPD]].value[pntIndex].valueType != 2))
   {
      printf("rtma_uwspd:%.0f ", match[allElem[RTMA_UWSPD]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_UWSPD],match[allElem[RTMA_UWSPD]].f_sector);
   }   
   if ((allElem[RTMA_SKY] != -1) &&
       (match[allElem[RTMA_SKY]].value[pntIndex].valueType != 2))
   {
      printf("rtma_sky:%.0f ", match[allElem[RTMA_SKY]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_SKY],match[allElem[RTMA_SKY]].f_sector);
   }
   if ((allElem[RTMA_PRECIPA] != -1) &&
       (match[allElem[RTMA_PRECIPA]].value[pntIndex].valueType != 2))
   {
      printf("rtma_precipa:%.0f ", match[allElem[RTMA_PRECIPA]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[RTMA_PRECIPA],match[allElem[RTMA_PRECIPA]].f_sector);
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
   if ((allElem[NDFD_ICEACC] != -1) &&
       (match[allElem[NDFD_ICEACC]].value[pntIndex].valueType != 2))
      {
         printf("icea:%2.3f ", match[allElem[NDFD_ICEACC]].value[pntIndex].data);
         printf ("match[%d].f_sector = %d\n",allElem[NDFD_ICEACC],match[allElem[NDFD_ICEACC]].f_sector);
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
   if ((allElem[NDFD_FWXWINDRH] != -1) &&
       (match[allElem[NDFD_FWXWINDRH]].value[pntIndex].valueType != 2))
   {
      printf("critfireo:%.0f ", match[allElem[NDFD_FWXWINDRH]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_FWXWINDRH],match[allElem[NDFD_FWXWINDRH]].f_sector);
   } 
   if ((allElem[NDFD_FWXTSTORM] != -1) &&
       (match[allElem[NDFD_FWXTSTORM]].value[pntIndex].valueType != 2))
   {
      printf("dryfireo:%.0f ", match[allElem[NDFD_FWXTSTORM]].value[pntIndex].data);
      printf ("match[%d].f_sector = %d\n",allElem[NDFD_FWXTSTORM],match[allElem[NDFD_FWXTSTORM]].f_sector);
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
   if ((allElem[NDFD_WWA] != -1) &&
       (match[allElem[NDFD_WWA]].value[pntIndex].valueType != 2))
   {
       printf("hazard: \t%s ", match[allElem[NDFD_WWA]].value[pntIndex].str);
       printf ("match[%d].f_sector = %d\n",allElem[NDFD_WWA],match[allElem[NDFD_WWA]].f_sector);
   }
   if ((allElem[LAMP_TSTMPRB] != -1) &&
       (match[allElem[LAMP_TSTMPRB]].value[pntIndex].valueType != 2))
   {
       printf("LAMP Tstrm Prb:%.0f ", match[allElem[LAMP_TSTMPRB]].value[pntIndex].data);
       printf ("match[%d].f_sector = %d\n",allElem[LAMP_TSTMPRB],match[allElem[LAMP_TSTMPRB]].f_sector);
   }
   if ((allElem[NDFD_MAXRH] != -1) &&
       (match[allElem[NDFD_MAXRH]].value[pntIndex].valueType != 2))
   {
       printf("MAX RH:%.0f ", match[allElem[NDFD_MAXRH]].value[pntIndex].data);
       printf ("match[%d].f_sector = %d\n",allElem[NDFD_MAXRH],match[allElem[NDFD_MAXRH]].f_sector);
   }
   if ((allElem[NDFD_MINRH] != -1) &&
       (match[allElem[NDFD_MINRH]].value[pntIndex].valueType != 2))
   {
       printf("MIN RH:%.0f ", match[allElem[NDFD_MINRH]].value[pntIndex].data);
       printf ("match[%d].f_sector = %d\n",allElem[NDFD_MINRH],match[allElem[NDFD_MINRH]].f_sector);
   }

   printf("\n");
}
