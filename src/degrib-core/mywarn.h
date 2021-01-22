#ifndef MYWARN_H
#define MYWARN_H

#include <stdio.h>
#include <stdarg.h>

#include "libaat_type.h"

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

#endif
