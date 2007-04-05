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
 * 1) Improvements: Put a creation time in the .flx file.
 * 2) Improvements: Missing value.
 *
 * .flx format: Little Endian, Binary. (USI = unsigned short int)
 *    (UC = unsigned Char) (ULI = unsigned 4 byte int) (LI = signed 4 byte)
 *
 * General idea: Probe walks through PDS based on user choices.  When it finds
 * a match, it checks the associated GDS and .flt file for that data.
 * 1) Imbedding GDS with PDS would make it difficult to jump around in the PDS
 *    and remember which GDS is appropriate.
 * 2) Creating an Index of the PDS would be difficult because "offsets" would
 *    change as GDS and Index grew.  Also to figure out what goes in the
 *    index, a number of choices as to what the future user would want to
 *    search on would need to be made.  Using an array of PDS allows the PDS
 *    to grow as needed (for example adding a Z component).
 *
 * Basic structure:
 *   [Header]
 *   [GDS Array]
 *   [PDS Super header]
 *      [PDS Array]
 *   [PDS Super header]
 *      [PDS Array]
 *   ...
 *
 * Header...
 *   [1..3] = "FLX"
 *   [4..7] = LI : File size.
 *   [8..20] = Reserved
 *
 * GDS...
 * USI : # of GDS (N)
 * N * sizeof (GDS) (each of fixed size)
 *   ULI : numPts
 *   2 * UC : projType, f_sphere
 *   2 * double : majEarth, minEarth in km
 *   2 * ULI : Nx, Ny
 *   2 * double : lat1, lon1
 *   UC : resFlag
 *   4 * double : orientLon, Dx, Dy, meshLat
 *   2 * UC = center, scan
 *   6 * double = lat2, lon2, scaleLat1, scaleLat2, southLat, southLon
 *   ======
 *   total size = 14 * double + 3 * ULI + 5 UC = 129
 *
 * USI : # of PDS Super Headers (M)
 * PDS Super Header...
 * LI : Size of Super Header and Associated PDS Array. (inclusive)
 *   (Set up this way for easier comparisons. size of supHead + pds Array,
 *    and # of pds in array grow, this part is constant.)
 *   USI : Size of Super Header. (inclusive)
 *   UC : len of element
 *   array char : element (max 255)               [sorted by this]
 *   double : time_t cast to double of RefTime
 *              [if same element, then largest (most recent) reftime is first]
 *   UC : len of unit
 *   array char : unit
 *   UC = len of comment
 *   array char = comment
 *   USI : integral number into GDS table.        [starts at 1]
 *   USI : Center (Who Produced it)
 *   USI : SubCenter (Who Produced it)
 * USI : # of PDS in Array.
 * PDS Array...
 *   USI : Size of PDS (inclusive)
 *   double : time_t cast to double of ValidTime
 *                       [smallest (shortest forecast period) validTime first]
 *   UC : len of filename
 *   array char : filename (no path info, (assumed same dir as .flx file))
 *   LI : offset into filename (typically 0)
 *   UC : Endian'ness of file. (0 is LittleEndian, 1 is BigEndian, typically 0)
 *   UC : scan of file. (bit 128 of scan => decrease x, bit 64 => increase y
 *                       bit 32 => column oriented.
 *                       bit 16 => reverse direction at end or row / column. )
 *                      (typically 0000 (flt file) or 0100 (-revFlt file))
 *   USI = # of entries in char table. (used for ugly strings).
 *                      (In theory an ugly string could be > 255 char)
 *     USI = len of table entry n
 *     array char = table entry n
 *****************************************************************************
 */
/*
 * This flag is used to test the database module.
 */
/* #define DATA_DEBUG */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "myassert.h"
#include "myutil.h"
#include "meta.h"
#include "tendian.h"
#include "type.h"
#include "database.h"
#include "clock.h"

/*****************************************************************************
 * BufferInsert() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To insert a char buffer into the middle of another char buffer,
 * realloc'ating space as needed.
 *
 * ARGUMENTS
 *    ansBuff = char buffer to insert into. (Input/Output)
 * ansBuffLen = Length of ansBuff. (Input/Output)
 *     offset = Where in ansBuff to insert. (Input)
 *     buffer = What to insert. (Input)
 *   numBytes = number of bytes from buffer to insert. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   0 = OK.
 *  -1 = offset was out of range.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *****************************************************************************
 */
static int BufferInsert (char **ansBuff, int *ansBuffLen, sInt4 offset,
                         char *buffer, int numBytes)
{
   int origLen;         /* The original length of ansBuff. */
   char *ptr;           /* Used to avoid the first offset bytes of ansBuff. */

   myAssert (*ansBuffLen >= 0);
   myAssert (buffer != NULL);
   myAssert (numBytes >= 0);
   myAssert (sizeof (char) == 1);

   origLen = *ansBuffLen;
   if ((origLen < offset) || (offset < 0)) {
      return -1;
   }
   *ansBuffLen += numBytes;
   *ansBuff = (char *) realloc ((void *) *ansBuff, *ansBuffLen);
   ptr = *ansBuff + offset;
   /* Move original data numBytes forward. There is the potential for overlap 
    * here, so we use memmove. */
   if (origLen != offset) {
      memmove (ptr + numBytes, ptr, origLen - offset);
   }
   /* Put in new data. */
   memcpy (ptr, buffer, numBytes);
   return 0;
}

/*****************************************************************************
 * BufferRemove() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To remove a set of bytes from the middle of a char buffer.
 *
 * ARGUMENTS
 *    ansBuff = char buffer to remove from. (Input/Output)
 * ansBuffLen = Length of ansBuff. (Input/Output)
 *     offset = Where in ansBuff to remove. (Input)
 *   numBytes = how many bytes to remove. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   0 = OK.
 *  -1 = offset was out of range.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *****************************************************************************
 */
static int BufferRemove (char *ansBuff, int *ansBuffLen, sInt4 offset,
                         int numBytes)
{
   myAssert (ansBuff != NULL);
   myAssert (*ansBuffLen >= 0);
   myAssert (numBytes >= 0);

   if ((*ansBuffLen < offset) || (offset < 0)) {
      return -1;
   }
   if (*ansBuffLen <= offset + numBytes) {
      *ansBuffLen = offset;
      return 0;
   }
   /* (memmove) original data, since there is the potential for overlap. */
   memmove (ansBuff + offset, ansBuff + offset + numBytes,
            *ansBuffLen - (offset + numBytes));
   *ansBuffLen -= numBytes;
   return 0;
}

/*****************************************************************************
 * ReadGDSBuffer() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Read the GDS from a char buffer.
 *
 * ARGUMENTS
 * ptr = char buffer to read from. (Input)
 * gds = GDS Structure to fill. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *****************************************************************************
 */
void ReadGDSBuffer (char *ptr, gdsType *gds)
{
   myAssert (ptr != NULL);
   myAssert (gds != NULL);
   myAssert (sizeof (double) == 8);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (char) == 1);

   MEMCPY_LIT (&(gds->numPts), ptr, sizeof (sInt4));
   ptr += 4;
   gds->projType = *ptr;
   ptr++;
   gds->f_sphere = *ptr;
   ptr++;
   MEMCPY_LIT (&(gds->majEarth), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->minEarth), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->Nx), ptr, sizeof (sInt4));
   ptr += 4;
   MEMCPY_LIT (&(gds->Ny), ptr, sizeof (sInt4));
   ptr += 4;
   MEMCPY_LIT (&(gds->lat1), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->lon1), ptr, sizeof (double));
   ptr += 8;
   gds->resFlag = *ptr;
   ptr++;
   MEMCPY_LIT (&(gds->orientLon), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->Dx), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->Dy), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->meshLat), ptr, sizeof (double));
   ptr += 8;
   gds->center = *ptr;
   ptr++;
   gds->scan = *ptr;
   ptr++;
   MEMCPY_LIT (&(gds->lat2), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->lon2), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->scaleLat1), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->scaleLat2), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->southLat), ptr, sizeof (double));
   ptr += 8;
   MEMCPY_LIT (&(gds->southLon), ptr, sizeof (double));
   gds->poleLat = 0;
   gds->poleLon = 0;
   gds->stretchFactor = 0;
   gds->f_typeLatLon = 0;
   gds->angleRotate = 0;
}

