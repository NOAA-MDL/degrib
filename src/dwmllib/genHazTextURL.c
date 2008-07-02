/******************************************************************************
 * genHazTextURL() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE  
 *   Creates the hazard Text URL of the specific hazard being formatted. *
 *   
 * ARGUMENTS
 *  uglyStr = Incoming string with the NDFD hazard phenomena. (Input)
 * transStr = Outgoing string with the translated phenomena. (Output)
 *   f_icon = Denotes if this hazard phenomena has an associated icon. (Output).
 *  iconStr = Outgoing icon string if phenomena has an icon. (Output).
 *    hzSig = Significance portion of hazard. Used to determine icon. (Input)
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
void  genHazTextURL(char *baseTextURL, char *cwaStr, char *phenomena, 
                    char *significance, char *hazardTextURL)
{
   char delim[] = " ";
   char *result = NULL; /* Result of check to determine how many words make up 
                         * the hazard phenomena. */
   if (cwaStr == NULL)
      sprintf (hazardTextURL, "%s%s%s", baseTextURL, "usa", "&wwwa=");
   else   
      sprintf (hazardTextURL, "%s%s%s", baseTextURL, cwaStr, "&wwwa=");

   /* Append the phenomena to the hazartTextURL string. */
   result = strtok (phenomena, delim);
   while (result != NULL)
   {
      strcat (hazardTextURL, result); 
      strcat (hazardTextURL, "%20");
   }

   /* Append the significance to the hazartTextURL string. */
   strcat (hazardTextURL, significance);

   return;
}
