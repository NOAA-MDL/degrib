set lcl_srcDir [file dirname [info script]]
if {[file pathtype $lcl_srcDir] != "absolute"} {
  set    cur_dir [pwd] ;  cd $lcl_srcDir
  set lcl_srcDir [pwd] ;  cd $cur_dir
}


image create photo pNo -file [file join $lcl_srcDir icons Nodes.gif]

image create photo pFO -file [file join $lcl_srcDir icons Folder32.gif]
image create photo pFo -file [file join $lcl_srcDir icons Folder.gif]
image create photo pFI -file [file join $lcl_srcDir icons File32.gif]
image create photo pFi -file [file join $lcl_srcDir icons File.gif]
image create photo pGRIB2 -file [file join $lcl_srcDir icons grib2.gif]
image create photo pGrib2 -file [file join $lcl_srcDir icons grib2s.gif]
image create photo pGRIB1 -file [file join $lcl_srcDir icons grib1.gif]
image create photo pGrib1 -file [file join $lcl_srcDir icons grib1s.gif]
image create photo pTDLP -file [file join $lcl_srcDir icons tdlp.gif]
image create photo pTdlp -file [file join $lcl_srcDir icons tdlps.gif]
image create photo pTXT -file [file join $lcl_srcDir icons txtfile.gif]
image create photo pTxt -file [file join $lcl_srcDir icons txtfiles.gif]
image create photo pUp -file [file join $lcl_srcDir icons UpFolder.gif]
image create photo pSmall -file [file join $lcl_srcDir icons Small.gif]
image create photo pLarge -file [file join $lcl_srcDir icons Large.gif]
image create photo pListControl -file [file join $lcl_srcDir icons Listcontrol.gif]

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

proc DisplayTxt {rayName file} {
  upvar #0 $rayName ray
  if {! [winfo ismapped $ray(GRIB2,txtBrowse)]} {
    pack forget $ray(GRIB2,gribBrowse)
    pack $ray(GRIB2,txtBrowse) -expand yes -fill both
  }
  set frame $ray(GRIB2,txtBrowse)
  $frame.top config -text $file
  set txt $frame.txt.text
  $txt delete 1.0 end
  set fp [open $file r]
  while {[gets $fp line] >= 0} {
    $txt insert end $line
    $txt insert end "\n"
  }
  close $fp
  $txt see 1.0
}

proc InventoryGRIB2 {rayName file f_unknown} {
  upvar #0 $rayName ray
  if {! [winfo ismapped $ray(GRIB2,gribBrowse)]} {
    pack forget $ray(GRIB2,txtBrowse)
    pack $ray(GRIB2,gribBrowse) -expand yes -fill both
  }
  set cur $ray(GRIB2,gribBrowse).mid

  # Get an inventory of the gribFile
  if {[catch {$ray(GRIB2Cmd) -I -in $file} ans]} {
    if {$f_unknown} {
      tk_messageBox -message "File: $file\n was not a GRIB2\
                              file.\nTreating it as a text file"
      DisplayTxt $rayName $file
      return
    } else {
      tk_messageBox -message "File $file appears corrupt?"
      return
    }
  }
  set ray(gribFile) $file
  # Remove old elements from the list boxes.
  foreach lb $ray(LB_list) {
    $lb delete 0 end
  }

  for {set i 0} {$i < [llength $ans]} {incr i} {
    set elem [lindex $ans $i]
    $cur.a.num.lb insert end [lindex $elem 0]
    $cur.a.a.elem.lb insert end [lindex $elem 1]
    $cur.a.a.a.long.lb insert end [lindex $elem 6]
    $cur.a.a.a.a.level.lb insert end [lindex $elem 7]
    $cur.a.a.a.a.a.refDate.lb insert end [lindex $elem 2]
    $cur.a.a.a.a.a.a.valDate.lb insert end [lindex $elem 3]
    $cur.a.a.a.a.a.a.proj.lb insert end [format "%.4f" [lindex $elem 4]]
  }
}

global AAT_Custom
set AAT_Custom 0
proc PaneValid {w} {
  set rayName NDFD_Ray
  global AAT_Custom
  upvar #0 $rayName ray
  if {$AAT_Custom == 0} {
    tk_messageBox -message "Custom! :-)"
    set var custom
    set value [split "Custom,*,custom" ,]
    set ray(sct,$var) $value
    if {$var != "default"} {
      lappend ray(SectorList) $var
    }
    if {[lindex $value 1] == "*"} {
      if {! [file exists [file join $ray(dir,NDFD_OpnlData) $var]]} {
        file mkdir [file join $ray(dir,NDFD_OpnlData) $var]
      }
    }
    set ray(custom,URL) http://ndfd.weather.gov/cgi-bin/ndfd/gribcut
    set ray(custom,lat1) 30
    set ray(custom,lon1) -100
    set ray(custom,lat2) 40
    set ray(custom,lon2) -90
    NDFD_SaveINI $rayName
    NDFD_ReadIni $rayName
    MainRefresh $rayName
    set AAT_Custom 1
  }
}

