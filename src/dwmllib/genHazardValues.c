/*****************************************************************************
 * genHazardValues() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Formats applicable Hazard information in the DWMLGen "time-series" and 
 *   "glance" products.
 *
 * ARGUMENTS
 *         pnt = The point index. (Input)
 *   layoutKey = The key linking the hazard elements to their valid times
 *               (ex. k-p3h-n42-1). (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *   numRowsHZ = The number of data rows for hazards. (Input)
 *  parameters = Xml Node parameter. The formatted hazards are child
 *               elements of this node. (Input)
 *    startNum = First index in match structure an individual point's data 
 *               matches can be found. (Input)
 *      endNum = Last index in match structure an individual point's data
 *               matches can be found. (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  5/2008 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void genHazardValues(size_t pnt, char *layoutKey, genMatchType *match,
                     numRowsInfo numRowsHZ, xmlNodePtr parameters,
                     int startNum, int endNum, char *cwaStr,
                     int f_noHazActiveForPoint)
{
   int i;                     /* Counter through match structure. */
   int priorElemCount;        /* Counter used to find elements' location in
                               * match. */
   int hzIndex = 0;           /* Counter thru hazard ugly strings (hazard data 
                               * rows). */
   int numGroups = 0;         /* Number of hazards per hazard ugly string. */
   int numValues;             /* An index denoting number of values (sig, phen, 
                               * ETN, etc) per hazard group. */
   int groupIndex;            /* An index into the HzGroups array denoting the 
                               * number of hazards per hazard ugly string.. */
/*   int valueIndex; */       /* An index into each weatherGroups fields (=
                               * 5). */
/*   int valueIsMissing = 0; */ /* Denotes if current weather is missing value. 
                                 */
   char *pstr = NULL;         /* Pointer to "hazard ugly string". */
   char *pstr1 = NULL;        /* Pointer to "hazard ugly string". */
   char *pstr2 = NULL;        /* Pointer to "hazard ugly string". */
   char **HzGroups = NULL;    /* An array holding the hazard groups for
                               * one ugly hazard string, each group delimited 
                               * by "^". */
   char **hzPhen = NULL;      /* An individual hazard group's phenomena. */
   char **hzSig = NULL;       /* An individual hazard group's significance. */
   char **hzETN = NULL;       /* An individual hazard group's significance. */
/*   char HzValues[5][50]; */ /* An associative array holding the current
                               * group's type, coverage, intensity, vis, &
                               * qualifier. */
   char hazardType[] = "long duration";
   char transPhenomenaStr[100];  /* String holding english translation of
                                  * hazard phenomena. */
   char transSignificanceStr[100];    /* String holding english translation of
                                       * hazard significance. */
   char iconStr[100]; /* The specific part of the icon string ("mf_gale.gif"). */
   char hazardIcon[200]; /* Total hazard Icon string. */
   char hazardTextURL[300]; /* Total hazard Text URL string. */
   int f_icon;        /* Denotes if a specific hazard group has an icon. */
   xmlNodePtr hazards = NULL; /* Xml Node Pointer for node "hazards". */
   xmlNodePtr hazard_conditions = NULL;  /* Xml Node Pointer for node
                                          * "hazard-conditions". */
   xmlNodePtr hazard = NULL;   /* Xml Node Pointer for node "hazard". */
   HZ *hzInfo = NULL;         /* Hazard data taken from the match array. */
   int numActualRowsHZ; /* Number may be reduced due to a smaller time window 
                         * chosen by user. */
 
/* Initialize the base strings for the hazard icons and hazard Text URL's. */
   char baseIconURL[] = "http://forecast.weather.gov/images/wtf/";
   char baseTextURL[] = "http://forecast.weather.gov/wwamap/wwatxtget.php?cwa=";

