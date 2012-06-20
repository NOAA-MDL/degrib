/*****************************************************************************
 * checkNeedForEndTime() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Determines whether or not an element uses an end time <end-valid-time> tag 
 *  in the formatted output XML.
 *  
 * ARGUMENTS
 *   parameterName = Number denoting the NDFD element currently processed. 
 *                   (Input) 
 *           f_XML = Flag for 1 of the 4 DWML products:
 *                     1 = DWMLgen's NDFD "time-series" product. 
 *                     2 = DWMLgen's "glance" product.
 *                     3 = DWMLgenByDay's "12 hourly" format product.
 *                     4 = DWMLgenByDay's "24 hourly" format product. (Input)
 *                     5 = DWMLgen's RTMA "time-series" product. 
 *                     6 = DWMLgen's RTMA & NDFD "time-series" product. 
 *                     (Input) 
 *  
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *  2/2008 Paul Hershberg (MDL): Added f_XML argument.
 * 10/2011 Paul Hershberg (MDL): Added Ice Accumulation element.
 *  3/2012 Paul Hershberg (MDL): Added MaxRH and MinRH elements.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
int checkNeedForEndTime(uChar parameterName, uChar f_XML)
{

   /* Determine which NDFD parameter we are processing and return TRUE if it
    * is valid for a period of time. 
    */
   switch (parameterName)
   {

      case NDFD_MAX:
      case NDFD_MIN:
      case NDFD_POP:
      case NDFD_SNOW:
      case NDFD_ICEACC:
      case NDFD_QPF:
      case NDFD_INC34:
      case NDFD_INC50:
      case NDFD_INC64:
      case NDFD_CUM34:
      case NDFD_CUM50:
      case NDFD_CUM64:
      case NDFD_FWXWINDRH:
      case NDFD_FWXTSTORM:
      case NDFD_CONHAZ:
      case NDFD_PTORN:
      case NDFD_PHAIL:
      case NDFD_PTSTMWIND:
      case NDFD_PXTORN: 
      case NDFD_PXHAIL: 
      case NDFD_PXTSTMWIND:
      case NDFD_PSTORM: 
      case NDFD_PXSTORM:
      case NDFD_TMPABV14D:
      case NDFD_TMPBLW14D:
      case NDFD_PRCPABV14D:
      case NDFD_PRCPBLW14D:
      case NDFD_TMPABV30D:
      case NDFD_TMPBLW30D:
      case NDFD_PRCPABV30D:
      case NDFD_PRCPBLW30D:
      case NDFD_TMPABV90D:
      case NDFD_TMPBLW90D:
      case NDFD_PRCPABV90D:
      case NDFD_PRCPBLW90D:
      case LAMP_TSTMPRB:
      case NDFD_MAXRH:
      case NDFD_MINRH:

         return 1;
         break;

      case RTMA_PRECIPA:
         if (f_XML == 6) /* Only use endTimes if RTMA Precip Amt is concatenated
                          * to the NDFD QPF portion, for consistency sake. 
                          */
         {
            return 1;
            break;
         }
         else
            return 0; 

      default:

         /* Its a snapshot parameter so no <end-valid-time> tag. */
         return 0;
   }
}