proc NotifyUser {} {
   global FUN
   set lst [list \
         "Thank you for flying Arthur Airlines" \
         "Polly want a cracker?" \
         "6*9 = 42 (b13) -Arthur Dent" \
         "Be Seeing You. #6" \
         "Th..Th..Th..Th.. That's All folks!" \
         "Supercalifragilisticexpialadocious" \
         "Antidisestablishmentarianism" \
         "Early to bed, early to rise...\nFranklin didn't do this in Paris" \
         "When you age,\n the second thing to go is your memory\nI forget the first." \
         "Doe, a deer, a female deer, ray a drop of golden sun" \
         "Hi-hoe Hi-hoe, its off to work I go." \
         "There is no gravity, mass just sucks" \
         "Theorem 1: Never trust anyone over 30." \
         "Corollary 1: Never trust anyone under 30." \
         "Only certainty in life: Death and taxes." \
         "C Program, C Program Run, C Program Run Please! " \
         "Ribbit."]
   if {$FUN} {
      set num [expr int (rand() * [expr [llength $lst] -1] + .5)]
      tk_messageBox -message [lindex $lst $num]
   } else {
      tk_messageBox -message "Done."
   }
}

proc Grib2Convert {rayName type} {
  upvar #0 $rayName ray
  set lb [lindex $ray(LB_list) 0]
  if {[$lb index end] == 0} {
    tk_messageBox -message "Please select a GRIB2 file first."
    return
  }
  set index [$lb curselection]
  if {$index == ""} {
    tk_messageBox -message "Please select a GRIB2 record / message first."
    return
  }
  set msgNum [$lb get $index]
  if {$ray(majEarth) != 0} {
    if {($ray(majEarth) < 6000) || ($ray(majEarth) > 7000)} {
      tk_messageBox -message "Radius of earth should be between 6000km and 7000km\nIgnoring $ray(majEarth)"
    }
  }
  if {$ray(minEarth) != 0} {
    if {($ray(minEarth) < 6000) || ($ray(minEarth) > 7000)} {
      tk_messageBox -message "Radius of earth should be between 6000km and 7000km\nIgnoring $ray(minEarth)"
    }
  }
  # Get the select listbox values.
  set ray(f_unit) [$ray(unitLB) index active]
  if {$ray(f_unit) == 0} {
    set ray(f_unit) n
  } elseif {$ray(f_unit) == 1} {
    set ray(f_unit) e
  } elseif {$ray(f_unit) == 2} {
    set ray(f_unit) m
  }
  set ray(f_Interp) [$ray(interpLB) index active]
  set ray(f_simpleVers) [expr [$ray(simpWxLB) index active] + 1]
  set ray(NetCDF_Version) [expr [$ray(NetCDF_LB) index active] + 1]

  if {! [file isdirectory [file dirname $ray(outFile)]]} {
    file mkdir [file dirname $ray(outFile)]
  }
  if {$ray(f_Missing)} {
    set nMissing 0
  } else {
    set nMissing 1
  }

  # They want flt files.
  set f_AscGrid 0
  if {$type == 5} {
    set type 1
    set f_AscGrid 1
  }

  set outFile $ray(outFile)
  # Remember [file extension $outFile] counts the '.'
  if {[string length [file extension $outFile]] == 3} {
     set outFile [file rootname $outFile].ncf
  }

  if {$type == 1} {
    $ray(GRIB2Cmd) -C -in $ray(gribFile) -Flt true -Met true -Shp false \
                  -msg $msgNum -out $outFile -Interp $ray(f_Interp) \
                  -MSB $ray(f_msb) -Unit $ray(f_unit) -Decimal $ray(decimal) \
                  -GrADS $ray(f_GrADS) -SimpleWx $ray(f_simpleWx) -SimpleVer $ray(f_simpleVers) \
                  -revFlt $ray(f_revFlt) -majEarth $ray(majEarth) \
                  -minEarth $ray(minEarth) -AscGrid $f_AscGrid

    if {$ray(f_GrADS) && (! $ray(f_Interp))} {
      tk_messageBox -message "Please Note: GrADS may run slowly if it has to resample \n\
                              (ie interpolate) the grid.\n\
                             \n\
                             You may want to use the 'Grid: Coverage' options, \n\
                             so we do the resampling (interpolation) instead of GrADS."
    }
  } elseif {$type == 2} {
    $ray(GRIB2Cmd) -C -in $ray(gribFile) -Flt false -Met true -Shp true \
                  -msg $msgNum -out $outFile -poly $ray(f_poly) \
                  -nMissing $nMissing -Unit $ray(f_unit) -Decimal $ray(decimal) \
                  -majEarth $ray(majEarth) -minEarth $ray(minEarth) -verboseShp $ray(f_verboseShp)
  } elseif {$type == 3} {
#    if {($ray(NetCDF_Version) != 1) && ($ray(NetCDF_Version) != 2) && ($ray(NetCDF_Version) != 3)} {
#       tk_messageBox -message "Currently degrib only has 3 versions of NetCDF"
#    }
    if {! $ray(f_append)} {
       file delete -force [file rootname $outFile].nc
    }
    $ray(GRIB2Cmd) -C -in $ray(gribFile) -Flt false -Met true -NetCDF $ray(NetCDF_Version) \
                  -msg $msgNum -out $outFile \
                  -Unit $ray(f_unit) -Decimal $ray(decimal) \
                  -majEarth $ray(majEarth) -minEarth $ray(minEarth)
  } elseif {$type == 4} {
    $ray(GRIB2Cmd) -C -in $ray(gribFile) -Flt false -Met true -Csv true \
                  -msg $msgNum -out $outFile \
                  -nMissing $nMissing -Unit $ray(f_unit) -Decimal $ray(decimal) \
                  -majEarth $ray(majEarth) -minEarth $ray(minEarth)
  } else {
    tk_messageBox -message "Invalid call to Grib2Convert."
  }
  NotifyUser
  return
}


