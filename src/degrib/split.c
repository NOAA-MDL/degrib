/*****************************************************************************
 * split.c
 *
 * DESCRIPTION
 *    This file contains the code needed to split the GRIB file into its
 * component messages.
 *
 * HISTORY
 *   5/2010 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "split.h"
#include "degrib2.h"
#include "myerror.h"

/*****************************************************************************
 * GRIB2Split() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Splits a file into its component GRIB messages (doesn't break up
 * subgrids).
 *
 * ARGUMENTS
 * filename = File to split. (Input)
 *   msgNum = Which message to look for (0 all, value otherwise). (Input)
 *   curMsg = Number of messages we've already looked at.
 *            In the procedured is the current message we are on. (Input)
 *
 * FILES/DATABASES:
 *    Opens a GRIB2 file for reading given its filename.
 *
 * RETURNS: int
 * +# = number of GRIB2 messages in the file.
 * -1 = Problems opening file for read.
 *
 * HISTORY
 *   5/2010 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
int GRIB2Split (char *filename, int msgNum, int curMsg)
{
   int grib_limit;      /* How many bytes to look for before the first "GRIB"
                         * in the file.  If not found, is not a GRIB file. */
   FILE *fp;            /* The opened GRIB2 file. */
   sInt4 offset = 0;    /* Where we are in the file. */
   char *ptr;           /* used to find the file extension. */
   char *buff;          /* Holds the info between records. */
   uInt4 buffLen;       /* Length of info between records. */
   int c;               /* Determine if end of the file without fileLen. */
   sInt4 sect0[SECT0LEN_WORD]; /* Holds the current Section 0. */
   uInt4 gribLen;       /* Length of the current GRIB message. */
   int version;         /* Which version of GRIB is in this message. */
   char *msg;           /* Used to pop messages off the error Stack. */
   sInt4 fileLen;       /* Length of the GRIB2 file. */
   char *outName = NULL; /* Name of the output file. */
   FILE *op;            /* The opened output file. */
   int i;               /* loop counter while writing bytes to output file. */

   grib_limit = GRIB_LIMIT;
   if (filename != NULL) {
      if ((fp = fopen (filename, "rb")) == NULL) {
         errSprintf ("ERROR: Problems opening %s for read.", filename);
         return -1;
      }
      ptr = strrchr (filename, '.');
      if (ptr != NULL) {
         if (strcmp (ptr, ".tar") == 0) {
            grib_limit = 5000;
         }
      }
      outName = (char *) malloc (strlen (filename) + 1 + 11);
   } else {
      fp = stdin;
      outName = (char *) malloc (strlen ("split.grb") + 1 + 11);
   }

   buff = NULL;
   buffLen = 0;
   while ((c = fgetc (fp)) != EOF) {
      ungetc (c, fp);
      /* curMsg++ done first so any error messages range from 1..n, instead
       * of 0.. n-1. Note curMsg should end up as n not (n-1) */
      curMsg++;
      /* Allow  2nd, 3rd, etc messages to have no limit to finding "GRIB". */
      if (curMsg > 1) {
         grib_limit = -1;
      }
      /* Read in the wmo header and sect0. */
      if (ReadSECT0 (fp, &buff, &buffLen, grib_limit, sect0, &gribLen,
                     &version) < 0) {
         if (curMsg == 1) {
            /* Handle case where we couldn't find 'GRIB' in the message. */
            preErrSprintf ("Inside GRIB2Split, Message # %d\n", curMsg);

            free (buff);
            free (outName);
            fclose (fp);
            return -2;
         } else {
            /* Handle case where there are trailing bytes. */
            msg = errSprintf (NULL);
            printf ("Warning: Inside GRIB2Split, Message # %d\n",
                    curMsg);
            printf ("%s", msg);
            free (msg);
            /* find out how big the file is. */
            fseek (fp, 0L, SEEK_END);
            fileLen = ftell (fp);
            /* fseek (fp, 0L, SEEK_SET); */
            printf ("There were %ld trailing bytes in the file.\n",
                    fileLen - offset);
            curMsg --;

            free (buff);
            free (outName);
            fclose (fp);
            return curMsg;
         }
      }

      if (version == -1) {
         /* TDLPack uses 4 bytes for FORTRAN record size, then another 8
          * bytes for the size of the record (so FORTRAN can see it), then
          * the data rounded up to an 8 byte boundary, then a trailing 4
          * bytes for a final FORTRAN record size.  However it only stores
          * in_ the gribLen the non-rounded amount, so we need to take care
          * of the rounding, and the trailing 4 bytes here. */
         gribLen = ((sInt4) ceil (gribLen / 8.0)) * 8 + 4;
      }

      /* Write to file from buffLen to buffLen + gribLen bytes. */
      if ((msgNum == 0) || (curMsg == msgNum)) {
         if (filename != NULL) {
            sprintf (outName, "%s.%d", filename, curMsg);
         } else {
            sprintf (outName, "split.grb.%d", curMsg);
         }
         if ((op = fopen (outName, "wb")) == NULL) {
            errSprintf ("ERROR: Problems opening %s for write.", outName);

            free (buff);
            free (outName);
            fclose (fp);
            return -1;
         }
         fseek (fp, offset + buffLen, SEEK_SET);
         for (i = 0; i < gribLen; i++) {
            if ((c = getc(fp)) == EOF) {
               errSprintf ("ERROR: Reached end of file too soon?");

               free (buff);
               free (outName);
               fclose (fp);
               return -1;
            }
            putc (c, op);
         }
         fclose (op);
      }

      /* Continue on to the next GRIB2 message. */
      offset += buffLen + gribLen;
      fseek (fp, offset, SEEK_SET);
   }

   free (buff);
   free (outName);
   fclose (fp);
   return curMsg;
}
