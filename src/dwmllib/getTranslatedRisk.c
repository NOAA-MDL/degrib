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
 * transStr   = Outgoing string with the translated qualifier. (Output)
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
      strcpy(transStr, "No Thunderstorms");
      return;
   }

   else if (convHazCat == 2)
   {
      strcpy(transStr, "General Thunderstorms");
      return;
   }

   else if (convHazCat == 4)
   {
      strcpy(transStr, "Slight Risk of Severe Thunderstorms");
      return;
   }

   else if (convHazCat == 6)
   {
      strcpy(transStr, "Moderate Risk of Severe Thunderstorms");
      return;
   }

   else if (convHazCat == 8)
   {
      strcpy(transStr, "High Risk of Severe Thunderstorms");
      return;
   }

   return;
}
