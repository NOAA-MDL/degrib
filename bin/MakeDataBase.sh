#!/bin/csh

#
# DESCRIPTION
#   This shell script is to convert all the GRIB2 data files
#   to Data Cubes with the same name
#
# HISTORY
#   9/2003 Arthur Taylor (MDL / RSIS) Created
#
# NOTES
#

set root=e:/prj/ndfd/degrib

set Exec=$root/bin/degrib
set ConusDir=$root/data/grib2/conus
set DstDir=$root/database

if (! -d $DstDir) then
  mkdir $DstDir
endif 
cd $DstDir
foreach file (`ls` $ConusDir/*.bin)
  if ($file:e == "bin") then
    set dst=$DstDir/$file:t
    $Exec $file -Data -Index $dst:r.ind -out $dst:r.dat
    echo "Done with $file -Data -Index $dst:r.ind -out $dst:r.dat"
  endif
end
