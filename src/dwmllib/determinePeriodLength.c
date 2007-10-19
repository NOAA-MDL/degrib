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
 *    parameterName = Number denoting the NDFD element currently processed. 
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
                          uChar numRows, uChar parameterName)
{
   int period = 0;            /* Length between an elements successive
                               * validTimes. */

   /* If there is just one row of data, guess the period length. Also, determine
    * climate product's period lengths here.
    */
   if (numRows == 1 || secondValidTime == 0.0 || parameterName == NDFD_TMPABV14D 
       || parameterName == NDFD_TMPBLW14D || parameterName == NDFD_PRCPABV14D || 
       parameterName == NDFD_PRCPBLW14D || parameterName == NDFD_TMPABV30D || 
       parameterName == NDFD_TMPBLW30D || parameterName == NDFD_PRCPABV30D || 
       parameterName == NDFD_PRCPBLW30D || parameterName == NDFD_TMPABV90D || 
       parameterName == NDFD_TMPBLW90D || parameterName == NDFD_PRCPABV90D || 
       parameterName == NDFD_PRCPBLW90D)
   {
      if (parameterName == NDFD_MAX || parameterName == NDFD_MIN ||
          parameterName == NDFD_CONHAZ || parameterName == NDFD_PTORN ||
          parameterName == NDFD_PHAIL || parameterName == NDFD_PTSTMWIND ||
          parameterName == NDFD_PXTORN || parameterName == NDFD_PXHAIL ||
          parameterName == NDFD_PXTSTMWIND || parameterName == NDFD_PSTORM ||
          parameterName == NDFD_PXSTORM)

          period = 24;

      else if (parameterName == NDFD_POP || parameterName == NDFD_WH)
         period = 12;

      else if (parameterName == NDFD_QPF || parameterName == NDFD_SNOW ||
               parameterName == NDFD_INC34 || parameterName == NDFD_INC50 ||
               parameterName == NDFD_INC64 || parameterName == NDFD_CUM34 ||
               parameterName == NDFD_CUM50 || parameterName == NDFD_CUM64)
         period = 6;

      else if (parameterName == NDFD_TMPABV14D || parameterName == NDFD_TMPBLW14D ||
               parameterName == NDFD_PRCPABV14D || parameterName == NDFD_PRCPBLW14D)

         /* Period for 8-14 Day Climate Outlook Products. */
         period = 6*24;

      else if (parameterName == NDFD_TMPABV30D || parameterName == NDFD_TMPBLW30D ||
               parameterName == NDFD_PRCPABV30D || parameterName == NDFD_PRCPBLW30D)
         /* Period for 1-Month Climate Outlook Products. Choose the number of 
          * hours in 30 days. The exact number of hours (in 28 to 31 days) is
          * not needed here, since we will output the period in months in the
          * layout key.
          */
         period = 30*24;

      else if (parameterName == NDFD_TMPABV90D || parameterName == NDFD_TMPBLW90D ||
               parameterName == NDFD_PRCPABV90D || parameterName == NDFD_PRCPBLW90D)
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
