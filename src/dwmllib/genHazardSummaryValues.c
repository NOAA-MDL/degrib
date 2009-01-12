/*****************************************************************************
 * genHazardSummaryValues() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Formats applicable Hazard information in the DWMLGenByDay 12 hourly and
 *   24 hourly summary products.
 *
 * ARGUMENTS
 *          pnt = The point index. (Input)
 * multiLayouts = The key linking each point's individual hazards to their 
 *                valid times (ex. k-p3h-n42-3). This array is allocated to the
 *                number of hazards per individual point, since each hazard is 
 *                treated as it's own element if user chose a summary product. 
 *                (Input)
 *       match = Pointer to the array of element matches from degrib. (Input) 
 *   numRowsHZ = The number of data rows for hazards (both active and not 
                 active). (Input)
 *  parameters = Xml Node parameter. The formatted hazards are child
 *               elements of this node. (Input)
 *    startNum = First index in match structure an individual point's data 
 *               matches can be found. (Input)
 *      endNum = Last index in match structure an individual point's data
 *               matches can be found. (Input)
 *      cwaStr = The CWA this point falls into. String is part of the 
 *               <hazardTextURL> element.(Input)
 * ptsIndivHazs = Array holding information about each point's individual 
 *                hazards. Structure contains hazard info members containing 
 *                startTime, the endTime, the number of consecutive hours the 
 *                hazard exists, the time of a resolution split (1hr res 
 *                changes to 3hr resolution after 3rd day) and the string code.
 *                (Input).
 *  numHazards = Number of active hazards for this point. (Input).
 *   layoutKey = The dummied up layoutKey that matches each forecast period. To
 *               be used when there are no active hazards. (Input)
 *
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  9/2008 Paul Hershberg (MDL): Created.
 * 12/2008 Paul Hershberg (MDL): Formats hazard name the number of times the 
 *                               hazard spans across different forecast 
 *                               periods.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void genHazardSummaryValues(size_t pnt, char **multiLayouts, genMatchType *match,
                            numRowsInfo numRowsHZ, xmlNodePtr parameters,
                            int startNum, int endNum, char *cwaStr, 
                            hazInfo *ptsIndivHazs, int numHazards, 
                            char *layoutKey)
{
   int i;                    /* Counter. */
   int hzIndex = 0;          /* Counter thru hazard ugly strings (hazard data 
                              * rows). */
   char *pstr1 = NULL;       /* Temp Pointer. */
   char *hzCode = NULL;      /* An individual hazard group's full code. */
   char *hzPhen = NULL;      /* An individual hazard group's phenomena. */
   char *hzSig = NULL;       /* An individual hazard group's significance. */
   char *hzETN = NULL;       /* An individual hazard group's event tracking 
                              * number. */
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
   xmlNodePtr name = NULL; /* Xml Node Pointer for node "name". */
   xmlNodePtr hazard_conditions = NULL;  /* Xml Node Pointer for node
                                          * "hazard-conditions". */
   xmlNodePtr hazard = NULL;   /* Xml Node Pointer for node "hazard". */
   int f_formatHazTextURL = 1; /* Flag to determine if <hazardTextURL> element 
                                * is to be formatted. */
   char tempBuff[150]; /* Temporary string. */
   char *pstr;         /* Temporary pointer character string. */
   int numHazSpanPeriods = 1; /* Number of hazard names we need to format per 
                               * individual hazard. Is equal to the number of 
                               * forecast periods this hazard spans (denoted by
                               * the # after 'n' in the time-layout). */

/* Initialize the base strings for the hazard icons and hazard Text URL's. */
   char baseIconURL[] = "http://forecast.weather.gov/images/wtf/";
   char baseTextURL[] = "http://forecast.weather.gov/wwamap/wwatxtget.php?cwa=";

