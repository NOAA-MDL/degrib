/*****************************************************************************
 * database.c
 *
 * DESCRIPTION
 *    This file creates a .flx index which allows us to treat a collection of
 * .flt files as a database, or combine the .flt files into a large "Cube"
 * format.

 * HISTORY
 *   7/2003 Arthur Taylor (MDL / RSIS): Started experimenting with.
 *   8/2003 AAT: Revisited.
 *
 * NOTES
 *****************************************************************************
 */
#ifndef DATABASE_H
#define DATABASE_H

#include <time.h>
#include "type.h"

#define GDSLEN 129
#define HEADLEN 20

#ifdef FLXTYPE_STRUCTURE
typedef struct {
   double valTime;
   char fltName[256]; /* strlen (fltName) <= 254 */
                      /* assumed same dir as .flx file */
   sInt4 offset;      /* offset into file */
   uChar f_endian;
   uChar f_scan;
   uShort2 numEntry;
   char **entry;
} flxPdsType;

typedef struct {
   char elem[256];    /* strlen (elem) <= 254 */
   double refTime;
   char unit[256];    /* strlen (unit) <= 254 */
   char comment[256]; /* strlen (comment) <= 254 */
   uShort2 gdsNum;     /* # in gds table [1..numGds] */
   uShort2 center;
   uShort2 subCenter;
   uShort2 numPds;
   flxPdsType *pds;
} flxSupHeadType;

typedef struct {
   uShort2 numGds;
   gdsType *gdsRay;
   uShort2 numSupHead;
   flxSupHeadType *supHead;
} flxType;
#endif

void ReadGDSBuffer (char *ptr, gdsType * gds);

uShort2 InsertGDS (char **flxArray, int *flxArrayLen, gdsType * gds);

int InsertPDS (char **flxArray, int *flxArrayLen, char *elem, time_t refTime,
               char *unit, char *comment, uShort2 gdsNum, uShort2 center,
               uShort2 subCenter, time_t validTime, char *fltName,
               sInt4 fltOffset, uChar endian, uChar scan, char **table,
               int tableLen);

void ReadPDSBuff (char *pdsPtr, double *validTime, char *dataFile,
                  sInt4 *dataOffset, uChar *endian, uChar *scan,
                  uShort2 *numTable, char ***table, char **nextPds);
void ReadSupPDSBuff (char *sPtr, char *elem, double *refTime, char *unit,
                     char *comment, uShort2 *gdsNum, uShort2 *center,
                     uShort2 *subCenter, uShort2 *numPDS, char **pdsPtr,
                     sInt4 *lenTotPds);

int PrintFLXBuffer (char *flxArray, int flxArrayLen);

void CreateFLX (char **flxArray, int *flxArrayLen);

int ReadFLX (const char *filename, char **flxArray, int *flxArrayLen);
/*
int ReadFLX2 (FILE *fp, char **flxArray, int *flxArrayLen);
*/

int Asc2Flx (char *inFile, char *outFile);

int WriteFLX (char *filename, char *flxArray, int flxArrayLen);

#endif
