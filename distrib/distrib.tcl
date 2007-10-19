#!/usr/bin/tclsh

set src_dir [file dirname [info script]]
if {[file pathtype $src_dir] != "absolute"} {
  set cur_dir [pwd]
  cd $src_dir
  set src_dir [pwd]
  cd $cur_dir
}


proc clean {filename outfile {trim 0} {mode unix} } {

  set fp [open $filename r]
  set lst ""
  while {[gets $fp line] >= 0} {
    if {$trim == 1} {
      set line [string trimright $line]
    }
    lappend lst $line
  }
  close $fp
# Get rid of trailing lines...
  while {([llength $lst] > 0) && ([lindex $lst end] == "")} {
    set lst [lreplace $lst end end]
  }
  set fp [open $outfile w]
  if {$mode == "unix"} {
    fconfigure $fp -translation lf
  } elseif {$mode == "dos"} {
    fconfigure $fp -translation crlf
  }
# make so we don't have a cr at end of file...
#  if {$trim == 1} {
#    set last_line [lindex $lst end]
#    set lst [lreplace $lst end end]
#  }
  foreach line $lst {
    puts $fp $line
  }
#  if {$trim == 1} {
#    puts -nonewline $fp $last_line
#  }
  close $fp
}

#-----------------------------------------------------------------------------
# ReadIni --
#
#   Reads data from a .ini file.  Returns "" or [list [list variable value]]
#
# Arguments:
#   file     : The name of the .ini file.
#   section  : The [section] of the .ini file of interest.
#   lstVars  : List of variables to read.
#
# 1/2002 Arthur Taylor (RSIS/MDL) Updated.
#-----------------------------------------------------------------------------
proc ReadIni {file section lstVars} {
  if {! [file isfile $file]} {
#    tk_messageBox -message "Unable to open $file"
    puts "Unable to open $file"
    return ""
  }
  set fp [open $file r]
#####
# Find the "section"
#####
  set f_found 0
  while {[gets $fp line] >= 0} {
    if {$line == "\[$section\]"} {
      set f_found 1
      break
    }
  }
  if {! $f_found} {
    close $fp
#    tk_messageBox -message "Unable to find \[$section\] section in $file"
    puts "Unable to find \[$section\] section in $file"
    return ""
  }
#####
# Read through the "section"
#####
  set ans ""
  while {[gets $fp line] >= 0} {
    set line [string trim $line]
    set lineLen [string length $line]
    if {([string index $line 0] == "\[") && \
        ([string index $line [expr {$lineLen -1}]] == "\]")} {
      break
    }
    if {$lineLen != 0} {
      set pair [split $line =]
      if {[lsearch $lstVars [lindex $pair 0]] != -1} {
        lappend ans $pair
      }
    }
  }
  close $fp
  return $ans
}

#-----------------------------------------------------------------------------
# WriteIni --
#
#   Reads in an old .ini file, stores it in a Local array, modifies or
# replaces a section, then saves it back to disk.
#
# Arguments:
#   file     : The name of the .ini file.
#   Section  : The [section] of the .ini file of interest.
#   lstVars  : [list [list variable value]] to set.
#
# 1/2002 Arthur Taylor (RSIS/MDL) Updated.
#
# Notes:
#   In a .ini file, we might want order to matter, so we shouldn't use an
# array names, hence the reason for $Section,List
#-----------------------------------------------------------------------------
proc WriteIni {file Section lstVars} {
#####
# Read in old .ini file if it exists.
#####
  set SectList ""
  if {[file isfile $file]} {
    set fp [open $file r]
    set section NULL
    while {[gets $fp line] >= 0} {
      set line [string trim $line]
      set lineLen [string length $line]
      if {([string index $line 0] == "\[") && \
          ([string index $line [expr {$lineLen -1}]] == "\]")} {
        set section [string range $line 1 [expr {$lineLen -2}]]
        lappend SectList $section
        set Local($section,List) ""
      } elseif {$lineLen != 0} {
        set pair [split $line =]
        set Local($section,[lindex $pair 0]) [lindex $pair 1]
        lappend Local($section,List) [lindex $pair 0]
      }
    }
    close $fp
  }
#####
# Add or overwrite new data to array
#   First initializing Section if it is new.
#####
  if {[lsearch $SectList $Section] == -1} {
    lappend SectList $Section
    set Local($Section,List) ""
  }
  foreach pair $lstVars {
    set var [lindex $pair 0]
    set Local($Section,$var) [lindex $pair 1]
    if {[lsearch $Local($Section,List) $var] == -1} {
      lappend Local($Section,List) $var
    }
  }
#####
# Write the data to disk.
#####
  set fp [open $file w]
  foreach section $SectList {
    puts $fp "\[$section\]"
    foreach var $Local($section,List) {
      if {$Local($section,$var) != ""} {
        puts $fp "$var=$Local($section,$var)"
      } else {
        puts $fp "$var"
      }
    }
    puts $fp ""
  }
  close $fp
  return
}

