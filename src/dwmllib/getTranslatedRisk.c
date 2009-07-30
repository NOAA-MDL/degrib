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
 *   5/2009 Paul Hershberg (MDL): Routine now looks for ranges. Also added
 *                                an initialization set to "No Thunderstorms".
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getTranslatedRisk(int convHazCat, char *transStr)
{
   /* Initialize phrase to "No Thunderstorms". */
   strcpy(transStr, "No Thunderstorms");

   if (convHazCat < 2)       /* Values of 0 or 1. */
   {
   /* strcpy(transStr, "No Thunderstorms"); */
      return;
   }

   else if (convHazCat < 4)  /* Values of 2 or 3. */
   {
      strcpy(transStr, "General Thunderstorms");
      return;
   }

   else if (convHazCat < 6)  /* Values of 4 or 5. */
   {
      strcpy(transStr, "Slight Risk of Severe Thunderstorms");
      return;
   }

   else if (convHazCat < 8)  /* Values of 6 or 7. */
   {
      strcpy(transStr, "Moderate Risk of Severe Thunderstorms");
      return;
   }

   else if (convHazCat == 8)  /* Values of 8. */
   {
      strcpy(transStr, "High Risk of Severe Thunderstorms");
      return;
   }

   return;
}
