#ifndef DBFFILE_H
#define DBFFILE_H

#ifndef DBFFILE_TYPE
#define DBFFILE_TYPE
#endif

#include <stdio.h>
#include "libaat_type.h"

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