proc UpdateHeader {file version date} {
  set fp [open $file r]
  set op [open [file rootname $file].hpp w]
  fconfigure $op -translation lf
  set cnt 0
  while {[gets $fp line] >= 0} {
    if {$cnt >= 2} {
      if {$cnt == 2} {
        puts $op "\#define PROGRAM_VERSION \"$version\""
        puts $op "\#define PROGRAM_DATE \"$date\""
      }
      puts $op $line
    }
    incr cnt
  }
  close $fp
  close $op
  file copy -force [file rootname $file].hpp $file
  file delete -force [file rootname $file].hpp
}

proc UpdateConfigure {file name version author date} {
  set fp [open $file r]
  set op [open [file rootname $file].hpp w]
  fconfigure $op -translation lf
  while {[gets $fp line] >= 0} {
    set line2 [string trim $line]
    if {[string range $line2 0 6] == "AC_INIT"} {
      puts $op "AC_INIT(\[$name\],\[$version\],\[$author\])"
    } elseif {[string range $line2 0 20] == "AC_SUBST(PACKAGE_DATE"} {
      puts $op "AC_SUBST(PACKAGE_DATE,'$date')"
    } else {
      puts $op $line
    }
  }
  close $fp
  close $op
  file copy -force [file rootname $file].hpp $file
  file delete -force [file rootname $file].hpp
}

proc UpdatePane {file} {
   set fp [open $file r]
   set lst ""
   gets $fp line  ;# Skip old setting of set PANE.
   while {[gets $fp line] >=0 } {
      lappend lst $line
   }
   close $fp
   set fp [open $file w]
   puts $fp "set FUN 0"
   foreach elem $lst {
      puts $fp $elem
   }
   close $fp
}

if {($argc != 2)} {
  puts "Usage: $argv0 <version (see version.txt)> <version date>"
  exit
}

#####
# 1) Update or verify version.txt
#####
set lst ""
foreach elem [ReadIni [file join [file dirname $src_dir] version.txt] \
                       degrib [list Version Date Web]] {
  set var [lindex $elem 0]
  set val [lindex $elem 1]
  set ray($var) $val
  lappend lst $var
}
if {$ray(Version) < [lindex $argv 0]} {
  set ray(Version) [lindex $argv 0]
  # Make sure that next time we compile the version is updated.
  file delete [file join [file dirname $src_dir] src degrib userparse.o]
} elseif {$ray(Version) == [lindex $argv 0]} {
} else {
  puts "release $ray(Version) is > [lindex $argv 0]"
  puts "Resetting release version"
  set ray(Version) [lindex $argv 0]
}
set date [clock format [clock scan [lindex $argv 1]] -format "%m/%d/%Y"]
set date2 [clock format [clock scan [lindex $argv 1]] -format "%Y%m%d"]
# set date [clock format [clock seconds] -format "%m/%d/%Y"]
set ray(Date) $date
set newLst ""
foreach elem $lst {
  lappend newLst [list $elem $ray($elem)]
}
WriteIni [file join [file dirname $src_dir] version.txt] degrib $newLst

