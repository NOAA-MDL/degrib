#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myzip.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

/* open the zip file, or if we're not creating a zip file set f_useZip to 0 */
/* allocates space for myzipFile structure */
/* Choices for append to zipOpen is
 *  0 = APPEND_STATUS_CREATE
 *  1 = APPEND_STATUS_CREATEAFTER
 *  2 = APPEND_STATUS_ADDINZIP
 * Currently if f_append, APPEND_STATUS_ADDINZIP else APPEND_STATUS_CREATE
 */
myZipFile * myZipInit (const char *filename, int f_useZip, int f_append)
{
   myZipFile *zp;
#ifdef USEWIN32IOAPI
   zlib_filefunc_def ffunc;
#endif

   zp = (myZipFile *) malloc (sizeof (myZipFile));
   if (zp == NULL) {
      return NULL;
   }
   zp->f_useZip = f_useZip;
   zp->fp = NULL;
   zp->zf = NULL;
   if (! f_useZip) {
      return zp;
   }

#ifdef USEWIN32IOAPI
   fill_win32_filefunc (&ffunc);
   zp->zf = zipOpen2 (filename, f_append ? 2 : 0, NULL, &ffunc);
#else
   zp->zf = zipOpen (filename, f_append ? 2 : 0);
#endif
   if (zp->zf == NULL) {
      free (zp);
      return NULL;
   }
   return zp;
}

/* open the file in the zip file, or open the unziped file */
/* attrib is typical fopen attributes.
 * sec,min,hour, mday,mon,year are attributes to associate with the zip file
 * for mon, Jan=0. */
int myZip_fopen (myZipFile *zp, const char *filename, const char *attrib,
                 int year, int mon, int day, int hour, int min, int sec)
{
   zip_fileinfo zi;
   int err = 0;

   if (! zp->f_useZip) {
      zp->fp = fopen (filename, attrib);
      if (zp->fp == NULL) {
         return 1;
      }
   } else {
      zi.dosDate = 0;
      zi.internal_fa = 0;
      zi.external_fa = 0;
      zi.tmz_date.tm_sec = sec;
      zi.tmz_date.tm_min = min;
      zi.tmz_date.tm_hour = hour;
      zi.tmz_date.tm_mday = day;
      zi.tmz_date.tm_mon = mon;
      zi.tmz_date.tm_year = year;

      err = zipOpenNewFileInZip (zp->zf, filename, &zi, NULL, 0, NULL, 0,
                                 NULL, (Z_DEFAULT_COMPRESSION != 0) ?
                                 Z_DEFLATED : 0, Z_DEFAULT_COMPRESSION);
      if (err != ZIP_OK) {
         return 1;
      }
   }
   return 0;
}

/* print a NULL terminated string to the file in the zip file, or the opened
 * unziped file.  The file in the zip file does not have CRLF.  The file out
 * of the zip file has CRLF based on whether myZip_fopen was called with "wt"
 * or with "wb".  Use "wb" to be consistent and to create unix flavor ASCII.
  */
int myZip_fputs (const char *s, const myZipFile *zp)
{
   int err = 0;
   size_t len = strlen (s);

   if (! zp->f_useZip) {
      return fputs (s, zp->fp);
   }
   err = zipWriteInFileInZip (zp->zf, s, len);
   if (err < 0) {
      return EOF;
   }
   return len;
}

/* closes the file in the zip file, or closes the unziped file */
int myZip_fclose (myZipFile *zp)
{
   int err = 0;

   if (! zp->f_useZip) {
      return fclose (zp->fp);
   }
   err = zipCloseFileInZip (zp->zf);
   if (err != ZIP_OK) {
      return EOF;
   }
   return 0;
}

/* closes the zip file, or set f_useZip to 0 */
/* frees space for myzipFile structure */
int myZipClose (myZipFile *zp)
{
   if (zp->f_useZip) {
      if (zipClose (zp->zf, NULL) != ZIP_OK) {
         free (zp);
         return EOF;
      }
   }
   free (zp);
   return 0;
}

#ifdef TEST_MYZIP
int main(int argc, char **argv)
{
   myZipFile *zp;
   int f_useZip = 1;

   if ((zp = myZipInit ("arthur.zip", f_useZip, 0)) == NULL) {
      printf ("error opening %s\n", "arthur.zip");
      return 1;
   }

   if (myZip_fopen (zp, "arthur.txt", "wb", 2009, 0, 16, 1, 58, 59)) {
      printf ("Problems opening file\n");
   } else {
      if (myZip_fputs ("test PI=3.1415926535\n", zp) == EOF) {
         printf ("Error in writing to file in the zipfile\n");
      }
      if (myZip_fclose (zp) == EOF) {
         printf ("Error closing file in the zipfile\n");
      }
   }

   if (myZipClose (zp) == EOF) {
      printf ("Error closing the zipfile\n");
   }
   return 0;
}
#endif

#ifdef ORIG
#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)
int main(int argc, char **argv)
{
   char filename_try[MAXFILENAME+16];
   int err = 0;
   int size_buf = 0;
   void * buf = NULL;
   zipFile zf;
#ifdef USEWIN32IOAPI
   zlib_filefunc_def ffunc;
#endif
   int f_append = 0;  /* 2 means append, 0 means overwrite */

   strcpy (filename_try, "arthur.zip");

   size_buf = WRITEBUFFERSIZE;
   buf = (void *) malloc (size_buf);
   if (buf == NULL) {
      printf ("Error allocating memory\n");
      return ZIP_INTERNALERROR;
   }

#ifdef USEWIN32IOAPI
   fill_win32_filefunc (&ffunc);
   zf = zipOpen2 (filename_try, f_append, NULL, &ffunc);
#else
   zf = zipOpen (filename_try, f_append);
#endif
   if (zf == NULL) {
      printf ("error opening %s\n", filename_try);
      err = ZIP_ERRNO;
   }

   if (1==1) {
      const char * filenameinzip = "./ans/arthur.txt";
      zip_fileinfo zi;

      zi.dosDate = 0;
      zi.internal_fa = 0;
      zi.external_fa = 0;
      zi.tmz_date.tm_sec = 59;
      zi.tmz_date.tm_min = 58;
      zi.tmz_date.tm_hour = 1;
      /* 1/16/2009 */
      zi.tmz_date.tm_mday = 16;
      zi.tmz_date.tm_mon = 0;
      zi.tmz_date.tm_year = 2009;

      err = zipOpenNewFileInZip (zf, filenameinzip, &zi, NULL, 0, NULL, 0, NULL /* comment */,
                                 (Z_DEFAULT_COMPRESSION != 0) ? Z_DEFLATED : 0,
                                 Z_DEFAULT_COMPRESSION);

      if (err != ZIP_OK) {
         printf ("error in opening %s in zipfile\n", filenameinzip);
      } else {
         sprintf (buf, "test PI=3.1415926535\n");
         err = zipWriteInFileInZip (zf, buf, strlen (buf));
         if (err < 0) {
            printf ("error in writing %s in the zipfile\n",
                    filenameinzip);
         }
      }

      if (err < 0) {
         err = ZIP_ERRNO;
      } else {
         err = zipCloseFileInZip (zf);
         if (err != ZIP_OK) {
            printf ("error in closing %s in the zipfile\n", filenameinzip);
         }
      }
   }
   if (zipClose (zf, NULL) != ZIP_OK) {
      printf ("error in closing %s\n", filename_try);
   }

   free (buf);
   return 0;
}
#endif
