#ifndef ALLOCSPRINTF_H
#define ALLOCSPRINTF_H

#include <stdarg.h>

/* From allocSprintf.c */
int allocSprintf(char **Ptr, size_t *Size, const char *fmt, va_list ap);
int mallocSprintf(char **Ptr, const char *fmt, ...);
int reallocSprintf(char **Ptr, const char *fmt, ...);

#endif
