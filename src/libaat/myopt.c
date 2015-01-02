/*****************************************************************************
 * myopt.c
 *
 * DESCRIPTION
 *    This file contains some code to help interface with getopt_long()
 *
 * HISTORY
 *  6/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "libaat.h"

/*****************************************************************************
 * myUsage() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This ouputs a Gnu usage message with getopt (as opposed to argp).  For
 * more info, see:
 * http://www.cs.utah.edu/dept/old/texinfo/standards/standards.html#SEC22
 * http://www.gnu.org/software/libc/manual/html_node/Argp.html#Argp
 *
 * ARGUMENTS
 *    name = Name of the program (Input)
 * argsDoc = Documentation on how to call the program (Input)
 *     doc = Main documentation for the program.  A \v separates the text
 *           into before and after the optlist (Input)
 * optLong = The long options used with getopt_long (Input)
 * optHelp = Supplemental to optLong, describes each option.  A \v at the
 *           beginning of an optHelp[].doc means no 'long option' (Typically
 *           used as a place holder).  A 0 for optHelp[].val means no 'short
 *           option'. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 ran out of optHelp[] array elements
 *
 * HISTORY
 *  6/2007 Arthur Taylor (MDL): Created.
 *  9/2007 AAT (MDL): Modified doc to be 2d because of ISO limit of 509 char
 *
 * NOTES
 ****************************************************************************/
int myUsage(const char *name, const char *argsDoc, char *doc[],
            const struct option *optLong, const optHelpType * optHelp)
{
/*   char *docPtr;  */      /* Pointer to \v in doc (if there is one) */
   size_t len;          /* Length of current long option.  Used to determine
                         * if we can fit everything on one line. */
   char *optDoc;        /* Current long option documentation. */
   char *ptr;           /* Pointer to \n in optDoc (if there is one) */

   printf("Usage %s %s\n", name, argsDoc);

   while (*doc != NULL) {
      if ((*doc)[0] == '\v') {
         break;
      }
      puts(*doc);
      ++doc;
   }
/*
   if ((docPtr = strchr(doc, '\v')) == NULL) {
      puts(doc);
   } else {
      while (doc != docPtr) {
         putc(*(doc++), stdout);
      }
      putc('\n', stdout);
      ++doc;
   }
*/

   while ((optLong->name != NULL) && (optHelp->val != -1)) {
      /* Determine if there is a short option name. */
      if (optHelp->val == 0) {
         printf("     ");
      } else {
         printf("  -%c,", optHelp->val);
      }
      /* Determine if there is a long option name. */
      optDoc = optHelp->doc;
      if (optDoc[0] == '\v') {
         printf("   ");
         len = 0;
         ++optDoc;
      } else {
         printf(" --%s", optLong->name);
         len = strlen(optLong->name);
         ++optLong;
      }
      ++optHelp;
      /* Print the spacing needed to get to the optDoc area. */
      if (len > 21) {
         printf("\n%30s", " ");
      } else {
         printf("%*s", (21 - len), " ");
      }
      /* Print the optDoc area. */
      while ((ptr = strchr(optDoc, '\n')) != NULL) {
         while (optDoc != ptr) {
            putc(*(optDoc++), stdout);
         }
         printf("\n%31s", " ");
         ++optDoc;
      }
      printf("%s\n", optDoc);
   }

   if (optLong->name != NULL) {
      myWarn_Err1Arg("Need more elements in optHelp[] array\n");
      return -1;
   }

   if (*doc != NULL) {
      puts((*doc) + 1);
      ++doc;
   }
   while (*doc != NULL) {
      puts(*doc);
      ++doc;
   }
/*
   if (docPtr != NULL) {
      puts(doc);
   }
*/
   return 0;
}

/*****************************************************************************
 * swap1() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Take item in location st, store it in location pl, and move items
 * that were in range of [pl to (st - 1)] to [pl + 1 to st]
 *
 * ARGUMENTS
 * argv = The command line options to permute (Input/Output)
 *   st = Location of item to shift. (Input)
 *   pl = Destination of item to shift. (Input)
 *
 * RETURNS: void
 *
 * HISTORY
 *  8/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *    Side effect: argv is permuted.
 ****************************************************************************/
static void swap1(char **argv, int st, int pl)
{
   char *t1;            /* temporary storage. */
   int i;               /* Used to loop over argv */

#ifdef DEBUG
   if (st <= pl) {
      fprintf(stderr, "Bad usage?\n");
      return;
   }
#endif
   t1 = argv[st];
   for (i = st - 1; i >= pl; --i) {
      argv[i + 1] = argv[i];
   }
   argv[pl] = t1;
}

