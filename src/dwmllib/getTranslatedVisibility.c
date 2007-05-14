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
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getTranslatedVisibility(char *uglyStr, char *transStr)
{
   if (strcmp(uglyStr, "0SM") == 0)
   {
      strcpy(transStr, "0");
      return;
   }

   else if (strcmp(uglyStr, "1/4SM") == 0)
   {
      strcpy(transStr, "1/4");
      return;
   }

   else if (strcmp(uglyStr, "1/2SM") == 0)
   {
      strcpy(transStr, "1/2");
      return;
   }

   else if (strcmp(uglyStr, "3/4SM") == 0)
   {
      strcpy(transStr, "3/4");
      return;
   }

   else if (strcmp(uglyStr, "1SM") == 0)
   {
      strcpy(transStr, "1");
      return;
   }

   else if (strcmp(uglyStr, "11/2SM") == 0)
   {
      strcpy(transStr, "1 1/2");
      return;
   }

   else if (strcmp(uglyStr, "2SM") == 0)
   {
      strcpy(transStr, "2");
      return;
   }

   else if (strcmp(uglyStr, "21/2SM") == 0)
   {
      strcpy(transStr, "2 1/2");
      return;
   }

   else if (strcmp(uglyStr, "3SM") == 0)
   {
      strcpy(transStr, "3");
      return;
   }

   else if (strcmp(uglyStr, "4SM") == 0)
   {
      strcpy(transStr, "4");
      return;
   }

   else if (strcmp(uglyStr, "5SM") == 0)
   {
      strcpy(transStr, "5");
      return;
   }

   else if (strcmp(uglyStr, "6SM") == 0)
   {
      strcpy(transStr, "6");
      return;
   }

   else if (strcmp(uglyStr, "P6SM") == 0)
   {
      strcpy(transStr, "6+");
      return;
   }
   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}
