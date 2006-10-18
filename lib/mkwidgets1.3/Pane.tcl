package require Metawidget

# name: Pane
# args: args: Option-value pairs
# creates two frames to hold the user widgets on each side of the Pane.
# creates the Pane as an invisible frame and assigns bindings to it for
# movements. A binding for window resize is created to recalculate the
# Pane position according to the window's new geometry. the metawidget
# options are initialized by calling their metawidget functions.
metawidget create Pane {
  frame $this.frm1
  frame $this.frm2
  frame $this.pane

  bind $this.pane <1>               "$this _click"
  bind $this.pane <B1-Motion>       "$this _move"
  bind $this.pane <ButtonRelease-1> "$this _release"
  bind $this <Configure>            "$this _resize"

  -orientation vertical
  -update      on
  -width       4
  -resize      both
} {} -default Pane

# name: _click
# args: -
# called when the mouse button is pressed over the Pane. Stores the Pane's x
# position and the width of the enclosing widget in two private variables.
# if the -orientation is horizontal, then x is in fact y, the width is in
# fact the height. also, if real-time update is off, creates a temporary
# frameless toplevel as a visual indicator for the pane position.
# during dragging this toplevel is laid over the pane frame.
metawidget proc Pane _click {} {
  set x [my sX]
  set w [my sW]

  my iPos0 [expr [mkw.decode -$x [place info $this.pane]]-[winfo pointer$x .]]
  my iArea [winfo $w $this]

  if { ! [my -update] } {
    toplevel $this.drag
    wm overrideredirect $this.drag 1
    wm geometry $this.drag =[my -width]x[winfo height $this.pane]+[winfo rootx $this.pane]+[winfo rooty $this.pane]
  }
}

# name: _move
# args: -
# called when the Pane is moved. the new Pane position is calculated.
# if the -update option is on, the _release metawidget is called which
# immediately redraws the Paned windows.
metawidget proc Pane _move {} {
  set x [my sX]

  my iPos [expr [my iPos0]+[winfo pointer$x .]]
  if { [my iPos] < 0 } {
    my iPos 0
  } elseif { [my iPos] > [my iArea]-[my -width] } {
    my iPos [expr [my iArea]-[my -width]]
  }

  place $this.pane -$x [my iPos]

  if { [my -update] } {
    _release
  } else {
    wm geometry $this.drag =[my -width]x[winfo height $this.pane]+[winfo rootx $this.pane]+[winfo rooty $this.pane]
  }
}

# name: _release
# args: -
# called after the Pane has been dragged, or while dragging if the -update
# option is on. calculates the new size and position of the Paned windows
# and places them accordingly. the variable fRatio contains the width (height)
# ratios of the two Paned windows.
metawidget proc Pane _release {} {
  set x [my sX]
  set w [my sW]

  place $this.frm1 -$x 0 -$w [my iPos]
  place $this.frm2 -$x [expr [my iPos]+[my -width]] -$w [expr [my iArea]-[my iPos]-[my -width]]
  my fRatio [expr 1.*[my iPos]/[my iArea]]

  if { ! [my -update] } {
    destroy $this.drag
  }
}

# name: _resize
# args: -
# called when the toplevel window is resized. depending on the -resize option
# the Pane position is recalculated and the windows are updated accordingly.
metawidget proc Pane _resize {} {
  set x [my sX]
  set w [my sW]
  set h [my sH]

  my iArea [winfo $w $this]

  if { ! [myinfo exists iPos] } {
    my iPos [expr [my iArea]/2]
    my fRatio .5
    place $this.pane -$x [my iPos] -$w [my -width] -rel$h 1
    place $this.frm1 -$x 0 -$w [my iPos] -rel$h 1
    place $this.frm2 -$x [expr [my iPos]+[my -width]] -$w [expr [my iArea]-[my iPos]-[my -width]] -rel$h 1
    update
  }

  switch [my iM] {
    0 {
      my iPos [expr [my fRatio]*[my iArea]]
      place $this.pane -$x [my iPos]
      place $this.frm1 -$w [my iPos]
      place $this.frm2 -$x [expr [my iPos]+[my -width]] -$w [expr [my iArea]-[my iPos]-[my -width]]
    }
    1 {
      my iPos [expr [my iArea]-[my -width]-[winfo $w $this.frm2]]
      place $this.pane -$x [my iPos]
      place $this.frm1 -$w [my iPos]
      place $this.frm2 -$x [expr [my iPos]+[my -width]]
    }
    2 {
      place $this.frm2 -$w [expr [my iArea]-[my iPos]-[my -width]]
    }
  }
}

# name: -orientation
# args: sOrient: vertical or horizontal
# option metawidget. depending on the orientation some variables are defined.
# they are later used as e.g. x, y, width and height options. if orientation
# is horizontal then x becomes y, width becomes height and vice versa.
metawidget proc Pane -orientation { sOrient } {
  my -orientation [mkw.complete $sOrient {vertical horizontal}]
  my iO [mkw.decode [my -orientation] {vertical 1 horizontal 0}]

  my sX [mkw.decode [my iO] {1 x 0 y}]
  my sY [mkw.decode [my iO] {1 y 0 x}]
  my sW [mkw.decode [my iO] {1 width 0 height}]
  my sH [mkw.decode [my iO] {1 height 0 width}]

  $this.pane config -cursor [mkw.decode [my iO] {1 sb_h_double_arrow 0 sb_v_double_arrow}]
}

