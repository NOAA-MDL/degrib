/*****************************************************************************
 * isDominant() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Routine takes two weather arguments and determines if the first one is 
 *   dominant.
 *
 * ARGUMENTS
 *        arg1 = The first argument, to be tested whether or not it dominates 
 *               the second argument. (Input)
 *        arg2 = The second argument, compared to the first. (Input)
 *     argType = Denotes which weather type we are comparing (coverage, 
 *               intensity, or type). (Input)
 * 
 * FILES/DATABASES: None
 *                
 * RETURNS: int (0 or 1)
 *
 *  6/2006 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
int isDominant(char *arg1, char *arg2, char *argType)
{

   /* The "Coverage" part of the ugly weather string. */
   if (strcmp(argType, "coverage") == 0)
   {
      if (strcmp(arg1, "none") == 0)
         return 0;
      else if (strcmp(arg1, "Patchy") == 0)
      {
         if (strcmp("none", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Areas") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Brf") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Inter") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Pds") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Ocnl") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Frq") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Iso") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "SChc") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Sct") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Chc") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0 ||
	    strcmp("Sct", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Num") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0 ||
	    strcmp("Sct", arg2) == 0 || strcmp("Chc", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Lkly") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0 ||
	    strcmp("Sct", arg2) == 0 || strcmp("Chc", arg2) == 0 || 
	    strcmp("Num", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Wide") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("Patchy", arg2) == 0 || 
	    strcmp("Areas", arg2) == 0 || strcmp("Brf", arg2) == 0 || 
	    strcmp("Inter", arg2) == 0 || strcmp("Pds", arg2) == 0 || 
	    strcmp("Ocnl", arg2) == 0 || strcmp("Frq", arg2) == 0 ||
	    strcmp("Iso", arg2) == 0 || strcmp("SChc", arg2) == 0 ||
	    strcmp("Sct", arg2) == 0 || strcmp("Chc", arg2) == 0 || 
	    strcmp("Num", arg2) == 0 || strcmp("Lkly", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "Def") == 0)
      {
         if (strcmp("Def", arg2) != 0)
    	    return 1;
	 else
            return 0;
      }
   }
   else if (strcmp(argType, "intensity") == 0)

   /* The "Intensity" part of the ugly weather string. */
   {
      if (strcmp(arg1, "none") == 0)
         return 0;
      else if (strcmp(arg1, "--") == 0)
      {
         if (strcmp("none", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "-") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("--", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "m") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("--", arg2) == 0 ||
            strcmp("-", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "+") == 0)
      {
         if (strcmp("+", arg2) != 0)
	     return 1;
	 else
	    return 0;
      }
   }
   else if (strcmp(argType, "type") == 0)

   /* The "Type" part of the ugly weather string. */
   {
      if (strcmp(arg1, "none") == 0)
         return 0;
      else if (strcmp(arg1, "F") == 0)
      {
         if (strcmp("none", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "BS") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "BD") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "BN") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "H") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "K") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "FR") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "VA") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "L") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "RW") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "R") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "IC") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "IF") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "SW") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "S") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "IP") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "ZF") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "ZY") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0 || strcmp("ZF", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "ZL") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0 || strcmp("ZF", arg2) == 0 ||
	    strcmp("ZY", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "ZR") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0 || strcmp("ZF", arg2) == 0 ||
	    strcmp("ZY", arg2) == 0 || strcmp("ZL", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "T") == 0)
      {
         if (strcmp("none", arg2) == 0 || strcmp("F", arg2) == 0 || 
            strcmp("BS", arg2) == 0 || strcmp("BD", arg2) == 0 ||
	    strcmp("BN", arg2) == 0 || strcmp("H", arg2) == 0 ||
	    strcmp("K", arg2) == 0 || strcmp("FR", arg2) == 0 ||
	    strcmp("VA", arg2) == 0 || strcmp("L", arg2) == 0 ||
	    strcmp("RW", arg2) == 0 || strcmp("R", arg2) == 0 ||
	    strcmp("IC", arg2) == 0 || strcmp("IF", arg2) == 0 ||
	    strcmp("SW", arg2) == 0 || strcmp("S", arg2) == 0 ||
	    strcmp("IP", arg2) == 0 || strcmp("ZF", arg2) == 0 ||
	    strcmp("ZY", arg2) == 0 || strcmp("ZL", arg2) == 0 ||
	    strcmp("ZR", arg2) == 0)
            return 1;
      }
      else if (strcmp(arg1, "WP") == 0)
      {
         if (strcmp("WP", arg2) != 0)
    	    return 1;
	 else
            return 0;
      }
   }
   return 0;
}
