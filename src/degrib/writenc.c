/*****************************************************************************
 * writenc.c
 *
 * DESCRIPTION
 *    This file contains all the routines used to write the grid out to
 * NetCDF format.  NetCDF can be viewed via ncview-1.92e
 *
 * HISTORY
 *   4/2004 Arthur Taylor (MDL / RSIS): Created.
 *  11/2004 Jason Craig (NCAR / RAP): Modified to allow multiple GRIB
 *          Messages be put in the same NetCDF File
 *
 * NOTES
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netcdf.h"
#include "myerror.h"
#include "myassert.h"
#include "myutil.h"
#include "write.h"
#include "mymapf.h"
#include "type.h"
#include "clock.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

/*****************************************************************************
 * nc_DefineDim() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that defines the dimension in a NetCDF file,
 * and checks if it was successful.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * IdPtr = The Name of dimension. (Input)
 * value = The value (or size) of the dimension. (Input)
 * dimId = The dimension id to use with this dimension. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int nc_DefineDim (int ncid, char *IdPtr, int value, int *dimId,
                         const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_def_dim (ncid, IdPtr, value, dimId);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_DefineVar() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that defines a variable in a NetCDF file, and
 * checks if it was successful.
 *
 * ARGUMENTS
 *   ncid = The opened NetCDF file handler. (Input)
 *  IdPtr = The Name of variable. (Input)
 *   type = The NetCDF type of the variable (Input)
 * numDim = The number of dimensions of the variable. (Input)
 * dimIds = The dimension ids for this variable. (Input)
 *  varId = The variable id to use with this variable. (Input)
 *   line = The line in the code that called this routine. (Input)
 *   file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int nc_DefineVar (int ncid, char *IdPtr, nc_type type, int numDim,
                         int *dimIds, int *varId, const int line,
                         const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_def_var (ncid, IdPtr, type, numDim, dimIds, varId);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutTextAtt() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that adds a text attribute to a given NetCDF
 * variable, and validates the result.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to add attributes to. (Input)
 * IdPtr = The Name of the attribute. (Input)
 *  data = The Value of the attribute. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int nc_PutTextAtt (int ncid, int varId, char *IdPtr, char *data,
                          const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_att_text (ncid, varId, IdPtr, strlen (data), data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

static int nc_GetIntAtt (int ncid, int varId, char *IdPtr, int *data,
                         const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_get_att_int (ncid, varId, IdPtr, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

static int nc_GetTextAtt (int ncid, int varId, char *IdPtr, char **data,
                          const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */
   size_t lenp;

   *data = NULL;
   stat = nc_inq_attlen (ncid, varId, IdPtr, &lenp);
   if (stat == NC_ENOTATT) {
      return 1;
   } else if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   *data = (char *) malloc ((lenp + 1) * sizeof (char));
   stat = nc_get_att_text (ncid, varId, IdPtr, *data);
   if (stat != NC_NOERR) {
      free (*data);
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   (*data)[lenp] = '\0';
   return 0;
}

/*****************************************************************************
 * nc_PutDoubleAtt() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that adds an attribute to a given NetCDF file
 * from a data source made of doubles.  It then validates the result.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to add attributes to. (Input)
 * IdPtr = The Name of the attribute. (Input)
 *  type = The NetCDF type of the attribute. (Input)
 *  data = The Value of the attribute. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *   Could join nc_PutDoubleAtt, nc_PutFloatAtt, and nc_PutIntAtt into one
 * routine using a switch statement, but I don't think it is worth it.
 *****************************************************************************
 */
