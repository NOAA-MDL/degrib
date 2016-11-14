
#*****************************************************************************
#  ndfd2.tcl
#
# DESCRIPTION
#    This is the main control for the tkdegrib porgram.  It is intended as a
# control program to handle the download and conversion of NDFD data sets.
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Re-designed from "ndfd.tcl"
#
# NOTES
#*****************************************************************************

set src_dir [file dirname [info script]]
if {[file pathtype $src_dir] != "absolute"} {
  set cur_dir [pwd] ;  cd $src_dir
  set src_dir [pwd] ;  cd $cur_dir
}

foreach script [list scroll.tcl util.tcl ftp_lib.tcl http.tcl print.tcl \
                config2.tcl pane.tcl gis.tcl browse.tcl atdir3.tcl draw.tcl ] {
  set file [file join $src_dir tclsrc $script]
  if {! [file exists $file]} {
    tk_messageBox -message "Fatal Error: Couldn't find required file '$file'"
    exit
  }
  source $file
}

lappend auto_path [file join [file dirname $src_dir] lib]
if {[catch {package require mkWidgets}]} {
  tk_messageBox -message "Fatal Error: Couldn't load the mkWidgets package."
  exit
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
    if {! [file exists $ray(dir,$var)]} {
      file mkdir $ray(dir,$var)
    }
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
      if {! [file exists $ray(imgGen,$var)]} {
        file mkdir $ray(imgGen,$var)
      }
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

#*****************************************************************************
#  SaveIni -- Arthur Taylor / MDL
#
# PURPOSE
#    Write the "ndfd.ini control file to disk
#
# ARGUMENTS
#      rayName = Global array (structure) containing the main variables for
#                this program.
#       (dir,) = "Directories" section
#    (imgGen,) = "ImageGen" section
#    (custom,) = "Custom" section
#       (ftp,) = "FTPSite" section
#      (http,) = "HTTPSite" section
#       (opt,) = "Options" section
# (subSector,) = "CONUS_SubSectors" section
# (subExprSector,) = "CONUS_ExprSubSectors" section
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
#   Some concern about multiple instances, and stale ndfd.ini files.
# Probably should check ndfd.ini time stamp.  If differs, reread?
#*****************************************************************************
proc SaveIni {rayName} {
  global src_dir
  upvar #0 $rayName ray
  set Parent [file dirname $src_dir]
  set ParentLen [string length $Parent]
  set filename [file join $src_dir ndfd.ini]
#  file delete $filename
  set ansLst ""

##### Save the "Directories" section (dir,) #####
  lappend ansList "\[NDFD_Directories\]"
  foreach var $ray(dirList) {
    set val $ray(dir,$var)
    # Check if we can shorten it using "Parent".
    if {[string compare -length $ParentLen $val $Parent] == 0} {
      if {[string length $val] == $ParentLen} {
        set val "Parent,"
      } else {
        set ans [string range $val $ParentLen end]
        if {[string index $ans 0] == "/"} {
          set val "Parent,[string range $ans 1 end]"
        }
      }
    }
    lappend ansList "$var=$val"
  }
#  ns_Util::WriteIni $filename NDFD_Directories $lst
  lappend ansList ""

##### Save the "ImageGen" section (imgGen,) #####
  lappend ansList "\[NDFD_ImageGen\]"
  foreach var $ray(imgGenList) {
    set val $ray(imgGen,$var)
    # Check if we can shorten it using "Parent".
    if {[string compare -length $ParentLen $val $Parent] == 0} {
      if {[string length $val] == $ParentLen} {
        set val "Parent,"
      } else {
        set ans [string range $val $ParentLen end]
        if {[string index $ans 0] == "/"} {
          set val "Parent,[string range $ans 1 end]"
        }
      }
    }
    lappend ansList "$var=$val"
  }
#  ns_Util::WriteIni $filename NDFD_ImageGen $lst
  lappend ansList ""

##### Save the "Custom" section (custom,) #####
  lappend ansList "\[Custom_Sector\]"
  foreach var $ray(customList) {
    lappend ansList "$var=$ray(custom,$var)"
  }
  foreach var $ray(customVarList) {
    lappend ansList "$var=[join $ray(custom,$var) ,]"
  }
#  ns_Util::WriteIni $filename Custom_Sector $lst
  lappend ansList ""

##### Save the "FTPSite" section (ftp,) #####
  lappend ansList "\[NDFD_FTPSite\]"
  foreach var $ray(ftpList) {
    lappend ansList "$var=$ray(ftp,$var)"
  }
#  ns_Util::WriteIni $filename NDFD_FTPSite $lst
  lappend ansList ""

##### Save the "HTTPSite" section (http,) #####
  lappend ansList "\[NDFD_HTTPSite\]"
  foreach var $ray(httpList) {
    lappend ansList "$var=$ray(http,$var)"
  }
#  ns_Util::WriteIni $filename NDFD_HTTPSite $lst
  lappend ansList ""

##### Save the "Options" section (opt,) #####
  lappend ansList "\[NDFD_Options\]"
  foreach var $ray(optList) {
    lappend ansList "$var=$ray(opt,$var)"
  }
#  ns_Util::WriteIni $filename NDFD_Options $lst
  lappend ansList ""

##### Save the "SubSectors" section (subSector,) #####
  lappend ansList "\[CONUS_SubSectors\]"
  foreach var $ray(subSectorList) {
    if {[string index $var 0] == "#"} {
      lappend ansList "$var=$ray(subSector,$var)"
    } else {
      lappend ansList "$var=[join $ray(subSector,$var) ,]"
    }
  }
#  ns_Util::WriteIni $filename NDFD_Options $lst
  lappend ansList ""

##### Save the "ExprSubSectors" section (subExprSector,) #####
  lappend ansList "\[CONUS_ExprSubSectors\]"
  foreach var $ray(subExprSectorList) {
    if {[string index $var 0] == "#"} {
      lappend ansList "$var=$ray(subExprSector,$var)"
    } else {
      lappend ansList "$var=[join $ray(subExprSector,$var) ,]"
    }
  }
#  ns_Util::WriteIni $filename NDFD_Options $lst
  lappend ansList ""

##### Save the "NDFD_Variables" section (foreVar,) #####
  lappend ansList "\[NDFD_Variables\]"
  foreach var $ray(foreVarList) {
    if {[string index $var 0] == "#"} {
      lappend ansList "$var=$ray(foreVar,$var)"
    } else {
      lappend ansList "$var=[join $ray(foreVar,$var) ,]"
    }
  }
#  ns_Util::WriteIni $filename NDFD_Variables $lst
  lappend ansList ""

##### Save the "NDFD_EXPR_Variables" section (foreExprVar,) #####
  lappend ansList "\[NDFD_EXPR_Variables\]"
  foreach var $ray(foreExprVarList) {
    if {[string index $var 0] == "#"} {
      lappend ansList "$var=$ray(foreExprVar,$var)"
    } else {
      lappend ansList "$var=[join $ray(foreExprVar,$var) ,]"
    }
  }
#  ns_Util::WriteIni $filename NDFD_Variables $lst
  lappend ansList ""

##### Save the "NDGD_Variables" section (guidVar,) #####
  lappend ansList "\[NDGD_Variables\]"
  foreach var $ray(guidVarList) {
    if {[string index $var 0] == "#"} {
      lappend ansList "$var=$ray(guidVar,$var)"
    } else {
      lappend ansList "$var=[join $ray(guidVar,$var) ,]"
    }
  }
#  ns_Util::WriteIni $filename NDGD_Variables $lst
  lappend ansList ""

##### Save the "NDGD_Variables" section (guidExprVar,) #####
  lappend ansList "\[NDGD_EXPR_Variables\]"
  foreach var $ray(guidExprVarList) {
    if {[string index $var 0] == "#"} {
      lappend ansList "$var=$ray(guidExprVar,$var)"
    } else {
      lappend ansList "$var=[join $ray(guidExprVar,$var) ,]"
    }
  }
#  ns_Util::WriteIni $filename NDGD_Variables $lst
#  lappend ansList ""

##### Write the data to disk. #####
  set fp [open $filename w]
  foreach line $ansList {
    puts $fp $line
  }
  close $fp
}

#*****************************************************************************
#  Close -- Arthur Taylor / MDL
#
# PURPOSE
#    Main closing routine for the tkdegrib program.  Intended to make sure
# that data gets freed properly before closing.  Useful for memwatch.
#
# ARGUMENTS
# rayName = Global structure containing temp global variables for this
#           program.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#*****************************************************************************
proc Close {rayName} {
  upvar #0 $rayName ray

##### Save program state variables that have been changed through
##### interaction with the user
  if {[info exists ray(.foo.tc.gis.top,path)]} {
    set GIS_IN $ray(.foo.tc.gis.top,path)
    if {! [file isdirectory $GIS_IN]} {
      set GIS_IN [file dirname $GIS_IN]
    }
    set ray(dir,GIS_IN) $GIS_IN
  }
  # Save current .ini and exit... (dangerous because of multiple writes, and
  # potential to corrupt the file.
  # SaveIni $rayName

  catch {destroy $ray(tl)}

##### The following forces Tcl/Tk to free the GRIB2 commands. #####
  proc $ray(GRIB2Cmd) {} {}
#  proc $ray(GRIB2WspdCmd) {} {}
#  proc $ray(GRIB2WdirCmd) {} {}
  exit
}

#*****************************************************************************
#  About -- Arthur Taylor / MDL
#
# PURPOSE
#    Describes the program's version, and gives a disclaimer
#
# ARGUMENTS
# rayName = Global structure containing temp global variables for this
#           program.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#*****************************************************************************
proc About {rayName} {
  upvar #0 $rayName ray
  set about [$ray(GRIB2Cmd) -V]
  set Version [lindex $about 2]
  set Date [lindex $about 4]

  set tl .about
  catch {destroy $tl}
  toplevel $tl
  wm title $tl "About"

  text $tl.txt -bg gray80 -height 12 -width 44 -relief ridge -bd 4
  text $tl.txt2 -bg gray80 -height 15 -width 50 -relief ridge -bd 4 -wrap word
  button $tl.ok -text "Ok" -command "destroy $tl" -width 10
  pack $tl.txt $tl.txt2 -fill both -expand yes
  pack $tl.ok -side top
  focus $tl.ok
  bind $tl.ok <Return> "destroy $tl"
  wm resizable $tl 0 0

  $tl.txt insert end "NDFD GRIB2 Decoder & Download Program ($Version)\n"
  $tl.txt insert end "Date: $Date\n"
  $tl.txt insert end "Author: Arthur Taylor\n"
  $tl.txt insert end "(MDL-NWS-NOAA)\n\n"
  $tl.txt insert end "The NDFD GRIB2 Decoder Page:\n"
  $tl.txt insert end "www.nws.noaa.gov/mdl/degrib/\n\n"
#  $tl.txt insert end "For images, the NDFD GRIB2 Decoder currently uses:\n"
#  $tl.txt insert end "NDFD superImageGen (MS-Windows only)\n"
#  $tl.txt insert end "Date: 8/11/2003\n"
#  $tl.txt insert end "Author: Rici Yu (SAIC/MDL-NWS-NOAA)\n"
  $tl.txt tag add Tag1 0.0 end
  $tl.txt tag configure Tag1 -justify center
  bind $tl.txt <Control-f> "PaneFun"
  bind $tl.txt <Control-Alt-F5> "PaneValid $tl.txt"
  bind $tl.txt <Control-F5> "PaneValid $tl.txt"

  $tl.txt2 insert end "DISCLAIMER:\n\nThe user assumes the entire risk related\
  to the use of this program, and its associated data.  NWS is providing this\
  program and data \"as is\" and NWS disclaims any and all warranties, whether\
  express or implied, including (without limitation) any implied warranties of\
  merchantability or fitness for a particular purpose.  In no event will\
  NWS or any other Federal Agency be liable to you or to any third party\
  for any direct, indirect,\
  incidental, consequential, special, or exemplary damages or lost profit\
  resulting from any use or misuse of this program and its associated data."
  $tl.txt2 configure -state disabled

  update
  return
}

#*****************************************************************************
#  Help -- Arthur Taylor / MDL
#
# PURPOSE
#    Provide some basic program help
#
# ARGUMENTS
# rayName = Global structure containing temp global variables for this
#           program.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#*****************************************************************************
proc Help {} {
  set tl .help
  catch {destroy $tl}
  toplevel $tl
  wm title $tl "NDFD DataDownload Help"
    frame $tl.t -relief ridge -bd 4
      text $tl.t.txt -width 60 -height 28 -relief flat \
            -setgrid true -yscrollcommand "$tl.t.sb set" -wrap word
      scrollbar $tl.t.sb -command "$tl.t.txt yview"
    pack $tl.t.sb -side right -fill y
    pack $tl.t.txt -side left -fill both -expand true
    button $tl.ok -text "Ok" -command "catch {destroy $tl}"
  pack $tl.ok -side bottom -fill y -expand no
  pack $tl.t -side top -fill both -expand yes

  set txt $tl.t.txt
  foreach line {
    "Welcome to the NDFD GRIB2 Decoder & Download Program."
    ""
    "     The purpose of this program is to allow one to download the \
     current National Weather Service's NDFD (National Digital Forecast \
     Database) weather forecasts.  After downloading it one can convert \
     the data to several different formats for use in various GIS systems."
    ""
    "     In addition it currently links to a different program which \
     can generate a set of images, which can be browsed using a standard \
     web browser"
    ""
    "To use the program:"
    "Step 1) Download current data (Download tab)."
    "     Highlight some sectors which do NOT have a 'N.A.' (Not Available) \
     for its 'Most Recent Data'.  Click on 'Download'.  This causes \
     the program to access the server and download all the variables \
     of interest (you can configure the program to download a subset)"
    ""
    "Step 2) Convert Images"
    "     Switch to the GIS Tab.  Browse for the data you want to convert. \
     Double click on the file.  Single click on the message that you're \
     interested in.  Select file output type, and press the generate \
     output button"
    ""
    "Optionally, you can click on 'Generate Images'.  This causes the \
     program to convert the data, and call 'superImgGen' to generate .png \
     files.  It then calls 'htmlGen' to generate some .html files, and \
     finally calls I.E. to display the web page it generated in the output \
     directory."
    ""
    "Note: In the future, we plan to allow dynamic interaction with the \
     data.  At that point we will allow arbitrary zooms, instead of the \
     pre-determined ones we are using initially.  In addition we will \
     probably move away from the dependence on html pages."
  } {
    $txt insert end "$line\n"
  }
}

#*****************************************************************************
#  CancelAction -- Arthur Taylor / MDL
#
# PURPOSE
#    Provide the cancel button option.
#
# ARGUMENTS
# rayName = Global struct containing variables for this program.
#    flag = 1 button press, 2 action responded
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#*****************************************************************************
proc CancelAction {rayName flag} {
  upvar #0 $rayName ray

  if {$flag == 1} {
    if {$ray(Action) == "None"} {
      ns_Print::puts "No action to cancel?"
      set ray(Cancel) 0
      set ray(Action) None
      ns_Util::DrawXPerc $ray(DoneCanv) 100
      return
    }
    set ray(Cancel) 1
    ns_Print::puts "Attempting to cancel $ray(Action)"
    return
  } else {
    ns_Print::puts "----------"
    ns_Print::puts "Canceled $ray(Action)"
    set ray(Cancel) 0
    set ray(Action) None
#    ns_Util::DrawXPerc $ray(DoneCanv) 100
#    MainRefresh $rayName
    return
  }
}

#*****************************************************************************
#  RefreshTimes -- Arthur Taylor / MDL
#
# PURPOSE
#    Refresh the times on the files.
#
# ARGUMENTS
# rayName = Global structure containing temp global variables for this
#           program.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#   Could probably clean this up as it has a lot of repetitive tasks.
#*****************************************************************************
proc RefreshTimes {rayName} {
  upvar #0 $rayName ray
  set f_update 0

##### Update the refrence times on the NDFD files. #####
#tk_messageBox -message "Update reference times on the NDFD files."
  foreach var $ray(foreVarList) {
    if {[string index $var 0] == "#"} {
      continue
    }
    set file [file join $ray(dir,NDFD_OpnlData) [lindex $ray(foreVar,$var) 2]]
    set times [split [lindex $ray(foreVar,$var) 5] :]
    set mtime [lindex $times 0]
    set rtime [lindex $times 1]
    if {[file exists $file]} {
      set fileMtime [file mtime $file]
      if {$mtime != $fileMtime} {
        set f_update 1
        set mtime $fileMtime
        if {[catch {$ray(GRIB2Cmd) -refTime -in $file} rtime]} {
          file delete -force $file
#          puts $file
          set ray(foreVar,$var) [lreplace $ray(foreVar,$var) 5 5 "0:0"]
        } else {
          set rtime [format "%.0f" $rtime]
          set ray(foreVar,$var) [lreplace $ray(foreVar,$var) 5 5 "$mtime:$rtime"]
        }
      }
    } elseif {$mtime != 0} {
      set ray(foreVar,$var) [lreplace $ray(foreVar,$var) 5 5 "0:0"]
      set f_update 1
    }
  }

##### Update the refrence times on the NDFD Expr files. #####
#tk_messageBox -message "Update reference times on the NDFD Expr files."
  foreach var $ray(foreExprVarList) {
    if {[string index $var 0] == "#"} {
      continue
    }
    set file [file join $ray(dir,NDFD_ExprData) [lindex $ray(foreExprVar,$var) 2]]
    set times [split [lindex $ray(foreExprVar,$var) 5] :]
    set mtime [lindex $times 0]
    set rtime [lindex $times 1]
    if {[file exists $file]} {
      set fileMtime [file mtime $file]
      if {$mtime != $fileMtime} {
        set f_update 1
        set mtime $fileMtime
        if {[catch {$ray(GRIB2Cmd) -refTime -in $file} rtime]} {
          file delete -force $file
#          puts $file
          set ray(foreExprVar,$var) [lreplace $ray(foreExprVar,$var) 5 5 "0:0"]
        } else {
          set rtime [format "%.0f" $rtime]
          set ray(foreExprVar,$var) [lreplace $ray(foreExprVar,$var) 5 5 "$mtime:$rtime"]
        }
      }
    } elseif {$mtime != 0} {
      set ray(foreExprVar,$var) [lreplace $ray(foreExprVar,$var) 5 5 "0:0"]
      set f_update 1
    }
  }

##### Update the refrence times on the Custom files. #####
#tk_messageBox -message "Update reference times on the Custom files."
  foreach var $ray(customVarList) {
    set file [file join $ray(dir,NDFD_OpnlData) [lindex $ray(custom,$var) 2]]
    set times [split [lindex $ray(custom,$var) 5] :]
    set mtime [lindex $times 0]
    set rtime [lindex $times 1]
    if {[file exists $file]} {
      set fileMtime [file mtime $file]
      if {$mtime != $fileMtime} {
        set f_update 1
        set mtime $fileMtime
        if {[catch {$ray(GRIB2Cmd) -refTime -in $file} rtime]} {
          file delete -force $file
#          puts $file
          set ray(custom,$var) [lreplace $ray(custom,$var) 5 5 "0:0"]
        } else {
          set rtime [format "%.0f" $rtime]
          set ray(custom,$var) [lreplace $ray(custom,$var) 5 5 "$mtime:$rtime"]
        }
      }
    } elseif {$mtime != 0} {
      set ray(custom,$var) [lreplace $ray(custom,$var) 5 5 "0:0"]
      set f_update 1
    }
  }

##### Update the refrence times on the NDGD files. #####
#tk_messageBox -message "Update reference times on the Guidance files"
  foreach var $ray(guidVarList) {
    if {[string index $var 0] == "#"} {
      continue
    }
    set file [file join $ray(dir,NDGD_OpnlData) [lindex $ray(guidVar,$var) 2]]
    set times [split [lindex $ray(guidVar,$var) 5] :]
    set mtime [lindex $times 0]
    set rtime [lindex $times 1]
    if {[file exists $file]} {
      set fileMtime [file mtime $file]
      if {$mtime != $fileMtime} {
        set f_update 1
        set mtime $fileMtime
        if {[catch {$ray(GRIB2Cmd) -refTime -in $file} rtime]} {
          file delete -force $file
#          puts $file
          set ray(guidVar,$var) [lreplace $ray(guidVar,$var) 5 5 "0:0"]
        } else {
          set rtime [format "%.0f" $rtime]
          set ray(guidVar,$var) [lreplace $ray(guidVar,$var) 5 5 "$mtime:$rtime"]
        }
      }
    } elseif {$mtime != 0} {
      set ray(guidVar,$var) [lreplace $ray(guidVar,$var) 5 5 "0:0"]
      set f_update 1
    }
  }

##### Update the refrence times on the NDGD files. #####
#tk_messageBox -message "Update reference times on the Expr Guidance files"
  foreach var $ray(guidExprVarList) {
    if {[string index $var 0] == "#"} {
      continue
    }
    set file [file join $ray(dir,NDGD_ExprData) [lindex $ray(guidExprVar,$var) 2]]
    set times [split [lindex $ray(guidExprVar,$var) 5] :]
    set mtime [lindex $times 0]
    set rtime [lindex $times 1]
    if {[file exists $file]} {
      set fileMtime [file mtime $file]
      if {$mtime != $fileMtime} {
        set f_update 1
        set mtime $fileMtime
        if {[catch {$ray(GRIB2Cmd) -refTime -in $file} rtime]} {
          file delete -force $file
#          puts $file
          set ray(guidExprVar,$var) [lreplace $ray(guidExprVar,$var) 5 5 "0:0"]
        } else {
          set rtime [format "%.0f" $rtime]
          set ray(guidExprVar,$var) [lreplace $ray(guidExprVar,$var) 5 5 "$mtime:$rtime"]
        }
      }
    } elseif {$mtime != 0} {
      set ray(guidExprVar,$var) [lreplace $ray(guidExprVar,$var) 5 5 "0:0"]
      set f_update 1
    }
  }

##### Update the refrence times on the CONUS SubSectors files. #####
#tk_messageBox -message "Update reference times on the sub Sector files"
  foreach var $ray(subSectorList) {
    if {! [info exists ray(foreVar,$var)]} {
      continue
    }
    if {[string range $var 0 4] != "conus"} {
      continue
    }
    for {set i 0} {$i < [llength $ray(subSector,LocalName)]} {incr i} {
      set sector [lindex $ray(subSector,LocalName) $i]
      set path [lindex $ray(foreVar,$var) 2]
      set path [string replace $path 0 4 $sector]
      set file [file join $ray(dir,NDFD_OpnlData) $path]
      set times [split [lindex $ray(subSector,$var) $i] :]
      set mtime [lindex $times 0]
      set rtime [lindex $times 1]
      if {[file exists $file]} {
        set fileMtime [file mtime $file]
        if {$mtime != $fileMtime} {
          set f_update 1
          set mtime $fileMtime
          if {[catch {$ray(GRIB2Cmd) -refTime -in $file} rtime]} {
            file delete -force $file
#            puts $file
            set ray(subSector,$var) [lreplace $ray(subSector,$var) $i $i "0:0"]
          } else {
            set rtime [format "%.0f" $rtime]
            set ray(subSector,$var) [lreplace $ray(subSector,$var) $i $i "$mtime:$rtime"]
          }
        }
      } elseif {$mtime != 0} {
        set ray(subSector,$var) [lreplace $ray(subSector,$var) $i $i "0:0"]
        set f_update 1
      }
    }
  }

##### Update the refrence times on the CONUS SubExprSectors files. #####
#tk_messageBox -message "Update reference times on the sub Expr Sector files"
  foreach var $ray(subExprSectorList) {
    if {! [info exists ray(foreExprVar,$var)]} {
      continue
    }
    if {[string range $var 0 4] != "conus"} {
      continue
    }
    for {set i 0} {$i < [llength $ray(subExprSector,LocalName)]} {incr i} {
      set sector [lindex $ray(subExprSector,LocalName) $i]
      set path [lindex $ray(foreExprVar,$var) 2]
      set path [string replace $path 0 4 $sector]
      set file [file join $ray(dir,NDFD_ExprData) $path]
      set times [split [lindex $ray(subExprSector,$var) $i] :]
      set mtime [lindex $times 0]
      set rtime [lindex $times 1]
      if {[file exists $file]} {
        set fileMtime [file mtime $file]
        if {$mtime != $fileMtime} {
          set f_update 1
          set mtime $fileMtime
          if {[catch {$ray(GRIB2Cmd) -refTime -in $file} rtime]} {
            file delete -force $file
#            puts $file
            set ray(subExprSector,$var) [lreplace $ray(subExprSector,$var) $i $i "0:0"]
          } else {
            set rtime [format "%.0f" $rtime]
            set ray(subExprSector,$var) [lreplace $ray(subExprSector,$var) $i $i "$mtime:$rtime"]
          }
        }
      } elseif {$mtime != 0} {
        set ray(subExprSector,$var) [lreplace $ray(subExprSector,$var) $i $i "0:0"]
        set f_update 1
      }
    }
  }

  if {$f_update} {
    SaveIni $rayName
  }
}

#*****************************************************************************
#  RefreshFolderList -- Arthur Taylor / MDL
#
# PURPOSE
#    Refresh the display of the times of the files in the folder list.
#
# ARGUMENTS
# rayName = Global structure containing temp global variables for this
#           program.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#   Inserting this many nodes into a tree is slow.
#   may need another tree type? Tcl 8.4?
#   Could probably clean this up as it has a lot of repetitive tasks.
#*****************************************************************************
proc RefreshFolderList {rayName {f_msg 0}} {
  global tcl_platform
  upvar #0 $rayName ray

  set ray(now) [clock seconds]
  # Handle the fact that clock seconds in Windows 9* is not in GMT
  if {[string range $tcl_platform(os) 0 8] == "Windows 9"} {
    set tStr [clock format $ray(now) -format "%D %T" -gmt false]
    set ray(now) [clock scan $tStr -gmt true]
  }

##### Update the NDFD folders. #####
  if {$f_msg} {
    ns_Print::puts "Updating the NDFD folders"
  }
  set ray(foreFolder,dirs) ""
  foreach var $ray(foreVarList) {
    if {[string index $var 0] == "#"} {
      continue
    }
    if {[lindex $ray(foreVar,$var) 0] == "NA"} {
      continue
    }
    set prgPath [split [lindex $ray(foreVar,$var) 1] /]
    set time [lindex $ray(foreVar,$var) 5]
    set level ndfd
    set cnt 1
    set len [llength $prgPath]
    foreach dir $prgPath {
      set path [join $dir -]
      ##### Check if we have to add a folder #####
      if {$cnt != $len} {
        set level2 [join [list $level $path] _]
        if {[lsearch $ray(foreFolder,dirs) $level2] == -1} {
          lappend ray(foreFolder,dirs) $level2
          set ray(foreFolder,fileContent,$level2) ""
          set ray(foreFolder,time,$level2) $time
          if {$time == "0:0"} {
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set deltaSec [expr ($ray(now) - $rtime)]
            if {$deltaSec >= 0} {
              set deltaDay [expr $deltaSec / (3600*24)]
              set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
            } else {
              set deltaDay 0
              set deltaHr 0
            }
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set dirRtime [lindex [split $ray(foreFolder,time,$level2) :] 1]
          if {$rtime != 0} {
            if {($dirRtime == 0) || ($rtime < $dirRtime)} {
              set ray(foreFolder,time,$level2) $time
              set deltaSec [expr ($ray(now) - $rtime)]
              if {$deltaSec >= 0} {
                set deltaDay [expr $deltaSec / (3600*24)]
                set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
              } else {
                set deltaDay 0
                set deltaHr 0
              }
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        }

      ##### Dealing with a file #####
      } else {
        set level2 [join [list $level $path $var] _]
        lappend ray(foreFolder,fileContent,$level) $level2
        if {$time == "0:0"} {
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set deltaSec [expr ($ray(now) - $rtime)]
          if {$deltaSec >= 0} {
            set deltaDay [expr $deltaSec / (3600*24)]
            set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
          } else {
            set deltaDay 0
            set deltaHr 0
          }
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
          }
        }
      }
      set level $level2
      incr cnt
    }
  }

##### Update the NDFD Expr folders. #####
  if {$f_msg} {
    ns_Print::puts "Updating the NDFD Experimental folders"
  }
  set ray(foreExprFolder,dirs) ""
  foreach var $ray(foreExprVarList) {
    if {[string index $var 0] == "#"} {
      continue
    }
    if {[lindex $ray(foreExprVar,$var) 0] == "NA"} {
      continue
    }
    set prgPath [split [lindex $ray(foreExprVar,$var) 1] /]
    set time [lindex $ray(foreExprVar,$var) 5]
    set level ndfdExpr
    set cnt 1
    set len [llength $prgPath]
    foreach dir $prgPath {
      set path [join $dir -]
      ##### Check if we have to add a folder #####
      if {$cnt != $len} {
        set level2 [join [list $level $path] _]
        if {[lsearch $ray(foreExprFolder,dirs) $level2] == -1} {
          lappend ray(foreExprFolder,dirs) $level2
          set ray(foreExprFolder,fileContent,$level2) ""
          set ray(foreExprFolder,time,$level2) $time
          if {$time == "0:0"} {
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set deltaSec [expr ($ray(now) - $rtime)]
            if {$deltaSec >= 0} {
              set deltaDay [expr $deltaSec / (3600*24)]
              set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
            } else {
              set deltaDay 0
              set deltaHr 0
            }
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set dirRtime [lindex [split $ray(foreExprFolder,time,$level2) :] 1]
          if {$rtime != 0} {
            if {($dirRtime == 0) || ($rtime < $dirRtime)} {
              set ray(foreExprFolder,time,$level2) $time
              set deltaSec [expr ($ray(now) - $rtime)]
              if {$deltaSec >= 0} {
                set deltaDay [expr $deltaSec / (3600*24)]
                set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
              } else {
                set deltaDay 0
                set deltaHr 0
              }
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        }

      ##### Dealing with a file #####
      } else {
        set level2 [join [list $level $path $var] _]
        lappend ray(foreExprFolder,fileContent,$level) $level2
        if {$time == "0:0"} {
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set deltaSec [expr ($ray(now) - $rtime)]
          if {$deltaSec >= 0} {
            set deltaDay [expr $deltaSec / (3600*24)]
            set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
          } else {
            set deltaDay 0
            set deltaHr 0
          }
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
          }
        }
      }
      set level $level2
      incr cnt
    }
  }

##### Update the NDGD folders. #####
  if {$f_msg} {
    ns_Print::puts "Updating the NDGD folders"
  }
  set ray(guidFolder,dirs) ""
  foreach var $ray(guidVarList) {
    if {[string index $var 0] == "#"} {
      continue
    }
    set prgPath [split [lindex $ray(guidVar,$var) 1] /]
    set time [lindex $ray(guidVar,$var) 5]
    set level ndgd
    set cnt 1
    set len [llength $prgPath]
    foreach dir $prgPath {
      set path [join $dir -]
      ##### Check if we have to add a folder #####
      if {$cnt != $len} {
        set level2 [join [list $level $path] _]
        if {[lsearch $ray(guidFolder,dirs) $level2] == -1} {
          lappend ray(guidFolder,dirs) $level2
          set ray(guidFolder,fileContent,$level2) ""
          set ray(guidFolder,time,$level2) $time
          if {$time == "0:0"} {
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set deltaSec [expr ($ray(now) - $rtime)]
            if {$deltaSec >= 0} {
              set deltaDay [expr $deltaSec / (3600*24)]
              set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
            } else {
              set deltaDay 0
              set deltaHr 0
            }
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set dirRtime [lindex [split $ray(guidFolder,time,$level2) :] 1]
          if {$rtime != 0} {
            if {($dirRtime == 0) || ($rtime < $dirRtime)} {
              set ray(guidFolder,time,$level2) $time
              set deltaSec [expr ($ray(now) - $rtime)]
              if {$deltaSec >= 0} {
                set deltaDay [expr $deltaSec / (3600*24)]
                set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
              } else {
                set deltaDay 0
                set deltaHr 0
              }
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        }

      ##### Dealing with a file #####
      } else {
        set level2 [join [list $level $path $var] _]
        lappend ray(guidFolder,fileContent,$level) $level2
        if {$time == "0:0"} {
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set deltaSec [expr ($ray(now) - $rtime)]
          if {$deltaSec >= 0} {
            set deltaDay [expr $deltaSec / (3600*24)]
            set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
          } else {
            set deltaDay 0
            set deltaHr 0
          }
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
          }
        }
      }
      set level $level2
      incr cnt
    }
  }

##### Update the NDGD folders. #####
  if {$f_msg} {
    ns_Print::puts "Updating the NDGD Experimental folders"
  }
  set ray(guidExprFolder,dirs) ""
  foreach var $ray(guidExprVarList) {
    if {[string index $var 0] == "#"} {
      continue
    }
    set prgPath [split [lindex $ray(guidExprVar,$var) 1] /]
    set time [lindex $ray(guidExprVar,$var) 5]
    set level ndgdExpr
    set cnt 1
    set len [llength $prgPath]
    foreach dir $prgPath {
      set path [join $dir -]
      ##### Check if we have to add a folder #####
      if {$cnt != $len} {
        set level2 [join [list $level $path] _]
        if {[lsearch $ray(guidExprFolder,dirs) $level2] == -1} {
          lappend ray(guidExprFolder,dirs) $level2
          set ray(guidExprFolder,fileContent,$level2) ""
          set ray(guidExprFolder,time,$level2) $time
          if {$time == "0:0"} {
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set deltaSec [expr ($ray(now) - $rtime)]
            if {$deltaSec >= 0} {
              set deltaDay [expr $deltaSec / (3600*24)]
              set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
            } else {
              set deltaDay 0
              set deltaHr 0
            }
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set dirRtime [lindex [split $ray(guidExprFolder,time,$level2) :] 1]
          if {$rtime != 0} {
            if {($dirRtime == 0) || ($rtime < $dirRtime)} {
              set ray(guidExprFolder,time,$level2) $time
              set deltaSec [expr ($ray(now) - $rtime)]
              if {$deltaSec >= 0} {
                set deltaDay [expr $deltaSec / (3600*24)]
                set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
              } else {
                set deltaDay 0
                set deltaHr 0
              }
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        }

      ##### Dealing with a file #####
      } else {
        set level2 [join [list $level $path $var] _]
        lappend ray(guidExprFolder,fileContent,$level) $level2
        if {$time == "0:0"} {
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set deltaSec [expr ($ray(now) - $rtime)]
          if {$deltaSec >= 0} {
            set deltaDay [expr $deltaSec / (3600*24)]
            set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
          } else {
            set deltaDay 0
            set deltaHr 0
          }
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
          }
        }
      }
      set level $level2
      incr cnt
    }
  }

##### Update the CONUS SubSectors folders. #####
  if {$f_msg} {
    ns_Print::puts "Updating the CONUS subSector folders"
  }
  set ray(subFolder,dirs) ""
  foreach var $ray(subSectorList) {
    if {! [info exists ray(foreVar,$var)]} {
#       tk_messageBox -message "No $var in conus expr"
      continue
    }
    if {[string range $var 0 4] != "conus"} {
      continue
    }
    for {set i 0} {$i < [llength $ray(subSector,LabelName)]} {incr i} {
      set name [lindex $ray(subSector,LabelName) $i]
      set localName [lindex $ray(subSector,LocalName) $i]
      set prgPath [split [lindex $ray(foreVar,$var) 1] /]
      set time [lindex $ray(subSector,$var) $i]
      set prgPath [lreplace $prgPath 0 0 $name]
#      set prgPath [linsert $prgPath 1 $name]
      set level ndfd
      set cnt 1
      set len [llength $prgPath]
      foreach dir $prgPath {
        set path [join $dir -]
        ##### Check if we have to add a folder #####
        if {$cnt != $len} {
          set level2 [join [list $level $path] _]
          if {[lsearch $ray(subFolder,dirs) $level2] == -1} {
            lappend ray(subFolder,dirs) $level2
            set ray(subFolder,fileContent,$level2) ""
            set ray(subFolder,time,$level2) $time
            if {$time == "0:0"} {
              if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pFo -parent $level}]} {
                $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
              }
            } else {
              set rtime [lindex [split $time :] 1]
              set deltaSec [expr ($ray(now) - $rtime)]
              if {$deltaSec >= 0} {
                set deltaDay [expr $deltaSec / (3600*24)]
                set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
              } else {
                set deltaDay 0
                set deltaHr 0
              }
              if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pFo -parent $level}]} {
                $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
              }
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set dirRtime [lindex [split $ray(subFolder,time,$level2) :] 1]
            if {$rtime != 0} {
              if {($dirRtime == 0) || ($rtime < $dirRtime)} {
                set ray(subFolder,time,$level2) $time
                set deltaSec [expr ($ray(now) - $rtime)]
                if {$deltaSec >= 0} {
                  set deltaDay [expr $deltaSec / (3600*24)]
                  set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
                } else {
                  set deltaDay 0
                  set deltaHr 0
                }
                $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
              }
            }
          }

        ##### Dealing with a file #####
        } else {
          set var2 [join [linsert [split $var -] 1 $localName] -]
          set level2 [join [list $level $path $var2] _]
          lappend ray(subFolder,fileContent,$level) $level2
          if {$time == "0:0"} {
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pGrib2 -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set deltaSec [expr ($ray(now) - $rtime)]
            if {$deltaSec >= 0} {
              set deltaDay [expr $deltaSec / (3600*24)]
              set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
            } else {
              set deltaDay 0
              set deltaHr 0
            }
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pGrib2 -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        }
        set level $level2
        incr cnt
      }
    }
  }

##### Update the CONUS SubExprSectors folders. #####
  if {$f_msg} {
    ns_Print::puts "Updating the CONUS Experimental subSector folders"
  }
  set ray(subExprFolder,dirs) ""
  foreach var $ray(subExprSectorList) {
    if {! [info exists ray(foreExprVar,$var)]} {
#       tk_messageBox -message "No $var in conus expr"
      continue
    }
    if {[string range $var 0 4] != "conus"} {
      continue
    }
    for {set i 0} {$i < [llength $ray(subExprSector,LabelName)]} {incr i} {
      set name [lindex $ray(subExprSector,LabelName) $i]
      set localName [lindex $ray(subExprSector,LocalName) $i]
      set prgPath [split [lindex $ray(foreExprVar,$var) 1] /]
      set time [lindex $ray(subExprSector,$var) $i]
      set prgPath [lreplace $prgPath 0 0 $name]
#      set prgPath [linsert $prgPath 1 $name]
      set level ndfdExpr
      set cnt 1
      set len [llength $prgPath]
      foreach dir $prgPath {
        set path [join $dir -]
        ##### Check if we have to add a folder #####
        if {$cnt != $len} {
          set level2 [join [list $level $path] _]
          if {[lsearch $ray(subExprFolder,dirs) $level2] == -1} {
            lappend ray(subExprFolder,dirs) $level2
            set ray(subExprFolder,fileContent,$level2) ""
            set ray(subExprFolder,time,$level2) $time
            if {$time == "0:0"} {
              if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pFo -parent $level}]} {
                $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
              }
            } else {
              set rtime [lindex [split $time :] 1]
              set deltaSec [expr ($ray(now) - $rtime)]
              if {$deltaSec >= 0} {
                set deltaDay [expr $deltaSec / (3600*24)]
                set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
              } else {
                set deltaDay 0
                set deltaHr 0
              }
              if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pFo -parent $level}]} {
                $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
              }
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set dirRtime [lindex [split $ray(subExprFolder,time,$level2) :] 1]
            if {$rtime != 0} {
              if {($dirRtime == 0) || ($rtime < $dirRtime)} {
                set ray(subExprFolder,time,$level2) $time
                set deltaSec [expr ($ray(now) - $rtime)]
                if {$deltaSec >= 0} {
                  set deltaDay [expr $deltaSec / (3600*24)]
                  set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
                } else {
                  set deltaDay 0
                  set deltaHr 0
                }
                $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
              }
            }
          }

        ##### Dealing with a file #####
        } else {
          set var2 [join [linsert [split $var -] 1 $localName] -]
          set level2 [join [list $level $path $var2] _]
          lappend ray(subExprFolder,fileContent,$level) $level2
          if {$time == "0:0"} {
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pGrib2 -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set deltaSec [expr ($ray(now) - $rtime)]
            if {$deltaSec >= 0} {
              set deltaDay [expr $deltaSec / (3600*24)]
              set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
            } else {
              set deltaDay 0
              set deltaHr 0
            }
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pGrib2 -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        }
        set level $level2
        incr cnt
      }
    }
  }

##### Update the CONUS Custom folder. #####
  if {$f_msg} {
    ns_Print::puts "Updating the Custom folders"
  }
  set ray(customFolder,dirs) ""
  foreach var $ray(customVarList) {
    if {[lindex $ray(custom,$var) 0] == "NA"} {
      continue
    }
    set prgPath [split [lindex $ray(custom,$var) 1] /]
    set time [lindex $ray(custom,$var) 5]
    set level ndfd
    set cnt 1
    set len [llength $prgPath]
    foreach dir $prgPath {
      set path [join $dir -]
      ##### Check if we have to add a folder #####
      if {$cnt != $len} {
        set level2 [join [list $level $path] _]
        if {[lsearch $ray(customFolder,dirs) $level2] == -1} {
          lappend ray(customFolder,dirs) $level2
          set ray(customFolder,fileContent,$level2) ""
          set ray(customFolder,time,$level2) $time

          if {$time == "0:0"} {
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
            }
          } else {
            set rtime [lindex [split $time :] 1]
            set deltaSec [expr ($ray(now) - $rtime)]
            if {$deltaSec >= 0} {
              set deltaDay [expr $deltaSec / (3600*24)]
              set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
            } else {
              set deltaDay 0
              set deltaHr 0
            }
            if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pFo -parent $level}]} {
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set dirRtime [lindex [split $ray(customFolder,time,$level2) :] 1]
          if {$rtime != 0} {
            if {($dirRtime == 0) || ($rtime < $dirRtime)} {
              set ray(customFolder,time,$level2) $time
              set deltaSec [expr ($ray(now) - $rtime)]
              if {$deltaSec >= 0} {
                set deltaDay [expr $deltaSec / (3600*24)]
                set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
              } else {
                set deltaDay 0
                set deltaHr 0
              }
              $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
            }
          }
        }

      ##### Dealing with a file #####
      } else {
        set level2 [join [list $level $path $var] _]
        lappend ray(customFolder,fileContent,$level) $level2
        if {$time == "0:0"} {
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: Need to Download" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: Need to Download"
          }
        } else {
          set rtime [lindex [split $time :] 1]
          set deltaSec [expr ($ray(now) - $rtime)]
          if {$deltaSec >= 0} {
            set deltaDay [expr $deltaSec / (3600*24)]
            set deltaHr [expr ($deltaSec - 3600*24*$deltaDay) / 3600]
          } else {
            set deltaDay 0
            set deltaHr 0
          }
          if {[catch {$ray(lb,tree) insert $level2 -text "$dir :: $deltaDay days $deltaHr hrs old" -image pGrib2 -parent $level}]} {
            $ray(lb,tree) nodeconfigure $level2 -text "$dir :: $deltaDay days $deltaHr hrs old"
          }
        }
      }
      set level $level2
      incr cnt
    }
  }

  if {$f_msg} {
    ns_Print::puts "Done with Refresh folderList"
  }
}

