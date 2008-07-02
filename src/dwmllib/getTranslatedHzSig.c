/******************************************************************************
 * getTranslatedHzSig() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Get the English translations for the NDFD hazard Significance part of the
 *   "hazard ugly string". For example, NDFD coverage "Y" is converted to
 *   its english equivilant "Advisory".
 *
 *   
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD hazard significance. (Input)
 * transStr = Outgoing string with the translated significance. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   6/2008 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getTranslatedHzSig(char *uglyStr, char *transStr)
{
   if (strcmp(uglyStr, "A") == 0)
   {
      strcpy(transStr, "Watch");
      return;
   }

   else if (strcmp(uglyStr, "S") == 0)
   {
      strcpy(transStr, "Statement");
      return;
   }

   else if (strcmp(uglyStr, "W") == 0)
   {
      strcpy(transStr, "Warning");
      return;
   }

   else if (strcmp(uglyStr, "Y") == 0)
   {
      strcpy(transStr, "Advisory");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}
