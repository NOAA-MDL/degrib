#ifndef MYUTIL_H
#define MYUTIL_H

#include <stdio.h>
#include <stdarg.h>

#include "libaat_type.h"

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

#endif