/*****************************************************************************
 * swap2() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Take items in location st, st + 1, store it in location pl, pl + 1 and
 * move items that were in [pl to (st - 1)] to [pl + 2] to (st + 1)].
 *
 * ARGUMENTS
 * argv = The command line options to permute (Input/Output)
 *   st = Location of item to shift. (Input)
 *   pl = Destination of item to shift. (Input)
 *
 * RETURNS: void
 *
 * HISTORY
 *  8/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *    Side effect: argv is permuted.
 ****************************************************************************/
static void swap2(char **argv, int st, int pl)
{
   char *t1;            /* temporary storage. */
   char *t2;            /* temporary storage. */
   int i;               /* Used to loop over argv */

#ifdef DEBUG
   if (st <= pl) {
      fprintf(stderr, "Bad usage?\n");
      return;
   }
#endif
   t1 = argv[st];
   t2 = argv[st + 1];
   for (i = st - 1; i >= pl; --i) {
      argv[i + 2] = argv[i];
   }
   argv[pl] = t1;
   argv[pl + 1] = t2;
}

/*****************************************************************************
 * myGetOpt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Decode options from argv.  A -- indicates a long option, or end of
 * options.  A - indicated first a long option.  If it doesn't match the long
 * options, then the - indicates a set of short options.  For example, '-foo'
 * means first look for long option "foo", then short options 'f', 'o', 'o'.
 *    If a option has an argument, a pointer to it is stored in optarg.
 *    After all options are found, gl.optind points to the next index in argv.
 *
 * For more info see:
 *   http://www.gnu.org/software/libc/manual/html_node/Getopt.html
 * Similar to getopt_long_only except it doesn't use global variables.
 *
 * Uses 2 static variables...
 *    f_first : true for first call (so it init gl.optind to 1) false after.
 *    nextChar : location in short option we are working on.
 *
 * ARGUMENTS
 *     argc = The number of command line options (Input)
 *     argv = The command line options (Input)
 * optShort = The short options.  A ':' means a required argument, a '::'
 *            means an optional argument. (Input)
 *  optLong = The long options (Input)
 *       gl = The return values: (Output)
 *            (char *) optarg => any arguments associated with option.
 *            (int) optind => index to point after options in argv
 *            (int) optopt => value of bad option if error with optShort
 *            (int) index => index value of current option in optLong or -1
 *
 * RETURNS: int
 *   -1 = found all options.
 *    0 = Set a long option flag (optLong.flag != NULL),
 *        or optLong.flag == NULL, and optLong.val = 0.
 *   '?' = unrecognized option.
 *   1..255 = val of the short option, or if optLong.flag == NULL then value
 *            of the optLong.val.
 *
 * HISTORY
 *  8/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 * 1) Side effect: argv can get permuted.
 ****************************************************************************/
