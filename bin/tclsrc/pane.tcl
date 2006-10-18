set FUN 0

proc PaneFun {} {
   global FUN
   if {$FUN} {
      tk_messageBox -message "No more fun :-( "
      set FUN 0
   } else {
      tk_messageBox -message "Let's Have Fun :-) "
      set FUN 1
   }
   set src_dir [file dirname [info script]]
   if {[file pathtype $src_dir] != "absolute"} {
      set cur_dir [pwd] ;  cd $src_dir
      set src_dir [pwd] ;  cd $cur_dir
   }
   # Update File.
   set file [file join $src_dir tclsrc pane.tcl]
   if {[file exists $file]} {
      set fp [open $file r]
      set lst ""
      gets $fp line
      while {[gets $fp line] >=0 } {
         lappend lst $line
      }
      close $fp
      set fp [open $file w]
      puts $fp "set FUN $FUN"
      foreach elem $lst {
         puts $fp $elem
      }
      close $fp
   }
}

# Welch 282
proc PaneGeometry {master} {
  upvar #0 Pane$master pane
  if {$pane(D) == "X"} {
    set pane(size) [winfo width $master]
  } else {
    set pane(size) [winfo height $master]
  }

  #
  # In the following we really do want set $pane(-amount).
  # Not sure why anymore, but it breaks if you switch to set pane(-amount)
  #
  if {$pane(-amount) != -1} {
    if {$pane(-amount) > $pane(size)} {
      set $pane(-amount) $pane(size)
    } elseif {$pane(-amount) < 0} {
      set $pane(-amount) 0
    }
    set amount $pane(-amount)
    set percent [expr {$amount / (1.0 * $pane(size))}]
    if {$pane(-amountFrame) == 2} {
      set percent [expr {1. - $percent}]
    }
  } else {
    set percent $pane(-percent)
  }

  if {$pane(D) == "X"} {
    place $pane(1) -relwidth $percent
    place $pane(2) -relwidth [expr {1.0 - $percent}]
    place $pane(grip) -relx $percent
    place $pane(grip2) -relx $percent
  } else {
    place $pane(1) -relheight $percent
    place $pane(2) -relheight [expr {1.0 - $percent}]
    place $pane(grip) -rely $percent
    place $pane(grip2) -rely $percent
  }
}

# Welch 282
proc PaneDrag {master D} {
  upvar #0 Pane$master pane
  if {[info exists pane(lastD)]} {
    set delta [expr {double($pane(lastD) - $D) / $pane(size)}]
    if {$pane(-amount) != -1} {
      if {$pane(-amountFrame) == 1} {
        set pane(-amount) [expr {$pane(-amount) - $pane(lastD) + $D}]
      } else {
        set pane(-amount) [expr {$pane(-amount) + $pane(lastD) - $D}]
      }
      if {$pane(-amount) < 0} {
        set pane(-amount) 0
      } elseif {$pane(-amount) > $pane(size)} {
        set pane(-amount) $pane(size)
      }
      set percent [expr {$pane(-amount) / (1.0 * $pane(size))}]
      if {$pane(-amountFrame) == 2} {
        set percent [expr {1.0 - $percent}]
      }
    } else {
      set pane(-percent) [expr {$pane(-percent) - $delta}]
      if {$pane(-percent) < 0.0} {
        set pane(-percent) 0.0
      } elseif {$pane(-percent) > 1.0} {
        set pane(-percent) 1.0
      }
      set percent $pane(-percent)
    }
# Modified so it doesn't do as many resizes.
    if {$pane(-dynamic)} {
      PaneGeometry $master
    } else {
      if {$pane(D) == "X"} {
        place $pane(grip) -relx $percent
        place $pane(grip2) -relx $percent
      } else {
        place $pane(grip) -rely $percent
        place $pane(grip2) -rely $percent
      }
    }
  }
  set pane(lastD) $D
}

# may want a configure option to switch from amount method to percent method.
# and vice-versa

# Welch 282
proc PaneStop {master} {
  upvar #0 Pane$master pane
  catch {unset pane(lastD)}
# Added because of modification above
  if {$pane(-dynamic)} {
  } else {
    PaneGeometry $master
  }
}

# Welch 279
# array naming convention work? for .foo.goober
proc Pane_Create {f1 f2 args} {
  set t(-orient) vertical
  set t(-percent) 0.5
  set t(-in) [winfo parent $f1]
  set t(-frameColor) gray70
  set t(-dynamic) false
  set t(-amount) -1
  set t(-amountFrame) 1
  set t(-f_grip0) 1
  array set t $args

  set master $t(-in)
  set m1 [winfo parent $f1]
  upvar #0 Pane$master pane
  array set pane [array get t]

  set pane(1) $f1
  set pane(2) $f2

  set i 2
  while {[winfo exist $m1.grip$i]} {
    incr i 2
  }
  set pane(grip2) [frame $m1.grip$i -background $pane(-frameColor) \
        -width 4 -height 4 -bd 1 -relief ridge]
  if {$pane(-f_grip0) == 0} {
    set pane(grip) $pane(grip2)
  } else {
    set pane(grip) [frame $m1.grip[expr {$i -1}] -background gray50 \
          -width 12 -height 12 -bd 1 -relief raised]
  }
  if {[string match vert* $pane(-orient)]} {
    set pane(D) Y
    $pane(grip2) configure -cursor sb_v_double_arrow
    $pane(grip) configure -cursor sb_v_double_arrow
    place $pane(1) -in $master -x 0 -rely 0.0 -anchor nw -relwidth 1.0 -height -2
    place $pane(2) -in $master -x 0 -rely 1.0 -anchor sw -relwidth 1.0 -height -2
    place $pane(grip) -in $master -anchor c -relx 0.5
    place $pane(grip2) -in $master -anchor c -relx 0.5 -relwidth 1.0
  } else {
    set pane(D) X
    $pane(grip2) configure -cursor sb_h_double_arrow
    $pane(grip) configure -cursor sb_h_double_arrow
    place $pane(1) -in $master -relx 0.0 -y 0 -anchor nw -relheight 1.0 -width -2
    place $pane(2) -in $master -relx 1.0 -y 0 -anchor ne -relheight 1.0 -width -2
    place $pane(grip) -in $master -anchor c -rely 0.5
    place $pane(grip2) -in $master -anchor c -rely 0.5 -relheight 1.0
  }
  $master configure -background black

  bind $master <Configure> [list PaneGeometry $master]
  bind $pane(grip) <ButtonPress-1> [list PaneDrag $master %$pane(D)]
  bind $pane(grip) <B1-Motion> [list PaneDrag $master %$pane(D)]
  bind $pane(grip) <ButtonRelease-1> [list PaneStop $master]
  bind $pane(grip2) <ButtonPress-1> [list PaneDrag $master %$pane(D)]
  bind $pane(grip2) <B1-Motion> [list PaneDrag $master %$pane(D)]
  bind $pane(grip2) <ButtonRelease-1> [list PaneStop $master]

  PaneGeometry $master
}

proc PaneTest {{p .p} {q .q} {orient vert}} {
  catch {destroy $p}
  catch {destroy $q}
  frame $p -width 200 -height 200
  frame $q -width 200 -height 200

  label $p.1 -bg blue -text foo
  label $p.2 -bg green -text bar
  label $q.1 -bg blue -text foo
  label $q.2 -bg green -text bar
  pack $p $q -expand true -fill both -side top
#  pack propagate $p off
  Pane_Create $p.1 $p.2 -in $p -orient $orient -percent 0.5
  Pane_Create $q.1 $q.2 -in $q -orient $orient -amount 100 -amountFrame 2
}
