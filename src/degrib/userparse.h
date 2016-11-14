#define PROGRAM_VERSION "2.08"
#define PROGRAM_DATE "11/10/2016"
/*****************************************************************************
 * userparse.h
 *
 * DESCRIPTION
 *    This file contains the code that is common to cstart.c and tcldegrib.c
 * when parsing the options provided by the user.
 *
 * HISTORY
 *    9/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#ifndef USERPARSE_H
#define USERPARSE_H

/* Include type.h for uChar and sChar */
#include "type.h"

/*
enum {
   CMD_INVENTORY, CMD_CONVERT, CMD_PROBE, CMD_VERSION, CMD_DATA,
   CMD_DATAPROBE, CMD_DATACONVERT, CMD_CALC, CMD_REFTIME, CMD_SECTOR,
   CMD_NCCONVERT, CMD_TOTAL
};
*/
enum {
   CMD_INVENTORY, CMD_CONVERT, CMD_PROBE, CMD_VERSION, CMD_DATA,
   CMD_DATAPROBE, CMD_DATACONVERT, CMD_REFTIME, CMD_SECTOR, CMD_NCCONVERT,
   CMD_SPLIT, CMD_TOTAL /*Mike*/
};

/* A structure containing the user's choices. */
typedef struct {
   char *cfgName;       /* cfgName = -cfg */
   size_t numInNames;   /* number of input files. */
   char **inNames;      /* inName = -in */
   char *f_inTypes;     /* file type from stat (1=dir, 2=file, 3=unknown). */
   sChar f_Command;     /* See CMD_* Enumeration.  */

/* sChar f_Inventory; */  /* f_Inventory = -I */
/* sChar f_Convert; */    /* f_Convert = -C */
/* sChar f_Probe; */      /* f_Probe = -P */
/* sChar f_Version; */    /* f_Version = -V */
/* sChar f_Data; */       /* f_Data = -Data */

   sChar f_Flt;         /* f_Flt = -Flt */
   sChar f_Met;         /* f_Met = -nMet */
   sChar f_IS0;         /* f_IS0 */
   sChar f_Freq;        /* f_Freq */
   sChar f_Map;         /* f_Map */
   sChar f_Shp;         /* f_Shp = -Shp */
   sChar f_Kml;         /* f_Kml = -Kml = 1, -Kmz = 2 */
   sChar f_kmlMerge;    /* True if we should merge by range of values */
   sChar f_verboseShp;  /* f_verboseShp = -verboseShp */
   sChar f_Csv;         /* f_Csv = -Csv */
   sChar f_Tdl;         /* f_TDL = -Tdl */
   sChar f_Grib2;       /* f_Grib2 = -Grib2 */
   sChar f_Cube;        /* f_Cube = -Cube */
   sChar f_Append;      /* f_Append = -Append */
	sChar f_poly;        /* Create polygon .shp or point .shp files? */
   sChar f_nMissing;    /* Don't store missing values in .shp files. */
   int msgNum;          /* msgNum = -msg (1..n) (0 means all messages). */
   int subgNum;         /* which subgrid in the message (0..m-1) */
   sChar f_unit;        /* f_unit = 0 -Unit n || 1 -Unit e || 2 -Unit m */
   sChar decimal;       /* How many decimals to round to. (default 3) */
   sChar LatLon_Decimal; /* How many decimals to round Lat/Lons (default 6) */
   char *nameStyle;     /* nameStyle = -nameStyle */
   char *namePath;      /* directory to store nameStyle files in. */
   char *sectFile;      /* Name of sectorFile (see -sectFile) */
   char *outName;       /* outName = -out or NULL (has 3 letter extension.) */
   sChar f_stdout;      /* true if outName is "stdout" */
   char *logName;       /* logName = -log or NULL (for error messages.) */
   sChar f_stormTotal;  /* f_stormTotal= -stormTotal Mike */
   sChar f_interp;      /* true = bilinear, false = Nearest Neighbor */
   sChar f_avgInterp;   /* true = if bilinear has missings, dist weight
                         *        average them, false (default) */
   sChar f_coverageGrid; /* (true if -Interp is provided on command line...
                         * This tells if we are calling gribInterpFloat()
                         * or gribWriteFloat() */
   sChar f_GrADS;       /* If we should create GrADS control file for .flt. */
   sChar f_AscGrid;     /* If we should create Ascii grids. */
   sChar f_NetCDF;      /* 0 for no NetCDF or integer for version of NetCDF
                         * to create (currently only have 1). */
   sChar f_XML;         /* What version of XML to create with a Probe. 0=None,
                         * 1=DWML version 1 */
   sChar f_Graph;       /* What version of -Graph to create with a Probe.
                         * 0=None, 1, etc */
   sChar f_MOTD;        /* What version of -MOTD to create with a Probe.
                         * 0=None, 1, etc */
   sChar f_SimpleWx;    /* If we should simplify the .flt file using the NDFD
                         * Weather table. */
   sChar f_SimpleVer;   /* Which version of the simple NDFD Weather table to
                         * use. (1 is 6/2003) (2 is 1/2004) (3 is 2/2004)
                         * (4 is 11/2004) (default 4) */
   sChar f_SimpleWWA;   /* Which version of the simple NDFD WWA table to
                         * use. (1 is 7/2008) (default 1) */
   sChar f_revFlt;      /* f_revFlt = -revFlt */
   sChar f_MSB;         /* f_MSB = -MSB */
   sChar f_validRange;  /* 0 if no -validMin / -validMax, 1 if -validMin,
                         * 2 if -validMax, 3 -validMin or -validMax. */
   double validMin;     /* -validMin value. Make sure the data is >= this
                         * value as a "sanity check". */
   double validMax;     /* -validMax value. Make sure the data is <= this
                         * value as a "sanity check". */
   char *pntFile;       /* pntFile = -pntFile */
   sChar f_pntStyle;    /* f_pntStyle = -pntStyle */
        /*  0 = Elem, Unit, refTime, valTime, (Value at Lat/lon 1), ... */
        /*  1 = Stn Name or (lat,lon), Elem[Unit], refTime, valTime, value */
   char *separator;     /* The separator to use for -P output. */
   sChar f_WxParse;     /* -WxParse option.  0 == ugly string,
                         * 1 == English Translation, 2 == -SimpleWx code */
                        /* Want to rename this to "f_txtParse" */
   sChar f_icon;        /* For XML, the icon version 0 == no icon,
                         * 1 == version 1 */
   Point *pnt;          /* pnt = -pnt option. */
   size_t numPnt;       /* number of points specified using the -pnt option. */
   size_t numCWA;       /* number of cwa's specified using the -cwa option
                         * (should = numPnt). */  
   LatLon lwlf;         /* lower left corner (cookie slicing) -lwlf */
   LatLon uprt;         /* upper right corner (cookie slicing) -uprt */
   sChar f_pntType;     /* 0 => lat/lon pnts, 1 => cells in pnts,
                         * 2 => all Cells. */
   sChar f_surface;     /* 0 => no surface info,
                         * 1 => short form of surface name
                         * 2 => long form of surface name */
   sChar f_nLabel;      /* 1 => Ignore pntFile label, 0 => use pntFile label*/
   double majEarth;     /* -radEarth if < 6000 ignore, otherwise use this to
                         * override the radEarth in the GRIB1 or GRIB2
                         * message.  Needed because NCEP lied.  They use
                         * 6371.2 but in GRIB1 could only state 6367.47. */
   double minEarth;     /* -minEarth if < 6000 ignore, otherwise use this to
                         * override the minEarth in the GRIB1 or GRIB2
                         * message. */
   char *indexFile;     /* indexFile = -Index option or NULL */
   char *mapIniFile;    /* mapIniFile = -MapIni option or NULL */
   char *kmlIniFile;    /* kmlIniFile = -KmlIni option or NULL */
   char *mapIniOptions; /* mapIniOption = -MapIniOptions or NULL */
   sChar f_Print;       /* Print option (for diagnosis of indexFile). */
   char *Asc2Flx_File;  /* Convert ASCII file to flx index file. */
   char *tmFormat;      /* Format for time strings. */
   sChar f_valTime;     /* -1 undefined, 0 false, 1 f_validStartTime,
                         * 2 f_validEndTime 3 both f_validStartTime,
                         * and f_validEndTime */
   double startTime;    /* Start of "valid times" we're interested (inclusive) */
   double endTime;      /* End of "valid times" we're interested (inclusive) */
   sInt4 numDays;       /* numDays (used when working with XML 3,4 */
   sChar f_timeFlavor;  /* -1 (have not seen StartDay,XML,numHr,startTime,etc
                         * 0 have seen XML 3,4 or StartDay,numHour
                         * 1 have seen XML 1,2,5 or StartTime,endTime
                         * It is an error to see a type 1 thing if we have seen
                         * a type 0 thing.
                         */
   char *ndfdVarsBuff;  /* The NDFD Variables string before we parse it. */
   sChar f_ndfdConven;  /* NDFD naming convention to use. */
   uChar *ndfdVars;     /* The array of NDFD variables that we are interested
                         * for probing.  (see NDFD enum in meta.h) */
   size_t numNdfdVars;  /* Number of "ndfdVars" */
   char *geoDataDir;    /* Directory to look in for NDFD geo data
                         * (timezone,daylightsavings) */
   char *gribFilter;    /* -gribFilter *.bin (pattern to use when filtering
                         * files for grib files. */
   char *lampDataDir;   /* Directory to look in for LAMP data. */
   char *rtmaDataDir;   /* Directory to look in for RTMA data. */
   char **cwaBuff;       /* Array holding 3 letter CWA's each point fall in. */
   
/* filter... for *.bin or *.ind or .. */
/*   sChar f_NDFDDir; */    /* If "input file" is a directory, then this describes
                         * the convention to use.
                         * 0 => "input file" should be a file.
                         * 1 => "input file" is a directory with *.bin, or
                         * *.ind (-DP, -DC) as interesting files
                         * 2 => revert to 0 except for (-DP / -P).  For them,
                         * use pnts to determine which "Major" sectors are of
                         * interest "conus,hawaii,guam,puertori"
                         * For AK/conus overlap we'll need some sort of mask
                         * Then combines "input file", "sector", ndfdVars,
                         * to create list of files to probe. */
/* Following 3 don't seem to be used. */
   /* char *matchElem; */    /* During Probe, entries have to match this element. */
   /* double matchRefTime; */ /* During Probe, entries have to match this refTime. */
   /* double matchValidTime; */ /* During Probe, entries match this validTime. */
} userType;

void UserInit (userType *usr);

void UserFree (userType *usr);

/* Possible error messages left in errSprintf() */
int UserValidate (userType *usr);

char *Grib2About (const char *name);

int IsUserOpt (char *str);

int UserParseCommandLine (userType * usr, int myArgc, char **myArgv);

int UserParseConfigFile (userType * usr, char *filename);

int expand_inName (size_t * NumInNames, char ***InNames,
                   char **F_inTypes, const char *filter);

#endif
