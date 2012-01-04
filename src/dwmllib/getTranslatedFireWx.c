/******************************************************************************
 * getTranslatedFireWx() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Gets the English translations for the Fire Wx Outlook element. 
 *      
 * ARGUMENTS
 * fireWxCat  = The categorical number for the Fire Wx Outlook. This 
 *              number needs to be translated into an English language 
 *              equivalent. (Input)
 * transStr   = Outgoing string with the translated qualifier. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   1/2011 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void getTranslatedFireWx(int fireWxCat, char *transStr)
{
   /* Initialize phrase to "No Areas". */
   strcpy(transStr, "No Areas");

   if (fireWxCat < 8)       /* Value of 0. */
   {
   /* strcpy(transStr, "No Areas"); */
      return;
   }

   else if (fireWxCat < 10)  /* Value of 8. */
   {
      strcpy(transStr, "Critical Areas");
      return;
   }

   else if (fireWxCat == 10)  /* Value of 10. */
   {
      strcpy(transStr, "Extremely Critical Areas");
      return;
   }

   return;
}
