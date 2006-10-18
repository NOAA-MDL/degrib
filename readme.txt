Welcome to the NDFD Degrib program.

The main web site for this project is currently at:
http://www.nws.noaa.gov/mdl/NDFD_GRIB2Decoder/

----- Intro: -----

The NDFD (National Digital Forecast Database) will shortly be putting the
national digital forecasts for the conus (continuous U.S.) on an anonymous
ftp site.  Those files will be in GRIB2 (Gridded Binary Edition 2) format,
which is outlined in the WMO (World Meteorological Organization) document:
The WMO GRIB2 Document FM92-X11 GRIB

In order to take advantage of this data, MDL (Meteorological Development
Laboratory) created and maintains the official GRIB2 Decoder library. The GRIB2
Decoder library does a very good job of unpacking GRIB2 messages, but in order
to understand how to use it, and what is returned, one needs a working
knowledge of "The WMO GRIB2 Document FM92-X11 GRIB".

MDL felt that in order to make it easier for people to use NDFD data, a
"driver" for the library should be provided. The main purposes for the
driver are:

  1.Provide an example of how to call the Decoder library.
  2.Provide software to convert from GRIB2 to a simpler file format.
  3.Provide a means to view the "meta" data without needing to reference
    "The WMO GRIB2 Document FM92-X11 GRIB" document.

Thus the creation of the degrib (and tkdegrib) programs, which convert a grid
in a GRIB2 message to a ".flt" file (ESRI Spatial Analyst file), or a .shp
file (ESRI ArcView file).

----- Install: -----

Please see:
http://www.nws.noaa.gov/mdl/NDFD_GRIB2Decoder/howto.htm

----- Directory structure: -----

./bin = Contains the programs (degrib, tkdegrib).
./docs = Contains project documentation.
./lib = Contains extra libraries needed by tkdegrib.
./output = Is the default output directory for tkdegrib.
./tclsrc = Contains extra scripts neede by tkdegrib.

./src = Contains the source code for the following:
  ./src/grib2unpacker = Contains the current distribution of grib2unpacker
        Created by: Dr. Bob Glahn (MDL/OST/NWS/NOAA))
  ./src/cmapf-c = Contains the current distribution of cmapf
        Created by: Dr. Albion Taylor (Air Resources Lab/NOAA))
  ./src/degrib = Contains the source code for degrib and tkdegrib
        which uses the libraries: cmapf and grib2unpacker.

The following are for testing, and are part of the test "suites".
./ans = Contains sample answers
./data = Contains sample grib2 files
./test = Contains test scripts, which should generate in ./test the files
       that are in ./ans.

----- Where to get more info: -----

A history list of changes and the current version information are here:
./history.txt
./version.txt

The following are the "man" pages for the two programs:
./docs/degrib.txt
./docs/tkdegrib.txt

There are several other files in the docs directory which give information
about the GRIB2 file format, as well as the ArcView Shape file formats.  See
./docs/readme.txt

