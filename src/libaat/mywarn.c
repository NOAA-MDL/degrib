/*****************************************************************************
 * mywarn.c
 *
 * DESCRIPTION
 *    This file contains the code to provide a warning handler.
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 * Originally this was part of "myerror.c"
 ****************************************************************************/
#include "libaat.h"

typedef struct {
   /* Following flags are as follows: 0=don't output here, 1=notes+warn+err,
    * 2=warn+err, 3(or more)=err. */
   uChar f_stdout;
   uChar f_stderr;
   uChar f_mem;
   uChar f_log;
   /* Stores the current built message. */
   char *buff;          /* Stores the current built up message. */
   size_t buffLen;      /* Allocated length of buff. */
   /* Stores a possible log file pointer. */
   FILE *fp;
} warnType;

static warnType warn = { 0, 1, 0, 0, NULL, 0, NULL };

/*****************************************************************************
 * myWarnSet() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This sets the parameters for the warning module.
 *
 * All errors to stderr (DEFAULT): myWarnSet(0, 1, 0, 0, NULL);
 * All errors to stdout: myWarnSet(1, 0, 0, 0, NULL);
 *
 * ARGUMENTS
 *    Following 4 flags are as follows: 0=don't output, 1=notes+warn+err,
 *                                      2=warn+err, 3(or more)=err.
 * f_stdout = flag for when to output to stdout (Input)
 * f_stderr = flag for when to output to stderr (Input)
 *    f_mem = flag for when to output to memory buffer (Input)
 *    f_log = flag for when to output to log (file) (Input)
 *  logFile = Opened file to write log messages to (or NULL) (Input)
 *
 * RETURNS: void
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
void myWarnSet(uChar f_stdout, uChar f_stderr, uChar f_mem, uChar f_log,
               FILE *logFile)
{
   warn.f_stdout = (f_stdout <= 3) ? f_stdout : 3;
   warn.f_stderr = (f_stderr <= 3) ? f_stderr : 3;
   warn.f_mem = (f_mem <= 3) ? f_mem : 3;
   warn.fp = logFile;
   if (logFile == NULL) {
      warn.f_log = 0;
   } else {
      warn.f_log = (f_log <= 3) ? f_log : 3;
   }
}

/*****************************************************************************
 * myWarnClear() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This clears the warning module.  It closes the logfile (if requested),
 * returns what is in memory, and resets memory to NULL.
 *
 * ARGUMENTS
 * f_closeFile = true if we should close the log file. (Input)
 *         msg = Any memory stored in the warning module (Output)
 *
 * RETURNS: void
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
void myWarnClear(uChar f_closeFile, char **msg)
{
   *msg = warn.buff;
   warn.buff = NULL;
   warn.buffLen = 0;
   if (f_closeFile) {
      fclose(warn.fp);
   }
}

/*****************************************************************************
 * _myWarn() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This prints a warning message of level "f_errCode" to the devices that
 * are allowed to receive those levels of warning messages.
 *
 * ARGUMENTS
 * f_errCode = 1=note, 2=warning, 3=error. (Input)
 *      file = File of initial call to myWarn module (or NULL). (Input)
 *   lineNum = Line number of inital call to myWarn module. (Input)
 *       fmt = Format to define how to print the msg (Input)
 *        ap = The arguments for the message. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf or fprintf had problems
 *   -2 allocSprintf had problems
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
static int _myWarn(uChar f_errCode, const char *file, int lineNum,
                   const char *fmt, va_list ap)
{
   int ierr = 0;        /* Error return code */

   if (fmt == NULL) {
      return ierr;
   }
   /* Check if the warnDetail level allows this message. */
   if (warn.f_stdout && (warn.f_stdout <= f_errCode)) {
      if (file != NULL) {
         if (fprintf(stdout, "(%s line %d) ", file, lineNum) < 0) {
            ierr = -1;
         }
      }
      if (vfprintf(stdout, fmt, ap) < 0) {
         ierr = -1;
      }
      fflush(stdout);
   }
   if (warn.f_stderr && (warn.f_stderr <= f_errCode)) {
      if (file != NULL) {
         if (fprintf(stderr, "(%s line %d) ", file, lineNum) < 0) {
            ierr = -1;
         }
      }
      if (vfprintf(stderr, fmt, ap) < 0) {
         ierr = -1;
      }
      fflush(stderr);
   }
   if (warn.f_log && (warn.f_log <= f_errCode)) {
      if (file != NULL) {
         if (fprintf(warn.fp, "(%s line %d) ", file, lineNum) < 0) {
            ierr = -1;
         }
      }
      if (vfprintf(warn.fp, fmt, ap) < 0) {
         ierr = -1;
      }
      fflush(warn.fp);
   }
   if (warn.f_mem && (warn.f_mem <= f_errCode)) {
      if (file != NULL) {
         if (reallocSprintf(&warn.buff, "(%s line %d) ", file, lineNum) < 0) {
            ierr = -2;
         }
      }
      if (allocSprintf(&warn.buff, &warn.buffLen, fmt, ap) != 0) {
         ierr = -2;
      }
   }
   return ierr;
}

