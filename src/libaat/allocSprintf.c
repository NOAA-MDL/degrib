/*****************************************************************************
 * allocSprintf.c
 *
 * DESCRIPTION
 *    This file contains code to handle a version of sprintf which allocates
 * memory for the calling routine, so that one doesn't have to guess the
 * maximum bounds of the message.
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL / RSIS): Created.
 * 12/2002 Rici Yu, Fangyu Chi, Mark Armstrong, & Tim Boyer
 *         (RY,FC,MA,&TB): Code Review 2.
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 * 1) See Kernighan & Ritchie C book (2nd edition) page 156.
 * 2) Originally this code was in myerror.c
 * 3) Would like a lazier allocSprintf routine (didn't alloc as often).
 ****************************************************************************/
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "libaat.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

/*****************************************************************************
 * allocSprintf() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Based on minprintf (see K&R C book (2nd edition) page 156.  This code
 * tries to provide some of the functionality of sprintf, while at the same
 * time handling memory allocation.  In addition, it provides a %S option,
 * which allows one to pass in an array of strings, and get back a comma
 * delimited string.
 *    The code allocates exactly the amount needed.  This could result in
 * extra calls to 'realloc'.  In addition, if Size != 0, it always starts new
 * writes at strlen(Ptr) (ie it doesn't over-write).
 *
 *    Supported formats:
 * %0.4f => float, double
 * %03d %ld %10ld => int, sInt4.
 * %c => int
 * %e => float, double
 * %g => float, double
 * %s => Null terminated char string. (no range specification)
 * %S => take a char ** and turn it into a comma delimited string.
 *
 * ARGUMENTS
 *  Ptr = An array of data that is of size LenBuff. (Input/Output)
 * Size = The allocated length of Ptr. (Input/Output)
 *  fmt = Format similar to the one used by sprintf to define how to print the
 *        message (Input)
 *   ap = argument list initialized by a call to va_start.  Contains the
 *        data needed by fmt. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 memory allocation error
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 * 12/2002 AAT: Fixed the mallocSprintf ("") error.
 *  2/2003 AAT: increased bufpart[80] to bufpart[330] because the largest 64
 *         bit double is: +1.7E+308, and I want 20 "slots" for stuff after the
 *         decimal place. There is the possibility of "Long doubles" (80 bits)
 *         which would have a max of: +3.4E+4932, but that is excessive for
 *         now.
 *  2/2004 AAT: if lenBuff != 0, switch from ipos-- to strlen (buffer);
 *  3/2004 AAT: Added %c option.
 * 11/2005 AAT: Added %e option.
 *  1/2006 AAT: Found a bug with multiple errSprintf.  Doesn't seem to be able
 *         to handle lenBuff > strlen(buffer) when procedure is first called.
 *         Something like format = "aaa%s", lenBuff = 3, buff = 'n' would
 *         result in 'naaa__<string>', instead of 'naaa<string>'.  Simple
 *         solution set lenBuff = strlen (buff).  Better solution: Maybe
 *         calculate correct place for ipos before switch.
 *  3/2007 AAT: Updated.
 *
 * NOTES
 * Assumes that no individual float or int will be more than 80 characters
 * Assumes that no % option is more than 20 char.
 ****************************************************************************/
