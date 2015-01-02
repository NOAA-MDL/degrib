/*****************************************************************************
 * dbffile.c
 *
 * DESCRIPTION
 *    This file contains some functions to save to a .dbf file
 *
 * HISTORY
 *  3/2007 Arthur Taylor (MDL): Created.
 *
 * NOTES
 ****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "libaat.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

/* Based on max(sShort2) = 32768 */
/* numCol = (32768 - 33) / 32 = 1022 */
#define MAX_NUMCOL 1022

/* Max Reclen is 4000 based on:
 * http://www.cs.cornell.edu/Courses/cs212/2001fa/Project/Part1/dbf.htm
 */
#define MAX_RECLEN 4000

typedef struct {
   char name[12];       /* Field name. */
   uChar type;          /* Field type. */
   uChar length;        /* Field length */
   uChar decimal;       /* Field decimals. */
   uChar f_keep;        /* I added.  1 if we want this column 0 otherwise. */
   sShort2 offset;      /* I added.  Offset to add to get to this field in
                         * the curRec */
} dbfColType;

typedef struct {
   FILE *fp;            /* Opened file to write to / read from. */
   char access;         /* access method 'w', 'r', 'a' */
   sInt4 numRec;        /* number of records in file */
   sShort2 recLen;      /* Size of a record. */
   sShort2 numCol;      /* number of columns in the file */
   dbfColType *col;     /* Array of columns in the file */
   char *curRec;        /* Current record allocated to recLen */
   sInt4 curNum;        /* Current record number [0..numRec) */
} DBFFILE;

void *DbfOpen(const char *Filename, char access)
{
   char *filename;
   DBFFILE *dbf;
   FILE *fp;
   uChar header[4];     /* Header info for dbf. */
   sShort2 sizeHead;    /* The size of the header record. */
   sInt4 Offset;        /* Used to move around in the file. */
   sShort2 i;           /* Counter over number of columns. */
   char *ptr;

   if (fileAllocNewExten(Filename, ".dbf", &filename) != 0) {
      return NULL;
   }
   if (access == 'w') {
      if ((fp = fopen(filename, "w+b")) == NULL) {
         myWarn_Err2Arg("Problems opening %s for write.\n", filename);
         free(filename);
         return NULL;
      }
   } else if (access == 'a') {
      if ((fp = fopen(filename, "r+b")) == NULL) {
         myWarn_Err2Arg("Problems opening %s for append.\n", filename);
         free(filename);
         return NULL;
      }
   } else if (access == 'r') {
      if ((fp = fopen(filename, "rb")) == NULL) {
         myWarn_Err2Arg("Problems opening %s for read.\n", filename);
         free(filename);
         return NULL;
      }
   } else {
      myWarn_Err2Arg("Access should be 'w','a','r', not '%c'\n", access);
      free(filename);
      return NULL;
   }
   free(filename);
   if ((dbf = (DBFFILE *) malloc(sizeof(DBFFILE))) == NULL) {
      myWarn_Err1Arg("Ran out of memory\n");
      fclose(fp);
      return NULL;
   }
   dbf->fp = fp;
   dbf->access = access;
   if (access == 'w') {
      dbf->recLen = 1;  /* 1 for the blank character. */
      if ((ptr = (char *)malloc((dbf->recLen + 1) * sizeof(char))) == NULL) {
         myWarn_Err1Arg("Ran out of memory\n");
         free(dbf);
         fclose(fp);
         return NULL;
      }
      dbf->curRec = ptr;
      dbf->curRec[0] = ' ';
      dbf->numRec = 0;
      dbf->curNum = 0;
      dbf->numCol = 0;
      dbf->col = NULL;
      return (void *)dbf;
   }

   /* Read the dbf header. */
   fread(header, sizeof(char), 4, fp);
   FREAD_LIT(&(dbf->numRec), sizeof(sInt4), 1, fp);
   FREAD_LIT(&sizeHead, sizeof(sShort2), 1, fp);
   dbf->numCol = (sizeHead - 1 - 32) / 32;
   FREAD_LIT(&(dbf->recLen), sizeof(sShort2), 1, fp);
   if ((ptr = (char *)malloc((dbf->recLen + 1) * sizeof(char))) == NULL) {
      myWarn_Err1Arg("Ran out of memory\n");
      free(dbf);
      fclose(fp);
      return NULL;
   }
   dbf->curRec = ptr;
   dbf->curNum = -1;
   Offset = 32;
   /* Skip the reserved. */
   fseek(fp, Offset, SEEK_SET);

   /* Read the column header info. */
   dbf->col = (dbfColType *) malloc(dbf->numCol * sizeof(dbfColType));
   if (dbf->col == NULL) {
      myWarn_Err1Arg("Ran out of memory\n");
      free(dbf);
      fclose(fp);
      return NULL;
   }
   for (i = 0; i < dbf->numCol; ++i) {
      fread(dbf->col[i].name, 1, 11, fp);
      dbf->col[i].name[11] = '\0';
      dbf->col[i].type = fgetc(fp);
      Offset += 11 + 1 + 4;
      fseek(fp, Offset, SEEK_SET);
      dbf->col[i].length = fgetc(fp);
      dbf->col[i].decimal = fgetc(fp);
      Offset += 32 - (11 + 1 + 4);
      fseek(fp, Offset, SEEK_SET);
      dbf->col[i].f_keep = 1;
      if (i == 0) {
         dbf->col[i].offset = 1;
      } else {
         dbf->col[i].offset = dbf->col[i - 1].offset + dbf->col[i - 1].length;
      }
   }
   if (fgetc(fp) == 13) {
      return dbf;
   } else {
      myWarn_Err1Arg("Headers section did not end in a '13'\n");
      free(dbf->col);
      free(dbf);
      fclose(fp);
      return NULL;
   }
}

