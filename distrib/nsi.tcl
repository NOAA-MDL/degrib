#!/usr/bin/tclsh

if {($argc != 2)} {
  puts "Usage: $argv0 <version (see version.txt)> <version date>"
  exit
}

set version [lindex $argv 0]
set date [lindex $argv 1]
set date2 [clock format [clock scan $date] -format "%B %d, %Y"]
set curOutPath "degrib"

proc cdOutPath {op outPath} {
  global curOutPath

  if {$outPath != $curOutPath} {
    set curOutPath $outPath
    puts $op "\${SetOutPath} \$INSTDIR\\[file nativename $outPath]"
  }
}

proc recurseWild {op path tail outPath} {
  set f_first 1
  foreach file [glob -nocomplain ../$path/$tail] {
    if {[file tail $file] == ".svn"} {
      continue
    }
    if {$f_first} {
      cdOutPath $op $outPath
      set f_first 0
    }
    puts $op "\${File} \"..\\[file nativename $path]\\\" [file tail $file]"
  }
  foreach file [glob -nocomplain ../$path/*] {
    if {[file isdirectory $file]} {
      recurseWild $op $path/[file tail $file] $tail degrib/$path/[file tail $file]
    }
  }
}

proc putPrjFiles {op {fileList all.txt}} {
  global curOutPath
  set fp [open $fileList r]
  set outPath $curOutPath
  while {[gets $fp line] >= 0} {
    set line [string trim $line]
    if {[string index $line 0] == "#"} {
      continue
    }
    set A [split $line :]
    if {[llength $A] == 2} {
      set name [lindex $A 1]
      set path [file dirname $name]
      set tail [file tail $name]
      if {$path != $outPath} {
        set outPath $path
        cdOutPath $op $outPath
      }
      set path [join [lrange [split $path /] 1 end] /]
      if {[llength $tail] != 1} {
        if {[lindex $tail 1] == "recurse"} {
          set tail [lindex $tail 0]
          recurseWild $op $path $tail $outPath
          # In case outpath has changed... reset it.
          cdOutPath $op $outPath
        } else {
          puts "DON'T HANDLE '$tail' yet"
        }
        continue
      }
      if {[string first * $tail] != -1} {
        foreach file [glob -nocomplain ../$path/$tail] {
          if {[file tail $file] == ".svn"} {
            continue
          }
          puts $op "\${File} \"..\\[file nativename $path]\\\" [file tail $file]"
        }
      } else {
        puts $op "\${File} \"..\\[file nativename $path]\\\" $tail"
      }
    }
  }
}

set fp [open degrib.tpl r]
set op [open degrib.nsi w]
fconfigure $op -translation lf
while {[gets $fp line] >= 0} {
  if {[set ans [string first PRJDATE $line]] != -1} {
    set line "[string range $line 0 [expr $ans - 1]]$date2[string range $line [expr $ans + 7] end]"
  }
  if {[set ans [string first PRJVER $line]] != -1} {
    set line "[string range $line 0 [expr $ans - 1]]$version[string range $line [expr $ans + 6] end]"
  }
  if {[set ans [string first PRJFILES $line]] != -1} {
    putPrjFiles $op all.txt
  } else {
    puts $op $line
  }
}
close $op
close $fp

catch {exec /cygdrive/c/nsis/makensis degrib.nsi} ans
puts $ans

