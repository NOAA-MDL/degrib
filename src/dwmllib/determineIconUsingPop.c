/******************************************************************************
 * determineIconUsingPop() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   This routine creates an icon name based on using a user provided POP value. 
 *   This name (string) is used in creating the URL for the icon.
 *
 * ARGUMENTS
 *         iconString = Individual member of iconInfo structure holding derived
 *                      Icon links and time info. (Output)
 *       wxStrSection = Part of string with weather info ("nra", etc). (Input)
 *      jpgStrSection = The "jpg" part of the string. (Input)
 *     POP12ValToPOP3 = Current value of the PoP12 concurrent with weather 
 *                      time. (Input)
 *            baseURL = String value holding the URL path to the icons. The
 *                      URL looks like http://www.crh.noaa.gov/weather/
 *                      images/fcicons/. (Input) 
 *                                         
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2006  Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void determineIconUsingPop(char *iconString, char *wxStrSection, 
		           char *jpgStrSection, int POP12ValToPOP3, 
			   char *baseURL) 
{
   /* First, round the POP12hr integer value to the nearest 10. */
   if (POP12ValToPOP3 >= 0)
      POP12ValToPOP3 = roundPopNearestTen(POP12ValToPOP3);
   
   if (POP12ValToPOP3 >= 10 && POP12ValToPOP3 <= 100)
      sprintf(iconString, "%s%s%d%s", baseURL, wxStrSection, 
	      POP12ValToPOP3, jpgStrSection);
   else
      sprintf(iconString, "%s%s%s", baseURL, wxStrSection, 
	      jpgStrSection);

   return;
}
