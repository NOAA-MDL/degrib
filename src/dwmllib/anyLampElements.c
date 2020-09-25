/*****************************************************************************
 * anyLampElements() -- 
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *   Checks to see if query has any Lamp elements. If so, we need to add to the
 *   directories genProbe can look for data.
 *   
 * ARGUMENTS
 *  numInFiles = Number of input files. (Output)
 *     InFiles = Pointer to character array holding the input files holding data
 *               to be formatted. (Output)
 * numNdfdVars = Number of NDFD and/or RTMA elements chosen on the command line
 *               arg to format. (Input)
 *    ndfdVars = Array holding the enum numbers of the elements chosen
 *               on the command line arg to format. (Input)
 * lampDataDir = Directory where LAMP data is located.  If not provided by the
 *               user, userparse.c has set this to the default value (if
 *               possible) (Input)
 *      
 * FILES/DATABASES: None
 *                
 * RETURNS: void
 *
 *  9/2009 Paul Hershberg (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
#include "xmlparse.h"
void anyLampElements(size_t *numInFiles, char ***inFiles, size_t numNdfdVars,
                     uChar *ndfdVars, char *lampDataDir)
{
   int i;                 /* Loop Counter. */
   int f_lampTstmProb = 0;
   char lampStrBuff[200]; /* String holding complete directory and file LAMP 
                           * data is found. */

   /* If numNdfdVars == 0, then -ndfdVars wasn't entered on the command line.
    * Then we format all elements, of which LAMP_TSTMPRB is, so flag it.
    */
   if (numNdfdVars == 0)
      f_lampTstmProb = 1;
   else if (numNdfdVars != 0) /* Go thru list of chosen elements. */
   {
      for (i = 0; i < numNdfdVars; i++)
      {
         if (ndfdVars[i] == LAMP_TSTMPRB)
         {
            f_lampTstmProb = 1;
            break;
         }
      }
   }

   /* Check to see if lampDataDir is null, if so, both the user supplied and 
    * default values for -lampDir are not directories, so abort 
    */
   if (lampDataDir == NULL)
      return;

      /**********************LAMP THUNDERSTORM PROB***************************/
      /***********************************************************************/

   /* If LAMP Tstm Prob in this query, create data files/directories. */
   if (f_lampTstmProb)
   {
      *numInFiles  = *numInFiles + 1;         
      sprintf (lampStrBuff, "%s%s", lampDataDir, "/conus/tstmprb.ind");
      (*inFiles) = (char **) realloc((*inFiles), *numInFiles * sizeof (char *));
      (*inFiles)[*numInFiles-1] = (char *) malloc((strlen(lampStrBuff)
                                           + 1) * sizeof (char));
      strcpy ((*inFiles)[*numInFiles-1], lampStrBuff);
   }

   return;
}
