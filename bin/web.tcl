#!/bin/sh
# the next line restarts \
exec ./tcldegrib "$0" "$@"
# FOR VAL USE FULL PATH TO TCLDEGRIB

### The following should be set if using f_rename option.
#set renameRootDir e:/archive/ndfdfcst/
# RENAMECONV 0 does: /conus/maxt.bin
# RENAMECONV 1 does: /co/2005110700.mx.co
# RENAMECONV 2 does: /co/mx/2005110700.mx.co
# RENAMECONV 3 does: /co/mx/2005110700.bin
# RENAMECONV 4 does: /mx.co
# RENAMECONV 5 does: /conus/maxt/2005110700.maxt_d13.bin

#*****************************************************************************
# web.tcl
#
# DESCRIPTION
#   This contains an example of how to automatically download the NDFD data
# from the web site.
#
# HISTORY
#   9/2003 Arthur Taylor (MDL / RSIS): Created.
#
# NOTES
# 1) renameRootDir should be set to where to move the data to when using the
#    f_rename option.
#*****************************************************************************

### The following should be set if using f_rename option.
set renameRootDir e:/archive/ndfdfcst/

# FOR VAL USE RENAMECONV 1 does: /co/2005110700.mx.co
#             RENAMECONV 2 does: /co/mx/2005110700.mx.co
set renameConv 2

set src_dir [file dirname [info script]]
if {[file pathtype $src_dir] != "absolute"} {
  set cur_dir [pwd] ;  cd $src_dir
  set src_dir [pwd] ;  cd $cur_dir
}

foreach script [list util.tcl] {
  set file [file join $src_dir tclsrc $script]
  if {! [file exists $file]} {
    puts "Fatal Error: Couldn't find required file '$file'"
    exit
  }
  source $file
}

