/*****************************************************************************
 * formatLocationInfo() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Code is used to format the Location info in XML.
 *  
 * ARGUMENTS
 *          numPnts = Number of points to probe for. (Input) 
 *             pnts = A pointer to the points to probe for (defined in type.h).
 *                    (Input)
 *             data = An xml Node Pointer denoting the <data> node of the 
 *                    formatted XML. (Output)
 *                            
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  5/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void formatLocationInfo(size_t numPnts, Point * pnts, xmlNodePtr data)
{

   int j;                     /* Point counter. */
   double lat;                /* Latitude of point. */
   double lon;                /* Longitude of point. */
   xmlNodePtr location = NULL;  /* Local xml Node Pointer denoting the
                                 * <location> node. */
   xmlNodePtr node = NULL;    /* Local xml Node Pointer. */
   char strPointBuff[20];     /* Temporary string holder for point. */
   char strLLBuff[20];        /* Temporary string holder for lat and lon. */

   /* Begin looping through the points to format each point's location,
    * grouping them together. 
    */
   for (j = 0; j < numPnts; j++)
   {
      if (isPntInASector(pnts[j]))
      {

         /* Before starting, make sure this point is in at least one of the
          * NDFD Sectors. 
          */
         lat = pnts[j].Y;
         lon = pnts[j].X;

         location = xmlNewNode(NULL, BAD_CAST "location");
         sprintf(strPointBuff, "point%d", (j + 1));
         xmlNewChild(location, NULL, BAD_CAST "location-key", BAD_CAST
                     strPointBuff);
         node = xmlNewChild(location, NULL, BAD_CAST "point", NULL);
         sprintf(strLLBuff, "%2.2f", lat);
         xmlNewProp(node, BAD_CAST "latitude", BAD_CAST strLLBuff);
         sprintf(strLLBuff, "%2.2f", lon);
         xmlNewProp(node, BAD_CAST "longitude", BAD_CAST strLLBuff);
         xmlAddChild(data, location);

      }
   }
   return;
}
