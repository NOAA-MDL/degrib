/*****************************************************************************
 * cstart.c
 *
 * DESCRIPTION
 *    This file contains the main code for the command line version of the
 * degrib program.  It parse's the command line input, and then calls the
 * correct procedures.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL / RSIS): Created.
 *  12/2002 Tim Kempisty, Ana Canizares, Tim Boyer, & Marc Saccucci
 *          (TK,AC,TB,&MS): Code Review 1.
 *
 * NOTES
 *   Following is useful for timing diagnostics:
 *      printf ("1 :: %f\n", clock() / (double) (CLOCKS_PER_SEC));
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "myerror.h"
#include "myutil.h"
#include "userparse.h"
#include "commands.h"
#include "myassert.h"
#include "clock.h"
#include "genprobe.h"
#ifdef MEMWATCH
#include "memwatch.h"
#endif

/*****************************************************************************
 * Usage() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   Prints the usage message.
 *
 * ARGUMENTS
 * argv0 = The name of the program. (Output)
 *   usr = The user inputed options (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: void
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *   8/2003 AAT: Added command based usage messages.
 *
 * NOTES
 *****************************************************************************
 */
static void Usage (char *argv0, userType *usr)
{
   printf ("usage: %s [input GRIB file (optional)] [options]\n", argv0);
   switch (usr->f_Command) {
      case CMD_INVENTORY:
         break;
      case CMD_CONVERT:
      case CMD_DATACONVERT:
         printf ("\nCONVERT OPTIONS (-C or -DC)\n");
         printf ("Default: -msg 1 -MSB -Flt -nShp -Met -Unit e -Decimal 3 "
                 "-nRevFlt\n");
         printf ("  -msg [msgNum].[subgrdNum] = Which grib message to "
                 "convert.\n");
         printf ("               If msgNum = 0 or 'all', do all messages."
                 "\n");
         printf ("               The subgrdNum starts at 0, and is used "
                 "for\n");
         printf ("                    multiple grids in the same message."
                 "\n");
         printf ("  -MSB or -nMSB = Generate binary files M.S.B. if "
                 "possible.\n");
         printf ("  -Flt or -nFlt = Create (or don't) Spatial Analyst .flt "
                 "file set.\n");
         printf ("  -Shp or -nShp = Create (or don't) ArcView .shp file set."
                 "\n");
         printf ("  -Shp2 = Same as -Shp except include lat/lon/x/y in "
                 "columns\n");
         printf ("  -Met or -nMet = Create (or don't) meta file set.\n");
         printf ("  -Csv or -nCsv = Create (or don't) comma/column separated"
                 " files.\n");
         printf ("  -NetCDF [0,1,2,3] = Create NetCDF files (0=Don't "
                 "1=degrib NetCDF version 1, 2=degrib NetCDF Version 2, "
                 "3=degrib NetCDF Version 3\n");
         printf ("  -Grib2 or -nGrib2 = Create (or don't) a GRIB2 file "
                 "(primarily for subgrids)\n");
         printf ("  -IS0         = Create text '.iso' file for debug "
                 "purposes\n");
         printf ("  -radEarth [6371.2 km] = Override the radEarth in GRIB "
                 "message.\n");
         printf ("  -Unit [type] = Use [type] units. [type] can be:\n");
         printf ("               'none' (use GRIB2 units (K, kg/m**2 or m, "
                 "m/s)\n");
         printf ("               'e' 'english' (use F, inches, knots)\n");
         printf ("               'm' 'metric' (use C, kg/m**2 or m, m/s)\n");
         printf ("  -Decimal [amount] = How many decimals to round to "
                 "[0..18]\n");
         printf ("\nFLT SPECIFIC OPTIONS (need -Flt)\n");
         printf ("  -GrADS [1,2] = Create version 1 or 2 of the .ctl file\n"
                 "for use with GrADS\n");
         printf ("  -Interp 0    = Use original grid.\n");
         printf ("  -Interp 1 (-Interp near) = Nearest point interpolate to "
                 "coverage grid.\n");
         printf ("  -Interp 2 (-Interp bilinear) = Bi-Linear Interpolate to "
                 "coverage grid.\n");
         printf ("  -revFlt or -nRevFlt = Create (or don't) .tlf file\n");
         printf ("                (start in lower left not upper left)\n");
         printf ("  -SimpleWx    = Use NDFD weather table to simplify "
                 "weather strings\n");
         printf ("  -SimpleVer 1 = use 6/2003 version of NDFD Simple Wx "
                 "code\n");
         printf ("  -SimpleVer 2 = use 1/2004 version of NDFD Simple Wx "
                 "code\n");
         printf ("  -SimpleVer 3 = use 2/2004 version of NDFD Simple Wx "
                 "code\n");
         printf ("\nSHP SPECIFIC OPTIONS (need -Shp)\n");
         printf ("  -poly 0      = Create points.\n");
         printf ("  -poly 1 (-poly small) = Create small polygons.\n");
         printf ("  -poly 2 (-poly big) = Create big or merged polygons.\n");
         printf ("  -nMissing    = Don't store missing values in .shp files."
                 "\n");
         break;
      case CMD_PROBE:
         printf ("\nPROBE OPTIONS (-P)\n");
         printf ("Default: -pntStyle 0 -WxParse 0 -Separator \", \" -Unit e "
                 "-Decimal 3\n");
         printf ("  -Interp      = Bi-Linear Interpolate to lat/lon.\n");
         printf ("  -pnt [lat,lon] = geographic point to probe."
                 " Longitudes are negative Westward.\n");
         printf ("  -pntFile [file] = file to find points to probe.\n");
         printf ("  -surface [form] = Add a column for the surface to the probed results\n");
         printf ("               [form] = short => abreviate\n");
         printf ("                      = long => full name\n");
         printf ("  -pntStyle [0,1] = Use point style where:\n");
         printf ("               0 => 'Elem,unit,refTime,valTime,loc1,loc2"
                 "..'\n");
         printf ("               1 =>'location,Elem[unit],refTime,"
                 "valTime,value'\n");
         printf ("  -nLabel      = Ignore the label using '(CellX,CellY,lat,lon)' instead.\n");
         printf ("  -cells [flag] = Probes all the cells using pntStyle 1\n");
         printf ("     [flag] = all => probe all grid cells regardless of -pnt/-pntFile options\n");
         printf ("            = true => Treat -pnt/-pntFiles option's lat/lons as CellY/cellX\n");
         printf ("  -validMax [value] = A maximum expected value in the field.\n");
         printf ("            If a cell is > 'value', file is corrupt, degrib should abort.\n");
         printf ("  -validMin [value] = A minimum expected value in the field. \n");
         printf ("            If a cell is < 'value', file is corrupt, degrib should abort.\n");
         printf ("  -WxParse [0,1,2] = Parse Weather?\n");
         printf ("               0 => print Weather ugly strings.\n");
         printf ("               1 => print 'English translation'\n");
         printf ("               2 => print -SimpleWx code\n");
         printf ("  -SimpleVer [version]: Use this simple weather code when working with Weather\n");
         printf ("               1 = 6/2003 version\n");
         printf ("               2 = 1/2004 version\n");
         printf ("               3 = 2/2004 version\n");
         printf ("               4 = 11/2004 version\n");
         printf ("  -Separator [string] = String to use as a separator in "
                 "output\n");
         printf ("  -radEarth [6371.2 km] = Override the radEarth in GRIB "
                 "message.\n");
         printf ("  -Unit [type] = Use [type] units. [type] can be:\n");
         printf ("               'none' (use GRIB2 units (K, kg/m**2 or m, "
                 "m/s)\n");
         printf ("               'e' 'english' (use F, inches, knots)\n");
         printf ("               'm' 'metric' (use C, kg/m**2 or m, m/s)\n");
         printf ("  -Decimal [amount] = How many decimals to round to "
                 "[0..18]\n");
         printf ("  -XML [type] = Create XML output at the specified point\n");
         printf ("      [type] = 0 => Don't create XML (default)\n");
         printf ("               1 => create DWML time-series product\n");
         printf ("               2 => create DWML glance product\n");
/*
         printf ("               3 => create DWML summarized over 12-hour periods\n");
         printf ("               4 => create DWML summarized over 24-hour periods\n");
*/
         printf ("      See also -ndfdVars, -ndfdConven, -Icon, -startTime, and -endTime\n");
         printf ("  -ndfdVars [string]\n");
         printf ("      Specifies the NDFD elements (wind speed, sky cover, weather, etc.)\n");
         printf ("      that you want to appear in the output DWML.  The string is a comma\n");
         printf ("      delimited list of weather element codes.  The element codes\n");
         printf ("      depend on the which -ndfdConven is used.  Omitting -ndfdVars will\n");
         printf ("      yield DWML for all ndfd elements.\n");
         printf ("   Example:  -ndfdVars mx,wx,dp (assumes an -ndfdConven 2)\n");
         printf ("      See also -gribFilter -ndfdConven\n");
         printf ("   -ndfdConven [type]\n");
         printf ("      Defines the naming convention used for files containing NDFD GRIB\n");
         printf ("      messages.  These names are also used with the -ndfdVars option to\n");
         printf ("      tell degrib which NDFD elements to process.  Omitting -ndfdConven\n");
         printf ("      is equivalent to -ndfdConven 1.\n");
         printf ("      [type]=0   1     2\n");
         printf ("      maxt       maxt  mx 'Maximum Temperature'\n");
         printf ("      mint       mint  mn 'Minimum Temperature'\n");
         printf ("      pop12      pop12 po '12-Hour Probability of Precipitation'\n");
         printf ("      t          temp  tt 'Temperature'\n");
         printf ("      winddir    wdir  wd 'Wind Direction'\n");
         printf ("      windspd    wspd  ws 'Wind Speed'\n");
         printf ("      td         td    dp 'Dewpoint Temperature'\n");
         printf ("      sky        sky   cl 'Sky Cover'\n");
         printf ("      qpf        qpf   qp 'Quantitative Precipitation Forecast'\n");
         printf ("      snowamt    snow  sn 'Snowfall Amount'\n");
         printf ("      wx         wx    wx 'Weather'\n");
         printf ("      waveheight waveh wh 'Wave Height'\n");
         printf ("      apparentt  apt   at 'Apparent Temperature (wind chill or heat index)'\n");
         printf ("      rh         rhm   rh 'Relative Humidity'\n");
         printf ("      windgust   wgust wg 'Wind Speed Gust'\n");
         printf ("      probwindspd34i tcwspdabv34i i3 'Probabilistic Tropical Cyclone Surface Wind Speeds > 34Kts (incremental)'\n");
         printf ("      probwindspd50i tcwspdabv50i i5 'Probabilistic Tropical Cyclone Surface Wind Speeds > 50Kts (incremental)'\n");
         printf ("      probwindspd64i tcwspdabv64i i6 'Probabilistic Tropical Cyclone Surface Wind Speeds > 64Kts (incremental)'\n");
         printf ("      probwindspd34c tcwspdabv34c c3 'Probabilistic Tropical Cyclone Surface Wind Speeds > 34Kts (cumulative)'\n");
         printf ("      probwindspd50c tcwspdabv50c c5 'Probabilistic Tropical Cyclone Surface Wind Speeds > 50Kts (cumulative)'\n");
         printf ("      probwindspd64c tcwspdabv64c c6 'Probabilistic Tropical Cyclone Surface Wind Speeds > 64Kts (cumulative)'\n");
         printf ("      fwxwindrh critriskfirewx fwxwdrh 'Fire Weather Risk from Wind and Relative Humidity'\n");
         printf ("      fwxdrytstorm drylightning fwxdry 'Fire Weather Risk from Dry Lightning'\n");
         printf ("      convoutlook conhazo ch 'Convective Hazard Outlook'\n");
         printf ("      tornadoprob ptornado pt 'Tornado Probability'\n");
         printf ("      hailprob   phail ph 'Hail Probability'\n");
         printf ("      windprob   ptstmwinds pw 'Damaging Thunderstorm Wind Probability'\n");
         printf ("      xtrmtornprob pxtornado xt 'Extreme Tornado Probability'\n");
         printf ("      xtrmhailprob pxhail xh 'Extreme Hail Probability'\n");
         printf ("      xtrmwindprob pxtstmwinds xw 'Extreme Thunderstorm Wind Probability'\n");
         printf ("      totalsvrprob ptotsvrtstm ps 'Total Probability of Severe Thunderstorms'\n");
         printf ("      totalxtrmprob ptotxsvrtstm xs 'Total Probability of Extreme Severe Thunderstorms'\n");
         printf ("      probtmpabv144 tmpabv14d ta6d 'Probability of 8-14 Day Average Temperature Above Normal'\n");
         printf ("      probtmpblw144 tmpblw14d tb6d 'Probability of 8-14 Day Average Temperature Below Normal'\n");
         printf ("      probprcpabv144 prcpabv14d pa6d 'Probability of 8-14 Day Average Precipitation Above Normal'\n");
         printf ("      probprcpblw144 prcpblw14d pb6d 'Probability of 8-14 Day Average Precipitatin Below Normal'\n");
         printf ("      probtmpabv01m tmpabv30d ta1m 'Probability of One-Month Average Temperature Above Normal'\n");
         printf ("      probtmpblw01m tmpblw30d tb1m 'Probability of One-Month Average Temperature Below Normal'\n");
         printf ("      probprcpabv01m prcpabv30d pa1m 'Probability of One-Month Average Precipitation Above Normal'\n");
         printf ("      probprcpblw01m prcpblw30d pb1m 'Probability of One-Month Average Precipitation Below Normal'\n");
         printf ("      probtmpabv03m tmpabv90d ta3m 'Probability of Three-Month Average Temperature Above Normal'\n");
         printf ("      probtmpblw03m tmpblw90d tb3m 'Probability of Three-Month Average Temperature Below Normal'\n");
         printf ("      probprcpabv03m prcpabv90d pa3m 'Probability of Three-Month Average Precipitation Above Normal'\n");
         printf ("      probprcpblw03m prcpblw90d pb3m 'Probability of Three-Month Average Precipitation Below Normal'\n");
         printf ("   Example:  -ndfdConven 1\n");
         printf ("   -gribFilter [string]\n");
         printf ("      By default when expanding a directory to find GRIB files or database\n");
         printf ("      index files, it looks for (in the GRIB case *.bin) (in the database\n");
         printf ("      case *.ind).  In addition, if it doesn't find *.bin (example maxt.bin),\n");
         printf ("      then it also looks for ds.*.bin (example ds.maxt.bin).  The reason for\n");
         printf ("      this is because while tcldegrib and tkdegrib rename the file without\n");
         printf ("      the 'ds.', the filename on the NDFD http site has a 'ds.'.\n");
         printf ("         The -gribFilter option lets you over ride the '*.bin' to whatever\n");
         printf ("      convention you are using.\n");
         printf ("         If the -ndfdVars option is given, then it uses that to further qualify\n");
         printf ("      the file.\n");
         printf ("   Example: -ndfdVars maxt,mint -gribFilter \"*.grb\"\n");
         printf ("         Means look for files maxt.grb, mint.grb.  If you can't find them,\n");
         printf ("         also look for ds.maxt.grb, ds.mint.grb.\n");
         printf ("   Example:  -gribFilter \"*.grb\"\n");
         printf ("   -Icon [type]\n");
         printf ("      Indicates that you want the DWML to contain icon information. The\n");
         printf ("      default value is -Icon 0 (no icon information).\n");
         printf ("      [type] is defined as:\n");
         printf ("         0 = Degrib will not format icon XML elements in the DWML.\n");
         printf ("         1 = Degrib will format icon XML elements in the DWML.\n");
         printf ("   Example:  -Icon 1\n");
         printf ("      NOTE:  To format icon elements, degrib will need temperature,\n");
         printf ("             sky cover, wind speed, and weather files.\n");
         printf ("   -startTime [string]\n");
         printf ("      Establishes the beginning time of the period for which you want data.\n");
         printf ("      The value is a UTC time and can be expressed in several ways.  If the\n");
         printf ("      start time is omitted, the DWML document will contain the most recent\n");
         printf ("      data available.  Two of the ways you can define the time string are:\n");
         printf ("         YYYY-MM-DDTHH:MM:SS\n");
         printf ("             YYYY = 4 digit year\n");
         printf ("             MM   = 2 digit month\n");
         printf ("             DD   = 2 digit day of the month\n");
         printf ("             HH   = 2 digit hour of the day (24 hour clock)\n");
         printf ("             SS   = 2 digit seconds\n");
         printf ("             -    = character to delimit date components\n");
         printf ("             T    = character to delimit date and time information\n");
         printf ("             :    = character to delimit time components\n");
         printf ("         \"MM/DD/YYYY HH:MM\"\n");
         printf ("             YYYY = 4 digit year\n");
         printf ("             MM   = 2 digit month\n");
         printf ("             DD   = 2 digit day of the month\n");
         printf ("             HH   = 1 or 2 digit hour of the day (24 hour clock)\n");
         printf ("             SS   = 2 digit seconds\n");
         printf ("             /    = character to delimit date components\n");
         printf ("             :    = character to delimit time components\n");
         printf ("         NOTE: The quotes are needed in this case to keep the day info\n");
         printf ("               together with the time info.  Without the quotes, the space\n");
         printf ("               would confuse the program into treating them as separate\n");
         printf ("               arguments, and not knowing what to do with the time info.\n");
         printf ("   Example:  -startTime \"10/20/2005 5:00\"\n");
         printf ("   Example:  -startTime 2005-10-20T05:00:00\n");
         printf ("   Example:  -startTime \"20051020 5:00\"\n");
         printf ("   Example:  -startTime \"2005-10-20 5:00\"\n");
         printf ("   Example:  -startTime \"October 20, 2005 5:00\"\n");
         printf ("   Example:  -startTime \"Oct 20, 2005 5:00\"\n");
         printf ("   -endTime [string]\n");
         printf ("      Establishes the ending time of the period for which you want data.\n");
         printf ("      The value is a UTC time and can be expressed in several ways.  If the\n");
         printf ("      end time is omitted, the DWML document will contain the latest data\n");
         printf ("      available.  See -startTime for how to define the time string\n");
         printf ("   Example:  -endTime 2006-12-25T23:00:00\n");
         break;
      case CMD_VERSION:
         break;
      case CMD_DATA:
         printf ("\nDATABASE OPTIONS (-Data)\n");
         printf ("Default: -Cube -msg 0 -Unit e -Decimal 3 -nMSB -revFlt\n");
         printf ("  -Index [file] = Index file to create or add data to.\n");
         printf ("  -Cube        = Store GRIB2 to a single '-out' file\n");
         printf ("  -nCube       = Store GRIB2 to a '-nameStyle' set of .flt"
                 " files\n");
         printf ("  -Append      = Append to data cube, instead of replacing"
                 " it.\n");
         printf ("  -msg [msgNum].[subgrdNum] = Which grib message to "
                 "convert.\n");
         printf ("               If msgNum = 0 or 'all', do all messages."
                 "\n");
         printf ("               The subgrdNum starts at 0, and is used for"
                 "\n");
         printf ("                    multiple grids in the same message."
                 "\n");
         printf ("  -radEarth [6371.2 km] = Override the radEarth in GRIB "
                 "message.\n");
         printf ("  -Unit [type] = Use [type] units. [type] can be:\n");
         printf ("               'none' (use GRIB2 units (K, kg/m**2 or m, "
                 "m/s)\n");
         printf ("               'e' 'english' (use F, inches, knots)\n");
         printf ("               'm' 'metric' (use C, kg/m**2 or m, m/s)\n");
         printf ("  -Decimal [amount] = How many decimals to round to "
                 "[0..18]\n");
         printf ("  -MSB or -nMSB = Generate binary files M.S.B. if "
                 "possible.\n");
         printf ("  -revFlt or -nRevFlt = Create (or don't) .tlf file\n");
         printf ("                (start in lower left not upper left)\n");
         break;
      case CMD_DATAPROBE:
         printf ("\nDATABASE PROBE OPTIONS (-DP)\n");
         printf ("Default: -pntStyle 0 -WxParse 0 -Separator \", \" "
                 "-Decimal 3\n");
         printf ("  -Print       = Print out the given index file.\n");
         printf ("  -Asc2Flx     = Convert Ascii .ini file to index "
                 "file.\n");
         printf ("  -pnt [lat,lon] = geographic point to probe.\n");
         printf ("  -pntFile [file] = file to find points to probe.\n");
         printf ("  -pntStyle [0,1] = Use point style where:\n");
         printf ("               0 => 'Elem,unit,refTime,valTime,loc1,loc2"
                 "..'\n");
         printf ("               1 =>'location,Elem[unit],refTime,"
                 "valTime,value'\n");
         printf ("  -WxParse [0,1,2] = Parse Weather?\n");
         printf ("               0 => print Weather ugly strings.\n");
         printf ("               1 => print 'English translation'\n");
         printf ("               2 => print -SimpleWx code\n");
         printf ("  -Separator [string] = String to use as a separator in "
                 "output\n");
         printf ("  -radEarth [6371.2 km] = Override the radius of Earth\n");
         printf ("  -Decimal [amount] = How many decimals to round to "
                 "[0..18]\n");
         printf ("  -XML [type] = Create XML output at the specified point\n");
         printf ("      [type] = 0 => Don't create XML (default)\n");
         printf ("               1 => create DWML time-series product\n");
         printf ("               2 => create DWML glance product\n");
/*
         printf ("               3 => create DWML summarized over 12-hour periods\n");
         printf ("               4 => create DWML summarized over 24-hour periods\n");
*/
         printf ("      See also -ndfdVars, -ndfdConven, -Icon, -startTime, and -endTime\n");
         printf ("  -ndfdVars [string]\n");
         printf ("      Specifies the NDFD elements (wind speed, sky cover, weather, etc.)\n");
         printf ("      that you want to appear in the output DWML.  The string is a comma\n");
         printf ("      delimited list of weather element codes.  The element codes\n");
         printf ("      depend on the which -ndfdConven is used.  Omitting -ndfdVars will\n");
         printf ("      yield DWML for all ndfd elements.\n");
         printf ("   Example:  -ndfdVars mx,wx,dp (assumes an -ndfdConven 2)\n");
         printf ("      See also -gribFilter -ndfdConven\n");
         printf ("   -ndfdConven [type]\n");
         printf ("      Defines the naming convention used for files containing NDFD GRIB\n");
         printf ("      messages.  These names are also used with the -ndfdVars option to\n");
         printf ("      tell degrib which NDFD elements to process.  Omitting -ndfdConven\n");
         printf ("      is equivalent to -ndfdConven 1.\n");
         printf ("      [type]=0   1     2\n");
         printf ("      maxt       maxt  mx 'Maximum Temperature'\n");
         printf ("      mint       mint  mn 'Minimum Temperature'\n");
         printf ("      pop12      pop12 po '12-Hour Probability of Precipitation'\n");
         printf ("      t          temp  tt 'Temperature'\n");
         printf ("      winddir    wdir  wd 'Wind Direction'\n");
         printf ("      windspd    wspd  ws 'Wind Speed'\n");
         printf ("      td         td    dp 'Dewpoint Temperature'\n");
         printf ("      sky        sky   cl 'Sky Cover'\n");
         printf ("      qpf        qpf   qp 'Quantitative Precipitation Forecast'\n");
         printf ("      snowamt    snow  sn 'Snowfall Amount'\n");
         printf ("      wx         wx    wx 'Weather'\n");
         printf ("      waveheight waveh wh 'Wave Height'\n");
         printf ("      apparentt  apt   at 'Apparent Temperature (wind chill or heat index)'\n");
         printf ("      rh         rhm   rh 'Relative Humidity'\n");
         printf ("      windgust   wgust wg 'Wind Speed Gust'\n");
         printf ("      probwindspd34i tcwspdabv34i i3 'Probabilistic Tropical Cyclone Surface Wind Speeds > 34Kts (incremental)'\n");
         printf ("      probwindspd50i tcwspdabv50i i5 'Probabilistic Tropical Cyclone Surface Wind Speeds > 50Kts (incremental)'\n");
         printf ("      probwindspd64i tcwspdabv64i i6 'Probabilistic Tropical Cyclone Surface Wind Speeds > 64Kts (incremental)'\n");
         printf ("      probwindspd34c tcwspdabv34c c3 'Probabilistic Tropical Cyclone Surface Wind Speeds > 34Kts (cumulative)'\n");
         printf ("      probwindspd50c tcwspdabv50c c5 'Probabilistic Tropical Cyclone Surface Wind Speeds > 50Kts (cumulative)'\n");
         printf ("      probwindspd64c tcwspdabv64c c6 'Probabilistic Tropical Cyclone Surface Wind Speeds > 64Kts (cumulative)'\n");
         printf ("      convoutlook conhazo ch 'Convective Hazard Outlook'\n");
         printf ("      tornadoprob ptornado pt 'Tornado Probability'\n");
         printf ("      hailprob   phail ph 'Hail Probability'\n");
         printf ("      windprob   ptstmwinds pw 'Damaging Thunderstorm Wind Probability'\n");
         printf ("      xtrmtornprob pxtornado xt 'Extreme Tornado Probability'\n");
         printf ("      xtrmhailprob pxhail xh 'Extreme Hail Probability'\n");
         printf ("      xtrmwindprob pxtstmwinds xw 'Extreme Thunderstorm Wind Probability'\n");
         printf ("      totalsvrprob ptotsvrtstm ps 'Total Probability of Severe Thunderstorms'\n");
         printf ("      totalxtrmprob ptotxsvrtstm xs 'Total Probability of Extreme Severe Thunderstorms'\n");
         printf ("   Example:  -ndfdConven 1\n");
         printf ("   -gribFilter [string]\n");
         printf ("      By default when expanding a directory to find GRIB files or database\n");
         printf ("      index files, it looks for (in the GRIB case *.bin) (in the database\n");
         printf ("      case *.ind).  In addition, if it doesn't find *.bin (example maxt.bin),\n");
         printf ("      then it also looks for ds.*.bin (example ds.maxt.bin).  The reason for\n");
         printf ("      this is because while tcldegrib and tkdegrib rename the file without\n");
         printf ("      the 'ds.', the filename on the NDFD http site has a 'ds.'.\n");
         printf ("         The -gribFilter option lets you over ride the '*.bin' to whatever\n");
         printf ("      convention you are using.\n");
         printf ("         If the -ndfdVars option is given, then it uses that to further qualify\n");
         printf ("      the file.\n");
         printf ("   Example: -ndfdVars maxt,mint -gribFilter \"*.grb\"\n");
         printf ("         Means look for files maxt.grb, mint.grb.  If you can't find them,\n");
         printf ("         also look for ds.maxt.grb, ds.mint.grb.\n");
         printf ("   Example:  -gribFilter \"*.grb\"\n");
         printf ("   -Icon [type]\n");
         printf ("      Indicates that you want the DWML to contain icon information. The\n");
         printf ("      default value is -Icon 0 (no icon information).\n");
         printf ("      [type] is defined as:\n");
         printf ("         0 = Degrib will not format icon XML elements in the DWML.\n");
         printf ("         1 = Degrib will format icon XML elements in the DWML.\n");
         printf ("   Example:  -Icon 1\n");
         printf ("      NOTE:  To format icon elements, degrib will need temperature,\n");
         printf ("             sky cover, wind speed, and weather files.\n");
         printf ("   -startTime [string]\n");
         printf ("      Establishes the beginning time of the period for which you want data.\n");
         printf ("      The value is a UTC time and can be expressed in several ways.  If the\n");
         printf ("      start time is omitted, the DWML document will contain the most recent\n");
         printf ("      data available.  Two of the ways you can define the time string are:\n");
         printf ("         YYYY-MM-DDTHH:MM:SS\n");
         printf ("             YYYY = 4 digit year\n");
         printf ("             MM   = 2 digit month\n");
         printf ("             DD   = 2 digit day of the month\n");
         printf ("             HH   = 2 digit hour of the day (24 hour clock)\n");
         printf ("             SS   = 2 digit seconds\n");
         printf ("             -    = character to delimit date components\n");
         printf ("             T    = character to delimit date and time information\n");
         printf ("             :    = character to delimit time components\n");
         printf ("         \"MM/DD/YYYY HH:MM\"\n");
         printf ("             YYYY = 4 digit year\n");
         printf ("             MM   = 2 digit month\n");
         printf ("             DD   = 2 digit day of the month\n");
         printf ("             HH   = 1 or 2 digit hour of the day (24 hour clock)\n");
         printf ("             SS   = 2 digit seconds\n");
         printf ("             /    = character to delimit date components\n");
         printf ("             :    = character to delimit time components\n");
         printf ("         NOTE: The quotes are needed in this case to keep the day info\n");
         printf ("               together with the time info.  Without the quotes, the space\n");
         printf ("               would confuse the program into treating them as separate\n");
         printf ("               arguments, and not knowing what to do with the time info.\n");
         printf ("   Example:  -startTime \"10/20/2005 5:00\"\n");
         printf ("   Example:  -startTime 2005-10-20T05:00:00\n");
         printf ("   Example:  -startTime \"20051020 5:00\"\n");
         printf ("   Example:  -startTime \"2005-10-20 5:00\"\n");
         printf ("   Example:  -startTime \"October 20, 2005 5:00\"\n");
         printf ("   Example:  -startTime \"Oct 20, 2005 5:00\"\n");
         printf ("   -endTime [string]\n");
         printf ("      Establishes the ending time of the period for which you want data.\n");
         printf ("      The value is a UTC time and can be expressed in several ways.  If the\n");
         printf ("      end time is omitted, the DWML document will contain the latest data\n");
         printf ("      available.  See -startTime for how to define the time string\n");
         printf ("   Example:  -endTime 2006-12-25T23:00:00\n");
         break;
      case CMD_REFTIME:
         printf ("\nREFERENCE TIME OPTIONS (-refTime)\n");
         printf ("Default: -tmFormat '%%D %%H:%%M'\n");
         printf ("  -tmFormat <format> = Return reftime using this time"
                 " format\n");
         break;
      case CMD_TOTAL:
         break;
      case CMD_SECTOR:
         printf ("\nSECTOR OPTIONS (-Sector)\n");
         printf ("  -sectFile [filename] = Contains the sectors.\n");
         printf ("  -pnt [lat,lon] = geographic point to probe.\n");
         printf ("  -cells all   = Print out the lat,lon,x,y,(in/out) for "
                 "each sector.\n");
         printf ("  -cells true  = same as -cells all, except lat,lon are "
                 "now Y,X\n");
         break;
      case -1:
/*      case CMD_CALC:*/
      case CMD_NCCONVERT:
      default:
         printf ("\nINPUT FILE OPTIONS\n");
         printf ("  -in [File]   = File to read GRIB2 message from.\n");
         printf ("  -cfg [File]  = File containing configuration Options."
                 "\n");
         printf ("\nOUTPUT FILE NAME OPTIONS\n");
         printf ("Default: -nameStyle 2\n");
         printf ("  -out [File]  = Name of the file to store results.\n");
         printf ("               Over-rides -nameStyle.\n");
         printf ("  -log [File]  = Log Weather key probe errors\n");
         printf ("  -nameStyle [string or style-Num] = convention to use"
                 " when creating files.\n");
         printf ("              %%e -> [element] (MaxT, MinT, etc)\n");
         printf ("              %%V -> [YYYYMMDDHHMM] (valid time)\n");
         printf ("              %%v -> [MMDDHHMM] (valid time)"
                 " short form shifted right.\n");
         printf ("              %%lv -> [YYYYMMDDHH] (valid time short form"
                 " shifted left.\n");
         printf ("              %%R -> [YYYYMMDDHH] (reference time)\n");
         printf ("              %%p -> [HH] (val time - ref time)\n");
         printf ("              %%c -> [HH] The valid cycle.\n");
         printf ("              %%D -> [DDDD or MDDD] where DDDD is the "
                 "difference in days (zulu)\n");
         printf ("                    between the reference and valid "
                 " day (the M means 'minus')\n");
         printf ("              %%d -> [D or MD] Same as %%D but no leading "
                 "0's\n");
         printf ("              %%y -> [###] ('020') integer part of lower"
                 " left latitude.\n");
         printf ("              %%x -> [###] ('121') abs (integer part of"
                 " lower left longitude).\n");
         printf ("              %%s -> [surface short form] (0[-]SFC)\n");
         printf ("              Predefined style numbers:\n");
         printf ("              0 = '%%e_%%V_%%y%%x.txt'\n");
         printf ("              1 = '%%e_%%R_%%p_%%y%%x.txt'\n");
         printf ("              2 = '%%e_%%v.txt'\n");
         printf ("              3 = '%%e_%%lv.txt'\n");
         printf ("              4 = '%%e_%%s_%%v.txt'\n");
         printf ("  -namePath [Path] = Path to store -nameStyle files in\n");
         printf ("\nCOMMAND OPTIONS\n");
         printf ("  -V           = Print version info.\n");
         printf ("  -I           = Perform an inventory on the input file"
                 "\n");
         printf ("  -C           = Convert a message from input file to "
                 "output file.\n");
         printf ("  -P           = Probe a GRIB file for all data related to"
                 " a point.\n");
         printf ("  -Data        = Create an index and a data cube from the "
                 "GRIB2 file\n");
         printf ("               = for faster 'Probes' in CGI programs.\n");
         printf ("  -DP          = Given an index file, probe a given point "
                 "(similar to -P)\n");
         printf ("  -refTime     = Return the oldest reference time in the"
                 " file.\n");
         printf ("  -Sector      = Return the sector a point is in.\n");
         printf ("  -StormTotal  = Return a storm total between a selected startTime and endTime\n");
   }
}

