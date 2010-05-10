/******************************************************************************
 * prepareVarFilter() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code calls code that sets up the varFilter array to show what NDFD 
 *  variables are of interest(1) or vital(2) to this procedure. Also, code
 *  manipulate the varFilter array depending on type of XML queried.
 *
 * ARGUMENTS
 *       f_XML = flag for 1 of the 4 DWML products (Input):
 *               1 = DWMLgen's "time-series" product. 
 *               2 = DWMLgen's "glance" product.
 *               3 = DWMLgenByDay's "12 hourly" format product.
 *               4 = DWMLgenByDay's "24 hourly" format product.
 *               5 = DWMLgen's RTMA "time-series" product.
 *               6 = DWMLgen's mix of "RTMA & NDFD time-series" product. 
 *               (Input)
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
 *     numElem = length of elem. (Output)
 *        elem = The element array to init. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2008 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void prepareVarFilter(sChar f_XML, sChar *f_icon, size_t numNdfdVars, 
                      uChar *ndfdVars, uChar varFilter[NDFD_MATCHALL+1], 
                      size_t *numElem, genElemDescript **elem)
{
   setVarFilter(f_XML, f_icon, numNdfdVars, ndfdVars, varFilter);

   /* Allow user filter of variables to reduce the element list, but include
    * all "vital" (Filter == 2) variables. 
    */
   genElemListInit2(varFilter, numNdfdVars, ndfdVars, numElem, elem);

   /* If product is DWMLgen's time-series, we need to decipher between
    * elements originating on the command line argument and those forced if
    * command line argument -Icon is set to 1 (turned on). 
    */
   if ((f_XML == 1 || f_XML == 6) && *f_icon == 1)
   {
      varFilter[NDFD_TEMP]--;
      varFilter[NDFD_WS]--;
      varFilter[NDFD_SKY]--;
      varFilter[NDFD_WX]--;
      varFilter[NDFD_POP]--;
   }

   /* We need to turn the f_icon flag on if all elements are to be formatted
    * by default (when there is no -ndfdfVars command option) AND there was
    * no forcing of elements. This will ensure Icons are formatted along
    * with all the NDFD elements. 
    */
   if ((f_XML == 1 || f_XML == 6) && numNdfdVars == 0 && *f_icon == 0)
      *f_icon = 1;

   /* Force Icons to be formatted if DWMLgen "glance" product. */
   if (f_XML == 2)
      *f_icon = 1;

   return;
}
