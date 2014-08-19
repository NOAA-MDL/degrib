#!/bin/sh
# The next line restarts with wish \
exec tclsh "$0" "$@"

set src_dir [file dirname [info script]]
if {[file pathtype $src_dir] != "absolute"} {
  set cur_dir [pwd]
  cd $src_dir
  set src_dir [pwd]
  cd $cur_dir
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

proc CreateLst {dir filter f_recurse} {
  set dir [join [file split $dir] /]
  set lst ""
  foreach file [glob -nocomplain [file join $dir *]] {
    if [file isfile $file] {
      if {[string match $filter [file tail $file]]} {
        set file [file split $file]
        set file [join $file /]
        lappend lst $file
      }
    } elseif {$f_recurse} {
      set ans [CreateLst $file $filter $f_recurse]
      set lst [concat $lst $ans]
    }
  }
  return $lst
}

proc DoUpdatePage {filename href size date version} {
  set fp [open $filename r]
  set lines ""
  while {[gets $fp line] >= 0} {
    lappend lines $line
  }
  close $fp
  if {$version == ""} {
    set ans "([format "%.2f" [expr $size / 1000000.]]\
              MB : [clock format $date -format "%m/%d/%Y"])"
  } else {
    set ans "(Version $version : [format "%.2f" [expr $size / 1000000.]]\
              MB : [clock format $date -format "%m/%d/%Y"])"
  }
  set text [join $lines "\n"]
  set index [string first "$href" $text 0]
  while {$index != -1} {
    set nindex [string first "(" $text $index]
    if {[expr $nindex - $index] < 100} {
      set index $nindex
      if {$index != -1} {
        set index2 [string first ")" $text $index]
        if {$index2 != -1} {
          puts "Replacing [string range $text $index $index2] with $ans"
          set text [string replace $text $index $index2 $ans]
        } else {
          set text [string replace $text $index end $ans]
        }
        set index [string first "$href" $text $index]
      }
    } else {
      set index [string first "$href" $text [expr $index +1]]
    }
  }
  set lines [split $text "\n"]
  set fp [open $filename w]
  foreach line $lines {
    puts $fp $line
  }
  close $fp
}

proc FindVersion {VersionDir} {
  set file [file join $VersionDir version.txt]
  set ans [ReadIni $file degrib [list Version]]
  if {$ans == ""} {
    puts "Couldn't FindVersion $VersionDir"
    return 0.0
  }
  set elem [lindex $ans 0]
  if {[llength $elem] != 2} {
    puts "Couldn't FindVersion $VersionDir"
    return 0.0
  }
  return [lindex $elem 1]
}

proc DoIt {filename} {
  global src_dir

  set ansFile degrib-src
  set Zip_Also false

  set StartDir [file dirname [file dirname $src_dir]]
#  set DestDir $StartDir/degrib/distrib/webdir/download
  set DestDir $src_dir/webdir/download
  set UpdatePages ""         ;# intended to be a list.
  set Version [FindVersion $StartDir/degrib]
  set DoGeneric true
  set DoExe true
  set DoSrc true

  set fp [open $filename r]
  puts $filename
  set FilterList ""
  while {[gets $fp line] >= 0} {
    set line [string trim $line]
    if {[string index $line 0] != "#"} {
      set spt [split $line =]
   #
   # Check if there is an '=' in the line...
   #
      if {[llength $spt] != 1} {
        set name [lindex $spt 0]
        set cmd [lindex $spt 1]
        if {[string index [string trim $cmd] 0] == "\["} {
          set $name [eval [string range $cmd 1 [expr [string length $cmd] -2]]]
        } else {
          set $name $cmd
        }
      } else {
        lappend FilterList $line
      }
    }
  }
  close $fp

  cd $StartDir
  set lst ""
  foreach elem $FilterList {
    set use 0
    set temp [split $elem :]
    if {[llength $temp] != 1} {
      set classList [split [lindex $temp 0] ,]
      foreach elem $classList {
        if {[info exists Class_$elem]} {
          if {[set Do[set Class_$elem]]} {
            set use 1
          }
        }
      }
      if {$use} {
        set elem [lindex $temp 1]
      }
    } else {
      set use 1
    }
    if {$use} {
      if {[llength $elem] != 1} {
        if {[lindex $elem 1] == "recurse"} {
          set elem [lindex $elem 0]
          set lst [concat $lst [CreateLst [file dirname $elem] [file tail $elem] 1]]
        } else {
          puts "Didn't understand $elem"
        }
      } else {
        set lst [concat $lst [CreateLst [file dirname $elem] [file tail $elem] 0]]
      }
    }
  }

  set fp [open $ansFile.txt w]
  fconfigure $fp -translation lf
  foreach file $lst {
    puts $fp "$file"
  }
  close $fp
  catch {file delete -force $ansFile.tar.gz}
  puts "[pwd] ... Creating $ansFile.tar"
  eval {exec tar -Scf} $ansFile.tar -T$ansFile.txt
#  if {$ansFile == "degrib-src"} {
#     file copy -force degrib-src.tar degrib-temp.tar
#  }
  if {$ansFile == "degrib-src"} {
    eval {exec gzip -c} $ansFile.tar > $ansFile.tar.gz
  } else {
    eval {exec gzip} $ansFile.tar
  }
#  if {$ansFile == "degrib-src"} {
#     file copy -force degrib-temp.tar degrib-src.tar
#     file delete -force degrib-temp.tar
#  }
  if {$Zip_Also} {
    eval {exec zip -@} $ansFile.zip {<} $ansFile.txt
  }
  file delete -force $ansFile.txt
  if {$StartDir != $DestDir} {
    puts "... Copying to $DestDir"
    file copy -force $ansFile.tar.gz $DestDir
    file delete -force $ansFile.tar.gz
    if {$ansFile == "degrib-src"} {
      file copy -force degrib-src.tar $DestDir
      file delete -force degrib-src.tar
    }
    if {$Zip_Also} {
      file copy -force $ansFile.zip $DestDir
      file delete -force $ansFile.zip
    }
  }
  cd $DestDir
  if {$Zip_Also} {
    puts "Creating $ansFile.exe"
    if {! [file isfile "k:/Programs/WinZip Self-Extractor/wzipse32.exe"]} {
       puts "Couldn't find wzipse32.exe... no self extracting zip created"
       puts "Please make sure your k: drive exists."
    } else {
       exec "k:/Programs/WinZip Self-Extractor/wzipse32.exe" $ansFile.zip \
             -y -d c:\\ -le -overwrite
       file delete -force $ansFile.zip
    }
  }
  if {$UpdatePages != ""} {
    foreach elem $UpdatePages {
      puts "Editing $elem"
      DoUpdatePage $elem $ansFile.tar.gz [file size $ansFile.tar.gz] \
            [file mtime $ansFile.tar.gz] $Version
      if {$Zip_Also} {
        DoUpdatePage $elem $ansFile.exe [file size $ansFile.exe] \
            [file mtime $ansFile.exe] $Version
      }
    }
  }
}

if {($argc != 1)} {
  puts "Usage: $argv0 (<.txt file to pack> or <all> or <no-tests> or <tests>)"
  exit
}

if {[lindex $argv 0] == "all"} {
  foreach file [glob -nocomplain [file join $src_dir *.txt]] {
    if {[file tail $file] != "readme.txt"} {
      DoIt $file
    }
  }
} elseif {[lindex $argv 0] == "no-tests"} {
  foreach file [glob -nocomplain [file join $src_dir *.txt]] {
    if {[file tail $file] != "readme.txt"} {
      if {[string range [file tail $file] 0 3] != "test"} {
        DoIt $file
      }
    }
  }
} elseif {[lindex $argv 0] == "tests"} {
  foreach file [glob -nocomplain [file join $src_dir *.txt]] {
    if {[file tail $file] != "readme.txt"} {
      if {[string range [file tail $file] 0 3] == "test"} {
        DoIt $file
      }
    }
  }
} else {
  DoIt [lindex $argv 0]
}

exit
