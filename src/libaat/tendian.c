/*****************************************************************************
 * tendian.c
 *
 * DESCRIPTION
 *    This file contains some functions to solve endian'ness related issues.
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL / RSIS): Created.
 * 12/2002 Rici Yu, Fangyu Chi, Mark Armstrong, & Tim Boyer
 *         (RY,FC,MA,&TB): Code Review 2.
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "libaat.h"

#ifndef TENDIAN_BITMASK
#define TENDIAN_BITMASK
static const uChar BitMask[] = { 0, 1, 3, 7, 15, 31, 63, 127, 255 };
#endif

/*****************************************************************************
 * memswp() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To swap memory in the Data array based on the knownledge that there are
 * "num" elements, each of size "size".
 *
 * ARGUMENTS
 * Data = A pointer to the data to be swapped. (Input/Output)
 * size = The size of an individual element. (Input)
 *  num = The number of elements to swap. (Input)
 *
 * RETURNS: void
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 *    Could try this with exclusive or?
 ****************************************************************************/
void memswp(void *Data, size_t size, size_t num)
{
   size_t i;            /* Element count */
   char *data;          /* Allows us to treat Data as an array of char. */
   char temp;           /* A temporary holder of a byte when swapping. */
   char *ptr, *ptr2;    /* Pointers to the two bytes to swap. */

   if (size == 1) {
      return;
   }
   data = (char *)Data;
   for (i = 0; i < size * num; i += size) {
      ptr = data + i;
      ptr2 = ptr + size - 1;
      while (ptr2 > ptr) {
         temp = *ptr;
         *(ptr++) = *ptr2;
         *(ptr2--) = temp;
      }
   }
}

/*****************************************************************************
 * MEMCPY_BIG (sometimes macro for revmemcpy()) -- Arthur Taylor / MDL
 * MEMCPY_LIT (sometimes macro for revmemcpy()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Copies and reverses memory (similar to memcpy).  In order to have the
 * same arguments as memcpy, this can not handle arrays.  For arrays use
 * revmemcpyRay().  Returns the same thing that memcpy does.
 *    This assumes that Dst is allocated to a size of "len".  If Dst is larger
 * then "len", the data will be in the first "len" bytes.
 *
 * ARGUMENTS
 * Dst = The destination for the data. (Output)
 * Src = The source of the data. (Input)
 * len = The length of Src in bytes. (Input)
 *
 * RETURNS: void *
 *    A pointer to Dst.
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 * 1) This came about as I was trying to improve on the use of memcpy.  I
 *    figured that revmemcpy would be faster than memcpy followed by memswp.
 ****************************************************************************/
void *revmemcpy(void *Dst, const void *Src, size_t len)
{
   size_t i;            /* Byte count */
   char *src = (char *)Src; /* Allows us to treat Src as an array of char. */
   char *dst = (char *)Dst; /* Allows us to treat Dst as an array of char. */

   src += len - 1;
   for (i = 0; i < len; ++i) {
      *(dst++) = *(src--);
   }
   return Dst;
}

/*****************************************************************************
 * revmemcpyRay() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Copies and reverses memory (similar to memcpy).  This handles the case
 * when we need to reverse memcpy an array of data.
 *    This assumes that Dst is allocated to a size of "len" * "num".  If Dst
 * is larger then "len" * "num", the data will be in the first "len" * "num"
 * bytes.
 *
 * ARGUMENTS
 * Dst = The destination for the data. (Output)
 * Src = The source of the data. (Input)
 * len = The size of a single element. (Input)
 * num = The number of elements in Src. (Input)
 *
 * RETURNS: void *
 *    A pointer to Dst.
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  2/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
void *revmemcpyRay(void *Dst, const void *Src, size_t len, size_t num)
{
   size_t i;            /* Element count. */
   size_t j;            /* Byte count. */
   char *src = (char *)Src; /* Allows us to treat Src as an array of char. */
   char *dst = (char *)Dst; /* Allows us to treat Dst as an array of char. */

   if (len == 1) {
      return memcpy(Dst, Src, num);
   }
   src -= (len + 1);
   for (i = 0; i < num; ++i) {
      src += 2 * len;
      for (j = 0; j < len; ++j) {
         *(dst++) = *(src--);
      }
   }
   return Dst;
}

