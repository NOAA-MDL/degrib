/*****************************************************************************
 * setVarFilter() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Fill the variable filtered array, varFilter, to show what NDFD variables
 *  are of interest (set to = 1) or vital (set to = 2) to this call. If the 
 *  variable is set to 2, then the variable wasn't set on the command line as
 *  an argument, but this call needs this element to derive other elements.
 *   
 * ARGUMENTS
 *       f_XML = flag for 1 of the 4 DWML products (Input):
 *               1 = DWMLgen's "time-series" product. 
 *               2 = DWMLgen's "glance" product.
 *               3 = DWMLgenByDay's "12 hourly" format product.
 *               4 = DWMLgenByDay's "24 hourly" format product. (Input)
 *               5 = DWMLgen's RTMA "time-series" product.
 *               6 = DWMLgen's mix of "RTMA & NDFD time-series" product. 
 *      f_icon = Flag denoting whether icons are to be derived and formatted.
 *               If this flag is chosen, the other 4 elements' data used to 
 *               derive the icons must be retrieved/derived too (WS, SKY, 
 *               TEMP, WX). (Input)
 *   varFilter = Array denotes what NDFD variables are of interest (set to = 1)
 *               or vital (set to = 2) to this call. If the variable is set to 
 *               2, then the variable wasn't set on the command line as an 
 *               argument, but the call needs this element to derive others. 
 *               (Output)
 * numNdfdVars = Number of ndfd elements chosen on the command line arg to
 *               format. (Input)
 *    ndfdVars = Array holding the enum numbers of the ndfd elements chosen on 
 *               the command line arg to format. (Input)
 *        
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  3/2007 Paul Hershberg (MDL): Created.
 *  8/2007 Paul Hershberg (MDL): Added 12 NDFD Climate Outlook Elements.
 * 11/2007 Paul Hershberg (MDL): Added 10 RTMA Elements.
 *  8/2007 Paul Hershberg (MDL): Added 6 RTMA+NDFD Concatenated Elements.
 *  6/2008 Paul Hershberg (MDL): Added Hazard Element.
 *  8/2008 Paul Hershberg (MDL): Added Hazard Element for summary products.
 *  8/2009 Paul Hershberg (MDL): Added Lamp Tstm element.
 *  8/2011 Paul Hershberg (MDL): Added 2 Fire Wx Elements.
 * 10/2011 Paul Hershberg (MDL): Added Ice Accumulation element. 
 *  3/2012 Paul Hershberg (MDL): Added MAXRH and MINRH elements.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void setVarFilter(sChar f_XML, sChar *f_icon, size_t numNdfdVars, 
                  const uChar *ndfdVars, uChar varFilter[NDFD_MATCHALL+1])
{
   int i;
   memset(varFilter, 0, NDFD_MATCHALL + 1);

   /* Initialize all elements to 1. */

   /* NDFD Forecast elements. */
   varFilter[NDFD_MAX] = 1;
   varFilter[NDFD_MIN] = 1;
   varFilter[NDFD_POP] = 1;
   varFilter[NDFD_TEMP] = 1;
   varFilter[NDFD_WD] = 1;
   varFilter[NDFD_WS] = 1;
   varFilter[NDFD_TD] = 1;
   varFilter[NDFD_SKY] = 1;
   varFilter[NDFD_QPF] = 1;
   varFilter[NDFD_SNOW] = 1;
   varFilter[NDFD_WX] = 1;
   varFilter[NDFD_WH] = 1;
   varFilter[NDFD_AT] = 1;
   varFilter[NDFD_RH] = 1;
   varFilter[NDFD_WG] = 1;
   varFilter[NDFD_WWA] = 1;
   varFilter[NDFD_MAXRH] = 1;
   varFilter[NDFD_MINRH] = 1;

   /* 6 Tropical Wind Threshold elements. */
   varFilter[NDFD_INC34] = 1;
   varFilter[NDFD_INC50] = 1;
   varFilter[NDFD_INC64] = 1;
   varFilter[NDFD_CUM34] = 1;
   varFilter[NDFD_CUM50] = 1;
   varFilter[NDFD_CUM64] = 1;

   /* 2 Fire Wx Elements. */
   varFilter[NDFD_FWXWINDRH] = 1;
   varFilter[NDFD_FWXTSTORM] = 1;

   /* 9 Surface Prediction Center elements. */
   varFilter[NDFD_CONHAZ] = 1;
   varFilter[NDFD_PTORN] = 1;
   varFilter[NDFD_PHAIL] = 1;
   varFilter[NDFD_PTSTMWIND] = 1;
   varFilter[NDFD_PXTORN] = 1;
   varFilter[NDFD_PXHAIL] = 1;
   varFilter[NDFD_PXTSTMWIND] = 1;
   varFilter[NDFD_PSTORM] = 1;
   varFilter[NDFD_PXSTORM] = 1;

   /* 12 Climate Outlook Elements. */
   varFilter[NDFD_TMPABV14D] = 1;
   varFilter[NDFD_TMPBLW14D] = 1;
   varFilter[NDFD_PRCPABV14D] = 1;
   varFilter[NDFD_PRCPBLW14D] = 1;
   varFilter[NDFD_TMPABV30D] = 1;
   varFilter[NDFD_TMPBLW30D] = 1;
   varFilter[NDFD_PRCPABV30D] = 1;
   varFilter[NDFD_PRCPBLW30D] = 1;
   varFilter[NDFD_TMPABV90D] = 1;
   varFilter[NDFD_TMPBLW90D] = 1;
   varFilter[NDFD_PRCPABV90D] = 1;
   varFilter[NDFD_PRCPBLW90D] = 1;

   /* 1 Ice Accumulation Element. */
   varFilter[NDFD_ICEACC] = 1;

   /* 1 LAMP Element. */
   varFilter[LAMP_TSTMPRB] = 1;

   /* 10 RTMA Elements. */
   varFilter[RTMA_PRECIPA] = 1;
   varFilter[RTMA_SKY] = 1;
   varFilter[RTMA_TD] = 1;
   varFilter[RTMA_TEMP] = 1;
   varFilter[RTMA_UTD] = 1;
   varFilter[RTMA_UTEMP] = 1;
   varFilter[RTMA_UWDIR] = 1;
   varFilter[RTMA_UWSPD] = 1;
   varFilter[RTMA_WDIR] = 1;
   varFilter[RTMA_WSPD] = 1;

   /* Force genprobe() to return required NDFD element(s). */
   if (f_XML == 2)
   {
      varFilter[NDFD_MAX] = 2;
      varFilter[NDFD_MIN] = 2;
      varFilter[NDFD_TEMP] = 2;
      varFilter[NDFD_WS] = 2;
      varFilter[NDFD_SKY] = 2;
      varFilter[NDFD_WX] = 2;
      varFilter[NDFD_POP] = 2;
      varFilter[NDFD_WWA] = 2;
   }
   else if ((f_XML == 1 || f_XML == 6 ) && *f_icon == 1) 
   {
      varFilter[NDFD_TEMP] = 2;
      varFilter[NDFD_WS] = 2;
      varFilter[NDFD_SKY] = 2;
      varFilter[NDFD_WX] = 2;
      varFilter[NDFD_POP] = 2;
   }
   else if (f_XML == 3 || f_XML == 4)
   {
      varFilter[NDFD_MAX] = 2;
      varFilter[NDFD_MIN] = 2;
      varFilter[NDFD_POP] = 2;
      varFilter[NDFD_WD] = 2;
      varFilter[NDFD_WS] = 2;
      varFilter[NDFD_WG] = 2;
      varFilter[NDFD_SKY] = 2;
      varFilter[NDFD_WX] = 2;
      varFilter[NDFD_WWA] = 2;
   }

   /* Force genprobe() to return required RTMA uncertainty element(s), if 
    * applicable.
    */
   if (f_XML == 5 || f_XML == 6)
   {
      for (i = 0; i < numNdfdVars; i++)
      {
         if (ndfdVars[i] == RTMA_TEMP)
         {
            varFilter[RTMA_UTEMP] = 2;
            continue;
         }
         if (ndfdVars[i] == RTMA_TD)
         {
            varFilter[RTMA_UTD] = 2;
            continue;
         }
         if (ndfdVars[i] == RTMA_WDIR)
         {
            varFilter[RTMA_UWDIR] = 2;
            continue;
         }
         if (ndfdVars[i] == RTMA_WSPD)
         {
            varFilter[RTMA_UWSPD] = 2;
            continue;
         }
      }
   }

   return;
}