proc SelectIcon {rayName frame} {
  upvar #0 $rayName ray

  set sPath [lindex [$ray($frame,ibox) selection get] 0]
  if {[file isdirectory $sPath]} {
    set $ray($frame,path) "$sPath"
    $ray($frame,tree) expand "$sPath"
    DirUpdateIbox $rayName $frame "$sPath" NULL NULL 0
  } else {
    set type [$ray($frame,ibox) iconcget $sPath -image]
    if {$type == "pTXT"} {
      set $ray($frame,path) $sPath
      DisplayTxt $rayName $sPath
    } elseif {($type == "pGRIB2") || ($type == "pGRIB1") || ($type == "pTDLP")} {
      set $ray($frame,path) $sPath
      InventoryGRIB2 $rayName $sPath 0
    } else {
      set $ray($frame,path) $sPath
      InventoryGRIB2 $rayName $sPath 1
    }
  }
}

proc SelectTree {rayName frame} {
  upvar #0 $rayName ray

  set sPath [lindex [$ray($frame,tree) selection get] 0]
  if {[file isdirectory $sPath]} {
    set $ray($frame,path) "$sPath"
    $ray($frame,tree) expand "$sPath"
    DirUpdateIbox $rayName $frame "$sPath" NULL NULL 0
  }
}


#*****************************************************************************
# GetOutFile --
#
#    Calls the built in tk_getOpenFile to get a file to save or read from.
#
# Arguments:
# rayName = Name of the Main global Array used by this program.
#
# Globals:
# $rayName = (see top of program for description of contents.)
#
# History:
#  2/2003 Arthur Taylor (RSIS/MDL): Created
#
# Notes:
#*****************************************************************************
proc GetOutFile {rayName} {
   upvar #0 $rayName ray

   if {$ray(type) == "shp"} {
      set pattern  [list [list "Shp Files" ".shp"] [list "All Files" "*"]]
      set file [tk_getSaveFile -initialdir [file dirname $ray(outFile)] \
                -initialfile [file tail $ray(outFile)] \
                -defaultextension .shp -filetypes $pattern]
   } elseif {$ray(type) == "flt"} {
      set pattern  [list [list "Flt Files" ".flt"] [list "All Files" "*"]]
      set file [tk_getSaveFile -initialdir [file dirname $ray(outFile)] \
                -initialfile [file tail $ray(outFile)] \
                -defaultextension .flt -filetypes $pattern]
   } else {
      set pattern  [list [list "Csv Files" ".csv"] [list "All Files" "*"]]
      set file [tk_getSaveFile -initialdir [file dirname $ray(outFile)] \
                -initialfile [file tail $ray(outFile)] \
                -defaultextension .csv -filetypes $pattern]
   }
   if {$file == ""} {
      return
   }
   set ray(outFile) $file
   return
}