/************************************************************************/

   numActualRowsHZ = numRowsHZ.total-numRowsHZ.skipBeg-numRowsHZ.skipEnd;
   
   /* Firstly, create a subset array of structures holding the hazard data from
    * the match structure.
    */
   hzInfo = malloc(numActualRowsHZ * sizeof(HZ));
      
   /* Fill Hazard Array. */
   priorElemCount = 0;
   for (i = startNum; i < endNum; i++)
   { 
      if (match[i].elem.ndfdEnum == NDFD_WWA && 
	  match[i].validTime >= numRowsHZ.firstUserTime &&
	  match[i].validTime <= numRowsHZ.lastUserTime)
      {
         hzInfo[i-priorElemCount-startNum].validTime =
               match[i].validTime;
         if (match[i].value[pnt].valueType != 0 &&
             match[i].value[pnt].valueType != 2)
         {
            strcpy(hzInfo[i-priorElemCount-startNum].str, 
                   match[i].value[pnt].str);
         }

         hzInfo[i-priorElemCount-startNum].valueType =
               match[i].value[pnt].valueType;
      }
      else
         priorElemCount++;
   }

   /* Format the <hazards> element tag. */
   hazards = xmlNewChild(parameters, NULL, BAD_CAST "hazards", NULL);
   xmlNewProp(hazards, BAD_CAST "time-layout", BAD_CAST layoutKey);

   /* Format the display <name> element tag. */
   xmlNewChild(hazards, NULL, BAD_CAST "name", BAD_CAST 
               "Watches, Warnings, and Advisories");

   /* Loop over the Hazard data values and prepare to format them. */
   for (hzIndex = 0; hzIndex < numActualRowsHZ; hzIndex++)
   {
      /* Format one <hazard-conditions> element per hazard ugly string. */
      hazard_conditions = xmlNewChild(hazards, NULL, BAD_CAST
                                      "hazard-conditions", NULL);

      /* Check to see if we will format a 'nil' for missing data. */
      if (hzInfo[hzIndex].valueType == 2)
      {
         xmlNewProp(hazard_conditions, BAD_CAST "xsi:nil", BAD_CAST "true");
         continue;
      }
/*    if (valueIsMissing != 1) */
      /* Check to see if there are no hazards, i.e. == NULL. If so, simply 
       * format an empty <hazard-conditions> XML element tag. Denoted when 
       * valueType == 0.
       */ 
/*      else if (hzInfo[hzIndex].str[0] == '<' && hzInfo[hzIndex].str[1] == 'N' &&
               hzInfo[hzIndex].str[2] == 'o')
*/
      else if (hzInfo[hzIndex].valueType == 0)
      {
         continue;
      }
      else  /* We have hazard data. Need to see if there are multiple hazards 
             * (groups) in this ugly hazard string. 
             */
      {                      
         /* Initialize/Reset a few things. */
         numGroups = 0;
         groupIndex = 0;

         /* Now put any hazard groupings into an array using the "^" as the
          * delimiter between 2 or more hazards in a hazard ugly string. Find
          * the total number of groups in the hazard ugly string. Fill 
          * the first array elements (groupIndex = 0) before the others. 
          */
         pstr = strtok(hzInfo[hzIndex].str, "^");
         while (pstr != NULL)
         { 
            numGroups++;
            pstr = strtok (NULL, "^");
         }
         HzGroups = (char **)calloc(numGroups, sizeof(char *));

         /* Fill the first array elements (groupIndex = 0) before the others. */ 
         pstr = HzGroups[0];
         for (i = 0; pstr[i]; i++)
         {
            if (pstr[i] != '^')
               /* Simply copy over character into HzGroups. */
               HzGroups[groupIndex][i] = pstr[i];
            else if (pstr[i] == '^')
            {
               /* Copy over null character into HzGroups in place of "^". */
               HzGroups[groupIndex][i] = '\0';
               break;
            }
         }

         /* Get the total number of Hazards for this one row of hazard 
          * data. Set pointer now to delimiter of second hazard, if there is
          * one.
          */
         pstr1 = strchr(hzInfo[hzIndex].str, '^');
         while (pstr1 != NULL)
         {
            numGroups++;
            pstr1 = strchr(pstr1 + 1, '^');
         }

         /* Continue filling the array of HzGroups. */
         pstr = strchr(hzInfo[hzIndex].str, '^');
         pstr2 = strchr(hzInfo[hzIndex].str, '^');

         for (groupIndex = 1; groupIndex < numGroups + 1; groupIndex++)
         {
            for (i = 1; pstr[i]; i++)
            {
               if (pstr[i] != '^')
               {
                  HzGroups[groupIndex][i - 1] = pstr[i];
               }
               else if (pstr[i] == '^')
                  {
                  HzGroups[groupIndex][i - 1] = '\0';
                  pstr = strchr(pstr + 1, '^');
                  break;
               }
            }
         }
/*         else
         {
            if (pstr2[i - 1] == ':')
               HzGroups[numGroups][i - 1] = '\0';
         }
*/

         /* For each group, process its hazard information. This can include
          * the Phenomena (i.e. "SC" == "Small Craft"), Significance (i.e. 
          * "Y" == "Advisory") and Event Tracking Number (i.e. "1001"). The
          * Phenomena and Significance will be delimited by a period (".").
          * The Event Tracking Number will be delimited from the Phenomena or
          * Significance by a colon (":").
          */
         hzPhen = (char **)calloc(numGroups + 1, sizeof(char *));
         hzSig = (char **)calloc(numGroups + 1, sizeof(char *));
         hzETN = (char **)calloc(numGroups + 1, sizeof(char *));
	    
         /* Loop over each group. */
         for (groupIndex = 0; groupIndex < numGroups + 1; groupIndex++)
         {
            f_icon = 0;
            /* Format each <hazard> denoting multiple hazards per one ugly
             * hazard string. 
             */
            hazard = xmlNewChild(hazard_conditions, NULL, BAD_CAST "hazard",
                     NULL);

	    /* Initialize the number of hazard values (phenomena, 
             * significance, ETN's) per group. 
	     */
            numValues = 0;

            /* Create the associative array holding the hazard phenomena,
             * significance, and ETN. Find the first Phenomena firstly.
	     */
            pstr = HzGroups[groupIndex];
/*               valueIndex = 0;
                 f_hzPhenFoundByDelimiter = 0;
*/
            for (i = 0; pstr[i]; i++)
            {
               if (pstr[i] != '.' && pstr[i] != ':')
/*                     HzValues[valueIndex][i] = pstr[i]; */
                  hzPhen[groupIndex][i] = pstr[i];
               else if (pstr[i] == '.' || pstr[i] == ':')
               {
                  hzPhen[groupIndex][i] = '\0';
/*                  f_hzPhenFoundByDelimiter = 1; */
                  break;
               }
            }
/*               if (!f_hzPhenFoundByDelimiter)
                   HzValues[valueIndex][i] = '\0';               
*/
            /* Get the total number of HzValues (phenomena, significance, 
             * and ETN) in this first HzGroups. If HzValues array == HzGroups
             * array, then there is only one Hazard value, the Hazard 
             * Phenomena. There can be 1, 2, or 3 (Phen, Sig, ETN) values per
             * group.
             */
/*               pstr1 = strchr(HzGroups[groupIndex], ':'); */
            if (strcmp(hzPhen[groupIndex], HzGroups[groupIndex]) == 0)
            {
               hzPhen[groupIndex][i] = '\0';
            }
            else
            {
               if (strchr(HzGroups[groupIndex], ':') != NULL)
                  numValues++;
               if (strchr(HzGroups[groupIndex], '.') != NULL)
                  numValues++;
            }

            /* Bump this number up by one to account for phenomena. */
            numValues++;

            /* Done with this group's Phenomena. */
            strTrim(hzPhen[groupIndex]);

            /* Continue with the Significance and Event Tracking Number. */
            if (numValues > 1)
            {
               if (strchr(HzGroups[groupIndex], '.') != NULL)
               {
                  pstr = strchr(HzGroups[groupIndex], '.');
                  for (i = 1; pstr[i]; i++)
                  {
                     if (pstr[i] != ':')
                        hzSig[groupIndex][i - 1] = pstr[i];
                     else if (pstr[i] == ':')
                     {
                        hzSig[groupIndex][i - 1] = '\0';
                        break;
                     }
                  }
               }
               if (strchr(HzGroups[groupIndex], ':') != NULL)
               {
                  pstr = strchr(HzGroups[groupIndex], ':');
                  for (i = 1; pstr[i]; i++)
                  {
                     hzETN[groupIndex][i - 1] = pstr[i];
                  }
                  hzETN[groupIndex][i - 1] = '\0';

                  /* If not 4 digits, it's not an ETN, but a WFO segment 
                   * identifier. 
                   */
                  if (strlen(hzETN[groupIndex]) < 4)
                  {
                     hzETN[groupIndex] = malloc(strlen("none") + 1);
                     strcpy (hzETN[groupIndex], "none");
                  }
               }
            }
            else /* No significance or ETN, so set these to = "none". */
            {
               strcpy (hzSig[groupIndex], "none");
               strcpy (hzETN[groupIndex], "none");
            }
            
            /* Begin formatting the hazard attributes. Need to translate the 
             * Phenomena and Significance code into English Words, first.
             */

            /* Format hazardCode (no translation). */
            strTrim (HzGroups[groupIndex]);
            xmlNewProp(hazard, BAD_CAST "hazardCode", BAD_CAST 
                       HzGroups[groupIndex]);

            /* Translate and format hazard phenomena and get the icon based 
             * off of the phenomena and significance.  
             */
            getHazPhenAndIcon(hzPhen[groupIndex], transPhenomenaStr, &f_icon, 
                              iconStr, hzSig[groupIndex]);
            xmlNewProp(hazard, BAD_CAST "phenomena", BAD_CAST
                       transPhenomenaStr);

            /* Translate and Format hazard significance. */
            strTrim(hzSig[groupIndex]);
            getTranslatedHzSig(hzSig[groupIndex], transSignificanceStr);
            xmlNewProp(hazard, BAD_CAST "significance", BAD_CAST
                       transSignificanceStr);

            /* Format hazard Event Tracking Number. */
            strTrim(hzETN[groupIndex]);
            xmlNewProp(hazard, BAD_CAST "EventTrackingNumber", BAD_CAST
                       hzETN[groupIndex]);

            /* Format hazard Type (duration). */
            xmlNewProp(hazard, BAD_CAST "hazardType", BAD_CAST
                       hazardType);

            /* Derive and format the <hazardTextURL> string. */
            genHazTextURL(baseTextURL, cwaStr, transPhenomenaStr, 
                          transSignificanceStr, hazardTextURL); 
            strTrim(hazardTextURL);
            xmlNewChild(hazard, NULL, BAD_CAST "hazardTextURL", BAD_CAST
                        hazardTextURL);

            /* Derive and format the <hazardIcon> element string. */
            if (f_icon)
            {
               sprintf(hazardIcon, "%s%s", baseIconURL, iconStr);
               strTrim(hazardIcon);  
               xmlNewChild(hazard, NULL, BAD_CAST "hazardIcon", BAD_CAST
                           hazardIcon);
            }

/*            memset(hzPhen, '\0', 10 * 100);
              memset(hzSig, '\0', 10 * 100);
*/
            free(HzGroups[groupIndex]);
            
         } /* Closing out groupIndex "for" loop */

         /* Re-initialize the HzGroups array and free a few things. */
         free(HzGroups);
         free(hzPhen);
         free(hzSig);

      } /* Closing out if we have hazard data "else" statement. */

   } /* Closing out hzIndex "for" loop */

   return;
}
