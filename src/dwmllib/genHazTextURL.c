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
 *   baseTextURL = Static, beginning portion of hazard Text URL. (Input)
 *        cwaStr = The CWA this point falls into. String is part of the 
 *                 <hazardTextURL> element.(Input)
 *     phenomena = Translated phenomena portion of the hazard. (Input)
 *  significance = Translated significance portion of the hazard. (Input)
 *          code = Untranslated hazard code (ugly hazard string). (Input)
 * hazardTextURL = Complete outgoing hazardTextURL. (Output)
 * f_formatHazTextURL = Flag denoting if we format <hazardTextURL> at all.
 *                      (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   6/2008 Paul Hershberg (MDL): Created
 *   7/2008 Paul Hershberg (MDL): Accounted for special cases of "Small Craft"
 *                                and "Areal Flood" hazards.
 *  11/2011 Paul Hershberg (MDL): Added new Marine Statement to = Marine 
 *                                Weather Statement                                
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void  genHazTextURL(char *baseTextURL, char *cwaStr, char *phenomena, 
                    char *significance, char *code, char *hazardTextURL, 
                    int *f_formatHazTextURL)
{

   char delim[] = " "; /* Delimiter between words making up hazard phenomena. */
   char *result = NULL; /* Result of check to determine how many words make up 
                         * the hazard phenomena. */

   /* Default base Text URL for those few hazards which don't have corresponding
    * links to the WWA hazard Text URL Pages: */
   char baseZFPurl[] = "http://www.weather.gov/view/validProds.php?prod=ZFP&node=K";
   char upperCWA[10]; /* Upper case CWA. */
   
   /* A few hazards don't have corresponding links to the WWA hazard Text URL 
    * Pages. In those cases, use the ZFP text URL pages, i.e. 
    * (http://www.weather.gov/view/validProds.php?prod=ZFP&node=KREV). If there
    * was no translated phenomena at all, simply return the ZFP text
    * URL as the <hazardTextURL if we have a cwa available. If no cwa is 
    * available, format no <hazardTextURL> at all.
    */
   if (strcmp(code, "RECHIP") == 0 || strcmp(code, "RECLOP") == 0 ||
       strcmp(code, "AQA") == 0 || strcmp(phenomena, "none") == 0)
   {
      if (cwaStr == NULL)
      {
         *f_formatHazTextURL = 0;
         return;
      }
      else
      {
         strcpy (upperCWA, cwaStr);
         strToUpper (upperCWA);
         sprintf (hazardTextURL, "%s%s", baseZFPurl, upperCWA);
         return;
      }
   }

   /* Create as much of the URL string as we can. */
   if (cwaStr == NULL)
      sprintf (hazardTextURL, "%s%s%s", baseTextURL, "usa", "&wwa=");
   else   
      sprintf (hazardTextURL, "%s%s%s", baseTextURL, cwaStr, "&wwa=");

   /* A special case concerning the "Small Craft" hazards. */
   if ((strstr(phenomena, "Small Craft") != '\0'))
   {       
      strcat (hazardTextURL, "Small");
      strcat (hazardTextURL, "%20");
      strcat (hazardTextURL, "Craft");
      strcat (hazardTextURL, "%20");

      /* Append the significance to the hazartTextURL string as long as it is 
       * not "none". 
       */
      if (strcmp(significance, "none") != 0)
         strcat (hazardTextURL, significance);

      return;
   }
 
   /* A special case concerning the "Areal Flood" hazards. */
   if ((strstr(phenomena, "Areal Flood") != '\0'))
   {       
      strcat (hazardTextURL, "Flood");
      strcat (hazardTextURL, "%20");

      /* Append the significance to the hazartTextURL string as long as it is 
       * not "none". 
       */
      if (strcmp(significance, "none") != 0)
         strcat (hazardTextURL, significance);

      return;
   }
 
   /* A special case concerning the four new Marine hazards. The "Marine" part
    * of the hazard needs to be dropped when making the URL. 
    */
   if ((strstr(phenomena, "Marine Dense Fog") != '\0'))
   {       
      strcat (hazardTextURL, "Dense");
      strcat (hazardTextURL, "%20");
      strcat (hazardTextURL, "Fog");
      strcat (hazardTextURL, "%20");

      /* Append the significance to the hazartTextURL string as long as it is 
       * not "none". 
       */
      if (strcmp(significance, "none") != 0)
         strcat (hazardTextURL, significance);

      return;
   }
   if ((strstr(phenomena, "Marine Dense Smoke") != '\0'))
   {       
      strcat (hazardTextURL, "Dense");
      strcat (hazardTextURL, "%20");
      strcat (hazardTextURL, "Smoke");
      strcat (hazardTextURL, "%20");

      /* Append the significance to the hazartTextURL string as long as it is 
       * not "none". 
       */
      if (strcmp(significance, "none") != 0)
         strcat (hazardTextURL, significance);

      return;
   }
   if ((strstr(phenomena, "Marine Volcanic Ashfall") != '\0'))
   {       
      strcat (hazardTextURL, "Volcanic");
      strcat (hazardTextURL, "%20");
      strcat (hazardTextURL, "Ashfall");
      strcat (hazardTextURL, "%20");

      /* Append the significance to the hazartTextURL string as long as it is 
       * not "none". 
       */
      if (strcmp(significance, "none") != 0)
         strcat (hazardTextURL, significance);

      return;
   }
   if ((strstr(phenomena, "Marine") != '\0'))
   {
      strcat (hazardTextURL, "Marine");
      strcat (hazardTextURL, "%20");
      strcat (hazardTextURL, "Weather");
      strcat (hazardTextURL, "%20");

      /* Append the significance to the hazartTextURL string as long as it is 
 *        * not "none". 
 *               */
      if (strcmp(significance, "none") != 0)
         strcat (hazardTextURL, significance);

      return;
   }

   /* All other cases.
    * Append the phenomena to the hazardTextURL string. 
    */
   result = strtok (phenomena, delim);
   while (result != NULL)
   {
      strcat (hazardTextURL, result); 
      strcat (hazardTextURL, "%20");
      result = strtok (NULL, delim);
   }

   /* Append the significance to the hazartTextURL string as long as it is 
    * not "none". 
    */
   if (strcmp(significance, "none") != 0)
      strcat (hazardTextURL, significance);

   return;
}