#*****************************************************************************
#  DownloadTab -- Arthur Taylor / MDL
#
# PURPOSE
#    Create the Download tab
#
# ARGUMENTS
#   frame = Frame to add the download widgets to.
# rayName = Global structure containing temp global variables for this
#           program.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#*****************************************************************************
proc DownloadTab {frame rayName} {
  upvar #0 $rayName ray

  set cur0 [frame $frame.top]
##### Build the tree control window. #####
    set cur [frame $cur0.left -relief ridge -bd 5]
      set tmp [treecontrol $cur.tree -text {Datasets} -image pNo]
      pack $tmp -fill both -expand 1
      $cur.tree insert ndfd -image pFo \
            -text "NDFD (National Digital Forecast Database)"
      $cur.tree insert ndfdExpr -image pFo \
            -text "Experimental NDFD"
      $cur.tree insert ndgd -image pFo \
            -text "NDGD (National Digital Guidance Database)"
      $cur.tree insert ndgdExpr -image pFo \
            -text "Experimental NDGD"
      set ray(lb,tree) $cur.tree

##### Build the text message window. #####
    set cur [frame $cur0.right -relief ridge -bd 5]
      label $cur.top -text "Message Window"
      set cur1 [frame $cur.debug]
        Scrolled_Text $cur1 $cur1.text -relief sunken -bd 2 -wrap none \
              -width 45 -height 25 -setgrid 1 -tabs {20p} -keepScroll
        ns_Print::Init $cur1.text NULL 1
      pack $cur.top -side top -expand no -fill x
      pack $cur.debug -side top -expand yes -fill both
    Pane_Create $cur0.left $cur0.right -in $cur0 -orient horizontal \
          -amount 360 -amountFrame 1

##### Build the "DoneCanv". #####
  set cur [frame $frame.status -relief ridge -bd 5]
    set ray(DoneCanv) [canvas $cur.canv -height 16 -width 450 \
          -relief sunken -bd 4 -bg white]
    pack $cur.canv -side top

##### Build the buttons. #####
  set cur [frame $frame.bot -relief ridge -bd 5]
    set cur1 [frame $cur.fr]
#      button $cur1.download0 -text "Download by ftp" -command "HttpDownload_Select $rayName 0"
      button $cur1.download1 -text "Download by http" -command "HttpDownload_Select $rayName 1"
      button $cur1.drawImg -text "Draw Images" -command "DrawImages $rayName"
      button $cur1.cancel -text "Cancel Action" -command "CancelAction $rayName 1"
#      pack $cur1.download0 $cur1.download1 $cur1.drawImg $cur1.cancel -side left -padx 20
      pack $cur1.download1 $cur1.drawImg $cur1.cancel -side left -padx 20
    pack $cur1 -side top -expand no -anchor c
  pack $frame.bot -side bottom -expand no -fill x
  pack $frame.status -side bottom -expand no -fill x
  pack $frame.top -side top -expand yes -fill both
}

