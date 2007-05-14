/******************************************************************************
 * genIconLinks() --
 *
 * Paul Hershberg / MDL
 *
 * PURPOSE
 *  This code formats the the Weather Conditions Icon element.
 *
 * ARGUMENTS
 *    iconInfo = Structure holding derived Icon links and time info. (Input) 
 *     numRows = The number of data rows formatted into ouput XML for icons
 *               (same as number of rows for weather). (Input)
 *   layoutKey = The key linking the icons to their valid times 
 *               (ex. k-p3h-n42-1). Its the same as the weather element's 
 *               layoutKey. (Input)
 *  parameters = An xml Node Pointer denoting the parameter node (Output).
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
void genIconLinks(icon_def *iconInfo, uChar numRows, char *layoutKey, 
                  xmlNodePtr parameters)
{

   int index;                   /* Counter for icons. */
   xmlNodePtr conditions_icon;  /* An Xml Node Pointer. */
   xmlNodePtr icon_link;        /* An Xml Node Pointer. */

   /* Format the <conditions_icon> element. */
   conditions_icon = xmlNewChild(parameters, NULL, BAD_CAST "conditions-icon",
                                 NULL);
   xmlNewProp(conditions_icon, BAD_CAST "type", BAD_CAST "forecast-NWS");
   xmlNewProp(conditions_icon, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element. */
   xmlNewChild(conditions_icon, NULL, BAD_CAST "name", BAD_CAST
               "Conditions Icons");

   /* Loop over all the icon values and format them (there is one for each
    * weather value). */
   for (index = 0; index < numRows; index++)
   {
      /* If the data is missing, so indicate in the XML (nil=true). */
      if (strcmp(iconInfo[index].str, "none") == 0)
      {
         icon_link = xmlNewChild(conditions_icon, NULL, BAD_CAST "icon-link",
                                 NULL);
         xmlNewProp(icon_link, BAD_CAST "xsi:nil", BAD_CAST "true");
      }
      else                    /* Good data, so format it. */
         xmlNewChild(conditions_icon, NULL, BAD_CAST "icon-link",
                     BAD_CAST iconInfo[index].str);
   }
   return;
}
