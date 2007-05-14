/*****************************************************************************
 * determinePeriodLength() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Determines the period length of an element. Time between the first validTime
 *  and the second validTime is used in the determination.
 *
 * ARGUMENTS
 *        startTime = First valid time of element. (Input)
 *          endTime = Second valid time of element. (Input)
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
int determinePeriodLength(double startTime, double endTime, uChar numRows, 
                          uChar parameterName)
{
   int period = 0;            /* Length between an elements successive
                               * validTimes. */

   /* If there is just one row of data, guess the period length. */
   if (numRows == 1 || endTime == 0.0)
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
      else
         period = 3;

      return period;
   }

   period = (endTime - startTime) / 3600;

   return period;
}
