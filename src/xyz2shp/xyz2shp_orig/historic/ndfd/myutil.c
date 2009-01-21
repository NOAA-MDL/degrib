/*****************************************************************************
 * myutil.c
 *
 * DESCRIPTION
 *    This file contains some simple utility functions.
 *
 * HISTORY
 *   12/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "myutil.h"
#include "myassert.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

/*****************************************************************************
 * reallocFGets() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Read in data from file until a \n is read.  Reallocate memory as needed.
 * Similar to fgets, except we don't know ahead of time that the line is a
 * specific length.
 *   Assumes that Ptr is either NULL, or points to lenBuff memory.
 *   Responsibility of caller to free the memory.
 *
 * ARGUMENTS
 *     Ptr = An array of data that is of size LenBuff. (Input/Output)
 * LenBuff = The Allocated length of Ptr. (Input/Output)
 *      fp = Input file stream (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   strlen (buffer)...
 *     0 if we Read only EOF.
 *     1 if we have "\nEOF" or "<char>EOF"
 *
 * HISTORY
 *  12/2002 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *  1) Based on getline (see K&R C book (2nd edition) p 29)
 *     and on the behavior of Tcl's gets routine.
 *  2) Chose MIN_STEPSIZE = 80 because pages are usually 80 columns.
 *  3) Could switch lenBuff = i + 1 / lenBuff = i to always true.
 *     Rather not... Less allocs... This way code behaves almost the
 *     same as fgets except it can expand as needed.
 *****************************************************************************
 */
#define MIN_STEPSIZE 80
int reallocFGets (char **Ptr, int *LenBuff, FILE * fp)
{
   char *buffer = *Ptr; /* Local copy of Ptr. */
   int lenBuff = *LenBuff; /* Local copy of LenBuff. */
   int c;               /* Current char read from stream. */
   int i;               /* Where to store c. */

   for (i = 0; ((c = getc (fp)) != EOF) && (c != '\n'); ++i) {
      if (i >= lenBuff) {
         lenBuff += MIN_STEPSIZE;
         buffer = (char *) realloc ((void *) buffer,
                                    lenBuff * sizeof (char));
      }
      buffer[i] = (char) c;
   }
   if (c == '\n') {
      if (lenBuff <= i + 1) {
         lenBuff = i + 2; /* Make room for \n\0. */
         buffer = (char *) realloc ((void *) buffer,
                                    lenBuff * sizeof (char));
      }
      buffer[i] = (char) c;
      ++i;
      buffer[i] = '\0';
      *Ptr = buffer;
      *LenBuff = lenBuff;
      return i;
   } else {
      if (lenBuff <= i) {
         lenBuff = i + 1; /* Make room for \0. */
         buffer = (char *) realloc ((void *) buffer,
                                    lenBuff * sizeof (char));
      }
      buffer[i] = '\0';
      *Ptr = buffer;
      *LenBuff = lenBuff;
      return i;
   }
}

#undef MIN_STEPSIZE

/*****************************************************************************
 * mySplit() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Split a character array according to a given symbol.
 *   Responsibility of caller to free the memory.
 *
 * ARGUMENTS
 *   data = character string to look through. (Input)
 * symbol = character to split based on. (Input)
 *   argc = number of groupings found. (Output)
 *   argv = characters in each grouping. (Output)
 * f_trim = True if we should white space trim each element in list. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void mySplit (char *data, char symbol, int *Argc, char ***Argv, char f_trim)
{
   char *head;
   char *ptr;
   int argc;
   char **argv;

   myAssert (*Argc == 0);
   myAssert (*Argv == NULL);
   argc = 0;
   argv = NULL;

   head = data;
   while (head != NULL) {
      ptr = strchr (head, symbol);
      if (ptr != NULL) {
         *ptr = '\0';
      }
      argc++;
      argv = (char **) realloc ((void *) argv, argc * sizeof (char *));
      argv[argc - 1] = (char *) malloc (strlen (head) + 1 * sizeof (char));
      strcpy (argv[argc - 1], head);
      if (f_trim) {
         strTrim (argv[argc - 1]);
      }
      if (ptr != NULL) {
         *ptr = symbol;
         ptr++;
         if (*ptr == '\0') {
            ptr = NULL;
         }
      }
      head = ptr;
   }
   *Argc = argc;
   *Argv = argv;
}

/*****************************************************************************
 * myIsReal() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *     Returns 0 if all char are digit except a trailing ',', or leading
 * character of '-'.  value is set to atof (ptr).
 *
 * ARGUMENTS
 *   ptr = character string to look at. (Input)
 * value = the converted value of ptr, if ptr is a number. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   0 if it is not a real number,
 *   1 if it is a real number.
 *
 * HISTORY
 *  7/2004 Arthur Taylor (MDL): Updated
 *
 * NOTES
 *****************************************************************************
 */
