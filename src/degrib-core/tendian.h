#ifndef TENDIAN_H
#define TENDIAN_H

#include "libaat_type.h"

/* From tendian.h */
/* Intel ==> LittleEndian; NotMadeOnIntel ==> BigEndian */
#ifdef WORDS_BIGENDIAN
/* Depricated...
  #define BIG_ENDIAN
  #undef LITTLE_ENDIAN
*/
  #define MEMCPY_BIG memcpy
  #define MEMCPY_LIT revmemcpy
  #define FREAD_BIG fread
  #define FREAD_LIT revfread
  #define FWRITE_BIG fwrite
  #define FWRITE_LIT revfwrite
#else
/* Depricated...
  #define LITTLE_ENDIAN
  #undef BIG_ENDIAN
*/
  #define MEMCPY_BIG revmemcpy
  #define MEMCPY_LIT memcpy
  #define FREAD_BIG revfread
  #define FREAD_LIT fread
  #define FWRITE_BIG revfwrite
  #define FWRITE_LIT fwrite
#endif

void memswp(void *Data, size_t size, size_t num);
void *revmemcpy(void *Dst, const void *Src, size_t len);
void *revmemcpyRay(void *Dst, const void *Src, size_t len, size_t num);
int memBitRead(void *Dst, size_t dstLen, const void *Src, size_t numBits,
               uChar *bufLoc, size_t *numUsed);
int memBitWrite(const void *Src, size_t srcLen, void *Dst, size_t numBits,
                uChar *bufLoc, size_t *numUsed);

size_t revfread(void *Dst, size_t size, size_t num, FILE *fp);
size_t revfwrite(const void *Src, size_t size, size_t num, FILE *fp);
size_t FREAD_ODDINT_BIG(sInt4 *dst, uChar len, FILE *fp);
size_t FREAD_ODDINT_LIT(sInt4 *dst, uChar len, FILE *fp);
size_t FWRITE_ODDINT_BIG(const sInt4 *src, uChar len, FILE *fp);
size_t FWRITE_ODDINT_LIT(const sInt4 *src, uChar len, FILE *fp);
int fileBitRead(void *Dst, size_t dstLen, size_t numBits, FILE *fp,
                uChar *gbuf, sChar *gbufLoc);
int fileBitWrite(const void *Src, size_t srcLen, size_t numBits, FILE *fp,
                 uChar *pbuf, sChar *pbufLoc);
#endif