/*****************************************************************************
 * memBitRead() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   To read bits from an uChar buffer array of memory.  Assumes BufLoc is
 * valid before first call.  Typically this means do a "bufLoc = 8;" before
 * the first call.
 *
 * ARGUMENTS
 *     Dst = Where to put the results. (Output)
 *  dstLen = Length in bytes of Dst. (Input)
 *     Src = The data to read the bits from. (Input)
 * numBits = How many bits to read. (Input)
 *  BufLoc = In Src, which bit to start reading from.
 *           Starts at 8 goes to 1. (Input/Output)
 * numUsed = How many bytes from Src were used while reading (Output)
 *
 * RETURNS: int
 *    1 on error, 0 if ok.
 *
 * HISTORY
 *  4/2003 Arthur Taylor (MDL/RSIS): Created
 *  5/2004 AAT: Bug in call to MEMCPY_BIG when numBytes != dstLen.
 *         On big endian machines we need to right justify the number.
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 * 1) Assumes binary bit stream is "big endian". Resulting in no byte
 *    boundaries ie 00100110101101 => 001001 | 10101101
 ****************************************************************************/
int memBitRead(void *Dst, size_t dstLen, const void *Src, size_t numBits,
               uChar *bufLoc, size_t *numUsed)
{
   uChar *src = (uChar *)Src; /* Allows us to treat Src as array of char. */
   uChar *dst = (uChar *)Dst; /* Allows us to treat Dst as array of char. */
   size_t numBytes;     /* How many bytes are needed in dst. */
   uChar dstLoc;        /* Where we are writing to in dst. */
   uChar *ptr;          /* Current byte we are writing to in dst. */
   /* uses: static const uChar BitMask[] */

   if (numBits == 0) {
      memset(Dst, 0, dstLen);
      (*numUsed) = 0;
      return 0;
   }
   /* Since numBits is always used with -1, may as well do --numBits here. */
   numBytes = ((--numBits) / 8) + 1;
   /* Check if dst has enough room. */
   if (dstLen < numBytes) {
      myWarn_Err1Arg("Dst doesn't have enough space\n");
      return 1;
   }
   memset(Dst, 0, dstLen);
   dstLoc = (uChar)((numBits % 8) + 1);
   if ((*bufLoc == 8) && (dstLoc == 8)) {
#ifndef WORDS_BIGENDIAN
      MEMCPY_BIG(Dst, Src, numBytes);
#else
      /* If numBytes != dstLen, then we need to right justify the ans */
      MEMCPY_BIG(dst + (dstLen - numBytes), Src, numBytes);
#endif
      (*numUsed) = numBytes;
      return 0;
   }
#ifndef WORDS_BIGENDIAN
   ptr = dst + (numBytes - 1);
#else
   ptr = dst + (dstLen - numBytes);
#endif

   *numUsed = 0;
   /* Deal with most significant byte in dst. */
   if (*bufLoc >= dstLoc) {
#ifndef WORDS_BIGENDIAN
      (*ptr--) |= ((*src & BitMask[*bufLoc]) >> (*bufLoc - dstLoc));
#else
      (*ptr++) |= ((*src & BitMask[*bufLoc]) >> (*bufLoc - dstLoc));
#endif
      (*bufLoc) -= dstLoc;
   } else {
      if (*bufLoc != 0) {
         *ptr |= ((*src & BitMask[*bufLoc]) << (dstLoc - *bufLoc));
         dstLoc -= *bufLoc;
         /* Assert: bufLoc should now be 0 */
      }
      src++;
      (*numUsed)++;
      /* Assert: bufLoc should now be 8 */
      /* Assert: We want to >> by bufLoc - dstLoc = 8 - dstLoc */
      /* 1) We can get fancy by moving "bufLoc = 8 - dstLoc" here, and then
       * shifting by bufLoc. */
      /* 2) Since dstLoc is no longer used, we can get fancy by not updating
       * dstLoc in earlier part.  So instead of: */
      /* A) dstLoc = dstLoc - bufLoc; bufLoc = 8 - dstLoc; we have */
      /* B) bufloc = 8 - (dstLoc - bufLoc) = bufLoc + 8 - dstLoc; */
#ifndef WORDS_BIGENDIAN
      *(ptr--) |= (*src >> (8 - dstLoc));
#else
      *(ptr++) |= (*src >> (8 - dstLoc));
#endif
      (*bufLoc) = 8 - dstLoc;
   }
   /* Assert: dstLoc should now be 8, but we don't use again in procedure. */

   /* Note bufLoc < dstLoc from here on.  Either it is 0 or < 8. */
   /* Also dstLoc is always 8 from here out. */
#ifndef WORDS_BIGENDIAN
   while (ptr >= dst) {
#else
   while (ptr < dst + dstLen) {
#endif
      if (*bufLoc != 0) {
         *ptr |= ((*src & BitMask[*bufLoc]) << (8 - *bufLoc));
         /* Assert: dstLoc should now be initDstLoc (8) - initBufLoc */
         /* Assert: bufLoc should now be 0 */
      }
      src++;
      (*numUsed)++;
      /* Assert: bufLoc should now be 8 */
      /* Assert: dstLoc should now be initDstLoc (8) - initBufLoc */
      /* Assert: We want to >> by bufLoc - dstLoc = (8 - (8 - initbufLoc)). */
#ifndef WORDS_BIGENDIAN
      *(ptr--) |= (*src >> *bufLoc);
#else
      *(ptr++) |= (*src >> *bufLoc);
#endif
   }
   if (*bufLoc == 0) {
      (*numUsed)++;
      *bufLoc = 8;
   }
   return 0;
}

/*****************************************************************************
 * memBitWrite() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To write bits from a data structure to an array of uChar.  Assumes that
 * the part of Dst we don't write to have been correctly initialized.
 * Typically this means do a "memset (dst, 0, sizeof (dst));" before the first
 * call.  Also assumes BufLoc is valid before first call.  Typically this
 * means do a "bufLoc = 8;" before the first call.
 *
 * ARGUMENTS
 *     Src = The data to read from. (Input)
 *  srcLen = Length in bytes of Src. (Input)
 *     Dst = The char buffer to write the bits to. (Output)
 * numBits = How many bits to write. (Input)
 *  BufLoc = Which bit in Dst to start writing to.
 *           Starts at 8 goes to 1. (Input/Output)
 * numUsed = How many bytes were written to Dst. (Output)
 *
 * RETURNS: int
 *    1 on error, 0 if ok.
 *
 * HISTORY
 *  4/2003 Arthur Taylor (MDL/RSIS): Created
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 * 1) Assumes binary bit stream should be "big endian". Resulting in no byte
 *    boundaries ie 00100110101101 => 001001 | 1010110
 * 2) Assumes that Dst is already zero'ed out.
 ****************************************************************************/
int memBitWrite(const void *Src, size_t srcLen, void *Dst, size_t numBits,
                uChar *bufLoc, size_t *numUsed)
{
   uChar *src = (uChar *)Src; /* Allows us to treat Src as array of char. */
   uChar *dst = (uChar *)Dst; /* Allows us to treat Dst as array of char. */
   size_t numBytes;     /* How many bytes are needed from src. */
   uChar srcLoc;        /* Which bit we are reading from in src. */
   uChar *ptr;          /* Current byte we are reading from in src. */
   /* uses: static const uChar BitMask[] */

   if (numBits == 0) {
      return 0;
   }
   /* Since numBits is always used with -1, may as well do --numBits here. */
   numBytes = ((--numBits) / 8) + 1;
   if (srcLen < numBytes) {
      myWarn_Err1Arg("Src doesn't have enough space\n");
      return 1;
   }
   srcLoc = (numBits % 8) + 1;

   if ((*bufLoc == 8) && (srcLoc == 8)) {
      MEMCPY_BIG(Dst, Src, numBytes);
      (*numUsed) = numBytes;
      return 0;
   }
#ifndef WORDS_BIGENDIAN
   ptr = src + (numBytes - 1);
#else
   ptr = src + (srcLen - numBytes);
#endif

   *numUsed = 0;
   /* Deal with most significant byte in src. */
   if (*bufLoc >= srcLoc) {
#ifndef WORDS_BIGENDIAN
      (*dst) |= ((*(ptr--) & BitMask[srcLoc]) << (*bufLoc - srcLoc));
#else
      (*dst) |= ((*(ptr++) & BitMask[srcLoc]) << (*bufLoc - srcLoc));
#endif
      (*bufLoc) -= srcLoc;
   } else {
      if (*bufLoc != 0) {
         (*dst) |= ((*ptr & BitMask[srcLoc]) >> (srcLoc - *bufLoc));
         /* Assert: srcLoc should now be srcLoc - InitBufLoc */
         srcLoc -= *bufLoc;
         /* Assert: bufLoc should now be 0 */
      }
      dst++;
      (*dst) = 0;
      (*numUsed)++;
      /* Assert: bufLoc should now be 8 */
      /* Assert: We want to >> by bufLoc - srcLoc = 8 - srcLoc */
      /* 1) We can get fancy by moving "bufLoc = 8 - srcLoc" here, and then
       * shifting by bufLoc. */
      /* 2) Since srcLoc is no longer used, we can get fancy by not updating
       * dstLoc in earlier part.  So instead of: */
      /* A) dstLoc = dstLoc - bufLoc; bufLoc = 8 - dstLoc; we have */
      /* B) bufloc = 8 - (dstLoc - bufLoc) = bufLoc + 8 - dstLoc; */
#ifndef WORDS_BIGENDIAN
      (*dst) |= (*(ptr--) << (8 - srcLoc));
#else
      (*dst) |= (*(ptr++) << (8 - srcLoc));
#endif
      (*bufLoc) = 8 - srcLoc;
   }
   /* Assert: srcLoc should now be 8, but we don't use again in procedure. */

#ifndef WORDS_BIGENDIAN
   while (ptr >= src) {
#else
   while (ptr < src + srcLen) {
#endif
      if (*bufLoc == 0) {
         dst++;
         (*numUsed)++;
#ifndef WORDS_BIGENDIAN
         (*dst) = *(ptr--);
#else
         (*dst) = *(ptr++);
#endif
      } else {
         (*dst) |= ((*ptr) >> (8 - *bufLoc));
         dst++;
         (*numUsed)++;
         (*dst) = 0;
#ifndef WORDS_BIGENDIAN
         (*dst) |= (*(ptr--) << *bufLoc);
#else
         (*dst) |= (*(ptr++) << *bufLoc);
#endif
      }
   }
   if (*bufLoc == 0) {
      dst++;
      (*numUsed)++;
      (*bufLoc) = 8;
      (*dst) = 0;
   }
   return 0;
}

/*****************************************************************************
 * FREAD_BIG (sometimes macro for revfread()) -- Arthur Taylor / MDL
 * FREAD_LIT (sometimes macro for revfread()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fread", but in a reverse manner.  It is assumed that file is
 * already opened and in the correct place.
 *
 * ARGUMENTS
 *  Dst = The destination for the data. (Output)
 * size = The size of a single element. (Input)
 *  num = The number of elements in Src. (Input)
 *   fp = The file to read from. (Input)
 *
 * RETURNS: size_t
 *    Number of elements read, or short count (possibly 0) on EOF or error.
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 *    Decided to read it in and then swap.  The thought is that it is faster
 * than doing several fgetc.  This is the exact opposite method as used in
 * revfwrite().
 ****************************************************************************/
size_t revfread(void *Dst, size_t size, size_t num, FILE *fp)
{
   size_t ans;          /* The answer from fread. */
   size_t i;            /* Byte count. */
   char *dst;           /* Allows us to treat Dst as an array of char. */
   char temp;           /* A temporary holder of a byte when swapping. */
   char *ptr, *ptr2;    /* Pointers to the two bytes to swap. */

   ans = fread(Dst, size, num, fp);
   if (size == 1) {
      return ans;
   }
   if (ans == num) {
      dst = (char *)Dst;
      for (i = 0; i < size * num; i += size) {
         ptr = dst + i;
         ptr2 = ptr + size - 1;
         while (ptr2 > ptr) {
            temp = *ptr;
            *(ptr++) = *ptr2;
            *(ptr2--) = temp;
         }
      }
   }
   return ans;
}

/*****************************************************************************
 * FWRITE_BIG (sometimes macro for revfwrite()) -- Arthur Taylor / MDL
 * FWRITE_LIT (sometimes macro for revfwrite()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fwrite", but in a reverse manner.  It is assumed that file is
 * already opened and in the correct place.
 *
 * ARGUMENTS
 *  Src = The source of the data. (Input)
 * size = The size of a single element. (Input)
 *  num = The number of elements in Src. (Input)
 *   fp = The file to write to. (Output)
 *
 * RETURNS: size_t
 *    Number of elements written, or short count (possibly 0) on EOF or error.
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 * 11/2002 Arthur Taylor (MDL/RSIS): Updated.
 * 12/2002 (RY,FC,MA,&TB): Code Review.
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 *    Decided to write using a bunch of fputc, since this is buffered.  The
 * thought here, is that it is faster than swapping memory and then writing.
 * This is the exact opposite method as revfread().
 ****************************************************************************/
size_t revfwrite(const void *Src, size_t size, size_t num, FILE *fp)
{
   char *ptr;           /* Current byte to put to file. */
   size_t i;            /* Byte count */
   size_t j;            /* Element count */
   char *src;           /* Allows us to treat Src as an array of char. */

   if (size == 1) {
      return fwrite(Src, size, num, fp);
   } else {
      src = (char *)Src;
      ptr = src - size - 1;
      for (j = 0; j < num; ++j) {
         ptr += 2 * size;
         for (i = 0; i < size; ++i) {
            if (fputc((int)*(ptr--), fp) == EOF) {
               return j;
            }
         }
      }
      return num;
   }
}

/*****************************************************************************
 * FREAD_ODDINT_BIG() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fread" into a sInt4, but in a reverse manner with not
 * necessarily all 4 bytes.  It reads big endian data from disk.  It is
 * assumed that the file is already opened and in the correct place.
 *
 * ARGUMENTS
 * dst = Where to store the data. (Output)
 * len = The number of bytes to read. (<= 4) (Input)
 *  fp = The file to read from. (Input)
 *
 * RETURNS: size_t
 *    Number of elements read, or short count (possibly 0) on EOF or error.
 *
 * HISTORY
 * 12/2004 Arthur Taylor (MDL): Created.
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
size_t FREAD_ODDINT_BIG(sInt4 *dst, uChar len, FILE *fp)
{
   *dst = 0;
   if (len > 4) {
      len = 4;
   }
#ifndef WORDS_BIGENDIAN
   return revfread(dst, len, 1, fp);
#else
   return fread((((char *)dst) + (4 - len)), len, 1, fp);
#endif
}

/*****************************************************************************
 * FREAD_ODDINT_LIT() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fread" into a sInt4, but in a reverse manner with not
 * necessarily all 4 bytes.  It reads little endian data from disk.  It is
 * assumed that the file is already opened and in the correct place.
 *
 * ARGUMENTS
 * dst = Where to store the data. (Output)
 * len = The number of bytes to read. (<= 4) (Input)
 *  fp = The file to read from. (Input)
 *
 * RETURNS: size_t
 *    Number of elements read, or short count (possibly 0) on EOF or error.
 *
 * HISTORY
 * 12/2004 Arthur Taylor (MDL): Created.
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
size_t FREAD_ODDINT_LIT(sInt4 *dst, uChar len, FILE *fp)
{
   *dst = 0;
   if (len > 4) {
      len = 4;
   }
#ifndef WORDS_BIGENDIAN
   return fread(dst, len, 1, fp);
#else
   return revfread((((char *)dst) + (4 - len)), len, 1, fp);
#endif
}

/*****************************************************************************
 * FWRITE_ODDINT_BIG() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fwrite" from a sInt4, but in a reverse manner with not
 * necessarily all 4 bytes.  It writes big endian data to disk.  It is
 * assumed that the file is already opened and in the correct place.
 *
 * ARGUMENTS
 * src = Where to read the data from. (Input)
 * len = The number of bytes to read. (<= 4) (Input)
 *  fp = The file to write the data to. (Input)
 *
 * RETURNS: size_t
 *    Number of elements written, or short count (possibly 0) on EOF or error.
 *
 * HISTORY
 * 12/2004 Arthur Taylor (MDL): Created.
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
size_t FWRITE_ODDINT_BIG(const sInt4 *src, uChar len, FILE *fp)
{
   if (len > 4) {
      len = 4;
   }
#ifndef WORDS_BIGENDIAN
   return revfwrite(src, len, 1, fp);
#else
   return fwrite((((char *)src) + (4 - len)), len, 1, fp);
#endif
}

/*****************************************************************************
 * FWRITE_ODDINT_LIT() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fwrite" from a sInt4, but in a reverse manner with not
 * necessarily all 4 bytes.  It writes little endian data to disk.  It is
 * assumed that the file is already opened and in the correct place.
 *
 * ARGUMENTS
 * src = Where to read the data from. (Input)
 * len = The number of bytes to read. (<= 4) (Input)
 *  fp = The file to write the data to. (Input)
 *
 * RETURNS: size_t
 *    Number of elements written, or short count (possibly 0) on EOF or error.
 *
 * HISTORY
 * 12/2004 Arthur Taylor (MDL): Created.
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
size_t FWRITE_ODDINT_LIT(const sInt4 *src, uChar len, FILE *fp)
{
   if (len > 4) {
      len = 4;
   }
#ifndef WORDS_BIGENDIAN
   return fwrite(src, len, 1, fp);
#else
   return revfwrite((((char *)src) + (4 - len)), len, 1, fp);
#endif
}

/*****************************************************************************
 * fileBitRead() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To read bits from a file.  Stores the current byte, and passes the bits
 * that were requested to the user.  Leftover bits, are stored in "gbuf",
 * which should be passed in for future reads.
 *    If numBits == 0, then flush the gbuf.
 *
 * ARGUMENTS
 *     Dst = The storage place for the data read from file. (Output)
 *  dstLen = The size of dst (in bytes) (Input)
 *      fp = The open file to read from. (Input)
 * numBits = The number of bits to read from the file. (Input)
 *    gbuf = The current bit buffer (Input/Output)
 * gbufLoc = Where we are in the current bit buffer. (Input/Output)
 *
 * RETURNS: int
 *    EOF if EOF, 1 if error, 0 if ok.
 *
 * HISTORY
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
int fileBitRead(void *Dst, size_t dstLen, FILE *fp, size_t numBits,
                uChar *gbuf, sChar *gbufLoc)
{
   uChar *dst = (uChar *)Dst; /* Allows us to treat Dst as array of char. */
   size_t numBytes;     /* How many bytes are needed in dst. */
   uChar dstLoc;        /* Where we are writing to in dst. */
   uChar *ptr;          /* Current byte we are writing to in dst. */
   /* uses: static const uChar BitMask[] */
   int c;               /* The current byte from file. */
   uChar buf;           /* The current buffer of extra bits. */
   uChar bufLoc;        /* Where we are reading from in buffer */

   if (numBits == 0) {
      memset(Dst, 0, dstLen);
      *gbuf = 0;
      *gbufLoc = 0;
      return 0;
   }
   /* Since numBits is always used with -1, may as well do --numBits here. */
   numBytes = ((--numBits) / 8) + 1;
   /* Check if dst has enough room. */
   if (dstLen < numBytes) {
      myWarn_Err1Arg("Dst doesn't have enough space\n");
      return 1;
   }
   memset(Dst, 0, dstLen);
   dstLoc = (uChar)((numBits % 8) + 1);
   bufLoc = *gbufLoc;
   buf = *gbuf;
#ifndef WORDS_BIGENDIAN
   ptr = dst + (numBytes - 1);
#else
   ptr = dst + (dstLen - numBytes);
#endif

   /* Deal with initial "remainder" part (most significant byte) in dst. */
   if (bufLoc >= dstLoc) {
      /* can now deal with entire "remainder". */
#ifndef WORDS_BIGENDIAN
      *(ptr--) |= ((buf & BitMask[bufLoc]) >> (bufLoc - dstLoc));
#else
      *(ptr++) |= ((buf & BitMask[bufLoc]) >> (bufLoc - dstLoc));
#endif
      bufLoc -= dstLoc;
   } else {
      /* need to do 2 calls to deal with entire "remainder". */
      if (bufLoc != 0) {
         *ptr |= ((buf & BitMask[bufLoc]) << (dstLoc - bufLoc));
         dstLoc -= bufLoc;
         /* Assert: bufLoc should now be 0 */
      }
      if ((c = fgetc(fp)) == EOF) {
         *gbufLoc = bufLoc;
         *gbuf = buf;
         return EOF;
      }
      buf = (uChar)c;
      /* Assert: bufLoc should now be 8 */
      /* Assert: We want to >> by bufLoc - dstLoc = 8 - dstLoc */
      /* 1) We can get fancy by moving "bufLoc = 8 - dstLoc" here, and then
       * shifting by bufLoc. */
      /* 2) Since dstLoc is no longer used, we can get fancy by not updating
       * dstLoc in earlier part.  So instead of: */
      /* A) dstLoc = dstLoc - bufLoc; bufLoc = 8 - dstLoc; we have */
      /* B) bufloc = 8 - (dstLoc - bufLoc) = bufLoc + 8 - dstLoc; */
#ifndef WORDS_BIGENDIAN
      *(ptr--) |= (buf >> (8 - dstLoc));
#else
      *(ptr++) |= (buf >> (8 - dstLoc));
#endif
      bufLoc = 8 - dstLoc;
   }
   /* Assert: dstLoc should now be 8, but we don't use again in procedure. */

   /* Note bufLoc < dstLoc from here on.  Either it is 0 or < 8. */
   /* Also dstLoc is always 8 from here out. */
#ifndef WORDS_BIGENDIAN
   while (ptr >= dst) {
#else
   while (ptr < dst + dstLen) {
#endif
      if (bufLoc != 0) {
         *ptr |= (uChar)((buf & BitMask[bufLoc]) << (8 - bufLoc));
         /* Assert: dstLoc should now be initDstLoc (8) - initBufLoc */
         /* Assert: bufLoc should now be 0 */
      }
      if ((c = fgetc(fp)) == EOF) {
         *gbufLoc = bufLoc;
         *gbuf = buf;
         return EOF;
      }
      buf = (uChar)c;
      /* Assert: bufLoc should now be 8 */
      /* Assert: dstLoc should now be initDstLoc (8) - initBufLoc */
      /* Assert: We want to >> by bufLoc - dstLoc = (8 - (8 - initbufLoc)). */
#ifndef WORDS_BIGENDIAN
      *(ptr--) |= (buf >> bufLoc);
#else
      *(ptr++) |= (buf >> bufLoc);
#endif
   }
   *gbufLoc = bufLoc;
   *gbuf = buf;
   return 0;
}

/*****************************************************************************
 * fileBitWrite() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   To write bits from src out to file.  First writes out any leftover bits
 * in pbuf, then bits from src.  Any leftover bits that aren't on a full byte
 * boundary, are stored in pbuf.
 *   If numBits == 0, then flush the pbuf.
 *
 * ARGUMENTS
 *     Src = The data to put out to file. (Input)
 *  srcLen = Length in bytes of src. (Input)
 * numBits = The number of bits to write to file. (Input)
 *      fp = The opened file ptr to write to. (Input)
 *    pbuf = The extra bit buffer (Input/Output)
 * pBufLoc = The location in the bit buffer.
 *
 * RETURNS: int
 *    1 on error, 0 if ok.
 *
 * HISTORY
 *  8/2004 Arthur Taylor (MDL): Created
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
int fileBitWrite(const void *Src, size_t srcLen, FILE *fp, size_t numBits,
                 uChar *pbuf, sChar *pbufLoc)
{
   uChar *src = (uChar *)Src; /* Allows us to treat Src as array of char. */
   size_t numBytes;     /* How many bytes are needed from src. */
   uChar srcLoc;        /* Which bit we are reading from in src. */
   uChar *ptr;          /* Current byte we are reading from in src. */
   uChar buf;           /* The current buffer of extra bits. */
   uChar bufLoc;        /* Where we are reading from in buffer */

   if (numBits == 0) {
      if (*pbufLoc != 8) {
         fputc((int)*pbuf, fp);
         *pbuf = 0;
         *pbufLoc = 8;
/*         return 8; */
         myWarn_Note1Arg("Changed return value from 8 to 0 (is this an "
                         "introduced bug?\n");
         return 0;
      } else {
         /* Assert: *pbufLoc == 8 */
         *pbuf = 0;
         return 0;
      }
   }
   /* Since numBits is always used with -1, may as well do --numBits here. */
   numBytes = ((--numBits) / 8) + 1;
   if (srcLen < numBytes) {
      myWarn_Err1Arg("Src doesn't have enough space\n");
      return 1;
   }
   srcLoc = (numBits % 8) + 1;

   bufLoc = *pbufLoc;
   buf = *pbuf;

#ifndef WORDS_BIGENDIAN
   ptr = src + (numBytes - 1);
#else
   ptr = src + (srcLen - numBytes);
#endif

   /* Deal with most significant byte in src. */
   if (bufLoc >= srcLoc) {
#ifndef WORDS_BIGENDIAN
      buf |= ((*(ptr--) & BitMask[srcLoc]) << (bufLoc - srcLoc));
#else
      buf |= ((*(ptr++) & BitMask[srcLoc]) << (bufLoc - srcLoc));
#endif
      bufLoc -= srcLoc;
   } else {
      if (bufLoc != 0) {
         buf |= ((*ptr & BitMask[srcLoc]) >> (srcLoc - bufLoc));
         /* Assert: srcLoc should now be srcLoc - InitBufLoc */
         srcLoc -= bufLoc;
         /* Assert: bufLoc should now be 0 */
      }
      if (fputc((int)buf, fp) == EOF) {
         *pbufLoc = bufLoc;
         *pbuf = buf;
         myWarn_Err1Arg("Problems writing to file\n");
         return 1;
      }
      buf = 0;
      /* Assert: bufLoc should now be 8 */
      /* Assert: We want to >> by bufLoc - srcLoc = 8 - srcLoc */
      /* 1) We can get fancy by moving "bufLoc = 8 - srcLoc" here, and then
       * shifting by bufLoc. */
      /* 2) Since srcLoc is no longer used, we can get fancy by not updating
       * dstLoc in earlier part.  So instead of: */
      /* A) dstLoc = dstLoc - bufLoc; bufLoc = 8 - dstLoc; we have */
      /* B) bufloc = 8 - (dstLoc - bufLoc) = bufLoc + 8 - dstLoc; */
#ifndef WORDS_BIGENDIAN
      buf |= (*(ptr--) << (8 - srcLoc));
#else
      buf |= (*(ptr++) << (8 - srcLoc));
#endif
      bufLoc = 8 - srcLoc;
   }
   /* Assert: srcLoc should now be 8, but we don't use again in procedure. */

#ifndef WORDS_BIGENDIAN
   while (ptr >= src) {
#else
   while (ptr < src + srcLen) {
#endif
      if (bufLoc == 0) {
         if (fputc((int)buf, fp) == EOF) {
            *pbufLoc = bufLoc;
            *pbuf = buf;
            myWarn_Err1Arg("Problems writing to file\n");
            return 1;
         }
#ifndef WORDS_BIGENDIAN
         buf = *(ptr--);
#else
         buf = *(ptr++);
#endif
      } else {
         buf |= ((*ptr) >> (8 - bufLoc));
         if (fputc((int)buf, fp) == EOF) {
            *pbufLoc = bufLoc;
            *pbuf = buf;
            myWarn_Err1Arg("Problems writing to file\n");
            return 1;
         }
         buf = 0;
#ifndef WORDS_BIGENDIAN
         buf |= (*(ptr--) << bufLoc);
#else
         buf |= (*(ptr++) << bufLoc);
#endif
      }
   }
   if (bufLoc == 0) {
      if (fputc((int)buf, fp) == EOF) {
         *pbufLoc = bufLoc;
         *pbuf = buf;
         myWarn_Err1Arg("Problems writing to file\n");
         return 1;
      }
      bufLoc = 8;
      buf = 0;
   }
   *pbufLoc = bufLoc;
   *pbuf = buf;
   return 0;
}

/*****************************************************************************
 * main() (Private) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   To test the memBitRead, and memBitWrite routines, to make sure that they
 * function correctly on some sample data..
 *
 * ARGUMENTS
 * argc = The number of arguments on the command line. (Input)
 * argv = The arguments on the command line. (Input)
 *
 * RETURNS: int
 *
 * HISTORY
 *  4/2003 Arthur Taylor (MDL/RSIS): Created.
 *  3/2007 AAT (MDL): Commented
 *
 * NOTES
 ****************************************************************************/
#ifdef DEBUG_TENDIAN
int main(int argc, char **argv)
{
   uChar buff[5], buff2[5];
   uChar bufLoc = 8;
   uChar *ptr, *ptr2;
   int numUsed;

   buff[0] = 0x8f;
   buff[1] = 0x8f;
   buff[2] = 0x8f;
   buff[3] = 0x8f;
   buff[4] = 0x8f;

   bufLoc = 7;
   memBitRead(buff2, sizeof(buff2), buff, 39, &bufLoc, &numUsed);
   printf("%d %d %d %d %d ", buff2[0], buff2[1], buff2[2], buff2[3],
          buff2[4]);
   printf("-------should be----- ");
   printf("143 143 143 143 15\n");

   memset(buff, 0, sizeof(buff));
   bufLoc = 8;
   ptr = buff;
   ptr2 = buff2;
   memBitWrite(ptr2, sizeof(buff2), ptr, 9, &bufLoc, &numUsed);
   ptr += numUsed;
   ptr2++;
   memBitWrite(ptr2, sizeof(buff2), ptr, 7, &bufLoc, &numUsed);
   ptr += numUsed;
   ptr2++;
   memBitWrite(ptr2, sizeof(buff2), ptr, 7, &bufLoc, &numUsed);
   ptr += numUsed;
   ptr2++;
   memBitWrite(ptr2, sizeof(buff2), ptr, 9, &bufLoc, &numUsed);
   ptr += numUsed;
   ptr2++;
   memBitWrite(ptr2, sizeof(buff2), ptr, 8, &bufLoc, &numUsed);
   ptr += numUsed;
   printf("%d %d %d %d %d ", buff[0], buff[1], buff[2], buff[3], buff[4]);
   printf("-------should be----- ");
   printf("199 143 31 143 15\n");
   return 0;
}
#endif
