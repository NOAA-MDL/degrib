/******************************************************************************
 * getTranslatedVisibility() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Gets the English translations for the NDFD weather visibility part of the 
 *   "ugly string". 
 *      
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather visibility. (Input)
 * transStr = Outgoing string with the translated visibility. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2006 Paul Hershberg (MDL): Created
 *   6/2011 Paul Hershberg (MDL): Added U.S. Standard unit to metric option.
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getTranslatedVisibility(sChar f_unit, char *uglyStr, char *transStr)
{
   if (strcmp(uglyStr, "0SM") == 0)
   {
      strcpy(transStr, "0");
      return;
   }
   else if (strcmp(uglyStr, "1/4SM") == 0)
   {
      if (f_unit != 2) /* U.S. Standard units */
         strcpy(transStr, "1/4");
      else /* Metric units. 1 statute mile = 1.609344 kimometers. */
         strcpy(transStr, "0.402");
      return;
   }
   else if (strcmp(uglyStr, "1/2SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "1/2");
      else
         strcpy(transStr, "0.805");
      return;
   }
   else if (strcmp(uglyStr, "3/4SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "3/4");
      else
         strcpy(transStr, "1.21");
      return;
   }
   else if (strcmp(uglyStr, "1SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "1");
      else
         strcpy(transStr, "1.61");
      return;
   }
   else if (strcmp(uglyStr, "11/2SM") == 0)
   {
      if (f_unit != 2)   
         strcpy(transStr, "1 1/2");
      else
         strcpy(transStr, "2.41");
      return;
   }
   else if (strcmp(uglyStr, "2SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "2");
      else
         strcpy(transStr, "3.22");
      return;
   }
   else if (strcmp(uglyStr, "21/2SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "2 1/2");
      else
         strcpy(transStr, "4.02");
      return;
   }
   else if (strcmp(uglyStr, "3SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "3");
      else
         strcpy(transStr, "4.83");
      return;
   }
   else if (strcmp(uglyStr, "4SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "4");
      else
         strcpy(transStr, "6.44");
      return;
   }
   else if (strcmp(uglyStr, "5SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "5");
      else
         strcpy(transStr, "8.05");
      return;
   }
   else if (strcmp(uglyStr, "6SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "6");
      else
         strcpy(transStr, "9.66");
      return;
   }
   else if (strcmp(uglyStr, "P6SM") == 0)
   {
      if (f_unit != 2)
         strcpy(transStr, "6+");
      else
         strcpy(transStr, "9.66+");
      return;
   }
   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}
