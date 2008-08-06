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
 *      cwaStr = The CWA this point falls into. String is part of the 
 *               <hazardTextURL> element.(Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  5/2008 Paul Hershberg (MDL): Created.
 *  7/2008 Paul Hershberg (MDL): Made change to ETN delimiter (from "." to ":")
 *  7/2008 Paul Hershberg (MDL): Changed type "HZ" to "HZtype".
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void genHazardValues(size_t pnt, char *layoutKey, genMatchType *match,
                     numRowsInfo numRowsHZ, xmlNodePtr parameters,
                     int startNum, int endNum, char *cwaStr)
{
   int i;                     /* Counter through match structure. */
   int priorElemCount;        /* Counter used to find elements' location in
                               * match. */
   int hzIndex = 0;           /* Counter thru hazard ugly strings (hazard data 
                               * rows). */
   int numGroups = 0;         /* Number of hazards per hazard ugly string. */
   int groupIndex;            /* An index into the HzGroups array denoting the 
                               * number of hazards per hazard ugly string.. */
   char *pstr = NULL;         /* Pointer to "hazard ugly string". */
   char *pstr1 = NULL;        /* Pointer to "hazard ugly string". */
   char **HzGroups = NULL;    /* An array holding the hazard groups for
                               * one ugly hazard string, each group delimited 
                               * by "^". */
   char **hzCode = NULL;      /* An individual hazard group's full code. */
   char **hzPhen = NULL;      /* An individual hazard group's phenomena. */
   char **hzSig = NULL;       /* An individual hazard group's significance. */
   char **hzETN = NULL;       /* An individual hazard group's significance. */
   char hazardType[] = "long duration";
   char transPhenomenaStr[100];  /* String holding english translation of
                                  * hazard phenomena. */
   char transSignificanceStr[100];    /* String holding english translation of
                                       * hazard significance. */
   char iconStr[40]; /* The specific part of the icon string ("mf_gale.gif"). */
   char hazardIcon[300]; /* Total hazard Icon string. */
   char hazardTextURL[300]; /* Total hazard Text URL string. */
   int f_icon;        /* Denotes if a specific hazard group has an icon. */
   xmlNodePtr hazards = NULL; /* Xml Node Pointer for node "hazards". */
   xmlNodePtr hazard_conditions = NULL;  /* Xml Node Pointer for node
                                          * "hazard-conditions". */
   xmlNodePtr hazard = NULL;   /* Xml Node Pointer for node "hazard". */
   HZtype *hzInfo = NULL;      /* Hazard data taken from the match array. */
   int numActualRowsHZ; /* Number may be reduced due to a smaller time window 
                         * chosen by user. */
   int f_formatHazTextURL = 1; /* Flag to determine if <hazardTextURL> element 
                                * is to be formatted. */

/* Initialize the base strings for the hazard icons and hazard Text URL's. */
   char baseIconURL[] = "http://forecast.weather.gov/images/wtf/";
   char baseTextURL[] = "http://forecast.weather.gov/wwamap/wwatxtget.php?cwa=";

/************************************************************************/

   numActualRowsHZ = numRowsHZ.total-numRowsHZ.skipBeg-numRowsHZ.skipEnd;
   
   /* Firstly, create a subset array of structures holding the hazard data from
    * the match structure.
    */
   hzInfo = malloc(numActualRowsHZ * sizeof(HZtype));
      
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

      /* Check to see if there are no hazards, i.e. == <None>. If so, simply 
       * format an empty <hazard-conditions> XML element tag.
       */ 
      else if (hzInfo[hzIndex].str[0] == '<' && hzInfo[hzIndex].str[1] == 'N' &&
               hzInfo[hzIndex].str[2] == 'o' && hzInfo[hzIndex].str[3] == 'n')

      {
         continue;
      }
      else  /* We have hazard data. Need to see if there are multiple hazards 
             * (groups) in this ugly hazard string. 
             */
      {
         /* Initialize/Reset a few things. */
         numGroups = 0;

         /* Now put any hazard groupings into an array using the "^" as the
          * delimiter between 2 or more hazards in a hazard ugly string. Find
          * the total number of groups in the hazard ugly string. Fill 
          * the first array elements (groupIndex = 0) before the others. 
          */
         pstr = strtok(hzInfo[hzIndex].str, "^");
         while (pstr != NULL)
         { 
            numGroups++;
            HzGroups = (char **) realloc(HzGroups, numGroups * sizeof(char *));
            HzGroups[numGroups-1] = (char *) malloc((strlen(pstr)+1) * sizeof(char));
            strcpy (HzGroups[numGroups-1], pstr);
            pstr = strtok (NULL, "^");
         }

         /* For each group, process its hazard information. This can include
          * the Phenomena (i.e. "SC" == "Small Craft"), Significance (i.e. 
          * "Y" == "Advisory") and Event Tracking Number (i.e. "1001"). The
          * Phenomena, Significance, and ETN will be delimited by a period (".").
          */
         hzCode = (char **)calloc(numGroups, sizeof(char *));
         hzPhen = (char **)calloc(numGroups, sizeof(char *));
         hzSig = (char **)calloc(numGroups, sizeof(char *));
         hzETN = (char **)calloc(numGroups, sizeof(char *));

         /* Loop over each group. */
         for (groupIndex = 0; groupIndex < numGroups; groupIndex++)
         {
            f_icon = 0;
            f_formatHazTextURL = 1;

            /* Format each <hazard> denoting multiple hazards per one ugly
             * hazard string. 
             */
            hazard = xmlNewChild(hazard_conditions, NULL, BAD_CAST "hazard",
                     NULL);

            /* Create the associative array holding the hazard phenomena,
             * significance, and ETN. Find the first Phenomena firstly.
	     */
            hzCode[groupIndex] = malloc(strlen(HzGroups[groupIndex]) + 1);
            strcpy (hzCode[groupIndex],HzGroups[groupIndex]);

            pstr1 = strtok(HzGroups[groupIndex], ".");

            /* Initialize a few things. */
            hzPhen[groupIndex] = malloc(strlen("none") + 1);
            strcpy (hzPhen[groupIndex], "none");
            hzSig[groupIndex] = malloc(strlen("none") + 1);
            strcpy (hzSig[groupIndex], "none");
            hzETN[groupIndex] = malloc(strlen("none") + 1);
            strcpy (hzETN[groupIndex], "none");

            while (pstr1 != NULL)
            {
               hzPhen[groupIndex] = (char *) malloc((strlen(pstr1)+1) * sizeof(char));
               strcpy (hzPhen[groupIndex], pstr1);
               pstr1 = strtok (NULL, ":");
               while (pstr1 != NULL)
               {
                  hzSig[groupIndex] = (char *) malloc((strlen(pstr1)+1) * sizeof(char));
                  strcpy (hzSig[groupIndex], pstr1);
                  pstr1 = strtok (NULL, ":");

                  while (pstr1 != NULL)
                  {
                     if (strlen(pstr1) >= 2)
                     {
                        hzETN[groupIndex] = (char *) malloc((strlen(pstr1)+1) * sizeof(char));
                        strcpy (hzETN[groupIndex], pstr1);
                     } 
                     pstr1 = strtok (NULL, ":");
                  }
               }
            }
             
            /* Begin formatting the hazard attributes. We can format the 
             * entire hazardCode (no translation). */
            strTrim (hzCode[groupIndex]);
            xmlNewProp(hazard, BAD_CAST "hazardCode", BAD_CAST 
                       hzCode[groupIndex]);

            /* Translate and format hazard phenomena and get the icon based 
             * off of the phenomena (and significance in a few cases).  
             */
            strTrim(hzPhen[groupIndex]);
            getHazPhenAndIcon(hzPhen[groupIndex], hzSig[groupIndex],
                              transPhenomenaStr, &f_icon, iconStr);
            
            xmlNewProp(hazard, BAD_CAST "phenomena", BAD_CAST
                       transPhenomenaStr);

            /* Translate and Format hazard significance. */
            strTrim(hzSig[groupIndex]);
            getTranslatedHzSig(hzSig[groupIndex], transSignificanceStr);
            
            xmlNewProp(hazard, BAD_CAST "significance", BAD_CAST
                       transSignificanceStr);

            /* Format hazard Event Tracking Number, if available. */
            if (strcmp(hzETN[groupIndex], "none") != 0)
            {
               strTrim(hzETN[groupIndex]);
               xmlNewProp(hazard, BAD_CAST "eventTrackingNumber", BAD_CAST
                          hzETN[groupIndex]);
            }

            /* Format hazard Type (duration). */
            xmlNewProp(hazard, BAD_CAST "hazardType", BAD_CAST
                       hazardType);

            /* Derive and format the <hazardTextURL> string. */
            genHazTextURL(baseTextURL, cwaStr, transPhenomenaStr, 
                          transSignificanceStr, hzCode[groupIndex], 
                          hazardTextURL, &f_formatHazTextURL);
            if (f_formatHazTextURL)
            {
               strTrim(hazardTextURL);
               xmlNewTextChild(hazard, NULL, BAD_CAST "hazardTextURL", BAD_CAST
                               hazardTextURL);
            }

            /* Derive and format the <hazardIcon> element string. */
            if (f_icon)
            {
               sprintf(hazardIcon, "%s%s", baseIconURL, iconStr);
               strTrim(hazardIcon);  
               xmlNewChild(hazard, NULL, BAD_CAST "hazardIcon", BAD_CAST
                           hazardIcon);
            }

            free(HzGroups[groupIndex]);
            free(hzCode[groupIndex]);
            free(hzPhen[groupIndex]);
            free(hzSig[groupIndex]);
            free(hzETN[groupIndex]);
              
         } /* Closing out groupIndex "for" loop */

         /* Free a few things. */
         free(hzCode);
         free(hzPhen);
         free(hzSig);
         free(hzETN);

      } /* Closing out if we have hazard data "else" statement. */

   } /* Closing out hzIndex "for" loop */

   free(HzGroups);
   free(hzInfo);

   return;
}