int myGetOpt(int argc, char *const *argv, const char *optShort,
             const struct option *optLong, struct getOptRet *gl)
{
   int firstNonopt;     /* First non-option found in the argv list. */
   int f_type;          /* 0 if non-option, 1 if one -, 2 if two --. */
   int f_optFound;      /* True if we have matched a long option. */
   char *ptr;           /* ptr to end of (long) option or location in short
                         * option list of match. */
   const struct option *p; /* Helps walk through optLong list. */
   int ans;             /* The value to return */
   static int nextChar = 0; /* Next short option character to parse. */
   static char f_first = 1; /* Used to init optind. */

   /* Set up default return values. */
   gl->optarg = NULL;
   if (f_first) {
      gl->optind = 1;
      f_first = 0;
   }
   gl->index = -1;
   gl->optopt = 0;

   /* Search for an option */
   firstNonopt = -1;
   f_type = 0;
   for (; gl->optind < argc; ++gl->optind) {
      /* See if we have an option. */
      if ((argv[gl->optind][0] == '-') && (argv[gl->optind][1] != '\0')) {
         /* See if we have a long option. */
         if (argv[gl->optind][1] == '-') {
            f_type = 2;
         } else {
            /* Found short (but possibly long) option. */
            f_type = 1;
         }
         break;
      } else {
         /* Found non-option. */
         if (firstNonopt == -1) {
            firstNonopt = gl->optind;
         }
      }
   }

   /* Handle the cases where we either didn't have an option, or have hit the
    * end of the options. */
   if (f_type == 2) {
      /* see if we have the end of options. */
      if ((argv[gl->optind][2] == '\0')) {
         ans = -1;
         if (firstNonopt != -1) {
            /* insert gl->optind to just before firstNonopt */
            swap1((char **)argv, gl->optind, firstNonopt);
            gl->optind = firstNonopt;
         }
         gl->optind++;
         return ans;
      } else if (optLong == NULL) {
         /* Error because there are no long options, but we have one. */
         if (gl->opterr) {
            fprintf(stderr, "unknown option %s\n", argv[gl->optind]);
         }
         gl->optopt = '-';
         ans = '?';
         if (firstNonopt != -1) {
            /* insert gl->optind to just before firstNonopt */
            swap1((char **)argv, gl->optind, firstNonopt);
            gl->optind = firstNonopt;
         }
         gl->optind++;
         return ans;
      }
   } else if (f_type == 0) {
      /* We didn't see any options. */
      if (firstNonopt != -1) {
         gl->optind = firstNonopt;
      }
      return -1;
   }

   /* Handle the long option if we have one. */
   if ((optLong != NULL) && (nextChar == 0)) {
      /* Find end of possible long option name ('\0' or '='). */
      for (ptr = argv[gl->optind] + f_type; *ptr && *ptr != '='; ++ptr) {
      }

      /* Seach long option list to see if we have an identical match. */
      f_optFound = 0;
      for (p = optLong, gl->index = 0; p->name; ++p, ++gl->index) {
         if (!strncmp(p->name, argv[gl->optind] + f_type,
                      ptr - (argv[gl->optind] + f_type))) {
            if ((unsigned int)(ptr - (argv[gl->optind] + f_type)) ==
                (unsigned int)strlen(p->name)) {
               /* Identical match */
               f_optFound = 1;
               break;
            }
         }
      }

      /* Handle option and return correct value. */
      if (f_optFound) {
         /* Determine the correct return values. */
         if (p->flag != NULL) {
            *(p->flag) = p->val;
            ans = 0;
         } else {
            ans = p->val;
         }

         /* Handle any arguments. */
         if (p->has_arg == no_argument) {
         } else if (*ptr == '=') {
            gl->optarg = ptr + 1;
         } else if (gl->optind + 1 >= argc) {
            /* Argument not found? */
            if (p->has_arg == required_argument) {
               if (gl->opterr) {
                  fprintf(stderr, "option requres an argument -- %s\n",
                          p->name);
               }
               ans = '?';
            }
         } else {
            gl->optarg = argv[gl->optind + 1];
            if (firstNonopt != -1) {
               /* insert gl->optind to just before firstNonopt */
               swap2((char **)argv, gl->optind, firstNonopt);
               gl->optind = firstNonopt;
            }
            /* Following is to use up the option and argument */
            gl->optind += 2;
            return ans;
         }
         if (firstNonopt != -1) {
            /* insert gl->optind to just before firstNonopt */
            swap1((char **)argv, gl->optind, firstNonopt);
            gl->optind = firstNonopt;
         }
         gl->optind++;
         return ans;
      }

      /* Determine if there is no way to have a short option. */
      if (f_type == 2) {
         if (gl->opterr) {
            fprintf(stderr, "unknown option -- %s\n", argv[gl->optind] + 2);
         }
         gl->optind++;
         return '?';
      }
   }

   /* We are now dealing with a short option. */
   if ((ptr = strchr(optShort, argv[gl->optind][1 + nextChar])) == NULL) {
      if (gl->opterr) {
         fprintf(stderr, "unknown option - %c\n",
                 argv[gl->optind][1 + nextChar]);
      }
      gl->optopt = argv[gl->optind][1 + nextChar];
      nextChar++;
      if (argv[gl->optind][1 + nextChar] == '\0') {
         nextChar = 0;
         gl->optind++;
      }
      return '?';
   }

   /* Found short option. */
   ans = *ptr;
   /* Deal with optional / required arguments */
   if (*(ptr + 1) == ':') {
      if (argv[gl->optind][2 + nextChar] != '\0') {
         gl->optarg = argv[gl->optind] + 2 + nextChar;
      } else if (gl->optind + 1 >= argc) {
         /* Argument not found? */
         if (*(ptr + 2) != ':') {
            if (gl->opterr) {
               fprintf(stderr, "option requres an argument -- %c\n", ans);
            }
            ans = '?';
         }
      } else {
         gl->optarg = argv[gl->optind + 1];
         if (firstNonopt != -1) {
            /* insert gl->optind to just before firstNonopt */
            swap2((char **)argv, gl->optind, firstNonopt);
            gl->optind = firstNonopt;
         }
         /* Following is to use up the option and argument */
         gl->optind += 2;
         nextChar = 0;
         return ans;
      }
      if (firstNonopt != -1) {
         /* insert gl->optind to just before firstNonopt */
         swap1((char **)argv, gl->optind, firstNonopt);
         gl->optind = firstNonopt;
      }
      gl->optind++;
      nextChar = 0;
      return ans;
   }
   /* Increment to next location in short option list. */
   nextChar++;
   if (argv[gl->optind][1 + nextChar] == '\0') {
      nextChar = 0;
      if (firstNonopt != -1) {
         /* insert gl->optind to just before firstNonopt */
         swap1((char **)argv, gl->optind, firstNonopt);
         gl->optind = firstNonopt;
      }
      gl->optind++;
   } else {
      /* Since we didn't swap, we want to make sure that we don't skip
       * any early non-options when we go back to look at more short options
       */
      gl->optind = firstNonopt;
   }
   return ans;
}

