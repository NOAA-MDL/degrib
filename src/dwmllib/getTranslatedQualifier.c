/******************************************************************************
 * getTranslatedQualifier() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Get the English translations for the NDFD weather qualifier part of the 
 *   "ugly string". 
 *      
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather qualifier. (Input)
 * transStr = Outgoing string with the translated qualifier. (Output)
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
void getTranslatedQualifier(char *uglyStr, char *transStr)
{
   if (strcmp(uglyStr, "FL") == 0)
   {
      strcpy(transStr, "frequent lightning");
      return;
   }

   else if (strcmp(uglyStr, "HvyRn") == 0)
   {
      strcpy(transStr, "heavy rain");
      return;
   }

   else if (strcmp(uglyStr, "SmA") == 0)
   {
      strcpy(transStr, "small hail");
      return;
   }

   else if (strcmp(uglyStr, "OLA") == 0)
   {
      strcpy(transStr, "outlying areas");
      return;
   }

   else if (strcmp(uglyStr, "GW") == 0)
   {
      strcpy(transStr, "gusty winds");
      return;
   }

   else if (strcmp(uglyStr, "DmgW") == 0)
   {
      strcpy(transStr, "damaging winds");
      return;
   }

   else if (strcmp(uglyStr, "LgA") == 0)
   {
      strcpy(transStr, "large hail");
      return;
   }

   else if (strcmp(uglyStr, "OBO") == 0)
   {
      strcpy(transStr, "on bridges and overpasses");
      return;
   }

   else if (strcmp(uglyStr, "OGA") == 0)
   {
      strcpy(transStr, "on grassy areas");
      return;
   }

   else if (strcmp(uglyStr, "OR") == 0)
   {
      strcpy(transStr, "or");
      return;
   }

   else if (strcmp(uglyStr, "Dry") == 0)
   {
      strcpy(transStr, "dry");
      return;
   }

   else if (strcmp(uglyStr, "Primary") == 0)
   {
      strcpy(transStr, "highest ranking");
      return;
   }

   else if (strcmp(uglyStr, "Mention") == 0)
   {
      strcpy(transStr, "include unconditionally");
      return;
   }

   else if (strcmp(uglyStr, "TOR") == 0)
   {
      strcpy(transStr, "tornado");
      return;
   }

   else if (strcmp(uglyStr, "MX") == 0)
   {
      strcpy(transStr, "mixture");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}
