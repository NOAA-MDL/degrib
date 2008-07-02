/*****************************************************************************
 * prepareDWMLgen() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Flags those elements that are ultimately formatted in the output XML for 
 *  the DWMLgen products. The user chooses the elements for the "time-series" 
 *  product. There are pre-defined elements formatted for the "glance" product.
 *  Routine also sets the flag determining if period names are used in the time
 *  layout generation.
 *
 * ARGUMENTS
 *           f_XML = Flag denoting type of XML product. 
 *                   1 = DWMLgen's "time-series" product,
 *                   2 = DWMLgen's "glance" product, 
 *                   3 = DWMLgenByDay's "12 hourly" product, 
 *                   4 = DWMLgenByDay's "24 hourly" product, 
 *                   5 = DWMLgen's "RTMA time-series" product,
 *                   6 = DWMLgen's mix of "RTMA & NDFD time-series" product. 
 *                   (Input) 
 *     wxParameters = Array containing the flags denoting whether a certain 
 *                    element is formatted in the output XML (= 1), or used in 
 *                    deriving another formatted element (= 2). (Input/Output) 
 *    summarization = The type of temporal summarization being used.
 *                    Currently, no summarization is done in time.
 * f_formatPeriodName = Flag to indicate if period names (i.e. "today") appear 
 *                      in the start valid time tag: 
 *                      <start-valid-time period-name="today"> (Input)
 *        varFilter = Array denotes what NDFD variables are of interest (set to
 *                    = 1) or vital (set to = 2) to this call. If the variable 
 *                    is set to 2, then the variable wasn't set on the command 
 *                    line as an argument, but the call needs this element to 
 *                    derive others. (Input)
 *          numPnts = Total number of points being processed. (Input)
 *           f_icon = Flag denoting if icons is to be formatted for this point.
 *                    (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *  9/2007 Paul Hershberg (MDL): Added 12 Climate Outlook Elements
 * 12/2007 Paul Hershberg (MDL): Added 10 RTMA Elements
 * 5/2008 Paul Hershberg (MDL): Added Hazard Element
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void prepareDWMLgen(uChar f_XML, uChar * f_formatPeriodName, 
                    uChar **wxParameters, char *summarization,
                    uChar varFilter[NDFD_MATCHALL + 1], sChar *f_icon, 
                    size_t numPnts)
{
   int j; /* Counter thru points. */
   strcpy(summarization, "none"); /* There is no hourly summariztion for the
                                   * DWMLgen products. */

   for (j = 0; j< numPnts; j++)
   {
      /* Flag those elements in the "time-series" product to be formatted in the 
       * output XML. 
       */
      if (f_XML == 1 || f_XML == 6)
      {
         if (varFilter[NDFD_MAX] == 2)
            wxParameters[j][NDFD_MAX] = 1;
         if (varFilter[NDFD_MIN] == 2)
            wxParameters[j][NDFD_MIN] = 1;
         if (varFilter[NDFD_POP] == 2)
            wxParameters[j][NDFD_POP] = 1;
         if (varFilter[NDFD_TEMP] == 2)
            wxParameters[j][NDFD_TEMP] = 1;
         if (varFilter[NDFD_WD] == 2)
            wxParameters[j][NDFD_WD] = 1;
         if (varFilter[NDFD_WS] == 2)
            wxParameters[j][NDFD_WS] = 1;
         if (varFilter[NDFD_TD] == 2)
            wxParameters[j][NDFD_TD] = 1;
         if (varFilter[NDFD_SKY] == 2)
            wxParameters[j][NDFD_SKY] = 1;
         if (varFilter[NDFD_QPF] == 2)
            wxParameters[j][NDFD_QPF] = 1;
         if (varFilter[NDFD_SNOW] == 2)
            wxParameters[j][NDFD_SNOW] = 1;
         if (varFilter[NDFD_WX] == 2)
            wxParameters[j][NDFD_WX] = 1;
         if (varFilter[NDFD_WH] == 2)
            wxParameters[j][NDFD_WH] = 1;
         if (varFilter[NDFD_AT] == 2)
            wxParameters[j][NDFD_AT] = 1;
         if (varFilter[NDFD_RH] == 2)
            wxParameters[j][NDFD_RH] = 1;
         if (varFilter[NDFD_WG] == 2)
            wxParameters[j][NDFD_WG] = 1;
         if (varFilter[NDFD_INC34] == 2)
            wxParameters[j][NDFD_INC34] = 1;
         if (varFilter[NDFD_INC50] == 2)
            wxParameters[j][NDFD_INC50] = 1;
         if (varFilter[NDFD_INC64] == 2)
            wxParameters[j][NDFD_INC64] = 1;
         if (varFilter[NDFD_CUM34] == 2)
            wxParameters[j][NDFD_CUM34] = 1;
         if (varFilter[NDFD_CUM50] == 2)
            wxParameters[j][NDFD_CUM50] = 1;
         if (varFilter[NDFD_CUM64] == 2)
            wxParameters[j][NDFD_CUM64] = 1;
         if (varFilter[NDFD_CONHAZ] == 2)
            wxParameters[j][NDFD_CONHAZ] = 1;
         if (varFilter[NDFD_PTORN] == 2)
            wxParameters[j][NDFD_PTORN] = 1;
         if (varFilter[NDFD_PHAIL] == 2)
            wxParameters[j][NDFD_PHAIL] = 1;
         if (varFilter[NDFD_PTSTMWIND] == 2)
            wxParameters[j][NDFD_PTSTMWIND] = 1;
         if (varFilter[NDFD_PXTORN] == 2)
            wxParameters[j][NDFD_PXTORN] = 1;
         if (varFilter[NDFD_PXHAIL] == 2)
            wxParameters[j][NDFD_PXHAIL] = 1;
         if (varFilter[NDFD_PXTSTMWIND] == 2)
            wxParameters[j][NDFD_PXTSTMWIND] = 1;
         if (varFilter[NDFD_PSTORM] == 2)
            wxParameters[j][NDFD_PSTORM] = 1;
         if (varFilter[NDFD_PXSTORM] == 2)
            wxParameters[j][NDFD_PXSTORM] = 1;
         if (varFilter[NDFD_TMPABV14D] == 2)
            wxParameters[j][NDFD_TMPABV14D] = 1;
         if (varFilter[NDFD_TMPBLW14D] == 2)
            wxParameters[j][NDFD_TMPBLW14D] = 1;
         if (varFilter[NDFD_TMPABV30D] == 2)
            wxParameters[j][NDFD_TMPABV30D] = 1;
         if (varFilter[NDFD_TMPBLW30D] == 2)
            wxParameters[j][NDFD_TMPBLW30D] = 1;
         if (varFilter[NDFD_TMPABV90D] == 2)
            wxParameters[j][NDFD_TMPABV90D] = 1;
         if (varFilter[NDFD_TMPBLW90D] == 2)
            wxParameters[j][NDFD_TMPBLW90D] = 1;
         if (varFilter[NDFD_PRCPABV14D] == 2)
            wxParameters[j][NDFD_PRCPABV14D] = 1;
         if (varFilter[NDFD_PRCPBLW14D] == 2)
            wxParameters[j][NDFD_PRCPBLW14D] = 1;
         if (varFilter[NDFD_PRCPABV30D] == 2)
            wxParameters[j][NDFD_PRCPABV30D] = 1;
         if (varFilter[NDFD_PRCPBLW30D] == 2)
            wxParameters[j][NDFD_PRCPBLW30D] = 1;
         if (varFilter[NDFD_PRCPABV90D] == 2)
            wxParameters[j][NDFD_PRCPABV90D] = 1;
         if (varFilter[NDFD_PRCPBLW90D] == 2)
            wxParameters[j][NDFD_PRCPBLW90D] = 1;
         if (varFilter[NDFD_WWA] == 2)
            wxParameters[j][NDFD_WWA] = 1;

         /* We need to create a time layout for the icons in the case that only
          * icons is to be formatted. When this occurs, make Icons use the
          * weather's time layout. Set the wxParameters flag for WX to = 3. 
          */
         if (*f_icon == 1)
         {
            if (wxParameters[j][NDFD_WX] != 1)
               wxParameters[j][NDFD_WX] = 3;
         }
      }

      /* For DWMLgen's "glance" product, there are six pre-defined set of NDFD
       * parameters to be formatted. Five of these are maxt, mint, sky, hazard, 
       * and wx. Two other elements not formatted (temp and wind speed) are used
       * to derive the 5th element formatted: Icons. 
       */
      if (f_XML == 2)
      {
         *f_formatPeriodName = 1;
         if (varFilter[NDFD_MAX] >= 2)
            wxParameters[j][NDFD_MAX] = 1;
         if (varFilter[NDFD_MAX] >= 2)
            wxParameters[j][NDFD_MIN] = 1;
         if (varFilter[NDFD_MAX] >= 2)
            wxParameters[j][NDFD_SKY] = 1;
         if (varFilter[NDFD_MAX] >= 2)
            wxParameters[j][NDFD_WX] = 1;
         if (varFilter[NDFD_WWA] >= 2)
            wxParameters[j][NDFD_WWA] = 1;
      }

      /* RTMA elements. */
      if (f_XML == 5 || f_XML == 6)
      {
         if (varFilter[RTMA_PRECIPA] == 2)
            wxParameters[j][RTMA_PRECIPA] = 1;
         if (varFilter[RTMA_SKY] == 2)
            wxParameters[j][RTMA_SKY] = 1;
         if (varFilter[RTMA_TD] == 2)
            wxParameters[j][RTMA_TD] = 1;
         if (varFilter[RTMA_TEMP] == 2)
            wxParameters[j][RTMA_TEMP] = 1;
         if (varFilter[RTMA_UTD] == 2)
            wxParameters[j][RTMA_UTD] = 1;
         if (varFilter[RTMA_UTEMP] == 2)
            wxParameters[j][RTMA_UTEMP] = 1;
         if (varFilter[RTMA_UWDIR] == 2)
            wxParameters[j][RTMA_UWDIR] = 1;
         if (varFilter[RTMA_UWSPD] == 2)
            wxParameters[j][RTMA_UWSPD] = 1;
         if (varFilter[RTMA_WDIR] == 2)
            wxParameters[j][RTMA_WDIR] = 1;
         if (varFilter[RTMA_WSPD] == 2)
            wxParameters[j][RTMA_WSPD] = 1;
      }

      /* Turn on the concatenated elements, if warranted. */
      if  (f_XML == 6)
      {
         if (wxParameters[j][NDFD_TEMP]==1 && wxParameters[j][RTMA_TEMP]==1)
            wxParameters[j][RTMA_NDFD_TEMP] = 1;
         if (wxParameters[j][NDFD_TD] == 1 && wxParameters[j][RTMA_TD] == 1)
            wxParameters[j][RTMA_NDFD_TD] = 1;
         if (wxParameters[j][NDFD_WS]==1 && wxParameters[j][RTMA_WSPD]==1)
            wxParameters[j][RTMA_NDFD_WSPD] = 1;
         if (wxParameters[j][NDFD_WD]==1 && wxParameters[j][RTMA_WDIR]==1)
            wxParameters[j][RTMA_NDFD_WDIR] = 1;
         if (wxParameters[j][NDFD_QPF]==1 && wxParameters[j][RTMA_PRECIPA]==1)
            wxParameters[j][RTMA_NDFD_PRECIPA] = 1;
         if (wxParameters[j][NDFD_SKY]==1 && wxParameters[j][RTMA_SKY]==1)
            wxParameters[j][RTMA_NDFD_SKY] = 1;
      }
   }

   return;
}
