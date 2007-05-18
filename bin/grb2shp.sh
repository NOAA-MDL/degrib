#!/bin/sh
# shell script to create shapefiles from GRIB2 data files
# using the degrib utility John M. Huddleston 20061201
# updated to use the 'all' option and only points 20070202
#
help ()
{
   echo "This script uses the degrib application to create shapefiles"
   echo ""
   echo "For references to the code and products, see the following"
   echo "http://www.nws.noaa.gov/mdl/NDFD_GRIB2Decoder/"
   echo "http://www.nco.ncep.noaa.gov/pmb/products/"
   echo "http://www.nco.ncep.noaa.gov/pmb/codes/"
   echo "http://www.cpc.ncep.noaa.gov/products/wesley/fast_downloading_grib.html"
}
usage ()
{
   echo "Usage: $0 \"grib file(s)\""
}
if [ $# -lt 1 ]; then
   usage
   exit
fi
if [ "$1" = 'h' -o "$1" = 'H' -o "$1" = "-h" ]; then
   help
   exit
fi
for i in $*
do
   if [ -f "$i" ]; then
      degrib "$i" -C -msg all -Shp -poly 0 -nameStyle "%e_%v_%s.txt"
   fi
done
