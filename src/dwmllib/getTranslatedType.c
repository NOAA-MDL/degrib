/******************************************************************************
 * getTranslatedType() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Get the English translations for the NDFD weather type part of the 
 *   "ugly string". 
 *      
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD weather type. (Input)
 * transStr = Outgoing string with the translated type. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2006 Paul Hershberg (MDL): Created
 *   3/2007 Paul Hershberg (MDL): Added entry for hail ("A").

 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getTranslatedType(char *uglyStr, char *transStr)
{
   if (strcmp(uglyStr, "ZL") == 0)
   {
      strcpy(transStr, "freezing drizzle");
      return;
   }

   else if (strcmp(uglyStr, "ZR") == 0)
   {
      strcpy(transStr, "freezing rain");
      return;
   }

   else if (strcmp(uglyStr, "SW") == 0)
   {
      strcpy(transStr, "snow showers");
      return;
   }

   else if (strcmp(uglyStr, "BS") == 0)
   {
      strcpy(transStr, "blowing snow");
      return;
   }

   else if (strcmp(uglyStr, "BD") == 0)
   {
      strcpy(transStr, "blowing dust");
      return;
   }

   else if (strcmp(uglyStr, "RW") == 0)
   {
      strcpy(transStr, "rain showers");
      return;
   }

   else if (strcmp(uglyStr, "IP") == 0)
   {
      strcpy(transStr, "ice pellets");
      return;
   }

   else if (strcmp(uglyStr, "FR") == 0)
   {
      strcpy(transStr, "frost");
      return;
   }

   else if (strcmp(uglyStr, "R") == 0)
   {
      strcpy(transStr, "rain");
      return;
   }

   else if (strcmp(uglyStr, "S") == 0)
   {
      strcpy(transStr, "snow");
      return;
   }

   else if (strcmp(uglyStr, "T") == 0)
   {
      strcpy(transStr, "thunderstorms");
      return;
   }

   else if (strcmp(uglyStr, "L") == 0)
   {
      strcpy(transStr, "drizzle");
      return;
   }

   else if (strcmp(uglyStr, "F") == 0)
   {
      strcpy(transStr, "fog");
      return;
   }

   else if (strcmp(uglyStr, "H") == 0)
   {
      strcpy(transStr, "haze");
      return;
   }

   else if (strcmp(uglyStr, "K") == 0)
   {
      strcpy(transStr, "smoke");
      return;
   }

   else if (strcmp(uglyStr, "BN") == 0)
   {
      strcpy(transStr, "blowing sand");
      return;
   }

   else if (strcmp(uglyStr, "IC") == 0)
   {
      strcpy(transStr, "ice crystals");
      return;
   }

   else if (strcmp(uglyStr, "VA") == 0)
   {
      strcpy(transStr, "volcanic ash");
      return;
   }

   else if (strcmp(uglyStr, "WP") == 0)
   {
      strcpy(transStr, "water spouts");
      return;
   }

   else if (strcmp(uglyStr, "ZF") == 0)
   {
      strcpy(transStr, "freezing fog");
      return;
   }

   else if (strcmp(uglyStr, "IF") == 0)
   {
      strcpy(transStr, "ice fog");
      return;
   }

   else if (strcmp(uglyStr, "ZY") == 0)
   {
      strcpy(transStr, "freezing spray");
      return;
   }

   else if (strcmp(uglyStr, "A") == 0)
   {
      strcpy(transStr, "hail");
      return;
   }

   else if (strcmp(uglyStr, "none") == 0)
   {
      strcpy(transStr, "");
      return;
   }

   return;
}
