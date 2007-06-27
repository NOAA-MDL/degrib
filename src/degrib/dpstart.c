/*****************************************************************************
 * dpstart.c
 *
 * DESCRIPTION
 *    This file contains just the code needed for probing the data cube.  Its
 * been extracted to assist with web servers that may need to have several
 * copies running simultaneously to probe various points.
 *
 * HISTORY
 *   2/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *   Following is useful for timing diagnostics:
 *      printf ("1 :: %f\n", clock() / (double) (CLOCKS_PER_SEC));
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
#include <ctype.h>
#include "commands.h"
#include "clock.h"
*/
#include "myassert.h"
#include "userparse.h"
#include "myutil.h"
#include "myerror.h"
#include "genprobe.h"
#include "sector.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

static void Usage (char *argv0, userType *usr)
{

}

/*****************************************************************************
 * main() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   The main entry point for the command line version of degrib.  Basically,
 * it parses input arguments, and then calls DegribIt.
 *
 * ARGUMENTS
 * argc = The number of arguments on the command line. (Input)
 * argv = The arguments on the command line. (Input)
 *
 * RETURNS: int
 *  0 = ok
 *  1 = error.
 *
 *  2/2006 Arthur Taylor (MDL): Created.
 *
 * NOTES:
 *****************************************************************************
 */