package require http 2.0
#*****************************************************************************
# http::copy() --
#
# PURPOSE
#    Teaches http library how to copy a URL to a file. (see "my_http.tcl")
#
# ARGUMENTS
#        url = Web site to copy from
#       file = File to copy to
# f_progress = Flag to say whether we should output a progress bar.
#      chunk = Amount to copy at one time (8192 is default)
#
# RETURNS void
#
# HISTORY
#  9/2003 Arthur Taylor (MDL/RSIS): Created.
#
# NOTES
#*****************************************************************************
namespace eval http {
  proc SetProxy {proxy port} {
    variable http
    if {($proxy != "") && ($port != "")} {
      http::config -proxyhost $proxy -proxyport $port
    } else {
      http::config -proxyhost "" -proxyport ""
    }
  }
  proc copy { url file {f_progress 0} {chunk 8192}} {
    set out [open $file w]
    if {$f_progress > 0} {
      puts -nonewline "$file (from $url): "
    }
    if {$f_progress > 1} {
      if {[catch {geturl $url -channel $out -progress ::http::Progress \
                 -blocksize $chunk -protocol 1.0} token]} {
        puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
      puts ""
    } else {
      if {[catch {geturl $url -channel $out \
                 -blocksize $chunk -protocol 1.0} token]} {
        puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
    }
    close $out
    if {[http::status $token] == "timeout"} {
      puts "timeout: $file"
      return 1
    }
    return 0
  }
  proc myAppend { url file {f_progress 0} {chunk 8192}} {
    set out [open $file a]
    if {$f_progress > 0} {
      puts -nonewline "$file (from $url): "
    }
    if {$f_progress > 1} {
      if {[catch {geturl $url -channel $out -progress ::http::Progress \
                 -blocksize $chunk -protocol 1.0} token]} {
        puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
      puts ""
    } else {
      if {[catch {geturl $url -channel $out \
                 -blocksize $chunk -protocol 1.0} token]} {
        puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
    }
    close $out
    if {[http::status $token] == "timeout"} {
      puts "timeout: $file"
      return 1
    }
    return 0
  }
  proc Progress {args} {
    puts -nonewline . ; flush stdout
  }
}

#*****************************************************************************
# Finished including libraries.  Now add program specific procedures.
#*****************************************************************************

proc SectorConv {sector f_renameConv} {
  if {($f_renameConv == 1) || ($f_renameConv == 2)} {
    if {$sector == "conus"}    {return co }
    if {$sector == "guam"}     {return gu }
    if {$sector == "hawaii"}   {return hi }
    if {$sector == "puertori"} {return pr }
    if {$sector == "alaska"}   {return ak }
    return $sector
  }
  return $sector
}

proc GroupConv {sector f_renameConv} {
  if {($f_renameConv == 1) || ($f_renameConv == 2)} {
    return $sector
  }
  return $sector
}

#*****************************************************************************
#  ReadIni -- Arthur Taylor / MDL
#
# PURPOSE
#    Read from the "ndfd.ini control file
#
# ARGUMENTS
#      rayName = Global array (structure) containing the main variables for
#                this program.
#      (path,) = "FULL_PATH" section
#       (dir,) = "Directories" section
#    (imgGen,) = "ImageGen" section
#    (custom,) = "Custom" section
#       (ftp,) = "FTPSite" section
#      (http,) = "HTTPSite" section
#       (opt,) = "Options" section
# (subSector,) = "CONUS_SubSectors" section
# (subExprSector,) = "CONUS_Expr_SubSectors" section
#   (foreVar,) = "NDFD_Variables" section
#   (foreExprVar,) = "NDFD_EXPR_Variables" section
#   (guidVar,) = "NDGD_Variables" section
#   (guidExprVar,) = "NDGD_EXPR_Variables" section
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Copied from "ndfd.tcl"
#
# NOTES
#*****************************************************************************
proc ReadIni {rayName} {
  global src_dir
  upvar #0 $rayName ray
  set Parent [file dirname $src_dir]
  set filename [file join $src_dir ndfd.ini]

##### Load the "Directories" section (dir,) #####
  set ray(dirList) ""
  set ans [ns_Util::ReadIni $filename NDFD_Directories *]
  foreach pair $ans {
    set var [lindex $pair 0]
    lappend ray(dirList) $var
    set val [split [lindex $pair 1] ,]
    if {[llength $val] == 1} {
      set ray(dir,$var) $val
    } elseif {[llength $val] > 2} {
      tk_messageBox -message "Didn't understand $pair, skipping"
    } elseif {[lindex $val 0] == "Parent"} {
      set ray(dir,$var) [file join $Parent [lindex $val 1]]
    } else {
      tk_messageBox -message "Didn't understand $pair, skipping"
    }
    # Make sure the directories exist.
#    if {! [file exists $ray(dir,$var)]} {
#      file mkdir $ray(dir,$var)
#    }
  }
  # Check to make sure essential directories are set.
  set critList [list Bin Mosaic GIS_IN Web_Data NDGD_OpnlData NDFD_OpnlData NDGD_ExprData NDFD_ExprData GIS_OUT]
  foreach var $critList {
    if {! [info exists ray(dir,$var)]} {
      tk_messageBox -message "One of the critical 'NDFD_Directories'\
                              variables in $filename was not set"
      tk_messageBox -message "NDFD_Directories should consist of $critList"
      exit
    }
  }

##### Load the "ImageGen" section (imgGen,)
##### Also set up the "env" variables so we can call imageGen
  global env
  set ans [ns_Util::ReadIni $filename NDFD_ImageGen *]
  set ray(imgGenList) ""
  foreach pair $ans {
    set var [lindex $pair 0]
    lappend ray(imgGenList) $var
    set val [split [lindex $pair 1] ,]
    if {[llength $val] == 1} {
      set ray(imgGen,$var) $val
    } elseif {[llength $val] > 2} {
      tk_messageBox -message "Didn't understand $pair, skipping"
    } elseif {[lindex $val 0] == "Parent"} {
      set ray(imgGen,$var) [file join $Parent [lindex $val 1]]
    } else {
      tk_messageBox -message "Didn't understand $pair, skipping"
    }
    # Make sure the directories exist.
    if {$var != "LOG_PREF"} {
#      if {! [file exists $ray(imgGen,$var)]} {
#        file mkdir $ray(imgGen,$var)
#      }
    }
    set env($var) $ray(imgGen,$var)
  }
  # Check to make sure essential directories are set.
  set critList [list LOG_PREF LOG_DIR DATABASE NDFD_DATA NDFD_CITIES \
                NDFD_COLOR NDFD_CONFIG NDFD_SHAPE WEB_IMAGE_PATH \
                NDFD_WEBDATA HTML_DIR]
  foreach var $critList {
    if {! [info exists ray(imgGen,$var)]} {
      tk_messageBox -message "One of the critical 'NDFD_ImageGen' variables\
                              in $filename was not set"
      tk_messageBox -message "NDFD_ImageGen should consist of $critList"
      exit
    }
  }

##### Load the "Custom" section (custom,) #####
  set ans [ns_Util::ReadIni $filename Custom_Sector *]
  set ray(customList) ""
  set ray(customVarList) ""
  foreach pair $ans {
    set var [lindex $pair 0]
    if {[string range $var 0 5] == "custom"} {
      set val [split [lindex $pair 1] ,]
      if {[llength $val] != 6} {
        set var "#$var"
        set ray(custom,$var) [lindex $pair 1]
        lappend ray(customList) $var
      } else {
        set ray(custom,$var) $val
        lappend ray(customVarList) $var
      }
    } else {
      lappend ray(customList) $var
      set ray(custom,$var) [lindex $pair 1]
    }
  }

##### Load the "FTPSite" section (ftp,) #####
  set ans [ns_Util::ReadIni $filename NDFD_FTPSite *]
  set ray(ftpList) ""
  foreach pair $ans {
    lappend ray(ftpList) [lindex $pair 0]
    set ray(ftp,[lindex $pair 0]) [lindex $pair 1]
  }

##### Load the "HTTPSite" section (http,) #####
  set ans [ns_Util::ReadIni $filename NDFD_HTTPSite *]
  set ray(httpList) ""
  foreach pair $ans {
    lappend ray(httpList) [lindex $pair 0]
    set ray(http,[lindex $pair 0]) [lindex $pair 1]
  }

##### Load the "Options" section (opt,) #####
  set ans [ns_Util::ReadIni $filename NDFD_Options *]
  set ray(optList) ""
  foreach pair $ans {
    lappend ray(optList) [lindex $pair 0]
    set ray(opt,[lindex $pair 0]) [lindex $pair 1]
  }

##### Load the "SubSectors" section (subSector,) #####
  set ans [ns_Util::ReadIni $filename CONUS_SubSectors *]
  set ray(subSectorList) ""
  foreach pair $ans {
    set var [lindex $pair 0]
    if {[string index $var 0] == "#"} {
      set ray(subSector,$var) [lindex $pair 1]
    } else {
      set val [split [lindex $pair 1] ,]
      if {[llength $val] != 16} {
        set var "#$var"
        set ray(subSector,$var) [lindex $pair 1]
      } else {
        set ray(subSector,$var) $val
      }
    }
    lappend ray(subSectorList) $var
  }

##### Load the "ExprSubSectors" section (subExprSector,) #####
  set ans [ns_Util::ReadIni $filename CONUS_ExprSubSectors *]
  set ray(subExprSectorList) ""
  foreach pair $ans {
    set var [lindex $pair 0]
    if {[string index $var 0] == "#"} {
      set ray(subExprSector,$var) [lindex $pair 1]
    } else {
      set val [split [lindex $pair 1] ,]
      if {[llength $val] != 16} {
        set var "#$var"
        set ray(subExprSector,$var) [lindex $pair 1]
      } else {
        set ray(subExprSector,$var) $val
      }
    }
    lappend ray(subExprSectorList) $var
  }

##### Load the "NDFD_Variables" section (foreVar,) #####
  set ans [ns_Util::ReadIni $filename NDFD_Variables *]
  set ray(foreVarList) ""
  foreach pair $ans {
    set var [lindex $pair 0]
    if {[string index $var 0] == "#"} {
      set ray(foreVar,$var) [lindex $pair 1]
    } else {
      set val [split [lindex $pair 1] ,]
      if {[llength $val] != 6} {
        set var "#$var"
        set ray(foreVar,$var) [lindex $pair 1]
      } else {
        set ray(foreVar,$var) $val
      }
    }
    lappend ray(foreVarList) $var
  }

##### Load the "NDFD_EXPR_Variables" section (foreExprVar,) #####
  set ans [ns_Util::ReadIni $filename NDFD_EXPR_Variables *]
  set ray(foreExprVarList) ""
  foreach pair $ans {
    set var [lindex $pair 0]
    if {[string index $var 0] == "#"} {
      set ray(foreExprVar,$var) [lindex $pair 1]
    } else {
      set val [split [lindex $pair 1] ,]
      if {[llength $val] != 6} {
        set var "#$var"
        set ray(foreExprVar,$var) [lindex $pair 1]
      } else {
        set ray(foreExprVar,$var) $val
      }
    }
    lappend ray(foreExprVarList) $var
  }

##### Load the "NDGD_Variables" section (guidVar,) #####
  set ans [ns_Util::ReadIni $filename NDGD_Variables *]
  set ray(guidVarList) ""
  foreach pair $ans {
    set var [lindex $pair 0]
    if {[string index $var 0] == "#"} {
      set ray(guidVar,$var) [lindex $pair 1]
    } else {
      set val [split [lindex $pair 1] ,]
      if {[llength $val] != 6} {
        set var "#$var"
        set ray(guidVar,$var) [lindex $pair 1]
      } else {
        set ray(guidVar,$var) $val
      }
    }
    lappend ray(guidVarList) $var
  }

##### Load the "NDGD_EXPR_Variables" section (guidExprVar,) #####
  set ans [ns_Util::ReadIni $filename NDGD_EXPR_Variables *]
  set ray(guidExprVarList) ""
  foreach pair $ans {
    set var [lindex $pair 0]
    if {[string index $var 0] == "#"} {
      set ray(guidExprVar,$var) [lindex $pair 1]
    } else {
      set val [split [lindex $pair 1] ,]
      if {[llength $val] != 6} {
        set var "#$var"
        set ray(guidExprVar,$var) [lindex $pair 1]
      } else {
        set ray(guidExprVar,$var) $val
      }
    }
    lappend ray(guidExprVarList) $var
  }

}

#####
# Return 1 if we think the file is an NDFD GRIB2 file.
# Return 0 otherwise
#####
proc IsGRIB2 {rayName filename {size -1}} {
  upvar #0 $rayName ray
  if {$size == -1} {
    set size [file size $filename]
  }
  if {$size < 84} {
    return 0
  }
  if {! [file readable $filename]} {
    return 0
  }
  return [$ray(GRIB2Cmd) isGrib2 -in $filename]
}

#*****************************************************************************
#  HttpTryOne -- Arthur Taylor / MDL
#
# PURPOSE
#    Try to get one of the files from the tryList.  Return 0 on success, 1 if
#  it fails.
#
# ARGUMENTS
#   rayName = global program array.
#      src0 = source URL (or list of URLs)
#       dst = where to store the file on the local system.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#   Could check flag field, but only NDFD has them.
#*****************************************************************************
proc HttpTryOne {rayName usrName src0 dst} {
  upvar #0 $rayName ray
  upvar #0 $usrName usr

  http::SetProxy $ray(http,Proxy) $ray(http,ProxyPort)
  if {$usr(-quiet) == 0} {
    set progress 2
  } elseif {$usr(-quiet) == 1} {
    set progress 1
  } else {
    set progress 0
  }
  if {[llength $src0] == 1} {
    if {[http::copy $src0 $dst $progress 20480] != 0} {
      return 1
    }
    ##### Check if it is a GRIB file, #####
    if {! [IsGRIB2 $rayName $dst -1]} {
      return 1
    }
  } else {
    set f_first 1
    foreach elem $src0 {
      if {$f_first} {
        if {[http::copy $elem $dst $progress 20480] != 0} {
          return 1
        }
        set f_first 0
      } else {
        if {[http::myAppend $elem $dst $progress 20480] != 0} {
          return 1
        }
      }
    }
    ##### Check if it is a GRIB file, #####
    if {! [IsGRIB2 $rayName $dst -1]} {
      return 1
    }
  }
  return 0
}

#*****************************************************************************
#  WebHandleTryList -- Arthur Taylor / MDL
#
# PURPOSE
#    Go throuth the trylist attempting to download the requested files to the
#  requested destinations.
#    It does this by assuming that tryList is a list of pairs of elements. The
#  second element is where to put the data (on the local machine).  The first
#  element is a list of URL's to try.  It tries each one "attempt" times
#  before giving up and going to the next one.  It goes to the next one by
#  creating a new tryList without the first URL, and then calling
#  HandleTryList again.
#
# ARGUMENTS
#   rayName = global program array.
#   tryList = List of pairs of source and destination.
#   attempt = number of times to try a URL.
#   f_first = First time called.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#   Could check flag field, but only NDFD has them.
#*****************************************************************************
proc WebHandleTryList {rayName usrName tryList attempt f_first} {
  upvar #0 $rayName ray
  upvar #0 $usrName usr

  for {set i 0} {$i < [expr $attempt - 1]} {incr i} {
    set tryList2 ""
    foreach elem $tryList {
      set dst [lindex $elem 1]
      set src [lindex $elem 0]
      set src0 [lindex $src 0]

#      if {! [file exists [file dirname $dst]]} {
#        file mkdir [file dirname $dst]
#      }
      set f_fail [HttpTryOne $rayName $usrName $src0 $dst]
      if {! $f_fail} {
        if {[catch {$ray(GRIB2Cmd) -I -in $dst} ans]} {
          puts "  Couldn't inventory the file. Try again."
          set f_fail 1
        } else {
          set refTime [lindex [lindex $ans 0] 2]
          set refClock [clock scan $refTime -gmt true]
          set time [clock format $refClock -format "%Y%m%d%H" -gmt true]
        }
      }
      if {$usr(-quiet) <= 1} {
        puts ""
      }
      if {$f_fail} {
        puts "  Couldn't 'get' $dst... Will ReTry later."
        file delete -force $dst
        lappend tryList2 [list $src $dst]
      } else {
        puts "Got $dst\n"
        set name [file tail $dst]
        set nameList [split $name .]
        set index [lsearch $nameList TIME]
        if {$index != -1} {
          set nameList [lreplace $nameList $index $index $time]
          set name [join $nameList .]
          set newName [file join [file dirname $dst] $name]
          file rename -force $dst $newName
        }
      }
    }
    if {$tryList2 == ""} {
      return 0
    } else {
      set tryList $tryList2
    }
  }

  ##### handle failures on last attempt by pop'ing them off the stack. #####
  set tryList2 ""
  foreach elem $tryList {
    set dst [lindex $elem 1]
    set src [lindex $elem 0]
    set src0 [lindex $src 0]

    set f_fail [HttpTryOne $rayName $usrName $src0 $dst]
    if {! $f_fail} {
      if {[catch {$ray(GRIB2Cmd) -I -in $dst} ans]} {
        puts "  Couldn't inventory the file. Try again."
        set f_fail 1
      } else {
        set refTime [lindex [lindex $ans 0] 2]
        set refClock [clock scan $refTime -gmt true]
        set time [clock format $refClock -format "%Y%m%d%H" -gmt true]
      }
    }
    puts ""
    if {$f_fail} {
      if {[llength $src] > 1} {
        puts "  Couldn't 'get' $dst... Try alternate URL."
        lappend tryList2 [list [lrange $src 1 end] $dst]
      } else {
        puts "  Failed to 'get' $dst."
      }
    } else {
      puts "Got $dst"
      set name [file tail $dst]
      set nameList [split $name .]
      set index [lsearch $nameList TIME]
      if {$index != -1} {
        set nameList [lreplace $nameList $index $index $time]
        set name [join $nameList .]
        set newName [file join [file dirname $dst] $name]
        file rename -force $dst $newName
      }
    }
  }

  if {$tryList2 != ""} {
    WebHandleTryList $rayName $usrName $tryList2 $attempt 0
    return 0
  }
  return 0
}

proc Download_Select {rayName usrName} {
  upvar #0 $rayName ray
  upvar #0 $usrName usr

  set tryList ""
  set Server1 $ray(http,Server)
  set Server2 $ray(http,Server)
  set foreExprDir $ray(http,foreExprDir)
  set foreOpnlDir $ray(http,foreOpnlDir)
  set guidExprDir $ray(http,guidExprDir)
  set guidOpnlDir $ray(http,guidOpnlDir)

  if {$usr(-dataSet) == "ndfd"} {
    if {(! $usr(subSector)) && (! $usr(custom))} {
      set varList ""
      ##### Set up valid variable List for this sector. #####
      foreach var $ray(foreVarList) {
        if {[string index $var 0] == "#"} {
          continue
        }
        set tmpList [split $var -]
        set sector [lindex $tmpList 0]
        set day [lindex $tmpList 2]
        if {$usr(shortSector) == $sector} {
          if {($usr(-day) == "all") || \
              (($usr(-day) == "d17") && ($day == "")) || \
              ($usr(-day) == $day)} {
            lappend varList [join [lrange $tmpList 1 end] -]
          }
        }
      }
      if {$usr(-variable) == "all"} {
        set varList2 ""
        foreach var $varList {
          lappend varList2 "$usr(shortSector)-$var"
        }
      } else {
        set varList2 ""
        foreach var $usr(-variable) {
          if {[lsearch $varList $var] == -1} {
            puts "Don't recognize variable '$var' in sector '$usr(-sector)', day '$usr(-day)'"
            puts "Valid variables for this sector '$usr(-sector)' are: 'all' and"
            puts "$varList\n"
            exit
          }
          lappend varList2 "$usr(shortSector)-$var"
        }
      }
      foreach var $varList2 {
        set status [lindex $ray(foreVar,$var) 0]
        set sec0 $usr(shortSector)
        if {$sec0 == "conus"} {
          set sec0 "co"
        }
        set var0 [lindex [split $var -] 1]
        if {[info exists ray(renameVar,$var0)]} {
          set var0 $ray(renameVar,$var0)
        }
        set local [lindex $ray(foreVar,$var) 2]
        set extra [split [file rootname [file tail $local]] _]
        if {[llength $extra] > 1} {
          set var1 "$var0\_[join [lrange $extra 1 end] _]"
          set var2 [join [lrange $extra 1 end] _]
        } else {
          set var1 $var0
          set var2 ""
        }
        if {$usr(-renameConv) == 0} {
          set localName [file join $usr(-renameRoot) $local]
        } elseif {$usr(-renameConv) == 1} {
          set localName [file join $usr(-renameRoot) $sec0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 2} {
          set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 3} {
          if {$var2 == ""} {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.bin]
          } else {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var2.bin]
          }
        } elseif {$usr(-renameConv) == 4} {
          set localName [file join $usr(-renameRoot) $var1.$sec0]
        } elseif {$usr(-renameConv) == 5} {
          set localName [file join $usr(-renameRoot) [lindex [split [file rootname $local] _] 0] TIME.[file tail $local]] 
        }

        set srvName "$Server1$foreOpnlDir[lindex $ray(foreVar,$var) 3]"
        if {$status == "join4"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          set srvSplitName4 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3 $srvSplitName4]] $localName]
        } elseif {$status == "join3"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3]] $localName]
        } elseif {$status == "join_d12"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
        } elseif {$status == "join_d23"} {
          set tempList [split $srvName /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          lappend tryList [list [list [list $srvSplitName2 $srvSplitName3]] $localName]
        } elseif {$status == "join"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
        } elseif {$status == "pure"} {
          lappend tryList [list $srvName $localName]
        } elseif {$status == "NA"} {
          continue
        } else {
          puts "Don't understand status of [lindex $ray(foreVar,$var) 0]"
          continue
        }

        if {! [file exists [file dirname $localName]]} {
          file mkdir [file dirname $localName]
        }
      }
    } elseif {! $usr(custom)} {
      set varList ""
      ##### Set up valid variable List for this subsector. #####
      foreach var $ray(subSectorList) {
        if {[string range $var 0 4] != "conus"} {
          continue
        }
        set tmpList [split $var -]
        set day [lindex $tmpList 2]
        if {($usr(-day) == "all") || \
            (($usr(-day) == "d17") && ($day == "")) || \
            ($usr(-day) == $day)} {
          lappend varList [join [lrange $tmpList 1 end] -]
        }
      }
      if {$usr(-variable) == "all"} {
        set varList2 ""
        foreach var $varList {
          lappend varList2 "conus-$var"
        }
      } else {
        set varList2 ""
        foreach var $usr(-variable) {
          if {[lsearch $varList $var] == -1} {
            puts "Don't recognize variable '$var' in sector '$usr(-sector)', day '$usr(-day)'"
            puts "Valid variables for this sector '$usr(-sector)' are: 'all' and"
            puts "$varList\n"
            exit
          }
          lappend varList2 "conus-$var"
        }
      }
      foreach var $varList2 {
        set sector $usr(-sector)
        set index [lsearch $ray(subSector,LocalName) $sector]
        set remote [lindex $ray(subSector,RemoteName) $index]
        set status [lindex $ray(foreVar,$var) 0]
        set local [string replace [lindex $ray(foreVar,$var) 2] 0 4 $sector]
        set sec0 $sector
        set var0 [lindex [split $var -] 1]
        if {[info exists ray(renameVar,$var0)]} {
          set var0 $ray(renameVar,$var0)
        }
        set extra [split [file rootname [file tail $local]] _]
        if {[llength $extra] > 1} {
          set var1 "$var0\_[join [lrange $extra 1 end] _]"
          set var2 [join [lrange $extra 1 end] _]
        } else {
          set var1 $var0
          set var2 ""
        }
        if {$usr(-renameConv) == 0} {
          set localName [file join $usr(-renameRoot) $local]
        } elseif {$usr(-renameConv) == 1} {
          set localName [file join $usr(-renameRoot) $sec0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 2} {
          set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 3} {
          if {$var2 == ""} {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.bin]
          } else {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var2.bin]
          }
        } elseif {$usr(-renameConv) == 4} {
          set localName [file join $usr(-renameRoot) $var1.$sec0]
        } elseif {$usr(-renameConv) == 5} {
          set localName [file join $usr(-renameRoot) [lindex [split [file rootname $local] _] 0] TIME.[file tail $local]]
        }
        set remotePath [string replace [lindex $ray(foreVar,$var) 3] 3 7 $remote]

        set srvName "$Server1$foreOpnlDir[lindex $ray(foreVar,$var) 3]"
        if {$status == "join4"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          set srvSplitName4 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3 $srvSplitName4]] $localName]
        } elseif {$status == "join3"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3]] $localName]
        } elseif {$status == "join_d12"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
        } elseif {$status == "join_d23"} {
          set tempList [split $srvName /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          lappend tryList [list [list [list $srvSplitName2 $srvSplitName3]] $localName]
        } elseif {$status == "join"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
        } elseif {$status == "pure"} {
          lappend tryList [list $srvName $localName]
        } elseif {$status == "NA"} {
          continue
        } else {
          puts "Don't understand status of [lindex $ray(foreVar,$var) 0]"
          continue
        }

        if {! [file exists [file dirname $localName]]} {
          file mkdir [file dirname $localName]
        }
      }
    } else {
      set varList ""
      ##### Set up valid variable List for this group. #####
      foreach var $ray(customVarList) {
        set tmpList [split $var -]
        set day [lindex $tmpList 2]
        if {($usr(-day) == "all") || \
            (($usr(-day) == "d17") && ($day == "")) || \
            ($usr(-day) == $day)} {
          lappend varList [join [lrange $tmpList 1 end] -]
        }
      }
      if {$usr(-variable) == "all"} {
        set varList2 ""
        foreach var $varList {
          lappend varList2 "custom-$var"
        }
      } else {
        foreach var $usr(-variable) {
          if {[lsearch $varList $var] == -1} {
            puts "Don't recognize variable '$var' in sector '$usr(-sector)', day '$usr(-day)'"
            puts "Valid variables for this sector '$usr(-sector)' are: 'all' and"
            puts "$varList\n"
            exit
          }
          lappend varList2 "custom-$var"
        }
      }
      foreach var $varList2 {
        set sec0 "custom"
        set var0 [lindex [split $var -] 1]
        if {[info exists ray(renameVar,$var0)]} {
          set var0 $ray(renameVar,$var0)
        }
        set local [lindex $ray(custom,$var) 2]
        set extra [split [file rootname [file tail $local]] _]
        if {[llength $extra] > 1} {
          set var1 "$var0\_[join [lrange $extra 1 end] _]"
          set var2 [join [lrange $extra 1 end] _]
        } else {
          set var1 $var0
          set var2 ""
        }
        if {$usr(-renameConv) == 0} {
          set localName [file join $usr(-renameRoot) $local]
        } elseif {$usr(-renameConv) == 1} {
          set localName [file join $usr(-renameRoot) $sec0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 2} {
          set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 3} {
          if {$var2 == ""} {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.bin]
          } else {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var2.bin]
          }
        } elseif {$usr(-renameConv) == 4} {
          set localName [file join $usr(-renameRoot) $var1.$sec0]
        } elseif {$usr(-renameConv) == 5} {
          set localName [file join $usr(-renameRoot) [lindex [split [file rootname $local] _] 0] TIME.[file tail $local]]
        }
        set box [split $ray(custom,box) ,]
        set opnlName "$ray(custom,URL)?var=[lindex $ray(custom,$var) 3]&lat1=[lindex $box 0]&lon1=[lindex $box 1]&lat2=[lindex $box 2]&lon2=[lindex $box 3]"
        lappend tryList [list [list $opnlName] $localName]
        if {! [file exists [file dirname $localName]]} {
          file mkdir [file dirname $localName]
        }
      }
    }

  } elseif {$usr(-dataSet) == "ndfdExpr"} {
    if {(! $usr(subSector)) && (! $usr(custom))} {
      set varList ""
      ##### Set up valid variable List for this sector. #####
      foreach var $ray(foreExprVarList) {
        if {[string index $var 0] == "#"} {
          continue
        }
        set tmpList [split $var -]
        set sector [lindex $tmpList 0]
        set day [lindex $tmpList 2]
        if {$usr(shortSector) == $sector} {
          if {($usr(-day) == "all") || \
              (($usr(-day) == "d17") && ($day == "")) || \
              ($usr(-day) == $day)} {
            lappend varList [join [lrange $tmpList 1 end] -]
          }
        }
      }
      if {$usr(-variable) == "all"} {
        set varList2 ""
        foreach var $varList {
          lappend varList2 "$usr(shortSector)-$var"
        }
      } else {
        set varList2 ""
        foreach var $usr(-variable) {
          if {[lsearch $varList $var] == -1} {
            puts "Don't recognize variable '$var' in sector '$usr(-sector)', day '$usr(-day)'"
            puts "Valid variables for this sector '$usr(-sector)' are: 'all' and"
            puts "$varList\n"
            exit
          }
          lappend varList2 "$usr(shortSector)-$var"
        }
      }
      foreach var $varList2 {
        set status [lindex $ray(foreExprVar,$var) 0]
        set sec0 $usr(shortSector)
        if {$sec0 == "conus"} {
          set sec0 "co"
        }
        set var0 [lindex [split $var -] 1]
        if {[info exists ray(renameVar,$var0)]} {
          set var0 $ray(renameVar,$var0)
        }
        set local [lindex $ray(foreExprVar,$var) 2]
        set extra [split [file rootname [file tail $local]] _]
        if {[llength $extra] > 1} {
          set var1 "$var0\_[join [lrange $extra 1 end] _]"
          set var2 [join [lrange $extra 1 end] _]
        } else {
          set var1 $var0
          set var2 ""
        }
        if {$usr(-renameConv) == 0} {
          set localName [file join $usr(-renameRoot) $local]
        } elseif {$usr(-renameConv) == 1} {
          set localName [file join $usr(-renameRoot) $sec0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 2} {
          set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 3} {
          if {$var2 == ""} {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.bin]
          } else {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var2.bin]
          }
        } elseif {$usr(-renameConv) == 4} {
          set localName [file join $usr(-renameRoot) $var1.$sec0]
        } elseif {$usr(-renameConv) == 5} {
          set localName [file join $usr(-renameRoot) [lindex [split [file rootname $local] _] 0] TIME.[file tail $local]]
        }

        set srvName "$Server1$foreExprDir[lindex $ray(foreExprVar,$var) 3]"
        if {$status == "join4"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          set srvSplitName4 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3 $srvSplitName4]] $localName]
        } elseif {$status == "join3"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3]] $localName]
        } elseif {$status == "join_d12"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
        } elseif {$status == "join_d23"} {
          set tempList [split $srvName /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          lappend tryList [list [list [list $srvSplitName2 $srvSplitName3]] $localName]
        } elseif {$status == "join"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
        } elseif {$status == "pure"} {
          lappend tryList [list $srvName $localName]
        } elseif {$status == "NA"} {
          continue
        } else {
          puts "Don't understand status of [lindex $ray(foreExprVar,$var) 0]"
          continue
        }

        if {! [file exists [file dirname $localName]]} {
          file mkdir [file dirname $localName]
        }
      }
    } elseif {! $usr(custom)} {
      set varList ""
      ##### Set up valid variable List for this subsector. #####
      foreach var $ray(subExprSectorList) {
        if {[string range $var 0 4] != "conus"} {
          continue
        }
        set tmpList [split $var -]
        set day [lindex $tmpList 2]
        if {($usr(-day) == "all") || \
            (($usr(-day) == "d17") && ($day == "")) || \
            ($usr(-day) == $day)} {
          lappend varList [join [lrange $tmpList 1 end] -]
        }
      }
      if {$usr(-variable) == "all"} {
        set varList2 ""
        foreach var $varList {
          lappend varList2 "conus-$var"
        }
      } else {
        set varList2 ""
        foreach var $usr(-variable) {
          if {[lsearch $varList $var] == -1} {
            puts "Don't recognize variable '$var' in sector '$usr(-sector)', day '$usr(-day)'"
            puts "Valid variables for this sector '$usr(-sector)' are: 'all' and"
            puts "$varList\n"
            exit
          }
          lappend varList2 "conus-$var"
        }
      }
      foreach var $varList2 {
        set sector $usr(-sector)
        set index [lsearch $ray(subExprSector,LocalName) $sector]
        set remote [lindex $ray(subExprSector,RemoteName) $index]
        set status [lindex $ray(foreExprVar,$var) 0]
        set local [string replace [lindex $ray(foreExprVar,$var) 2] 0 4 $sector]
        set sec0 $sector
        set var0 [lindex [split $var -] 1]
        if {[info exists ray(renameVar,$var0)]} {
          set var0 $ray(renameVar,$var0)
        }
        set extra [split [file rootname [file tail $local]] _]
        if {[llength $extra] > 1} {
          set var1 "$var0\_[join [lrange $extra 1 end] _]"
          set var2 [join [lrange $extra 1 end] _]
        } else {
          set var1 $var0
          set var2 ""
        }
        if {$usr(-renameConv) == 0} {
          set localName [file join $usr(-renameRoot) $local]
        } elseif {$usr(-renameConv) == 1} {
          set localName [file join $usr(-renameRoot) $sec0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 2} {
          set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var1.$sec0]
        } elseif {$usr(-renameConv) == 3} {
          if {$var2 == ""} {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.bin]
          } else {
             set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var2.bin]
          }
        } elseif {$usr(-renameConv) == 4} {
          set localName [file join $usr(-renameRoot) $var1.$sec0]
        } elseif {$usr(-renameConv) == 5} {
          set localName [file join $usr(-renameRoot) [lindex [split [file rootname $local] _] 0] TIME.[file tail $local]]
        }
        set remotePath [string replace [lindex $ray(foreExprVar,$var) 3] 3 7 $remote]

        set srvName "$Server1$foreExprDir[lindex $ray(foreExprVar,$var) 3]"
        if {$status == "join4"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          set srvSplitName4 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3 $srvSplitName4]] $localName]
        } elseif {$status == "join3"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3]] $localName]
        } elseif {$status == "join_d12"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
        } elseif {$status == "join_d23"} {
          set tempList [split $srvName /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
          set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
          lappend tryList [list [list [list $srvSplitName2 $srvSplitName3]] $localName]
        } elseif {$status == "join"} {
          set tempList [split $srvName /]
          set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
          set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
          lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
        } elseif {$status == "pure"} {
          lappend tryList [list $srvName $localName]
        } elseif {$status == "NA"} {
          continue
        } else {
          puts "Don't understand status of [lindex $ray(foreExprVar,$var) 0]"
          continue
        }

        if {! [file exists [file dirname $localName]]} {
          file mkdir [file dirname $localName]
        }
      }
    } else {
      # No expr NDFD custom.
    }

  } elseif {$usr(-dataSet) == "ndgd"} {
    set varList ""
    ##### Set up valid variable List for this group. #####
    foreach var $ray(guidVarList) {
      if {[string index $var 0] == "#"} {
        continue
      }
      set tmpList [split $var -]
      set day [lindex $tmpList 3]
      set group [lindex $tmpList 0]
      if {$usr(-group) == $group} {
        if {($usr(-day) == "all") || \
            (($usr(-day) == "d17") && ($day == "")) || \
            ($usr(-day) == $day)} {
          lappend varList [join [lrange $tmpList 1 end] -]
        }
      }
    }
    if {$usr(-variable) == "all"} {
      set varList2 ""
      foreach var $varList {
        lappend varList2 "$usr(-group)-$var"
      }
    } else {
      set varList2 ""
      foreach var $usr(-variable) {
        if {[lsearch $varList $var] == -1} {
          puts "Don't recognize variable '$var' in group '$usr(-group)', day '$usr(-day)'"
          puts "Valid variables for this group '$usr(-group)' are: 'all' and"
          puts "$varList\n"
          exit
        }
        lappend varList2 "$usr(-group)-$var"
      }
    }
    foreach var $varList2 {
      set status [lindex $ray(guidVar,$var) 0]
      set sec0 $usr(-group)
      set var0 [lindex [split $var -] 1]
      if {[info exists ray(renameVar,$var0)]} {
        set var0 $ray(renameVar,$var0)
      }
      set local [lindex $ray(guidVar,$var) 2]
      set extra [split [file rootname [file tail $local]] _]
      if {[llength $extra] > 1} {
        set var1 "$var0\_[join [lrange $extra 1 end] _]"
        set var2 [join [lrange $extra 1 end] _]
      } else {
        set var1 $var0
        set var2 ""
      }
      if {$usr(-renameConv) == 0} {
        set localName [file join $usr(-renameRoot) $local]
      } elseif {$usr(-renameConv) == 1} {
        set localName [file join $usr(-renameRoot) $sec0 TIME.$var1.$sec0]
      } elseif {$usr(-renameConv) == 2} {
        set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var1.$sec0]
      } elseif {$usr(-renameConv) == 3} {
        if {$var2 == ""} {
           set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.bin]
        } else {
           set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var2.bin]
        }
      } elseif {$usr(-renameConv) == 4} {
        set localName [file join $usr(-renameRoot) $var1.$sec0]
      } elseif {$usr(-renameConv) == 5} {
        set localName [file join $usr(-renameRoot) [file tail [file dirname $local]] [string toupper [file dirname [file dirname $local]]] [lindex [split [file rootname [file tail $local]] _] 0] TIME.[file tail $local]] 
      }

      set srvName "$Server1$guidOpnlDir[lindex $ray(guidVar,$var) 3]"
      if {$status == "join4"} {
        set tempList [split $srvName /]
        set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
        set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
        set srvSplitName4 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
        lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3 $srvSplitName4]] $localName]
      } elseif {$status == "join3"} {
        set tempList [split $srvName /]
        set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
        set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
        lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3]] $localName]
      } elseif {$status == "join_d12"} {
        set tempList [split $srvName /]
        set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
        lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
      } elseif {$status == "join_d23"} {
        set tempList [split $srvName /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
        set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
        lappend tryList [list [list [list $srvSplitName2 $srvSplitName3]] $localName]
      } elseif {$status == "join"} {
        set tempList [split $srvName /]
        set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
        lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
      } elseif {$status == "pure"} {
        lappend tryList [list $srvName $localName]
      } elseif {$status == "NA"} {
        continue
      } else {
        puts "Don't understand status of [lindex $ray(foreVar,$var) 0]"
        continue
      }

      if {! [file exists [file dirname $localName]]} {
        file mkdir [file dirname $localName]
      }
    }

  } elseif {$usr(-dataSet) == "ndgdExpr"} {
    set varList ""
    ##### Set up valid variable List for this group. #####
    foreach var $ray(guidExprVarList) {
      if {[string index $var 0] == "#"} {
        continue
      }
      set tmpList [split $var -]
      set day [lindex $tmpList 3]
      set group [lindex $tmpList 0]
      if {$usr(-group) == $group} {
        if {($usr(-day) == "all") || \
            (($usr(-day) == "d17") && ($day == "")) || \
            ($usr(-day) == $day)} {
          lappend varList [join [lrange $tmpList 1 end] -]
        }
      }
    }
    if {$usr(-variable) == "all"} {
      set varList2 ""
      foreach var $varList {
        lappend varList2 "$usr(-group)-$var"
      }
    } else {
      set varList2 ""
      foreach var $usr(-variable) {
        if {[lsearch $varList $var] == -1} {
          puts "Don't recognize variable '$var' in group '$usr(-group)', day '$usr(-day)'"
          puts "Valid variables for this group '$usr(-group)' are: 'all' and"
          puts "$varList\n"
          exit
        }
        lappend varList2 "$usr(-group)-$var"
      }
    }
    foreach var $varList2 {
      set status [lindex $ray(guidExprVar,$var) 0]
      set sec0 $usr(-group)
      set var0 [lindex [split $var -] 1]
      if {[info exists ray(renameVar,$var0)]} {
        set var0 $ray(renameVar,$var0)
      }
      set local [lindex $ray(guidExprVar,$var) 2]
      set extra [split [file rootname [file tail $local]] _]
      if {[llength $extra] > 1} {
        set var1 "$var0\_[join [lrange $extra 1 end] _]"
        set var2 [join [lrange $extra 1 end] _]
      } else {
        set var1 $var0
        set var2 ""
      }
      if {$usr(-renameConv) == 0} {
        set localName [file join $usr(-renameRoot) $local]
      } elseif {$usr(-renameConv) == 1} {
        set localName [file join $usr(-renameRoot) $sec0 TIME.$var1.$sec0]
      } elseif {$usr(-renameConv) == 2} {
        set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var1.$sec0]
      } elseif {$usr(-renameConv) == 3} {
        if {$var2 == ""} {
           set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.bin]
        } else {
           set localName [file join $usr(-renameRoot) $sec0 $var0 TIME.$var2.bin]
        }
      } elseif {$usr(-renameConv) == 4} {
        set localName [file join $usr(-renameRoot) $var1.$sec0]
      } elseif {$usr(-renameConv) == 5} {
        set localName [file join $usr(-renameRoot) [file tail [file dirname $local]] [string toupper [file dirname [file dirname $local]]] [lindex [split [file rootname [file tail $local]] _] 0] TIME.[file tail $local]]
      }

      set srvName "$Server1$guidExprDir[lindex $ray(guidExprVar,$var) 3]"
      if {$status == "join4"} {
        set tempList [split $srvName /]
        set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
        set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
        set srvSplitName4 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
        lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3 $srvSplitName4]] $localName]
      } elseif {$status == "join3"} {
        set tempList [split $srvName /]
        set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
        set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
        lappend tryList [list [list [list $srvSplitName1 $srvSplitName2 $srvSplitName3]] $localName]
      } elseif {$status == "join_d12"} {
        set tempList [split $srvName /]
        set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001] /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
        lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
      } elseif {$status == "join_d23"} {
        set tempList [split $srvName /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.002] /]
        set srvSplitName3 [join [linsert $tempList [expr [llength $tempList] -1] VP.003] /]
        lappend tryList [list [list [list $srvSplitName2 $srvSplitName3]] $localName]
      } elseif {$status == "join"} {
        set tempList [split $srvName /]
        set srvSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
        set srvSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
        lappend tryList [list [list [list $srvSplitName1 $srvSplitName2]] $localName]
      } elseif {$status == "pure"} {
        lappend tryList [list $srvName $localName]
      } elseif {$status == "NA"} {
        continue
      } else {
        puts "Don't understand status of [lindex $ray(foreVar,$var) 0]"
        continue
      }

      if {! [file exists [file dirname $localName]]} {
        file mkdir [file dirname $localName]
      }
    }

  } else {
    puts "Don't recognize dataSet $usr(-dataSet)"
  }

  WebHandleTryList $rayName $usrName $tryList 3 1
}

