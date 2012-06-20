/*****************************************************************************
 * determinePeriodLength() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Determines the period length of an element in hours. Time between the 
 *  first validTime and the second validTime is used in the determination.
 *  If element is a Climate Outlook product, we'll assume there is no 
 *  secondValidTime. Calculations must occur to retrieve the number of hours
 *  to subtract from the firstValidtime (which is the first endTime) to 
 *  retrieve the associated startTime.
 *
 * ARGUMENTS
 *   firstValidTime = First valid time of element. (Input)
 *  secondValidTime = Second valid time of element. (Input)
 *         ndfdEnum = Number denoting the NDFD element currently processed. 
 *                    (Input) 
 *          numRows = Total number of data rows (values) for element. (Input)
 *           period = Length between an elements successive validTimes. (Output)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: int period
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
int determinePeriodLength(double firstValidTime, double secondValidTime,
                          uChar numRows, uChar ndfdEnum)
{
   int period = 0;            /* Length between an elements successive
                               * validTimes. */

   /* If there is just one row of data, guess the period length. Also, determine
    * climate product's period lengths here.
    */
   if (numRows == 1 || secondValidTime == 0.0 || ndfdEnum == NDFD_TMPABV14D 
       || ndfdEnum == NDFD_TMPBLW14D || ndfdEnum == NDFD_PRCPABV14D || 
       ndfdEnum == NDFD_PRCPBLW14D || ndfdEnum == NDFD_TMPABV30D || 
       ndfdEnum == NDFD_TMPBLW30D || ndfdEnum == NDFD_PRCPABV30D || 
       ndfdEnum == NDFD_PRCPBLW30D || ndfdEnum == NDFD_TMPABV90D || 
       ndfdEnum == NDFD_TMPBLW90D || ndfdEnum == NDFD_PRCPABV90D || 
       ndfdEnum == NDFD_PRCPBLW90D)
   {
      if (ndfdEnum == NDFD_MAX || ndfdEnum == NDFD_MIN ||
          ndfdEnum == NDFD_CONHAZ || ndfdEnum == NDFD_PTORN ||
          ndfdEnum == NDFD_PHAIL || ndfdEnum == NDFD_PTSTMWIND ||
          ndfdEnum == NDFD_PXTORN || ndfdEnum == NDFD_PXHAIL ||
          ndfdEnum == NDFD_PXTSTMWIND || ndfdEnum == NDFD_PSTORM ||
          ndfdEnum == NDFD_PXSTORM || ndfdEnum == NDFD_FWXWINDRH || 
          ndfdEnum == NDFD_FWXTSTORM || ndfdEnum == NDFD_MAXRH ||
          ndfdEnum == NDFD_MINRH)

          period = 24;

      else if (ndfdEnum == NDFD_POP || ndfdEnum == NDFD_WH)
         period = 12;

      else if (ndfdEnum == NDFD_QPF || ndfdEnum == NDFD_SNOW ||
               ndfdEnum == NDFD_INC34 || ndfdEnum == NDFD_INC50 ||
               ndfdEnum == NDFD_INC64 || ndfdEnum == NDFD_CUM34 ||
               ndfdEnum == NDFD_CUM50 || ndfdEnum == NDFD_CUM64 ||
               ndfdEnum == NDFD_ICEACC)
         period = 6;

      else if (ndfdEnum == LAMP_TSTMPRB)
         period = 2;

      else if (ndfdEnum == NDFD_TMPABV14D || ndfdEnum == NDFD_TMPBLW14D ||
               ndfdEnum == NDFD_PRCPABV14D || ndfdEnum == NDFD_PRCPBLW14D)

         /* Period for 8-14 Day Climate Outlook Products. */
         period = 6*24;

      else if (ndfdEnum == NDFD_TMPABV30D || ndfdEnum == NDFD_TMPBLW30D ||
               ndfdEnum == NDFD_PRCPABV30D || ndfdEnum == NDFD_PRCPBLW30D)
         /* Period for 1-Month Climate Outlook Products. Choose the number of 
          * hours in 30 days. The exact number of hours (in 28 to 31 days) is
          * not needed here, since we will output the period in months in the
          * layout key.
          */
         period = 30*24;

      else if (ndfdEnum == NDFD_TMPABV90D || ndfdEnum == NDFD_TMPBLW90D ||
               ndfdEnum == NDFD_PRCPABV90D || ndfdEnum == NDFD_PRCPBLW90D)
         /* Period for 3-Month Climate Outlook Products. Choose the number of 
          * hours in 90 days. The exact number of hours is not needed here, 
          * since we will output the period in months in the layout key.
          */
         period = 90*24;
                 
      else
         period = 3;

      return period;
   }

   period = (secondValidTime - firstValidTime) / 3600;

   return period;
}