/*****************************************************************************
 * WriteGDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Write the GDS to a buffer, which is then inserted into a file.
 *
 * ARGUMENTS
 *    gds = GDS Structure to write. (Input)
 * buffer = Where to write GDS to.  Assumed at least GDSLEN bytes (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Commented.
 *
 * NOTES
 *****************************************************************************
 */
static void WriteGDS (gdsType *gds, char *buffer)
{
   myAssert (gds != NULL);
   myAssert (buffer != NULL);
   myAssert (sizeof (double) == 8);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (char) == 1);

   MEMCPY_LIT (buffer, &(gds->numPts), sizeof (sInt4));
   buffer += 4;
   *buffer = gds->projType;
   buffer++;
   *buffer = gds->f_sphere;
   buffer++;
   MEMCPY_LIT (buffer, &(gds->majEarth), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->minEarth), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->Nx), sizeof (sInt4));
   buffer += 4;
   MEMCPY_LIT (buffer, &(gds->Ny), sizeof (sInt4));
   buffer += 4;
   MEMCPY_LIT (buffer, &(gds->lat1), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->lon1), sizeof (double));
   buffer += 8;
   *buffer = gds->resFlag;
   buffer++;
   MEMCPY_LIT (buffer, &(gds->orientLon), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->Dx), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->Dy), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->meshLat), sizeof (double));
   buffer += 8;
   *buffer = gds->center;
   buffer++;
   *buffer = gds->scan;
   buffer++;
   MEMCPY_LIT (buffer, &(gds->lat2), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->lon2), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->scaleLat1), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->scaleLat2), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->southLat), sizeof (double));
   buffer += 8;
   MEMCPY_LIT (buffer, &(gds->southLon), sizeof (double));
}

/*****************************************************************************
 * InsertGDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Check if a GDS is in the char array already.  If it is, return which
 * number it is, otherwise Insert it into the char array.
 *
 * ARGUMENTS
 *    flxArray = The char array to insert into. (Output)
 * flxArrayLen = The length of flxArray. (Input/Output)
 *         gds = GDS data to instert. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: unsigned short int
 *   1..n : Which record matched the given GDS
 *        : (where n is how many GDS there were before call)
 *   n+1  : If we added a GDS
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Commented.
 *
 * NOTES
 *****************************************************************************
 */
uShort2 InsertGDS (char **flxArray, int *flxArrayLen, gdsType *gds)
{
   sInt4 offset;        /* Keep track of where we are in the array. */
   uShort2 numGDS;      /* How many GDS in the GDS array. */
   int i;               /* loop counter over the GDS array. */
   char buffer[GDSLEN]; /* The GDS in char form form ready to insert. */
   char *ptr;           /* A pointer to where we are in the array. */

   myAssert (gds != NULL);
   myAssert (*flxArrayLen >= HEADLEN + 2 + 2);
   myAssert (*flxArray != NULL);
   myAssert (sizeof (uShort2) == 2);

   /* Write the GDS to buffer for easier comparisons, and later for insert. */
   WriteGDS (gds, buffer);

   /* Check if GDS array has a given GDS. */
   ptr = *flxArray + HEADLEN;
   MEMCPY_LIT (&numGDS, ptr, sizeof (uShort2));
   ptr += 2;
   for (i = 0; i < numGDS; i++) {
      if (memcmp (ptr, buffer, GDSLEN) == 0) {
         return (uShort2) (i + 1);
      }
      ptr += GDSLEN;
   }

   /* Append / insert GDS. */
   offset = HEADLEN + 2 + GDSLEN * numGDS;
   BufferInsert (flxArray, flxArrayLen, offset, buffer, GDSLEN);

   /* Update number of GDS in GDS Array. */
   numGDS++;
   MEMCPY_LIT (*flxArray + HEADLEN, &numGDS, sizeof (uShort2));

   return numGDS;
}

/*****************************************************************************
 * WriteSupPDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Write the Super header part of a PDS to a buffer.
 *
 * ARGUMENTS
 *    buffer = The char array to write the super header to. (Output)
 * bufferLen = The length of buffer. (Input/Output)
 *      elem = The weather variable name. (Input)
 *   refTime = The reference time as a time_t cast to a double. (Input)
 *      unit = The Unit of the weather variable (Input)
 *   comment = Any comments associated with this variable (Input)
 *    gdsNum = The GDS number associated with this variable (In)
 *    center = The originating center id. (Input)
 * subCenter = The originating sub center's id. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *****************************************************************************
 */
static void WriteSupPDS (char **buffer, int *bufferLen, char *elem,
                         double refTime, char *unit, char *comment,
                         uShort2 gdsNum, uShort2 center, uShort2 subCenter)
{
   char *ptr;           /* A pointer to where we are in the array. */
   uChar elemLen;       /* The length of elem (<= 254) */
   uChar unitLen;       /* The length of unit (<= 254) */
   uChar commentLen;    /* The length of comment (<= 254) */
   uShort2 supLen;      /* The total size of the super PDS header. */

   myAssert (elem != NULL);
   myAssert (unit != NULL);
   myAssert (comment != NULL);
   myAssert (sizeof (double) == 8);
   myAssert (sizeof (uShort2) == 2);
   myAssert (sizeof (char) == 1);

   if (strlen (elem) > 254) {
      elemLen = 254;
   } else {
      elemLen = strlen (elem);
   }
   if (strlen (unit) > 254) {
      unitLen = 254;
   } else {
      unitLen = strlen (unit);
   }
   if (strlen (comment) > 254) {
      commentLen = 254;
   } else {
      commentLen = strlen (comment);
   }
   *bufferLen = 1 + elemLen + 1 + unitLen + 1 + commentLen + sizeof (double)
         + 4 * sizeof (uShort2);
   supLen = (uShort2) *bufferLen;
   *buffer = (char *) malloc (*bufferLen);
   ptr = *buffer;
   MEMCPY_LIT (ptr, &supLen, sizeof (uShort2));
   ptr += 2;
   *ptr = elemLen;
   ptr++;
   memcpy (ptr, elem, elemLen);
   ptr += elemLen;
   MEMCPY_LIT (ptr, &refTime, sizeof (double));
   ptr += 8;
   *ptr = unitLen;
   ptr++;
   memcpy (ptr, unit, unitLen);
   ptr += unitLen;
   *ptr = commentLen;
   ptr++;
   memcpy (ptr, comment, commentLen);
   ptr += commentLen;
   MEMCPY_LIT (ptr, &gdsNum, sizeof (uShort2));
   ptr += 2;
   MEMCPY_LIT (ptr, &center, sizeof (uShort2));
   ptr += 2;
   MEMCPY_LIT (ptr, &subCenter, sizeof (uShort2));
}

/*****************************************************************************
 * WritePDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Write the PDS to a buffer.
 *
 * ARGUMENTS
 *    buffer = The char array to write the PDS to. (Output)
 * bufferLen = The length of buffer. (Input/Output)
 * validTime = The valid time as a time_t cast to a double. (Input)
 *   fltName = The file name of where to look for the associated data. (In)
 * fltOffset = Where in the fltName file to look for data. (Input)
 *    endian = What endian'ness is the file 'fltName'  (Input)
 *      scan = What orientation is the file. (Input)
 *     table = Any extra ASCII strings to associate with this data. (Input)
 *  tableLen = Length of table.
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *   Assumes that fltName <= 254 bytes long.
 *****************************************************************************
 */
