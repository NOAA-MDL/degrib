#*****************************************************************************
# <print.tcl>
#
#     A package intended to ease the creation of console / diagnostic text
#   widgets.  Basic idea: create a text window, and hand over control to this
#   module, calling ns_Print::puts.
#
# History:
#   3/2002 Arthur Taylor (RSIS/MDL): Created
#
# Notes: (example of usage)
#   source p:/ver/verif/print.tcl
#   text .foo -state disabled
#   pack .foo
#   ns_Print::Init .foo
#   ns_Print::puts here
#*****************************************************************************
# Global variables:
#   TextWindow : Contains path to "text" widget or "NULL" to use stdout.
#   ErrorFile  : Filename to use for error messages, or "NULL" to ignore.
#*****************************************************************************
namespace eval ns_Print {
  variable TextWindow NULL
  variable ErrorFile NULL
  variable f_Enabled 0
}

#*****************************************************************************
# ns_Print::Init --
#
#    Initialize the print module.
#
# Arguments:
#    text_path : (optional) Default: NULL, which means use stdout.
#                   otherwise is the "path" to the text widget to put text in.
#    filename  : (optional) Default: NULL, ignore fputs commands.
#                   otherwise name of a valid file to append to.
#    enabled   : (optional) Default: false,
#                   True if user can type in message window.  False if not.
#
# Globals:
#   TextWindow : Contains path to "text" widget or "NULL" to use stdout.
#   ErrorFile  : Filename to use for error messages, or "NULL" to ignore.
#   f_Enabled  : True if user can type in message window.  False if not.
#
# History:
#   3/2002 Arthur Taylor (RSIS/MDL): Created
#
# Notes:
#*****************************************************************************
proc ns_Print::Init {{text_path NULL} {filename NULL} {enabled 0}} {
  variable TextWindow $text_path
  variable ErrorFile $filename
  variable f_Enabled $enabled
  if {$filename != "NULL"} {
    catch {file delete $filename}
    if {! [file exists [file dirname $filename]]} {
      file mkdir [file dirname $filename]
    }
  }
}

#*****************************************************************************
# ns_Print::Clear --
#
#    Clears the text window if there is one.
#
# Arguments:
#
# Globals:
#   TextWindow : Contains path to "text" widget or "NULL" to use stdout.
#   f_Enabled  : True if user can type in message window.  False if not.
#
# History:
#   2/2003 Arthur Taylor (RSIS/MDL): Created
#
# Notes:
#*****************************************************************************
proc ns_Print::Clear {} {
  variable TextWindow
  variable f_Enabled 
  if {$TextWindow != "NULL"} {
    $TextWindow configure -state normal
    $TextWindow delete 1.0 end
    if {! $f_Enabled} {
      $TextWindow configure -state disabled
    }
    $TextWindow see end
  }
  update
}

#*****************************************************************************
# ns_Print::puts --
#
#    Prints a message to stdout or the Text Window.
#
# Arguments:
#    string    : The message to print.
#    f_newline : (optional) true (1) means add a newline after message,
#                   false (0), don't add newline.
#
# Globals:
#   TextWindow : Contains path to "text" widget or "NULL" to use stdout.
#
# History:
#   3/2002 Arthur Taylor (RSIS/MDL): Created
#
# Notes:
#*****************************************************************************
proc ns_Print::puts {string {f_newline 1}} {
  variable TextWindow
  variable f_Enabled
  if {$TextWindow == "NULL"} {
    if {$f_newline} {
      puts $string
    } else {
      puts -nonewline $string
    }
  } else {
    $TextWindow configure -state normal
    $TextWindow insert end $string
    if {$f_newline} {
      $TextWindow insert end "\n"
    }
    if {! $f_Enabled} {
      $TextWindow configure -state disabled
    }
    $TextWindow see end
  }
  update
}

#*****************************************************************************
# ns_Print::fputs --
#
#    Appends a message to ErrorFile (if not NULL)
#
# Arguments:
#    string    : The message to print.
#    f_newline : (optional) true (1) means add a newline after message,
#                   false (0), don't add newline.
#
# Globals:
#   ErrorFile  : Filename to use for error messages, or "NULL" to ignore.
#
# History:
#   3/2002 Arthur Taylor (RSIS/MDL): Created
#
# Notes:
#*****************************************************************************
proc ns_Print::fputs {string {f_newline 1}} {
  variable ErrorFile
  if {$ErrorFile != "NULL"} {
    set fp [open $ErrorFile a]
    if {$f_newline} {
      ::puts $fp $string
    } else {
      ::puts $fp -nonewline $string
    }
    close $fp
  }
}