proc Usage {argv0} {
  set optList [list -dataSet -sector -group -variable -day -renameConv -renameRoot -quiet]
  set date [clock format [clock seconds] -format "%Y%m%d"]
  set optDes [list \
     "Data set to use (ndfd, ndfdExpr, ndgd or ndgdExpr)" \
     "sector to use (-dataSet ndfd):\n\
      \t\tconus, hawaii, puertori, guam, alaska, nhemi, npacocn, custom,\n\
      \t\tpacnwest, pacswest, nrockies, crrocks, srockies, nplains,\n\
      \t\tcrplains, splains, umissvly, crmissvy, smissvly, crgrlake,\n\
      \t\tergrlake, neast, seast, midatlan" \
     "group to use (-dataSet ndgd): aq, mosgfs, lampgfs" \
     "Variable: all, maxt-d13, maxt-d47, maxt" \
     "Days to download (d17, d13, d47, all) (Default d17)" \
     "Version of rename convention (0, 1, 2, 3) (Default 0) Example:\n\
      \t\t0 => -renameRoot/conus/maxt_d47.bin\n\
      \t\t1 => -renameRoot/co/$date.mx_d47.co\n\
      \t\t2 => -renameRoot/co/mx/$date.mx_d47.co\n\
      \t\t3 => -renameRoot/co/mx/$date.d47.bin\n\
      \t\t4 => -renameRoot/mx_d47.co"\
     "Rename Root directory (defaults to degrib/data/ndfd or\n\
      \t\tdegrib/data/ndgd depending on -dataSet)"\
     "Level of discourse with user (0 lot, 1 some, 2 fewer)"]

  puts "$argv0 \[OPTION\]..."
  puts "\nOptions:"
  for {set i 0} {$i < [llength $optList]} {incr i} {
    puts "[format "%13s" [lindex $optList $i]] = [lindex $optDes $i]"
  }
  puts "\nSimplest way to run requires: -dataSet, -sector or -group, and -variable"
  puts "Examples:"
  puts "\t$argv0 -dataSet ndfd -sector conus -variable maxt,mint"
  puts "\t$argv0 -dataSet ndgd -group aq -variable ozone01o"
  puts "\t$argv0 -dataSet ndgd -group mosgfs -variable maxt-d13,mint-d47"
  puts ""
}