#####
# 2) Update the version in the C-Code.
#####
UpdateHeader [file join [file dirname $src_dir] src degrib userparse.h] \
       $ray(Version) $ray(Date)

#####
# 2b) Update the version in configure.ac
#####
UpdateConfigure [file join [file dirname $src_dir] src configure.ac] \
       degrib $ray(Version) arthur.taylor@noaa.gov $date2
#cd [file join [file dirname $src_dir] src]
#exec /usr/bin/autoconf
#cd $src_dir

#####
# 3) Make sure "Fun" is turned off in the tcl code.
#####
UpdatePane [file join [file dirname $src_dir] bin tclsrc pane.tcl]

#####
# 4) update history.txt
#####
set fp [open [file join [file dirname $src_dir] history.txt] r]
set txtLst ""
while {[gets $fp line] >= 0} {
  lappend txtLst [string trimright $line]
}
close $fp
set txt [join $txtLst "\n"]
set MatchLine "#\n#----- Release: Version $ray(Version)"
while {[string first $MatchLine $txt] == 0} {
  set txtLst [lrange $txtLst 3 end]
  set txt [join $txtLst "\n"]
}
if {[string first $MatchLine $txt] != -1} {
  puts "This version number was embeded in history.txt?"
  exit
}
set txtLst [linsert $txtLst 0 "#" \
                    "#----- Release: Version $ray(Version) $date -----" "#"]
set fp [open [file join [file dirname $src_dir] history.txt] w]
foreach line $txtLst {
  puts $fp [string trimright $line]
}
close $fp

#####
# 5) Clean up some files.
#####
set file [file join [file dirname $src_dir] bin ndfd.ini]
clean $file $file 0 unix
set file [file join [file dirname $src_dir] bin ndfd.tcl]
clean $file $file 0 unix
foreach file [glob -nocomplain [file join [file dirname $src_dir] bin tclsrc *.tcl]] {
  clean $file $file 0 unix
}

#####
# 6) Create the packages.
#####
puts [exec bash pack.tcl no-tests]

#####
# 7) Update some of the pages on the web site.
#####
set webDir [file dirname [file dirname $src_dir]]/degrib.web/degrib2/
# set webDir e:/www/htdocs/degrib2/
set date [lindex $argv 1]
set date2 [clock format [clock scan $date] -format "%Y%m%d"]
puts $date2
puts $date
exec cp [file join $webDir download degrib-all.tar.gz] \
        [file join $webDir download archive degrib-$date2.tar.gz]
exec cp [file join [file dirname $src_dir] history.txt] \
        [file join $webDir history.txt]
exec cp [file join [file dirname $src_dir] docs degrib.txt] \
        [file join $webDir degrib.txt]
exec cp [file join [file dirname $src_dir] docs tkdegrib.txt] \
        [file join $webDir tkdegrib.txt]
file copy -force [file join [file dirname $src_dir] version.txt] [file join $webDir download version.txt]

puts "----------"
puts "You may want to make the program and rerun distrib.tcl"
puts "Since userparse.h probably got updated."
puts "This may mean: first 'rm userparse.o'"
puts "----------"
puts "Also recommend running autoconf since configure.ac was updated"
puts "----------"
puts "Check ./degrib/checklst.txt for other suggestions"
puts "----------"
puts "use rainbow to check compiling on hp."
puts "use tyr to check compiling on linux."
puts "use frost to check compiling on AIX"
puts "----------"
puts "Recommend in degrib/src:"
puts "   A) svn update"
puts "   B) autoconf.sh"
puts "   C) config-win.sh"
puts "   D) make"
puts "Then rerun the distrib.tcl again"