int myIsReal (char *ptr, double *value)
{
   int len, i;

   *value = 0;
   if ((!isdigit (*ptr)) && (*ptr != '.'))
      if (*ptr != '-')
         return 0;
   len = strlen (ptr);
   for (i = 1; i < len - 1; i++) {
      if ((!isdigit (ptr[i])) && (ptr[i] != '.'))
         return 0;
   }
   if ((!isdigit (ptr[len - 1])) && (ptr[len - 1] != '.')) {
      if (ptr[len - 1] != ',') {
         return 0;
      } else {
         ptr[len - 1] = '\0';
         *value = atof (ptr);
         ptr[len - 1] = ',';
         return 1;
      }
   }
   *value = atof (ptr);
   return 1;
}

/*****************************************************************************
 * FileCopy() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Copy a file from one location to another.
 *
 * ARGUMENTS
 *  fileIn = source file to read from. (Input)
 * fileOut = destation file to write to. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   -1 if any problems opening the files
 *   0 on success.
 *
 * HISTORY
 *  5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int FileCopy (char *fileIn, char *fileOut)
{
   FILE *ifp;
   FILE *ofp;
   int c;

   if ((ifp = fopen (fileIn, "rb")) == NULL) {
      printf ("Couldn't open %s for read\n", fileIn);
      return -1;
   }
   if ((ofp = fopen (fileOut, "wb")) == NULL) {
      printf ("Couldn't open %s for write\n", fileOut);
      fclose (ifp);
      return -1;
   }
   while ((c = getc (ifp)) != EOF)
      putc (c, ofp);
   fclose (ifp);
   fclose (ofp);
   return 0;
}

/*****************************************************************************
 * FileTail() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Returns the characters in a filename after the last directory separator.
 *   Responsibility of caller to free the memory.
 *
 * ARGUMENTS
 * fileName = fileName to look in. (Input)
 *     tail = Tail of the filename. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void FileTail (char *fileName, char *tail)
{
   char *ptr;

   myAssert (tail == NULL);
   myAssert (fileName != NULL);
   ptr = strchr (fileName, '/');
   if (ptr == NULL) {
      ptr = strchr (fileName, '\\');
      if (ptr == NULL) {
         ptr = fileName;
      } else {
         ptr++;
      }
   } else {
      ptr++;
   }
   tail = (char *) malloc ((strlen (ptr) + 1) * sizeof (char));
   strcpy (tail, ptr);
}

/*****************************************************************************
 * myRound() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Round a number to a given number of decimal places.
 *
 * ARGUMENTS
 *  data = number to round (Input)
 * place = How many decimals to round to (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: double (rounded value)
 *
 * HISTORY
 *  5/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *  1) It is probably inadvisable to make a lot of calls to this routine,
 *     considering the fact that a context swap is made, so this is provided
 *     primarily as an example, but it can be used for some rounding.
 *****************************************************************************
 */
double POWERS_ONE[] = {
   1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
   1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17
};

double myRound (double data, signed char place)
{
   if (place > 17)
      place = 17;
   if (place < 0)
      place = 0;
   return (floor (data * POWERS_ONE[place] + .5)) / POWERS_ONE[place];
}

/*****************************************************************************
 * strTrim() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To trim the white space from both sides of a char string.
 *
 * ARGUMENTS
 * str = The string to trim (Input/Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  10/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void strTrim (char *str)
{
   int i, j;            /* loop counters for traversing str. */
   int len;             /* The length of str. */

   /* str shouldn't be null, but if it is, we want to handle it. */
   myAssert (str != NULL);
   if (str == NULL)
      return;

   /* Remove the trailing white space before working on the leading ones. */
   len = strlen (str);
   for (i = len - 1; ((i >= 0) && (isspace (str[i]))); i--) {
   }
   str[i + 1] = '\0';
   len = i + 1;

   /* Find first non-white space char. */
   for (i = 0; ((i < len) && (isspace (str[i]))); i++) {
   }

   /* Copy [i .. end] to [0 .. end - i]. */
   if (i != 0) {
      for (j = 0; j < strlen (str) - i; j++) {
         str[j] = str[i + j];
      }
      str[j] = '\0';
   }
}

/*****************************************************************************
 * strTrimRight() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To trim white space and a given char from the right.
 *
 * ARGUMENTS
 * str = The string to trim (Input/Output)
 *   c = The character to remove. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  7/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void strTrimRight (char *str, char c)
{
   int i;               /* loop counter for traversing str. */

   /* str shouldn't be null, but if it is, we want to handle it. */
   myAssert (str != NULL);
   if (str == NULL)
      return;

   /* Remove the trailing white space before working on the leading ones. */
   for (i = strlen (str) - 1;
        ((i >= 0) && ((isspace (str[i])) || (str[i] == c))); i--) {
   }
   str[i + 1] = '\0';
}

/*****************************************************************************
 * strCompact() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Replace any multiple instances of 'c' in the string with 1 instance.
 *
 * ARGUMENTS
 * str = The string to compact (Input/Output)
 *   c = The character to look for. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 * 10/2004 Arthur Taylor (MDL): Created.
 *
 * NOTES
 *****************************************************************************
 */