#*****************************************************************************
#  main -- Arthur Taylor / MDL
#
# PURPOSE
#    Main control for the tkdegrib program
#
# ARGUMENTS
# rayName = Global structure containing temp global variables for this
#           program.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#*****************************************************************************
proc main {rayName} {
  upvar #0 $rayName ray

##### Create the top level window #####
  set tl .foo
  set ray(tl) $tl
  catch {destroy $tl}
  toplevel $tl
  wm title $tl "tkdegrib: NDFD GRIB2 Decoder & Download Program"
  wm protocol $tl WM_DELETE_WINDOW "Close $rayName"
  wm minsize $tl 115 45

  if {! [file exists [file join $ray(dir,Bin) superimageGen.exe]]} {
    set f_imgGen 0
  } else {
    set f_imgGen 1
  }

##### Build Menus #####
  set cur [menu $tl.menu]
  $cur add cascade -label File -menu $cur.file -underline 0
    set cur1 [menu $cur.file -tearoff 0]
    $cur1 add command -label "Configure" -command "Configure $rayName" -state disabled
    $cur1 add separator
    $cur1 add command -label "Download-http" -command "HttpDownload_Select $rayName 1"
    $cur1 add command -label "Download-ftp" -command "HttpDownload_Select $rayName 0"
    if {$f_imgGen} {
      $cur1 add command -label "Draw Images" -command "DrawImages $rayName"
    }
    $cur1 add separator
    $cur1 add command -label "Clear message window" -command "ns_Print::Clear"
    $cur1 add command -label "Clean up files" -command "NDFD_CleanUp $rayName" -state disabled
    $cur1 add separator
	 $cur1 add command -label "Quit" -command "Close $rayName"
  $cur add cascade -label Help -menu $cur.help -underline 0
    set cur1 [menu $cur.help -tearoff 0]
    $cur1 add command -label "Help" -command "Help"
    $cur1 add command -label "About" -command "About $rayName"
  $tl configure -menu $tl.menu

##### Build Main Window Tabs #####
  set tabc [tabcontrol $tl.tc -width auto]
  ###### Next Line is because of a glitch in tabcontrol #####
  bind $tabc <Configure> "update; $tabc _drawTabs"
    set cur [frame $tabc.download]
      DownloadTab $cur $rayName
    $tabc insert download 0 -text " Download " -window $cur
    set cur [frame $tabc.gis]
      set ray(outFile) [file join $ray(dir,GIS_OUT) sample.shp]
      GISTab $cur $rayName
    $tabc insert gis 0 -text " GIS " -window $cur
  pack $tl.tc -side top -fill both -expand 1
  update

  if {! $f_imgGen} {
    $tabc.download.bot.fr.drawImg config -state disabled
    $tabc invoke gis
  } else {
    $tabc invoke download
  }

  update
  ns_Util::DrawXPerc $ray(DoneCanv) 100

  if {$ray(opt,InitMessage)} {
    ns_Print::puts "Welcome to the NDFD Data Decoder & Download Program"
    ns_Print::puts "-----"
    ns_Print::puts "To Download data:"
    ns_Print::puts "  1. Highlight a file or a folder on the left that has files in it"
    ns_Print::puts "  2. Press 'Download'"
    ns_Print::puts "  3. Optional: press 'Generate Images'"
    ns_Print::puts "-----"
    ns_Print::puts "To Convert (Decode) data:"
    ns_Print::puts "  1. On the GIS Tab, browse for the file"
    ns_Print::puts "  2. Double click to populate the middle windows"
    ns_Print::puts "  3. Hightlight the message to convert"
    ns_Print::puts "  4. Press Recommend for a reasonable output filename"
    ns_Print::puts "  5. Select output file type"
    ns_Print::puts "  6. Press 'Generate File' button"
    ns_Print::puts "-----"
  }
  RefreshTimes $rayName
  RefreshFolderList $rayName 1
#  MainTimer $rayName
  $ray(lb,tree) expand ndfd
  set ray(Cancel) 0
  set ray(Action) None
  wm minsize $tl 0 0

  global tcl_platform
  if {[string range $tcl_platform(os) 0 5] == "Window"} {
    wm geometry $tl 165x70
  } else {
    wm geometry $tl 165x70+50+50
  }
  return
}


#*****************************************************************************
# Start main program
#*****************************************************************************
catch {unset progArray}
ReadIni progArray

font create default_degrib -family Arial -size 9 -slant roman
option add *font default_degrib startupFile
option add *font default_degrib userDefault

##### Try to load the grib package.  #####
if {[catch {package require grib2}]} {
  tk_messageBox -message "Fatal Error: Couldn't load the grib2 package."
  exit
}
Grib2Init GRIB2
set progArray(GRIB2Cmd) GRIB2
wm withdraw .
option add *foreground black interactive
main progArray
#SaveIni progArray