int allocSprintf(char **Ptr, size_t *Size, const char *fmt, va_list ap)
{
   char *buffer = *Ptr; /* Local copy of Ptr. */
   size_t lenBuff = *Size; /* Local copy of Size. */
   const char *p;       /* Points to % char in % option. */
   const char *p1;      /* Points to end of % option. */
   char bufpart[330];   /* Used for formating the int / float options. */
   char format[20];     /* Used to store the % option. */
   char *sval;          /* For pulling strings off va_list. */
   char **Sval;         /* For pulling lists of strings off va_list. */
   size_t slen;         /* Length of used part of temp. */
   char f_inLoop;       /* Flag to state whether we got into %S , loop. */
   char flag;           /* If they have a l,L,h in string. */
   size_t ipos;         /* The current index to start storing data. */
   int c_type;          /* Used when handling %c option. */

   myAssert(sizeof(char) == 1);
   if ((fmt == NULL) || (strlen(fmt) == 0)) {
      return 0;
   }
   p = fmt;
   /* If lenBuff = 0, then make room for the '\0' character. */
   if (lenBuff == 0) {
      lenBuff++;
      if ((buffer = (char *)realloc((void *)buffer, lenBuff)) == NULL) {
         return -1;
      }
      ipos = 0;
   } else {
      myAssert(lenBuff >= strlen(buffer) + 1);
      lenBuff = strlen(buffer) + 1;
      ipos = lenBuff - 1;
   }
   /* buffer, and lenBuff are in agreement now as to allocated space and
    * strlen(), so we don't have to do the while loop at all. */
   while (p < fmt + strlen(fmt)) {
      p1 = p;
      p = strchr(p1, '%');
      /* Handle simple case when no more % in format string. */
      if (p == NULL) {
         /* No more format strings; copy rest of format and return */
         lenBuff += strlen(p1);
         if ((buffer = (char *)realloc((void *)buffer, lenBuff)) == NULL) {
            return -1;
         }
         strcpy(buffer + ipos, p1);
         goto done;
      }
      /* Handle data up to the current % in format string. */
      lenBuff += p - p1;
      if ((buffer = (char *)realloc((void *)buffer, lenBuff)) == NULL) {
         return -1;
      }
      strncpy(buffer + ipos, p1, p - p1);
      ipos = lenBuff - 1;
      /* Start dealing with % of format. */
      p1 = p + strspn(p + 1, "0123456789.");
      p1++;
      /* p1 points to first letter after %. */
      switch (*p1) {
         case 'l':
         case 'L':
            flag = *p1;
            p1++;
            break;
         case '\0':
            /* Handle improper use of '%' for example: '%##' */
            lenBuff += p1 - p - 1;
            if ((buffer = (char *)realloc((void *)buffer, lenBuff)) == NULL) {
               return -1;
            }
            strncpy(buffer + ipos, p + 1, p1 - p - 1);
            goto done;
         default:
            flag = ' ';
      }
      if ((p1 - p + 1) > (int)(sizeof(format)) - 1) {
         /* Protect against overflow of format string. */
         lenBuff += p1 - p + 1;
         if ((buffer = (char *)realloc((void *)buffer, lenBuff)) == NULL) {
            return -1;
         }
         strncpy(buffer + ipos, p, p1 - p + 1);
         ipos = lenBuff - 1;
      } else {
         strncpy(format, p, p1 - p + 1);
         format[p1 - p + 1] = '\0';
         switch (*p1) {
            case 'd':
               switch (flag) {
                  case 'l':
                  case 'L':
                     sprintf(bufpart, format, va_arg(ap, sInt4));
                     break;
                  default:
                     sprintf(bufpart, format, va_arg(ap, int));
               }
               slen = strlen(bufpart);
               lenBuff += slen;
               if ((buffer = (char *)realloc((void *)buffer,
                                             lenBuff)) == NULL) {
                  return -1;
               }
               strncpy(buffer + ipos, bufpart, slen);
               ipos = lenBuff - 1;
               break;
            case 'f':
               sprintf(bufpart, format, va_arg(ap, double));
               slen = strlen(bufpart);
               lenBuff += slen;
               if ((buffer = (char *)realloc((void *)buffer,
                                             lenBuff)) == NULL) {
                  return -1;
               }
               strncpy(buffer + ipos, bufpart, slen);
               ipos = lenBuff - 1;
               break;
            case 'e':
               sprintf(bufpart, format, va_arg(ap, double));
               slen = strlen(bufpart);
               lenBuff += slen;
               if ((buffer = (char *)realloc((void *)buffer,
                                             lenBuff)) == NULL) {
                  return -1;
               }
               strncpy(buffer + ipos, bufpart, slen);
               ipos = lenBuff - 1;
               break;
            case 'g':
               sprintf(bufpart, format, va_arg(ap, double));
               slen = strlen(bufpart);
               lenBuff += slen;
               if ((buffer = (char *)realloc((void *)buffer,
                                             lenBuff)) == NULL) {
                  return -1;
               }
               strncpy(buffer + ipos, bufpart, slen);
               ipos = lenBuff - 1;
               break;
            case 'c':
               c_type = va_arg(ap, int);
               lenBuff += 1;
               if ((buffer = (char *)realloc((void *)buffer,
                                             lenBuff)) == NULL) {
                  return -1;
               }
               buffer[ipos] = (char)c_type;
               buffer[ipos + 1] = '\0';
               ipos = lenBuff - 1;
               break;
            case 's':
               if ((p1 - p) == 1) {
                  sval = va_arg(ap, char *);
                  slen = strlen(sval);
                  lenBuff += slen;
                  if ((buffer = (char *)realloc((void *)buffer,
                                                lenBuff)) == NULL) {
                     return -1;
                  }
                  strncpy(buffer + ipos, sval, slen);
                  ipos = lenBuff - 1;
                  break;
               }
               /* Intentionally fall through. */
            case 'S':
               if ((p1 - p) == 1) {
                  f_inLoop = 0;
                  for (Sval = va_arg(ap, char **); *Sval; Sval++) {
                     slen = strlen(*Sval);
                     lenBuff += slen + 1;
                     if ((buffer = (char *)realloc((void *)buffer,
                                                   lenBuff)) == NULL) {
                        return -1;
                     }
                     strcpy(buffer + ipos, *Sval);
                     strcat(buffer + ipos + slen, ",");
                     ipos = lenBuff - 1;
                     f_inLoop = 1;
                  }
                  if (f_inLoop) {
                     lenBuff--;
                     buffer[lenBuff] = '\0';
                     ipos = lenBuff - 1;
                  }
                  break;
               }
               /* Intentionally fall through. */
            default:
               lenBuff += p1 - p;
               if ((buffer = (char *)realloc((void *)buffer,
                                             lenBuff)) == NULL) {
                  return -1;
               }
               strncpy(buffer + ipos, p + 1, p1 - p);
               ipos = lenBuff - 1;
         }
      }
      p = p1 + 1;
   }
 done:
   buffer[lenBuff - 1] = '\0';
   *Ptr = buffer;
   *Size = lenBuff;
   return 0;
}