void strCompact (char *str, char c)
{
   char *ptr;           /* The next good value in str to keep. */

   /* str shouldn't be null, but if it is, we want to handle it. */
   myAssert (str != NULL);
   if (str == NULL)
      return;

   ptr = str;
   while (*str != '\0') {
      *str = *(ptr++);
      if (*(str++) == c) {
         while ((*ptr != '\0') && (*ptr == c)) {
            ptr++;
         }
      }
      if (*ptr == '\0') {
         *str = '\0';
         return;
      }
   }
}

/*****************************************************************************
 * strReplace() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To replace all instances of c1 in str with c2..
 *
 * ARGUMENTS
 * str = The string to trim (Input/Output)
 *  c1 = The character(s) in str to be replaced. (Input)
 *  c2 = The char to replace c1 with. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  7/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void strReplace (char *str, char c1, char c2)
{
   char *ptr = str;

   /* str shouldn't be null, but if it is, we want to handle it. */
   myAssert (str != NULL);
   if (str == NULL)
      return;

   while (*ptr != '\0') {
      if (*ptr == c1)
         *ptr = c2;
      ptr++;
   }
}

/*****************************************************************************
 * strToUpper() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To convert a string to all uppercase.
 *
 * ARGUMENTS
 * str = The string to adjust (Input/Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  10/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void strToUpper (char *str)
{
   char *ptr;           /* Used to traverse str. */

   /* str shouldn't be null, but if it is, we want to handle it. */
   myAssert (str != NULL);
   if (str == NULL)
      return;

   for (ptr = str; *ptr != '\0'; ptr++) {
      *ptr = toupper (*ptr);
   }
}

/*****************************************************************************
 * strToLower() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To convert a string to all uppercase.
 *
 * ARGUMENTS
 * str = The string to adjust (Input/Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void strToLower (char *str)
{
   char *ptr;           /* Used to traverse str. */

   /* str shouldn't be null, but if it is, we want to handle it. */
   myAssert (str != NULL);
   if (str == NULL)
      return;

   for (ptr = str; *ptr != '\0'; ptr++) {
      *ptr = tolower (*ptr);
   }
}

/*****************************************************************************
 * GetIndexFromStr() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Looks through a list of strings (with a NULL value at the end) for a
 * given string.  Returns the index where it found it.
 *
 * ARGUMENTS
 *   arg = The string to look for. (Input)
 *   Opt = The list to look for arg in. (Input)
 * Index = The location of arg in Opt (or -1 if it couldn't find it) (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  # = Where it found it.
 * -1 = Couldn't find it.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *
 * NOTES
 *****************************************************************************
 */
int GetIndexFromStr (char *arg, char **Opt, int *Index)
{
   char **ptr = Opt;
   int cnt = 0;
   while (*ptr != NULL) {
      if (strcmp (arg, *ptr) == 0) {
         *Index = cnt;
         return cnt;
      }
      ptr++;
      cnt++;
   }
   return -1;
}

/* "is" is something like "19730724000000" */
int myParseTime (char *is, time_t * AnsTime)
{
   int year;
   int mon;
   int day;
   int hour;
   int min;
   int sec;
   struct tm time;      /* A temporary variable to put the time info into. */
   char buffer[10];     /* Used when printing the AnsTime's Hr. */
   int timeZone;        /* The adjustment in Hr needed to get the right UTC
                         * time. */
   char c;

   memset (&time, 0, sizeof (struct tm));
   if (strlen (is) != 14) {
      printf ("%s is not formated correctly\n", is);
      return -1;
   }
   c = is[4];
   is[4] = '\0';
   year = atoi (is);
   is[4] = c;
   c = is[6];
   is[6] = '\0';
   mon = atoi (is + 4);
   is[6] = c;
   c = is[8];
   is[8] = '\0';
   day = atoi (is + 6);
   is[8] = c;
   c = is[10];
   is[10] = '\0';
   hour = atoi (is + 8);
   is[10] = c;
   c = is[12];
   is[12] = '\0';
   min = atoi (is + 10);
   is[12] = c;
   sec = atoi (is + 12);
   if ((year > 2001) || (year < 1900) || (mon > 12) || (mon < 1) ||
       (day > 31) || (day < 1) || (hour > 23) || (hour < 0) ||
       (min > 59) || (min < 0) || (sec > 60) || (sec < 0)) {
      printf ("date %s is invalid\n", is);
      return -1;
   }
   time.tm_year = year - 1900;
   time.tm_mon = mon - 1;
   time.tm_mday = day;
   time.tm_hour = hour;
   time.tm_min = min;
   time.tm_sec = sec;
   *AnsTime = mktime (&time);
   /* Cheap method of getting global time_zone variable. */
   strftime (buffer, 10, "%H", gmtime (AnsTime));
   timeZone = atoi (buffer) - hour;
   if (timeZone < 0) {
      timeZone += 24;
   }
   *AnsTime = *AnsTime - (timeZone * 3600);
   return 0;
}
