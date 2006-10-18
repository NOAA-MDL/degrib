/*****************************************************************************
 * tendian.h
 *
 * DESCRIPTION
 *    This file contains all the utility functions that the Driver uses to
 * solve endian'ness related issues.
 *
 * HISTORY
 *    9/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#ifndef TENDIAN_H
#define TENDIAN_H

#include <stdio.h>
#include "type.h"

/*
 * MadeOnIntel    ==> LittleEndian
 * NotMadeOnIntel ==> BigEndian
 */
#undef BIG_ENDIAN
#undef LITTLE_ENDIAN

#ifdef WORDS_BIGENDIAN
  #define BIG_ENDIAN
#else
  #define LITTLE_ENDIAN
#endif

/* The following #defines are used to make the code easier to read. */
#ifdef BIG_ENDIAN
  #define FREAD_BIG fread
  #define FREAD_LIT revfread
  #define FWRITE_BIG fwrite
  #define FWRITE_LIT revfwrite
  #define MEMCPY_BIG memcpy
  #define MEMCPY_LIT revmemcpy
#else
  #define FREAD_BIG revfread
  #define FREAD_LIT fread
  #define FWRITE_BIG revfwrite
  #define FWRITE_LIT fwrite
  #define MEMCPY_BIG revmemcpy
  #define MEMCPY_LIT memcpy
#endif

void memswp (void *Data, const size_t elem_size, const size_t num_elem);

void *revmemcpy (void *Dst, void *Src, const size_t len);
void *revmemcpyRay (void *Dst, void *Src, const size_t elem_size,
                    const size_t num_elem);

size_t revfread (void *Dst, size_t elem_size, size_t num_elem, FILE *fp);
size_t revfwrite (void *Src, size_t elem_size, size_t num_elem, FILE *fp);

size_t FREAD_ODDINT_BIG (sInt4 * dst, uChar len, FILE *fp);
size_t FREAD_ODDINT_LIT (sInt4 * dst, uChar len, FILE *fp);
size_t FWRITE_ODDINT_BIG (sInt4 * src, uChar len, FILE *fp);
size_t FWRITE_ODDINT_LIT (sInt4 * src, uChar len, FILE *fp);

char memBitRead (void *Dst, size_t dstLen, void *Src, size_t numBits,
                 uChar * bufLoc, size_t *numUsed);
char memBitWrite (void *Src, size_t srcLen, void *Dst, size_t numBits,
                  uChar * bufLoc, size_t *numUsed);

int fileBitRead (void *Dst, size_t dstLen, uShort2 num_bits, FILE *fp,
                 uChar * gbuf, sChar * gbufLoc);
char fileBitWrite (void *Src, size_t srcLen, uShort2 numBits, FILE *fp,
                   uChar * pbuf, sChar * pbufLoc);

#endif
