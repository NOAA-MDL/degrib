#!/bin/sh

# 
# Use this to check if we are up to date with NCEP local tables
#

list="table2"

for file in $list
do
../../../bin/getUrl.tcl http://www.nco.ncep.noaa.gov/pmb/docs/on388/${file}.html ${file}.html
nocr.exp ${file}.html
done