#*****************************************************************************
#  Download_Select -- Arthur Taylor / MDL
#
# PURPOSE
#    Goes through the selected window, and determines which files the user
#  has selected for download.
#
# ARGUMENTS
#   rayName = Global array (structure) containing the main variables for
#             this program.
#   usrName = Array of user options.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#   Could probably clean this up as it has a lot of repetitive tasks.
#*****************************************************************************

#*****************************************************************************
# Finished program specific procedures, starting program
#*****************************************************************************

##### Parse the user's input. #####
set optList [list -dataSet -sector -group -variable -day -renameConv -renameRoot -quiet]
#Set Defaults:
set usr(-renameConv) 0
set usr(-day) d17
set usr(-renameRoot) -1
set usr(-quiet) 0

if {[lsearch $argv "-help"] != -1} {
  Usage $argv0
  exit
}
foreach {opt val} $argv {
  if {[lsearch $optList $opt] == -1} {
    Usage $argv0
    puts "Had problems with '$opt' in: '$argv0 $argv'\n"
    exit
  }
  set usr($opt) $val
}
set valDataSet [list "ndfd" "ndgd" "ndfdExpr" "ndgdExpr"]
if {(! [info exists usr(-dataSet)]) ||
    ([lsearch $valDataSet $usr(-dataSet)] == -1)} {
  Usage $argv0
  puts "Please specify -dataSet with either ndfd, ndfdExpr, ndgd or ndgdExpr\n"
  exit
}
if {![info exists usr(-variable)]} {
  Usage $argv0
  puts "Please specify -variable\n"
  exit
} elseif {$usr(-variable) != "all"} {
  set usr(-variable) [split $usr(-variable) ,]
}
if {($usr(-dataSet) == "ndfd") || ($usr(-dataSet) == "ndfdExpr")} {
  set valSector [list "conus" "hawaii" "puertori" "guam" "alaska" "nhemi" "npacocn"]
  set shortSector [list conus hi pr gu ak nhemi npacocn]
  set valSubSector [list "pacnwest" "pacswest" "nrockies" "crrocks" "srockies" "nplains" \
                 "crplains" "splains" "umissvly" "crmissvy" "smissvly" "crgrlake" \
                 "ergrlake" "neast" "seast" "midatlan"]
  if {(! [info exists usr(-sector)]) ||
      (([lsearch $valSector $usr(-sector)] == -1) &&
       ([lsearch $valSubSector $usr(-sector)] == -1) &&
       ($usr(-sector) != "custom"))} {
    Usage $argv0
    puts "NDFD requires a -sector from: $valSector custom $valSubSector\n"
    exit
  }
  set index [lsearch $valSector $usr(-sector)]
  set usr(subSector) 0
  set usr(custom) 0
  if {$index != -1} {
    set usr(shortSector) [lindex $shortSector $index]
  } elseif {$usr(-sector) == "custom"} {
    set usr(custom) 1
  } else {
    set usr(subSector) 1
  }
} elseif {$usr(-dataSet) == "ndgd"} {
  set valGroup [list "aq" "mosgfs" "lampgfs" "rtma00" "rtma01" "rtma02" "rtma03" "rtma04" "rtma05" "rtma06" "rtma07" "rtma08" "rtma09" "rtma10" "rtma11" "rtma11" "rtma12" "rtma13" "rtma14" "rtma15" "rtma16" "rtma17" "rtma18" "rtma19" "rtma20" "rtma21" "rtma22" "rtma23"]
  if {(! [info exists usr(-group)]) ||
      ([lsearch $valGroup $usr(-group)] == -1)} {
    Usage $argv0
    puts "NDGD requires a group from: $valGroup\n"
    exit
  }
}