int main (int argc, char **argv)
{
   userType usr;        /* The user inputed options */
   char *msg;           /* Used to print the error stack. */
   int type;
   char perm;
   char *filter;
   int i, j;

   UserInit (&usr);
   if (argc < 2) {
      Usage (argv[0], &usr);
      printf ("\n");
      return 1;
   }

   /* Find out if first argument is a valid option.. */
   /* If yes then parse the command line.  If no then, ask if it is a file.
    * If it is not a file, then error, else use that file for -in option,
    * and_ parse the rest of the command line. */
   if (!IsUserOpt (argv[1])) {
      type = myStat (argv[1], &perm, NULL, NULL);
      /* check if it is a file or directory */
      if ((type != MYSTAT_ISDIR) && (type != MYSTAT_ISFILE)) {
         Usage (argv[0], &usr);
         printf ("\nError was: '%s' is not a file or directory\n\n", argv[1]);
         return 1;
      }
      /* check that it is readable */
      if (!(perm & 4)) {
         Usage (argv[0], &usr);
         printf ("\nError was: No read permissions on '%s'\n\n", argv[1]);
         return 1;
      }
      usr.numInNames = 1;
      usr.inNames = malloc (usr.numInNames * sizeof (char *));
      usr.inNames[0] = (char *) malloc ((strlen (argv[1]) + 1) *
                                        sizeof (char));
      strcpy (usr.inNames[0], argv[1]);
      usr.f_inTypes = malloc (usr.numInNames * sizeof (char));
      usr.f_inTypes[0] = type;

      for (i = 2; i < argc; i++) {
         if (!IsUserOpt (argv[i])) {
            type = myStat (argv[i], &perm, NULL, NULL);
            /* check if it is a file or directory */
            if ((type != MYSTAT_ISDIR) && (type != MYSTAT_ISFILE)) {
               Usage (argv[0], &usr);
               printf ("\nError was: '%s' is not a file or directory\n\n",
                       argv[i]);
               for (j = 0; j < usr.numInNames; j++) {
                  free (usr.inNames[j]);
               }
               free (usr.inNames);
               free (usr.f_inTypes);
               return 1;
            }
            /* check that it is readable */
            if (!(perm & 4)) {
               Usage (argv[0], &usr);
               printf ("\nError was: No read permissions on '%s'\n\n",
                       argv[i]);
               for (j = 0; j < usr.numInNames; j++) {
                  free (usr.inNames[j]);
               }
               free (usr.inNames);
               free (usr.f_inTypes);
               return 1;
            }
            usr.numInNames++;
            usr.inNames = realloc (usr.inNames,
                                   usr.numInNames * sizeof (char *));
            usr.inNames[usr.numInNames - 1] =
                  malloc ((strlen (argv[i]) + 1) * sizeof (char));
            strcpy (usr.inNames[usr.numInNames - 1], argv[i]);
            usr.f_inTypes = realloc (usr.f_inTypes,
                                     usr.numInNames * sizeof (char));
            usr.f_inTypes[usr.numInNames - 1] = type;
         } else {
            break;
         }
      }
      if (i == argc) {
         Usage (argv[0], &usr);
         printf ("\nError was: Couldn't find a command line option\n\n");
         for (j = 0; j < usr.numInNames; j++) {
            free (usr.inNames[j]);
         }
         free (usr.inNames);
         free (usr.f_inTypes);
         return 1;
      }

      if (UserParseCommandLine (&usr, argc - i, argv + i) != 0) {
         Usage (argv[0], &usr);
         msg = errSprintf (NULL);
         printf ("\nError was: %s", msg);
         free (msg);
         UserFree (&usr);
         return 1;
      }
   } else {
      if (UserParseCommandLine (&usr, argc - 1, argv + 1) != 0) {
         Usage (argv[0], &usr);
         msg = errSprintf (NULL);
         printf ("\nError was: %s", msg);
         free (msg);
         UserFree (&usr);
         return 1;
      }
   }
   /* Load options from file. */
   if (usr.cfgName != NULL) {
      if (UserParseConfigFile (&usr, usr.cfgName) != 0) {
         Usage (argv[0], &usr);
         msg = errSprintf (NULL);
         printf ("\nError was: %s", msg);
         free (msg);
         UserFree (&usr);
         return 1;
      }
   }
   /* Make sure that they have a command. */
   if (usr.f_Command == -1) {
      Usage (argv[0], &usr);
      printf ("\nPlease provide a command option.\n");
      UserFree (&usr);
      return 1;
   }

   /* Validate choices and set up defaults. */
   if (usr.f_Command == -1) {
      usr.f_Command = CMD_DATAPROBE;
   } else if (usr.f_Command == CMD_VERSION) {
      msg = Grib2About ("degrib_DP");
      printf ("%s", msg);
      free (msg);
      UserFree (&usr);
      return 0;
   } else if ((usr.f_Command != CMD_DATAPROBE) &&
              (usr.f_Command != CMD_SECTOR)) {
      Usage (argv[0], &usr);
      printf ("\nError was: Only command options with %s is -V -DP, and "
              "-Sector", argv[0]);
      UserFree (&usr);
      return 1;
   }
   if (UserValidate (&usr) != 0) {
      Usage (argv[0], &usr);
      msg = errSprintf (NULL);
      printf ("\nError was: %s", msg);
      free (msg);
      UserFree (&usr);
      return 1;
   }

   /* Convert usr.inNames from a list of dir's and file's to just files for
    * everything except -P and -DP. -P or -DP try to "tack on" the sector
    * for_ their points and filter the files based on ndfdVars when
    * converting to just files. */
   if ((usr.f_Command != CMD_PROBE) && (usr.f_Command != CMD_DATAPROBE)) {
      if (usr.gribFilter != NULL) {
         filter = usr.gribFilter;
      } else {
         if ((usr.f_Command == CMD_DATACONVERT) ||
             (usr.f_Command == CMD_DATA)) {
            filter = "*.ind";
         } else {
            filter = "*.bin";
         }
      }
      if (expand_inName (&(usr.numInNames), &(usr.inNames), &(usr.f_inTypes),
                         filter) != 0) {
         Usage (argv[0], &usr);
         msg = errSprintf (NULL);
         printf ("\nError was: %s", msg);
         free (msg);
         UserFree (&usr);
         return 1;
      }
   }

   /* Do it. */
#ifdef DEBUG
   if (!usr.f_stdout) {
      fprintf (stderr, "Timing info. %f\n", clock () /
               (double) (CLOCKS_PER_SEC));
   }
#endif
   if (usr.f_Command == CMD_SECTOR) {
      for (i = 0; i < usr.numPnt; i++) {
         if (WhichSector (usr.sectFile, usr.pnt[i], usr.f_pntType) != 0) {
            msg = errSprintf (NULL);
            printf ("ERROR: In call to Grib2DataProbe.\n%s\n", msg);
            free (msg);
         }
      }
   } else {
      if (ProbeCmd (usr.f_Command, &usr) != 0) {
         msg = errSprintf (NULL);
         printf ("ERROR: In call to GRIB2Probe.\n%s\n", msg);
         free (msg);
         UserFree (&usr);
         return -1;
      }
   }
#ifdef DEBUG
   if (!usr.f_stdout) {
      fprintf (stderr, "Timing info. %f\n", clock () /
               (double) (CLOCKS_PER_SEC));
   }
#endif

   UserFree (&usr);
   return 0;
}
