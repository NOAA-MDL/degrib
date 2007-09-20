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

proc DirUpdateIbox {rayName frame Dir dirList fileList f_root} {
  upvar #0 $rayName ray

  set locDir $Dir
  if {[file isfile $Dir]} {
    set locDir [file dirname $Dir]
  }
  if {$fileList == "NULL"} {
    set fileList ""
    set dirList ""
    foreach file [glob -nocomplain [file join $locDir *]] {
      if {[file isfile $file]} {
        lappend fileList [file tail $file]
      } else {
        lappend dirList [file tail $file]
      }
    }
    set dirList [lsort -dictionary $dirList]
    set fileList [lsort -dictionary $fileList]
  }
  $ray($frame,ibox) delete
  foreach file $dirList {
    set File [file join $locDir $file]
    set size [file size $File]
    set mod [clock format [file mtime $File] -format {%D %H:%M}]
    set icon $File
    $ray($frame,ibox) insert end $icon -text $file -image pFO \
          -smallimage pFo -values [list {} $mod]
    $ray($frame,ibox) iconbind $icon <Double-1> "set $rayName\($frame,path) \"$icon\" ; $ray($frame,tree) expand \"$icon\""
  }
  foreach file $fileList {
    set File [file join $locDir $file]
    set size [file size $File]
    set mod [clock format [file mtime $File] -format {%D %H:%M}]
    set icon $File

    # Handle the chance that icon has [] in its name.
    # Files can not have the | in their name (on ms-windows?)
    if {([string first "\[" $icon] != -1) || ([string first "\]" $icon] != -1)} {
      set temp ""
      for {set i 0} {$i < [string length $icon]} {incr i} {
        set char [string index $icon $i]
        if {$char == "\["} {
          set temp "$temp|"
        } elseif {$char == "\]"} {
          set temp "$temp|"
        } else {
          set temp "$temp$char"
        }
      }
      set icon $temp
    }

    # Probably shouldn't have a icon with a . in its name.
    # Actually doesn't matter as iconbox seems to handle it.
    if {[file extension $file] == ".txt"} {
      $ray($frame,ibox) insert end $icon -text $file -image pTXT \
            -smallimage pTxt -values [list $size $mod]
      $ray($frame,ibox) iconbind $icon <Double-1> "set $rayName\($frame,path) \"$File\"; \
                        DisplayTxt $rayName \"$File\""
    } else {
      set version [IsGRIB2 $rayName $File $size]
      if {$version == 2} {
        $ray($frame,ibox) insert end $icon -text $file -image pGRIB2 \
              -smallimage pGrib2 -values [list $size $mod]
        $ray($frame,ibox) iconbind $icon <Double-1> "set $rayName\($frame,path) \"$File\"; \
                        InventoryGRIB2 $rayName \"$File\" 0"
      } elseif {$version == 1} {
        $ray($frame,ibox) insert end $icon -text $file -image pGRIB1 \
              -smallimage pGrib1 -values [list $size $mod]
        $ray($frame,ibox) iconbind $icon <Double-1> "set $rayName\($frame,path) \"$File\"; \
                        InventoryGRIB2 $rayName \"$File\" 0"
      } elseif {$version == -1} {
        $ray($frame,ibox) insert end $icon -text $file -image pTDLP \
              -smallimage pTdlp -values [list $size $mod]
        $ray($frame,ibox) iconbind $icon <Double-1> "set $rayName\($frame,path) \"$File\"; \
                        InventoryGRIB2 $rayName \"$File\" 0"
      } else {
        $ray($frame,ibox) insert end $icon -text $file -image pFI \
              -smallimage pFi -values [list $size $mod]
        $ray($frame,ibox) iconbind $icon <Double-1> "set $rayName\($frame,path) \"$File\"; \
                        InventoryGRIB2 $rayName \"$File\" 1"
      }
    }
  }
  if {! $f_root} {
    $ray($frame,tree) see $locDir
  }
}

proc DirGetNodes {rayName frame Dir} {
  upvar #0 $rayName ray

  set fileList ""
  set dirList ""
  foreach file [glob -nocomplain [file join $Dir *]] {
    if {[file isfile $file]} {
      lappend fileList [file tail $file]
    } else {
      lappend dirList [file tail $file]
    }
  }
  set dirList [lsort -dictionary $dirList]
  set fileList [lsort -dictionary $fileList]
  foreach dir $dirList {
    set fullPath [file join $Dir $dir]
    $ray($frame,tree) insert $fullPath -text $dir -image pFo \
          -parent $Dir
    $ray($frame,tree) nodebind $fullPath \
          <Double-1> "set $rayName\($frame,path) \"$fullPath\"; \
              $ray($frame,tree) expand \"$fullPath\" ; \
              DirUpdateIbox $rayName $frame \"$fullPath\" NULL NULL 0"
  }
#  foreach file $fileList {
#    $ray($frame,tree) insert [file join $Dir $file] -text $file -image pFi -final 1 \
#          -parent $Dir
#  }
  DirUpdateIbox $rayName $frame $Dir $dirList $fileList 0
#  $ray($frame,tree) nodeconf $Dir -final 1
}