##### Get program configuration settings. #####
ReadIni ray

if {$usr(-renameRoot) == -1} {
  if {$usr(-dataSet) == "ndfd"} {
    set usr(-renameRoot) $ray(dir,NDFD_OpnlData)
  } elseif {$usr(-dataSet) == "ndfdExpr"} {
    set usr(-renameRoot) $ray(dir,NDFD_ExprData)
  } elseif {$usr(-dataSet) == "ndgd"} {
    set usr(-renameRoot) $ray(dir,NDGD_OpnlData)
  } elseif {$usr(-dataSet) == "ndgdExpr"} {
    set usr(-renameRoot) $ray(dir,NDGD_ExprData)
  } else {
    puts "Don't recognise the -dataSet $usr(-dataSet)"
  }
}
if {! [file exists $usr(-renameRoot)]} {
  file mkdir $usr(-renameRoot)
}

if {[info exists usr(-day)]} {
  set valDay [list d13 d47 d17 all]
  if {[lsearch $valDay $usr(-day)] == -1} {
    Usage $argv0
    puts "-day option requires an element in: $valDay\n"
    exit
  }
}

##### Set Up rename conventions. #####
# What about ots02, pts02, pts03 pts06 pts12 ozone01e ozone08e smokec01 smokes01
# tcwspdabv64c tcwspdabv64i tcwspdabv50c tcwspdabv50i tcwspdabv34c tcwspdabv34i
foreach {var1 var2} [list "maxt" mx "mint" mn "pop12" po "qpf" qp "sky" cl "snow" sn \
               "td" dp "temp" tt "waveh" wh "wdir" wd "wspd" ws "wx" wx "apt" at \
               "rhm" rh "pop06" p6 "ozone01o" z1 "ozone08o" z8 "wgust" wg] {
  set ray(renameVar,$var1) $var2
}
if {[file exists [file join $src_dir rename.txt]]} {
  set fp [open [file join $src_dir rename.txt] r]
  while {[gets $fp line] >= 0} {
    set line [string trim $line]
    if {[string index $line 0] != "#"} {
      set line [split $line "="]
      if {[llength $line] == 2} {
        set ray(renameVar,[lindex $line 0]) [lindex $line 1]
      }
    }
  }
  close $fp
} 

##### Try to load the grib package.  #####
if {[catch {package require grib2}]} {
  puts "Fatal Error: Couldn't load the grib2 package."
  exit
}
Grib2Init GRIB2
set ray(GRIB2Cmd) GRIB2

Download_Select ray usr

##### The following forces Tcl/Tk to free the GRIB2 commands. #####
proc GRIB2 {} {}

exit
