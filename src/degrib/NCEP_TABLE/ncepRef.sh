#!/bin/sh

# 
# Use this to check if we are up to date with NCEP local tables
#

list="table4-2-0-0 table4-2-0-1 table4-2-0-2 table4-2-0-3 table4-2-0-4 table4-2-0-5 table4-2-0-6 table4-2-0-7 table4-2-0-13 table4-2-0-14 table4-2-0-15 table4-2-0-16 table4-2-0-17 table4-2-0-18 table4-2-0-19 table4-2-0-20 table4-2-0-190 table4-2-0-191 table4-2-0-192 table4-2-1-0 table4-2-1-1 table4-2-2-0 table4-2-2-3 table4-2-3-0 table4-2-3-1 table4-2-3-192 table4-2-10-0 table4-2-10-1 table4-2-10-2 table4-2-10-3 table4-2-10-4 table4-2-10-191" 

for file in $list
do
../../../bin/getUrl.tcl http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_${file}.shtml grib2_${file}.shtml
nocr.exp grib2_${file}.shtml
done
