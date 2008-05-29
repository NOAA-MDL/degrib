#*****************************************************************************
# util.tcl
#
#   Contains a set of helper procs for grib2flt.tcl.
#
# History:
#   9/2002 Arthur Taylor (RSIS/MDL): Created.
#
# Notes:
#*****************************************************************************

#*****************************************************************************
# AdjustSelectList --
#
#    Adjust all the selected listboxes to stay in sync with each other.
#
# Arguments:
#    cur_lb = Which list box caused the Adjustment.
#    listLB = List of listboxes to update.
# direction = which direction -1 is up, 1 is down, 0 is to propagate a
#             selection in one listbox to the other listboxes..
#
# Globals:
#
# History:
#  9/2002 Arthur Taylor (RSIS/MDL): Created
#
# Notes:
#*****************************************************************************
proc AdjustSelectList {cur_lb ListLB direction} {
   update idletasks

   set temp [expr [$cur_lb curselection] + $direction]
   set len [$cur_lb index end]
   if {$temp < 0} {
      set temp 0
   } elseif {$temp >= $len} {
      set temp [expr $len -1]
   }

   foreach lb $ListLB {
      if {$lb != $cur_lb} {
         $lb selection clear 0 end
         $lb selection set $temp
         $lb activate $temp
      }
   }
   return
}

#*****************************************************************************
# Scroll_Set --
#
#    Used to help hide a scrollbar when not needed.
#
# Arguments:
# scrollBar = The scrollbar to set or hide
#    geoCmd = Command used to pack the window (grid or pack)
#    offset = Where we have scrolled to.
#      size = Size of the scrollbar
#
# Globals:
#
# History:
#  9/2002 Arthur Taylor (RSIS/MDL): Borrowed from SLOSH Display program.
#
# Notes:
#    See Welch, Example 27-2, p347.
#*****************************************************************************
proc Scroll_Set {scrollbar geoCmd offset size} {
   if {($offset != 0.0) || ($size != 1.0)} {
      eval $geoCmd
      $scrollbar set $offset $size
   } else {
      set manager [lindex $geoCmd 0]
      $manager forget $scrollbar
   }
   return
}

#*****************************************************************************
# Scrolled_Canvas --
#
#    Used to create a scrolled canvas with "hideable" scroll bars.
#
# Arguments:
#      c = The frame created (by caller) to contain the scrolled canvas
# canvas = Name of the canvas (provided by caller)
#   args = Any extra arguments to give canvas when creating it.
#
# Globals:
#
# History:
#  9/2002 Arthur Taylor (RSIS/MDL): Borrowed from SLOSH Display program.
#
# Notes:
#    See Welch, Example 31-1, p392 (modified somewhat to take advantage of
# Scroll_Set (See Scrolled_ListBox Example 27-3 p348)
#*****************************************************************************
proc Scrolled_Canvas { c canvas args } {
   eval {canvas $canvas \
         -xscrollcommand [list Scroll_Set $c.xscroll \
               [list grid $c.xscroll -row 1 -column 0 -sticky we]] \
         -yscrollcommand [list Scroll_Set $c.yscroll \
               [list grid $c.yscroll -row 0 -column 1 -sticky ns]] \
         -highlightthickness 0 -borderwidth 0} $args
   scrollbar $c.xscroll -orient horizontal -command [list $canvas xview]
   scrollbar $c.yscroll -orient vertical -command [list $canvas yview]
   grid $canvas $c.yscroll -sticky news
   grid $c.xscroll -sticky ew
   grid rowconfigure $c 0 -weight 1
   grid columnconfigure $c 0 -weight 1
   return $canvas
}

#*****************************************************************************
# multi_scroll --
#
#    A Scrollbar instigated a yscrollcommand.  This is where we deal with
# scrolling the list of listboxes associated with that scrollbar.
#
# Arguments:
# scroll_list = list of listboxes linked to the scrollbar
#        args = List specifying scroll design options.
#
# Globals:
#
# History:
#  9/2002 Arthur Taylor (RSIS/MDL): Borrowed from SLOSH Display program.
#
# Notes:
#     Welch (Practical programming in TCL ed. 2 p. 514)
#*****************************************************************************
proc multi_scroll {scroll_list args} {
	foreach item $scroll_list {
		eval {$item yview} $args
	}
   return
}

