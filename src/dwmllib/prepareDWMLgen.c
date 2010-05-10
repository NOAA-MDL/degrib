/*****************************************************************************
 * prepareDWMLgen() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Flags those elements that are ultimately formatted in the output XML for 
 *  the DWMLgen products. The user chooses the elements for the "time-series" 
 *  product. There are pre-defined elements formatted for the "glance" product.
 *  Routine also sets the flag determining if period names are used in the time
 *  layout generation.
 *
 * ARGUMENTS
 *           f_XML = Flag denoting type of XML product. 
 *                   1 = DWMLgen's "time-series" product,
 *                   2 = DWMLgen's "glance" product, 
 *                   3 = DWMLgenByDay's "12 hourly" product, 
 *                   4 = DWMLgenByDay's "24 hourly" product, 
 *                   5 = DWMLgen's "RTMA time-series" product,
 *                   6 = DWMLgen's mix of "RTMA & NDFD time-series" product. 
 *                   (Input) 
 *     wxParameters = Array containing the flags denoting whether a certain 
 *                    element is formatted in the output XML (= 1), or used in 
 *                    deriving another formatted element (= 2). (Input/Output) 
 *    summarization = The type of temporal summarization being used.
 *                    Currently, no summarization is done in time.
 * f_formatPeriodName = Flag to indicate if period names (i.e. "today") appear 
 *                      in the start valid time tag: 
 *                      <start-valid-time period-name="today"> (Input)
 *        varFilter = Array denotes what NDFD variables are of interest (set to
 *                    = 1) or vital (set to = 2) to this call. If the variable 
 *                    is set to 2, then the variable wasn't set on the command 
 *                    line as an argument, but the call needs this element to 
 *                    derive others. (Input)
 *          numPnts = Total number of points being processed. (Input)
 *           f_icon = Flag denoting if icons is to be formatted for this point.
 *                    (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  2/2006 Paul Hershberg (MDL): Created.
 *  9/2007 Paul Hershberg (MDL): Added 12 Climate Outlook Elements
 * 12/2007 Paul Hershberg (MDL): Added 10 RTMA Elements
 *  5/2008 Paul Hershberg (MDL): Added Hazard Element
 *  8/2009 Paul Hershberg (MDL): Added Lamp Tstm element. 
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void prepareDWMLgen(uChar f_XML, uChar * f_formatPeriodName,
                    uChar ***wxParameters, size_t numPnts, char *summarization,
                    uChar varFilter[NDFD_MATCHALL+1], sChar *f_icon, 
                    size_t *numElem, genElemDescript **elem)
{
   int i, k, j, m; /* Counter(s) thru elements. */
   int f_elemAllocatedForSky = 0;
   int f_elemAllocatedForQpf = 0;
   int f_elemAllocatedForTd = 0;
   int f_elemAllocatedForTemp = 0;
   int f_elemAllocatedForWdir = 0;
   int f_elemAllocatedForWspd = 0;

   strcpy(summarization, "none"); /* There is no hourly summariztion for the
                                   * DWMLgen products. */

   for (j = 0; j < numPnts; j++)
   {
      /* Flag those elements in the "time-series" product to be formatted in the 
       * output XML. 
       */
      if (f_XML == 1 || f_XML == 5 || f_XML == 6)
      {
         for (i = 0; i < *numElem; i++)
         {
            if (varFilter[(*elem)[i].ndfdEnum] == 2)
               ((*wxParameters)[j][i]) = 1;
         }
         /* We need to create a time layout for the icons in the case that only
          * icons is to be formatted. When this occurs, make Icons use the
          * weather's time layout. Set the wxParameters flag for WX to = 3. 
          */

         for (i = 0; i < *numElem; i++)
         {
            if (*f_icon == 1 && (*elem)[i].ndfdEnum == NDFD_WX && 
                ((*wxParameters)[j][i]) != 1)
            {
                ((*wxParameters)[j][i]) = 3;
                break;
            }
         }
      }

      /* For DWMLgen's "glance" product, there are six pre-defined set of NDFD
       * parameters to be formatted. Five of these are maxt, mint, sky, hazard, 
       * and wx. Three other elements not formatted (Pop, temp and wind speed) 
       * are used to derive the 6th element formatted: Icons. 
       */
      if (f_XML == 2)
      {
         *f_formatPeriodName = 1;
         for (i = 0; i < *numElem; i++)
         {
            if ((*elem)[i].ndfdEnum == NDFD_MAX && varFilter[(*elem)[i].ndfdEnum] >= 2)
            {
               ((*wxParameters)[j][i]) = 1;
               continue;
            }
            if ((*elem)[i].ndfdEnum == NDFD_MIN && varFilter[(*elem)[i].ndfdEnum] >= 2)
            {
               ((*wxParameters)[j][i]) = 1;
               continue;
            }
            if ((*elem)[i].ndfdEnum == NDFD_SKY && varFilter[(*elem)[i].ndfdEnum] >= 2)
            {
               ((*wxParameters)[j][i]) = 1;
               continue;
            }
            if ((*elem)[i].ndfdEnum == NDFD_WX && varFilter[(*elem)[i].ndfdEnum] >= 2)
            {
               ((*wxParameters)[j][i]) = 1;
               continue;
            }
            if ((*elem)[i].ndfdEnum == NDFD_WWA && varFilter[(*elem)[i].ndfdEnum] >= 2)
            {
               ((*wxParameters)[j][i]) = 1;
               continue;
            }
         }
      }

      /* Turn on the concatenated RTMA-NDFD elements, if warranted. This will 
       * necessitate a reallocation of wxParameters and elem arrays as these 
       * (up to 6) elements are new. Follow the order specified in xmlparse.h
       * for the concatenated elements (#55 thru #60). 
       */
      if  (f_XML == 6)
      {
         for (i = 0; i < *numElem; i++)
         {
            if ((*elem)[i].ndfdEnum == NDFD_SKY && ((*wxParameters)[j][i]) == 1)
            {
                for (k = i; k < *numElem; k++)
                {
                   if ((*elem)[k].ndfdEnum == RTMA_SKY && ((*wxParameters)[j][k]) == 1)  
                   {
                      /* Since elem array is not point allocated, only realloc once 
                       * in the point loop, if needed. Flag this to prevent another 
                       * allocation for subsequent points. 
                      */
                      if (!f_elemAllocatedForSky)
                      {
                         *numElem = *numElem + 1;
                         (*elem) = realloc((*elem), *numElem * sizeof (genElemDescript));                   
                         (*elem)[*numElem-1].ndfdEnum = RTMA_NDFD_SKY;
                         f_elemAllocatedForSky = 1;
                         ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                         ((*wxParameters)[j][*numElem-1]) = 1;
                         break;
                      }
                      else
                      {
                         for (m = k; m < *numElem; m++)
                         {
                            if ((*elem)[m].ndfdEnum == RTMA_NDFD_SKY)
                            { 
                               ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                               ((*wxParameters)[j][m]) = 1;
                               break;                        
                            }
                         }
                      }
                   }
               }
            }
         }

         for (i = 0; i < *numElem; i++)
         {
            if ((*elem)[i].ndfdEnum == NDFD_QPF && ((*wxParameters)[j][i]) == 1)
            {
                for (k = i; k < *numElem; k++)
                {
                   if ((*elem)[k].ndfdEnum == RTMA_PRECIPA && ((*wxParameters)[j][k]) == 1)  
                   {
                      /* Since elem array is not point allocated, only realloc once 
                       * in the point loop, if needed. Flag this to prevent another 
                       * allocation for subsequent points. 
                       */
                      if (!f_elemAllocatedForQpf)
                      {
                         *numElem = *numElem + 1;
                         (*elem) = realloc((*elem), *numElem * sizeof (genElemDescript));                   
                         (*elem)[*numElem-1].ndfdEnum = RTMA_NDFD_PRECIPA;
                         f_elemAllocatedForQpf = 1;
                         ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                         ((*wxParameters)[j][*numElem-1]) = 1;
                         break;
                      }
                      else
                      {
                         for (m = k; m < *numElem; m++)
                         {
                            if ((*elem)[m].ndfdEnum == RTMA_NDFD_PRECIPA)
                            { 
                               ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                               ((*wxParameters)[j][m]) = 1;
                               break;                        
                            }
                         }
                      }
                   }
               }
            }
         }

         for (i = 0; i < *numElem; i++)
         {
            if ((*elem)[i].ndfdEnum == NDFD_TD && ((*wxParameters)[j][i]) == 1)
            {
               for (k = i; k < *numElem; k++)
               {
                  if ((*elem)[k].ndfdEnum == RTMA_TD && ((*wxParameters)[j][k]) == 1)  
                  {

                     /* Since elem array is not point allocated, only realloc once 
                      * in the point loop, if needed. Flag this to prevent another 
                      * allocation for subsequent points. 
                      */
                     if (!f_elemAllocatedForTd)
                     {
                        *numElem = *numElem + 1;
                        (*elem) = realloc((*elem), *numElem * sizeof (genElemDescript));
                        (*elem)[*numElem-1].ndfdEnum = RTMA_NDFD_TD;
                        f_elemAllocatedForTd = 1;
                        ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                        ((*wxParameters)[j][*numElem-1]) = 1;
                        break;
                     } 
                     else
                     {
                        for (m = k; m < *numElem; m++)
                        {
                           if ((*elem)[m].ndfdEnum == RTMA_NDFD_TD)
                           { 
                              ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                              ((*wxParameters)[j][m]) = 1;
                              break;                        
                           }
                        }
                     }
                  }
               }
            }
         }

         for (i = 0; i < *numElem; i++)
         {
            if ((*elem)[i].ndfdEnum == NDFD_TEMP && ((*wxParameters)[j][i]) == 1)
            {
               for (k = i; k < *numElem; k++)
               {
                  if ((*elem)[k].ndfdEnum == RTMA_TEMP && ((*wxParameters)[j][k]) == 1)  
                  {
                     /* Since elem array is not point allocated, only realloc once 
                      * in the point loop, if needed. Flag this to prevent another 
                      * allocation for subsequent points. 
                      */
                     if (!f_elemAllocatedForTemp)
                     {
                        *numElem = *numElem + 1;
                        (*elem) = realloc((*elem), *numElem * sizeof (genElemDescript));
                        (*elem)[*numElem-1].ndfdEnum = RTMA_NDFD_TEMP;
                        f_elemAllocatedForTemp = 1;
                        ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                        ((*wxParameters)[j][*numElem-1]) = 1;
                        break;
                     }
                     else
                     {
                        for (m = k; m < *numElem; m++)
                        {
                           if ((*elem)[m].ndfdEnum == RTMA_NDFD_TEMP)
                           { 
                              ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                              ((*wxParameters)[j][m]) = 1;
                              break;                        
                           }
                        }
                     }
                  }
               }
            }
         }

         for (i = 0; i < *numElem; i++)
         {
            if ((*elem)[i].ndfdEnum == NDFD_WD && ((*wxParameters)[j][i]) == 1)
            {
               for (k = i; k < *numElem; k++)
               {
                  if ((*elem)[k].ndfdEnum == RTMA_WDIR && ((*wxParameters)[j][k]) == 1)  
                 {
                     /* Since elem array is not point allocated, only realloc once 
                      * in the point loop, if needed. Flag this to prevent another 
                      * allocation for subsequent points. 
                      */
                     if (!f_elemAllocatedForWdir)
                     {
                        *numElem = *numElem + 1;
                        (*elem) = realloc((*elem), *numElem * sizeof (genElemDescript));                   
                        (*elem)[*numElem-1].ndfdEnum = RTMA_NDFD_WDIR;
                        f_elemAllocatedForWdir = 1;
                        ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                        ((*wxParameters)[j][*numElem-1]) = 1;
                        break;
                     }
                     else
                     {
                        for (m = k; m < *numElem; m++)
                        {
                           if ((*elem)[m].ndfdEnum == RTMA_NDFD_WDIR)
                           { 
                              ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                              ((*wxParameters)[j][m]) = 1;
                              break;                        
                           }
                        }
                     }
                  }
               }
            }
         }

         for (i = 0; i < *numElem; i++)
         {
            if ((*elem)[i].ndfdEnum == NDFD_WS && ((*wxParameters)[j][i]) == 1)
            {
               for (k = i; k < *numElem; k++)
               {
                  if ((*elem)[k].ndfdEnum == RTMA_WSPD && ((*wxParameters)[j][k]) == 1)  
                  {
                     /* Since elem array is not point allocated, only realloc once 
                      * in the point loop, if needed. Flag this to prevent another 
                      * allocation for subsequent points. 
                      */
                     if (!f_elemAllocatedForWspd)
                     {
                        *numElem = *numElem + 1;
                        (*elem) = realloc((*elem), *numElem * sizeof (genElemDescript));                   
                        (*elem)[*numElem-1].ndfdEnum = RTMA_NDFD_WSPD;
                        f_elemAllocatedForWspd = 1;
                        ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                        ((*wxParameters)[j][*numElem-1]) = 1;
                        break;
                     }
                     else
                     {
                        for (m = k; m < *numElem; m++)
                        {
                           if ((*elem)[m].ndfdEnum == RTMA_NDFD_WSPD)
                           { 
                              ((*wxParameters)[j]) = (uChar *) (realloc(((*wxParameters)[j]), *numElem * sizeof (uChar)));
                              ((*wxParameters)[j][m]) = 1;
                              break;                        
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   return;
}
