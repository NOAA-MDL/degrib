This directory contains several documents that were thought to be of use to
someone who was interested in working with the National Digital Forecast
Database.  Since the NDFD is given out through GRIB2 we have also put that
documentation here.

----------------------------------
Man Pages (Description of how to use the programs located in ../bin)
----------------------------------

degrib.txt:  Discuses how to call the degrib program and what it is used for
             Basically it is a command line based driver to unpack GRIB2
             messages.

tkdegrib.txt: Discuses how to call the tkdegrib program and what it is for.
             Basically it is a Tcl/Tk wrapper for the GRIB2 library and driver
             used in degrib.

----------------------------------
File format specifications:
----------------------------------

./specs/FM92-GRIB-Edition2.pdf: WMO specifications of a generic GRIB2 message
             and what is expected in each section.

./specs/shapefile.pdf: ESRI ArcView Shapefile specification.  (Since the
             drivers can create shapefiles from the GRIB2 messages, it was
             felt that we should provide information about shapefiles.)

----------------------------------
NDFD use of GRIB2:
----------------------------------
GRIB2_encoding.html: Describes how NDFD uses the GRIB2 specifications.
      (Downloaded on 2/4/2003)
      (see http://www.nws.noaa.gov/datamgmt/doc/GRIB2_encoding.html
              for the most recent copy)

NDFD_GRIB2Decoder.html: Is a copy of the main home page for this project.
             Specifically it describes what GRIB2 is, where the GRIB2 library
             is, and why a driver was created.
      (Downloaded on 2/4/2003)
      (see http://www.nws.noaa.gov/mdl/NDFD_GRIB2Decoder/index.htm
              for the most recent copy)

----------------------------------
Papers:
----------------------------------
On the NDFD Demo CD there is:
./papers/newDB_color.pdf The New Digital Forecast Database at the National
             Weather Service, by Harry R. Glahn and David P. Ruth. (2003)
             (Actual article in color)

----------------------------------
In the install dir under ./degrib/docs/ there is:

./papers/newDB_bw.pdf: The New Digital Forecast Database at the National
             Weather Service, by Harry R. Glahn and David P. Ruth. (2003)
             (Draft in B/W)

./papers/CreateNDFD.pdf: Creating a National Digital Forecast Database of
             Official National Weather Service forecasts, by David P. Ruth and
             Harry R. Glahn. (2003)

./papers/NDFDdesign.pdf: National Digital Forecast Database Design and
             Development, Timothy R. Boyer and David P. Ruth (2003)

./papers/IFPFuture.pdf: Interactive Forecast Perparation - The Future Has Come,
             David P. Ruth (2002)

./papers/GRIB.427.wpd: A Discussion of GRIB2 and the capabilities of MDL
             Software, by Harry R. Glahn and Bryon Lawrence.

----------------------------------
Source code help:
----------------------------------
../src/grib2unpacker/unpkgrib.315.wpd: How to use the FORTRAN Unpacking
             library routine.

../src/cmapf-c/readme.txt: A Discussion of how to call the cmapf library
             routines.

../src/degrib/readme.txt: A Discussion of the file structure for the
             "driver" part of the project.
