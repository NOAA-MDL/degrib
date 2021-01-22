#ifndef LIBAAT_TYPE_H
#define LIBAAT_TYPE_H

/* Pays attention to the following compile time #defines:
 * 1a) __64BIT__ => use 64 bit compilation as opposed to 32 bit
 * 1b) SIZEOF_LONG_INT =8 vs =4 => use 64 bit compilation as opposed to 32 bit
 */

/* From type.h */
#ifndef SINT4_TYPE
#define SINT4_TYPE
 #ifdef __64BIT__
  typedef signed int sInt4;
  typedef unsigned int uInt4;
 #else
  #ifdef _64Bit
   typedef signed int sInt4;
   typedef unsigned int uInt4;
  #else
   #if SIZEOF_LONG_INT == 8
    typedef signed int sInt4;
    typedef unsigned int uInt4;
   #else
    typedef signed long int sInt4;
    typedef unsigned long int uInt4;
   #endif
  #endif
 #endif
 typedef unsigned char uChar;
 typedef signed char sChar;
 typedef unsigned short int uShort2;
 typedef signed short int sShort2;
#endif
#endif