static void WritePDS (char **buffer, int *bufferLen, double validTime,
                      char *fltName, sInt4 fltOffset, uChar endian,
                      uChar scan, char **table, int tableLen)
{
   char *ptr;           /* A pointer to where we are in the array. */
   uChar fltNameLen;    /* The length of fltName. */
   uShort2 pdsLen;      /* The length of the PDS section. */
   uShort2 s_temp;      /* A holder to store the length of the table in. */
   int i;               /* A counter used to loop over the ASCII table. */

   myAssert (fltName != NULL);
   myAssert (fltOffset >= 0);
   myAssert ((endian == 0) || (endian == 1));
   myAssert ((table != NULL) || ((table == NULL) && (tableLen == 0)));
   myAssert (sizeof (double) == 8);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (uShort2) == 2);
   myAssert (sizeof (char) == 1);

   fltNameLen = strlen (fltName);
   pdsLen = sizeof (uShort2) + sizeof (double) + 1 + fltNameLen +
         sizeof (sInt4) + 1 + 1 + sizeof (uShort2);
   if (tableLen != 0) {
      for (i = 0; i < tableLen; i++) {
         pdsLen += sizeof (uShort2) + strlen (table[i]);
      }
   }
   *bufferLen = pdsLen;
   *buffer = (char *) malloc (*bufferLen);
   ptr = *buffer;
   MEMCPY_LIT (ptr, &pdsLen, sizeof (uShort2));
   ptr += 2;
   MEMCPY_LIT (ptr, &validTime, sizeof (double));
   ptr += 8;
   *ptr = fltNameLen;
   ptr++;
   memcpy (ptr, fltName, fltNameLen);
   ptr += fltNameLen;
   MEMCPY_LIT (ptr, &fltOffset, sizeof (sInt4));
   ptr += 4;
   *ptr = endian;
   ptr++;
   *ptr = scan;
   ptr++;
   s_temp = tableLen;
   MEMCPY_LIT (ptr, &s_temp, sizeof (uShort2));
   ptr += 2;
   if (tableLen != 0) {
      for (i = 0; i < tableLen; i++) {
         s_temp = strlen (table[i]);
         MEMCPY_LIT (ptr, &s_temp, sizeof (uShort2));
         ptr += 2;
         memcpy (ptr, table[i], s_temp);
         ptr += s_temp;
      }
   }
}

/*****************************************************************************
 * InsertPDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Create a new PDS, and figure out where to store in in the char array.
 * Use an insertion sort method to make sure that the super header for the
 * PDS and the PDS arrays are properly sorted.
 *
 * ARGUMENTS
 *    flxArray = The char array to insert into. (Output)
 * flxArrayLen = The length of flxArray. (Input/Output)
 *        elem = The weather variable name. (Input)
 *     refTime = The reference time as a time_t. (Input)
 *        unit = The Unit of the weather variable (Input)
 *     comment = Any comments associated with this variable (Input)
 *      gdsNum = The GDS number associated with this variable (In)
 *      center = The originating center id. (Input)
 *   subCenter = The originating sub center's id. (Input)
 *   validTime = The valid time as a time_t cast to a double. (Input)
 *     fltName = The file name of where to look for the associated data. (In)
 *   fltOffset = Where in the fltName file to look for data. (Input)
 *      endian = What endian'ness is the file 'fltName'  (Input)
 *        scan = What orientation is the file. (Input)
 *       table = Any extra ASCII strings to associate with this data. (Input)
 *    tableLen = Length of table.
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *   0 = OK.
 *  -1 = fltName was too long.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *****************************************************************************
 */
int InsertPDS (char **flxArray, int *flxArrayLen, char *elem, time_t refTime,
               char *unit, char *comment, uShort2 gdsNum, uShort2 center,
               uShort2 subCenter, time_t validTime, char *fltName,
               sInt4 fltOffset, uChar endian, uChar scan, char **table,
               int tableLen)
{
   char *ptr;           /* A pointer to where we are in the array. */
   sInt4 offset;        /* Keep track of where we are in the array. */
   uShort2 numGDS;      /* How many GDS's there are. */
   char *superHead;     /* A super header created from the given data. */
   int lenSuperHead;    /* length of superHead. */
   char *pdsBuff;       /* A PDS created from the given data. */
   int lenPdsBuff;      /* length of pdsBuff. */
   uShort2 numSuperPDS; /* How many super headers there are. */
   char f_found;        /* Flag as to whether we know where to insert PDS. */
   int i;               /* loop counter over number of super Headers. */
   sInt4 sizeSuperPDS;  /* Total size of current superHeader + PDS array. */
   uShort2 supLen;      /* Length of just the current superHeader part. */
   uChar elemLen;       /* Length of current element. */
   char localElem[256]; /* string value of current element. */
   int ans;             /* Answer of strcmp(elem, localElem). Helps sort. */
   double localRefTime; /* value of current Reference Time. */
   sInt4 superHeadStart; /* Where the current super header started. */
   sInt4 pdsArrayStart; /* Where the current PDS array started. */
   uShort2 numPDS;      /* How many PDS in current PDS array. */
   int j;               /* loop counter over PDS array. */
   uShort2 sizePDS;     /* Size of current PDS. */
   double localValidTime; /* value of valid time of current PDS. */
   char buffer[10];     /* Used to assist in inserting new size of super PDS, 
                         * and new num PDS. */

   myAssert (*flxArrayLen >= HEADLEN + 2 + 2);
   myAssert (*flxArray != NULL);
   myAssert (elem != NULL);
   myAssert (unit != NULL);
   myAssert (comment != NULL);
   myAssert (fltName != NULL);
   myAssert (fltOffset >= 0);
   myAssert ((endian == 0) || (endian == 1));
   myAssert ((table != NULL) || ((table == NULL) && (tableLen == 0)));
   myAssert (sizeof (double) == 8);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (uShort2) == 2);
   myAssert (sizeof (char) == 1);

   if (strlen (fltName) > 254) {
      printf ("ERROR: fltName is more than 254 char.\n");
      return -1;
   }

   /* Get past the GDS */
   ptr = *flxArray + HEADLEN;
   MEMCPY_LIT (&numGDS, ptr, sizeof (uShort2));
   ptr += 2 + numGDS * GDSLEN;
   offset = HEADLEN + 2 + numGDS * GDSLEN;

   /* Create a Super Header from the given data. */
   WriteSupPDS (&superHead, &lenSuperHead, elem, (double) refTime, unit,
                comment, gdsNum, center, subCenter);
   WritePDS (&pdsBuff, &lenPdsBuff, (double) validTime, fltName, fltOffset,
             endian, scan, table, tableLen);

   /* Read in number of super headers. */
   MEMCPY_LIT (&numSuperPDS, ptr, sizeof (uShort2));
   ptr += 2;
   offset += 2;
   f_found = 0;
   for (i = 0; i < numSuperPDS; i++) {
      MEMCPY_LIT (&sizeSuperPDS, ptr, sizeof (sInt4));
      ptr += 4;
      MEMCPY_LIT (&supLen, ptr, sizeof (uShort2));
      ptr += 2;
      elemLen = *ptr;
      ptr++;
      memcpy (localElem, ptr, elemLen);
      ptr += elemLen;
      localElem[elemLen] = '\0';
      /* Following allows insertion sort by name. */
      if ((ans = strcmp (localElem, elem)) > 0) {
         break;
      }
      MEMCPY_LIT (&localRefTime, ptr, sizeof (double));
      ptr += 8;
      /* Following allows insertion sort by refTime with more recent times
       * first. */
      if ((ans == 0) && (localRefTime < refTime)) {
         break;
      }
      if (supLen != lenSuperHead) {
         offset += sizeSuperPDS;
         ptr = *flxArray + offset;
         /* The +2 is because we already compared the "supLen" */
      } else if (memcmp (superHead + 2, *flxArray + offset + 6, supLen - 2)
                 != 0) {
         offset += sizeSuperPDS;
         ptr = *flxArray + offset;
      } else {
         /* Get to the PDS Array. */
         superHeadStart = offset;
         /* supLen does not count sInt4 for "Size of Super Header and
          * Associated PDS Array". */
         offset += supLen + 4;
         pdsArrayStart = offset;
         ptr = *flxArray + offset;
         MEMCPY_LIT (&numPDS, ptr, sizeof (uShort2));
         ptr += 2;
         offset += 2;
         /* Find out where to insert the current PDS. */
         for (j = 0; j < numPDS; j++) {
            MEMCPY_LIT (&sizePDS, ptr, sizeof (uShort2));
            ptr += 2;
            MEMCPY_LIT (&localValidTime, ptr, sizeof (double));
            /* No ptr + 8 because we are about to "jump" locations. */
            if (localValidTime > validTime) {
               /* Insert before the record. */
               BufferInsert (flxArray, flxArrayLen, offset, pdsBuff,
                             lenPdsBuff);
               sizeSuperPDS += lenPdsBuff;
               numPDS++;
               f_found = 1;
               break;
            } else if (localValidTime == validTime) {
               /* Replace this record. */
               BufferRemove (*flxArray, flxArrayLen, offset, sizePDS);
               sizeSuperPDS -= sizePDS;
               BufferInsert (flxArray, flxArrayLen, offset, pdsBuff,
                             lenPdsBuff);
               sizeSuperPDS += lenPdsBuff;
               f_found = 1;
               break;
            } else {
               /* Go to the next pds. */
               offset += sizePDS;
               ptr = *flxArray + offset;
            }
         }
         if (!f_found) {
            /* Didn't insert it yet, so append to array. */
            BufferInsert (flxArray, flxArrayLen, offset, pdsBuff, lenPdsBuff);
            sizeSuperPDS += lenPdsBuff;
            numPDS++;
         }
         /* Update numPDS */
         MEMCPY_LIT (*flxArray + pdsArrayStart, &numPDS, sizeof (uShort2));
         /* Update Sizeof (SuperHeader + pdsArray) */
         MEMCPY_LIT (*flxArray + superHeadStart, &sizeSuperPDS,
                     sizeof (sInt4));
         /* Clean up and Return. */
         free (superHead);
         free (pdsBuff);
         return 0;
      }
   }
   /* Didn't find a superPDS to put the PDS in, so append a superPDS and add
    * a record for the PDS. */
   sizeSuperPDS = lenSuperHead + lenPdsBuff + 4 + 2;
   MEMCPY_LIT (buffer, &sizeSuperPDS, sizeof (sInt4));
   BufferInsert (flxArray, flxArrayLen, offset, buffer, 4);
   offset += 4;
   BufferInsert (flxArray, flxArrayLen, offset, superHead, lenSuperHead);
   offset += lenSuperHead;
   numPDS = 1;
   MEMCPY_LIT (buffer, &numPDS, sizeof (uShort2));
   BufferInsert (flxArray, flxArrayLen, offset, buffer, 2);
   offset += 2;
   BufferInsert (flxArray, flxArrayLen, offset, pdsBuff, lenPdsBuff);
   /* Update numSuperPDS */
   numSuperPDS++;
   offset = HEADLEN + 2 + numGDS * GDSLEN;
   MEMCPY_LIT (*flxArray + offset, &numSuperPDS, sizeof (uShort2));
   /* Clean up and Return. */
   free (superHead);
   free (pdsBuff);
   return 0;
}