/*****************************************************************************
 * mallocSprintf() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    The procedure combines malloc and sprintf.  It does so by allocating the
 * memory as it does the sprintf.  It does not save any information that the
 * original pointer may have pointed to.
 *
 *    Supported formats:
 * %0.4f => float, double
 * %03d %ld %10ld => int, sInt4.
 * %c => int
 * %e => float, double
 * %g => float, double
 * %s => Null terminated char string. (no range specification)
 * %S => take a char ** and turn it into a comma delimited string.
 *
 * ARGUMENTS
 * Ptr = Place to point to new memory which contains the message (Output)
 * fmt = Format similar to the one used by sprintf to define how to print the
 *       message (Input)
 * ... = Extra arguments
 *
 * RETURNS: int
 *    0 ok
 *   -1 memory allocation error
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  3/2007 AAT: Updated.
 *
 * NOTES
 ****************************************************************************/
int mallocSprintf(char **Ptr, const char *fmt, ...)
{
   va_list ap;          /* Contains the data needed by fmt. */
   size_t size = 0;     /* Allocated size of buffer. */
   int ierr = 0;        /* Error return code */

   *Ptr = NULL;
   if (fmt != NULL) {
      va_start(ap, fmt); /* make ap point to 1st unnamed arg. */
      ierr = allocSprintf(Ptr, &size, fmt, ap);
      va_end(ap);       /* clean up when done. */
   }
   return ierr;
}

/*****************************************************************************
 * reallocSprintf() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    The procedure combines malloc, sprintf, and strcat.  It does so by
 * starting to perform an sprintf at the end of the string (kind of like
 * strcat) but then allocating more memory as it needs to.
 *
 *    Supported formats:
 * %0.4f => float, double
 * %03d %ld %10ld => int, sInt4.
 * %c => int
 * %e => float, double
 * %g => float, double
 * %s => Null terminated char string. (no range specification)
 * %S => take a char ** and turn it into a comma delimited string.
 *
 * ARGUMENTS
 * Ptr = Pointer to memory to add the message to. (Input/Output)
 * fmt = Format similar to the one used by sprintf to define how to print the
 *       message (Input)
 * ... = Extra arguments
 *
 * RETURNS: int
 *    0 ok
 *   -1 memory allocation error
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  3/2007 AAT: Updated.
 *
 * NOTES
 ****************************************************************************/
int reallocSprintf(char **Ptr, const char *fmt, ...)
{
   va_list ap;          /* Contains the data needed by fmt. */
   size_t size;         /* Allocated size of buffer. */
   int ierr = 0;        /* Error return code */

   if (fmt != NULL) {
      va_start(ap, fmt); /* make ap point to 1st unnamed arg. */
      if (*Ptr == NULL) {
         size = 0;
      } else {
         size = strlen(*Ptr) + 1;
      }
      ierr = allocSprintf(Ptr, &size, fmt, ap);
      va_end(ap);       /* clean up when done. */
   }
   return ierr;
}

/*****************************************************************************
 * The following 2 procedures are included only to test allocSprintf.c, and
 * only if DEBUG_ALLOCSPRINTF is defined.
 ****************************************************************************/

/*****************************************************************************
 * checkAns() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To verify that a test gives the expected result.
 *
 * ARGUMENTS
 *  ptr = The results of the test. (Input)
 *  Ans = An array of correct answers. (Input)
 * test = Which test we are checking. (Input)
 *
 * RETURNS: void
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  3/2007 AAT: Updated.
 *
 * NOTES
 ****************************************************************************/
#ifdef DEBUG_ALLOCSPRINTF
static void checkAns(char *ptr, char **Ans, int test)
{
   if (ptr == NULL) {
      printf("-----Check test (%d)--(ptr == NULL)-----\n", test);
      return;
   }
   if (strcmp(ptr, Ans[test]) != 0) {
      printf("-----Failed test %d-------\n", test);
      printf("%s %d =?= %s %d\n", ptr, strlen(ptr),
             Ans[test], strlen(Ans[test]));
   } else {
      printf("passed test %d\n", test);
   }
}
#endif

