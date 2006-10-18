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
#include <ctype.h>

char float_var_fmt[] = "%f";
char double_var_fmt[] = "%f";
char float_att_fmt[] = "%f";
char double_att_fmt[] = "%f";

struct ncdim {          /* dimension */
   char name[NC_MAX_NAME];
   size_t size;
};

struct ncvar {          /* variable */
   char name[NC_MAX_NAME];
   nc_type type;
   int ndims;
   int dims[MAX_VAR_DIMS];
   int natts;
   int has_fillval;
   double fillval;
};

struct ncatt {          /* attribute */
   int var;
   char name[NC_MAX_NAME];
   nc_type type;
   size_t len;
   char *string;        /* for text attributes (type = NC_CHAR) */
   double *vals;        /* for numeric attributes of all types */
};

/* structure for list of variables specified with -v option */
struct vnode {
   struct vnode *next;
   int id;
};
typedef struct vnode vnode;

static char *type_name (nc_type type)
{
   switch (type) {
      case NC_BYTE:
         return "byte";
      case NC_CHAR:
         return "char";
      case NC_SHORT:
         return "short";
      case NC_INT:
         return "int";
      case NC_FLOAT:
         return "float";
      case NC_DOUBLE:
         return "double";
      default:
         printf ("type_name: bad type %d", type);
         return "bogus";
   }
}

/*
 * Remove trailing zeros (after decimal point) but not trailing decimal
 * point from ss, a string representation of a floating-point number that
 * might include an exponent part.
 */
static void tztrim (char *ss)
{
   char *cp, *ep;

   cp = ss;
   if (*cp == '-')
      cp++;
   while (isdigit ((int) *cp) || *cp == '.')
      cp++;
   if (*--cp == '.')
      return;
   ep = cp + 1;
   while (*cp == '0')
      cp--;
   cp++;
   if (cp == ep)
      return;
   while (*ep)
      *cp++ = *ep++;
   *cp = '\0';
   return;
}

/*
 * Print list of attribute values, for numeric attributes.  Attribute values
 * must be printed with explicit type tags, because CDL doesn't have explicit
 * syntax to declare an attribute type.
 */
static void pr_att_vals (nc_type type, size_t len, const double *vals)
{
   size_t iel;
   signed char sc;
   short ss;
   int ii;
   char gps[30];
   float ff;
   double dd;

   if (len == 0)
      return;
   for (iel = 0; iel < len - 1; iel++) {
      switch (type) {
         case NC_BYTE:
            sc = (signed char) vals[iel] & 0377;
            printf ("%db, ", sc);
            break;
         case NC_SHORT:
            ss = (short) vals[iel];
            printf ("%ds, ", ss);
            break;
         case NC_INT:
            ii = (int) vals[iel];
            printf ("%d, ", ii);
            break;
         case NC_FLOAT:
            ff = vals[iel];
            sprintf (gps, float_att_fmt, ff);
            tztrim (gps); /* trim trailing 0's after '.' */
            printf ("%s, ", gps);
            break;
         case NC_DOUBLE:
            dd = vals[iel];
            sprintf (gps, double_att_fmt, dd);
            tztrim (gps);
            printf ("%s, ", gps);
            break;
         default:
            printf ("pr_att_vals: bad type");
      }
   }
   switch (type) {
      case NC_BYTE:
         sc = (signed char) vals[iel] & 0377;
         printf ("%db", sc);
         break;
      case NC_SHORT:
         ss = (short) vals[iel];
         printf ("%ds", ss);
         break;
      case NC_INT:
         ii = (int) vals[iel];
         printf ("%d", ii);
         break;
      case NC_FLOAT:
         ff = vals[iel];
         sprintf (gps, float_att_fmt, ff);
         tztrim (gps);
         printf ("%s", gps);
         break;
      case NC_DOUBLE:
         dd = vals[iel];
         sprintf (gps, double_att_fmt, dd);
         tztrim (gps);
         printf ("%s", gps);
         break;
      default:
         printf ("pr_att_vals: bad type");
   }
}

/*
 * Print attribute string, for text attributes.
 */
static void pr_att_string (size_t len, const char *string)
{
   size_t iel;
   const char *cp;
   const char *sp;
   unsigned char uc;

   cp = string;
   printf ("\"");
   /* adjust len so trailing nulls don't get printed */
   sp = cp + len - 1;
   while (len != 0 && *sp-- == '\0')
      len--;
   for (iel = 0; iel < len; iel++)
      switch (uc = *cp++ & 0377) {
         case '\b':
            printf ("\\b");
            break;
         case '\f':
            printf ("\\f");
            break;
         case '\n':    /* generate linebreaks after new-lines */
            printf ("\\n\",\n\t\t\t\"");
            break;
         case '\r':
            printf ("\\r");
            break;
         case '\t':
            printf ("\\t");
            break;
         case '\v':
            printf ("\\v");
            break;
         case '\\':
            printf ("\\\\");
            break;
         case '\'':
            printf ("\\'");
            break;
         case '\"':
            printf ("\\\"");
            break;
         default:
            printf ("%c", uc);
            break;
      }
   printf ("\"");
}

