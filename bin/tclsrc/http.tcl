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

proc NDFD_Download_Http {rayName} {
  upvar #0 $rayName ray
  global tcl_version

  if {$tcl_version < 8.3} {
    set TIME [clock clicks]
  } else {
    set TIME [clock clicks -milliseconds]
  }

  if {$ray(NDFD,Action) != "None"} {
    if {$ray(NDFD,Cancel) != 0} {
      ns_Print::puts "Please wait for the $ray(NDFD,Action) action to complete"
    }
    return
  }
  set ray(NDFD,Action) Download
  set ray(NDFD,Cancel) 0

#  set indexList [$ray(lb,sct) curselection]
  set nodeList [$ray(lb,tree) selection get]

  set tryList ""
  foreach node $nodeList {
    set nList [split $node _]
    set database [lindex $nList 0]
    if {$database == "ndgd2"} {
      set var [lindex $nList [expr [llength $nList] - 1]]
#      ns_Print::puts "$var"
      if {! [info exists ray(ndgd2_var,$var)]} {
#         set ndfd_NodeList [$ray(lb,tree) nodecget $var -children]
#         ns_Print::puts "$ndfd_NodeList"
         continue
      }
      if {([lindex $ray(ndgd2_var,$var) 0] == "*") || ([lindex $ray(ndgd2_var,$var) 0] == "expr")} {
        set srcName "$ray(http,Server)$ray(http,ndgd_ExprDir)[lindex $ray(ndgd2_var,$var) 3]"
        set localName [file join $ray(dir,NDGD_Data) [lindex $ray(ndgd2_var,$var) 2]]
        lappend tryList [list $srcName $localName]
      } elseif {[lindex $ray(ndgd2_var,$var) 0] == "opnl"} {
        set srcName "$ray(http,Server)$ray(http,ndgd_OpnlDir)[lindex $ray(ndgd2_var,$var) 3]"
        set localName [file join $ray(dir,NDGD_Data) [lindex $ray(ndgd2_var,$var) 2]]
        lappend tryList [list $srcName $localName]
      } else {
        tk_messageBox -message "Don't understand status of [lindex $ray(ndgd2_var,$var) 0]"
      }
      set dirname [file dirname $localName]
      if {! [file exists $dirname]} {
        file mkdir $dirname
      }
      continue
    }
    set sct [lindex $nList 1]
    if {[lindex $ray(sct,$sct) 1] == "*"} {
      # if == 2 then dealing with a whole sector.
      if {[llength $nList] == 2} {
        if {$database == "ndfd"} {
          if {$sct == "custom"} {
            if {([expr abs ($ray(custom,lat1) - $ray(custom,lat2))] > 20) ||
                ([expr abs ($ray(custom,lon1) - $ray(custom,lon2))] > 20)} {
              ns_Print::puts "currently 'custom' is restricted to a delta of 20 degrees lat/lon"
              ns_Print::puts "Your values were: "
              ns_Print::puts "Lat of: $ray(custom,lat1), $ray(custom,lat2)"
              ns_Print::puts "Lon of: $ray(custom,lon1), $ray(custom,lon2)"
            } else {
              foreach var $ray(varList) {
                set srcName "$ray(custom,URL)?var=$var&lat1=$ray(custom,lat1)&lon1=$ray(custom,lon1)&lat2=$ray(custom,lat2)&lon2=$ray(custom,lon2)"
                set localName [file join $ray(dir,NDFD_Data) $sct $var.bin]
                lappend tryList [list $srcName $localName]
              }
            }
          } else {
            foreach var $ray(varList) {
              if {$sct == "puertori"} {
                set status [lindex $ray(var,$var) 3]
              } elseif {$sct == "hawaii"} {
                set status [lindex $ray(var,$var) 4]
              } elseif {$sct == "guam"} {
                set status [lindex $ray(var,$var) 5]
              } elseif {$sct == "alaska"} {
                set status [lindex $ray(var,$var) 6]
              } elseif {$sct == "nhemi"} {
                set status [lindex $ray(var,$var) 7]
              } else {
                set status [lindex $ray(var,$var) 1]
              }
              if {$status == "expr"} {
                set srcName "$ray(http,Server)$ray(http,ExprDir)AR.$sct/ds.$var.bin"
                set localName [file join $ray(dir,NDFD_Data) $sct $var.bin]
                lappend tryList [list $srcName $localName]
              } elseif {$status == "opnl"} {
                set srcName "$ray(http,Server)$ray(http,OpnlDir)AR.$sct/ds.$var.bin"
                set localName [file join $ray(dir,NDFD_Data) $sct $var.bin]
                lappend tryList [list $srcName $localName]
              }
            }
          }
        } else {
#          foreach var $ray(ndgd_varList) {
#            if {[lindex $ray(ndgd_var,$var) 1] == "*"} {
#              set srcName "$ray(http,Server)$ray(http,ndgd_ExprDir)AR.$sct/ds.$var.bin"
#              set localName [file join $ray(dir,NDGD_Data) $sct $var.bin]
#              lappend tryList [list $srcName $localName]
#            } elseif {[lindex $ray(ndgd_var,$var) 1] == "opnl"} {
#              set srcName "$ray(http,Server)$ray(http,ndgd_OpnlDir)AR.$sct/ds.$var.bin"
#              set localName [file join $ray(dir,NDGD_Data) $sct $var.bin]
#              lappend tryList [list $srcName $localName]
#            }
#          }
        }
      } else {
        set sctNode [join [lrange $nList 0 1] _]
        if {[lsearch $nodeList $sctNode] == -1} {
          set var [lindex $nList 2]
          if {$database == "ndfd"} {
            if {$sct == "puertori"} {
              set status [lindex $ray(var,$var) 3]
            } elseif {$sct == "hawaii"} {
              set status [lindex $ray(var,$var) 4]
            } elseif {$sct == "guam"} {
              set status [lindex $ray(var,$var) 5]
            } elseif {$sct == "alaska"} {
              set status [lindex $ray(var,$var) 6]
            } elseif {$sct == "nhemi"} {
              set status [lindex $ray(var,$var) 7]
            } else {
              set status [lindex $ray(var,$var) 1]
            }
            if {$sct == "custom"} {
              if {([expr abs ($ray(custom,lat1) - $ray(custom,lat2))] > 20) ||
                  ([expr abs ($ray(custom,lon1) - $ray(custom,lon2))] > 20)} {
                ns_Print::puts "currently 'custom' is restricted to a delta of 20 degrees lat/lon"
                ns_Print::puts "Your values were: "
                ns_Print::puts "Lat of: $ray(custom,lat1), $ray(custom,lat2)"
                ns_Print::puts "Lon of: $ray(custom,lon1), $ray(custom,lon2)"
              } else {
                set srcName "$ray(custom,URL)?var=$var&lat1=$ray(custom,lat1)&lon1=$ray(custom,lon1)&lat2=$ray(custom,lat2)&lon2=$ray(custom,lon2)"
                set localName [file join $ray(dir,NDFD_Data) $sct $var.bin]
                lappend tryList [list $srcName $localName]
              }
            } elseif {$status == "expr"} {
              set srcName "$ray(http,Server)$ray(http,ExprDir)AR.$sct/ds.$var.bin"
              set localName [file join $ray(dir,NDFD_Data) $sct $var.bin]
              lappend tryList [list $srcName $localName]
            } elseif {$status == "opnl"} {
              set srcName "$ray(http,Server)$ray(http,OpnlDir)AR.$sct/ds.$var.bin"
              set localName [file join $ray(dir,NDFD_Data) $sct $var.bin]
              lappend tryList [list $srcName $localName]
            }
          } else {
            if {[lindex $ray(ndgd_var,$var) 1] == "*"} {
              set srcName "$ray(http,Server)$ray(http,ndgd_ExprDir)AR.$sct/ds.$var.bin"
              set localName [file join $ray(dir,NDGD_Data) $sct $var.bin]
              lappend tryList [list $srcName $localName]
            } elseif {[lindex $ray(ndgd_var,$var) 1] == "opnl"} {
              set srcName "$ray(http,Server)$ray(http,ndgd_OpnlDir)AR.$sct/ds.$var.bin"
              set localName [file join $ray(dir,NDGD_Data) $sct $var.bin]
              lappend tryList [list $srcName $localName]
            }
          }
        }
      }
    }
  }

  if {[llength $tryList] == 0} {
    ns_Print::puts "Please select one or more sectors or variables"
    ns_Print::puts "   Check if they have a N.A. next to them,"
    ns_Print::puts "   which means Not Available"
  } else {
    # Establish connection.
    set percent 5
    ns_Util::DrawXPerc $ray(DoneCanv) $percent
    ns_Print::puts "-----"
    ns_Print::puts "Starting downloads from $ray(http,Server)"
    set percent 10
    ns_Util::DrawXPerc $ray(DoneCanv) $percent

    set reTryList ""
    set totTry [llength $tryList]
    set tryCnt 0
    foreach pair $tryList {
      if {$ray(NDFD,Cancel)} {
        ns_Print::puts "Closed connection"
        NDFD_Cancel $rayName 2
        return
      }
      set srcName [lindex $pair 0]
      set localName [lindex $pair 1]
      set shrtName [file split $srcName]
      set shrtName [lrange $shrtName [expr [llength $shrtName] -5] end]
      set shrtName [join $shrtName /]
      ns_Print::puts "Attempting to get $shrtName"
      http::SetProxy $ray(http,Proxy) $ray(http,ProxyPort)
      if {[http::copy $srcName $localName 1 20480] != 0} {
        ns_Print::puts "  Couldn't 'get' $shrtName... Will ReTry later."
        lappend reTryList $pair
        file delete -force $localName
      } else {
        # Check if it is a GRIB file, or a message saying it isn't on the
        # server.
        if {! [IsGRIB2 $rayName $localName -1]} {
          ns_Print::puts "  Couldn't 'get' $shrtName... Will ReTry later."
          lappend reTryList $pair
          file delete -force $localName
        } else {
          set f_gotIt 1
          # flag field separators are not in mosGFS or in custom
          # They are in aq grids, but I ignore them.
          if {$database == "ndfd"} {
            if {$sct != "custom"} {
              set f_gotIt [GoodFlagField $localName]
            }
          }
          if {! $f_gotIt} {
            ns_Print::puts "Incomplete $shrtName ... Will try again."
            lappend reTryList $pair
            file delete -force $localName
          } else {
            ns_Print::puts "Got $shrtName"
          }
        }
      }
      incr tryCnt
      set percent [expr (($tryCnt * 100) / ($totTry + 0.0))]
      set percent [expr $percent * .9 + 10]
      ns_Util::DrawXPerc $ray(DoneCanv) $percent
    }

    set reReTryList ""
    if {[llength $reTryList] != 0} {
      foreach pair $reTryList {
        if {$ray(NDFD,Cancel)} {
          ns_Print::puts "Closed connection"
          NDFD_Cancel $rayName 2
          return
        }
        set srcName [lindex $pair 0]
        set localName [lindex $pair 1]
        set shrtName [file split $srcName]
        set shrtName [lrange $shrtName [expr [llength $shrtName] -5] end]
        set shrtName [join $shrtName /]
        ns_Print::puts "Attempting to get $shrtName"
        http::SetProxy $ray(http,Proxy) $ray(http,ProxyPort)
        if {[http::copy $srcName $localName 1 20480] != 0} {
          ns_Print::puts "  Couldn't 'get' $shrtName... Will Re Try again."
          file delete -force $localName
          lappend reReTryList $pair
        } else {
          # Check if it is a GRIB file, or a message saying it isn't on the
          # server.
          if {! [IsGRIB2 $rayName $localName -1]} {
            ns_Print::puts "  Couldn't 'get' $shrtName... Will Re Try again."
            file delete -force $localName
            lappend reReTryList $pair
          } else {
            set f_gotIt 1
            if {$sct != "custom"} {
              set f_gotIt [GoodFlagField $localName]
            }
            if {! $f_gotIt} {
              ns_Print::puts "Incomplete $shrtName ... Skipping."
              file delete -force $localName
            } else {
              ns_Print::puts "Got $shrtName"
            }
          }
        }
      }
    }

    if {[llength $reReTryList] != 0} {
      foreach pair $reReTryList {
        if {$ray(NDFD,Cancel)} {
          ns_Print::puts "Closed connection"
          NDFD_Cancel $rayName 2
          return
        }
        set srcName [lindex $pair 0]

        set srcName [split $srcName /]
        set index [expr [llength $srcName] - 5]
        if {[lindex $srcName $index] == "ST.expr"} {
          set srcName [join [lreplace $srcName $index $index ST.opnl] /]
        } elseif {[lindex $srcName [expr [llength $srcName] - 5]] == "ST.opnl"} {
          set srcName [join [lreplace $srcName $index $index ST.expr] /]
        }

        set localName [lindex $pair 1]
        set shrtName [file split $srcName]
        set shrtName [lrange $shrtName [expr [llength $shrtName] -5] end]
        set shrtName [join $shrtName /]
        ns_Print::puts "Attempting to get $shrtName"
        http::SetProxy $ray(http,Proxy) $ray(http,ProxyPort)
        if {[http::copy $srcName $localName 1 20480] != 0} {
          ns_Print::puts "  Couldn't 'get' $shrtName... Skipping."
          file delete -force $localName
        } else {
          # Check if it is a GRIB file, or a message saying it isn't on the
          # server.
          if {! [IsGRIB2 $rayName $localName -1]} {
            ns_Print::puts "  Couldn't 'get' $shrtName... Skipping."
            file delete -force $localName
          } else {
            set f_gotIt 1
            if {$sct != "custom"} {
              set f_gotIt [GoodFlagField $localName]
            }
            if {! $f_gotIt} {
              ns_Print::puts "Incomplete $shrtName ... Skipping."
              file delete -force $localName
            } else {
              ns_Print::puts "Got $shrtName"
            }
          }
        }
      }
    }
    ns_Print::puts "Finished with downloads"

    if {$tcl_version < 8.3} {
      ns_Print::puts "Time elapsed [expr [clock clicks] - $TIME] units"
    } else {
      ns_Print::puts "Time elapsed [expr [clock clicks -milliseconds] - $TIME] milliseconds"
    }
  }
  set ray(NDFD,Action) None
  MainRefresh $rayName
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
    if {[http::copy $src0 $dst 1 20480] != 0} {
      return 1
    } else {
      ##### Check if it is a GRIB file, #####
      if {! [IsGRIB2 $rayName $dst -1]} {
        return 1
      }
    }
  } elseif {[llength $src0] == 2} {
    if {[http::copy [lindex $src0 0] $dst 1 20480] != 0} {
      return 1
    } else {
      ##### Check if it is a GRIB file, #####
      if {! [IsGRIB2 $rayName $dst -1]} {
        return 1
      } else {
        if {[http::myAppend [lindex $src0 1] $dst 1 20480] != 0} {
          return 1
        } else {
          if {! [IsGRIB2 $rayName $dst -1]} {
            return 1
          }
        }
      }
    }
  } else {
    return 1
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
    if {[FTP::Get $src0 $dst] != 1} {
      return 1
    } else {
      ##### Check if it is a GRIB file, #####
      if {! [IsGRIB2 $rayName $dst -1]} {
        return 1
      }
    }
  } elseif {[llength $src0] == 2} {
    ns_Print::puts "Don't know how to append while ftp'ing (yet) (Try http)"
    return 1
  } else {
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
      set shrtName [file split $src0]
      set shrtName [join [lrange $shrtName [expr [llength $shrtName] -5] end] /]
      ns_Print::puts "Attempting to get $shrtName"
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
        ns_Print::puts "  Couldn't 'get' $shrtName... Will ReTry later."
        file delete -force $dst
        lappend tryList2 [list $src $dst]
      } else {
        ns_Print::puts "Got $shrtName"
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
    set shrtName [file split $src0]
    set shrtName [join [lrange $shrtName [expr [llength $shrtName] -5] end] /]
    ns_Print::puts "Attempting to get $shrtName"
    if {! [file exists [file dirname $dst]]} {
      file mkdir [file dirname $dst]
    }
    if {$f_http == 1} {
      set f_fail [HttpTryOne $rayName $src0 $dst]
    } else {
      set f_fail [FtpTryOne $rayName $src0 $dst]
    }
    if {$f_fail} {
      if {[llength $src] > 1} {
        ns_Print::puts "  Couldn't 'get' $shrtName... Try alternate URL."
        lappend tryList2 [list [lrange $src 1 end] $dst]
      }
    } else {
      ns_Print::puts "Got $shrtName"
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
    set localName [file join $ray(dir,NDFD_Data) [lindex $ray(foreVar,$var) 2]]
    set exprName "$Server1$foreExprDir[lindex $ray(foreVar,$var) 3]"
    set opnlName "$Server1$foreOpnlDir[lindex $ray(foreVar,$var) 3]"
    if {$status == "expr"} {
      set tempList [split $exprName /]
      set exprSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set exprSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
      set tempList [split $opnlName /]
      set opnlSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set opnlSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
#      lappend tryList [list [list $exprName $opnlName [list $exprSplitName1 $exprSplitName2] [list $opnlSplitName1 $opnlSplitName2]] $localName]
      lappend tryList [list [list [list $exprSplitName1 $exprSplitName2] [list $opnlSplitName1 $opnlSplitName2]] $localName]
    } elseif {$status == "expr-split"} {
      lappend tryList [list [list $exprName $opnlName] $localName]
    } elseif {$status == "expr-pure"} {
      lappend tryList [list [list $exprName] $localName]
    } elseif {$status == "opnl"} {
      set tempList [split $exprName /]
      set exprSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set exprSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
      set tempList [split $opnlName /]
      set opnlSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set opnlSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
#      lappend tryList [list [list $opnlName $exprName [list $opnlSplitName1 $opnlSplitName2] [list $exprSplitName1 $exprSplitName2]] $localName]
      lappend tryList [list [list [list $opnlSplitName1 $opnlSplitName2] [list $exprSplitName1 $exprSplitName2]] $localName]
    } elseif {$status == "opnl-split"} {
      lappend tryList [list [list $opnlName $exprName] $localName]
    } elseif {$status == "opnl-pure"} {
      lappend tryList [list [list $opnlName] $localName]
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

##### Update the trylist based on the NDGD (guidance). #####
  foreach var $guidList {
    set status [lindex $ray(guidVar,$var) 0]
    set localName [file join $ray(dir,NDGD_Data) [lindex $ray(guidVar,$var) 2]]
    set exprName "$Server1$guidExprDir[lindex $ray(guidVar,$var) 3]"
    set opnlName "$Server1$guidOpnlDir[lindex $ray(guidVar,$var) 3]"
    if {$status == "expr"} {
      set tempList [split $exprName /]
      set exprSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set exprSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
      set tempList [split $opnlName /]
      set opnlSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set opnlSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
#      lappend tryList [list [list $exprName $opnlName [list $exprSplitName1 $exprSplitName2] [list $opnlSplitName1 $opnlSplitName2]] $localName]
      lappend tryList [list [list [list $exprSplitName1 $exprSplitName2] [list $opnlSplitName1 $opnlSplitName2]] $localName]
    } elseif {$status == "expr-split"} {
      lappend tryList [list [list $exprName $opnlName] $localName]
    } elseif {$status == "expr-pure"} {
      lappend tryList [list [list $exprName] $localName]
    } elseif {$status == "opnl"} {
      set tempList [split $exprName /]
      set exprSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set exprSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
      set tempList [split $opnlName /]
      set opnlSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set opnlSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
#      lappend tryList [list [list $opnlName $exprName [list $opnlSplitName1 $opnlSplitName2] [list $exprSplitName1 $exprSplitName2]] $localName]
      lappend tryList [list [list [list $opnlSplitName1 $opnlSplitName2] [list $exprSplitName1 $exprSplitName2]] $localName]
    } elseif {$status == "opnl-split"} {
      lappend tryList [list [list $opnlName $exprName] $localName]
    } elseif {$status == "opnl-pure"} {
      lappend tryList [list [list $opnlName] $localName]
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

##### Update the trylist based on the subsectors. #####
  foreach sub $subList {
    set tempList [split $sub -]
    set var [join [lreplace $tempList 1 1] -]
    set sector [lindex $tempList 1]
    set index [lsearch $ray(subSector,LocalName) $sector]
    set remote [lindex $ray(subSector,RemoteName) $index]
    set status [lindex $ray(foreVar,$var) 0]
    set local [string replace [lindex $ray(foreVar,$var) 2] 0 4 $sector]
    set localName [file join $ray(dir,NDFD_Data) $local]
    set remotePath [string replace [lindex $ray(foreVar,$var) 3] 3 7 $remote]
    set exprName "$Server1$foreExprDir$remotePath"
    set opnlName "$Server1$foreOpnlDir$remotePath"
    if {$status == "expr"} {
      set tempList [split $exprName /]
      set exprSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set exprSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
      set tempList [split $opnlName /]
      set opnlSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set opnlSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
#      lappend tryList [list [list $exprName $opnlName [list $exprSplitName1 $exprSplitName2] [list $opnlSplitName1 $opnlSplitName2]] $localName]
      lappend tryList [list [list [list $exprSplitName1 $exprSplitName2] [list $opnlSplitName1 $opnlSplitName2]] $localName]
    } elseif {$status == "expr-split"} {
      lappend tryList [list [list $exprName $opnlName] $localName]
    } elseif {$status == "expr-pure"} {
      lappend tryList [list [list $exprName] $localName]
    } elseif {$status == "opnl"} {
      set tempList [split $exprName /]
      set exprSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set exprSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
      set tempList [split $opnlName /]
      set opnlSplitName1 [join [linsert $tempList [expr [llength $tempList] -1] VP.001-003] /]
      set opnlSplitName2 [join [linsert $tempList [expr [llength $tempList] -1] VP.004-007] /]
#      lappend tryList [list [list $opnlName $exprName [list $opnlSplitName1 $opnlSplitName2] [list $exprSplitName1 $exprSplitName2]] $localName]
      lappend tryList [list [list [list $opnlSplitName1 $opnlSplitName2] [list $exprSplitName1 $exprSplitName2]] $localName]
    } elseif {$status == "opnl-split"} {
      lappend tryList [list [list $opnlName $exprName] $localName]
    } elseif {$status == "opnl-pure"} {
      lappend tryList [list [list $opnlName] $localName]
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

##### Update the trylist based on the custom sector. #####
  set f_noMessage 0
  if {$f_http == 0} {
    if {[llength $customList] != 0} {
      ns_Print::puts "Can't ftp custom sector data."
      set f_noMessage 1
    }
  } else {
    foreach var $customList {
      set localName [file join $ray(dir,NDFD_Data) [lindex $ray(custom,$var) 2]]
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
