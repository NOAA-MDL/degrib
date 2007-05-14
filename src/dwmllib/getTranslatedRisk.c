/******************************************************************************
 * getTranslatedRisk() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Gets the English translations for the Categorical Convective Hazard 
 *   Outlook element. 
 *      
 * ARGUMENTS
 * convHazCat = The categorical number for the Convective Hazard Outlook. This 
 *              number needs to be translated into an English language 
 *              equivalent. (Input)
 * transStr = Outgoing string with the translated qualifier. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2007 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getTranslatedRisk(int convHazCat, char *transStr)
{
   if (convHazCat == 0)
   {
      strcpy(transStr, "no thunderstorms");
      return;
   }

   else if (convHazCat == 2)
   {
      strcpy(transStr, "general thunderstorms");
      return;
   }

   else if (convHazCat == 4)
   {
      strcpy(transStr, "slight risk of severe thunderstorms");
      return;
   }

   else if (convHazCat == 6)
   {
      strcpy(transStr, "moderate risk of severe thunderstorms");
      return;
   }

   else if (convHazCat == 8)
   {
      strcpy(transStr, "high risk of severe thunderstorms");
      return;
   }

   return;
}