/*****************************************************************************
 * myWarn_Note() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This prints a warning message of level "1=Note" to the devices that are
 * allowed to receive those levels of warning messages.
 *
 * ARGUMENTS
 * fmt = Format to define how to print the msg (Input)
 * ... = The actual message arguments. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf had problems
 *   -2 allocSprintf had problems
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
int myWarn_Note(const char *fmt, ...)
{
   va_list ap;          /* Contains the data needed by fmt. */
   int ierr = 0;        /* Error return code */

   if (fmt == NULL) {
      return ierr;
   }
   va_start(ap, fmt);   /* make ap point to 1st unnamed arg. */
   ierr = _myWarn(1, NULL, 0, fmt, ap);
   va_end(ap);          /* clean up when done. */
   return ierr;
}

/*****************************************************************************
 * myWarn_Warn() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This prints a warning message of level "2=Warn" to the devices that are
 * allowed to receive those levels of warning messages.
 *
 * ARGUMENTS
 * fmt = Format to define how to print the msg (Input)
 * ... = The actual message arguments. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf had problems
 *   -2 allocSprintf had problems
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
int myWarn_Warn(const char *fmt, ...)
{
   va_list ap;          /* Contains the data needed by fmt. */
   int ierr = 0;        /* Error return code */

   if (fmt == NULL) {
      return ierr;
   }
   va_start(ap, fmt);   /* make ap point to 1st unnamed arg. */
   ierr = _myWarn(2, NULL, 0, fmt, ap);
   va_end(ap);          /* clean up when done. */
   return ierr;
}

/*****************************************************************************
 * myWarn_Err() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This prints a warning message of level "3=Err" to the devices that are
 * allowed to receive those levels of warning messages.
 *
 * ARGUMENTS
 * fmt = Format to define how to print the msg (Input)
 * ... = The actual message arguments. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf had problems
 *   -2 allocSprintf had problems
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
int myWarn_Err(const char *fmt, ...)
{
   va_list ap;          /* Contains the data needed by fmt. */
   int ierr = 0;        /* Error return code */

   if (fmt == NULL) {
      return ierr;
   }
   va_start(ap, fmt);   /* make ap point to 1st unnamed arg. */
   ierr = _myWarn(3, NULL, 0, fmt, ap);
   va_end(ap);          /* clean up when done. */
   return ierr;
}

/*****************************************************************************
 * myWarn_Loc() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This allows us to create a set of macros which will provide the filename
 * and line number to myWarn at various warning levels.  This should allow one
 * to switch from:
 * myWarn_Err("(%s line %d) Test: Ran out of memory\n", __FILE__, __LINE__);
 * to:
 * myWarn_Err1ARG("Test: Ran out of memory\n");
 * myWarn_Err2ARG("Test: Ran out of memory %d\n", value);
 * ...
 *
 * ARGUMENTS
 *     fmt = Format to define how to print the msg (Input)
 *    file = File of initial call to myWarn module (or NULL). (Input)
 * lineNum = Line number of inital call to myWarn module. (Input)
 *     ... = The actual message arguments. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf had problems
 *   -2 allocSprintf had problems
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
int myWarn_Loc(uChar f_errCode, const char *file, int lineNum,
               const char *fmt, ...)
{
   va_list ap;          /* Contains the data needed by fmt. */
   int ierr = 0;        /* Error return code */

   if (fmt == NULL) {
      return ierr;
   }
   va_start(ap, fmt);   /* make ap point to 1st unnamed arg. */
   ierr = _myWarn(f_errCode, file, lineNum, fmt, ap);
   va_end(ap);          /* clean up when done. */
   return ierr;
}

/*****************************************************************************
 * The following 1 procedure is included only to test mywarn.c, and only if
 * DEBUG_MYWARN is defined.
 ****************************************************************************/

/*****************************************************************************
 * main() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   To test the mywarn module functions.
 *
 * ARGUMENTS
 * argc = The number of arguments on the command line. (Input)
 * argv = The arguments on the command line. (Input)
 *
 * RETURNS: int
 *
 * HISTORY
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
#ifdef DEBUG_MYWARN
int main(int argc, char **argv)
{
   char *msg;           /* pointer to returned memory from warn module. */
   FILE *log;           /* Open log file pointer to hand to warn module. */

   log = fopen("logfile.txt", "wt");
   myWarnSet(1, 1, 2, 3, log);

   myWarn_Note("Hello %f\n", 3.1415);
   myWarn_Note("Hello %f %s %d\n", 3.1415, __FILE__, __LINE__);
   myWarn_Warn("Hello2 %f\n", 3.1415);
   myWarn_Err("Hello3 %f\n", 3.1415);
   myWarn_Err1Arg("Test: Ran out of memory\n");
   myWarn_Err("(%s line %d) Test: Ran out of memory\n", __FILE__, __LINE__);

   myWarnClear(1, &msg);

   printf("After program memory was:\n");
   printf("'%s'\n", msg);

   return 0;
}
#endif
