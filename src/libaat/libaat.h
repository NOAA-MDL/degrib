#ifndef LIBAAT_H
#define LIBAAT_H

#define LIBAAT_VERSION 1.0

/* Pays attention to the following compile time #defines:
 * 1a) __64BIT__ => use 64 bit compilation as opposed to 32 bit
 * 1b) _64Bit => use 64 bit compilation as opposed to 32 bit
 * 1c) SIZEOF_LONG_INT =8 vs =4 => use 64 bit compilation as opposed to 32 bit
 * 2) WORDS_BIGENDIAN => files are bigendian
 * 3) DEBUG => Pay attention to assert statements.
 * 4) MEMWATCH => Track memory.
 * 5) __STDC_VERSION__ => Used to detect if the compiler is C99 vs ANSI C
 * 6) NEED_ERF => Defined if math.h doesn't provide erf().
 * 7) PROVIDE_OLD_SPLINE => if we should provide the depricated spline code.
 */

#include <stdarg.h>
#include <stdio.h>

#include "libaat_type.h"

/* From myopt.h */
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

/* From allocSprintf.c */
int allocSprintf(char **Ptr, size_t *Size, const char *fmt, va_list ap);
int mallocSprintf(char **Ptr, const char *fmt, ...);
int reallocSprintf(char **Ptr, const char *fmt, ...);

/* From mywarn.c */
void myWarnSet(uChar f_stdout, uChar f_stderr, uChar f_mem, uChar f_file,
               FILE *logFile);
void myWarnClear(uChar f_closeFile, char **msg);

int myWarn_Note(const char *fmt, ...);
int myWarn_Warn(const char *fmt, ...);
int myWarn_Err(const char *fmt, ...);
int myWarn_Loc(uChar f_errCode, const char *file, int lineNum,
               const char *fmt, ...);

#define myWarn_Note1Arg(f) myWarn_Loc(1, __FILE__, __LINE__, f)
#define myWarn_Warn1Arg(f) myWarn_Loc(2, __FILE__, __LINE__, f)
#define myWarn_Err1Arg(f) myWarn_Loc(3, __FILE__, __LINE__, f)

#define myWarn_Note2Arg(f,g) myWarn_Loc(1, __FILE__, __LINE__, f, g)
#define myWarn_Warn2Arg(f,g) myWarn_Loc(2, __FILE__, __LINE__, f, g)
#define myWarn_Err2Arg(f,g) myWarn_Loc(3, __FILE__, __LINE__, f, g)

#define myWarn_Note3Arg(f,g,h) myWarn_Loc(1, __FILE__, __LINE__, f, g, h)
#define myWarn_Warn3Arg(f,g,h) myWarn_Loc(2, __FILE__, __LINE__, f, g, h)
#define myWarn_Err3Arg(f,g,h) myWarn_Loc(3, __FILE__, __LINE__, f, g, h)

#define myWarn_Note4Arg(f,g,h,i) myWarn_Loc(1, __FILE__, __LINE__, f, g, h, i)
#define myWarn_Warn4Arg(f,g,h,i) myWarn_Loc(2, __FILE__, __LINE__, f, g, h, i)
#define myWarn_Err4Arg(f,g,h,i) myWarn_Loc(3, __FILE__, __LINE__, f, g, h, i)

/* From myutil.h */
int reallocFGets(char **S, size_t *Size, FILE *fp);
char *strncpyTrim (char *dst, const char *src, size_t n);
int mySplit(const char *data, char symbol, size_t *Argc, char ***Argv,
            char f_trim);
int myAtoI(const char *s, sInt4 *value);
int myAtoI_Len(char *s, size_t len, sInt4 *value);
int myAtoF(const char *s, double *value);
int myAtoF_Len(char *s, size_t len, double *value);
double myRound(double x, uChar place);
int myDoubleEq(double x, double y, uChar place);
void strTrim(char *str);
void strToLower(char *s);
void strToUpper(char *s);
int ListSearch(char **List, size_t N, const char *s);
/* GetIndexFromStr -->Renamed--> ListSearch */
/*int GetIndexFromStr (const char *str, char **Opt, int *Index);*/
int fileAllocNewExten(const char *name, const char *ext, char **newName);
double myCyclicBounds(double value, double min, double max);
size_t myCntNumLines(FILE * fp);

#include "mycomplex.h"

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
int fileBitRead(void *Dst, size_t dstLen, FILE *fp, size_t numBits,
                uChar *gbuf, sChar *gbufLoc);
int fileBitWrite(const void *Src, size_t srcLen, FILE *fp, size_t numBits,
                 uChar *pbuf, sChar *pbufLoc);

/* From sphere.c */
void BearCompute(char f_radian, double lat1, double lon1, double lat2,
                 double lon2, double *theta);
void DistCompute(double R, double lat1, double lon1, double lat2, double lon2,
                 double *dist);
void LatLonCompute(double R, double lat1, double lon1, double dist,
                   double bear, double *lat2, double *lon2);

/* From spline.c */
int mySplineSnd(const double *x, const double *y, size_t n, double A,
                double B, double *y2);
int mySplintCube(const double *xa, const double *ya, const double *y2a,
                 size_t n, double x, double *y);
int mySplintLine(const double *xa, const double *ya, size_t n, double x,
                 double *y);

#ifdef PROVIDE_OLD_SPLINE
void spline2(float x[], float y[], size_t n0, float A, float B, float y2[]);
void splint(float xa[], float ya[], float y2a[], size_t n, float x, float *y);
#endif

/* From probability.c */
double probNormDist(double x);
double probNormBound(double prob, double left);

/* From clock.c */
void Clock_PrintDate (double clock, sInt4 *year, int *month, int *day,
                      int *hour, int *min, double *sec);
void Clock_ScanDate (double *clock, sInt4 year, int mon, int day);
int Clock_PrintBuffDate (double clock, char *buffer, size_t len);
int Clock_ScanBuffDate (double *clock, char *buffer);
int Clock_ScanMonth (char *ptr);
void Clock_PrintMonth3 (int mon, char *buffer, int buffLen);

/* From shpfile.c */
#ifndef LATLON_TYPE
#define LATLON_TYPE
typedef struct {
   double lat, lon;
} LatLon;
#endif

int shpCreatePnt(const char *Filename, const LatLon *dp, size_t numDP);
int shpCreatePrj(const char *Filename, const char *gcs, const char *datum,
                 const char *spheroid, double A, double B);

#ifndef DBFFILE_TYPE
#define DBFFILE_TYPE
#endif

void *DbfOpen(const char *Filename, char access);
int DbfAddField(void * Dbf, const char *name, uChar type, uChar len,
                uChar decimal);
int DbfWriteHead(void * Dbf);
int DbfSetCurRec(void * Dbf, ...);
void DbfWriteCurRec(void * Dbf);
sShort2 DbfFindCol(void * Dbf, const char *name);
void DbfPrintColHeader(void * Dbf);
void DbfGetSizeSpecs(void *Dbf, sInt4 *numRec, sShort2 *recLen,
                     sShort2 *numCol);
int DbfReadCurRec(void * Dbf, sInt4 recNum, char *ptr, int field);
int DbfClose(void * Dbf);

#endif