proc GetRecommendFile {rayName} {
  upvar #0 $rayName ray

  set lb $ray(GRIB2,gribBrowse).mid.a.a.a.a.a.a.valDate.lb
  set elemLb $ray(GRIB2,gribBrowse).mid.a.a.elem.lb
  if {[$lb index end] == 0} {
    set ray(outFile) [file join $ray(dir,Output) sample]
  } else {
    set index [$lb curselection]
    set subDir [file tail [file dirname $ray(gribFile)]]
    if {[llength $subDir] > 1} {
       set subDir [join $subDir _]
    }
    if {$index == ""} {
       set name [file rootname [file tail $ray(gribFile)]]
       set ray(outFile) [file join $ray(dir,Output) $subDir $name]
    } else {
       set validTime [$lb get $index]
       set elem [string tolower [$elemLb get $index]]
       set time [clock scan $validTime -gmt true]
#       set name [file rootname [file tail $ray(gribFile)]]
       set name $elem
       set name2 [clock format $time -format "%Y%m%d%H%M" -gmt true]
       set ray(outFile) [file join $ray(dir,Output) $subDir $name2\_$name]
       if {($elem == "qpf")} {
         set ray(decimal) 2
       } elseif {($elem == "snowamt") || ($elem == "waveheight")} {
         set ray(decimal) 1
       } else {
         set ray(decimal) 0
       }
    }
  }
  if {$ray(type) == "shp"} {
    set ray(outFile) "$ray(outFile).shp"
  } elseif {$ray(type) == "flt"} {
    set ray(outFile) "$ray(outFile).flt"
  } elseif {$ray(type) == "netcdf"} {
    set ray(outFile) "$ray(outFile).nc"
  } else {
    set ray(outFile) "$ray(outFile).csv"
  }
}

proc GRIB2_ListBox {rayName fr txt wid} {
  upvar #0 $rayName ray

  set cur1 [frame $fr]
    label $cur1.lab -text $txt
#    listbox $cur1.lb -bg #cdb79e -width $wid
    listbox $cur1.lb -bg #ffffe6 -width $wid
    lappend ray(LB_list) $cur1.lb
    pack $cur1.lab -side top
    pack $cur1.lb -side top -expand yes -fill both
}

proc ScrollListBox {lb incr} {
  set index [expr [$lb nearest 0] + $incr]
  if {$index < 0} {
    set index 0
  } elseif {$index >= [$lb index end]} {
    set index [expr [$lb index end] - 1]
  }
  $lb see $index
  $lb activate $index
  $lb selection clear 0 end
}

proc updnButton {w} {
  frame $w
  canvas $w.up -width 12 -height 8 -borderwidth 2 -highlightthickness 0 -relief raised
  canvas $w.down -width 12 -height 7 -borderwidth 2 -highlightthickness 0 -relief raised
  pack $w.up
  pack $w.down
  $w.up create polygon 6 1 12 8 1 8 7 1 -fill #000000
  $w.down create polygon 6 8 12 1 1 1 7 8 -fill #000000
  return $w
}

proc GIS_Switch {tl rayName val} {
  upvar #0 $rayName ray
  set ray(type) $val
  if {$ray(type) == "flt"} {
    pack forget $tl.shp
    pack forget $tl.netcdf
    pack forget $tl.csv
    pack forget $tl.gen
    pack $tl.flt -side left -expand yes -fill both
#    $tl.lf.options configure -text "FLT"
  } elseif {$ray(type) == "shp"} {
    pack forget $tl.flt
    pack forget $tl.netcdf
    pack forget $tl.csv
    pack forget $tl.gen
    pack $tl.shp -side left -expand yes -fill both
#    $tl.lf.options configure -text "SHP"
  } elseif {$ray(type) == "netcdf"} {
    pack forget $tl.flt
    pack forget $tl.csv
    pack forget $tl.gen
    pack forget $tl.shp
    pack $tl.netcdf -side left -expand yes -fill both
#    $tl.lf.options configure -text "NetCDF"
  } else {
    pack forget $tl.flt
    pack forget $tl.shp
    pack forget $tl.netcdf
    pack forget $tl.gen
    pack $tl.csv -side left -expand yes -fill both
#    $tl.lf.options configure -text "CSV"
  }
  pack $tl.gen -side left -fill y
}

