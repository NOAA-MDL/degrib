#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zip.h"

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
