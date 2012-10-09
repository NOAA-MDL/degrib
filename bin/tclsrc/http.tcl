package require http 2.0

#*****************************************************************************
# Procedure http::copy
#  (helper: http::Progress)
#
# Purpose: Teaches http how to copy a URL to a file, and print meta-data.
#   (see "my_http.tcl")
#
# Variables:
#   url        : Web site to copy from
#   file       : File to copy to
#   f_progress : Flag to say whether we should output a progress bar.
#   chunk      : Amount to copy at one time.
#                (8192 is default for geturl.)
#
# Returns: NULL
#*****************************************************************************
namespace eval http {
  proc SetProxy {proxy port} {
    if {($proxy != "") && ($port != "")} {
      http::config -proxyhost $proxy -proxyport $port
    } else {
      http::config -proxyhost "" -proxyport ""
    }
  }
  proc copy { url file {f_progress 0} {chunk 8192}} {
    set out [open $file w]
    if {$f_progress} {
      # -timeout 16000 -> 64000
      if {[catch {geturl $url -channel $out -progress ::http::Progress \
                 -blocksize $chunk -protocol 1.0} token ]} {
        ns_Print::puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
    # This ends the line started by http::Progress
      ns_Print::puts ""
    } else {
      # -timeout 16000 -> 64000
      if {[catch {geturl $url -channel $out \
                 -blocksize $chunk -protocol 1.0} token]} {
        ns_Print::puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
    }
    close $out
    if {[http::status $token] == "timeout"} {
      ns_Print::puts "timeout: $file"
      return 1
    }
    return 0
  }
  proc myAppend { url file {f_progress 0} {chunk 8192}} {
    set out [open $file a]
    if {$f_progress} {
      # -timeout 16000 -> 64000
      if {[catch {geturl $url -channel $out -progress ::http::Progress \
                 -blocksize $chunk -protocol 1.0} token]} {
        ns_Print::puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
    # This ends the line started by http::Progress
      ns_Print::puts ""
    } else {
      # -timeout 16000 -> 64000
      if {[catch {geturl $url -channel $out \
                 -blocksize $chunk -protocol 1.0} token]} {
        ns_Print::puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
    }
    close $out
    if {[http::status $token] == "timeout"} {
      ns_Print::puts "timeout: $file"
      return 1
    }
    return 0
  }

  proc Progress {args} {
    ns_Print::puts "." 0
  }
}

#*****************************************************************************
# GoodFlagField() --
#
# PURPOSE
#    After downloading an NDFD WMO GRIB2 file, checks the flag field in the
# first 19 bytes, and compares that with the acutal file size, to try to make
# sure that the file is not corrupt.
#    This only works for NDFD files that have a flag field... If it is a
# custom designed message, this information is not available (would have to
# add up all the individual GRIB2 messages.. Slower than flag field check,
# but might be good to do.)
#
# ARGUMENTS
# file = The file to check (Input)
#
# RETURNS int
#   0 = bad File
#   1 = good file.
#
# HISTORY
#  9/2003 Arthur Taylor (MDL/RSIS): Created.
#
# NOTES
#*****************************************************************************
proc GoodFlagField {file} {
  set fp [open $file r]
  gets $fp line
  close $fp
  if {[file size $file] != \
      [expr [string trimleft [string range $line 4 13] 0] + 19]} {
    return 0
  } else {
    return 1
  }
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
proc HttpTryOne {rayName src0 dst} {
  upvar #0 $rayName ray

  http::SetProxy $ray(http,Proxy) $ray(http,ProxyPort)
  if {[llength $src0] == 1} {
    ns_Print::puts "Copy $src0 \nTo $dst"
    if {[http::copy $src0 $dst 1 20480] != 0} {
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
        ns_Print::puts "Copy $elem \nTo $dst"
        if {[http::copy $elem $dst 1 20480] != 0} {
          return 1
        }
        set f_first 0
      } else {
        ns_Print::puts "Append $elem \nTo $dst"
        if {[http::myAppend $elem $dst 1 20480] != 0} {
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
#  FtpTryOne -- Arthur Taylor / MDL
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
proc FtpTryOne {rayName src0 dst} {
  upvar #0 $rayName ray

  if {[llength $src0] == 1} {
    ns_Print::puts "Copy $src0 \nTo $dst"
    if {[FTP::Get $src0 $dst] != 1} {
      return 1
    } else {
      ##### Check if it is a GRIB file, #####
      if {! [IsGRIB2 $rayName $dst -1]} {
        return 1
      }
    }
  } else {
    ns_Print::puts "Don't know how to append while ftp'ing (yet) (Try http)"
    return 1
  }
  return 0
}

#*****************************************************************************
#  HandleTryList -- Arthur Taylor / MDL
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
#   percent = current percent of the way complete
#     fract = fraction to add each time a file is downloaded.
#    f_http = 1 if we want to get data via http.  0 if we want data via ftp.
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
proc HandleTryList {rayName tryList attempt percent fract f_http f_first} {
  upvar #0 $rayName ray

  if {$f_first} {
    if {$f_http == 0} {
      set FTP::VERBOSE 0
      if {! [FTP::Open $ray(ftp,Server) $ray(ftp,User) $ray(ftp,Pass)]} {
        ns_Print::puts "Unable to open connection to $ray(ftp,Server)."
        return 1
      }
      ns_Print::puts "Connected to $ray(ftp,Server)."
      FTP::Type bin
    }
  }
  for {set i 0} {$i < [expr $attempt - 1]} {incr i} {
    set tryList2 ""
    foreach elem $tryList {
      set dst [lindex $elem 1]
      set src [lindex $elem 0]
      set src0 [lindex $src 0]

#      foreach elem $src0 {
#        ns_Print::puts "Attempting to get $elem"
#      }
#      ns_Print::puts "Placing result in $dst"
#      ns_Print::puts ""

      if {! [file exists [file dirname $dst]]} {
        file mkdir [file dirname $dst]
      }
      if {$f_http == 1} {
        set f_fail [HttpTryOne $rayName $src0 $dst]
      } else {
        set f_fail [FtpTryOne $rayName $src0 $dst]
      }
      if {$ray(Cancel)} {
        CancelAction $rayName 2
        if {$f_http == 0} {
          FTP::Close
        }
        return 1
      }
      if {$f_fail} {
        ns_Print::puts "  Couldn't 'get' $dst... Will ReTry later."
#        foreach elem $src0 {
#          ns_Print::puts "  Couldn't 'get' $elem... Will ReTry later."
#        }
        file delete -force $dst
        lappend tryList2 [list $src $dst]
      } else {
        ns_Print::puts "Got $dst"
#        foreach elem $src0 {
#          ns_Print::puts "Got $elem"
#        }
        set percent [expr $percent + $fract]
        ns_Util::DrawXPerc $ray(DoneCanv) $percent
      }
    }
    if {$tryList2 == ""} {
      if {$f_http == 0} {
        FTP::Close
      }
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

#    foreach elem $src0 {
#      ns_Print::puts "Attempting to get $elem"
#    }
#    ns_Print::puts "Placing result in $dst"
#    ns_Print::puts ""

    if {! [file exists [file dirname $dst]]} {
      file mkdir [file dirname $dst]
    }
    if {$f_http == 1} {
      set f_fail [HttpTryOne $rayName $src0 $dst]
    } else {
      set f_fail [FtpTryOne $rayName $src0 $dst]
    }
    if {$f_fail} {
      ns_Print::puts "  Couldn't 'get' $dst"
#      foreach elem $src0 {
#        ns_Print::puts "  Couldn't 'get' $elem"
#      }
      file delete -force $dst
      if {[llength $src] > 1} {
        ns_Print::puts "  Trying alternate URL."
        lappend tryList2 [list [lrange $src 1 end] $dst]
      }
    } else {
       ns_Print::puts "Got $dst"
#      foreach elem $src0 {
#        ns_Print::puts "Got $elem"
#      }
      set precent [expr $percent + $fract]
      ns_Util::DrawXPerc $ray(DoneCanv) $percent
    }
  }

  if {$tryList2 != ""} {
    HandleTryList $rayName $tryList2 $attempt $percent $fract $f_http 0
    return 0
  }
  if {$f_http == 0} {
    FTP::Close
  }
  return 0
}

#*****************************************************************************
#  HttpDownload_Select -- Arthur Taylor / MDL
#
# PURPOSE
#    Goes through the selected window, and determines which files the user
#  has selected for download.
#
# ARGUMENTS
#   rayName = Global array (structure) containing the main variables for
#             this program.
#    f_http = 1 if we want to get data via http.  0 if we want data via ftp.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#   Could probably clean this up as it has a lot of repetitive tasks.
#*****************************************************************************
proc HttpDownload_Select {rayName f_http} {
  upvar #0 $rayName ray
#  set TIME [clock clicks -milliseconds]

  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  } else {
    set ray(Action) Download
  }

  set nodeList [$ray(lb,tree) selection get]
  set tryList ""
  set guidList ""
  set foreList ""
  set subList ""
  set customList ""
  set guidExprList ""
  set foreExprList ""
  set subExprList ""

  foreach node $nodeList {
    set nList [split $node _]
    set database [lindex $nList 0]
    set var [lindex $nList [expr [llength $nList] - 1]]
    ##### Update the guidList based on selected NDGD data. #####
    if {$database == "ndgd"} {
      if {! [info exists ray(guidVar,$var)]} {
        # Why would we get here?
        #   Answer: User selected a folder rather than a file
        #
        if {[info exists ray(guidFolder,fileContent,$node)]} {
          foreach node2 $ray(guidFolder,fileContent,$node) {
            set nList2 [split $node2 _]
            lappend guidList [lindex $nList2 [expr [llength $nList2] - 1]]
          }
        } else {
          puts "$var does not exist"
        }
      } else {
        lappend guidList $var
      }

    ##### Update the guidList based on selected NDGD data. #####
    } elseif {$database == "ndgdExpr"} {
      if {! [info exists ray(guidExprVar,$var)]} {
        # Why would we get here?
        #   Answer: User selected a folder rather than a file
        #
        if {[info exists ray(guidExprFolder,fileContent,$node)]} {
          foreach node2 $ray(guidExprFolder,fileContent,$node) {
            set nList2 [split $node2 _]
            lappend guidExprList [lindex $nList2 [expr [llength $nList2] - 1]]
          }
        } else {
          puts "$var does not exist"
        }
      } else {
        lappend guidExprList $var
      }

    ##### Update the foreList and subList based on selected NDFD data. #####
    } elseif {$database == "ndfd"} {
      set tempList [split $var -]
      set f_subsector 0
      set f_custom 0
      if {[lindex $tempList 0] == "custom"} {
        set f_custom 1
      }
      if {[lindex $tempList 0] == "conus"} {
        set subsector [lindex $tempList 1]
        if {[lsearch $ray(subSector,LocalName) $subsector] != -1} {
          set f_subsector 1
        }
      }
      if {! $f_subsector} {
        if {! $f_custom} {
          if {! [info exists ray(foreVar,$var)]} {
            # Why would we get here?
            #   Answer: User selected a folder rather than a file
            #
            if {[info exists ray(foreFolder,fileContent,$node)]} {
              foreach node2 $ray(foreFolder,fileContent,$node) {
                set nList2 [split $node2 _]
                lappend foreList [lindex $nList2 [expr [llength $nList2] - 1]]
              }
            } elseif {[info exists ray(subFolder,fileContent,$node)]} {
              foreach node2 $ray(subFolder,fileContent,$node) {
                set nList2 [split $node2 _]
                lappend subList [lindex $nList2 [expr [llength $nList2] - 1]]
              }
            } elseif {[info exists ray(customFolder,fileContent,$node)]} {
              foreach node2 $ray(customFolder,fileContent,$node) {
                set nList2 [split $node2 _]
                lappend customList [lindex $nList2 [expr [llength $nList2] - 1]]
              }
            } else {
              puts "$var does not exist"
            }
          } else {
            lappend foreList $var
          }
        } else {
          lappend customList $var
        }
      } else {
        lappend subList $var
      }

    ##### Update the foreExprList and subExprList based on selected NDFD data. #####
    } elseif {$database == "ndfdExpr"} {
      set tempList [split $var -]
      set f_subsector 0
      set f_custom 0
      if {[lindex $tempList 0] == "custom"} {
        set f_custom 1
      }
      if {[lindex $tempList 0] == "conus"} {
        set subsector [lindex $tempList 1]
        if {[lsearch $ray(subExprSector,LocalName) $subsector] != -1} {
          set f_subsector 1
        }
      }
      if {! $f_subsector} {
        if {! $f_custom} {
          if {! [info exists ray(foreExprVar,$var)]} {
            # Why would we get here?
            #   Answer: User selected a folder rather than a file
            #
            if {[info exists ray(foreExprFolder,fileContent,$node)]} {
              foreach node2 $ray(foreExprFolder,fileContent,$node) {
                set nList2 [split $node2 _]
                lappend foreList [lindex $nList2 [expr [llength $nList2] - 1]]
              }
            } elseif {[info exists ray(subExprFolder,fileContent,$node)]} {
              foreach node2 $ray(subExprFolder,fileContent,$node) {
                set nList2 [split $node2 _]
                lappend subList [lindex $nList2 [expr [llength $nList2] - 1]]
              }
            } elseif {[info exists ray(customFolder,fileContent,$node)]} {
              foreach node2 $ray(customFolder,fileContent,$node) {
                set nList2 [split $node2 _]
                lappend customList [lindex $nList2 [expr [llength $nList2] - 1]]
              }
            } else {
              puts "$var does not exist"
            }
          } else {
            lappend foreExprList $var
          }
        } else {
          lappend customList $var
        }
      } else {
        lappend subExprList $var
      }
    }

  }
  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  }

##### Update the trylist based on the NDFD (forecast). #####
  if {$f_http == 1} {
    set Server1 $ray(http,Server)
    set Server2 $ray(http,Server)
    set foreExprDir $ray(http,foreExprDir)
    set foreOpnlDir $ray(http,foreOpnlDir)
    set guidExprDir $ray(http,guidExprDir)
    set guidOpnlDir $ray(http,guidOpnlDir)
  } else {
    set Server1 ""
    set Server2 $ray(ftp,Server)
    set foreExprDir $ray(ftp,foreExprDir)
    set foreOpnlDir $ray(ftp,foreOpnlDir)
    set guidExprDir $ray(ftp,guidExprDir)
    set guidOpnlDir $ray(ftp,guidOpnlDir)
  }

  foreach var $foreList {
    set status [lindex $ray(foreVar,$var) 0]
    set localName [file join $ray(dir,NDFD_OpnlData) [lindex $ray(foreVar,$var) 2]]
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
      tk_messageBox -message "Don't understand status of [lindex $ray(foreVar,$var) 0]"
      continue
    }
  }
  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  }

  foreach var $foreExprList {
    set status [lindex $ray(foreExprVar,$var) 0]
    set localName [file join $ray(dir,NDFD_ExprData) [lindex $ray(foreExprVar,$var) 2]]
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
      tk_messageBox -message "Don't understand status of [lindex $ray(foreExprVar,$var) 0]"
      continue
    }
  }
  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  }

##### Update the trylist based on the subsectors. #####
  foreach sub $subList {
    set tempList [split $sub -]
    set var [join [lreplace $tempList 1 1] -]
    set status [lindex $ray(foreVar,$var) 0]

    set sector [lindex $tempList 1]
    set local [string replace [lindex $ray(foreVar,$var) 2] 0 4 $sector]
    set localName [file join $ray(dir,NDFD_OpnlData) $local]

    set remote [lindex $ray(subSector,RemoteName) [lsearch $ray(subSector,LocalName) $sector]]
    set remotePath [string replace [lindex $ray(foreVar,$var) 3] 3 7 $remote]
    set srvName "$Server1$foreOpnlDir$remotePath"

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
      tk_messageBox -message "Don't understand status of [lindex $ray(foreVar,$var) 0]"
      continue
    }
  }
  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  }

##### Update the trylist based on the expr subsectors. #####
  foreach sub $subExprList {
    set tempList [split $sub -]
    set var [join [lreplace $tempList 1 1] -]
    set status [lindex $ray(foreExprVar,$var) 0]

    set sector [lindex $tempList 1]
    set local [string replace [lindex $ray(foreExprVar,$var) 2] 0 4 $sector]
    set localName [file join $ray(dir,NDFD_ExprData) $local]

    set remote [lindex $ray(subExprSector,RemoteName) [lsearch $ray(subExprSector,LocalName) $sector]]
    set remotePath [string replace [lindex $ray(foreExprVar,$var) 3] 3 7 $remote]
    set srvName "$Server1$foreExprDir$remotePath"

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
      tk_messageBox -message "Don't understand status of [lindex $ray(foreVar,$var) 0]"
      continue
    }
  }
  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  }

