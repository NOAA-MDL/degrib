#!/bin/sh
# The next line restarts with wish \
if test -x /usr/bin/tclsh
# \
then
  # \
  exec tclsh "$0" "$@"
# \
else
  # \
  exec cygtclsh "d:/cygwin$0" "$@"
# \
fi

set src_dir [file dirname [info script]]
if {[file pathtype $src_dir] != "absolute"} {
  set cur_dir [pwd]
  cd $src_dir
  set src_dir [pwd]
  cd $cur_dir
}

#
# This script packs up and distributes the tests.
#

# Get rid of the \n\r in text files that might be there from the PC.
#puts [exec bash ../ans/clean.tcl ../ans/ .txt 0 unix]
#puts [exec bash ../ans/clean.tcl ../ans/ .prj 0 unix]
#puts [exec bash ../ans/clean.tcl ../ans/ .hdr 0 unix]
#puts [exec bash ../ans/clean.tcl ../ans/ .ave 0 unix]

puts "!! Caution !!"
puts "You should first source e:/prj/ndfd/degrib/ans/clean.tcl from windows Tcl."
puts "!! Caution !!"

puts [exec bash pack.tcl testtmax.txt]
puts [exec bash pack.tcl testwnds.txt]
puts [exec bash pack.tcl testwndd.txt]
puts [exec bash pack.tcl testsky.txt]
puts [exec bash pack.tcl testt.txt]
puts [exec bash pack.tcl testtd.txt]
puts [exec bash pack.tcl testpop.txt]
puts [exec bash pack.tcl testtmin.txt]
puts [exec bash pack.tcl testnewtmax.txt]

