#ifndef MYOPT_H
#define MYOPT_H

#define no_argument 0
#define required_argument 1
#define optional_argument 2
struct option {
   const char *name;    /* Name of the long option. */
   int has_arg;         /* What kind of argument requirements. */
   int *flag;           /* NULL => return val. */
                        /* != NULL => set this pointer to val, and return 0. */
   int val;             /* value to store if seen. */
};

typedef struct {
   int val;             /* 0 or Short version option */
   char *doc;           /* documentation for the option. */
} optHelpType;

struct getOptRet {
   int opterr;          /* Set by caller. 0 means no error printing
                         * (typically != 0). */
   char *optarg;        /* The optional argument value */
   int optind;          /* Index to point after all options in argv. */
   int optopt;          /* The unrecognized short option. */
   int index;           /* Index value of current option in optLong or -1 */
};

int myUsage(const char *name, const char *argsDoc, char *doc[],
            const struct option *optLong, const optHelpType * optHelp);
int myGetOpt(int argc, char *const *argv, const char *optShort,
             const struct option *optLong, struct getOptRet *gl);

#endif
