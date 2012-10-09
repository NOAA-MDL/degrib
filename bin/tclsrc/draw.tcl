#*****************************************************************************
#  DrawImages -- Arthur Taylor / MDL
#
# PURPOSE
#    Goes through the selected window, and determines which files the user
#  has selected for drawing.  Then converts those to "mosaic files".  Then
#  calls superimagegen to create images.  Then calls htmlmaker to create
#  a web page wrapper.  Finally calls internet explorer to view the pages.
#
# ARGUMENTS
#   rayName = Global array (structure) containing the main variables for
#             this program.
#
# RETURNS void
#
# HISTORY
#  9/2006 Arthur Taylor (MDL): Created
#
# NOTES
#   This should make the draw abilities available in version 1.80, also
# available in 1.83.  No new capability has been added.
#*****************************************************************************
proc DrawImages {rayName} {
  upvar #0 $rayName ray

  if {$ray(Cancel)} {
    CancelAction $rayName 2
    return
  } else {
    set ray(Action) Draw
  }

#####
# Figure out which season of colortables to use
#####
  set month [format "%.0f" [clock format $ray(now) -format "%m" -gmt true].0]
  if {($month > 5 ) && ($month < 9 )} {
     set season summer
  } elseif {($month > 11 ) && ($month < 3 )} {
     set season winter
  } else {
     set season springfall
  }

  set nodeList [$ray(lb,tree) selection get]
  set foreList ""
  set subList ""
  foreach node $nodeList {
    set nList [split $node _]
    set database [lindex $nList 0]
    set var [lindex $nList [expr [llength $nList] - 1]]
    if {$database == "ndfd"} {
      set tempList [split $var -]
      set f_subsector 0
      if {[lindex $tempList 0] == "custom"} {
        ns_Print::puts "Can't generate images for $database : $var"
      } else {
        if {[lindex $tempList 0] == "conus"} {
          set subsector [lindex $tempList 1]
          if {[lsearch $ray(subSector,LocalName) $subsector] != -1} {
            set f_subsector 1
          }
        }

        if {! $f_subsector} {
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
              ns_Print::puts "Can't generate images for custom folder"
            } else {
              ns_Print::puts "$var does not exist"
            }
          } else {
            lappend foreList $var
          }
        } else {
          lappend subList $var
        }
      }
    } elseif {$database == "ndgd"} {
      ns_Print::puts "Can't generate images for $database"
    } else {
      ns_Print::puts "Can't generate images for $database"
    }
  }

  set percent 5
  ns_Util::DrawXPerc $ray(DoneCanv) $percent

  # Check that we are capable of drawing this variable.
  # and that the file has been downloaded.
  set capableList [list maxt mint pop12 temp td wx qpf snow wdir wspd waveh sky apt rhm]
  set foreList2 ""
  set sectList ""
  foreach var $foreList {
    set oconus [lindex [split $var -] 0]
    set type [lindex [split $var -] 1]
    if {$oconus == "ak"} {
      ns_Print::puts "Can't generate images for Alaska"
      continue
    }
    if {[lsearch $capableList $type] != -1} {
      set localName [file join $ray(dir,NDFD_OpnlData) [lindex $ray(foreVar,$var) 2]]
      if {[file exists $localName]} {
        lappend foreList2 $var
        # Find refTime of var
        set refTime [lindex [split [lindex $ray(foreVar,$var) 5] :] 1]
        # refTime should not be 0 (since we have a file)
        if {[lsearch $sectList $oconus] == -1} {
          lappend sectList $oconus
          set refRay(refTime,$oconus) $refTime
        } else {
          if {$refTime < $refRay(refTime,$oconus)} {
            set refRay(refTime,$oconus) $refTime
          }
        }
      } else {
        ns_Print::puts "You need to download data for $var"
      }
    } else {
      ns_Print::puts "Can't generate images for $type"
    }
  }
  set subList2 ""
  foreach sub $subList {
    set tempList [split $sub -]
    set type [lindex $tempList 2]
    if {[lsearch $capableList $type] != -1} {
      set var [join [lreplace $tempList 1 1] -]
      set sector [lindex $tempList 1]
      set local [string replace [lindex $ray(foreVar,$var) 2] 0 4 $sector]
      set localName [file join $ray(dir,NDFD_OpnlData) $local]
      if {[file exists $localName]} {
        lappend subList2 $sub
        # Find refTime of subsector
        set index [lsearch $ray(subSector,LocalName) $sector]
        set refTime [lindex [split [lindex $ray(subSector,$var) $index] :] 1]
        # refTime should not be 0 (since we have a file)
        if {[lsearch $sectList $sector] == -1} {
          lappend sectList $sector
          set refRay(refTime,$sector) $refTime
        } else {
          if {$refTime < $refRay(refTime,$sector)} {
            set refRay(refTime,$sector) $refTime
          }
        }
      } else {
        ns_Print::puts "You need to download data for $sub"
      }
    } else {
      ns_Print::puts "Can't generate images for $type"
    }
  }

  set percent 10
  ns_Util::DrawXPerc $ray(DoneCanv) $percent
  if {([llength $foreList2] == 0) && ([llength $subList2] == 0)} {
    set percent 100
    ns_Util::DrawXPerc $ray(DoneCanv) $percent
    return
  }

  # Clean the mosaic / sector directories.
  if {! [file exists $ray(dir,Mosaic)]} {
    file mkdir $ray(dir,Mosaic)
  } else {
    foreach file [glob -nocomplain [file join $ray(dir,Mosaic) *.*]] {
      catch {file delete $file}
    }
  }

  # "Create Mosaics for $foreList2 and subList2"
  ns_Print::puts "----------"
  ns_Print::puts "Creating 'Mosaics'"

  #### Set up ImageGen's sector naming convention.
  set imgGen(sector,conus) conus
  set imgGen(sector,pr) sju
  set imgGen(sector,gu) guam
  set imgGen(sector,hi) hawaii
  set imgGen(sector,pacnwest) pacnorthwest
  set imgGen(sector,pacswest) pacsouthwest
  set imgGen(sector,nrockies) northrockies
  set imgGen(sector,crrocks) centrockies
  set imgGen(sector,srockies) southrockies
  set imgGen(sector,nplains) northplains
  set imgGen(sector,crplains) centplains
  set imgGen(sector,splains) southplains
  set imgGen(sector,umissvly) uppermissvly
  set imgGen(sector,crmissvy) centmissvly
  set imgGen(sector,smissvly) southmissvly
  set imgGen(sector,crgrlake) centgrtlakes
  set imgGen(sector,ergrlake) eastgrtlakes
  set imgGen(sector,neast) northeast
  set imgGen(sector,seast) southeast
  set imgGen(sector,midatlan) midatlantic

  set imgGen(map,conus) ""
  set imgGen(map,pr) sju
  set imgGen(map,gu) guam
  set imgGen(map,hi) hawaii
  set imgGen(map,pacnwest) ""
  set imgGen(map,pacswest) ""
  set imgGen(map,nrockies) ""
  set imgGen(map,crrocks) ""
  set imgGen(map,srockies) ""
  set imgGen(map,nplains) ""
  set imgGen(map,crplains) ""
  set imgGen(map,splains) ""
  set imgGen(map,umissvly) ""
  set imgGen(map,crmissvy) ""
  set imgGen(map,smissvly) ""
  set imgGen(map,crgrlake) ""
  set imgGen(map,ergrlake) ""
  set imgGen(map,neast) ""
  set imgGen(map,seast) ""
  set imgGen(map,midatlan) ""

  set imgGen(shp,conus) states.shp
  set imgGen(shp,pr) states_pr.shp
  set imgGen(shp,gu) states_gu.shp
  set imgGen(shp,hi) states_hi.shp
  set imgGen(shp,pacnwest) states.shp
  set imgGen(shp,pacswest) states.shp
  set imgGen(shp,nrockies) states.shp
  set imgGen(shp,crrocks) states.shp
  set imgGen(shp,srockies) states.shp
  set imgGen(shp,nplains) states.shp
  set imgGen(shp,crplains) states.shp
  set imgGen(shp,splains) states.shp
  set imgGen(shp,umissvly) states.shp
  set imgGen(shp,crmissvy) states.shp
  set imgGen(shp,smissvly) states.shp
  set imgGen(shp,crgrlake) states.shp
  set imgGen(shp,ergrlake) states.shp
  set imgGen(shp,neast) states.shp
  set imgGen(shp,seast) states.shp
  set imgGen(shp,midatlan) states.shp

  set percent 15
  ns_Util::DrawXPerc $ray(DoneCanv) $percent

  set fract [expr 40 / ([llength $foreList2] + [llength $subList2])]
  foreach var $foreList2 {
    set tempList [split $var -]
    set sector [lindex $tempList 0]
    set localName [file join $ray(dir,NDFD_OpnlData) [lindex $ray(foreVar,$var) 2]]
    ns_Print::puts "extracting from $localName"
    if {[catch {$ray(GRIB2Cmd) -C -in $localName -msg 0 -nameStyle 3 \
                -SimpleWx true -namePath $ray(dir,Mosaic) -nMSB -Flt -revFlt -Unit e \
                -nShp -nMet} ans]} {
      ns_Print::puts "$ans"
    }
    foreach file [glob -nocomplain [file join $ray(dir,Mosaic) *.tlf]] {
      file rename $file "[file rootname $file]_$imgGen(sector,$sector).mosaic"
    }
    set percent [expr $percent + $fract]
    ns_Util::DrawXPerc $ray(DoneCanv) $percent
  }
  foreach sub $subList2 {
    set tempList [split $sub -]
    set var [join [lreplace $tempList 1 1] -]
    set sector [lindex $tempList 1]
    set local [string replace [lindex $ray(foreVar,$var) 2] 0 4 $sector]
    set localName [file join $ray(dir,NDFD_OpnlData) $local]
    ns_Print::puts "extracting from $localName"
    if {[catch {$ray(GRIB2Cmd) -C -in $localName -msg 0 -nameStyle 3 \
                -SimpleWx true -namePath $ray(dir,Mosaic) -nMSB -Flt -revFlt -Unit e \
                -nShp -nMet} ans]} {
      ns_Print::puts "$ans"
    }
    foreach file [glob -nocomplain [file join $ray(dir,Mosaic) *.tlf]] {
      file rename $file "[file rootname $file]_$imgGen(sector,$sector).mosaic"
    }
    set percent [expr $percent + $fract]
    ns_Util::DrawXPerc $ray(DoneCanv) $percent
  }
  set percent 55
  ns_Util::DrawXPerc $ray(DoneCanv) $percent

  ns_Print::puts "----------"
  ns_Print::puts "Start Creating images"

  set numSect [llength $sectList]
  foreach sect $sectList {
    set varList ""
    set ctList ""
    set f_start 0
#    ns_Print::puts "$ray(dir,Mosaic) *_$imgGen(sector,$sect).mosaic"
    set fileList [glob -nocomplain [file join $ray(dir,Mosaic) *_$imgGen(sector,$sect).mosaic]]
    set fract [expr (25 / ($numSect * [llength $fileList] + 0.0))]
    set origPercent $percent
    foreach file $fileList {
      set tmpList [split [file tail [file rootname $file]] "_"]
      set var [lindex $tmpList 0]
      set valTime [lindex $tmpList 1]
      if {! $f_start} {
        set begTime $valTime
        set endTime $valTime
        set f_start 1
      } else {
        if {$valTime < $begTime} {
          set begTime $valTime
        }
        if {$valTime > $endTime} {
          set endTime $valTime
        }
      }
      if {[lsearch $varList $var] == -1} {
        lappend varList $var
        # Check existance of colortable.
        if {($var != "WindDir") && ($var != "WindSpd")} {
          set fileName [file join $ray(imgGen,NDFD_COLOR) [join [list $var $season.colortable] _]]
          if {! [file exists $fileName]} {
            set fileName [file join $ray(imgGen,NDFD_COLOR) $var.colortable]
            if {! [file exists $fileName]} {
              ns_Print::puts "!! Couldn't find default colorTable : $fileName"
              lappend ctList ""
            } else {
              lappend ctList [file tail $fileName]
            }
          } else {
            lappend ctList [file tail $fileName]
          }
        } else {
          lappend ctList ""
        }
      }
    }
    if {$f_start} {
      # Clean up any old image files.
      foreach file [glob -nocomplain [file join $ray(imgGen,WEB_IMAGE_PATH) *_$imgGen(sector,$sect).png]] {
        file delete $file
      }

      if {$imgGen(map,$sect) == ""} {
#        ns_Print::puts "-sec $imgGen(sector,$sect) -w $ray(opt,ImgWidth) \
#            -elem [join $varList ,] -ct [join $ctList ,] -stime $begTime \
#            -etime $endTime -shp $imgGen(shp,$sect) -defw $ray(opt,ImgWidth) \
#            -subg -img %e_%t_%s"
        set fp [open "|{[file join $ray(dir,Bin) superimageGen]} \
            -sec $imgGen(sector,$sect) -w $ray(opt,ImgWidth) \
            -elem [join $varList ,] -ct [join $ctList ,] -stime $begTime \
            -etime $endTime -shp $imgGen(shp,$sect) -defw $ray(opt,ImgWidth) \
            -subg -img %e_%t_%s" r]
      } else {
#        ns_Print::puts "-sec $imgGen(sector,$sect) -w $ray(opt,ImgWidth) \
#            -elem [join $varList ,] -ct [join $ctList ,] -stime $begTime \
#            -etime $endTime -map $imgGen(map,$sect) -shp $imgGen(shp,$sect) \
#            -defw $ray(opt,ImgWidth) -subg -img %e_%t_%s"

        set fp [open "|{[file join $ray(dir,Bin) superimageGen]} \
            -sec $imgGen(sector,$sect) -w $ray(opt,ImgWidth) \
            -elem [join $varList ,] -ct [join $ctList ,] -stime $begTime \
            -etime $endTime -map $imgGen(map,$sect) -shp $imgGen(shp,$sect) \
            -defw $ray(opt,ImgWidth) -subg -img %e_%t_%s" r]
      }
      while {[gets $fp line] >= 0} {
        ns_Print::puts $line
        set percent [expr $percent + $fract]
        ns_Util::DrawXPerc $ray(DoneCanv) $percent
        update
      }
      close $fp
    }

    set percent [expr $origPercent + 25 / $numSect]
    ns_Util::DrawXPerc $ray(DoneCanv) $percent
    ns_Print::puts "----------"
    ns_Print::puts "Generating: $imgGen(sector,$sect).html in $ray(imgGen,HTML_DIR)"
    set refTime [clock format $refRay(refTime,$sect) -format "%Y%m%d%H" -gmt true]
    ns_Print::puts "with a reference date of $refTime GMT"
    exec [file join $ray(dir,Bin) htmlmaker] $imgGen(sector,$sect) $refTime [join $varList ,]
    set percent [expr $percent + 10 / $numSect]
    ns_Util::DrawXPerc $ray(DoneCanv) $percent

  ##### Start Browser #####
    ns_Print::puts "----------"
    ns_Print::puts "Starting Browser: $ray(opt,Browser)"
    exec $ray(opt,Browser) [file join $ray(imgGen,HTML_DIR) $imgGen(sector,$sect).html] &
    set percent [expr $percent + 10 / $numSect]
    ns_Util::DrawXPerc $ray(DoneCanv) $percent
  }

  set percent 100
  ns_Util::DrawXPerc $ray(DoneCanv) $percent
}