  foreach var $guidList {
    set status [lindex $ray(guidVar,$var) 0]
    set localName [file join $ray(dir,NDGD_OpnlData) [lindex $ray(guidVar,$var) 2]]
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
      tk_messageBox -message "Don't understand status of [lindex $ray(guidVar,$var) 0]"
      continue
    }
  }
  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  }

  foreach var $guidExprList {
    set status [lindex $ray(guidExprVar,$var) 0]
    set localName [file join $ray(dir,NDGD_ExprData) [lindex $ray(guidExprVar,$var) 2]]
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
      tk_messageBox -message "Don't understand status of [lindex $ray(guidExprVar,$var) 0]"
      continue
    }
  }
  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  }

##### Update the trylist based on the custom sector. #####
  set f_noMessage 0
  if {$f_http == 0} {
    if {[llength $customList] != 0} {
      ns_Print::puts "Can't ftp custom sector data."
      set f_noMessage 1
    }
  } else {
    foreach var $customList {
      set localName [file join $ray(dir,NDFD_OpnlData) [lindex $ray(custom,$var) 2]]
      set box [split $ray(custom,box) ,]
      set opnlName "$ray(custom,URL)?var=[lindex $ray(custom,$var) 3]&lat1=[lindex $box 0]&lon1=[lindex $box 1]&lat2=[lindex $box 2]&lon2=[lindex $box 3]"
      lappend tryList [list [list $opnlName] $localName]
    }
  }
  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  }

  if {[llength $tryList] == 0} {
    if {! $f_noMessage} {
      ns_Print::puts "Please highlight one or more files in the list"
      ns_Print::puts "Alternatively, highlight an entire folder to "
      ns_Print::puts "   have it download all files (not folders) in it"
    }
  } else {
    # Establish connection.
    set percent 5
    ns_Util::DrawXPerc $ray(DoneCanv) $percent
    ns_Print::puts "-----"
    ns_Print::puts "Starting downloads from $Server2"
    set percent 10
    ns_Util::DrawXPerc $ray(DoneCanv) $percent

    HandleTryList $rayName $tryList 3 $percent [expr 80.0 / ([llength $tryList] + 0.0)] $f_http 1
    RefreshTimes $rayName
    RefreshFolderList $rayName

    set percent 100
    ns_Util::DrawXPerc $ray(DoneCanv) $percent
  }

  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  } else {
    set ray(Action) None
  }
}