static int pr_att (int ncid, int varid, const char *varname, int ia)
{
   struct ncatt att;    /* attribute */
   int stat;            /* Return value from NetCDF call */

   stat = nc_inq_attname (ncid, varid, ia, att.name);
   if (stat != NC_NOERR) {
      errSprintf ("ERROR in pr_att");
      return -1;
   }
   printf ("\t\t%s:%s = ", varname, att.name);

   stat = nc_inq_att (ncid, varid, att.name, &att.type, &att.len);
   if (stat != NC_NOERR) {
      errSprintf ("ERROR in pr_att");
      return -1;
   }

   if (att.len == 0) {  /* show 0-length attributes as empty strings */
      att.type = NC_CHAR;
      att.len = 1;
   }
   switch (att.type) {
      case NC_CHAR:
         att.string = (char *) malloc (att.len);
         if (!att.string) {
            printf ("Out of memory!");
            stat = nc_close (ncid);
            if (stat != NC_NOERR) {
               errSprintf ("ERROR in pr_att");
               return -1;
            }
            return 0;
         }
         stat = nc_get_att_text (ncid, varid, att.name, att.string);
         if (stat != NC_NOERR) {
            errSprintf ("ERROR in pr_att");
            return -1;
         }
         pr_att_string (att.len, att.string);
         free (att.string);
         break;
      default:
         att.vals = (double *) malloc (att.len * sizeof (double));
         if (!att.vals) {
            printf ("Out of memory!");
            stat = nc_close (ncid);
            if (stat != NC_NOERR) {
               errSprintf ("ERROR in pr_att");
               return -1;
            }
            return 0;
         }
         stat = nc_get_att_double (ncid, varid, att.name, att.vals);
         if (stat != NC_NOERR) {
            errSprintf ("ERROR in pr_att");
            return -1;
         }
         pr_att_vals (att.type, att.len, att.vals);
         free (att.vals);
         break;
   }
   printf (" ;\n");
   return 0;
}

/* Output the data for a single variable, in CDL syntax. */
/* vp = variable
 * vdims = variable dimension sizes
 * ncid = netcdf id
 * varid = variable id */
int vardata (const struct ncvar *vp, size_t vdims[], int ncid, int varid)
{
   return 0;
}

