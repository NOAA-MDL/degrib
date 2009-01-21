#!/usr/bin/tclsh
#!/bin/tclsh
#!/mdl2/save/bin/tclsh

# tell the usage of this program
if {$argc != 3} {
  puts "usage: $argv0 <infile> <station.tbl> <outfile>"
  exit
}

set inFile [lindex $argv 0]
set stnFile [lindex $argv 1]
set outFile [lindex $argv 2]

proc readStnFile {rayName stnFile} {
  upvar #0 $rayName ray
  set fp [open $stnFile r]
  while {[gets $fp line] >= 0} {
    set line [split $line :]
    set stn [string trim [lindex $line 0]]
    set ray($stn,lat) [string range [string trim [lindex $line 6]] 1 end]
    set ray($stn,lon) [expr -1 * [string range [string trim [lindex $line 7]] 1 end]]
  }
  close $fp
}

readStnFile ray $stnFile
set fp [open $inFile r]
set op [open $outFile w]
gets $fp line
puts $op "LAT, LON, RGN-C, WFO-C, STN-C, TEMP-N, 0.25-N, 0.75-N, 50%-N"
while {[gets $fp line] >= 0} {
  set stn [lindex $line 2]
  if {[info exists ray($stn,lat)]} {
    puts $op "$ray($stn,lat), $ray($stn,lon), [lindex $line 0],\
              [lindex $line 1], [lindex $line 2], [lindex $line 3],\
              [lindex $line 4], [lindex $line 5], [lindex $line 6]"
  } else {
    puts "Couldn't find $stn"
  }
}
close $fp
close $op