/*
 * char datafile[256]; A holder for element from meta data.
 */
void ReadPDSBuff (char *pdsPtr, double *validTime, char *dataFile,
                  sInt4 *dataOffset, uChar *endian, uChar *scan,
                  uShort2 *numTable, char ***table, char **nextPds)
{
#ifdef DEBUG
   sInt4 lenPDS;        /* Length of PDS section. */
#endif
   char *ptr;           /* A pointer to where we are in the array. */
   uChar numBytes;      /* number of bytes in following string. */
   uShort2 sNumBytes;   /* number of bytes in an "ugly" string. */
   int k;               /* Loop counter over "ugly" string. */

#ifdef DEBUG
   MEMCPY_LIT (&lenPDS, pdsPtr, sizeof (uShort2));
#endif
   ptr = pdsPtr + 2;
   MEMCPY_LIT (validTime, ptr, sizeof (double));
   ptr += 8;
   numBytes = *ptr;
   ptr++;
   memcpy (dataFile, ptr, numBytes);
   dataFile[numBytes] = '\0';
   ptr += numBytes;
   MEMCPY_LIT (dataOffset, ptr, sizeof (sInt4));
   ptr += 4;
   *endian = *ptr;
   ptr++;
   *scan = *ptr;
   ptr++;

   myAssert (*numTable == 0);
   myAssert (*table == NULL);

   MEMCPY_LIT (numTable, ptr, sizeof (uShort2));
   ptr += 2;
   if (*numTable != 0) {
      *table = (char **) malloc (*numTable * sizeof (char *));
      for (k = 0; k < *numTable; k++) {
         MEMCPY_LIT (&sNumBytes, ptr, sizeof (uShort2));
         ptr += 2;
         (*table)[k] = (char *) malloc (sNumBytes + 1);
         memcpy ((*table)[k], ptr, sNumBytes);
         (*table)[k][sNumBytes] = '\0';
         ptr += sNumBytes;
      }
   }
   *nextPds = ptr;
}

/*
 * char elem[256]; A holder for element from meta data.
 * char unit[256]; A holder for unit for this data.
 * char comment[256]; A holder for comment for this data.
 */
void ReadSupPDSBuff (char *sPtr, char *elem, double *refTime, char *unit,
                     char *comment, uShort2 *gdsNum, uShort2 *center,
                     uShort2 *subCenter, uShort2 *numPDS, char **pdsPtr,
                     sInt4 *lenTotPds)
{
#ifdef DEBUG
   uShort2 supLen;      /* Length of just the current superHeader part. */
#endif
   char *ptr;           /* A pointer to where we are in the array. */
   uChar numBytes;      /* number of bytes in following string. */

   MEMCPY_LIT (lenTotPds, sPtr, sizeof (sInt4));
   ptr = sPtr + 4;
#ifdef DEBUG
   MEMCPY_LIT (&supLen, ptr, sizeof (uShort2));
#endif
   ptr += 2;
   numBytes = *ptr;
   ptr++;
   memcpy (elem, ptr, numBytes);
   elem[numBytes] = '\0';
   ptr += numBytes;
   MEMCPY_LIT (refTime, ptr, sizeof (double));
   ptr += 8;
   numBytes = *ptr;
   ptr++;
   memcpy (unit, ptr, numBytes);
   unit[numBytes] = '\0';
   ptr += numBytes;
   numBytes = *ptr;
   ptr++;
   memcpy (comment, ptr, numBytes);
   comment[numBytes] = '\0';
   ptr += numBytes;
   MEMCPY_LIT (gdsNum, ptr, sizeof (uShort2));
   ptr += 2;
   MEMCPY_LIT (center, ptr, sizeof (uShort2));
   ptr += 2;
   MEMCPY_LIT (subCenter, ptr, sizeof (uShort2));
   ptr += 2;
   MEMCPY_LIT (numPDS, ptr, sizeof (uShort2));
   *pdsPtr = ptr + 2;
}

/*****************************************************************************
 * PrintGDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Outputs the GDS to stdout.
 *
 * ARGUMENTS
 * gds = GDS Structure to write. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Commented.
 *
 * NOTES
 *****************************************************************************
 */
static void PrintGDS (gdsType *gds)
{
   myAssert (gds != NULL);

   printf ("NumPts %ld, projType %d, sphere? %d, EarthMaj %f, EarthMin %f\n",
           gds->numPts, gds->projType, gds->f_sphere, gds->majEarth,
           gds->minEarth);
   printf ("Nx %ld, Ny %ld, lat1 %f, lon1 %f\n", gds->Nx, gds->Ny, gds->lat1,
           gds->lon1);
   printf ("ResFlag %d, OrientLon %f, Dx %f, Dy %f, meshLat %f\n",
           gds->resFlag, gds->orientLon, gds->Dx, gds->Dy, gds->meshLat);
   printf ("Center %d, Scan %d, lat2 %f, lon2 %f, scaleLat1 %f, "
           "scaleLat2 %f, southLat %f, southLon %f\n", gds->center,
           gds->scan, gds->lat2, gds->lon2, gds->scaleLat1, gds->scaleLat2,
           gds->southLat, gds->southLon);
}

