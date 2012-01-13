#!/bin/sh
# the next line restarts \
exec /usr/bin/tclsh "$0" "$@"

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
      set http(-proxyhost) $proxy
      set http(-proxyport) $port
    } else {
      set http(-proxyhost) ""
      set http(-proxyport) ""
    }
  }

  proc copy { url file {f_progress 0} {chunk 8192}} {
    set out [open $file w]
    if {$f_progress} {
       puts -nonewline stderr "$file: "
       if {[catch {geturl $url -channel $out -progress ::http::Progress \
                  -blocksize $chunk -protocol 1.0} token]} {
        puts "Error Geturl $file : Message $token"
        close $out ; return 1
      }
    # This ends the line started by http::Progress
      puts stderr ""
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
    puts -nonewline stderr . ; flush stderr
  }
}

#*****************************************************************************
#*****************************************************************************

if {($argc != 2) && ($argc != 3)} {
  puts "usage: $argv0 <URL> <destination> <optional quiet>"
  exit
}

set URL [lindex $argv 0]
set Dest [lindex $argv 1]
set quiet 0
if {$argc == 3} {
  if {[lindex $argv 2] == "quiet"} {
    set quiet 1
  }
}

if {$quiet} {
  if {[http::copy $URL $Dest 0 20480] != 0} {
    puts "problems downloading $URL... http lines full?"
  }
} else {
  if {[http::copy $URL $Dest 1 20480] != 0} {
    puts "problems downloading $URL... http lines full?"
  }
}
exit
