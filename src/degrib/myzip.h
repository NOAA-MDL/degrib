#ifndef MYZIP_H
#define MYZIP_H

#include "zip.h"

typedef struct {
   zipFile zf;
   FILE *fp;
   int f_useZip;
} myZipFile;

/* open the zip file, or set f_useZip to 0 */
/* allocates space for myzipFile structure */
/* Choices for append to zipOpen is
 *  0 = APPEND_STATUS_CREATE
 *  1 = APPEND_STATUS_CREATEAFTER
 *  2 = APPEND_STATUS_ADDINZIP
 * Currently if f_append, APPEND_STATUS_ADDINZIP else APPEND_STATUS_CREATE
 */
myZipFile * myZipInit (const char *filename, int f_useZip, int f_append);

/* open the file in the zip file, or open the unziped file */
/* attrib is typical fopen attributes.
 * sec,min,hour, mday,mon,year are attributes to associate with the zip file
 * for mon, Jan=0. */
int myZip_fopen (myZipFile *zp, const char *filename, const char *attrib,
                 int year, int mon, int day, int hour, int min, int sec);


/* print a NULL terminated string to the file in the zip file, or the opened
 * unziped file.  The file in the zip file does not have CRLF.  The file out
 * of the zip file has CRLF based on whether myZip_fopen was called with "wt"
 * or with "wb".  Use "wb" to be consistent and to create unix flavor ASCII.
  */
int myZip_fputs (const char *s, const myZipFile *zp);

/* closes the file in the zip file, or closes the unziped file */
int myZip_fclose (myZipFile *zp);

/* closes the zip file, or set f_useZip to 0 */
/* frees space for myzipFile structure */
int myZipClose (myZipFile *zp);

#endif
