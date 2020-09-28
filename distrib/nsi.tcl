#!/usr/bin/tclsh

if {($argc != 2)} {
  puts "Usage: $argv0 <version (see version.txt)> <version date>"
  exit
}

set version [lindex $argv 0]
set date [lindex $argv 1]
set date2 [clock format [clock scan $date] -format "%B %d, %Y"]
set curOutPath "degrib"

set fileList ""
set dirList ""

proc dosName {name} {
  return [join [file split $name] "\\"]
}

proc addFile {op path tail} {
  global curOutPath
  global fileList

  if {$path != ""} {
    puts $op "File \"..\\[dosName $path/$tail]\""
  } else {
    puts $op "File \"..\\$tail\""
  }
  lappend fileList "\$INSTDIR\\[dosName $curOutPath/$tail]"
}

proc putUninstFiles {op} {
  global fileList
  for {set i [expr [llength $fileList] - 1]} {$i >= 0} {incr i -1} {
    puts $op "\${LogIt} Delete \"[lindex $fileList $i]\""
  }
}

proc cdOutPath {op outPath} {
  global curOutPath
  global dirList

  if {$outPath != $curOutPath} {
    set curOutPath $outPath
    puts $op "SetOutPath \$INSTDIR\\[dosName $curOutPath]"
    if {[lsearch $dirList $curOutPath] == -1} {
      lappend dirList $curOutPath
    }
  }
}

proc putUninstDir {op} {
  global dirList

  set maxI 1
  foreach dir $dirList {
    set A [file split $dir]
    set lenA [llength $A]
    if {$lenA > $maxI} {
      set maxI $lenA
    }
    for {set i 1} {$i <= $lenA} {incr i} {
      set cur [join [lrange $A 0 [expr $i -1]] /]
      if {! [info exists ray($i)]} {
        set ray($i) $cur
      } elseif {[lsearch $ray($i) $cur] == -1} {
        lappend ray($i) $cur
      }
    }
  }
  for {set i $maxI} {$i >= 1} {incr i -1} {
    foreach lst [lsort $ray($i)] {
      puts $op "\${LogIt} RMDir \"\$INSTDIR\\[dosName $lst]\""
    }
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
    addFile $op $path [file tail $file]
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
          addFile $op $path [file tail $file]
        }
      } else {
        addFile $op $path $tail
      }
    }
  }
}

set icon "[file normalize [file dirname [info script]]/../bin/ndfd.ico]"
set unIcon "[file normalize [file dirname [info script]]/uninst.ico]"
catch {exec cygpath -d $icon} icon
catch {exec cygpath -d $unIcon} unIcon
# puts "$icon"
# puts "$unIcon"

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
  if {[set ans [string first PRJICON $line]] != -1} {
    set line "[string range $line 0 [expr $ans -1]]$icon[string range $line [expr $ans + 7] end]"
  }
  if {[set ans [string first PRJUNICON $line]] != -1} {
    set line "[string range $line 0 [expr $ans -1]]$unIcon[string range $line [expr $ans + 9] end]"
  }
  if {[set ans [string first PRJFILES $line]] != -1} {
    putPrjFiles $op all.txt
  } elseif {[set ans [string first PRJUNINST_FILE $line]] != -1} {
    putUninstFiles $op
  } elseif {[set ans [string first PRJUNINST_DIR $line]] != -1} {
    putUninstDir $op
  } else {
    puts $op $line
  }
}
close $op
close $fp

#Mike catch {exec /arthur/myPrograms/nsis/makensis degrib.nsi} ans
if {[file exists /cygdrive/c/NSIS/makensis]} {
   catch {exec /cygdrive/c/NSIS/makensis degrib.nsi} ans
} elseif {[file exists /cygdrive/c/sys/Portable/PortableApps/NSISPortable/App/NSIS/makensis]} {
   catch {exec /cygdrive/c/sys/Portable/PortableApps/NSISPortable/App/NSIS/makensis degrib.nsi} ans
} else {
   set ans "Couldn't find makensis.\n"
   append ans "Wasn't in c:/NSIS\n"
   append ans "nor C:/sys/Portable/PorableApps/NSISPortable/App/NSIS"
}
# catch {exec /cygdrive/c/arthur/myPrograms/nsis/makensis degrib.nsi} ans
puts $ans