/*****************************************************************************
 * PrintSupPDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Outputs the PDS (super header and PDS array) to stdout.  Assumes that
 * buffer is at the beginning of a super PDS section.
 *
 * ARGUMENTS
 *  buffer = The char representation of a super Header. (Input)
 * buffLen = Length of buffer. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static void PrintSupPDS (char *buffer, sInt4 buffLen)
{
   char *ptr;           /* A pointer to where we are in the array. */
   double d_temp;       /* A holder for doubles. */
   sInt4 li_temp;       /* A holder for 4 byte integers. */
   uShort2 si_temp;     /* A holder for short integers. */
   uChar uc_temp;       /* A holder for char. */
   char elem[256];      /* A holder for < 256 long ASCII strings. */
   uShort2 numPDS;      /* Number of PDS in the PDS array. */
   int i;               /* A loop counter over the PDS array. */
   uShort2 numTable;    /* Number of strings in the table */
   char *table = NULL;  /* Table of strings associated with this PDS. */
   int j;               /* A loop counter over the table array. */

   myAssert (buffer != NULL);
   myAssert (sizeof (double) == 8);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (uShort2) == 2);
   myAssert (sizeof (char) == 1);

   ptr = buffer;
   MEMCPY_LIT (&li_temp, ptr, sizeof (sInt4));
   ptr += 4;
   printf ("Total size of SuperHead + PDS Array: %ld\n", li_temp);
   MEMCPY_LIT (&si_temp, ptr, sizeof (uShort2));
   ptr += 2;
   printf ("Size of SuperHead: %d\n", si_temp);
   uc_temp = *ptr;
   ptr++;
   memcpy (elem, ptr, uc_temp);
   ptr += uc_temp;
   elem[uc_temp] = '\0';
   printf ("Element: %s\n", elem);
   MEMCPY_LIT (&d_temp, ptr, sizeof (double));
   ptr += 8;
   printf ("RefTime: %f\n", d_temp);
   uc_temp = *ptr;
   ptr++;
   memcpy (elem, ptr, uc_temp);
   ptr += uc_temp;
   elem[uc_temp] = '\0';
   printf ("Unit: %s\n", elem);
   uc_temp = *ptr;
   ptr++;
   memcpy (elem, ptr, uc_temp);
   ptr += uc_temp;
   elem[uc_temp] = '\0';
   printf ("Comment: %s\n", elem);
   MEMCPY_LIT (&si_temp, ptr, sizeof (uShort2));
   ptr += 2;
   printf ("GDS Number: %d\n", si_temp);
   MEMCPY_LIT (&si_temp, ptr, sizeof (uShort2));
   ptr += 2;
   printf ("Center: %d\n", si_temp);
   MEMCPY_LIT (&si_temp, ptr, sizeof (uShort2));
   ptr += 2;
   printf ("SubCenter: %d\n", si_temp);
   MEMCPY_LIT (&numPDS, ptr, sizeof (uShort2));
   ptr += 2;
   printf ("NumPDS: %d\n", numPDS);
   for (i = 0; i < numPDS; i++) {
      printf ("... PDS %d ...\n", i);
      MEMCPY_LIT (&si_temp, ptr, sizeof (uShort2));
      ptr += 2;
      printf ("Sizeof PDS: %d\n", si_temp);
      MEMCPY_LIT (&d_temp, ptr, sizeof (double));
      ptr += 8;
      printf ("ValidTime: %f\n", d_temp);
      uc_temp = *ptr;
      ptr++;
      memcpy (elem, ptr, uc_temp);
      ptr += uc_temp;
      elem[uc_temp] = '\0';
      printf ("filename: %s\n", elem);
      MEMCPY_LIT (&li_temp, ptr, sizeof (sInt4));
      ptr += 4;
      printf ("File Offset: %ld\n", li_temp);
      uc_temp = *ptr;
      ptr++;
      printf ("endian: %d\n", uc_temp);
      uc_temp = *ptr;
      ptr++;
      printf ("scan: %d\n", uc_temp);
      MEMCPY_LIT (&numTable, ptr, sizeof (uShort2));
      ptr += 2;
      printf ("number of table entries: %d\n", numTable);
      for (j = 0; j < numTable; j++) {
         MEMCPY_LIT (&si_temp, ptr, sizeof (uShort2));
         ptr += 2;
         table = (char *) realloc ((void *) table, si_temp + 1);
         memcpy (table, ptr, si_temp);
         ptr += si_temp;
         table[si_temp] = '\0';
         printf ("table entry %d: %s\n", j, table);
      }
   }
   free (table);
}

/*****************************************************************************
 * OpenFLX() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Open a .flx file, and make sure that it is a .flx file.
 *
 * ARGUMENTS
 * filename = Name of the file to open. (Input)
 *       fp = FILE pointer which will point to filename (Output)
 *  f_write = True if one needs to write to the file. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = OK
 * -1 = couldn't open the filename for reading / writing.
 * -2 = It wasn't a FLX file.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Commented.
 *   9/2003 AAT: Added f_write option.
 *
 * NOTES
 *    After call, FILE *fp points to data after the 3 letter identifier.
 *****************************************************************************
 */
static int OpenFLX (const char *filename, FILE **fp, sChar f_write)
{
   char header[4];      /* Used to validate the Header. */

   myAssert (filename != NULL);

   if (f_write) {
      if ((*fp = fopen (filename, "r+b")) == NULL) {
         return -1;
      }
   } else {
      if ((*fp = fopen (filename, "rb")) == NULL) {
         return -1;
      }
   }
   /* Read the header. */
   fread (header, sizeof (char), 3, *fp);
   header[3] = '\0';
   if (strcmp (header, "FLX") != 0) {
      fclose (*fp);
      return -2;
   }
   return 0;
}

/*****************************************************************************
 * CreateFLX() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To create a new FLX buffer (to be stored on disk later.)
 *
 * ARGUMENTS
 *    flxArray = The FLX array to create (assumed unInitialized). (Output)
 * flxArrayLen = The Length of flxArray (assumed unInitialized). (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
void CreateFLX (char **flxArray, int *flxArrayLen)
{
   char *ptr;           /* A pointer to where we are in the array. */
   uShort2 si_temp;     /* Used to store number of GDS and PDS. */
   sInt4 fileLen;       /* Used to help store the file size. */

   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (char) == 1);

   *flxArrayLen = HEADLEN + 2 + 2;
   *flxArray = (char *) malloc (*flxArrayLen);
   ptr = *flxArray;
   memcpy (ptr, "FLX", 3);
   ptr += 3;
   fileLen = *flxArrayLen;
   MEMCPY_LIT (ptr, &fileLen, sizeof (sInt4));
   ptr += 4;
   memcpy (ptr, "rolyat ruhtra", 13);
   ptr += 13;
   /* Write the number of GDS */
   si_temp = 0;
   MEMCPY_LIT (ptr, &si_temp, sizeof (uShort2));
   ptr += 2;
   /* Write the number of PDS */
   si_temp = 0;
   MEMCPY_LIT (ptr, &si_temp, sizeof (uShort2));
}