/************************************************************************/

   /* If no hazards are occurring, set a nil for entire forecast period. */
   if (numHazards == 0)
   {
      hazards = xmlNewChild(parameters, NULL, BAD_CAST "hazards", NULL);
      xmlNewProp(hazards, BAD_CAST "time-layout", BAD_CAST layoutKey);

      /* Format the display <name> element tag. */
      name = xmlNewChild(hazards, NULL, BAD_CAST "name", BAD_CAST 
                         "Watches, Warnings, and Advisories");

      /* Format one <hazard-conditions> element with a nil denoting a time
       * duration spanning the entire forecast period. 
       */
      hazard_conditions = xmlNewChild(hazards, NULL, BAD_CAST
                                      "hazard-conditions", NULL);
      xmlNewProp(hazard_conditions, BAD_CAST "xsi:nil", BAD_CAST "true");

      return;
   }

   /* Loop over each active hazards occurring at this point and prepare to 
    * format them. 
    */
   for (hzIndex = 0; hzIndex < numHazards; hzIndex++)
   {
      if (ptsIndivHazs[hzIndex].numConsecRows != 0)
      {
         /* Format the <hazards> element tag. */
         hazards = xmlNewChild(parameters, NULL, BAD_CAST "hazards", NULL);
         xmlNewProp(hazards, BAD_CAST "time-layout", BAD_CAST 
                    multiLayouts[hzIndex]);

         /* Format the display <name> element tag. */
         name = xmlNewChild(hazards, NULL, BAD_CAST "name", BAD_CAST 
                            "Watches, Warnings, and Advisories");

         /* Format one <hazard-conditions> element per each hazard. */
         hazard_conditions = xmlNewChild(hazards, NULL, BAD_CAST
                                         "hazard-conditions", NULL);

         /* How many forecast periods does this one hazard span? We need to
          * duplicate the hazard name for each period it spans. Find this info
          * from the time-layout string -- it's denoted by the # after the "n"
          * in the time-layout. 
          */
         pstr = strchr(multiLayouts[hzIndex], 'n'); 
         numHazSpanPeriods = atoi(pstr+1);

         for (i = 0; i < numHazSpanPeriods; i++)
         { 
            /* For each hazard, process its hazard information. This can include
             * the Phenomena (i.e. "SC" == "Small Craft"), Significance (i.e. 
             * "Y" == "Advisory") and Event Tracking Number (i.e. "1001"). The
             * Phenomena, Significance, and ETN will be delimited by ("." or ":").
             */
            f_icon = 0;
            f_formatHazTextURL = 1;

            /* Format each <hazard>. */
            hazard = xmlNewChild(hazard_conditions, NULL, BAD_CAST "hazard",
                     NULL);

            /* Create the associative array holding the hazard phenomena,
             * significance, and ETN. Find the first Phenomena firstly.
             */
            hzCode = malloc(strlen(ptsIndivHazs[hzIndex].code) + 1);
            strcpy (hzCode, ptsIndivHazs[hzIndex].code);

            /* Since strtok affects the string, make a temp copy. */
            strcpy (tempBuff, ptsIndivHazs[hzIndex].code);
            pstr1 = strtok(tempBuff, ".");

            /* Initialize a few things. */
            hzPhen = malloc(strlen("none") + 1);
            strcpy (hzPhen, "none");
            hzSig = malloc(strlen("none") + 1);
            strcpy (hzSig, "none");
            hzETN = malloc(strlen("none") + 1);
            strcpy (hzETN, "none");

            while (pstr1 != NULL)
            {
               hzPhen = (char *) realloc(hzPhen, (strlen(pstr1)+1) * 
                        sizeof(char)); 
               strcpy (hzPhen, pstr1);
               pstr1 = strtok (NULL, ":");
               while (pstr1 != NULL)
               {
                  hzSig = (char *) realloc(hzSig, (strlen(pstr1)+1) * 
                                           sizeof(char)); 
                  strcpy (hzSig, pstr1);
                  pstr1 = strtok (NULL, ":");
                  while (pstr1 != NULL)
                  {
                     if (strlen(pstr1) >= 2)
                     {
                        hzETN = (char *) realloc(hzETN, (strlen(pstr1)+1) * 
                                                 sizeof(char)); 
                        strcpy (hzETN, pstr1);
                     } 
                     pstr1 = strtok (NULL, ":");
                  }
               }
            }
             
            /* Begin formatting the hazard attributes. We can format the entire
             * hazardCode (no translation). 
             */
            strTrim (hzCode);
            xmlNewProp(hazard, BAD_CAST "hazardCode", BAD_CAST hzCode);

            /* Translate and format hazard phenomena and get the icon based 
             * off of the phenomena (and significance in a few cases).  
             */
            strTrim(hzPhen);
            getHazPhenAndIcon(hzPhen, hzSig, transPhenomenaStr, &f_icon, 
                              iconStr);
            xmlNewProp(hazard, BAD_CAST "phenomena", BAD_CAST 
                       transPhenomenaStr);

            /* Translate and Format hazard significance. */
            strTrim(hzSig);
            getTranslatedHzSig(hzSig, transSignificanceStr);
            
            xmlNewProp(hazard, BAD_CAST "significance", BAD_CAST
                       transSignificanceStr);

            /* Format hazard Event Tracking Number, if available. */
            if (strcmp(hzETN, "none") != 0)
            {
               strTrim(hzETN);
               xmlNewProp(hazard, BAD_CAST "eventTrackingNumber", BAD_CAST 
               hzETN);
            }

            /* Format hazard Type (duration). */
            xmlNewProp(hazard, BAD_CAST "hazardType", BAD_CAST hazardType);

            /* Derive and format the <hazardTextURL> string. */
            genHazTextURL(baseTextURL, cwaStr, transPhenomenaStr, 
                          transSignificanceStr, hzCode, 
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

            /* Free a few things. */
            free(hzCode);
            free(hzPhen);
            free(hzSig);
            free(hzETN);
         }

      } /* Else, hazard was combined, continue on to next hazard. */

   } /* Closing out hzIndex "for" loop */

   return;
}