#*****************************************************************************
# multi_scroll2 --
#
#    A Listbox (not a Scrollbar) instigated a yscrollcommand.  This is where
# we deal with scrolling the scrollbar and any other listboxes that are
# associated with that scrollbar.
#
# Arguments:
# scroll_path = name of the scrollbar widget
#    box_list = list of listboxes linked to the scrollbar
#        args = List specifying scroll design options
#
# Globals:
#
# History:
#  8/1998 Howard Berger (NWS): Commented
#  9/2002 Arthur Taylor (RSIS/MDL): Borrowed from SLOSH Display program.
#
# Notes:
#     Modified from Welch (Practical programming in TCL p.514)
#*****************************************************************************
proc multi_scroll2 {scroll_path box_list args} {
   eval {$scroll_path set} $args
   foreach i $box_list {
      eval {$i yview moveto} [lindex $args 0]
   }
   return
}

#*****************************************************************************
# CanvasResize --
#
#    Handles a resize event in the canvas.  Makes sure that all the listboxes
# in the canvas get resized.
#
# Arguments:
# c = Canvas to resize.
#
# Globals:
#
# History:
#  9/2002 Arthur Taylor (RSIS/MDL): Created.
#
# Notes:
#*****************************************************************************
proc CanvasResize {c} {
   update idletasks
   set width [winfo width $c]
   set height [winfo height $c]
   set dimen [$c cget -scrollregion]
   set x [lindex $dimen 2]
   set y [lindex $dimen 3]
   if {$y != $height} {
      $c configure -scrollregion [list 0 0 $x $height]
      $c itemconfigure listframe -height $height
   }
   return
}

#*****************************************************************************
# CanvasAddFrame --
#
#    A helper function to add a frame to a scrolled canvas.
#
# Arguments:
# rayName = Name of the Main global Array used by this program.
#       c = canvas to add the frame to.
#      fr = frame to add.
#      x0 = where to add horizontal. (-1 use canvasX0 to determine it)
#      y0 = where to add vertical. (-1 use canvasY0 to determine it)
#   width = desired width of frame.
#  height = desired height of frame.
#
# Globals:
# $rayName (see top of grib2flt.tcl for description of contents.)
#   ray(canvX0) = Current x location to place a listbox
#   ray(canvY0) = Current y location to place a listbox
#
# History:
#  9/2002 Arthur Taylor (RSIS/MDL): Created.
#
# Notes:
#*****************************************************************************
proc CanvasAddFrame {rayName c fr x0 y0 width height} {
   upvar #0 $rayName ray

   if {! [info exist ray(canvX0)]} {
      set ray(canvX0) 0
   }
   if {! [info exist ray(canvY0)]} {
      set ray(canvY0) 0
   }
   if {$x0 == -1} {
      set x0 $ray(canvX0)
   }
   if {$y0 == -1} {
      set y0 $ray(canvY0)
   }
   if {$width == -1} {
      set width [winfo width $fr]
   }
   if {$height == -1} {
      set height [winfo height $fr]
   }
   $c create window $x0 $y0 -height $height -width $width -anchor nw \
         -window $fr -tag listframe
   set ray(canvX0) [expr $x0 + $width]
   set ray(canvY0) [expr $y0 + $height]
   return
}

