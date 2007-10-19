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
 *  
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
int checkNeedForEndTime(uChar parameterName)
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
      case NDFD_QPF:
      case NDFD_INC34:
      case NDFD_INC50:
      case NDFD_INC64:
      case NDFD_CUM34:
      case NDFD_CUM50:
      case NDFD_CUM64:
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

         return 1;
         break;

      default:

         /* Its a snapshot parameter so no <end-valid-time> tag. */
         return 0;
   }
}