int DbfAddField(void * Dbf, const char *name, uChar type, uChar len,
                uChar decimal)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;
   dbfColType *cur;     /* current column index. */
   sShort2 offset;

   /* Check that type is one of: 'C' (String), 'F' (Float), 'N' (Numeric,
    * with or without decimal), ... */
   /* Don't allow 'M' (Memo: 10 digits .DBT block ptr) */
   /* Don't allow 'D' (Date), 'L' (Logical) */
   if ((type != 'C') && (type != 'F') && (type != 'N') &&
       (type != 'L') && (type != 'D')) {
      myWarn_Err2Arg("Invalid field type '%c'\n", type);
      return -1;
   }
   if ((type == 'L') && (len != 1)) {
      myWarn_Err2Arg("Logical fields have length 1, not '%d'\n", len);
      return -1;
   }
   if ((type == 'D') && (len != 8)) {
      myWarn_Err2Arg("Logical fields have length 8, not '%d'\n", len);
      return -1;
   }
   if (dbf->access == 'r') {
      myWarn_Err1Arg("Opened file for Read, can't add column\n");
      return -2;
   }
   if (dbf->numCol == MAX_NUMCOL) {
      myWarn_Err1Arg("Have reached the maximum number of columns\n");
      return -3;
   }
   if (dbf->numCol == 0) {
      offset = 1;
   } else {
      cur = dbf->col + (dbf->numCol - 1);
      offset = cur->offset + cur->length;
   }
   dbf->numCol++;
   dbf->col = (dbfColType *) realloc((void *)dbf->col,
                                     dbf->numCol * sizeof(dbfColType));
   if (dbf->col == NULL) {
      myWarn_Err1Arg("Ran out of memory\n");
      return -4;
   }
   /* Update the record length */
   dbf->recLen += len;
   dbf->curRec = (char *)realloc(dbf->curRec,
                                 (dbf->recLen + 1) * sizeof(char));
   if (dbf->curRec == NULL) {
      myWarn_Err1Arg("Ran out of memory\n");
      return -4;
   }
   cur = dbf->col + (dbf->numCol - 1);
   strncpy(cur->name, name, 11);
   cur->name[11] = '\0';
   cur->type = type;
   cur->length = len;
   cur->decimal = decimal;
   cur->f_keep = 1;
   cur->offset = offset;
   if (dbf->access == 'w') {
      return 0;
   }
   if (dbf->access == 'a') {
      myWarn_Err1Arg("Opened file for Append, can't add field (yet)\n");
      return -2;
   }
   return 0;
}

