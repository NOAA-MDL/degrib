/*****************************************************************************
 * myutil.c
 *
 * DESCRIPTION
 *    This file contains some simple utility functions.
 *
 * HISTORY
 *   12/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#ifndef MYUTIL_H
#define MYUTIL_H

#include <stdio.h>
#include <time.h>

int reallocFGets (char **Ptr, int *LenBuff, FILE *fp);

void mySplit (char *data, char symbol, int *Argc, char ***Argv, char f_trim);

int myIsReal (char *ptr, double *value);

int FileCopy (char *fileIn, char *fileOut);

void FileTail (char *fileName, char *tail);

double myRound (double data, signed char place);

void strTrim (char *str);

void strTrimRight (char *str, char c);

void strCompact (char *str, char c);

void strReplace (char *str, char c1, char c2);

void strToUpper (char *str);

void strToLower (char *str);

int GetIndexFromStr (char *arg, char **Opt, int *Index);

int myParseTime (char *is, time_t * AnsTime);

#endif