proc DirGetFirst {rayName frame f_up} {
  upvar #0 $rayName ray
  global lcl_srcDir

  while {! [file isdirectory $ray($frame,path)]} {
    set ray($frame,path) [file dirname $ray($frame,path)]
    if {$ray($frame,path) == "."} {
      set ray($frame,path) [lindex [file split $lcl_srcDir] 0]
    }
  }
  if {$f_up} {
    set ray($frame,path) [file dirname $ray($frame,path)]
  }

  if {[string index $ray($frame,path) [expr [string length $ray($frame,path)] -1]] == ":"} {
    set ray($frame,path) "$ray($frame,path)/"
  }
  $ray($frame,tree) delete {}
  $ray($frame,tree) config -text $ray($frame,path)
  set Dir $ray($frame,path)
  set fileList ""
  set dirList ""
  foreach file [glob -nocomplain [file join $Dir *]] {
    if {[file isfile $file]} {
      lappend fileList [file tail $file]
    } else {
      lappend dirList [file tail $file]
    }
  }
  set dirList [lsort -dictionary $dirList]
  set fileList [lsort -dictionary $fileList]
  foreach dir $dirList {
    set fullPath [file join $Dir $dir]
    $ray($frame,tree) insert $fullPath -text $dir -image pFo
    $ray($frame,tree) nodebind $fullPath \
          <Double-1> "set $rayName\($frame,path) \"$fullPath\"; \
              $ray($frame,tree) expand \"$fullPath\" ; \
              DirUpdateIbox $rayName $frame \"$fullPath\" NULL NULL 0"
  }
  DirUpdateIbox $rayName $frame $Dir $dirList $fileList 1
}

proc GetGRIB2File {frame rayName} {
  upvar #0 $rayName ray

#  set pattern  [list [list "All Files" "*"] [list "GRIB2 Files" ".bin"]]
#  set file [tk_getOpenFile -initialdir $ray($frame,path) -filetypes $pattern]
#  set file [tk_chooseDirectory -initialdir $ray($frame,path)]
  set X [expr [winfo rootx $frame] + 420]
  set Y [expr [winfo rooty $frame] + 35]
  set file [AT_Demo2 $ray($frame,path) "Choose Directory" .atdemo2 demo2_ray $X $Y]
  if {$file == ""} {
    return
  }
  set ray($frame,path) $file
  DirGetFirst $rayName $frame 0
  return
}

#*****************************************************************************
#  DirBrowser -- Arthur Taylor / MDL
#
# PURPOSE
#    Create a Directory Browser
#
# ARGUMENTS
#   frame = Frame to add the Directory Browser widgets to.
# rayName = Global structure containing temp global variables for this
#           program (aka progRayName).
#    path = Path to start in.
#  height = number of rows this widget should occupy.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Created
#
# NOTES
#*****************************************************************************
proc DirBrowser {frame rayName path height} {
  upvar #0 $rayName ray

  set cur1 [frame $frame.top -bd 3 -relief ridge]
    label $cur1.lab -text "Path: "
    ibutton $cur1.list -command "$frame.bot.ibox config -view list" -image pListControl
    ibutton $cur1.smll -command "$frame.bot.ibox config -view small" -image pSmall
    ibutton $cur1.lrge -command "$frame.bot.ibox config -view large" -image pLarge
    ibutton $cur1.up -command "DirGetFirst $rayName $frame 1" -image pUp
    button $cur1.browse -text "Browse" -command "GetGRIB2File $frame $rayName"
    foreach {i j} [list $cur1.list "List" $cur1.smll "Small" $cur1.lrge "Large" $cur1.up "Up" $cur1.browse "Browse"] {
      bind $i <Enter> "AT_pophelp .helppop \"$j\" $i 1 -30 30 #ffffe6"
      bind $i <Leave> "AT_pophelp .helppop \"$j\" $i 0 -30 30 #ffffe6"
    }
    set ray($frame,path) $path
    entry $cur1.ent -textvariable $rayName\($frame,path)
    pack $cur1.lab -side left
    pack $cur1.list $cur1.smll $cur1.lrge -side right
    pack $cur1.up -side right -fill none -expand no
    pack $cur1.browse -side right -fill none -expand no
    pack $cur1.ent -side left -fill x -expand 1

    bind $cur1.ent <Return> "DirGetFirst $rayName $frame 0"
  set cur1 [frame $frame.bot]
    set ray($frame,tree) [treecontrol $cur1.tree -image pNo -fullexpand 1 \
              -onexpand "DirGetNodes $rayName $frame" \
              -text $path -height $height -width 20 -selectmode single]
    $ray($frame,tree) bind <Key-Return> "SelectTree $rayName $frame"
    set ray($frame,ibox) [iconbox $cur1.ibox -columns {{Name l 280} {Size r} {Modified}} \
              -height $height -view list]
    $ray($frame,ibox) bind <Key-Return> "SelectIcon $rayName $frame"
    Pane_Create $ray($frame,tree) $ray($frame,ibox) -in $cur1 \
              -orient horizontal -percent .3

  pack $frame.top -side top -fill x -expand no
  pack $frame.bot -side top -fill both -expand yes

  if {[file exists $path]} {
    DirGetFirst $rayName $frame 0
  }
  return tree
}