/*****************************************************************************
 * main() -- Review 12/2002
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *   The main entry point for the command line version of degrib.  Basically,
 * it parses input arguments, and then calls DegribIt.
 *
 * ARGUMENTS
 * argc = The number of arguments on the command line. (Input)
 * argv = The arguments on the command line. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = ok
 *  1 = error.
 *
 * HISTORY
 *   9/2002 Arthur Taylor (MDL/RSIS): Created.
 *  12/2002 (TK,AC,TB,&MS): Code Review.
 *
 * NOTES
 *****************************************************************************
 */
int main (int argc, char **argv)
{
   userType usr;        /* The user inputed options */
   char *msg;           /* Used to print the error stack. */
   int type;
   char perm;
   char *filter;
   int i, j;
   int ans;

   UserInit (&usr);
   if (argc < 2) {
      Usage (argv[0], &usr);
      printf ("\n");
      return 1;
   }

   /* Find out if first argument is a valid option.. */
   /* If yes then parse the command line.  If no then, ask if it is a file.
    * If it is not a file, then error, else use that file for -in option and
    * parse the rest of the command line. */
   if (!IsUserOpt (argv[1])) {
      type = myStat (argv[1], &perm, NULL, NULL);
      /* check if it is a file or directory */
      if ((type != MYSTAT_ISDIR) && (type != MYSTAT_ISFILE)) {
         Usage (argv[0], &usr);
         printf ("\nError was: '%s' is nOt a file or directory\n\n", argv[1]);
         return 1;
      }
      /* check that it is readable */
      if (!(perm & 4)) {
         Usage (argv[0], &usr);
         printf ("\nError was: No read permissions on '%s'\n\n", argv[1]);
         return 1;
      }
      usr.numInNames = 1;
      usr.inNames = malloc (usr.numInNames * sizeof (char *));
      usr.inNames[0] = (char *) malloc ((strlen (argv[1]) + 1) *
                                        sizeof (char));
      strcpy (usr.inNames[0], argv[1]);
      usr.f_inTypes = malloc (usr.numInNames * sizeof (char));
      usr.f_inTypes[0] = type;

      for (i = 2; i < argc; i++) {
         if (!IsUserOpt (argv[i])) {
            type = myStat (argv[i], &perm, NULL, NULL);
            /* check if it is a file or directory */
            if ((type != MYSTAT_ISDIR) && (type != MYSTAT_ISFILE)) {
               Usage (argv[0], &usr);
               printf ("\nError was: '%s' is noT a file or directory\n\n",
                       argv[i]);
               for (j = 0; j < usr.numInNames; j++) {
                  free (usr.inNames[j]);
               }
               free (usr.inNames);
               free (usr.f_inTypes);
               return 1;
            }
            /* check that it is readable */
            if (!(perm & 4)) {
               Usage (argv[0], &usr);
               printf ("\nError was: No read permissions on '%s'\n\n",
                       argv[i]);
               for (j = 0; j < usr.numInNames; j++) {
                  free (usr.inNames[j]);
               }
               free (usr.inNames);
               free (usr.f_inTypes);
               return 1;
            }
            usr.numInNames++;
            usr.inNames = realloc (usr.inNames,
                                   usr.numInNames * sizeof (char *));
            usr.inNames[usr.numInNames - 1] =
                  malloc ((strlen (argv[i]) + 1) * sizeof (char));
            strcpy (usr.inNames[usr.numInNames - 1], argv[i]);
            usr.f_inTypes = realloc (usr.f_inTypes,
                                     usr.numInNames * sizeof (char));
            usr.f_inTypes[usr.numInNames - 1] = type;
         } else {
            break;
         }
      }
      if (i == argc) {
         Usage (argv[0], &usr);
         printf ("\nError was: Couldn't find a command line option\n\n");
         for (j = 0; j < usr.numInNames; j++) {
            free (usr.inNames[j]);
         }
         free (usr.inNames);
         free (usr.f_inTypes);
         return 1;
      }

      if (UserParseCommandLine (&usr, argc - i, argv + i) != 0) {
         Usage (argv[0], &usr);
         msg = errSprintf (NULL);
         printf ("\nError was: %s", msg);
         free (msg);
         UserFree (&usr);
         return 1;
      }
   } else {
      if (UserParseCommandLine (&usr, argc - 1, argv + 1) != 0) {
         Usage (argv[0], &usr);
         msg = errSprintf (NULL);
         printf ("\nError was: %s", msg);
         free (msg);
         UserFree (&usr);
         return 1;
      }
   }
   /* Load options from file. */
   if (usr.cfgName != NULL) {
      if (UserParseConfigFile (&usr, usr.cfgName) != 0) {
         Usage (argv[0], &usr);
         msg = errSprintf (NULL);
         printf ("\nError was: %s", msg);
         free (msg);
         UserFree (&usr);
         return 1;
      }
   }
   /* Make sure that they have a command. */
   if (usr.f_Command == -1) {
      Usage (argv[0], &usr);
      printf ("\nPlease provide a command option.\n");
      UserFree (&usr);
      return 1;
   }

   /* Validate choices and set up defaults. */
   if (UserValidate (&usr) != 0) {
      Usage (argv[0], &usr);
      msg = errSprintf (NULL);
      printf ("\nError was: %s", msg);
      free (msg);
      UserFree (&usr);
      return 1;
   }

   /* Convert usr.inNames from a list of dir's and file's to just files for
    * everything except -P and -DP.  -P or -DP try to "tack on" the sector
    * for their points and filter the files based on ndfdVars when
    * converting_ to just files. */
   if ((usr.f_Command != CMD_PROBE) && (usr.f_Command != CMD_DATAPROBE)) {
      if (usr.gribFilter != NULL) {
         filter = usr.gribFilter;
      } else {
         if ((usr.f_Command == CMD_DATACONVERT) ||
             (usr.f_Command == CMD_DATA)) {
            filter = "*.ind";
         } else {
            filter = "*.bin";
         }
      }
      if (expand_inName (&(usr.numInNames), &(usr.inNames), &(usr.f_inTypes),
                         filter) != 0) {
         Usage (argv[0], &usr);
         msg = errSprintf (NULL);
         printf ("\nError was: %s", msg);
         free (msg);
         UserFree (&usr);
         return 1;
      }
   }

   if (usr.f_Command == CMD_VERSION) {
/*      msg = Grib2About (argv[0]);*/
      msg = Grib2About ("degrib");
      printf ("%s", msg);
      free (msg);
      UserFree (&usr);
      return 0;
   }

   /* Do it. */
   ans = DegribIt (&usr);

   UserFree (&usr);
   return ans;
}
