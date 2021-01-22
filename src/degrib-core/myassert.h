#ifndef MYASSERT_H
#define MYASSERT_H
/* From myassert.h */
#ifdef DEBUG
   void _myAssert(const char *file, int lineNum);
   #define myAssert(f) \
      if (f)          \
         {}           \
      else            \
         _myAssert (__FILE__, __LINE__)
#else
   #define myAssert(f)
#endif


#endif
