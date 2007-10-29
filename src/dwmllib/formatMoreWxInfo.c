/*****************************************************************************
 * formatMoreWxInfo() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Code used to format the <moreWeatherInfo> element in XML which contains 
 *  link to point and click NDFD forecast page for each set of user requested 
 *  lat/lon pairs. An example point and click URL for Wash DC area is:
 *
 *  "http://forecast.weather.gov/MapClick.php?textField1=38.99&textField2=-77.99"
 *  
 * ARGUMENTS
 *          numPnts = Number of user supplied lat/lon points of query. (Input)
 *             pnts = A pointer to the user supplied points (defined in type.h).
 *                    (Input)
 *             data = An xml Node Pointer denoting the <data> node of the 
 *                    formatted XML which <moreWeatherInformation> element will
 *                    be attached. (Output)
 *                            
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  6/2007 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void formatMoreWxInfo(size_t numPnts, Point * pnts, xmlNodePtr data)
{
   int j;                         /* Point counter. */
   double lat;                    /* Latitude of point. */
   double lon;                    /* Longitude of point. */
   char URL[150]; /* Denotes the URL link to the appropriate point 
                   * and click page for each lat/lon pair in the 
                   * query. */
   char URLpart1[] = "http://forecast.weather.gov/MapClick.php?textField1=";
   char URLpart2[] = "&textField2=";
   char pointBuff[20];        /* String holding point number. */
   xmlNodePtr pointClickURL = NULL; /* An xml Node Pointer denoting the
                                     * <moreWeatherInformation> node in the 
                                     * formatted XML. */

   /* Begin looping through the points to format the point and click URL 
    * string.    
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
         sprintf(URL, "%s%2.2f%s%2.2f", URLpart1, lat, URLpart2, lon);
         pointClickURL = xmlNewTextChild(data, NULL, BAD_CAST 
                         "moreWeatherInformation", BAD_CAST URL);
         sprintf(pointBuff, "point%d", (j + 1));
         xmlNewProp(pointClickURL, BAD_CAST "applicable-location", BAD_CAST pointBuff);
      }
   }
   return;
}
