/******************************************************************************
 * getTranslatedIntensity() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Gets the English translations for the NDFD weather intensity part of the 
 *   "ugly string". 
 *      
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather intensity. (Input)
 * transStr = Outgoing string with the translated intensity. (Output)
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
void getTranslatedIntensity(char *uglyStr, char *transStr)
{
   if (strcmp(uglyStr, "--") == 0)
   {
      strcpy(transStr, "very light");
      return;
   }

   else if (strcmp(uglyStr, "-") == 0)
   {
      strcpy(transStr, "light");
      return;
   }

   else if (strcmp(uglyStr, "+") == 0)
   {
      strcpy(transStr, "heavy");
      return;
   }

   else if (strcmp(uglyStr, "m") == 0)
   {
      strcpy(transStr, "moderate");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}
