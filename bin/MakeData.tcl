#!/bin/sh
# the next line restarts \
exec ./tcldegrib "$0" "$@"

set src_dir [file dirname [info script]]
if {[file pathtype $src_dir] != "absolute"} {
  set cur_dir [pwd] ;  cd $src_dir
  set src_dir [pwd] ;  cd $cur_dir
}
set root [file dirname $src_dir]
set Exec $root/bin/degrib
if {($argc != 1) && ($argc != 2)} {
  puts "Usage: $argv0 <sourceDir> <outDir (optional)>"
  exit
}
if {$argc == 1} {
  set GRIB_Dir [lindex $argv 0]
  set DstDir [lindex $argv 0]
} elseif {$argc == 2} {
  set GRIB_Dir [lindex $argv 0]
  set DstDir [lindex $argv 1]
}
if {! [file exists $GRIB_Dir]} {
  puts "Couldn't find '$GRIB_Dir"
  exit
}

# set root /www/ndfd/public
# set Exec $root/bin/degrib.cur
# set GRIB_Dir $root/database/grib
# set DstDir $root/database/cube

#*****************************************************************************
# makedata.tcl
#
# DESCRIPTION
#   This contains an example of how to automatically convert the NDFD GRIB2
# data to Data Cubes, so one can use -DP to probe it (faster).
#
# HISTORY
#   9/2003 Arthur Taylor (MDL / RSIS): Created.
#
# NOTES
#*****************************************************************************

if {! [file isdirectory $DstDir]} {
  file mkdir $DstDir
}
cd $DstDir
foreach file [glob -nocomplain $GRIB_Dir/*.bin] {
  if {[file extension $file] == ".bin"} {
    set tail [split [file tail $file] .]
    if {[llength $tail] > 2} {
       set tail [join [lrange $tail [expr [llength $tail] - 2] end] .]
    } else {
       set tail [file tail $file]
    }
    set dst [file join $DstDir $tail]
    catch {exec $Exec $file -Data -Index [file rootname $dst].ind -out [file rootname $dst].dat}
    puts "Done with $file -Data -Index [file rootname $dst].ind -out [file rootname $dst].dat"
  }
}
