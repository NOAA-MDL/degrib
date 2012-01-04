/*****************************************************************************
 * formatMetaDWML() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  Creates the Meta Data for the formatted XML products (DWMLgen's 
 *  "time-series" and "glance" products and DWMLgenByDay's "12 hourly" and "24
 *  hourly" formatted products.
 *
 * ARGUMENTS
 *       f_XML = Flag denoting type of XML product. 
 *               1 = DWMLgen's "time-series" product,
 *               2 = DWMLgen's "glance" product, 
 *               3 = DWMLgenByDay's "12 hourly" product, 
 *               4 = DWMLgenByDay's "24 hourly" product, 
 *               5 = DWMLgen's "RTMA time-series" product,
 *               6 = DWMLgen's mix of "RTMA & NDFD time-series" product. 
 *               (Input) 
 *         doc = An xml Node Pointer denoting the top-level document node. 
 *               (Input)
 *        data = An xml Node Pointer denoting the <data> node. (Input)
 *        dwml = An xml Node Pointer denoting the <dwml> node. (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 * HISTORY:
 *  1/2006 Paul Hershberg (MDL): Created.
 *  3/2007 Paul Hershberg (MDL): --Changed operational status to "official".
 *                               --Added the srsName attribute to <product> 
 *                                 element.
 * 11/2007 Paul Hershberg (MDL): --Added RTMA info.
 *  9/2011 Paul Hershberg (MDL): --Updated instances of 
 *                                 "www.weather.gov/foreasts" to 
 *                                 "graphical.weather.gov" for move of Web 
 *                                 Service from gate to NIDS
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void formatMetaDWML(uChar f_XML, xmlDocPtr * doc, xmlNodePtr * data,
                    xmlNodePtr * dwml)
{
   char *spatialRefSystem = NULL; /* Denotes the spatial reference system for 
                                   * use in properly geo-locating the data when
                                   * overlaying forecasts on maps and 
                                   * integrating them with other datasets, 
                                   * etc. */
   char *prodOrFormat = NULL; /* Denotes the product of format for output
                               * into XML. */
   char *operationalMode = NULL;  /* Denotes the operational mode for output
                                   * into XML. */
   char *productTitle = NULL; /* Denotes the product title for output into
                               * XML. */
   char *moreInfo = NULL;     /* Denotes the URL where more info can be found 
                               * for output into XML. */
   char *prod_center = NULL;  /* Denotes the place of development for output
                               * into XML. */
   char *sub_center = NULL;   /* Denotes the sub-place of development for
                               * output into XML. */
   char *category = NULL;     /* Denotes category (forecast or analysis). */
   char currentTime[30];      /* Denotes current UTC time the product is
                               * created. */
   double currentDoubTime;    /* Denotes current UTC time (as a double), the
                               * product is created. */

   /* Local XML Node pointers */
   xmlNodePtr node = NULL;
   xmlNodePtr head = NULL;
   xmlNodePtr production_center = NULL;

   /* Set up the header information depending on product. */
   switch (f_XML)
   {
      case 1: /* DWMLGen, NDFD elements only, time-series product. */
         spatialRefSystem = "WGS 1984";
         prodOrFormat = "time-series";
         productTitle = "NOAA's National Weather Service Forecast Data";
         operationalMode = "official";
         moreInfo = "http://graphical.weather.gov/xml/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         category = "forecast";
         break;
      case 2: /* DWMLGen, NDFD elements only, glance product. */
         spatialRefSystem = "WGS 1984";
         prodOrFormat = "glance";
         productTitle = "NOAA's National Weather Service Forecast at a Glance";
         operationalMode = "official";
         moreInfo = "http://graphical.weather.gov/xml/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         category = "forecast";
         break;
      case 3: /* DWMLGenByDay NDFD 12hr summary product. */
         spatialRefSystem = "WGS 1984";
         prodOrFormat = "dwmlByDay";
         productTitle =
               "NOAA's National Weather Service Forecast by 12 Hour Period";
         operationalMode = "official";
         moreInfo = "http://graphical.weather.gov/xml/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         category = "forecast";
         break;
      case 4: /* DWMLGenByDay NDFD 24hr summary product. */
         spatialRefSystem = "WGS 1984";
         prodOrFormat = "dwmlByDay";
         productTitle =
               "NOAA's National Weather Service Forecast by 24 Hour Period";
         operationalMode = "official";
         moreInfo = "http://graphical.weather.gov/xml/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         category = "forecast";
         break;
      case 5: /* DWMLGen, RTMA elements only, as a time-series. */
         spatialRefSystem = "WGS 1984";
         prodOrFormat = "time-series";
         productTitle =
               "NOAA's National Weather Service Real Time Mesoscale Analysis Data";
         operationalMode = "official";
         moreInfo = "http://www.emc.ncep.noaa.gov/mmb/rtma/";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         category = "analysis";
         break;
      case 6: /* DWMLGen, mix of RTMA & NDFD elements, as a time-series. */
         spatialRefSystem = "WGS 1984";
         prodOrFormat = "time-series";
         productTitle =
               "NOAA's National Weather Service Real Time Mesoscale Analysis And Forecast Data";
         operationalMode = "official";
         moreInfo = "http://products.weather.gov/search.php";
         prod_center = "Meteorological Development Laboratory";
         sub_center = "Product Generation Branch";
         category = "analysis and forecast";
         break;
   }

   /* Get the document creation date. */
   currentDoubTime = Clock_Seconds();
   Clock_Print2(currentTime, 30, currentDoubTime, "%Y-%m-%dT%H:%M:%S", 0, 0);
   strcat(currentTime, "Z");

   /* Create the XML document and format the header data. */
   *doc = xmlNewDoc(BAD_CAST "1.0");
   *dwml = xmlNewNode(NULL, BAD_CAST "dwml");
   xmlNewProp(*dwml, BAD_CAST "version", BAD_CAST "1.0");
   xmlNewProp(*dwml, BAD_CAST "xmlns:xsd", BAD_CAST
              "http://www.w3.org/2001/XMLSchema");
   xmlNewProp(*dwml, BAD_CAST "xmlns:xsi", BAD_CAST
              "http://www.w3.org/2001/XMLSchema-instance");
   xmlNewProp(*dwml, BAD_CAST "xsi:noNamespaceSchemaLocation", BAD_CAST
              "http://graphical.weather.gov/xml/DWMLgen/schema/DWML.xsd");

   /* Set root element <dwml>. */
   xmlDocSetRootElement(*doc, *dwml);

   /* Add <head> as child element to <dwml> element. */
   head = xmlNewNode(NULL, BAD_CAST "head");

   /* Add <product> as child element to <head> element and add attributes. */
   node = xmlNewChild(head, NULL, BAD_CAST "product", NULL);
   xmlNewProp(node, BAD_CAST "srsName", BAD_CAST spatialRefSystem);
   xmlNewProp(node, BAD_CAST "concise-name", BAD_CAST prodOrFormat);
   xmlNewProp(node, BAD_CAST "operational-mode", BAD_CAST operationalMode);

   /* Add <title>, <field>, <category>, and <creation-date> as child elements 
    * to <product> element. 
    */
   xmlNewChild(node, NULL, BAD_CAST "title", BAD_CAST productTitle);
   xmlNewChild(node, NULL, BAD_CAST "field", BAD_CAST "meteorological");
   xmlNewChild(node, NULL, BAD_CAST "category", BAD_CAST category);
   node = xmlNewChild(node, NULL, BAD_CAST "creation-date", BAD_CAST
                      currentTime);
   xmlNewProp(node, BAD_CAST "refresh-frequency", BAD_CAST "PT1H");

   /* Add <source> as child element to <head> element. */
   node = xmlNewNode(NULL, BAD_CAST "source");
   xmlNewChild(node, NULL, BAD_CAST "more-information", BAD_CAST moreInfo);

  /* Add <production-center>, <disclaimer>, <credit>, and <feedback> as child 
   * elements to <source> element. */   
   production_center = xmlNewChild(node, NULL, BAD_CAST
                                   "production-center", BAD_CAST prod_center);
   xmlNewChild(production_center, NULL, BAD_CAST "sub-center", BAD_CAST
               sub_center);
   xmlNewChild(node, NULL, BAD_CAST "disclaimer", BAD_CAST
               "http://www.nws.noaa.gov/disclaimer.html");
   xmlNewChild(node, NULL, BAD_CAST "credit", BAD_CAST
               "http://www.weather.gov/");
   xmlNewChild(node, NULL, BAD_CAST "credit-logo", BAD_CAST
               "http://www.weather.gov/images/xml_logo.gif");
   xmlNewChild(node, NULL, BAD_CAST "feedback", BAD_CAST
               "http://www.weather.gov/feedback.php");

   /* Set <source> element as child to <head> element and <head> element as child
    * to <dwml> element.
    */
   xmlAddChild(head, node);
   xmlAddChild(*dwml, head);
 
   /* Create the <data> element in which all formatted XML will be attached. */
   *data = xmlNewNode(NULL, BAD_CAST "data");

   return;
}