int DbfWriteHead(void * Dbf)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;
   uChar header[4] = { 3, 101, 4, 20 }; /* Header info for dbf. */
   sShort2 sizeHead;    /* The size of the header record. */
   sInt4 reserved[] = { 0, 0, 0, 0, 0 }; /* need 20 bytes of 0. */
   sShort2 i;           /* counter over number of columns. */
   sInt4 address = 0;   /* Address to find data? */
   uChar uc_temp;       /* Temp. storage of type unsigned char. */

   if (dbf->access == 'r') {
      myWarn_Err1Arg("Opened file for Read, can't write header\n");
      return -1;
   }
   if (dbf->access == 'a') {
      myWarn_Err1Arg("Opened file for Append, can't write header\n");
      return -1;
   }

   fwrite(header, sizeof(char), 4, dbf->fp);
   /* Write number of records, size of header, then length of record. */
   FWRITE_LIT(&(dbf->numRec), sizeof(sInt4), 1, dbf->fp);
   sizeHead = (sShort2)(32 + 32 * dbf->numCol + 1);
   FWRITE_LIT(&sizeHead, sizeof(sShort2), 1, dbf->fp);
   FWRITE_LIT(&(dbf->recLen), sizeof(sShort2), 1, dbf->fp);
   /* Write Reserved bytes.. 20 bytes of them.. all 0. */
   fwrite(reserved, sizeof(char), 20, dbf->fp);

   for (i = 0; i < dbf->numCol; ++i) {
      /* Already taken care of padding with 0's since we did a strncpy */
      fwrite(dbf->col[i].name, sizeof(char), 11, dbf->fp);
      fputc(dbf->col[i].type, dbf->fp);
      /* Should be FWRITE_LIT, but doesn't matter. 11/8/2004 */
      fwrite(&(address), sizeof(sInt4), 1, dbf->fp); /* with address 0 */
      fputc(dbf->col[i].length, dbf->fp);
      fputc(dbf->col[i].decimal, dbf->fp);
      fwrite(reserved, sizeof(char), 14, dbf->fp); /* reserved (has 0's) */
   }
   /* Write trailing header character. */
   uc_temp = 13;
   fputc(uc_temp, dbf->fp);
   return 0;
}

int DbfSetCurRec(void * Dbf, ...)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;
   va_list ap;          /* Contains the variable list data. */
   int index = 1;       /* Index into curRec as to where to write. */
   char *sval;          /* Dereference 'C' type data */
   double dval;         /* Dereference 'N' or 'F' type data */
   int ival;            /* Dereference 'L' or 'D' type data */
   sShort2 i;           /* Counter over number of columns. */
   char bufpart[330];   /* Used to prevent buffer overruns (double/date) */

   va_start(ap, Dbf);   /* make ap point to 1st unnamed arg. */
   for (i = 0; i < dbf->numCol; ++i) {
      switch (dbf->col[i].type) {
         case 'C':
            sval = va_arg(ap, char *);
            strncpy(dbf->curRec + index, sval, dbf->col[i].length);
            break;
         case 'N':
         case 'F':
            dval = va_arg(ap, double);
            sprintf(bufpart, "%*.*f", dbf->col[i].length, dbf->col[i].decimal,
                    dval);
            strncpy(dbf->curRec + index, bufpart, dbf->col[i].length);
            break;
         case 'L':
            ival = va_arg(ap, int);
            if ((ival != 'Y') && (ival != 'y') && (ival != 'N') &&
                (ival != 'n') && (ival != 'T') && (ival != 't') &&
                (ival != 'F') && (ival != 'f') && (ival != '?')) {
               myWarn_Err2Arg("Logical fields are in set of 'YyNnTtFf?', "
                              "not '%c'\n", ival);
               va_end(ap); /* clean up when done. */
               return -1;
            }
            dbf->curRec[index] = ival;
            break;
         case 'D':
            ival = va_arg(ap, int);
            sprintf(bufpart, "%08d", ival);
            strncpy(dbf->curRec + index, bufpart, 8);
            break;
         default:
            myWarn_Err2Arg("Unhandled field type '%c'\n", dbf->col[i].type);
            va_end(ap); /* clean up when done. */
            return -1;
      }
      index += dbf->col[i].length;
   }
   va_end(ap);          /* clean up when done. */
   return 0;
}

