/******************************************************************************
 * getTranslatedCoverage() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Get the English translations for the NDFD weather coverage part of the 
 *   "ugly string". For example, NDFD coverage "SChc" is converted to its 
 *   english equivilant "slight chance".
 *
 *   
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather coverage. (Input)
 * transStr = Outgoing string with the translated coverage. (Output)
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
void getTranslatedCoverage(char *uglyStr, char *transStr)
{
   if (strcmp(uglyStr, "SChc") == 0)
   {
      strcpy(transStr, "slight chance");
      return;
   }

   else if (strcmp(uglyStr, "Chc") == 0)
   {
      strcpy(transStr, "chance");
      return;
   }

   else if (strcmp(uglyStr, "Lkly") == 0)
   {
      strcpy(transStr, "likely");
      return;
   }

   else if (strcmp(uglyStr, "Ocnl") == 0)
   {
      strcpy(transStr, "occasional");
      return;
   }

   else if (strcmp(uglyStr, "Def") == 0)
   {
      strcpy(transStr, "definitely");
      return;
   }

   else if (strcmp(uglyStr, "Iso") == 0)
   {
      strcpy(transStr, "isolated");
      return;
   }

   else if (strcmp(uglyStr, "Sct") == 0)
   {
      strcpy(transStr, "scattered");
      return;
   }

   else if (strcmp(uglyStr, "Num") == 0)
   {
      strcpy(transStr, "numerous");
      return;
   }

   else if (strcmp(uglyStr, "Areas") == 0)
   {
      strcpy(transStr, "areas");
      return;
   }

   else if (strcmp(uglyStr, "Patchy") == 0)
   {
      strcpy(transStr, "patchy");
      return;
   }

   else if (strcmp(uglyStr, "Wide") == 0)
   {
      strcpy(transStr, "widespread");
      return;
   }

   else if (strcmp(uglyStr, "Pds") == 0)
   {
      strcpy(transStr, "periods of");
      return;
   }

   else if (strcmp(uglyStr, "Frq") == 0)
   {
      strcpy(transStr, "frequent");
      return;
   }

   else if (strcmp(uglyStr, "Inter") == 0)
   {
      strcpy(transStr, "intermittent");
      return;
   }

   else if (strcmp(uglyStr, "Brf") == 0)
   {
      strcpy(transStr, "brief");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "none");
      return;
   }

   return;
}
