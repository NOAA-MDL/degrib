/*****************************************************************************
 * user.c
 *
 * DESCRIPTION
 *    This file contains the code needed to parse the command line options
 *
 * HISTORY
 *  8/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libaat.h"
#include "xyzuser.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

/* A description of the arguments we accept. */
static char argsDoc[] = "[OPTION]... [INFILE] [OUTFILE]";

/* Program documentation. */
static char *doc[] = {
      "Convert a comma delimited ASCII xyz file to a point .shp file.\n",
      NULL
};

/*****************************************************************************
 * usrInit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This initializes the user data structure
 *
 * ARGUMENTS
 * usr = The user data structure to initialize (Output)
 *
 * RETURNS: void
 *
 * HISTORY
 *  8/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
static void usrInit(usrType * usr)
{
   usr->f_filter = 0;
   usr->input = NULL;
   usr->output = NULL;
}

/*****************************************************************************
 * usrFree() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This frees the user data structure
 *
 * ARGUMENTS
 * usr = The user data structure to free (Output)
 *
 * RETURNS: void
 *
 * HISTORY
 *  8/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
void usrFree(usrType * usr)
{
   if (usr->input != NULL) {
      free(usr->input);
   }
   if (usr->output != NULL) {
      free(usr->output);
   }
}

/*****************************************************************************
 * usrParse() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This initilizes its copy of the usrType structure, then parses the argc,
 * argv command line, and passes a pointer to that data to the caller.
 *    This is bad form, but we can do it since the local copy is static, and
 * there is only one usrType per instance.
 *
 * ARGUMENTS
 *     Usr = Pointer to the completed user data structure (Output)
 *    argc = Number of command line arguments (Input)
 *    argv = Command line arguments (Input)
 * pkgName = Name of the program (Input)
 * pkgVers = Version number of the program (Input)
 * pkgDate = Date of the program (Input)
 *
 * RETURNS: int
 *  -1 if we ran out of memory
 *  -2 if we saw an unknown option
 *  -3 if we had an unhandled option
 *  -4 if we had an unhandled argument
 *  -5 if we didn't have enough arguments 
 *   1 if we handleded --help, --version,-V so we should quit now.
 *   0 if everything was ok.
 *
 * HISTORY
 *  6/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
int usrParse(usrType ** Usr, int argc, char **argv, char *pkgName,
             char *pkgVers, char *pkgDate)
{
   static int f_help;   /* Used to determine the type of help. */
   static usrType usr;  /* Used for local usr structure */

   /* optShort are the short form options, a ':' => required option, while a
    * '::' => optional option */
   static char optShort[] = "Vf";

   /* optLong are the long form options */
   static struct option optLong[] = {
      {"help", no_argument, &f_help, 1},
      {"version", no_argument, &f_help, 2},
      {"filter", required_argument, NULL, 'f'},
      {NULL, 0, NULL, 0}
   };

   /* optHelp is a descriptive supplement to optLong.  For no short opt, use
    * 0 for val.  For no long option, use \v for first char in doc[]. */
   static optHelpType optHelp[] = {
      {0, "Display this help and exit."}, /* help */
      {'V', "Output version information and exit."}, /* version */
      {'f', "Filter identical lat/lon, keeping the last one seen"}, /* filter */
      {-1, NULL}
   };

   int c;               /* The current option. */
   int ierr;            /* The current error code to return */
   struct getOptRet getOp; /* The "global" variables for getopt_long. */

   getOp.opterr = 0;
   ierr = 0;
   *Usr = &usr;
   usrInit(&usr);
   while ((c = myGetOpt(argc, argv, optShort, optLong, &getOp)) != -1) {
      switch (c) {
         case 0:
            break;
         case 'V':
            f_help = 2;
            break;
         case 'f':
            usr.f_filter = 1;
            break;
         case '?':
            printf("unknown option '%c'\n", getOp.optopt);
            ierr = -2;
            break;
         default:
            myWarn_Err2Arg("Unhandled option '%c'\n", c);
            return -3;
      }
   }

   /* Handle --help, -V,--version options */
   if (f_help == 1) {
      myUsage(pkgName, argsDoc, doc, optLong, optHelp);
      return 1;
   } else if (f_help == 2) {
      printf("%s\nVersion: %s\nDate: %s\nCompile Date: %s\n"
             "Author: Arthur Taylor\n", pkgName, pkgVers, pkgDate, __DATE__);
      return 1;
   }

   /* Handle any extra arguments. */
   while (getOp.optind < argc) {
      if (usr.input == NULL) {
         usr.input = (char *)malloc((strlen(argv[getOp.optind]) + 1) *
                                    sizeof(char));
         if (usr.input == NULL) {
            myWarn_Err1Arg("Ran out of memory\n");
            return -1;
         }
         strcpy(usr.input, argv[getOp.optind++]);
      } else if (usr.output == NULL) {
         usr.output = (char *)malloc((strlen(argv[getOp.optind]) + 1) *
                                    sizeof(char));
         if (usr.output == NULL) {
            myWarn_Err1Arg("Ran out of memory\n");
            return -1;
         }
         strcpy(usr.output, argv[getOp.optind++]);
      } else {
         myWarn_Err2Arg("Unhandled argument '%s'\n", argv[getOp.optind++]);
         ierr = -4;
      }
   }
   if (usr.output == NULL) {
      myUsage(pkgName, argsDoc, doc, optLong, optHelp);
      return 1;
   }
   return ierr;
}