void DbfWriteCurRec(void * Dbf)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;

   fwrite(dbf->curRec, sizeof(char), dbf->recLen, dbf->fp);
   dbf->numRec++;
   dbf->curNum = dbf->numRec;
}

sShort2 DbfFindCol(void * Dbf, const char *name)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;
   int i;

   for (i = 0; i < dbf->numCol; ++i) {
      if (strcmp (name, dbf->col[i].name) == 0) {
         return i;
      }
   }
   return -1;
}

void DbfPrintColHeader(void * Dbf)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;
   int i;

   for (i = 0; i < dbf->numCol; ++i) {
      printf ("name=%-11s type=%c size=%d.%d offset=%d\n", dbf->col[i].name,
              dbf->col[i].type, dbf->col[i].length, dbf->col[i].decimal,
              dbf->col[i].offset);
   }
}

void DbfGetSizeSpecs(void *Dbf, sInt4 *numRec, sShort2 *recLen,
                     sShort2 *numCol)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;
   *numRec = dbf->numRec;
   *recLen = dbf->recLen;
   *numCol = dbf->numCol;
}

/*
 * if curNum != i, then jump to i, and read it in
 * if field = -1 store whole record in buffer
 * if field = #, store just that column in buffer
 */
/* Speed advantage to reading it in sequence, since we don't have as
 * many seeks.  Could cause problems if user reads from the file and doesn't
 * properly update curNum;
 */
int DbfReadCurRec(void * Dbf, sInt4 recNum, char *ptr, int field)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;
   sInt4 offset;

   /* check that input is ok */
   if ((field < -1) || (field > dbf->numCol)) {
      myWarn_Err2Arg("field %d is out of range.\n", field);
      return -1;
   }
   if ((recNum < 0) || (recNum > dbf->numRec)) {
      myWarn_Err2Arg("record %d is out of range.\n", recNum);
      return -1;
   }
   if (recNum != dbf->curNum) {
      if ((recNum != dbf->curNum + 1) || (recNum == 0)) {
         if (recNum > 0) {
            offset = ((dbf->numCol * 32) + 1 + 32) + (recNum - 1) * dbf->recLen;
         } else {
            offset = ((dbf->numCol * 32) + 1 + 32);
         }
         dbf->curNum = recNum;
         fseek(dbf->fp, offset, SEEK_SET);
      } else {
         dbf->curNum++;
      }
      fread(dbf->curRec, sizeof(char), dbf->recLen, dbf->fp);
   }
   if (field == -1) {
      strncpy (ptr, dbf->curRec, dbf->recLen);
      ptr[dbf->recLen] = '\0';
   } else {
      strncpy (ptr, dbf->curRec + dbf->col[field].offset,
               dbf->col[field].length);
      ptr[dbf->col[field].length] = '\0';
   }
   return 0;
}

int DbfClose(void * Dbf)
{
   DBFFILE *dbf = (DBFFILE *) Dbf;
   sInt4 totSize;       /* Total size of the .dbf file. */
   int ierr = 0;

   if (dbf->access == 'w') {
      /* Update file total # of records. */
      fseek(dbf->fp, 4, SEEK_SET);
      FWRITE_LIT(&(dbf->numRec), sizeof(sInt4), 1, dbf->fp);
      totSize = 1 + 32 + 32 * dbf->numCol + dbf->recLen * dbf->numRec;

      /* Check that .dbf is now the correct file size. */
      fseek(dbf->fp, 0L, SEEK_END);
      if (ftell(dbf->fp) != totSize) {
         myWarn_Err2Arg("dbf file is not %ld bytes long.\n", totSize);
         ierr = -1;
      }
   }
   fclose(dbf->fp);
   free(dbf->col);
   free(dbf->curRec);
   free(dbf);
   return ierr;
}
