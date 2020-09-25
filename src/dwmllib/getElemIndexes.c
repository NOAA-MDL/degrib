/******************************************************************************

 * getElemIndexes() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  We need to see if certain elements were queried for or needed by the 
 *  summary products. If so, denote the index they occur in inside the
 *  elem array.
 *
 * ARGUMENTS
 *         ndfdMaxIndex = Index in elem array holding NDFD_MAXT. (Output)
 *         ndfdMinIndex = Index in elem array holding NDFD_MINT. (Output)
 *         ndfdPopIndex = Index in elem array holding NDFD_POP. (Output)
 *         ndfdWwaIndex = Index in elem array holding NDFD_WWA. (Output)
 *         ndfdTempIndex = Index in elem array holding NDFD_TEMP. (Output)
 *         ndfdTdIndex = Index in elem array holding NDFD_TD. (Output)
 *         ndfdQpfIndex = Index in elem array holding NDFD_QPF. (Output)
 *         ndfdSkyIndex = Index in elem array holding NDFD_SKY. (Output)
 *         ndfdWdirIndex = Index in elem array holding NDFD_WDIR. (Output)
 *         ndfdWspdIndex = Index in elem array holding NDFD_WSPD. (Output)
 *         ndfdWgustIndex = Index in elem array holding NDFD_WGUST. (Output)
 *         ndfdWgustOrWspdIndex = Index in elem array holding either Wgust if 
 *                                available, else Wspd. (Output)
 *         rtmaPrecipaIndex = Index in elem array holding RTMA_PRECIPA. (Output)
 *         rtmaSkyIndex = Index in elem array holding RTMA_SKY. (Output)
 *         rtmaTdIndex = Index in elem array holding RTMA_TD. (Output)
 *         rtmaTempIndex = Index in elem array holding RTMA_TEMP. (Output)
 *         rtmaWdirIndex = Index in elem array holding RTMA_WDIR. (Output)
 *         rtmaWspdIndex = Index in elem array holding RTMA_WSPD. (Output)
 *         rtmaNdfdSkyIndex = Index in elem array holding RTMA_NDFD_SKY. (Output)
 *         rtmaNdfdPrecipaIndex = Index in elem array holding RTMA_NDFD_PRECIPA. (Output)
 *         rtmaNdfdTdIndex = Index in elem array holding RTMA_NDFD_TD. (Output)
 *         rtmaNdfdTempIndex = Index in elem array holding RTMA_NDFD_TEMP. (Output)
 *         rtmaNdfdWdirIndex = Index in elem array holding RTMA_NDFD_WDIR. (Output)
 *         rtmaNdfdWSpdIndex = Index in elem array holding RTMA_NDFD_WSPD. (Output)
 *         f_XML = flag for 1 of the 6 DWML products:
 *               1 = DWMLgen's "time-series" product. 
 *               2 = DWMLgen's "glance" product.
 *               3 = DWMLgenByDay's "12 hourly" format product.
 *               4 = DWMLgenByDay's "24 hourly" format product.
 *               5 = DWMLgen's RTMA "time-series" product.
 *               6 = DWMLgen's RTMA + NDFD "time-series" product. (Input) 
 *         elem = Structure with info about the element.
 *         numElem = Number of elements returned by genProbe (those formatted 
 *                   plus those used in deriving formatted elements) (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  11/2009  Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getElemIndexes(int *ndfdMaxIndex, int *ndfdMinIndex, int *ndfdPopIndex, 
                    int *ndfdWwaIndex, int *ndfdTempIndex, int *ndfdTdIndex, 
                    int *ndfdQpfIndex, int *ndfdWspdIndex, int *ndfdWdirIndex, 
                    int *ndfdSkyIndex, int *ndfdWgustIndex, 
                    int *rtmaPrecipaIndex, int *rtmaSkyIndex, int *rtmaTdIndex,
                    int *rtmaTempIndex, int *rtmaWdirIndex, int *rtmaWspdIndex, 
                    int *rtmaNdfdSkyIndex, int *rtmaNdfdPrecipaIndex,
                    int *rtmaNdfdTdIndex, int *rtmaNdfdTempIndex,
                    int *rtmaNdfdWdirIndex, int *rtmaNdfdWspdIndex, uChar f_XML, 
                    size_t numElem, genElemDescript *elem)
{
   int m;

   /* For the concatenated and single RTMA products. Find in order of their 
    * enumeration. 
    */
   if (f_XML == 3 || f_XML == 4)
   {
      for (m = 0; m < numElem; m++)
      {
         if (elem[m].ndfdEnum == NDFD_MAX)
         {
            *ndfdMaxIndex = m;
            continue;
         } 
         if (elem[m].ndfdEnum == NDFD_MIN)
         {
            *ndfdMinIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_POP)
         {
            *ndfdPopIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_WWA)
         {
            *ndfdWwaIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_WS)
         {
            *ndfdWspdIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_WD)
         {
            *ndfdWdirIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_SKY)
         {
            *ndfdSkyIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_WG)
         {
            *ndfdWgustIndex = m; 
            continue;
         }
      } 
   }

   if (f_XML == 5 || f_XML == 6)
   {
      for (m = 0; m < numElem; m++)
      {
         if (elem[m].ndfdEnum == RTMA_PRECIPA)
         {
            *rtmaPrecipaIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_SKY)
         {
            *rtmaSkyIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_TD)
         {
            *rtmaTdIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_TEMP)
         {
            *rtmaTempIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_WDIR)
         {
            *rtmaWdirIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_WSPD)
         {
            *rtmaWspdIndex = m;
            continue;
        }
         if (elem[m].ndfdEnum == RTMA_NDFD_SKY)
         {
            *rtmaNdfdSkyIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_NDFD_PRECIPA)
         {
            *rtmaNdfdPrecipaIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_NDFD_TD)
         {
            *rtmaNdfdTdIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_NDFD_TEMP)
         {
            *rtmaNdfdTempIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_NDFD_WDIR)
         {
            *rtmaNdfdWdirIndex = m;
            continue;
         }
         if (elem[m].ndfdEnum == RTMA_NDFD_WSPD)
         {
            *rtmaNdfdWspdIndex = m;
            continue;
         }
      } 
   }
   if (f_XML == 1 || f_XML == 2 || f_XML == 5 || f_XML == 6)
   {
      for (m = 0; m < numElem; m++)
      {
         if (elem[m].ndfdEnum == NDFD_TEMP)
         {
            *ndfdTempIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_TD)
         {
            *ndfdTdIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_QPF)
         {
            *ndfdQpfIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_WS)
         {
            *ndfdWspdIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_WD)
         {
            *ndfdWdirIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_SKY)
         {
            *ndfdSkyIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_POP)
         {
            *ndfdPopIndex = m; 
            continue;
         }
         if (elem[m].ndfdEnum == NDFD_WG)
         {
            *ndfdWgustIndex = m; 
            continue;
         }
      }
   }

    return;
}