proc Build_DoItFrame {tl rayName} {
  upvar #0 $rayName ray

  set ray(decimal) 0
  set ray(majEarth) 0
  set ray(minEarth) 0
  set ray(type) shp
  set ray(f_poly) 1
  set ray(f_Missing) 0
  set ray(f_verboseShp) 0
  set ray(f_msb) 1
  set ray(f_GrADS) 0
  set ray(f_simpleWx) 0
  set ray(f_simpleVers) 4
  set ray(f_revFlt) 0
  set ray(f_Interp) 0

#  set cur [frame $tl.lf]
#    label $cur.lab -text "Choose File Type: "
#    menubutton $cur.options -text "SHP" -direction right -menu $cur.options.m -relief raised -indicatoron true
#    menu $cur.options.m -tearoff 0
#    $cur.options.m add command -label "SHP" -command "GIS_Switch $tl $rayName shp"
#    $cur.options.m add command -label "FLT" -command "GIS_Switch $tl $rayName flt"
#    $cur.options.m add command -label "NetCDF" -command "GIS_Switch $tl $rayName netcdf"
#    $cur.options.m add command -label "CSV" -command "GIS_Switch $tl $rayName csv"
#    pack $cur.lab -expand yes -fill both
#    pack $cur.options
  set cur0 [frame $tl.flt -relief ridge -bd 5]
   set curA [frame $cur0.fr]
    set cur [frame $curA.lf]
      label $cur.lab -text "Choose File Type: "
      menubutton $cur.options -text "FLT" -direction right -menu $cur.options.m -relief raised -indicatoron true
      menu $cur.options.m -tearoff 0
      $cur.options.m add command -label "SHP" -command "GIS_Switch $tl $rayName shp"
      $cur.options.m add command -label "FLT" -command "GIS_Switch $tl $rayName flt"
      $cur.options.m add command -label "NetCDF" -command "GIS_Switch $tl $rayName netcdf"
      $cur.options.m add command -label "CSV" -command "GIS_Switch $tl $rayName csv"
      pack $cur.lab -expand yes -fill both
      pack $cur.options
    set cur [frame $curA.rt]
     set cur1 [frame $cur.cover -relief ridge -bd 2]
      label $cur1.lab -text "Grid: "
      listbox $cur1.lb -height 1 -bg white -width 25
      $cur1.lb insert end "Projected: Original GRIB"
      $cur1.lb insert end "Coverage: Nearest Point"
      $cur1.lb insert end "Coverage: Bi-Linear"
      set ray(interpLB) $cur1.lb
      updnButton $cur1.updn
      bind $cur1.lb <Button-1> "+ focus $cur1.lb"
      bind $cur1.updn.up <Button-1> "ScrollListBox $cur1.lb -1"
      bind $cur1.updn.down <Button-1> "ScrollListBox $cur1.lb +1"
      $cur1.lb activate 0
      $cur1.lb see 0
      checkbutton $cur1.msb -text "M.S.B. First" -variable $rayName\(f_msb)
      pack $cur1.lab $cur1.lb $cur1.updn $cur1.msb -side left
    set cur1 [frame $cur.extra]
      checkbutton $cur1.grads -text "Create GrADS .ctl file" -relief ridge -variable $rayName\(f_GrADS)
      checkbutton $cur1.revFlt -text "Start at Lower Left (.tlf)" -relief ridge -variable $rayName\(f_revFlt)
      pack $cur1.grads $cur1.revFlt -side left
    set cur1 [frame $cur.extra2 -relief ridge -bd 2]
      checkbutton $cur1.wx -text "use NDFD Weather code" -variable $rayName\(f_simpleWx)
#      label $cur1.lab -text "Version: "
      listbox $cur1.lb -height 1 -bg white -width 17
      $cur1.lb insert end "Version 1  (6/2003)"
      $cur1.lb insert end "Version 2  (1/2004)"
      $cur1.lb insert end "Version 3  (2/2004)"
      $cur1.lb insert end "Version 4  (11/2004)"
      set ray(simpWxLB) $cur1.lb
      updnButton $cur1.updn
      bind $cur1.lb <Button-1> "+ focus $cur1.lb"
      bind $cur1.updn.up <Button-1> "ScrollListBox $cur1.lb -1"
      bind $cur1.updn.down <Button-1> "ScrollListBox $cur1.lb +1"
      $cur1.lb activate 3
      $cur1.lb see 3
#      pack $cur1.wx $cur1.lab $cur1.lb $cur1.updn -side left
      pack $cur1.wx $cur1.lb $cur1.updn -side left
    set cur1 [frame $cur.but]
      button $cur1.but1 -text "Generate .flt file" -bd 4 -command "Grib2Convert $rayName 1"
      button $cur1.but2 -text "Generate .asc file" -bd 4 -command "Grib2Convert $rayName 5"
      pack $cur1.but1 $cur1.but2 -side left -fill x -expand yes
    pack $cur.cover $cur.extra $cur.extra2 -side top
    pack $cur.but -side top -fill x
   pack $curA.lf $curA.rt -side left
   pack $cur0.fr -side top
  set cur0 [frame $tl.shp -relief ridge -bd 5]
   set curA [frame $cur0.fr]
    set cur [frame $curA.lf]
      label $cur.lab -text "Choose File Type: "
      menubutton $cur.options -text "SHP" -direction right -menu $cur.options.m -relief raised -indicatoron true
      menu $cur.options.m -tearoff 0
      $cur.options.m add command -label "SHP" -command "GIS_Switch $tl $rayName shp"
      $cur.options.m add command -label "FLT" -command "GIS_Switch $tl $rayName flt"
      $cur.options.m add command -label "NetCDF" -command "GIS_Switch $tl $rayName netcdf"
      $cur.options.m add command -label "CSV" -command "GIS_Switch $tl $rayName csv"
      pack $cur.lab -expand yes -fill both
      pack $cur.options
    set cur [frame $curA.rt]
     set cur1 [frame $cur.shptype]
      label $cur1.lab -text "Type of .shp file: " -bd 2 -relief flat
      set cur2 [frame $cur1.choice]
        radiobutton $cur2.pnt -text "Point" -relief groove -bd 2 \
              -variable $rayName\(f_poly) -value 0
        radiobutton $cur2.sml -text "Small Polygon" -relief groove -bd 2 \
              -variable $rayName\(f_poly) -value 1
        radiobutton $cur2.big -text "Large Polygon" -relief groove -bd 2 \
              -variable $rayName\(f_poly) -value 2
        pack $cur2.pnt $cur2.sml $cur2.big -side left
      pack $cur1.lab $cur1.choice -side top
    set cur1 [frame $cur.flags]
      checkbutton $cur1.verbose -text "Verbose .shp file" -relief ridge -variable $rayName\(f_verboseShp)
      checkbutton $cur1.miss -text "Include Missing Values" -relief ridge -variable $rayName\(f_Missing)
      pack $cur1.verbose $cur1.miss -side left
    button $cur.but -text "Generate .shp file" -bd 4 -command "Grib2Convert $rayName 2"
    pack $cur.shptype $cur.flags -side top
    pack $cur.but -side top -fill x
   pack $curA.lf $curA.rt -side left
   pack $cur0.fr -side top
  set cur0 [frame $tl.netcdf -relief ridge -bd 5]
   set curA [frame $cur0.fr]
    set cur [frame $curA.lf]
      label $cur.lab -text "Choose File Type: "
      menubutton $cur.options -text "NetCDF" -direction right -menu $cur.options.m -relief raised -indicatoron true
      menu $cur.options.m -tearoff 0
      $cur.options.m add command -label "SHP" -command "GIS_Switch $tl $rayName shp"
      $cur.options.m add command -label "FLT" -command "GIS_Switch $tl $rayName flt"
      $cur.options.m add command -label "NetCDF" -command "GIS_Switch $tl $rayName netcdf"
      $cur.options.m add command -label "CSV" -command "GIS_Switch $tl $rayName csv"
      pack $cur.lab -expand yes -fill both
      pack $cur.options
    set cur [frame $curA.rt]
    label $cur.lab -text "This creates a NetCDF file using the 'CF Metadata \n\
                          Convention Ver. 1.0, 28 Oct 2003'"
    set cur1 [frame $cur.opt -bd 4 -relief ridge]
      set cur2 [frame $cur1.ver]
        label $cur2.lab -text "degrib NetCDF-CF:"

        listbox $cur2.lb -height 1 -bg white -width 17
        $cur2.lb insert end "Version 1  (5/2004)"
        $cur2.lb insert end "Version 2  (12/2004)"
        $cur2.lb insert end "Version 3  (10/2005)"
        set ray(NetCDF_LB) $cur2.lb
        updnButton $cur2.updn
        bind $cur2.lb <Button-1> "+ focus $cur2.lb"
        bind $cur2.updn.up <Button-1> "ScrollListBox $cur2.lb -1"
        bind $cur2.updn.down <Button-1> "ScrollListBox $cur2.lb +1"

        set ray(NetCDF_Version) 3
        $cur2.lb activate 2
        $cur2.lb see 2

#        entry $cur2.ent -textvariable $rayName\(NetCDF_Version) -width 5
        pack $cur2.lab $cur2.lb $cur2.updn -side left
      set ray(f_append) 1
      checkbutton $cur1.append -text "Append to file? (Version 2, or 3)" -variable $rayName\(f_append)
      pack $cur1.ver $cur1.append -side top

    button $cur.but -text "Generate NetCDF (.nc) file" -command "Grib2Convert $rayName 3"
    pack $cur.lab -fill x -side top
    pack $cur.opt -side top -fill x
    pack $cur.but -side top -fill x
   pack $curA.lf $curA.rt -side left
   pack $cur0.fr -side top
  set cur0 [frame $tl.csv -relief ridge -bd 5]
   set curA [frame $cur0.fr]
    set cur [frame $curA.lf]
      label $cur.lab -text "Choose File Type: "
      menubutton $cur.options -text "CSV" -direction right -menu $cur.options.m -relief raised -indicatoron true
      menu $cur.options.m -tearoff 0
      $cur.options.m add command -label "SHP" -command "GIS_Switch $tl $rayName shp"
      $cur.options.m add command -label "FLT" -command "GIS_Switch $tl $rayName flt"
      $cur.options.m add command -label "NetCDF" -command "GIS_Switch $tl $rayName netcdf"
      $cur.options.m add command -label "CSV" -command "GIS_Switch $tl $rayName csv"
      pack $cur.lab -expand yes -fill both
      pack $cur.options
    set cur [frame $curA.rt]
    label $cur.lab -text "This creates a comma delimited text file \n\
                          which can be imported into a spreadsheet \n\
                          (as could the .dbf from the shp option)."
    checkbutton $cur.miss -text "Include Missing Values" -relief ridge -variable $rayName\(f_Missing)
    button $cur.but -text "Generate .csv file" -bd 4 -command "Grib2Convert $rayName 4"
    pack $cur.lab -fill x -side top
    pack $cur.miss -side top
    pack $cur.but -side top -fill x
   pack $curA.lf $curA.rt -side left
   pack $cur0.fr -side top
  set cur [frame $tl.gen -relief ridge -bd 5]
    label $cur.declab -text "Round data to:"
    set cur1 [frame $cur.decval]
      entry $cur1.ent -textvariable $rayName\(decimal) -width 10
      label $cur1.lab -text "decimals"
      pack $cur1.ent $cur1.lab -side left
    grid $cur.declab $cur.decval -sticky ew
    label $cur.unitlab -text "Units (when possible):"
    set cur1 [frame $cur.unitval]
      listbox $cur1.lb -height 1 -bg white
      $cur1.lb insert end "GRIB units (K, m/s)"
      $cur1.lb insert end "English (F, kts, inch)"
      $cur1.lb insert end "Metric (C, m/s, kg/m**2)"
      set ray(unitLB) $cur1.lb
      updnButton $cur1.updn
      bind $cur1.lb <Button-1> "+ focus $cur1.lb"
      bind $cur1.updn.up <Button-1> "ScrollListBox $cur1.lb -1"
      bind $cur1.updn.down <Button-1> "ScrollListBox $cur1.lb +1"
      $cur1.lb activate 1
      $cur1.lb see 1
#      $cur1.lb selection set 0
      pack $cur1.lb $cur1.updn -side left
    grid $cur.unitlab $cur.unitval -sticky ew
    label $cur.radlab -text "Force Major Earth Axis"
    set cur1 [frame $cur.radval]
      entry $cur1.ent -textvariable $rayName\(majEarth) -width 10
      label $cur1.lab -text "km (NCEP used 6371.2)"
      pack $cur1.ent $cur1.lab -side left
    grid $cur.radlab $cur.radval -sticky ew
    label $cur.rad2lab -text "Force Minor Earth Axis"
    set cur1 [frame $cur.rad2val]
      entry $cur1.ent -textvariable $rayName\(minEarth) -width 10
      label $cur1.lab -text "km (NCEP used 6371.2)"
      pack $cur1.ent $cur1.lab -side left
    grid $cur.rad2lab $cur.rad2val -sticky ew
#  if {$ray(type) == "flt"} {
#    pack $tl.lf -side left
#    pack $tl.flt -side left -expand yes -fill both
#    pack $tl.gen -side left -fill y
#  } else {
#    pack $tl.lf -side left
    pack $tl.shp -side left -expand yes -fill both
    pack $tl.gen -side left -fill y
#  }
}