/*****************************************************************************
 * ReadFLX() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To read a FLX from file
 *
 * ARGUMENTS
 *    filename = File to read from. (Input)
 *    flxArray = The FLX array to read to (assumed unInitialized). (Output)
 * flxArrayLen = The Length of flxArray (assumed unInitialized). (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = OK
 * -1 = Problems with filename.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int ReadFLX (const char *filename, char **flxArray, int *flxArrayLen)
{
   FILE *fp;            /* A open pointer to file to read from. */
   char *ptr;           /* A pointer to where we are in the array. */
   sInt4 fileLen;       /* How big the file claims to be. */

   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (char) == 1);

   if (filename != NULL) {
      if (OpenFLX (filename, &fp, 0) != 0) {
         return -1;
      }
   } else {
      fp = stdin;
   }
   FREAD_LIT (&fileLen, sizeof (sInt4), 1, fp);
   *flxArrayLen = fileLen;
   *flxArray = (char *) malloc (*flxArrayLen);
   ptr = *flxArray;
   memcpy (ptr, "FLX", 3);
   ptr += 3;
   MEMCPY_LIT (ptr, &fileLen, sizeof (sInt4));
   ptr += 4;
   fread (ptr, sizeof (char), fileLen - 7, fp);
   fclose (fp);
   return 0;
}

#ifdef REMOVED_FROM_176
int ReadFLX2 (FILE *fp, char **flxArray, int *flxArrayLen)
{
   char header[4];      /* Used to validate the Header. */
   char *ptr;           /* A pointer to where we are in the array. */
   sInt4 fileLen;       /* How big the file claims to be. */

   myAssert (fp != NULL);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (char) == 1);

   fread (header, sizeof (char), 3, fp);
   header[3] = '\0';
   if (strcmp (header, "FLX") != 0) {
      printf ("Problems with the header file\n");
      return -1;
   }
   FREAD_LIT (&fileLen, sizeof (sInt4), 1, fp);
   *flxArrayLen = fileLen;
   *flxArray = (char *) malloc (*flxArrayLen);
   ptr = *flxArray;
   memcpy (ptr, header, 3);
   ptr += 3;
   MEMCPY_LIT (ptr, &fileLen, sizeof (sInt4));
   ptr += 4;
   fread (ptr, sizeof (char), fileLen - 7, fp);
   return 0;
}
#endif

/*****************************************************************************
 * WriteFLX() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To write a FLX to file.
 *
 * ARGUMENTS
 *    filename = File to write to. (Output)
 *    flxArray = The FLX array to read from. (Input)
 * flxArrayLen = The Length of flxArray. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = OK
 * -1 = Problems with filename.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int WriteFLX (char *filename, char *flxArray, int flxArrayLen)
{
   FILE *fp;            /* A open pointer to file to write to. */

   myAssert (filename != NULL);
   myAssert (flxArray != NULL);
   myAssert (sizeof (char) == 1);

   if ((fp = fopen (filename, "wb")) == NULL) {
      printf ("Couldn't open %s for writing.\n", filename);
      return -1;
   }
   fwrite (flxArray, sizeof (char), flxArrayLen, fp);
   fclose (fp);
   return 0;
}

/*****************************************************************************
 * PrintFLXBuffer() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Outputs the char representation of FLX to stdout for diagnostics.
 *
 * ARGUMENTS
 *    flxArray = The FLX array to print from. (Input)
 * flxArrayLen = The Length of flxArray. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = OK
 * -1 = Not a valid buffer.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Commented.
 *
 * NOTES
 *****************************************************************************
 */
int PrintFLXBuffer (char *flxArray, int flxArrayLen)
{
   char *ptr;           /* A pointer to where we are in the array. */
   sInt4 fileLen;       /* How big the file claims to be. */
   uShort2 numGDS;      /* number of GDS Sections. */
   uShort2 numPDS;      /* number of PDS Sections. */
   int i;               /* Loop counter for either the GDS or PDS. */
   gdsType local;       /* The current GDS looped over in the GDS array. */
   sInt4 lenTotPDS;     /* Length of total PDS section. */

   myAssert (flxArray != NULL);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (uShort2) == 2);

   ptr = flxArray;
   if (strncmp (ptr, "FLX", 3) != 0) {
      return -1;
   }
   ptr += 3;
   MEMCPY_LIT (&fileLen, ptr, sizeof (sInt4));
   printf ("FileLen = %ld\n", fileLen);
   ptr = flxArray + HEADLEN;
   MEMCPY_LIT (&numGDS, ptr, sizeof (uShort2));
   ptr += 2;
   printf ("Number of GDS sections = %d\n", numGDS);
   for (i = 0; i < numGDS; i++) {
      ReadGDSBuffer (ptr, &local);
      ptr += GDSLEN;
      PrintGDS (&local);
   }

   /* Read in the pds section. */
   MEMCPY_LIT (&numPDS, ptr, sizeof (uShort2));
   ptr += 2;
   printf ("Number of PDS sections = %d\n", numPDS);
   for (i = 0; i < numPDS; i++) {
      MEMCPY_LIT (&lenTotPDS, ptr, sizeof (sInt4));
      PrintSupPDS (ptr, lenTotPDS);
      ptr += lenTotPDS;
   }
   return 0;
}

/*****************************************************************************
 * Asc2Flx() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *
 * ARGUMENTS
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *
 * HISTORY
 *
 * NOTES
 *****************************************************************************
 */