/*****************************************************************************
 * main() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To test reallocSprintf, and mallocSprintf to make sure that they pass
 * certain basic tests.
 *
 * ARGUMENTS
 * argc = The number of arguments on the command line. (Input)
 * argv = The arguments on the command line. (Input)
 *
 * RETURNS: int
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  3/2007 AAT: Updated.
 *
 * NOTES
 ****************************************************************************/
#ifdef DEBUG_ALLOCSPRINTF
int main(int argc, char **argv)
{
   char *ptr;
   static char *Cmd[] = { "configure", "inquire", "convert", NULL };
   sInt4 li_temp = 100000L;
   short int sect = 5;
   char varName[] = "Helium is a gas";
   sInt4 lival = 22;
   char unit[] = "km", sval[] = "ans";
   double dval = 2.71828;

   char *buffer = NULL;
   short int ssect = 0;
   char vvarName[] = "DataType";
   sInt4 llival = 0;
   char ssval[] = "Meteorological products";

   static char *Ans[] = { "S0 | DataType | 0 (Meteorological products)\n",
      "<testing>",
      "<05><3.1415><D><20>",
      "<configure,inquire,convert> ?options?",
      "100000",
      "25.123",
      "02s",
      "01234567890123456789012345",
      "25.123,05, hello world",
      "S5 | Helium is a gas | 22 (ans)\nS5 | Helium is a gas | 22\n"
            "S5 | Helium is a gas | 22 (ans (km))\nS5 | Helium is a gas | ans\n"
            "S5 | Helium is a gas | 2.718280\nS5 | Helium is a gas | "
            "2.718280 (km)\n",
      "5.670000e+001"
   };

   /* Test -2. (See if it can handle blank). */
   mallocSprintf(&ptr, "");
   free(ptr);
   ptr = NULL;
   mallocSprintf(&ptr, " ");
   free(ptr);
   ptr = NULL;

   /* Test 0 */
   reallocSprintf(&buffer, "S%d | %s | %ld (%s)\n", ssect, vvarName,
                  llival, ssval);
   checkAns(buffer, Ans, 0);
   free(buffer);

   /* Test 1. */
   ptr = NULL;
   reallocSprintf(&ptr, "<testing>");
   checkAns(ptr, Ans, 1);
   free(ptr);

   /* Test 2. */
   ptr = NULL;
   reallocSprintf(&ptr, "<%02d><%.4f><%D><%ld>", 5, 3.1415, 20, 24);
   checkAns(ptr, Ans, 2);
   free(ptr);

   /* Test 3. */
   ptr = NULL;
   reallocSprintf(&ptr, "<%S> ?options?", Cmd);
   checkAns(ptr, Ans, 3);
   free(ptr);

   /* Test 4. */
   ptr = NULL;
   reallocSprintf(&ptr, "%ld", li_temp);
   checkAns(ptr, Ans, 4);
   free(ptr);

   /* Test 5. */
   ptr = NULL;
   reallocSprintf(&ptr, "%.3f", 25.1234);
   checkAns(ptr, Ans, 5);
   free(ptr);

   /* Test 6. */
   ptr = NULL;
   reallocSprintf(&ptr, "%02s", 25.1234);
   checkAns(ptr, Ans, 6);
   free(ptr);

   /* Test 7. */
   ptr = NULL;
   reallocSprintf(&ptr, "%01234567890123456789012345");
   checkAns(ptr, Ans, 7);
   free(ptr);

   /* Test 8. */
   mallocSprintf(&ptr, "%.3f", 25.1234);
   reallocSprintf(&ptr, ",%02d", 5);
   reallocSprintf(&ptr, ", %s", "hello world");
   checkAns(ptr, Ans, 8);
   free(ptr);
   ptr = NULL;

   /* Test 9. */
   ptr = NULL;
   reallocSprintf(&ptr, "S%d | %s | %ld (%s)\n", sect, varName, lival, sval);
   reallocSprintf(&ptr, "S%d | %s | %ld\n", sect, varName, lival);
   reallocSprintf(&ptr, "S%d | %s | %ld (%s (%s))\n", sect, varName, lival,
                  sval, unit);
   reallocSprintf(&ptr, "S%d | %s | %s\n", sect, varName, sval);
   reallocSprintf(&ptr, "S%d | %s | %f\n", sect, varName, dval);
   reallocSprintf(&ptr, "S%d | %s | %f (%s)\n", sect, varName, dval, unit);
   checkAns(ptr, Ans, 9);
   free(ptr);

   /* Test 10. */
   ptr = NULL;
   reallocSprintf(&ptr, "%e", 56.7);
   checkAns(ptr, Ans, 10);
   free(ptr);

   return 0;
}
#endif
