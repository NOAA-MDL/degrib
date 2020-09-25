/******************************************************************************

 * dwmlEnumSort() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Get a new order of enumerations, strictly for the way the DWML elements 
 *   need to be formatted in the layout (dictated by the parameters.xsd schema).
 * 
 * ARGUMENTS
 *         numElem = Number of elements returned by genProbe (those formatted 
 *                   plus those used in deriving formatted elements)A = First 
 *                   NDFD match for comparison. (Input)
 *
 *            Dwml = Subset of NDFD2DWML above that holds the DWML enumerations
 *                   (the order the elements need to be formatted in, dictated 
 *                   by schema). Array holds just those elemetns queried for. 
 *                   (Output)
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
void dwmlEnumSort(size_t numElem, dwmlEnum **Dwml)
{
   int i, j;
   int holder = 0;

   for (i = numElem - 1; i >= 0; i--)
   {
      for (j = 1; j <= i; j++)
      {
         if ((*Dwml)[j-1].Ndfd2Dwml > (*Dwml)[j].Ndfd2Dwml)
         {
            holder = (*Dwml)[j-1].Ndfd2Dwml;
            (*Dwml)[j-1].Ndfd2Dwml = (*Dwml)[j].Ndfd2Dwml;
            (*Dwml)[j].Ndfd2Dwml = holder;
            holder = (*Dwml)[j-1].origNdfdIndex;
            (*Dwml)[j-1].origNdfdIndex = (*Dwml)[j].origNdfdIndex;
            (*Dwml)[j].origNdfdIndex = holder;
         }
      }
   }

    return;
}