int Asc2Flx (char *inFile, char *outFile)
{
   FILE *fp;            /* The file pointer to read from. */
   static char *Sections[] = { "GDSSection", "Super_PDS_Section",
      "PDS_Section", NULL
   };
   enum { GDS, SUPER_PDS, PDS };
   static char *Gds_Elem[] = { "numPts", "projType", "majEarth", "minEarth",
      "nx", "ny", "lat1", "lon1", "resFlag",
      "orientLon", "dx", "dy", "meshLat", "center",
      "scan", "lat2", "lon2", "scaleLat1",
      "scaleLat2", "southLat", "southLon", NULL
   };
   enum { GDS_numPts, GDS_projType, GDS_majEarth, GDS_minEarth, GDS_nx,
      GDS_ny, GDS_lat1, GDS_lon1, GDS_resFlag, GDS_orientLon, GDS_dx,
      GDS_dy, GDS_meshLat, GDS_center, GDS_scan, GDS_lat2, GDS_lon2,
      GDS_scaleLat1, GDS_scaleLat2, GDS_southLat, GDS_southLon
   };
   static char *Sup_Pds_Elem[] = { "element", "refTime", "unit", "comment",
      "Center", "SubCenter", NULL
   };
   enum { SUP_PDS_element, SUP_PDS_refTime, SUP_PDS_unit, SUP_PDS_comment,
      SUP_PDS_Center, SUP_PDS_SubCenter
   };
   static char *Pds_Elem[] = { "validTime", "filename", "offset", "endian",
      "scan"
   };
   enum { PDS_validTime, PDS_filename, PDS_offset, PDS_endian, PDS_scan
   };

   char *buffer = NULL; /* Holds a line from the file. */
   size_t buffLen = 0;  /* Current length of buffer. */
   char *first;
   char *second;
   char *third;
   int curSect = -1;
   int index;
   gdsType gds;         /* The GDS. */
   char *flxArray;      /* The index file in a char buffer. */
   int flxArrayLen;     /* The length of the flxArray buffer. */
   sInt4 flxLen;
   char elem[256];
/*   time_t refTime = 0;*/
   double refTime = 0;
   char unit[256];
   char comment[256];
   uShort2 gdsNum;
   uShort2 center = 0;
   uShort2 subCenter = 0;
/*   time_t validTime = 0;*/
   double validTime = 0;
   char fltName[256];
   sInt4 fltOffset = 0;
   uChar endian = 0;
   uChar scan = 64;

   myAssert (inFile != NULL);
   myAssert (outFile != NULL);
   if ((fp = fopen (inFile, "rt")) == NULL) {
      printf ("Couldn't open %s for read\n", inFile);
      return -1;
   }
   while (reallocFGets (&buffer, &buffLen, fp) > 0) {
      first = buffer;
      while ((isspace (*first)) && (*first != '\0')) {
         first++;
      }
      if ((first != NULL) && (*first != '#')) {
         if (*first == '[') {
            second = strchr (first, ']');
            if (second != NULL) {
               *second = '\0';
               if (GetIndexFromStr (first + 1, Sections, &index) < 0) {
                  printf ("Invalid section '%s'\n", first + 1);
               } else {
                  curSect = index;
               }
            }
         } else {
            second = strchr (first, '=');
            if (second != NULL) {
               *second = '\0';
               second++;
               third = strchr (second, '#');
               if (third != NULL) {
                  *third = '\0';
               }
               third = strchr (second, '\n');
               if (third != NULL) {
                  *third = '\0';
               }
               switch (curSect) {
                  case -1:
                     break;
                  case GDS:
                     if (GetIndexFromStr (first, Gds_Elem, &index) < 0) {
                        printf ("Invalid section '%s'\n", first + 1);
                        goto error;
                     }
                     switch (index) {
                        case GDS_numPts:
                           gds.numPts = atoi (second);
                           break;
                        case GDS_projType:
                           gds.projType = atoi (second);
                           break;
                        case GDS_majEarth:
                           gds.majEarth = atof (second);
                           break;
                        case GDS_minEarth:
                           gds.minEarth = atof (second);
                           break;
                        case GDS_nx:
                           gds.Nx = atoi (second);
                           break;
                        case GDS_ny:
                           gds.Ny = atoi (second);
                           break;
                        case GDS_lat1:
                           gds.lat1 = atof (second);
                           break;
                        case GDS_lon1:
                           gds.lon1 = atof (second);
                           break;
                        case GDS_resFlag:
                           gds.resFlag = atoi (second);
                           break;
                        case GDS_orientLon:
                           gds.orientLon = atof (second);
                           break;
                        case GDS_dx:
                           gds.Dx = atof (second);
                           break;
                        case GDS_dy:
                           gds.Dy = atof (second);
                           break;
                        case GDS_meshLat:
                           gds.meshLat = atof (second);
                           break;
                        case GDS_center:
                           gds.center = atoi (second);
                           break;
                        case GDS_scan:
                           gds.scan = atoi (second);
                           break;
                        case GDS_lat2:
                           gds.lat2 = atof (second);
                           break;
                        case GDS_lon2:
                           gds.lon2 = atof (second);
                           break;
                        case GDS_scaleLat1:
                           gds.scaleLat1 = atof (second);
                           break;
                        case GDS_scaleLat2:
                           gds.scaleLat2 = atof (second);
                           break;
                        case GDS_southLat:
                           gds.southLat = atof (second);
                           break;
                        case GDS_southLon:
                           gds.southLon = atof (second);
                           break;
                     }
                     break;
                  case SUPER_PDS:
                     if (GetIndexFromStr (first, Sup_Pds_Elem, &index) < 0) {
                        printf ("Invalid section '%s'\n", first + 1);
                        goto error;
                     }
                     switch (index) {
                        case SUP_PDS_element:
                           strcpy (elem, second);
                           break;
                        case SUP_PDS_refTime:
                           Clock_ScanDateNumber (&refTime, second);
                           /* myParseTime2 (second, &refTime); */
                           break;
                        case SUP_PDS_unit:
                           strcpy (unit, second);
                           break;
                        case SUP_PDS_comment:
                           strcpy (comment, second);
                           break;
                        case SUP_PDS_Center:
                           center = atoi (second);
                           break;
                        case SUP_PDS_SubCenter:
                           subCenter = atoi (second);
                           break;
                     }
                     break;
                  case PDS:
                     if (GetIndexFromStr (first, Pds_Elem, &index) < 0) {
                        printf ("Invalid section '%s'\n", first + 1);
                        goto error;
                     }
                     switch (index) {
                        case PDS_validTime:
                           Clock_ScanDateNumber (&validTime, second);
                           /* myParseTime2 (second, &validTime); */
                           break;
                        case PDS_filename:
                           strcpy (fltName, second);
                           break;
                        case PDS_offset:
                           fltOffset = atoi (second);
                           break;
                        case PDS_endian:
                           endian = atoi (second);
                           break;
                        case PDS_scan:
                           scan = atoi (second);
                           break;
                     }
                     break;
                  default:
                     printf ("Should be impossible to get here\n");
               }
            }
         }
      }
   }
   if (gds.majEarth == gds.minEarth) {
      gds.f_sphere = 1;
   } else {
      gds.f_sphere = 1;
   }
   if (gds.Nx * gds.Ny != gds.numPts) {
      printf ("Wrong number of points %ld * %ld != %ld\n", gds.Nx, gds.Ny,
              gds.numPts);
      goto error;
   }

   CreateFLX (&flxArray, &flxArrayLen);
   gdsNum = InsertGDS (&flxArray, &flxArrayLen, &gds);
   if (InsertPDS (&flxArray, &flxArrayLen, elem, (time_t) refTime, unit,
                  comment, gdsNum, center, subCenter, (time_t) validTime,
                  fltName, fltOffset, endian, scan, NULL, 0) != 0) {
      printf ("Problems updating the PDS part.\n");
      goto error;
   }
   /* Update fileLen and write the index file out. */
   flxLen = flxArrayLen;
   MEMCPY_LIT (flxArray + 3, &flxLen, sizeof (sInt4));
   WriteFLX (outFile, flxArray, flxArrayLen);
   free (flxArray);

   fclose (fp);
   free (buffer);
   return 0;
 error:
   free (flxArray);
   fclose (fp);
   free (buffer);
   return -1;
}

#ifdef DATA_DEBUG
/*****************************************************************************
 * ReadGDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Read the GDS from a .flx file.
 *
 * ARGUMENTS
 *  fp = File to read from (assumed to be in correct location). (Input)
 * gds = GDS Structure to fill. (Output)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Commented.
 *
 * NOTES
 *****************************************************************************
 */
static void ReadGDS (FILE *fp, gdsType *gds)
{
   myAssert (fp != NULL);
   myAssert (gds != NULL);
   myAssert (sizeof (double) == 8);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (char) == 1);

   FREAD_LIT (&(gds->numPts), sizeof (sInt4), 1, fp);
   fread (&(gds->projType), sizeof (char), 1, fp);
   fread (&(gds->f_sphere), sizeof (char), 1, fp);
   FREAD_LIT (&(gds->majEarth), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->minEarth), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->Nx), sizeof (sInt4), 1, fp);
   FREAD_LIT (&(gds->Ny), sizeof (sInt4), 1, fp);
   FREAD_LIT (&(gds->lat1), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->lon1), sizeof (double), 1, fp);
   fread (&(gds->resFlag), sizeof (char), 1, fp);
   FREAD_LIT (&(gds->orientLon), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->Dx), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->Dy), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->meshLat), sizeof (double), 1, fp);
   fread (&(gds->center), sizeof (char), 1, fp);
   fread (&(gds->scan), sizeof (char), 1, fp);
   FREAD_LIT (&(gds->lat2), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->lon2), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->scaleLat1), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->scaleLat2), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->southLat), sizeof (double), 1, fp);
   FREAD_LIT (&(gds->southLon), sizeof (double), 1, fp);
}

/*****************************************************************************
 * PrintFLX() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Outputs the FLX to stdout.
 *
 * ARGUMENTS
 * filename = FLX file to read from. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = OK
 * -1 = Problems with filename.
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Commented.
 *
 * NOTES
 *****************************************************************************
 */