#*****************************************************************************
#  GRIBFrame -- Arthur Taylor / MDL
#
# PURPOSE
#    Create the GRIB index frame
#
# ARGUMENTS
#   frame = Frame to add the GIS widgets to.
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
proc GRIBFrame {frame rayName} {
  upvar #0 $rayName ray

  ##### Create the middle part of the window. #####
  set cur [frame $frame.mid -relief ridge -bd 5]
    set ray(LB_list) ""
    set cur1 [frame $cur.a]
      GRIB2_ListBox $rayName $cur1.num "#" 6
      set cur2 [frame $cur1.a]
        GRIB2_ListBox $rayName $cur2.elem "Short name" 8
        set cur3 [frame $cur2.a]
          GRIB2_ListBox $rayName $cur3.long "Long name" 25
          set cur4 [frame $cur3.a]
            GRIB2_ListBox $rayName $cur4.level "Level" 13
            set cur5 [frame $cur4.a]
              GRIB2_ListBox $rayName $cur5.refDate "Ref. Date (UTC)" 13
              set cur6 [frame $cur5.a]
                GRIB2_ListBox $rayName $cur6.valDate "Valid Date (UTC)" 13
                GRIB2_ListBox $rayName $cur6.proj "Val-Ref(hr)" 7
                Pane_Create $cur6.valDate $cur6.proj -in $cur6 \
                      -orient horizontal -percent .6
              Pane_Create $cur5.refDate $cur5.a -in $cur5 -orient horizontal \
                    -percent .38
            Pane_Create $cur4.level $cur4.a -in $cur4 -orient horizontal \
                  -percent .3
          Pane_Create $cur3.long $cur3.a -in $cur3 -orient horizontal \
                -percent .28
        Pane_Create $cur2.elem $cur2.a -in $cur2 -orient horizontal -percent .1
      Pane_Create $cur1.num $cur1.a -in $cur1 -orient horizontal -percent .05

    set cur1 [frame $cur.scroll]
      label $cur1.lab -text ""
      scrollbar $cur1.yscroll -orient vertical \
            -command [list multi_scroll $ray(LB_list)]
      pack $cur1.lab -side top
      pack $cur1.yscroll -side top -expand yes -fill both
    pack $cur.a -side left -expand yes -fill both
    pack $cur.scroll -side left -expand no -fill y

  ##### Create binding for the lists #####
    foreach lb $ray(LB_list) {
      $lb configure -exportselection false -takefocus 1 \
            -yscrollcommand [list multi_scroll2 $cur1.yscroll \
                             $ray(LB_list)]
#      bind $lb <Enter> "+ focus $lb"
      bind $lb <ButtonRelease-1> \
            "+ AdjustSelectList $lb [list $ray(LB_list)] 0 ; focus $lb"
      bind $lb <B1-Motion> \
            "+ AdjustSelectList $lb [list $ray(LB_list)] 0 ; focus $lb"
      bind $lb <Up> "+ AdjustSelectList $lb [list $ray(LB_list)] -1"
      bind $lb <Down> "+ AdjustSelectList $lb [list $ray(LB_list)] 1"
    }

  ##### Create the bottom part of the window. #####
  set val [frame $frame.bot]
    set cur [frame $val.outFile -relief ridge -bd 4]
      label $cur.lab -text "OUTPUT Filename: "
      entry $cur.ent -textvariable $rayName\(outFile)
      button $cur.but -text "Browse" -command "GetOutFile $rayName" -bd 5
      button $cur.but2 -text "Recommend" -command "GetRecommendFile $rayName" -bd 5
      pack $cur.lab -side left
      pack $cur.ent -side left -expand yes -fill both
      pack $cur.but -side left
      pack $cur.but2 -side left
    set cur [frame $val.doit]
    Build_DoItFrame $val.doit $rayName
    pack $val.outFile $val.doit -side top -expand yes -fill both

  ##### Pack in the following order because .mid handles small areas the best.
  pack $frame.bot -side bottom -expand no -fill x
  pack $frame.mid -side top -expand yes -fill both
}