# name: -width
# args: iWidth: Pane width
# the desired Pane width is stored in a private variable.
metawidget proc Pane -width { iWidth } {
  if { $iWidth < 1 } {
    error "Pane $this: Pane must be at least 1 pixel wide."
  }

  my -width $iWidth
}

# name: -resize
# args: sMode: resize mode
# the new resize mode is stored in a private variable. 'left' means
# that only the left Paned window is resized, the right keeps its size.
# 'right' means the same for the right window. 'both' will resize both
# Paned windows proportionally.
metawidget proc Pane -resize { sMode } {
  my -resize [mkw.complete $sMode {both first second}]
  my iM [mkw.decode [my -resize] {both 0 first 1 second 2}]
}

# name: -update
# args: sUpdate: flag
# stores the update mode (a boolean) in a private variable.
metawidget proc Pane -update { sUpdate } {
  set sUpdate [mkw.complete $sUpdate {on yes true 1 off no false 0}]
  my -update [mkw.decode $sUpdate {on 1 yes 1 true 1 off 0 no 0 false 0} $sUpdate]
}

# name: set
# args: iPos: new Pane position
#       sMode: same as for -resize
# for 'both' iPos is considered to be a percentage value (0-100). 0 is left
# (top), 100 is right (bottom). for 'first' and 'second' iPos is taken as a
# pixel value. the Pane is positioned that amount of pixels from left (top)
# resp. right (bottom).
metawidget proc Pane set_ { iPos {sMode both} } {
  switch [mkw.complete $sMode {both first second}] {
    both   { set iPos [expr $iPos/100.*[my iArea]] }
    first  { set iPos $iPos }
    second { set iPos [expr [my iArea]-[my -width]-$iPos] }
  }

  if { $iPos < 0 } {
    set iPos 0
  } elseif { $iPos > [my iArea]-[my -width] } {
    set iPos [expr [my iArea]-[my -width]]
  }

  my iPos $iPos
  place $this.pane -[my sX] [my iPos]
  $this _release
}

# name: get
# args: sMode: same as for -resize
# the 'inverse' for the 'set' metawidget. retrieves the Pane position in a format
# depending on sMode: for 'both' it is a percentage value, otherwise the
# pixel position from the left resp. right (top resp. bottom).
metawidget proc Pane get { {sMode both} } {
  set w [my sW]

  switch [mkw.complete $sMode {both first second}] {
    both   { return [expr 100*[winfo $w $this.frm1]/[my iArea]] }
    first  { return [winfo $w $this.frm1] }
    second { return [winfo $w $this.frm2] }
  }
}

# name: place
# args: sMode: first or second. Paned window to place widgets into.
#       sWindow: widget to place
#       args: option-value pairs for the place command
# places sWindow in one of the two Paned windows.
metawidget proc Pane place_ { sMode sWindow args } {
  set sPane $this.[mkw.decode $sMode {first frm1 second frm2}]
  eval place $sWindow -in $sPane $args
  raise $sWindow
}

# name: pack
# args: sMode: first or second. Paned window to pack widgets into.
#       sWindow: widget to pack
#       args: option-value pairs for the pack command
# packs sWindow in one of the two Paned windows.
metawidget proc Pane pack_ { sMode sWindow args } {
  set sPane $this.[mkw.decode $sMode {first frm1 second frm2}]
  eval pack $sWindow -in $sPane $args
  raise $sWindow
}

# name: grid
# args: sMode: first or second. Paned window to grid widgets into.
#       sWindow: widget to place
#       args: option-value pairs for the grid command
# grids sWindow in one of the two Paned windows. sWindow can in fact be
# something like 'configure' which is detected with the if-clause.
metawidget proc Pane grid_ { sMode sWindow args } {
  set sPane $this.[mkw.decode $sMode {first frm1 second frm2}]

  if { [winfo exists $sWindow] } {
    eval grid $sWindow -in $sPane $args
  } else {
    eval grid $sWindow $sPane $args
  }

  raise $sWindow
}

metawidget command Pane _click   _click
metawidget command Pane _move    _move
metawidget command Pane _release _release
metawidget command Pane _resize  _resize

metawidget command Pane set      set_
metawidget command Pane get      get
metawidget command Pane place    place_
metawidget command Pane pack     pack_
metawidget command Pane grid     grid_

metawidget option  Pane -orientation -orientation
metawidget option  Pane -resize      -resize
metawidget option  Pane -width       -width
metawidget option  Pane -update      -update

proc test {} {
  wm geometry . 300x300

  pack [checkbutton .chk1 -text "Don't update while dragging" -on 0 -off 1 -command {.pan1 config -update $chk1}] -anchor w
  pack [checkbutton .chk2 -text "Don't resize left editor"    -on s -off b -command {.pan1 config -resize $chk2}] -anchor w

  text .txt1 -bg white
  text .txt2 -bg white
  text .txt3 -bg white
  .txt1 insert end "left editor: the quick brown fox jumps over the lazy dog"
  .txt2 insert end "upper right editor: bla bla bla bla bla"
  .txt3 insert end [info body test]

  pane .pan1 -width 3
  pane .pan2 -width 3 -orientation horiz

  pack .pan1 -fill both -expand 1
  .pan1 pack first  .txt1 -fill both -expand 1
  .pan1 pack second .pan2 -fill both -expand 1
  .pan2 pack first  .txt2 -fill both -expand 1
  .pan2 pack second .txt3 -fill both -expand 1

}

#test