static int PrintFLX (char *filename)
{
   FILE *fp;            /* The file pointer to read from. */
   int ans;             /* Keeps track of any errors during OpenFLX. */
   sInt4 fileLen;       /* How big the file claims to be. */
   sInt4 offset;        /* Used to get to end of Header. */
   uShort2 numGDS;      /* number of GDS Sections. */
   uShort2 numPDS;      /* number of PDS Sections. */
   int i;               /* Loop counter for either the GDS or PDS. */
   gdsType local;       /* The current GDS looped over in the GDS array. */
   char *buffer;        /* Used to hold the whole PDS section. */
   sInt4 lenTotPDS;     /* Length of total PDS section. */

   myAssert (filename != NULL);
   myAssert (sizeof (sInt4) == 4);
   myAssert (sizeof (uShort2) == 2);
   myAssert (sizeof (char) == 1);

   if ((ans = OpenFLX (filename, &fp, 0)) != 0) {
      if (ans == -1) {
         printf ("Couldn't open %s for reading / writing\n", filename);
      } else if (ans == -2) {
         printf ("Invalid header in %s.\n", filename);
      }
      return -1;
   }
   FREAD_LIT (&fileLen, sizeof (sInt4), 1, fp);
   printf ("FileLen = %ld\n", fileLen);
   offset = HEADLEN;
   fseek (fp, offset, SEEK_SET);
   /* Read in the gds section. */
   FREAD_LIT (&numGDS, sizeof (uShort2), 1, fp);
   printf ("Number of GDS sections = %d\n", numGDS);
   for (i = 0; i < numGDS; i++) {
      ReadGDS (fp, &local);
      PrintGDS (&local);
   }
   /* Read in the pds section. */
   FREAD_LIT (&numPDS, sizeof (uShort2), 1, fp);
   printf ("Number of PDS sections = %d\n", numPDS);
   buffer = NULL;
   for (i = 0; i < numPDS; i++) {
      FREAD_LIT (&lenTotPDS, sizeof (sInt4), 1, fp);
      buffer = (char *) realloc ((void *) buffer, lenTotPDS);
      MEMCPY_LIT (buffer, &lenTotPDS, sizeof (sInt4));
      fread (buffer + 4, sizeof (char), (lenTotPDS - 4), fp);
      PrintSupPDS (buffer, lenTotPDS);
   }
   free (buffer);
   fclose (fp);
   return 0;
}

/*****************************************************************************
 * InitGDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Fill a GDS with dummy test data
 *
 * ARGUMENTS
 *       gds = GDS data to fill. (Output)
 *  projType = Projection type. (Input)
 *  majEarth = Earth's Major Axis (km) (Input)
 *  minEarth = Earth's Minor Axis (km) (Input
 *        Nx = Number of X (Input)
 *        Ny = Number of Y (Input)
 *      lat1 = lat1 (Input)
 *      lon1 = lon1 (Input)
 *   resFlag = resolution flag (Input)
 * orientLon = orientation longitude (Input)
 *        Dx = Dx (Input)
 *        Dy = Dy (Input)
 *   meshLat = mesh latitude (Input)
 *    center = center (Input)
 *      scan = scan (Input)
 *      lat2 = lat2 (Input)
 *      lon2 = lon2 (Input)
 * scaleLat1 = Scale Lat 1 (Input)
 * scaleLat2 = Scale Lat 2 (Input)
 *  southLat = South Lat (Input)
 *  southLon = South Lon (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   8/2003 Arthur Taylor (MDL/RSIS): Commented.
 *
 * NOTES
 *****************************************************************************
 */
static void InitGDS (gdsType *gds, uChar projType, double majEarth,
                     double minEarth, sInt4 Nx, sInt4 Ny, double lat1,
                     double lon1, uChar resFlag, double orientLon, double Dx,
                     double Dy, double meshLat, uChar center, uChar scan,
                     double lat2, double lon2, double scaleLat1,
                     double scaleLat2, double southLat, double southLon)
{
   myAssert (gds != NULL);

   gds->projType = projType;
   gds->majEarth = majEarth;
   gds->minEarth = minEarth;
   gds->f_sphere = (majEarth == minEarth);
   gds->Nx = Nx;
   gds->Ny = Ny;
   gds->numPts = Nx * Ny;
   gds->lat1 = lat1;
   gds->lon1 = lon1;
   gds->resFlag = resFlag;
   gds->orientLon = orientLon;
   gds->Dx = Dx;
   gds->Dy = Dy;
   gds->meshLat = meshLat;
   gds->center = center;
   gds->scan = scan;
   gds->lat2 = lat2;
   gds->lon2 = lon2;
   gds->scaleLat1 = scaleLat1;
   gds->scaleLat2 = scaleLat2;
   gds->southLat = southLat;
   gds->southLon = southLon;
}

/*****************************************************************************
 * main() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   To test the database routines.
 *
 * ARGUMENTS
 * argc = The number of command line arguments. (Input)
 * argv = The values of the command line arguments. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *  8/2003 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int main (int argc, char **argv)
{
   gdsType gds;
   char filename[] = "database.flx";
   sInt4 fileLen;       /* Length of file, needed for FileInsert. */
   char *flxArray;
   int flxArrayLen;
   time_t refTime;
   int gdsNum;
   uShort2 center;
   uShort2 subCenter;
   time_t validTime;
   sInt4 fltOffset;
   uChar endian;
   uChar scan;

   if (argc == 2) {
      PrintFLX (argv[1]);
      return 0;
   }

   InitGDS (&gds, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
            12, 13, 14, 15, 16, 17, 18, 19, 20);
   center = 1;
   subCenter = 2;
   endian = 0;
   scan = 64;

   if (ReadFLX (filename, &flxArray, &flxArrayLen) != 0) {
      CreateFLX (&flxArray, &flxArrayLen);
   }

   PrintFLXBuffer (flxArray, flxArrayLen);
   printf ("......\n");

   gdsNum = InsertGDS (&flxArray, &flxArrayLen, &gds);
   /* Update fileLen. */
   fileLen = flxArrayLen;
   MEMCPY_LIT (flxArray + 3, &fileLen, sizeof (sInt4));

   PrintFLXBuffer (flxArray, flxArrayLen);
   printf ("......\n");

   refTime = 0;
   validTime = 2;
   fltOffset = 0;
   InsertPDS (&flxArray, &flxArrayLen, "maxt", refTime, "[F]", "Max T in F",
              gdsNum, center, subCenter, validTime, "foobar.flt", fltOffset,
              endian, scan, NULL, 0);

   /* Update fileLen. */
   fileLen = flxArrayLen;
   MEMCPY_LIT (flxArray + 3, &fileLen, sizeof (sInt4));

   PrintFLXBuffer (flxArray, flxArrayLen);
   printf ("......\n");

   refTime = 2;
   validTime = 2;
   fltOffset = 6000;
   InsertPDS (&flxArray, &flxArrayLen, "maxt", refTime, "[F]", "Max T in F",
              gdsNum, center, subCenter, validTime, "foobar.flt", fltOffset,
              endian, scan, NULL, 0);

   refTime = 0;
   validTime = 3;
   fltOffset = 2000;
   InsertPDS (&flxArray, &flxArrayLen, "maxt", refTime, "[F]", "Max T in F",
              gdsNum, center, subCenter, validTime, "foobar.flt", fltOffset,
              endian, scan, NULL, 0);

   refTime = 0;
   validTime = 2;
   fltOffset = 6000;
   InsertPDS (&flxArray, &flxArrayLen, "mint", refTime, "[F]", "Min T in F",
              gdsNum, center, subCenter, validTime, "foobar.flt", fltOffset,
              endian, scan, NULL, 0);

   refTime = 0;
   validTime = 1;
   fltOffset = 4000;
   InsertPDS (&flxArray, &flxArrayLen, "maxt", refTime, "[F]", "Max T in F",
              gdsNum, center, subCenter, validTime, "foobar.flt", fltOffset,
              endian, scan, NULL, 0);

   refTime = 0;
   validTime = 2;
   fltOffset = 6000;
   InsertPDS (&flxArray, &flxArrayLen, "maxt", refTime, "[F]", "Max T in F",
              gdsNum, center, subCenter, validTime, "foobar.flt", fltOffset,
              endian, scan, NULL, 0);

   /* Update fileLen. */
   fileLen = flxArrayLen;
   MEMCPY_LIT (flxArray + 3, &fileLen, sizeof (sInt4));

   PrintFLXBuffer (flxArray, flxArrayLen);
   printf ("......\n");

   WriteFLX (filename, flxArray, flxArrayLen);
   free (flxArray);

   PrintFLX (filename);
   printf ("......\n");
   return 0;
}
#endif