static int nc_PutDoubleAtt (int ncid, int varId, char *IdPtr, nc_type type,
                            int numData, double *data, const int line,
                            const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_att_double (ncid, varId, IdPtr, type, numData, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutFloatAtt() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that adds an attribute to a given NetCDF file
 * from a data source made of floats.  It then validates the result.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to add attributes to. (Input)
 * IdPtr = The Name of the attribute. (Input)
 *  type = The NetCDF type of the attribute. (Input)
 *  data = The Value of the attribute. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *   Could join nc_PutDoubleAtt, nc_PutFloatAtt, and nc_PutIntAtt into one
 * routine using a switch statement, but I don't think it is worth it.
 *****************************************************************************
 */
static int nc_PutFloatAtt (int ncid, int varId, char *IdPtr, nc_type type,
                           int numData, float *data, const int line,
                           const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_att_float (ncid, varId, IdPtr, type, numData, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

static int nc_GetFloatAtt (int ncid, int varId, char *IdPtr, float *data,
                           const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_get_att_float (ncid, varId, IdPtr, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

static int nc_GetShortAtt (int ncid, int varId, char *IdPtr, short int *data,
                           const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_get_att_short (ncid, varId, IdPtr, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

static int nc_GetDoubleAtt (int ncid, int varId, char *IdPtr, double *data,
                            const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_get_att_double (ncid, varId, IdPtr, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutIntAtt() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that adds an attribute to a given NetCDF file
 * from a data source made of integers.  It then validates the result.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to add attributes to. (Input)
 * IdPtr = The Name of the attribute. (Input)
 *  type = The NetCDF type of the attribute. (Input)
 *  data = The Value of the attribute. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *   Could join nc_PutDoubleAtt, nc_PutFloatAtt, and nc_PutIntAtt into one
 * routine using a switch statement, but I don't think it is worth it.
 *****************************************************************************
 */
static int nc_PutIntAtt (int ncid, int varId, char *IdPtr, nc_type type,
                         int numData, int *data, const int line,
                         const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_att_int (ncid, varId, IdPtr, type, numData, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutFloatVar() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that sets a NetCDF variable to a given array
 * of floats, and then checks that everything worked out ok.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to set the values of. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *   Could join nc_PutFloatVar, nc_PutShortVar, and nc_PutCharVar into one
 * routine using a switch statement, but I don't think it is worth it.
 *****************************************************************************
 */
static int nc_PutFloatVar (int ncid, int varId, float *data, const int line,
                           const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_var_float (ncid, varId, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutDoubleVar() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that sets a NetCDF variable to a given array
 * of doubles, and then checks that everything worked out ok.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to set the values of. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   9/2005 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int nc_PutDoubleVar (int ncid, int varId, double *data,
                            const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_var_double (ncid, varId, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutFloatVara() --
 *
 * Jason Craig (NCAR / RAP)
 *
 * PURPOSE
 *   Writes a specified section of floats to NetCDF file.  The part of the
 * netCDF variable to write is specified by giving a corner (in start) and a
 * vector of edge lengths (in count) that refer to the desired array section
 * of the netCDF variable.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to set the values of. (Input)
 * start = Array of Start Indexs. (Input)
 * count = Array of counts to put (should match size of data). (Input)
 *  data = Array of floats (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *  11/2004 Jason Craig (NCAR / RAP): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int nc_PutFloatVara (int ncid, int varId, size_t start[],
                            size_t count[], float *data, const int line,
                            const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_vara_float (ncid, varId, start, count, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutShortIntVar1() --
 *
 * Jason Craig (NCAR / RAP)
 *
 * PURPOSE
 *   Writes a single short value to NetCDF File.  The index tells it where to
 * put the data.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to set the values of. (Input)
 * index = Array of Indexs to put value at (Input)
 *  data = short int data value (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *  11/2004 Jason Craig (NCAR / RAP): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int nc_PutShortIntVar1 (int ncid, int varId, size_t index[],
                               short int *data, const int line,
                               const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_var1_short (ncid, varId, index, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

static int nc_GetShortVara (int ncid, int varid, const size_t start[],
                            const size_t count[], short *sp,
                            const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_get_vara_short (ncid, varid, start, count, sp);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

static int nc_GetFloatVara (int ncid, int varid, const size_t start[],
                            const size_t count[], float *sp,
                            const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_get_vara_float (ncid, varid, start, count, sp);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutShortVar() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that sets a NetCDF variable to a given array
 * of short integers, and then checks that everything worked out ok.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to set the values of. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *   Could join nc_PutFloatVar, nc_PutShortVar, and nc_PutCharVar into one
 * routine using a switch statement, but I don't think it is worth it.
 *****************************************************************************
 */
static int nc_PutShortVar (int ncid, int varId, short int *data,
                           const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_var_short (ncid, varId, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * nc_PutCharVar() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This is a wrapper routine that sets a NetCDF variable to a given array
 * of characters, and then checks that everything worked out ok.
 *
 * ARGUMENTS
 *  ncid = The opened NetCDF file handler. (Input)
 * varId = The variable to set the values of. (Input)
 *  line = The line in the code that called this routine. (Input)
 *  file = The source file that called this routine. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *   Could join nc_PutFloatVar, nc_PutShortVar, and nc_PutCharVar into one
 * routine using a switch statement, but I don't think it is worth it.
 *****************************************************************************
 */
static int nc_PutCharVar (int ncid, int varId, char *data,
                          const int line, const char *file)
{
   int stat;            /* Return value from NetCDF call */

   stat = nc_put_var_text (ncid, varId, data);
   if (stat != NC_NOERR) {
      errSprintf ("line %d of %s: %s\n", line, file, nc_strerror (stat));
      return -1;
   }
   return 0;
}

/*****************************************************************************
 * writeGDS() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This adds all the attributes to a netcdf variable, which explain the map
 * projection that was used.
 *
 * ARGUMENTS
 *     ncid = The opened NetCDF file handler. (Input)
 *    varId = The variable to add attributes to. (Input)
 *      gds = Grid Definiton from the parsed GRIB msg to write. (Input)
 * f_NetCDF = Version of degrib-NetCDF to use (1,2 or 3) (In)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   4/2004 Arthur Taylor (MDL/RSIS): Created.
 *   9/2005 AAT: Modified to be more CF compliant
 *
 * NOTES
 *   See: http://www.cgd.ucar.edu/cms/eaton/cf-metadata/CF-1.0.html#gmap_app
 * for a convention on grid specifications.
 *****************************************************************************
 */
static int writeGDS (int ncid, myMaparam *map, int varId, gdsType *gds,
                     sChar f_NetCDF)
{
   int i_temp;
   double lon;
   double d_temp[2];
   char *buffer;

   /* Put out the map projection type. */
   switch (gds->projType) {
      case GS3_LATLON: /* 0 */
         if (nc_PutTextAtt (ncid, varId, "grid_mapping_name",
                            "latitude_longitude", __LINE__, __FILE__) != 0)
            return -1;
         break;
      case GS3_MERCATOR: /* 10 */
         if (nc_PutTextAtt (ncid, varId, "grid_mapping_name",
                            "mercator", __LINE__, __FILE__) != 0)
            return -1;
         break;
      case GS3_POLAR:  /* 20 */
         if (nc_PutTextAtt (ncid, varId, "grid_mapping_name",
                            "polar_stereographic", __LINE__, __FILE__) != 0)
            return -1;
         break;
      case GS3_LAMBERT: /* 30 */
         if (nc_PutTextAtt (ncid, varId, "grid_mapping_name",
                            "lambert_conformal_conic", __LINE__,
                            __FILE__) != 0)
            return -1;
         break;
      default:
         errSprintf ("Un-supported Map Projection %ld\n", gds->projType);
         return -1;
   }

   /* Put out the geoid that we used. */
   if (gds->f_sphere) {
      if (nc_PutTextAtt (ncid, varId, "EarthShape", "sphere", __LINE__,
                         __FILE__) != 0)
         return -1;
      if (nc_PutDoubleAtt (ncid, varId, "EarthRadius_KM", NC_DOUBLE, 1,
                           &(gds->majEarth), __LINE__, __FILE__) != 0)
         return -1;
      mallocSprintf (&buffer, "%f km", gds->majEarth);
      if (nc_PutTextAtt (ncid, varId, "EarthRadius", buffer, __LINE__,
                         __FILE__) != 0) {
         free (buffer);
         return -1;
      }
      free (buffer);

   } else {
      if (nc_PutTextAtt (ncid, varId, "EarthShape", "oblate spheroid",
                         __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutDoubleAtt (ncid, varId, "EarthMajorAxis_KM", NC_DOUBLE, 1,
                           &(gds->majEarth), __LINE__, __FILE__) != 0)
         return -1;
      mallocSprintf (&buffer, "%f km", gds->majEarth);
      if (nc_PutTextAtt (ncid, varId, "EarthMajorAxis", buffer, __LINE__,
                         __FILE__) != 0) {
         free (buffer);
         return -1;
      }
      free (buffer);
      if (nc_PutDoubleAtt (ncid, varId, "EarthMinorAxis_KM", NC_DOUBLE, 1,
                           &(gds->minEarth), __LINE__, __FILE__) != 0)
         return -1;
      mallocSprintf (&buffer, "%f km", gds->minEarth);
      if (nc_PutTextAtt (ncid, varId, "EarthMinorAxis", buffer, __LINE__,
                         __FILE__) != 0) {
         free (buffer);
         return -1;
      }
      free (buffer);
   }

   /* Put out the central meridian and standard parallels. */
   if (gds->projType != GS3_LATLON) {
      lon = gds->orientLon;
      while (lon > 180)
         lon -= 360;
      while (lon <= -180)
         lon += 360;
      if (f_NetCDF <= 2) {
         if (nc_PutDoubleAtt (ncid, varId, "longitude_of_central_meridian",
                              NC_DOUBLE, 1, &(lon), __LINE__, __FILE__) != 0)
            return -1;
         if (nc_PutDoubleAtt (ncid, varId, "standard_parallel_1", NC_DOUBLE,
                              1, &(gds->scaleLat1), __LINE__, __FILE__) != 0)
            return -1;
         if (nc_PutDoubleAtt (ncid, varId, "standard_parallel_2", NC_DOUBLE,
                              1, &(gds->scaleLat2), __LINE__, __FILE__) != 0)
            return -1;
      } else {
         d_temp[0] = gds->scaleLat1;
         d_temp[1] = gds->scaleLat2;
         if (d_temp[0] != d_temp[1]) {
            if (nc_PutDoubleAtt (ncid, varId, "standard_parallel", NC_DOUBLE,
                                 2, d_temp, __LINE__, __FILE__) != 0)
               return -1;
         } else {
            if (nc_PutDoubleAtt (ncid, varId, "standard_parallel", NC_DOUBLE,
                                 1, d_temp, __LINE__, __FILE__) != 0)
               return -1;
         }
         if (nc_PutDoubleAtt (ncid, varId, "longitude_of_central_meridian",
                              NC_DOUBLE, 1, &(lon), __LINE__, __FILE__) != 0)
            return -1;
         if (gds->scaleLat1 == gds->scaleLat2) {
            if (nc_PutDoubleAtt (ncid, varId,
                                 "latitude_of_projection_origin", NC_DOUBLE,
                                 1, &(gds->scaleLat1), __LINE__,
                                 __FILE__) != 0)
               return -1;
         } else {
            d_temp[0] = eqvlat (&(map->stcprm), gds->scaleLat1,
                                gds->scaleLat2);
            if (nc_PutDoubleAtt (ncid, varId,
                                 "latitude_of_projection_origin", NC_DOUBLE,
                                 1, d_temp, __LINE__, __FILE__) != 0)
               return -1;
         }
      }
   }

   /* Put out the Grid specifications. */
   if (f_NetCDF < 3) {
      i_temp = gds->Nx;
      if (nc_PutIntAtt (ncid, varId, "NX_NumPntsOnParallel", NC_INT, 1,
                        &(i_temp), __LINE__, __FILE__) != 0)
         return -1;
      i_temp = gds->Ny;
      if (nc_PutIntAtt (ncid, varId, "NY_NumPntsOnMeridian", NC_INT, 1,
                        &(i_temp), __LINE__, __FILE__) != 0)
         return -1;
   }
   if (nc_PutDoubleAtt (ncid, varId, "LowerLeftLatitude", NC_DOUBLE, 1,
                        &(gds->lat1), __LINE__, __FILE__) != 0)
      return -1;
   lon = gds->lon1;
   while (lon > 180)
      lon -= 360;
   while (lon <= -180)
      lon += 360;
   if (nc_PutDoubleAtt (ncid, varId, "LowerLeftLongitude", NC_DOUBLE, 1,
                        &(lon), __LINE__, __FILE__) != 0)
      return -1;

   /* Upper Right Lat/Lon is only given out for LATLON or mercator grids. */
   if ((gds->projType == GS3_LATLON) || (gds->projType == GS3_MERCATOR)) {
      if (nc_PutDoubleAtt (ncid, varId, "UpperRightLatitude", NC_DOUBLE, 1,
                           &(gds->lat2), __LINE__, __FILE__) != 0)
         return -1;
      lon = gds->lon2;
      while (lon > 180)
         lon -= 360;
      while (lon <= -180)
         lon += 360;
      if (nc_PutDoubleAtt (ncid, varId, "UpperRightLongitude", NC_DOUBLE, 1,
                           &(lon), __LINE__, __FILE__) != 0)
         return -1;
   }

   /* Grid size is in degrees for LatLon.  Meters for other projections. */
   if (f_NetCDF < 3) {
      if (gds->projType != GS3_LATLON) {
         if (nc_PutDoubleAtt (ncid, varId, "TrueGridLength_Latitude",
                              NC_DOUBLE, 1, &(gds->meshLat), __LINE__,
                              __FILE__) != 0)
            return -1;
      }
      if (gds->projType == GS3_LATLON) {
         if (nc_PutDoubleAtt (ncid, varId, "XGridLength_Degrees", NC_DOUBLE,
                              1, &(gds->Dx), __LINE__, __FILE__) != 0)
            return -1;
         if (nc_PutDoubleAtt (ncid, varId, "YGridLength_Degrees", NC_DOUBLE,
                              1, &(gds->Dx), __LINE__, __FILE__) != 0)
            return -1;
      } else {
         if (nc_PutDoubleAtt (ncid, varId, "XGridLength_M", NC_DOUBLE, 1,
                              &(gds->Dx), __LINE__, __FILE__) != 0)
            return -1;
         if (nc_PutDoubleAtt (ncid, varId, "YGridLength_M", NC_DOUBLE, 1,
                              &(gds->Dx), __LINE__, __FILE__) != 0)
            return -1;

         if ((gds->projType == GS3_POLAR) || (gds->projType == GS3_LAMBERT)) {
            if (gds->center & GRIB2BIT_1) {
               if (nc_PutTextAtt (ncid, varId, "PoleOnPlane", "south",
                                  __LINE__, __FILE__) != 0)
                  return -1;
            } else {
               if (nc_PutTextAtt (ncid, varId, "PoleOnPlane", "north",
                                  __LINE__, __FILE__) != 0)
                  return -1;
            }
            if (gds->center & GRIB2BIT_2) {
               if (nc_PutTextAtt (ncid, varId, "BiPolar", "yes",
                                  __LINE__, __FILE__) != 0)
                  return -1;
            } else {
               if (nc_PutTextAtt (ncid, varId, "BiPolar", "no",
                                  __LINE__, __FILE__) != 0)
                  return -1;
            }
         }
         if (nc_PutDoubleAtt (ncid, varId, "SouthernLatitude", NC_DOUBLE, 1,
                              &(gds->southLat), __LINE__, __FILE__) != 0)
            return -1;
         lon = gds->southLon;
         while (lon > 180)
            lon -= 360;
         while (lon <= -180)
            lon += 360;
         if (nc_PutDoubleAtt (ncid, varId, "SouthernLongitude", NC_DOUBLE, 1,
                              &(lon), __LINE__, __FILE__) != 0)
            return -1;
      }
   }
   if (f_NetCDF < 3) {
      if (gds->resFlag & GRIB2BIT_5) {
         if (nc_PutTextAtt (ncid, varId, "VectorsRelativeTo", "grid",
                            __LINE__, __FILE__) != 0)
            return -1;
      } else {
         if (nc_PutTextAtt (ncid, varId, "VectorsRelativeTo",
                            "easterly_northerly", __LINE__, __FILE__) != 0)
            return -1;
      }
   }
   return 0;
}

/*****************************************************************************
 * DefineSpatialInfo() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This adds all the dimensions / variables and attributes to a netcdf
 * file to describe the spatial grid.  In other words it adds an XCells,
 * YCells coordinate axis, as well as a map-projection variable, and 2-d
 * variables for latitude and longitide.
 *
 * ARGUMENTS
 *           ncid = The opened NetCDF file handler. (Input)
 *            map = Already defined map projection corresponding to gds (In)
 *            gds = Grid Definiton from the parsed GRIB msg to write. (Input)
 *     GridDimIds = The NetCDF Ids to the dimensions of the grid. (Input)
 *          NX_id = The NetCDF variable ID to the NX coordinate. (Output)
 *          NY_id = The NetCDF variable ID to the NY coordinate. (Output)
 *         lat_id = The NetCDF variable ID to the latitude variable (Output)
 *         lon_id = The NetCDF variable ID to the longitude variable (Output)
 * LatLon_Decimal = How many decimals to calculate lat/lon to. (Input)
 *       f_NetCDF = Version of degrib-NetCDF to use (1,2 or 3) (In)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   4/2004 Arthur Taylor (MDL/RSIS): Created.
 *  11/2004 Jason Craig (NCAR / RAP): Modified for 2nd degrib version of
 *          NetCDF
 *   9/2005 AAT: Modified to have map projection come in.
 *
 * NOTES
 *   See: http://www.cgd.ucar.edu/cms/eaton/cf-metadata/CF-1.0.html#ctype
 * for a convention on grid specifications.
 *****************************************************************************
 */
static int DefineSpatialInfo (int ncid, myMaparam *map, gdsType *gds,
                              int GridDimIds[3], int *NX_id, int *NY_id,
                              int *lat_id, int *lon_id, int LatLon_Decimal,
                              sChar f_NetCDF)
{
   int map_id;          /* Map NetCDF Variable ID */
   char *buffer;        /* Used to write the grid spacing size. */

   /* define variables */
   if (nc_DefineVar (ncid, "MapProjection", NC_CHAR, 0, NULL, &map_id,
                     __LINE__, __FILE__) != 0)
      return -1;
   if (nc_PutTextAtt (ncid, map_id, "long_name", "MapProjection", __LINE__,
                      __FILE__) != 0)
      return -1;
   if (writeGDS (ncid, map, map_id, gds, f_NetCDF) != 0)
      return -1;

   /* Removed from 2nd version of degrib-NetCDF */
   if (f_NetCDF == 1) {
      if (nc_DefineVar (ncid, "XCells", NC_SHORT, 1, &(GridDimIds[1]), NX_id,
                        __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NX_id, "long_name", "grid_cells in"
                         " x-Direction", __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NX_id, "standard_name",
                         "projection_x_coordinate", __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NX_id, "units", "m", __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NX_id, "axis", "x", __LINE__, __FILE__) != 0)
         return -1;

      if (nc_DefineVar (ncid, "YCells", NC_SHORT, 1, &(GridDimIds[0]), NY_id,
                        __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NY_id, "long_name", "grid_cells in"
                         " y-coordinate", __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NY_id, "standard_name",
                         "projection_y_coordinate", __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NY_id, "units", "m", __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NY_id, "axis", "y", __LINE__, __FILE__) != 0)
         return -1;
   } else if (f_NetCDF == 3) {
      if (nc_DefineVar (ncid, "YCells", NC_DOUBLE, 1, &(GridDimIds[1]),
                        NY_id, __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NY_id, "units", "m", __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NY_id, "long_name",
                         "y coordinate of projection", __LINE__,
                         __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NY_id, "standard_name",
                         "projection_y_coordinate", __LINE__, __FILE__) != 0)
         return -1;
      mallocSprintf (&buffer, "%f m", gds->Dy);
      if (nc_PutTextAtt (ncid, *NY_id, "grid_spacing",
                         buffer, __LINE__, __FILE__) != 0) {
         free (buffer);
         return -1;
      }
      free (buffer);
      if (gds->projType != GS3_LATLON) {
         if (nc_PutDoubleAtt (ncid, *NY_id, "TrueGridLength_Latitude",
                              NC_DOUBLE, 1, &(gds->meshLat), __LINE__,
                              __FILE__) != 0)
            return -1;
      }
      if (nc_PutTextAtt (ncid, *NY_id, "_CoordinateAxisType", "GeoY",
                         __LINE__, __FILE__) != 0)
         return -1;

      if (nc_DefineVar (ncid, "XCells", NC_DOUBLE, 1, &(GridDimIds[2]),
                        NX_id, __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NX_id, "units", "m", __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NX_id, "long_name",
                         "x coordinate of projection", __LINE__,
                         __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *NX_id, "standard_name",
                         "projection_x_coordinate", __LINE__, __FILE__) != 0)
         return -1;
      mallocSprintf (&buffer, "%f m", gds->Dx);
      if (nc_PutTextAtt (ncid, *NX_id, "grid_spacing",
                         buffer, __LINE__, __FILE__) != 0) {
         free (buffer);
         return -1;
      }
      if (gds->projType != GS3_LATLON) {
         if (nc_PutDoubleAtt (ncid, *NX_id, "TrueGridLength_Latitude",
                              NC_DOUBLE, 1, &(gds->meshLat), __LINE__,
                              __FILE__) != 0)
            return -1;
      }
      free (buffer);
      if (nc_PutTextAtt (ncid, *NX_id, "_CoordinateAxisType", "GeoX",
                         __LINE__, __FILE__) != 0)
         return -1;
   }

   /* Changed for 2nd version of degrib-NetCDF */
   if (f_NetCDF == 1) {
      if (nc_DefineVar (ncid, "longitude", NC_FLOAT, 2, GridDimIds, lon_id,
                        __LINE__, __FILE__) != 0)
         return -1;
   } else {
      if (nc_DefineVar (ncid, "longitude", NC_FLOAT, 2, &(GridDimIds[1]),
                        lon_id, __LINE__, __FILE__) != 0)
         return -1;
   }

   if (f_NetCDF < 3) {
      if (nc_PutTextAtt (ncid, *lon_id, "long_name", "longitude", __LINE__,
                         __FILE__) != 0)
         return -1;
   } else {
      if (nc_PutTextAtt (ncid, *lon_id, "long_name", "longitude coordinate",
                         __LINE__, __FILE__) != 0)
         return -1;
   }
   if (nc_PutTextAtt (ncid, *lon_id, "units", "degrees_east", __LINE__,
                      __FILE__) != 0)
      return -1;
   if (nc_PutIntAtt (ncid, *lon_id, "precision", NC_INT, 1, &LatLon_Decimal,
                     __LINE__, __FILE__) != 0)
      return -1;
   if (nc_PutTextAtt (ncid, *lon_id, "grid_mapping", "MapProjection",
                      __LINE__, __FILE__) != 0)
      return -1;
   if (f_NetCDF < 3) {
      if (writeGDS (ncid, map, *lon_id, gds, f_NetCDF) != 0)
         return -1;
   } else {
      if (nc_PutTextAtt (ncid, *lon_id, "standard_name", "longitude",
                         __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *lon_id, "_CoordinateAxisType", "Lon",
                         __LINE__, __FILE__) != 0)
         return -1;
   }

   /* Changed for 2nd version of degrib-NetCDF */
   if (f_NetCDF == 1) {
      if (nc_DefineVar (ncid, "latitude", NC_FLOAT, 2, GridDimIds, lat_id,
                        __LINE__, __FILE__) != 0)
         return -1;
   } else {
      if (nc_DefineVar (ncid, "latitude", NC_FLOAT, 2, &(GridDimIds[1]),
                        lat_id, __LINE__, __FILE__) != 0)
         return -1;
   }

   if (f_NetCDF < 3) {
      if (nc_PutTextAtt (ncid, *lat_id, "long_name", "latitude", __LINE__,
                         __FILE__) != 0)
         return -1;
   } else {
      if (nc_PutTextAtt (ncid, *lat_id, "long_name", "latitude coordinate",
                         __LINE__, __FILE__) != 0)
         return -1;
   }
   if (nc_PutTextAtt (ncid, *lat_id, "units", "degrees_north", __LINE__,
                      __FILE__) != 0)
      return -1;
   if (nc_PutIntAtt (ncid, *lat_id, "precision", NC_INT, 1, &LatLon_Decimal,
                     __LINE__, __FILE__) != 0)
      return -1;
   if (nc_PutTextAtt (ncid, *lat_id, "grid_mapping", "MapProjection",
                      __LINE__, __FILE__) != 0)
      return -1;
   if (f_NetCDF < 3) {
      if (writeGDS (ncid, map, *lat_id, gds, f_NetCDF) != 0)
         return -1;
   } else {
      if (nc_PutTextAtt (ncid, *lat_id, "standard_name", "latitude",
                         __LINE__, __FILE__) != 0)
         return -1;
      if (nc_PutTextAtt (ncid, *lat_id, "_CoordinateAxisType", "Lat",
                         __LINE__, __FILE__) != 0)
         return -1;
   }
   return 0;
}

/*****************************************************************************
 * StoreSpatialInfo() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This computes and stores the spatial data in NetCDF.  In other words it
 * fills out the XCells, YCells coordinate axis, as well as computing the 2-d
 * variables of latitude and longitide.
 *
 * ARGUMENTS
 *           ncid = The opened NetCDF file handler. (Input)
 *            map = Already defined map projection corresponding to gds (In)
 *            gds = Grid Definiton from the parsed GRIB msg to write. (Input)
 *          NX_id = The NetCDF variable ID to the NX coordinate. (Input)
 *          NY_id = The NetCDF variable ID to the NY coordinate. (Input)
 *         lat_id = The NetCDF variable ID to the latitude variable (Input)
 *         lon_id = The NetCDF variable ID to the longitude variable (Input)
 * LatLon_Decimal = How many decimals to calculate lat/lon to. (Input)
 *       f_NetCDF = Version of degrib-NetCDF to use (1, or 2) (In)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   4/2004 Arthur Taylor (MDL/RSIS): Created.
 *  11/2004 Jason Craig (NCAR / RAP): Modified for 2nd degrib version of
 *          NetCDF
 *   9/2005 AAT: Modified to have map projection Set elsewhere.
 *
 * NOTES
 *   See: http://www.cgd.ucar.edu/cms/eaton/cf-metadata/CF-1.0.html#gmap
 * for a convention on grid specifications.
 *****************************************************************************
 */
static int StoreSpatialInfo (int ncid, myMaparam *map, gdsType *gds,
                             int NX_id, int NY_id, int lat_id, int lon_id,
                             int LatLon_Decimal, sChar f_NetCDF)
{
   float *data_lat;     /* The grid of latitude data. */
   float *data_lon;     /* The grid of longitude data. */
   uInt4 i, j;          /* loop counters over the grid. */
   double lat, lon;     /* The converted to lat/lon values. */
   short int *axis;     /* Used to help store the coordinate varaiables. */
   myMaparam map2;      /* Used to set up local map projection */
   double lonProjOrigin;
   double latProjOrigin;
   double scaleLat;
   double scaleLon;
   double mesh;
   double x, y;
   double *d_axis;

   data_lat = (float *) malloc (gds->Nx * gds->Ny * sizeof (float));
   data_lon = (float *) malloc (gds->Nx * gds->Ny * sizeof (float));
   for (j = 0; j < gds->Ny; j++) {
      for (i = 0; i < gds->Nx; i++) {
         myCxy2ll (map, i + 1, j + 1, &lat, &lon);
         data_lat[i + j * gds->Nx] = myRound (lat, LatLon_Decimal);
         data_lon[i + j * gds->Nx] = myRound (lon, LatLon_Decimal);
      }
   }
   if (nc_PutFloatVar (ncid, lat_id, data_lat, __LINE__, __FILE__) != 0) {
      free (data_lat);
      free (data_lon);
      return -1;
   }
   free (data_lat);
   if (nc_PutFloatVar (ncid, lon_id, data_lon, __LINE__, __FILE__) != 0) {
      free (data_lon);
      return -1;
   }
   free (data_lon);

   /* Removed from 2nd version of degrib-NetCDF */
   if (f_NetCDF == 1) {
      axis = (short int *) malloc (gds->Nx * sizeof (short int));
      for (i = 0; i < gds->Nx; i++) {
         axis[i] = i + 1;
      }
      if (nc_PutShortVar (ncid, NX_id, axis, __LINE__, __FILE__) != 0) {
         free (axis);
         return -1;
      }
      axis = (short int *) realloc ((void *) axis,
                                    gds->Ny * sizeof (short int));
      for (i = 0; i < gds->Ny; i++) {
         axis[i] = i + 1;
      }
      if (nc_PutShortVar (ncid, NY_id, axis, __LINE__, __FILE__) != 0) {
         free (axis);
         return -1;
      }
      free (axis);
   } else if (f_NetCDF == 3) {
      mkGeoid (&(map2.stcprm), AB, gds->majEarth, gds->minEarth);
      lonProjOrigin = gds->orientLon;
      while (lonProjOrigin > 180)
         lonProjOrigin -= 360;
      while (lonProjOrigin <= -180)
         lonProjOrigin += 360;
      if (gds->scaleLat1 == gds->scaleLat2) {
         latProjOrigin = gds->scaleLat1;
      } else {
         latProjOrigin = eqvlat (&(map->stcprm), gds->scaleLat1,
                                 gds->scaleLat2);
      }

      stlmbr (&(map2.stcprm), latProjOrigin, lonProjOrigin);

      scaleLat = latProjOrigin;
      scaleLon = lonProjOrigin;
      mesh = .001;      /* Want mesh size in m instead of km */
      stcm1p (&(map2.stcprm), 0., 0., latProjOrigin, lonProjOrigin, scaleLat,
              scaleLon, mesh, 0);
      cll2xy (&(map2.stcprm), gds->lat1, gds->lon1, &x, &y);

      d_axis = (double *) malloc (gds->Nx * sizeof (double));
      d_axis[0] = x;
      for (i = 1; i < gds->Nx; i++) {
         d_axis[i] = x + i * gds->Dx;
      }
      if (nc_PutDoubleVar (ncid, NX_id, d_axis, __LINE__, __FILE__) != 0) {
         free (d_axis);
         return -1;
      }
      d_axis = (double *) realloc ((void *) d_axis,
                                   gds->Ny * sizeof (double));
      d_axis[0] = y;
      for (i = 1; i < gds->Ny; i++) {
         d_axis[i] = y + i * gds->Dy;
      }
      if (nc_PutDoubleVar (ncid, NY_id, d_axis, __LINE__, __FILE__) != 0) {
         free (d_axis);
         return -1;
      }
      free (d_axis);
   }

   return 0;
}

/*****************************************************************************
 * DefineWeatherInfo() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This adds all the dimensions / variables and attributes to a netcdf
 * file to describe the weather keys.
 *
 * ARGUMENTS
 *         ncid = The opened NetCDF file handler. (Input)
 *           wx = The Weather section to prepare for.. (Input)
 *        wx_id = NetCDF variable ID to the wx coordinate. (Output)
 * wx_String_id = NetCDF variable ID to the wx_String coordinate. (Output)
 *     wxKey_id = NetCDF variable ID to the wx_Key grid. (Output)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int DefineWeatherInfo (int ncid, sect2_WxType *wx, int *wx_id,
                              int *wx_String_id, int *wxKey_id)
{
   int WX_dim;          /* number of a weather strings. */
   int WX_STRING_dim;   /* Max length of a weather string. */
   int wxLen;           /* Number of weather strings. */
   int wxStringLen;     /* Max String length of a weather string. */
   int wxKeyIds[2];     /* Dimension ID's for wxKeys variable. */

   wxLen = wx->dataLen;
   wxStringLen = wx->maxLen;

   /* define dimensions */
   if (nc_DefineDim (ncid, "WX_Dim", wxLen, &WX_dim, __LINE__, __FILE__) != 0)
      return -1;
   if (nc_DefineDim (ncid, "WX_STRING_Dim", wxStringLen, &WX_STRING_dim,
                     __LINE__, __FILE__) != 0)
      return -1;

   /* define variables */
   if (nc_DefineVar (ncid, "WX_Dim", NC_SHORT, 1, &WX_dim, wx_id, __LINE__,
                     __FILE__) != 0)
      return -1;
   if (nc_PutTextAtt (ncid, *wx_id, "long_name", "Weather Index Number",
                      __LINE__, __FILE__) != 0)
      return -1;

   if (nc_DefineVar (ncid, "WX_STRING_Dim", NC_SHORT, 1, &WX_STRING_dim,
                     wx_String_id, __LINE__, __FILE__) != 0)
      return -1;
   if (nc_PutTextAtt (ncid, *wx_String_id, "long_name",
                      "Weather Character String Axis", __LINE__,
                      __FILE__) != 0)
      return -1;

   wxKeyIds[0] = WX_dim;
   wxKeyIds[1] = WX_STRING_dim;
   if (nc_DefineVar (ncid, "Wx_SFC_wxKeys", NC_CHAR, 2, wxKeyIds, wxKey_id,
                     __LINE__, __FILE__) != 0)
      return -1;
   if (nc_PutTextAtt (ncid, *wxKey_id, "long_name", "Weather Keys", __LINE__,
                      __FILE__) != 0)
      return -1;
   return 0;
}

/*****************************************************************************
 * StoreWeatherInfo() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This stores the weather keys to their NetCDF variables.
 *
 * ARGUMENTS
 *         ncid = The opened NetCDF file handler. (Input)
 *           wx = The Weather section to prepare for.. (Input)
 *        wx_id = NetCDF variable ID to the wx coordinate. (Input)
 * wx_String_id = NetCDF variable ID to the wx_String coordinate. (Input)
 *     wxKey_id = NetCDF variable ID to the wx_Key grid. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
static int StoreWeatherInfo (int ncid, sect2_WxType *wx, int wx_id,
                             int wx_String_id, int wxKey_id)
{
   short int *axis;     /* Used to help store the coordinate varaiables. */
   int i;               /* Counter over the coordinate variables. */
   int wxLen;           /* Number of weather strings. */
   int wxStringLen;     /* Max String length of a weather string. */
   char *c_data;        /* Used to help store the weather strings. */

   wxLen = wx->dataLen;
   wxStringLen = wx->maxLen;
   axis = (short int *) malloc (wxLen * sizeof (short int));
   for (i = 0; i < wxLen; i++) {
      axis[i] = i;
   }
   if (nc_PutShortVar (ncid, wx_id, axis, __LINE__, __FILE__) != 0) {
      free (axis);
      return -1;
   }
   axis = (short int *) realloc ((void *) axis,
                                 wxStringLen * sizeof (short int));
   for (i = 0; i < wxStringLen; i++) {
      axis[i] = i;
   }
   if (nc_PutShortVar (ncid, wx_String_id, axis, __LINE__, __FILE__) != 0) {
      free (axis);
      return -1;
   }
   free (axis);
   c_data = (char *) malloc (wxLen * wxStringLen * sizeof (char));
   memset (c_data, '\0', wxLen * wxStringLen);
   for (i = 0; i < wxLen; i++) {
      strcpy (c_data + i * wxStringLen, wx->data[i]);
   }
   if (nc_PutCharVar (ncid, wxKey_id, c_data, __LINE__, __FILE__) != 0) {
      free (c_data);
      return -1;
   }
   free (c_data);
   return 0;
}

/*****************************************************************************
 * netCDF_V1() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This stores the GRIB data to a NetCDF file (which follows the CF
 * conventions as closely as I could make it.
 *   Implements the first version of NetCDF for degrib
 *
 * ARGUMENTS
 *       filename = The file to write the data to. (Input)
 *      grib_data = The decoded Grid of data to write to file. (Input)
 *           meta = meta data for the GRIB data to be stored in NetCDF (In)
 *       f_NetCDF = Version of degrib-NetCDF to use (1, or 2) (In)
 *        decimal = How many decimals to store the data with. (Input)
 * LatLon_Decimal = How many decimals to calculate lat/lon to. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *   1/2005 AAT: Updated for pdsTdlp type
 *
 * NOTES
 *   See: http://www.cgd.ucar.edu/cms/eaton/cf-metadata/CF-1.0.html#gmap
 * for a convention on grid specifications.
 *****************************************************************************
 */
static int netCDF_V1 (char *filename, double *grib_Data,
                      grib_MetaData *meta, sChar f_NetCDF, sChar decimal,
                      sChar LatLon_Decimal)
{
   int ncid;            /* netCDF file id */
   int stat;            /* Return value from NetCDF call */
   int NX_dim;          /* NX Dimension. */
   int NY_dim;          /* NY Dimension. */
   int NX_id;           /* Axis-Nx Variable ID */
   int NY_id;           /* Axis-Ny Variable ID */
   int lon_id;          /* longitude Variable ID */
   int lat_id;          /* latitude Variable ID */
   int GridDimIds[2];   /* Dimension ID's for the grid (NY,NX) */
   char f_wx;           /* Boolean as to whether we have a weather grid. */
   int wx_id;           /* Axis-WX Variable ID. */
   int wx_String_id;    /* Axis-WX_STRING Variable ID. */
   int wxKey_id;        /* Weather Key variable ID. */
   char *level;         /* Points to the interesting part of the level name */
   char gridName[NC_MAX_NAME]; /* Name of the Grid Data. */
   int grid_id;         /* Grid Variable ID */
   char *ptr;           /* Assist with various text strings */
   char c_temp = ' ';   /* Temporary variable holding characters. */
   char *buffer;        /* used to get units without [] */
   int i_temp;          /* Used to store the time since the epoch */
   char timeBuffer[35]; /* Used to store the time in String form */
   double d_temp;       /* Used to help with Projection in hours. */
   int origin[2];       /* originating center/subcenter. */
   float f_temp;        /* Temporary variable holding floats. */
   float unDef;         /* Aids with missing values. */
   char *ptr2;          /* Used to help with the original GRIB units. */
   uInt4 i;             /* Counter over the data. */
   float *data;         /* The data converted to a float. */
   myMaparam map;       /* Used to compute the grid lat/lon points. */

   /* Set up map projection. */
   if (GDSValid (&(meta->gds)) != 0) {
      preErrSprintf ("ERROR: Grid Definition Section was not Valid.\n");
      return -1;
   }
   SetMapParamGDS (&map, &(meta->gds));

   /* Begin the creation of the NetCDF dataset (enter define mode) */
   stat = nc_create (filename, NC_CLOBBER, &ncid);
   if (stat != NC_NOERR) {
      errSprintf ("ERROR in WriteNetCDF: %s %s", filename,
                  nc_strerror (stat));
      return -1;
   }

   /* Create the dimensions, variables and attributes for the weather info. */
   if (strcmp (meta->element, "Wx") == 0) {
      if (DefineWeatherInfo (ncid, &(meta->pds2.sect2.wx), &wx_id,
                             &wx_String_id, &wxKey_id) != 0)
         goto error;
      f_wx = 1;
   } else {
      f_wx = 0;
   }

   /* define dimensions */
   if (nc_DefineDim (ncid, "XCells", meta->gds.Nx, &NX_dim, __LINE__,
                     __FILE__) != 0)
      return -1;
   if (nc_DefineDim (ncid, "YCells", meta->gds.Ny, &NY_dim, __LINE__,
                     __FILE__) != 0)
      return -1;
   GridDimIds[0] = NY_dim;
   GridDimIds[1] = NX_dim;

   /* Define the main data field. */
   level = meta->shortFstLevel;
   if (strcmp (level, "0-SFC") == 0) {
      level = level + 2;
   } else if (strcmp (level, "0-0-SFC") == 0) {
      level = level + 4;
   }
   if (strlen (meta->element) + 1 + strlen (level) < NC_MAX_NAME) {
      sprintf (gridName, "%s_%s", meta->element, level);
   } else if (strlen (meta->element) < NC_MAX_NAME) {
      sprintf (gridName, "%s", meta->element);
   } else {
      strncpy (gridName, meta->element, NC_MAX_NAME);
   }
   if (f_wx) {
      if (nc_DefineVar (ncid, gridName, NC_SHORT, 2, GridDimIds, &grid_id,
                        __LINE__, __FILE__) != 0)
         goto error;
   } else {
      if (nc_DefineVar (ncid, gridName, NC_FLOAT, 2, GridDimIds, &grid_id,
                        __LINE__, __FILE__) != 0)
         goto error;
   }

   /* Create the dimensions, variables and attributes for the spatial
    * information. */
   if (DefineSpatialInfo (ncid, &(map), &(meta->gds), GridDimIds, &NX_id,
                          &NY_id, &lat_id, &lon_id, LatLon_Decimal,
                          f_NetCDF) != 0)
      goto error;

   /* Figure out the long_name of this data. */
   ptr = strchr (meta->comment, '[');
   if (ptr != NULL) {
      ptr--;
      c_temp = *ptr;
      *ptr = '\0';
   }
   if (nc_PutTextAtt (ncid, grid_id, "long_name", meta->comment, __LINE__,
                      __FILE__) != 0) {
      if (ptr != NULL) {
         *ptr = c_temp;
      }
      goto error;
   }
   if (ptr != NULL) {
      *ptr = c_temp;
   }

   /* Figure out the units of this data. */
   buffer = (char *) malloc (strlen (meta->unitName) + 1);
   ptr = strchr (meta->unitName, '[');
   if (ptr != NULL) {
      strcpy (buffer, ptr + 1);
   } else {
      strcpy (buffer, meta->unitName);
   }
   ptr = strchr (buffer, ']');
   if (ptr != NULL)
      *ptr = '\0';
   if (strcmp (buffer, "-") != 0) {
      if (nc_PutTextAtt (ncid, grid_id, "units", buffer, __LINE__,
                         __FILE__) != 0) {
         free (buffer);
         goto error;
      }
   }
   free (buffer);

   /* Cross reference to other variables in the NetCDF file. */
   if (nc_PutTextAtt (ncid, grid_id, "coordinates", "longitude latitude",
                      __LINE__, __FILE__) != 0)
      goto error;
   if (f_wx) {
      if (nc_PutTextAtt (ncid, grid_id, "keys", "Wx_SFC_wxKeys", __LINE__,
                         __FILE__) != 0)
         goto error;
   }

   /* Deal with time (reference and valid) */
   if (meta->GribVersion == 1) {
      i_temp = (int) meta->pds1.refTime;
/*      strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                gmtime (&(meta->pds1.refTime)));
*/
      Clock_Print (timeBuffer, 25, meta->pds1.refTime,
                   "%Y-%m-%d_%H:%M:%S_UTC", 0);
   } else if (meta->GribVersion == 2) {
      i_temp = (int) meta->pds2.refTime;
/*
      strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                gmtime (&(meta->pds2.refTime)));
*/
      Clock_Print (timeBuffer, 25, meta->pds2.refTime,
                   "%Y-%m-%d_%H:%M:%S_UTC", 0);
   } else {
      i_temp = (int) meta->pdsTdlp.refTime;
/*
      strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                gmtime (&(meta->pdsTdlp.refTime)));
*/
      Clock_Print (timeBuffer, 25, meta->pdsTdlp.refTime,
                   "%Y-%m-%d_%H:%M:%S_UTC", 0);
   }
   if (nc_PutIntAtt (ncid, grid_id, "ReferenceTime", NC_INT, 1, &i_temp,
                     __LINE__, __FILE__) != 0)
      goto error;
   if (nc_PutTextAtt (ncid, grid_id, "ReferenceTimeString", timeBuffer,
                      __LINE__, __FILE__) != 0)
      goto error;
   if (meta->GribVersion == 1) {
      i_temp = (int) meta->pds1.P1;
/*
      strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                gmtime (&(meta->pds1.P1)));
*/
      Clock_Print (timeBuffer, 25, meta->pds1.P1, "%Y-%m-%d_%H:%M:%S_UTC", 0);
   } else if (meta->GribVersion == 2) {
      i_temp = (int) meta->pds2.sect4.validTime;
/*
      strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                gmtime (&(meta->pds2.sect4.validTime)));
*/
      Clock_Print (timeBuffer, 25, meta->pds2.sect4.validTime,
                   "%Y-%m-%d_%H:%M:%S_UTC", 0);
   } else {
/*      time_t temp = meta->pdsTdlp.refTime + meta->pdsTdlp.project;*/
      i_temp = (int) meta->pdsTdlp.refTime + meta->pdsTdlp.project;
/*      strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                  gmtime (&(temp))); */
      Clock_Print (timeBuffer, 25, meta->pdsTdlp.refTime +
                   meta->pdsTdlp.project, "%Y-%m-%d_%H:%M:%S_UTC", 0);
   }
   if (nc_PutIntAtt (ncid, grid_id, "validTime", NC_INT, 1, &i_temp,
                     __LINE__, __FILE__) != 0)
      goto error;
   if (nc_PutTextAtt (ncid, grid_id, "ValidTimeString", timeBuffer,
                      __LINE__, __FILE__) != 0)
      goto error;

   if (meta->GribVersion == 1) {
      d_temp = (meta->pds1.P1 - meta->pds1.P2) / 3600.;
   } else if (meta->GribVersion == 2) {
      d_temp = meta->pds2.sect4.foreSec / 3600.;
   } else {
      d_temp = meta->pdsTdlp.project / 3600.;
   }
   if (nc_PutDoubleAtt (ncid, grid_id, "Projection_HR", NC_DOUBLE, 1,
                        &d_temp, __LINE__, __FILE__) != 0)
      goto error;

   /* Figure out if it is scalar or vector data. */
   if (f_wx) {
      if (nc_PutTextAtt (ncid, grid_id, "gridType", "WEATHER", __LINE__,
                         __FILE__) != 0)
         goto error;
   } else if ((strcmp (meta->element, "WindDir") == 0) ||
              (strcmp (meta->element, "WindSpeed") == 0)) {
      if (nc_PutTextAtt (ncid, grid_id, "gridType", "VECTOR", __LINE__,
                         __FILE__) != 0)
         goto error;
   } else {
      if (nc_PutTextAtt (ncid, grid_id, "gridType", "SCALAR", __LINE__,
                         __FILE__) != 0)
         goto error;
   }

   /* Handle Probability events. */
   if ((meta->GribVersion == 2) && (meta->pds2.sect4.templat == 9)) {
      d_temp = (meta->pds2.sect4.lowerLimit.value *
                pow (10, -1 * meta->pds2.sect4.lowerLimit.factor));
      if (nc_PutDoubleAtt (ncid, grid_id, "LowerLimit_ofProbabiltyEvent",
                           NC_DOUBLE, 1, &d_temp, __LINE__, __FILE__) != 0)
         goto error;
      d_temp = (meta->pds2.sect4.upperLimit.value *
                pow (10, -1 * meta->pds2.sect4.upperLimit.factor));
      if (nc_PutDoubleAtt (ncid, grid_id, "UpperLimit_ofProbabiltyEvent",
                           NC_DOUBLE, 1, &d_temp, __LINE__, __FILE__) != 0)
         goto error;
   }

   /* Handle the surface (or level). */
   if (nc_PutTextAtt (ncid, grid_id, "level", level, __LINE__, __FILE__) != 0)
      goto error;
   if (nc_PutTextAtt (ncid, grid_id, "DescriptiveLevel", meta->longFstLevel,
                      __LINE__, __FILE__) != 0)
      goto error;

   /* Handle the "author" information */
   origin[0] = meta->center;
   origin[1] = meta->subcenter;
   if (nc_PutIntAtt (ncid, grid_id, "OriginatingCenter", NC_INT, 2, origin,
                     __LINE__, __FILE__) != 0)
      goto error;

   /* Handle data specific information. */
   i_temp = decimal;
   if (nc_PutIntAtt (ncid, grid_id, "precision", NC_INT, 1, &i_temp,
                     __LINE__, __FILE__) != 0)
      goto error;
   if (meta->gridAttrib.f_maxmin) {
      if (f_wx) {
         f_temp = myRound (meta->gridAttrib.min, decimal);
         if (nc_PutFloatAtt (ncid, grid_id, "valid_min", NC_SHORT, 1,
                             &f_temp, __LINE__, __FILE__) != 0)
            goto error;
         f_temp = myRound (meta->gridAttrib.max, decimal);
         if (nc_PutFloatAtt (ncid, grid_id, "valid_max", NC_SHORT, 1,
                             &f_temp, __LINE__, __FILE__) != 0)
            goto error;
      } else {
         f_temp = myRound (meta->gridAttrib.min, decimal);
         if (nc_PutFloatAtt (ncid, grid_id, "valid_min", NC_FLOAT, 1,
                             &f_temp, __LINE__, __FILE__) != 0)
            goto error;
         f_temp = myRound (meta->gridAttrib.max, decimal);
         if (nc_PutFloatAtt (ncid, grid_id, "valid_max", NC_FLOAT, 1,
                             &f_temp, __LINE__, __FILE__) != 0)
            goto error;
      }
      i_temp = meta->gridAttrib.numMiss;
      if (nc_PutIntAtt (ncid, grid_id, "NumMissing", NC_INT, 1, &i_temp,
                        __LINE__, __FILE__) != 0)
         goto error;
   }

   /* Handle missing values. */
   unDef = 9999;
   if (meta->gridAttrib.f_miss != 0) {
      unDef = myRound (meta->gridAttrib.missPri, decimal);
      if (f_wx) {
         if (nc_PutFloatAtt (ncid, grid_id, "_FillValue", NC_SHORT, 1,
                             &unDef, __LINE__, __FILE__) != 0)
            goto error;
         /* ncview requested this. */
         if (nc_PutFloatAtt (ncid, grid_id, "missing_value", NC_SHORT, 1,
                             &unDef, __LINE__, __FILE__) != 0)
            goto error;
      } else {
         if (nc_PutFloatAtt (ncid, grid_id, "_FillValue", NC_FLOAT, 1,
                             &unDef, __LINE__, __FILE__) != 0)
            goto error;
         /* ncview requested this. */
         if (nc_PutFloatAtt (ncid, grid_id, "missing_value", NC_FLOAT, 1,
                             &unDef, __LINE__, __FILE__) != 0)
            goto error;
      }
   }

   /* Handle map projection information. */
   if (nc_PutTextAtt (ncid, grid_id, "grid_mapping", "MapProjection",
                      __LINE__, __FILE__) != 0)
      goto error;
   if (writeGDS (ncid, &map, grid_id, &(meta->gds), f_NetCDF) != 0)
      goto error;

   /* Handle GRIB specific helpful information. */
   i_temp = meta->GribVersion;
   if (nc_PutIntAtt (ncid, grid_id, "GRIBMessageVersion", NC_INT, 1, &i_temp,
                     __LINE__, __FILE__) != 0)
      goto error;
   ptr = strchr (meta->comment, '[');
   if (ptr != NULL) {
      ptr++;
      ptr2 = strchr (meta->comment, ']');
      if (ptr2 != NULL) {
         c_temp = *ptr2;
         *ptr2 = '\0';
      }
      if (nc_PutTextAtt (ncid, grid_id, "GRIBMessageUnits", ptr, __LINE__,
                         __FILE__) != 0) {
         if (ptr2 != NULL) {
            *ptr = c_temp;
         }
         goto error;
      }
      if (ptr2 != NULL) {
         *ptr = c_temp;
      }
   }
   if (meta->GribVersion == 1) {
      i_temp = meta->pds1.cat;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB1_ParameterCategory", NC_INT,
                        1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds1.genProcess;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB1_GenerationProcess", NC_INT,
                        1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds1.levelType;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB1_FixedSurfaceType", NC_INT,
                        1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds1.levelVal;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB1_FixedSurfaceValue", NC_INT,
                        1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
   } else if (meta->GribVersion == 2) {
      i_temp = meta->pds2.prodType;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB2_Discipline", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds2.sect4.templat;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB2_Sect4TemplateNumber", NC_INT,
                        1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds2.sect4.cat;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB2_Sect4ParameterCategory",
                        NC_INT, 1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds2.sect4.subcat;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB2_Sect4ParameterNumber", NC_INT,
                        1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds2.sect4.genProcess;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB2_GenerationProcess", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds2.sect4.fstSurfType;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB2_FstFixedSurfaceType", NC_INT,
                        1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      d_temp = meta->pds2.sect4.fstSurfValue;
      if (nc_PutDoubleAtt (ncid, grid_id, "GRIB2_FstFixedSurfaceValue",
                           NC_DOUBLE, 1, &d_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pds2.sect4.sndSurfType;
      if (nc_PutIntAtt (ncid, grid_id, "GRIB2_SndFixedSurfaceType", NC_INT,
                        1, &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      d_temp = meta->pds2.sect4.sndSurfValue;
      if (nc_PutDoubleAtt (ncid, grid_id, "GRIB2_SndFixedSurfaceValue",
                           NC_DOUBLE, 1, &d_temp, __LINE__, __FILE__) != 0)
         goto error;
   } else {
      i_temp = meta->pdsTdlp.ID1;
      if (nc_PutIntAtt (ncid, grid_id, "TDLP_ID1", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pdsTdlp.ID2;
      if (nc_PutIntAtt (ncid, grid_id, "TDLP_ID2", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pdsTdlp.ID3;
      if (nc_PutIntAtt (ncid, grid_id, "TDLP_ID3", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pdsTdlp.ID4;
      if (nc_PutIntAtt (ncid, grid_id, "TDLP_ID4", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pdsTdlp.procNum;
      if (nc_PutIntAtt (ncid, grid_id, "TDLP_ProcessNumber", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      i_temp = meta->pdsTdlp.seqNum;
      if (nc_PutIntAtt (ncid, grid_id, "TDLP_SequenceNumber", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      if (nc_PutTextAtt (ncid, grid_id, "TDLP_Comment",
                         meta->pdsTdlp.Descriptor, __LINE__, __FILE__) != 0)
         goto error;
   }

   /* Set some global Attributes. */
   if (nc_PutTextAtt (ncid, NC_GLOBAL, "title",
                      "GRIB Data translated to NetCDF", __LINE__,
                      __FILE__) != 0)
      goto error;
   if (nc_PutTextAtt (ncid, NC_GLOBAL, "Conventions", "CF-1.0", __LINE__,
                      __FILE__) != 0)
      goto error;
   if (nc_PutTextAtt (ncid, NC_GLOBAL, "CreatedBy", "degrib", __LINE__,
                      __FILE__) != 0)
      goto error;
   if (nc_PutTextAtt (ncid, NC_GLOBAL, "comment",
                      "Used degrib with: -NetCDF 1", __LINE__, __FILE__) != 0)
      goto error;
   if (nc_PutTextAtt (ncid, NC_GLOBAL, "references",
                      "http://www.nws.noaa.gov/mdl/NDFD_GRIB2Decoder/",
                      __LINE__, __FILE__) != 0)
      goto error;

   /* Leave define mode. */
   stat = nc_enddef (ncid);
   if (stat != NC_NOERR) {
      errSprintf ("ERROR leaving define mode in WriteNetCDF: %s %s",
                  filename, nc_strerror (stat));
      goto error;
   }

   if (StoreSpatialInfo (ncid, &map, &(meta->gds), NX_id, NY_id, lat_id,
                         lon_id, LatLon_Decimal, f_NetCDF) != 0)
      goto error;

   if (f_wx) {
      if (StoreWeatherInfo (ncid, &(meta->pds2.sect2.wx), wx_id,
                            wx_String_id, wxKey_id) != 0)
         goto error;
   }

   data = (float *) malloc (meta->gds.Nx * meta->gds.Ny * sizeof (float));
   for (i = 0; i < meta->gds.Nx * meta->gds.Ny; i++) {
      if ((meta->gridAttrib.f_miss == 2) &&
          (grib_Data[i] == meta->gridAttrib.missSec)) {
         data[i] = (float) unDef;
      } else if ((meta->gridAttrib.f_miss != 0) &&
                 (grib_Data[i] == meta->gridAttrib.missPri)) {
         data[i] = (float) unDef;
      } else {
         data[i] = myRound (grib_Data[i], decimal);
      }
   }
   if (nc_PutFloatVar (ncid, grid_id, data, __LINE__, __FILE__) != 0) {
      free (data);
      goto error;
   }
   free (data);

   /* close: save new netCDF dataset */
   nc_close (ncid);
   return 0;

 error:
   nc_close (ncid);
   return -1;
}

/*****************************************************************************
 * netCDF_V2() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This stores the GRIB data to a NetCDF file (which follows the CF
 * conventions as closely as I could make it.
 *   Implements the second version of NetCDF for degrib
  *
 * ARGUMENTS
 *       filename = The file to write the data to. (Input)
 *      grib_data = The decoded Grid of data to write to file. (Input)
 *           meta = meta data for the GRIB data to be stored in NetCDF (In)
 *       f_NetCDF = Version of degrib-NetCDF to use (1, or 2) (In)
 *        decimal = How many decimals to store the data with. (Input)
 * LatLon_Decimal = How many decimals to calculate lat/lon to. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *  11/2004 Jason Craig (NCAR / RAP): Modified to handle multiple grids in
 *          the same NetCDF file.
 *  12/2004 Arthur Taylor (MDL): Updated indentation style.
 *   1/2005 AAT: Updated for pdsTdlp type
 *   9/2005 AAT: Modified to handle version 3 which should be more CF
 *          compliant
 *   3/2007 AAT: Realized that msgNum is not actually used anymore (removed).
 *
 * NOTES
 *   See: http://www.cgd.ucar.edu/cms/eaton/cf-metadata/CF-1.0.html#gmap
 * for a convention on grid specifications.
 *****************************************************************************
 */
static int netCDF_V2 (char *filename, double *grib_Data,
                      grib_MetaData *meta, sChar f_NetCDF, sChar decimal,
                      sChar LatLon_Decimal)
{
   int ncid;            /* netCDF file id */
   int stat;            /* Return value from NetCDF call */
   int Hr_dim;          /* Hour Dimension ID */
   int NX_dim;          /* NX Dimension ID */
   int NY_dim;          /* NY Dimension ID */
   int NX_id;           /* Axis-Nx Variable ID */
   int NY_id;           /* Axis-Ny Variable ID */
   int Pr_id;           /* Projection Hour Variable ID */
   int lon_id;          /* longitude Variable ID */
   int lat_id;          /* latitude Variable ID */
   int GridDimIds[3];   /* Dimension ID's for the grid (Hr,NY,NX) */
   char f_wx = 0;       /* Boolean as to whether we have a weather grid. */
   int wx_id;           /* Axis-WX Variable ID. */
   int wx_String_id;    /* Axis-WX_STRING Variable ID. */
   int wxKey_id;        /* Weather Key variable ID. */
   char *level;         /* Points to the interesting part of the level name */
   char gridName[NC_MAX_NAME]; /* Name of the Grid Data. */
   int grid_id;         /* Grid Variable ID */
   char *ptr;           /* Assist with various text strings */
   char c_temp = ' ';   /* Temporary variable holding characters. */
   char *buffer;        /* used to get units without [] */
   int i_temp;          /* Used to store the time since the epoch */
   int i_temp2;
   char timeBuffer[30]; /* Used to store the time in String form */
   double d_temp;       /* Used to help with Projection in hours. */
   double d_temp2;
   int origin[2];       /* originating center/subcenter. */
   float f_temp;        /* Temporary variable holding floats. */
   float f_temp2;
   short int si_temp;
   float unDef;         /* Aids with missing values. */
   char *ptr2;          /* Used to help with the original GRIB units. */
   size_t i;            /* Counter over the data. */
   float *data;         /* The data converted to a float. */
   char new_ncdf;       /* Boolean as to weather this is a new netcdf file */
   char create_gridName = 1; /* Boolean as to weather to create gridName
                              * variable */
   short int Pr_hr;     /* Projection Hour of GRIB Data */
   char Pr_gridName[NC_MAX_NAME];
   size_t Nx, Ny;       /* Size of Nx Ny Axis' */
   size_t Hr;           /* Size of the time Axis */
   int ndimsp;          /* Number of Dimensions */
   size_t start[3];     /* Start indexs for variable put */
   size_t count[3];     /* Count numbers for variable put */
   myMaparam map;       /* Used to compute the grid lat/lon points. */
   nc_type t_type;      /* Used to test version of degrib -NetCDF option */
   size_t t_len;        /* length of t_type */
   size_t insertIndex;  /* Where to insert the data. */
   short int *sp;       /* pointer to array of short ints. */
   sChar f_makeRoom;

   /* Set up map projection. */
   if (GDSValid (&(meta->gds)) != 0) {
      preErrSprintf ("ERROR: Grid Definition Section was not Valid.\n");
      return -1;
   }
   SetMapParamGDS (&map, &(meta->gds));

   /* Get the Main Data Field's Name */
   level = meta->shortFstLevel;
   if (strcmp (level, "0-SFC") == 0) {
      level = level + 2;
   } else if (strcmp (level, "0-0-SFC") == 0) {
      level = level + 4;
   }
   if (strlen (meta->element) + 1 + strlen (level) < NC_MAX_NAME) {
      sprintf (gridName, "%s_%s", meta->element, level);
   } else if (strlen (meta->element) < NC_MAX_NAME) {
      sprintf (gridName, "%s", meta->element);
   } else {
      strncpy (gridName, meta->element, NC_MAX_NAME);
   }
   if (f_NetCDF > 2) {
      for (i = 0; i < strlen (gridName); i++) {
         if (gridName[i] == '-') {
            gridName[i] = '_';
         }
      }
   }

/*   sprintf (Pr_gridName, "%s_%s", "Pr_HR", meta->element); */
   if (f_NetCDF > 2) {
      sprintf (Pr_gridName, "%s", "ProjectionHr");
   } else {
      sprintf (Pr_gridName, "%s", "ProjectionHour");
   }

   /* Check to see if NetCDF dataset exists */
   new_ncdf = 1;
   Hr = 0;
   if (nc_open (filename, NC_WRITE, &ncid) == NC_NOERR) {
      /* Check if this is one of "our" netCDF files. */
      stat = nc_inq_att (ncid, NC_GLOBAL, "comment", &t_type, &t_len);
      if ((stat == NC_NOERR) && (t_type == NC_CHAR)) {
         buffer = (char *) malloc (t_len + 1);
         stat = nc_get_att_text (ncid, NC_GLOBAL, "comment", buffer);
         buffer[t_len] = '\0';
         /* Check if this is one of "our" netCDF files. */
         if (strncmp (buffer, "Used degrib with: -NetCDF", 25) == 0) {
            /* Check that the version in the file matches the version we're
             * trying to create. */
            if ((f_NetCDF == 2) && (strcmp (buffer + 25, " 2") == 0)) {
               new_ncdf = 0;
            } else if ((f_NetCDF == 3) && (strcmp (buffer + 25, " 3") == 0)) {
               new_ncdf = 0;
            } else {
               nc_close (ncid);
               new_ncdf = 1;
            }
            free (buffer);
         } else {
            free (buffer);
            errSprintf ("Unrecognized NetCDF file %s. Aborting", filename);
            return -1;
         }
      } else {
         errSprintf ("Unrecognized NetCDF file %s. Aborting", filename);
         return -1;
      }

      if (!new_ncdf) {
         /* Check dataset for correct dimensions */
         if (f_NetCDF > 2) {
            stat = nc_inq_dimid (ncid, "ProjectionHr", &Hr_dim);
         } else {
            stat = nc_inq_dimid (ncid, "HrCells", &Hr_dim);
         }
         stat = nc_inq_dimlen (ncid, Hr_dim, &Hr);
         stat = nc_inq_dimid (ncid, "XCells", &NX_dim);
         stat = nc_inq_dimlen (ncid, NX_dim, &Nx);
         stat = nc_inq_dimid (ncid, "YCells", &NY_dim);
         stat = nc_inq_dimlen (ncid, NY_dim, &Ny);
         GridDimIds[0] = Hr_dim;
         GridDimIds[1] = NY_dim;
         GridDimIds[2] = NX_dim;
         if ((Nx != meta->gds.Nx) || (Ny != meta->gds.Ny)) {
            errSprintf ("ERROR Dimensions in %s do not match GRIB data",
                        filename);
            return -1;
         }
         if (strcmp (meta->element, "Wx") == 0) {
            /* Wx check on opened dataset not implemented yet */
            f_wx = 1;
         } else {
            f_wx = 0;
         }

         /* Verify Main Data Field */
         if (nc_inq_varid (ncid, gridName, &grid_id) != NC_NOERR) {
            /* Take the next two lines out to allow multiple variables in one 
             * file */
            errSprintf ("ERROR Variable %s in %s does not exist", gridName,
                        filename);
            goto error;
         } else {
            /* Figure out the refTime of this data. */
            if (meta->GribVersion == 1) {
               i_temp = (int) meta->pds1.refTime;
            } else if (meta->GribVersion == 2) {
               i_temp = (int) meta->pds2.refTime;
            } else {
               i_temp = (int) meta->pdsTdlp.refTime;
            }
            if (nc_GetIntAtt (ncid, grid_id, "ReferenceTime", &i_temp2,
                              __LINE__, __FILE__) != 0) {
               new_ncdf = 1;
            } else if (i_temp != i_temp2) {
               new_ncdf = 1;
            }

            /* Figure out the units of this data. */
            buffer = (char *) malloc (strlen (meta->unitName) + 1);
            ptr = strchr (meta->unitName, '[');
            if (ptr != NULL) {
               strcpy (buffer, ptr + 1);
            } else {
               strcpy (buffer, meta->unitName);
            }
            ptr = strchr (buffer, ']');
            if (ptr != NULL)
               *ptr = '\0';
            if (strcmp (buffer, "-") != 0) {
               /* Should have a unit attribute */
               if (nc_GetTextAtt (ncid, grid_id, "units", &ptr, __LINE__,
                                  __FILE__) != 0) {
                  new_ncdf = 1;
               } else {
                  /* unit attribute should match buffer */
                  if (strcmp (ptr, buffer) != 0) {
                     new_ncdf = 1;
                  }
               }
            } else {
               /* Should NOT have a unit attribute */
               /* 1 says that "units" does not exist */
               if (nc_GetTextAtt (ncid, grid_id, "units", &ptr, __LINE__,
                                  __FILE__) != 1) {
                  new_ncdf = 1;
               }
            }
            free (ptr);
            free (buffer);

            if (nc_inq_varid (ncid, Pr_gridName, &Pr_id) != NC_NOERR) {
               errSprintf ("ERROR Variable %s in %s does not exist",
                           Pr_gridName, filename);
               goto error;
            }
            create_gridName = 0;

            stat = nc_inq_var (ncid, grid_id, 0, 0, &ndimsp, GridDimIds, 0);
            if ((ndimsp != 3) || (GridDimIds[0] != Hr_dim) ||
                (GridDimIds[1] != NY_dim) || (GridDimIds[2] != NX_dim)) {
               errSprintf ("ERROR Variable %s in %s has bad dimensions",
                           gridName, filename);
               return -1;
            }
         }
         stat = nc_redef (ncid);
      }
   }
   if (new_ncdf) {
      /* Begin the creation of the NetCDF dataset (enter define mode) */
      stat = nc_create (filename, NC_CLOBBER, &ncid);
      if (stat != NC_NOERR) {
         errSprintf ("ERROR in WriteNetCDF: %s %s", filename,
                     nc_strerror (stat));
         return -1;
      }
      /* Create the dimensions, variables and attributes for the weather
       * info. */
      if (strcmp (meta->element, "Wx") == 0) {
         if (DefineWeatherInfo (ncid, &(meta->pds2.sect2.wx), &wx_id,
                                &wx_String_id, &wxKey_id) != 0)
            goto error;
         f_wx = 1;
      } else {
         f_wx = 0;
      }

      /* Define dimensions */
      if (f_NetCDF > 2) {
         if (nc_DefineDim (ncid, "ProjectionHr", NC_UNLIMITED, &Hr_dim,
                           __LINE__, __FILE__) != 0)
            return -1;
      } else {
         if (nc_DefineDim (ncid, "HrCells", NC_UNLIMITED, &Hr_dim, __LINE__,
                           __FILE__) != 0)
            return -1;
      }
      if (nc_DefineDim (ncid, "YCells", meta->gds.Ny, &NY_dim, __LINE__,
                        __FILE__) != 0)
         return -1;
      if (nc_DefineDim (ncid, "XCells", meta->gds.Nx, &NX_dim, __LINE__,
                        __FILE__) != 0)
         return -1;
      GridDimIds[0] = Hr_dim;
      GridDimIds[1] = NY_dim;
      GridDimIds[2] = NX_dim;

      /* Create the dimensions, variables and attributes for the spatial
       * information. */
      if (DefineSpatialInfo (ncid, &(map), &(meta->gds), GridDimIds, &NX_id,
                             &NY_id, &lat_id, &lon_id, LatLon_Decimal,
                             f_NetCDF) != 0)
         goto error;
   }

   /* Set up the undefined variable */
   unDef = 9999;
   if (meta->gridAttrib.f_miss != 0) {
      unDef = myRound (meta->gridAttrib.missPri, decimal);
   }

   if (new_ncdf || create_gridName) {
      /* Define the main data field. */
      if (f_wx) {
         if (nc_DefineVar (ncid, gridName, NC_SHORT, 3, GridDimIds, &grid_id,
                           __LINE__, __FILE__) != 0)
            goto error;
      } else {
         if (nc_DefineVar (ncid, gridName, NC_FLOAT, 3, GridDimIds, &grid_id,
                           __LINE__, __FILE__) != 0)
            goto error;
      }

      /* Figure out the long_name of this data. */
      ptr = strchr (meta->comment, '[');
      if (ptr != NULL) {
         ptr--;
         c_temp = *ptr;
         *ptr = '\0';
      }
      if (nc_PutTextAtt (ncid, grid_id, "long_name", meta->comment, __LINE__,
                         __FILE__) != 0) {
         if (ptr != NULL) {
            *ptr = c_temp;
         }
         goto error;
      }
      if (ptr != NULL) {
         *ptr = c_temp;
      }

      /* Figure out the units of this data. */
      buffer = (char *) malloc (strlen (meta->unitName) + 1);
      ptr = strchr (meta->unitName, '[');
      if (ptr != NULL) {
         strcpy (buffer, ptr + 1);
      } else {
         strcpy (buffer, meta->unitName);
      }
      ptr = strchr (buffer, ']');
      if (ptr != NULL)
         *ptr = '\0';
      if (strcmp (buffer, "-") != 0) {
         if (nc_PutTextAtt (ncid, grid_id, "units", buffer, __LINE__,
                            __FILE__) != 0) {
            free (buffer);
            goto error;
         }
      }
      free (buffer);

      /* Cross reference to other variables in the NetCDF file. */
      if (nc_PutTextAtt (ncid, grid_id, "coordinates", "longitude latitude",
                         __LINE__, __FILE__) != 0)
         goto error;
      if (f_wx) {
         if (nc_PutTextAtt (ncid, grid_id, "keys", "Wx_SFC_wxKeys", __LINE__,
                            __FILE__) != 0)
            goto error;
      }

      /* Deal with time (reference and valid) */
      if (meta->GribVersion == 1) {
         i_temp = (int) meta->pds1.refTime;
         if (f_NetCDF == 2) {
/*            strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                      gmtime (&(meta->pds1.refTime)));*/
            Clock_Print (timeBuffer, 25, meta->pds1.refTime,
                         "%Y-%m-%d_%H:%M:%S_UTC", 0);
         } else {
/*            strftime (timeBuffer, 30, "%Y-%m-%d %H:%M:%S 00:00",
                      gmtime (&(meta->pds1.refTime)));
*/
            Clock_Print (timeBuffer, 30, meta->pds1.refTime,
                         "%Y-%m-%d %H:%M:%S 00:00", 0);
         }
      } else if (meta->GribVersion == 2) {
         i_temp = (int) meta->pds2.refTime;
         if (f_NetCDF == 2) {
/*            strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                      gmtime (&(meta->pds2.refTime)));
*/
            Clock_Print (timeBuffer, 25, meta->pds2.refTime,
                         "%Y-%m-%d_%H:%M:%S_UTC", 0);
         } else {
/*            strftime (timeBuffer, 30, "%Y-%m-%d %H:%M:%S 00:00",
                      gmtime (&(meta->pds2.refTime)));
*/
            Clock_Print (timeBuffer, 30, meta->pds2.refTime,
                         "%Y-%m-%d %H:%M:%S 00:00", 0);
         }
      } else {
         i_temp = (int) meta->pdsTdlp.refTime;
         if (f_NetCDF == 2) {
/*            strftime (timeBuffer, 25, "%Y-%m-%d_%H:%M:%S_UTC",
                      gmtime (&(meta->pdsTdlp.refTime)));
*/
            Clock_Print (timeBuffer, 25, meta->pdsTdlp.refTime,
                         "%Y-%m-%d_%H:%M:%S_UTC", 0);
         } else {
/*
            strftime (timeBuffer, 30, "%Y-%m-%d %H:%M:%S 00:00",
                      gmtime (&(meta->pdsTdlp.refTime)));
*/
            Clock_Print (timeBuffer, 30, meta->pdsTdlp.refTime,
                         "%Y-%m-%d %H:%M:%S 00:00", 0);
         }
      }
      if (nc_PutIntAtt (ncid, grid_id, "ReferenceTime", NC_INT, 1, &i_temp,
                        __LINE__, __FILE__) != 0)
         goto error;
      if (nc_PutTextAtt (ncid, grid_id, "ReferenceTimeString", timeBuffer,
                         __LINE__, __FILE__) != 0)
         goto error;

      /* Define the Projection Hour Variable */
      if (nc_DefineVar (ncid, Pr_gridName, NC_SHORT, 1, &(GridDimIds[0]),
                        &Pr_id, __LINE__, __FILE__) != 0)
         goto error;

      /* Projection Hour Attributes */
      if (nc_PutTextAtt (ncid, Pr_id, "long_name", "Projection Hour After"
                         " ReferenceTime", __LINE__, __FILE__) != 0)
         goto error;

      if (f_NetCDF == 2) {
         if (nc_PutTextAtt (ncid, Pr_id, "units", "Hours", __LINE__,
                            __FILE__) != 0)
            goto error;
      } else {
         if (meta->GribVersion == 1) {
            i_temp = (int) meta->pds1.refTime;
/*
            strftime (timeBuffer, 30, "%Y-%m-%d %H:%M:%S 00:00",
                      gmtime (&(meta->pds1.refTime)));
*/
            Clock_Print (timeBuffer, 30, meta->pds1.refTime,
                         "%Y-%m-%d %H:%M:%S 00:00", 0);
         } else if (meta->GribVersion == 2) {
            i_temp = (int) meta->pds2.refTime;
/*
            strftime (timeBuffer, 30, "%Y-%m-%d %H:%M:%S 00:00",
                      gmtime (&(meta->pds2.refTime)));
*/
            Clock_Print (timeBuffer, 30, meta->pds2.refTime,
                         "%Y-%m-%d %H:%M:%S 00:00", 0);
         } else {
            i_temp = (int) meta->pdsTdlp.refTime;
/*
            strftime (timeBuffer, 30, "%Y-%m-%d %H:%M:%S 00:00",
                      gmtime (&(meta->pdsTdlp.refTime)));
*/
            Clock_Print (timeBuffer, 30, meta->pdsTdlp.refTime,
                         "%Y-%m-%d %H:%M:%S 00:00", 0);
         }
         mallocSprintf (&buffer, "hours since %s", timeBuffer);
         if (nc_PutTextAtt (ncid, Pr_id, "units", buffer, __LINE__,
                            __FILE__) != 0) {
            free (buffer);
            goto error;
         }
         free (buffer);
      }
      if (nc_PutTextAtt (ncid, Pr_id, "_CoordinateAxisType", "Time",
                         __LINE__, __FILE__) != 0)
         return -1;
   }


   if (meta->GribVersion == 1) {
      i_temp = (int) meta->pds1.P1;
      if (f_NetCDF == 2) {
         Clock_Print (timeBuffer, 25, meta->pds1.P1,
                      "%Y-%m-%d_%H:%M:%S_UTC", 0);
      } else {
         Clock_Print (timeBuffer, 30, meta->pds1.P1,
                      "%Y-%m-%d %H:%M:%S 00:00", 0);
      }
   } else if (meta->GribVersion == 2) {
      i_temp = (int) meta->pds2.sect4.validTime;
      if (f_NetCDF == 2) {
         Clock_Print (timeBuffer, 25, meta->pds2.sect4.validTime,
                      "%Y-%m-%d_%H:%M:%S_UTC", 0);
      } else {
         Clock_Print (timeBuffer, 30, meta->pds2.sect4.validTime,
                      "%Y-%m-%d %H:%M:%S 00:00", 0);
      }
   } else {
      i_temp = (int) (meta->pdsTdlp.refTime + meta->pdsTdlp.project);
      if (f_NetCDF == 2) {
         Clock_Print (timeBuffer, 25, meta->pdsTdlp.refTime +
                      meta->pdsTdlp.project, "%Y-%m-%d_%H:%M:%S_UTC", 0);
      } else {
         Clock_Print (timeBuffer, 30, meta->pdsTdlp.refTime +
                      meta->pdsTdlp.project, "%Y-%m-%d %H:%M:%S 00:00", 0);
      }
   }
   if (meta->GribVersion == 1) {
      d_temp = (meta->pds1.P1 - meta->pds1.P2) / 3600.;
   } else if (meta->GribVersion == 2) {
      d_temp = meta->pds2.sect4.foreSec / 3600.;
   } else {
      d_temp = meta->pdsTdlp.project / 3600.;
   }
   if (new_ncdf) {
      if (nc_PutIntAtt (ncid, Pr_id, "FirstValidTime", NC_INT, 1, &i_temp,
                        __LINE__, __FILE__) != 0)
         goto error;
      if (nc_PutTextAtt (ncid, Pr_id, "FirstValidTimeString", timeBuffer,
                         __LINE__, __FILE__) != 0)
         goto error;
      if (nc_PutDoubleAtt (ncid, Pr_id, "FirstProjectionHR", NC_DOUBLE, 1,
                           &d_temp, __LINE__, __FILE__) != 0)
         goto error;
   } else {
      if (nc_GetDoubleAtt (ncid, Pr_id, "FirstProjectionHR", &d_temp2,
                           __LINE__, __FILE__) != 0)
         goto error;
      if (d_temp < d_temp2) {
         if (nc_PutIntAtt (ncid, Pr_id, "FirstValidTime", NC_INT, 1, &i_temp,
                           __LINE__, __FILE__) != 0)
            goto error;
         if (nc_PutTextAtt (ncid, Pr_id, "FirstValidTimeString", timeBuffer,
                            __LINE__, __FILE__) != 0)
            goto error;
         if (nc_PutDoubleAtt (ncid, Pr_id, "FirstProjectionHR", NC_DOUBLE, 1,
                              &d_temp, __LINE__, __FILE__) != 0)
            goto error;
      }
   }


   if (new_ncdf || create_gridName) {
      /* Figure out if it is scalar or vector data. */
      if (f_wx) {
         if (nc_PutTextAtt (ncid, grid_id, "gridType", "WEATHER", __LINE__,
                            __FILE__) != 0)
            goto error;
      } else if ((strcmp (meta->element, "WindDir") == 0) ||
                 (strcmp (meta->element, "WindSpeed") == 0)) {
         if (nc_PutTextAtt (ncid, grid_id, "gridType", "VECTOR", __LINE__,
                            __FILE__) != 0)
            goto error;
      } else {
         if (nc_PutTextAtt (ncid, grid_id, "gridType", "SCALAR", __LINE__,
                            __FILE__) != 0)
            goto error;
      }

      /* Handle Probability events. */
      if ((meta->GribVersion == 2) && (meta->pds2.sect4.templat == 9)) {
         d_temp = (meta->pds2.sect4.lowerLimit.value *
                   pow (10, -1 * meta->pds2.sect4.lowerLimit.factor));
         if (nc_PutDoubleAtt (ncid, grid_id, "LowerLimit_ofProbabiltyEvent",
                              NC_DOUBLE, 1, &d_temp, __LINE__, __FILE__) != 0)
            goto error;
         d_temp = (meta->pds2.sect4.upperLimit.value *
                   pow (10, -1 * meta->pds2.sect4.upperLimit.factor));
         if (nc_PutDoubleAtt (ncid, grid_id, "UpperLimit_ofProbabiltyEvent",
                              NC_DOUBLE, 1, &d_temp, __LINE__, __FILE__) != 0)
            goto error;
      }

      /* Handle the surface (or level). */
      if (nc_PutTextAtt (ncid, grid_id, "level", level, __LINE__,
                         __FILE__) != 0)
         goto error;
      if (nc_PutTextAtt (ncid, grid_id, "DescriptiveLevel",
                         meta->longFstLevel, __LINE__, __FILE__) != 0)
         goto error;

      /* Handle the "author" information */
      origin[0] = meta->center;
      origin[1] = meta->subcenter;
      if (nc_PutIntAtt (ncid, grid_id, "OriginatingCenter", NC_INT, 2,
                        origin, __LINE__, __FILE__) != 0)
         goto error;

      /* Handle data specific information. */
      i_temp = decimal;
      if (nc_PutIntAtt (ncid, grid_id, "precision", NC_INT, 1, &i_temp,
                        __LINE__, __FILE__) != 0)
         goto error;
   }

   if (new_ncdf) {
      if (meta->gridAttrib.f_maxmin) {
         if (f_wx) {
            f_temp = myRound (meta->gridAttrib.min, decimal);
            if (nc_PutFloatAtt (ncid, grid_id, "valid_min", NC_SHORT, 1,
                                &f_temp, __LINE__, __FILE__) != 0)
               goto error;
            f_temp = myRound (meta->gridAttrib.max, decimal);
            if (nc_PutFloatAtt (ncid, grid_id, "valid_max", NC_SHORT, 1,
                                &f_temp, __LINE__, __FILE__) != 0)
               goto error;
         } else {
            f_temp = myRound (meta->gridAttrib.min, decimal);
            if (nc_PutFloatAtt (ncid, grid_id, "valid_min", NC_FLOAT, 1,
                                &f_temp, __LINE__, __FILE__) != 0)
               goto error;
            f_temp = myRound (meta->gridAttrib.max, decimal);
            if (nc_PutFloatAtt (ncid, grid_id, "valid_max", NC_FLOAT, 1,
                                &f_temp, __LINE__, __FILE__) != 0)
               goto error;
         }
         i_temp = meta->gridAttrib.numMiss;
         if (nc_PutIntAtt (ncid, grid_id, "NumMissing", NC_INT, 1, &i_temp,
                           __LINE__, __FILE__) != 0)
            goto error;
      }
   } else {
      if (meta->gridAttrib.f_maxmin) {
         if (f_wx) {
            if (nc_GetShortAtt (ncid, grid_id, "valid_min", &si_temp,
                                __LINE__, __FILE__) != 0)
               goto error;
            f_temp2 = myRound (meta->gridAttrib.min, decimal);
            if (f_temp2 < si_temp) {
               if (nc_PutFloatAtt (ncid, grid_id, "valid_min", NC_SHORT, 1,
                                   &f_temp2, __LINE__, __FILE__) != 0)
                  goto error;
            }
            if (nc_GetShortAtt (ncid, grid_id, "valid_max", &si_temp,
                                __LINE__, __FILE__) != 0)
               goto error;
            f_temp2 = myRound (meta->gridAttrib.max, decimal);
            if (f_temp2 > si_temp) {
               if (nc_PutFloatAtt (ncid, grid_id, "valid_max", NC_SHORT, 1,
                                   &f_temp2, __LINE__, __FILE__) != 0)
                  goto error;
            }
         } else {
            if (nc_GetFloatAtt (ncid, grid_id, "valid_min", &f_temp,
                                __LINE__, __FILE__) != 0)
               goto error;
            f_temp2 = myRound (meta->gridAttrib.min, decimal);
            if (f_temp2 < f_temp) {
               if (nc_PutFloatAtt (ncid, grid_id, "valid_min", NC_FLOAT, 1,
                                   &f_temp2, __LINE__, __FILE__) != 0)
                  goto error;
            }
            if (nc_GetFloatAtt (ncid, grid_id, "valid_max", &f_temp,
                                __LINE__, __FILE__) != 0)
               goto error;
            f_temp2 = myRound (meta->gridAttrib.max, decimal);
            if (f_temp2 > f_temp) {
               if (nc_PutFloatAtt (ncid, grid_id, "valid_max", NC_FLOAT, 1,
                                   &f_temp2, __LINE__, __FILE__) != 0)
                  goto error;
            }
         }
      }
   }

   if (new_ncdf || create_gridName) {
      /* Handle missing values. */
      if (meta->gridAttrib.f_miss != 0) {
         if (f_wx) {
            if (nc_PutFloatAtt (ncid, grid_id, "_FillValue", NC_SHORT, 1,
                                &unDef, __LINE__, __FILE__) != 0)
               goto error;
            /* ncview requested this. */
            if (nc_PutFloatAtt (ncid, grid_id, "missing_value", NC_SHORT, 1,
                                &unDef, __LINE__, __FILE__) != 0)
               goto error;
         } else {
            if (nc_PutFloatAtt (ncid, grid_id, "_FillValue", NC_FLOAT, 1,
                                &unDef, __LINE__, __FILE__) != 0)
               goto error;
            /* ncview requested this. */
            if (nc_PutFloatAtt (ncid, grid_id, "missing_value", NC_FLOAT, 1,
                                &unDef, __LINE__, __FILE__) != 0)
               goto error;
         }
      }

      /* Handle map projection information. */
      if (nc_PutTextAtt (ncid, grid_id, "grid_mapping", "MapProjection",
                         __LINE__, __FILE__) != 0)
         goto error;
      if (f_NetCDF < 3) {
         if (writeGDS (ncid, &map, grid_id, &(meta->gds), f_NetCDF) != 0)
            goto error;
      }

      /* Handle GRIB specific helpful information. */
      i_temp = meta->GribVersion;
      if (nc_PutIntAtt (ncid, grid_id, "GRIBMessageVersion", NC_INT, 1,
                        &i_temp, __LINE__, __FILE__) != 0)
         goto error;
      ptr = strchr (meta->comment, '[');
      if (ptr != NULL) {
         ptr++;
         ptr2 = strchr (meta->comment, ']');
         if (ptr2 != NULL) {
            c_temp = *ptr2;
            *ptr2 = '\0';
         }
         if (nc_PutTextAtt (ncid, grid_id, "GRIBMessageUnits", ptr, __LINE__,
                            __FILE__) != 0) {
            if (ptr2 != NULL) {
               *ptr = c_temp;
            }
            goto error;
         }
         if (ptr2 != NULL) {
            *ptr = c_temp;
         }
      }
      if (meta->GribVersion == 1) {
         i_temp = meta->pds1.cat;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB1_ParameterCategory", NC_INT,
                           1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds1.genProcess;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB1_GenerationProcess", NC_INT,
                           1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds1.levelType;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB1_FixedSurfaceType", NC_INT,
                           1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds1.levelVal;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB1_FixedSurfaceValue", NC_INT,
                           1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
      } else if (meta->GribVersion == 2) {
         i_temp = meta->pds2.prodType;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB2_Discipline", NC_INT, 1,
                           &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds2.sect4.templat;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB2_Sect4TemplateNumber",
                           NC_INT, 1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds2.sect4.cat;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB2_Sect4ParameterCategory",
                           NC_INT, 1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds2.sect4.subcat;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB2_Sect4ParameterNumber",
                           NC_INT, 1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds2.sect4.genProcess;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB2_GenerationProcess", NC_INT,
                           1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds2.sect4.fstSurfType;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB2_FstFixedSurfaceType",
                           NC_INT, 1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         d_temp = meta->pds2.sect4.fstSurfValue;
         if (nc_PutDoubleAtt (ncid, grid_id, "GRIB2_FstFixedSurfaceValue",
                              NC_DOUBLE, 1, &d_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pds2.sect4.sndSurfType;
         if (nc_PutIntAtt (ncid, grid_id, "GRIB2_SndFixedSurfaceType",
                           NC_INT, 1, &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         d_temp = meta->pds2.sect4.sndSurfValue;
         if (nc_PutDoubleAtt (ncid, grid_id, "GRIB2_SndFixedSurfaceValue",
                              NC_DOUBLE, 1, &d_temp, __LINE__, __FILE__) != 0)
            goto error;
      } else {
         i_temp = meta->pdsTdlp.ID1;
         if (nc_PutIntAtt (ncid, grid_id, "TDLP_ID1", NC_INT, 1,
                           &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pdsTdlp.ID2;
         if (nc_PutIntAtt (ncid, grid_id, "TDLP_ID2", NC_INT, 1,
                           &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pdsTdlp.ID3;
         if (nc_PutIntAtt (ncid, grid_id, "TDLP_ID3", NC_INT, 1,
                           &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pdsTdlp.ID4;
         if (nc_PutIntAtt (ncid, grid_id, "TDLP_ID4", NC_INT, 1,
                           &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pdsTdlp.procNum;
         if (nc_PutIntAtt (ncid, grid_id, "TDLP_ProcessNumber", NC_INT, 1,
                           &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         i_temp = meta->pdsTdlp.seqNum;
         if (nc_PutIntAtt (ncid, grid_id, "TDLP_SequenceNumber", NC_INT, 1,
                           &i_temp, __LINE__, __FILE__) != 0)
            goto error;
         if (nc_PutTextAtt (ncid, grid_id, "TDLP_Comment",
                            meta->pdsTdlp.Descriptor, __LINE__,
                            __FILE__) != 0)
            goto error;
      }
   }

   if (new_ncdf) {
      /* Set some global Attributes. */
      if (nc_PutTextAtt (ncid, NC_GLOBAL, "title",
                         "GRIB Data translated to NetCDF", __LINE__,
                         __FILE__) != 0)
         goto error;
      if (nc_PutTextAtt (ncid, NC_GLOBAL, "Conventions", "CF-1.0", __LINE__,
                         __FILE__) != 0)
         goto error;
      if (nc_PutTextAtt (ncid, NC_GLOBAL, "CreatedBy", "degrib", __LINE__,
                         __FILE__) != 0)
         goto error;
      if (f_NetCDF == 2) {
         if (nc_PutTextAtt (ncid, NC_GLOBAL, "comment",
                            "Used degrib with: -NetCDF 2", __LINE__,
                            __FILE__) != 0)
            goto error;
      } else {
         if (nc_PutTextAtt (ncid, NC_GLOBAL, "comment",
                            "Used degrib with: -NetCDF 3", __LINE__,
                            __FILE__) != 0)
            goto error;
      }
      if (nc_PutTextAtt (ncid, NC_GLOBAL, "references",
                         "http://www.nws.noaa.gov/mdl/NDFD_GRIB2Decoder/",
                         __LINE__, __FILE__) != 0)
         goto error;
   }

   /* Leave define mode. */
   stat = nc_enddef (ncid);
   if (stat != NC_NOERR) {
      errSprintf ("ERROR leaving define mode in WriteNetCDF: %s %s",
                  filename, nc_strerror (stat));
      goto error;
   }

   if (new_ncdf) {
      if (StoreSpatialInfo (ncid, &map, &(meta->gds), NX_id, NY_id, lat_id,
                            lon_id, LatLon_Decimal, f_NetCDF) != 0)
         goto error;

      if (f_wx) {
         if (StoreWeatherInfo (ncid, &(meta->pds2.sect2.wx), wx_id,
                               wx_String_id, wxKey_id) != 0)
            goto error;
      }
   }

   Pr_hr = (int) (meta->pds2.sect4.foreSec / 3600.);
   f_makeRoom = 0;
   if (!new_ncdf) {
      start[0] = 0;
      count[0] = Hr;
      sp = (short int *) malloc (Hr * sizeof (short int));
      if (nc_GetShortVara (ncid, Pr_id, start, count, sp, __LINE__,
                           __FILE__) != 0) {
         free (sp);
         goto error;
      }
      for (i = 0; i < Hr; i++) {
         if (Pr_hr < sp[i]) {
            f_makeRoom = 1;
            break;
         }
         if (Pr_hr == sp[i]) {
            f_makeRoom = 0;
            break;
         }
      }
      insertIndex = i;
      /* Make room. */
      if (f_makeRoom) {
         for (i = Hr; i > insertIndex; i--) {
            start[0] = i;
            if (nc_PutShortIntVar1 (ncid, Pr_id, start, &(sp[i - 1]),
                                    __LINE__, __FILE__) != 0) {
               goto error;
            }
         }
      }
      free (sp);
   } else {
      insertIndex = 0;
   }

   start[0] = insertIndex;
   start[1] = 0;
   start[2] = 0;
   count[0] = 1;
   count[1] = (size_t) meta->gds.Ny;
   count[2] = (size_t) meta->gds.Nx;
   data = (float *) malloc (meta->gds.Nx * meta->gds.Ny * sizeof (float));
   /* Make room. */
   if (f_makeRoom) {
      for (i = Hr; i > insertIndex; i--) {
         start[0] = i - 1;
         if (nc_GetFloatVara (ncid, grid_id, start, count, data, __LINE__,
                              __FILE__) != 0) {
            free (data);
            goto error;
         }
         start[0] = i;
         if (nc_PutFloatVara (ncid, grid_id, start, count, data, __LINE__,
                              __FILE__) != 0) {
            free (data);
            goto error;
         }
      }

   }
   start[0] = insertIndex;

   /* Store the Grib data */
   for (i = 0; i < meta->gds.Nx * meta->gds.Ny; i++) {
      if ((meta->gridAttrib.f_miss == 2) &&
          (grib_Data[i] == meta->gridAttrib.missSec)) {
         data[i] = (float) unDef;
      } else if ((meta->gridAttrib.f_miss != 0) &&
                 (grib_Data[i] == meta->gridAttrib.missPri)) {
         data[i] = (float) unDef;
      } else {
         data[i] = myRound (grib_Data[i], decimal);
      }
   }
   if (nc_PutFloatVara (ncid, grid_id, start, count, data, __LINE__,
                        __FILE__) != 0) {
      free (data);
      goto error;
   }

   /* Store the Projection hour */
   if (nc_PutShortIntVar1 (ncid, Pr_id, start, &Pr_hr, __LINE__,
                           __FILE__) != 0) {
      free (data);
      goto error;
   }
   free (data);

   /* close: save new netCDF dataset */
   nc_close (ncid);
   return 0;

 error:
   nc_close (ncid);
   return -1;
}

/*****************************************************************************
 * gribWriteNetCDF() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   This stores the GRIB data to a NetCDF file (which follows the CF
 * conventions as closely as I could make it.
 *   Toggles between the first and second versions of NetCDF.  The second
 * version is recommended, but the first is provided for backward
 * compatibility.
 *
 * ARGUMENTS
 *       filename = The file to write the data to. (Input)
 *      grib_data = The decoded Grid of data to write to file. (Input)
 *           meta = meta data for the GRIB data to be stored in NetCDF (In)
 *       f_NetCDF = Version of degrib-NetCDF to use (1, or 2) (Input)
 *        decimal = How many decimals to store the data with. (Input)
 * LatLon_Decimal = How many decimals to calculate lat/lon to. (Input)
 *
 * FILES/DATABASES:
 *
 * RETURNS: int (could use errSprintf())
 *  0 = OK
 * -1 = Problems writing attributes.
 *
 * HISTORY
 *   5/2004 Arthur Taylor (MDL/RSIS): Created.
 *  11/2004 Jason Craig (NCAR / RAP): Modified to handle multiple grids in
 *          the same NetCDF file.
 *  12/2004 Arthur Taylor (MDL): Modified to toggle between netCDF_V1 and
 *          netCDF_V2.
 *   3/2007 AAT: Realized that msgNum is not actually used anymore (removed).
 *
 * NOTES
 *   See: http://www.cgd.ucar.edu/cms/eaton/cf-metadata/CF-1.0.html#gmap
 * for a convention on grid specifications.
 *****************************************************************************
 */
int gribWriteNetCDF (char *filename, double *grib_Data, grib_MetaData *meta,
                     sChar f_NetCDF, sChar decimal, sChar LatLon_Decimal)
{
   if (f_NetCDF == 1) {
      return (netCDF_V1 (filename, grib_Data, meta, f_NetCDF, decimal,
                         LatLon_Decimal));
   } else if ((f_NetCDF == 2) || (f_NetCDF == 3)) {
      return (netCDF_V2 (filename, grib_Data, meta, f_NetCDF, decimal,
                         LatLon_Decimal));
   } else {
      errSprintf ("Warning: Only 2 versions of NetCDF currently available\n"
                  "Using the default one");
      return (netCDF_V2 (filename, grib_Data, meta, f_NetCDF, decimal,
                         LatLon_Decimal));
   }
}
