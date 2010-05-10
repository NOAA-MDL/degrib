/******************************************************************************
 * rtmaFileNames() --
 *
 * Paul Hershberg / MDL
 * Linux
 *
 * PURPOSE
 *  This code creates the directories/files where the RTMA data resides and
 *  updates the inFiles array and numInFiles accordingly.
 *
 * ARGUMENTS
 *    numInFiles = Number of input files. (Output)
 *       InFiles = Pointer to character array holding the input files holding data
 *                 to be formatted. (Output)
 * directoryTail = Tail portion of directory RTMA data is found in. (Input)
 *   rtmaDataDir = Directory where RTMA data is located.  If not provided by the
 *                 user, userparse.c has set this to the default value (if
 *                 possible) (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   3/2008 Paul Hershberg (MDL): Created
 *
 * NOTES
 ******************************************************************************
 */
#include "xmlparse.h"
void rtmaFileNames(size_t *numInFiles, char ***inFiles, char *directoryTail, 
                   char *rtmaDataDir)
{
   int i;                 /* Counter. */
   char rtmaStrBuff[200]; /* String holding complete directory and file RTMA 
                           * data is found. */
   int numRtmaHrs = 24;   /* Number of prior RTMA hours we are processing. */

   for (i = 0; i < numRtmaHrs; i++)
   {
      *numInFiles  = *numInFiles + 1;

      if (i >= 10)
         sprintf (rtmaStrBuff, "%s%s%d%s", rtmaDataDir, directoryTail, i, 
                  ".ind");
      else
         sprintf (rtmaStrBuff, "%s%s%s%d%s", rtmaDataDir, directoryTail, "0", i,
                  ".ind");

      (*inFiles) = (char **) realloc((*inFiles), ((*numInFiles) * 
                             sizeof (char *)));
      ((*inFiles)[*numInFiles-1]) = (char *) malloc((strlen(rtmaStrBuff) + 1)
                                             * sizeof (char));
      strcpy (((*inFiles)[*numInFiles-1]), rtmaStrBuff);
   }

   return;
}