int gribReadNetCDF (char *filename)
{
   int ncid;            /* netCDF file id */
   int stat;            /* Return value from NetCDF call */
   int ndims;           /* number of dimensions */
   int nvars;           /* number of variables */
   int ngatts;          /* number of global attributes */
   int xdimid;          /* id of unlimited dimension */
   int dimid;           /* dimension id */
   int varid;           /* variable id */
   struct ncdim dims[NC_MAX_DIMS]; /* dimensions */
   size_t vdims[NC_MAX_DIMS]; /* dimension sizes for a single variable */
   struct ncvar var;    /* variable */
   struct ncatt att;    /* attribute */
   int id;              /* dimension number per variable */
   int ia;              /* attribute number */
   int is_coord;        /* true if variable is a coordinate variable */
   vnode *vlist = 0;    /* list for vars specified with -v option */
   int nc_status;       /* return from netcdf calls */

   stat = nc_open (filename, NC_NOWRITE, &ncid);
   if (stat != NC_NOERR) {
      errSprintf ("ERROR in ReadNetCDF: %s %s", filename, nc_strerror (stat));
      return -1;
   }

   /* 
    * get number of dimensions, number of variables, number of global
    * atts, and dimension id of unlimited dimension, if any
    */
   stat = nc_inq (ncid, &ndims, &nvars, &ngatts, &xdimid);
   if (stat != NC_NOERR) {
      errSprintf ("ERROR in ReadNetCDF: %s", nc_strerror (stat));
      return -1;
   }

   /* get dimension info */
   if (ndims > 0)
      printf ("dimensions:\n");
   for (dimid = 0; dimid < ndims; dimid++) {
      stat = nc_inq_dim (ncid, dimid, dims[dimid].name, &dims[dimid].size);
      if (stat != NC_NOERR) {
         errSprintf ("ERROR in ReadNetCDF: %s", nc_strerror (stat));
         return -1;
      }
      if (dimid == xdimid)
         printf ("\t%s = %s ; // (%ld currently)\n", dims[dimid].name,
                 "UNLIMITED", (long) dims[dimid].size);
      else
         printf ("\t%s = %ld ;\n", dims[dimid].name, (long) dims[dimid].size);
   }

   if (nvars > 0)
      printf ("variables:\n");
   /* get variable info, with variable attributes */
   for (varid = 0; varid < nvars; varid++) {
      stat = nc_inq_var (ncid, varid, var.name, &var.type, &var.ndims,
                         var.dims, &var.natts);
      if (stat != NC_NOERR) {
         errSprintf ("ERROR in ReadNetCDF: %s", nc_strerror (stat));
         return -1;
      }
      printf ("\t%s %s", type_name (var.type), var.name);
      if (var.ndims > 0)
         printf ("(");
      for (id = 0; id < var.ndims; id++) {
         printf ("%s%s", dims[var.dims[id]].name,
                 id < var.ndims - 1 ? ", " : ")");
      }
      printf (" ;\n");

      /* get variable attributes */
      for (ia = 0; ia < var.natts; ia++)
         pr_att (ncid, varid, var.name, ia); /* print ia-th attribute */
   }

   /* get global attributes */
   if (ngatts > 0)
      printf ("\n// global attributes:\n");
   for (ia = 0; ia < ngatts; ia++)
      pr_att (ncid, NC_GLOBAL, "", ia); /* print ia-th global attribute */

   /* Start dumping the data. */
   if (nvars > 0) {
      printf ("data:\n");
   }
   /* output variable data */
   for (varid = 0; varid < nvars; varid++) {
      stat = nc_inq_var (ncid, varid, var.name, &var.type, &var.ndims,
                         var.dims, &var.natts);
      if (stat != NC_NOERR) {
         errSprintf ("ERROR in ReadNetCDF: %s", nc_strerror (stat));
         return -1;
      }
      /* Find out if this is a coordinate variable */
      is_coord = 0;
      for (dimid = 0; dimid < ndims; dimid++) {
         if (strcmp (dims[dimid].name, var.name) == 0 && var.ndims == 1) {
            is_coord = 1;
            break;
         }
      }
      /* 
       * Only get data for variable if it is not a record variable,
       * or if it is a record variable and at least one record has
       * been written.
       */
      if ((var.ndims == 0) || (var.dims[0] != xdimid) ||
          (dims[xdimid].size != 0)) {
         /* Collect variable's dim sizes */
         for (id = 0; id < var.ndims; id++)
            vdims[id] = dims[var.dims[id]].size;
         var.has_fillval = 1; /* by default, but turn off for bytes */

         /* get _FillValue attribute */
         nc_status = nc_inq_att (ncid, varid, _FillValue, &att.type,
                                 &att.len);
         if ((nc_status == NC_NOERR) && (att.type == var.type) &&
             (att.len == 1)) {
            if (var.type == NC_CHAR) {
               char fillc;
               stat = nc_get_att_text (ncid, varid, _FillValue, &fillc);
               if (stat != NC_NOERR) {
                  errSprintf ("ERROR in ReadNetCDF: %s", nc_strerror (stat));
                  return -1;
               }
               var.fillval = fillc;
            } else {
               stat = nc_get_att_double (ncid, varid, _FillValue,
                                         &var.fillval);
               if (stat != NC_NOERR) {
                  errSprintf ("ERROR in ReadNetCDF: %s", nc_strerror (stat));
                  return -1;
               }
            }
         } else {
            switch (var.type) {
               case NC_BYTE:
                  /* don't do default fill-values for bytes, too risky */
                  var.has_fillval = 0;
                  break;
               case NC_CHAR:
                  var.fillval = NC_FILL_CHAR;
                  break;
               case NC_SHORT:
                  var.fillval = NC_FILL_SHORT;
                  break;
               case NC_INT:
                  var.fillval = NC_FILL_INT;
                  break;
               case NC_FLOAT:
                  var.fillval = NC_FILL_FLOAT;
                  break;
               case NC_DOUBLE:
                  var.fillval = NC_FILL_DOUBLE;
                  break;
               default:
                  break;
            }
         }
         if (vardata (&var, vdims, ncid, varid) == -1) {
            printf ("can't output data for variable %s", var.name);
            stat = nc_close (ncid);
            if (stat != NC_NOERR) {
               errSprintf ("ERROR in ReadNetCDF: %s", nc_strerror (stat));
               return -1;
            }
            if (vlist)
               free (vlist);
            return -1;
         }
      }
   }

   printf ("}\n");
   stat = nc_close (ncid);
   if (stat != NC_NOERR) {
      errSprintf ("ERROR in ReadNetCDF: %s", nc_strerror (stat));
      return -1;
   }
   if (vlist)
      free (vlist);
   return 0;
}

int Grib2NCConvert (userType *usr)
{
   return gribReadNetCDF (usr->inNames[0]);
}