#*****************************************************************************
# NewListBox --
#
#    A helper function to add a listbox to the scrolled canvas, and maintain
# the appropriate global data.
#
# Arguments:
# rayName = Name of the Main global Array used by this program.
#      fr = Name of frame to create and add to the canv.
#    canv = canvas to add the frame to.
#     txt = Label for the listbox.
#     wid = desired width of frame.
#     hei = desired height of frame.
#
# Globals:
# $rayName (see top of grib2flt.tcl for description of contents.)
#    ray(canvX0) = Current x location to place a listbox (In CanvasAddFrame)
#    ray(canvY0) = Current y location to place a listbox (In CanvasAddFrame)
#   ray(LB_list) = List of all the listboxes in the middle of the window.
#
# History:
#  9/2002 Arthur Taylor (RSIS/MDL): Created.
#
# Notes:
#*****************************************************************************
proc NewListBox {rayName fr canv txt wid hei} {
   upvar #0 $rayName ray

   set cur1 [frame $fr]
      label $cur1.lab -text $txt
      set ans [listbox $cur1.lb -bg #cdb79e -width 10]
      lappend ray(LB_list) $ans
      pack $cur1.lab -side top
      pack $cur1.lb -side top -expand yes -fill both
      CanvasAddFrame $rayName $canv $cur1 -1 0 $wid $hei
   return $ans
}

#----------------- Added for download.tcl ----------------------
namespace eval ns_Util {
}

#-----------------------------------------------------------------------------
# ::ns_Util::DrawXPerc --
#
#   Draws a bar across a given percentage of a given canvas.
#
# Arguments:
#   canv     : The canvas to draw on.
#   x        : The percentage to cover
#
# Globals:
#   utilbar (tag) : Tag for the drawn rectangle, so we can erase it later.
#
# 1/2002 Arthur Taylor (RSIS/MDL) Updated.
#-----------------------------------------------------------------------------
proc ::ns_Util::DrawXPerc {canv x} {
  set x [expr {int ($x)}]
  set bd [$canv cget -bd]
  set width [winfo width $canv]
  $canv delete withtag utilbar
  $canv create rectangle 0 0 [expr {$width / 100. * $x}] [winfo height $canv] \
        -fill skyblue -tag utilbar
  $canv create text [expr {$width / 2}] $bd -text "$x %" -justify center \
        -anchor nw -tag utilbar
  update
  return
}

#-----------------------------------------------------------------------------
# ::ns_Util::ReadIni --
#
#   Reads data from a .ini file.  Returns "" or [list [list variable value]]
#
# Arguments:
#   file     : The name of the .ini file.
#   section  : The [section] of the .ini file of interest.
#   lstVars  : List of variables to read. (* for all variables)
#
# 1/2002 Arthur Taylor (RSIS/MDL) Updated.
#-----------------------------------------------------------------------------
proc ::ns_Util::ReadIni {file section lstVars} {
  if {! [file isfile $file]} {
    puts "Unable to open $file"
    return ""
  }
  if {! [file readable $file]} {
    puts "Unable to read $file"
    return ""
  }
  set fp [open $file r]
#####
# Find the "section"
#####
  set f_found 0
  while {[gets $fp line] >= 0} {
    if {$line == "\[$section\]"} {
      set f_found 1
      break
    }
  }
  if {! $f_found} {
    close $fp
    puts "Unable to find \[$section\] section in $file"
    return ""
  }
#####
# Read through the "section"
#####
  set ans ""
  while {[gets $fp line] >= 0} {
    set line [string trim $line]
    set lineLen [string length $line]
    if {([string index $line 0] == "\[") && \
        ([string index $line [expr {$lineLen -1}]] == "\]")} {
      break
    }
    if {$lineLen != 0} {
      set pair [split $line =]
      if {$lstVars != "*"} {
        if {[lsearch $lstVars [lindex $pair 0]] != -1} {
          lappend ans $pair
        }
      } else {
        lappend ans $pair
      }
    }
  }
  close $fp
  return $ans
}

#-----------------------------------------------------------------------------
# ::ns_Util::WriteIni --
#
#   Reads in an old .ini file, stores it in a Local array, modifies or
# replaces a section, then saves it back to disk.
#
# Arguments:
#   file     : The name of the .ini file.
#   Section  : The [section] of the .ini file of interest.
#   lstVars  : [list [list variable value]] to set.
#
# 1/2002 Arthur Taylor (RSIS/MDL) Updated.
#
# Notes:
#   In a .ini file, we might want order to matter, so we shouldn't use an
# array names, hence the reason for $Section,List
#-----------------------------------------------------------------------------
proc ::ns_Util::WriteIni {file Section lstVars} {
#####
# Read in old .ini file if it exists.
#####
  set SectList ""
  if {[file isfile $file]} {
    if {! [file writable $file]} {
      return ""
    }
    set fp [open $file r]
    set section NULL
    while {[gets $fp line] >= 0} {
      set line [string trim $line]
      set lineLen [string length $line]
      if {([string index $line 0] == "\[") && \
          ([string index $line [expr {$lineLen -1}]] == "\]")} {
        set section [string range $line 1 [expr {$lineLen -2}]]
        lappend SectList $section
        set Local($section,List) ""
      } elseif {$lineLen != 0} {
        set pair [split $line =]
        set Local($section,[lindex $pair 0]) [lindex $pair 1]
        lappend Local($section,List) [lindex $pair 0]
      }
    }
    close $fp
  }
#####
# Add or overwrite new data to array
#   First initializing Section if it is new.
#####
  if {[lsearch $SectList $Section] == -1} {
    lappend SectList $Section
    set Local($Section,List) ""
  }
  foreach pair $lstVars {
    set var [lindex $pair 0]
    set Local($Section,$var) [lindex $pair 1]
    if {[lsearch $Local($Section,List) $var] == -1} {
      lappend Local($Section,List) $var
    }
  }
#####
# Write the data to disk.
#####
  set fp [open $file w]
  foreach section $SectList {
    puts $fp "\[$section\]"
    foreach var $Local($section,List) {
      puts $fp "$var=$Local($section,$var)"
    }
    puts $fp ""
  }
  close $fp
  return
}

#-----------------------------------------------------------------------------
# ::ns_Util::ListBoxSelect --
#
#   Make sure that listbox 2 selects the same elements as listbox 1.
#
# Arguments:
#   lb1      : The correct selection listbox
#   lb2      : The incorrect selection listbox
#
# 1/2002 Arthur Taylor (RSIS/MDL) Updated.
#-----------------------------------------------------------------------------
proc ::ns_Util::ListBoxSelect {lb1 lb2} {
  $lb2 selection clear 0 end
  foreach index [$lb1 curselection] {
    $lb2 selection set $index
  }
  return
}

proc ::ns_Util::MultiListBoxSelect {lb lbList} {
  foreach lb2 $lbList {
    if {$lb2 != $lb} {
      $lb2 selection clear 0 end
      foreach index [$lb curselection] {
        $lb2 selection set $index
      }
    }
  }
  return
}

#*****************************************************************************
#  <AT_pophelp>
#
# Purpose:
#     To Display a help message when the cursor is on an icon.
#
# Variables:(I=input)(O=output)(G=global)
#   tl         (I) A toplevel to put the message in.
#   name       (I) The message to display.
#   win        (I) The window to put it next to (contains the icon).
#   flag       (I) 0 remove the message, 1 display the message.
#   X,Y        (I) offset from original place to put the message.
#   AT_POP_HELP (G) Make sure we don't call proc while in the middle.
#
# Returns: NULL
#
# History:
#    7/1998 Arthur Taylor (RDC/TDL) Cleaned up.
#    6/1999 Arthur Taylor (RSIS/TDL) Moved into separate module.
#
# Notes:
#   1)wm overrideredirect is the key.  (Aside: wm transient is also useful.)
#   2)With wm geometry need []x[]+[]+[] without spaces or breaks in line.
#   3)Tried moving toplevel command out of this proc, and using wm withdraw /
#     wm deiconify but it kept breaking.
# Make sure you deiconify prior to wm geometry.
# Need update idletasks to make it work in win3.1
#*****************************************************************************
proc AT_pophelp {tl name win flag {X 0} {Y 0} {bg SkyBlue}} {
  global AT_POP_HELP

  if {! [info exists AT_POP_HELP]} {
    set AT_POP_HELP 0
  }

# Make sure we don't end up calling this again while we are in the middle of
# handling a previous event.
  if {$AT_POP_HELP != 0} {
    return
  }
  set AT_POP_HELP 1
  if {$flag == 0} {
    catch {destroy $tl}
  } else {
    if {[winfo exists $tl]} {
      catch {raise $tl $ray(main_tl)}
      $tl.label configure -text $name
      wm withdraw $tl
      update idletasks
      catch {wm deiconify $tl}
# The update idletasks could have killed the $tl window.
      if {[winfo exists $tl]} {
        wm geometry $tl "[winfo reqwidth $tl.label]x[winfo reqheight \
              $tl.label]+[expr {[winfo reqwidth $win] + $X + [winfo rootx \
              $win]}]+[expr {[winfo rooty $win] +$Y}]"
      }
    } else {
      catch {destroy $tl}
      toplevel $tl -background $bg
# Needed for Win3.1 (without it we get flicker
      catch {wm withdraw $tl}
      wm overrideredirect $tl 1
      label $tl.label -text $name -foreground black -background $bg -bd 2 -relief ridge
      pack $tl.label
      set width [winfo reqwidth $tl.label]
      set height [winfo reqheight $tl.label]
      update idletasks
      global tcl_platform
      if {$tcl_platform(os) != "HP-UX"} {
        catch {wm deiconify $tl}
      }
# The update idletasks could have killed the $tl window.
      if {[winfo exists $tl]} {
        wm geometry $tl "$width\x$height+[expr {[winfo reqwidth $win] + $X + \
              [winfo rootx $win]}]+[expr {[winfo rooty $win] +$Y}]"
      }
      if {$tcl_platform(os) == "HP-UX"} {
        catch {wm deiconify $tl}
      }
    }
  }
  set AT_POP_HELP 0
}