/*****************************************************************************
 * main() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To test the myGetOpt routines, to make sure that they function correctly
 * Focused on long options.
 *
 * ARGUMENTS
 * argc = The number of arguments on the command line. (Input)
 * argv = The arguments on the command line. (Input)
 *
 * RETURNS: int
 *
 * HISTORY
 *  8/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
#ifdef DEBUG_MYOPT_LONG
int main(int argc, char **argv)
{
   int c;
   /* Flag set by `--verbose'. */
   static int verbose_flag;
   static char optShort[] = "abc:d:f:";
   static struct option optLong[] = {
      /* These options set a flag. */
      {"verbose", no_argument, &verbose_flag, 1},
      {"brief", no_argument, &verbose_flag, 0},
      /* These options don't set a flag. We distinguish them by their
       * indices. */
      {"add", no_argument, 0, 'a'},
      {"append", no_argument, 0, 'b'},
      {"delete", required_argument, 0, 'd'},
      {"create", required_argument, 0, 'c'},
      {"file", required_argument, 0, 'f'},
      {0, 0, 0, 0}
   };
   struct getOptRet getOp;

   getOp.opterr = 1;
   while ((c = myGetOpt(argc, argv, optShort, optLong, &getOp)) != -1) {
      switch (c) {
         case 0:
            /* If this option set a flag, do nothing else now. */
            if (optLong[getOp.index].flag != NULL) {
               break;
            }
            printf("option %s", optLong[getOp.index].name);
            if (getOp.optarg) {
               printf(" with arg %s", getOp.optarg);
            }
            printf("\n");
            break;
         case 'a':
            puts("option -a\n");
            break;
         case 'b':
            puts("option -b\n");
            break;
         case 'c':
            printf("option -c with value `%s'\n", getOp.optarg);
            break;
         case 'd':
            printf("option -d with value `%s'\n", getOp.optarg);
            break;
         case 'f':
            printf("option -f with value `%s'\n", getOp.optarg);
            break;
         case '?':
            /* getopt_long already printed an error message. */
            break;
         default:
            abort();
      }
   }
   if (verbose_flag) {
      puts("verbose flag is set");
   }
   /* Print any remaining command line arguments (not options). */
   if (getOp.optind < argc) {
      printf("non-option ARGV-elements: ");
      while (getOp.optind < argc) {
         printf("%s ", argv[getOp.optind++]);
      }
      putchar('\n');
   }
   exit(0);
}
#endif

/*****************************************************************************
 * main() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To test the myGetOpt routines, to make sure that they function correctly
 * Focused on short options.
 *
 * ARGUMENTS
 * argc = The number of arguments on the command line. (Input)
 * argv = The arguments on the command line. (Input)
 *
 * RETURNS: int
 *
 * HISTORY
 *  8/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
#ifdef DEBUG_MYOPT_SHORT
int main (int argc, char **argv)
{
   int aflag = 0;
   int bflag = 0;
   char *cvalue = NULL;
   int index;
   int c;
   struct getOptRet getOp;

   getOp.opterr = 0;
   while ((c = myGetOpt(argc, argv, "abc:", NULL, &getOp)) != -1) {
      switch (c) {
         case 'a':
            aflag = 1;
            break;
         case 'b':
            bflag = 1;
            break;
         case 'c':
            cvalue = getOp.optarg;
            break;
         case '?':
            if (getOp.optopt == 'c')
               fprintf (stderr, "Option -%c requires an argument.\n",
                        getOp.optopt);
            else if (isprint (getOp.optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", getOp.optopt);
            else
               fprintf (stderr, "Unknown option character `\\x%x'.\n",
                        getOp.optopt);
            return 1;
         default:
            abort ();
      }
   }
   printf ("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue);

   for (index = getOp.optind; index < argc; index++)
      printf ("Non-option argument %s\n", argv[index]);
   return 0;
}
#endif