#*****************************************************************************
#  GISTab -- Arthur Taylor / MDL
#
# PURPOSE
#    Create the GIS tab
#
# ARGUMENTS
#   frame = Frame to add the GIS widgets to.
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
proc GISTab {frame rayName} {
  upvar #0 $rayName ray

  set cur [frame $frame.top]
    DirBrowser $cur $rayName $ray(dir,GIS_IN) 10
  set cur [frame $frame.bot]

##### Build the lower half #####
    set cur1 [frame $cur.grib]

  ##### Build the GRIB Frame (when dealing with GRIB files) #####
      GRIBFrame $cur.grib $rayName
    set cur1 [frame $cur.txt -relief ridge -bd 5]

  ##### Build the txt Frame (when dealing with text files) #####
      label $cur1.top -text "Blank"
      set cur2 [frame $cur1.txt]
        Scrolled_Text $cur2 $cur2.text -relief sunken -bd 2 \
              -wrap none -width 45 -height 25 -setgrid 1 -tabs {20p} \
              -keepScroll
      pack $cur1.top -side top -expand no -fill x
      pack $cur1.txt -side top -expand yes -fill both
    pack $cur.grib -expand yes -fill both
    set ray(GRIB2,txtBrowse) $cur.txt
    set ray(GRIB2,gribBrowse) $cur.grib
  Pane_Create $frame.top $frame.bot -in $frame \
              -orient vertical -percent .3
}
