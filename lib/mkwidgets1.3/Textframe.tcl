package require Metawidget

# name: Textframe
# args: args: option-value pairs
# creates the widget elements and initializes the metawidget options.
metawidget create Textframe {
  frame $this.deco -border 2 -relief groove
  label $this.text -border 0 -padx 0 -pady 0 -text Headline
  frame $this.work

  -text $this
  -offset 12
  -anchor w
  -font [$this.text cget -font]
} {} -default deco

# name: -anchor
# args: sAnchor: anchor (n, e, s, w, c) for the label
# option member. sets a new anchor for the label widget.
metawidget proc Textframe -anchor { sAnchor } {
  place $this.text -in $this.deco -anchor $sAnchor
  my -anchor $sAnchor
}

# name: -offset
# args: sOffset: offset value for the label.
# option member. sets a new offset value for the label widget. three different
# formats: E.g. 20% sets the label 20% of the frame's width off from the left.
# 20 sets the label 20 pixel from the left border (good with -anchor w).
# end-20 sets it from the frame's right border (good with -anchor e).
metawidget proc Textframe -offset { sOffset } {
  if { [regexp {^([0-9]+)%$} $sOffset sDummy fPct] } {
    set iX 0;
    set fRelx [expr $iPct/100.]
  } elseif { [regexp {^[0-9]+$} $sOffset iX] } {
    set fRelx 0
  } elseif { [regexp {^end(-[0-9]+)$} $sOffset sDummy iX] } {
    set fRelx 1
  } else {
    error "Bad offset $sOffset. Must be an integer, end-integer or integer%"
  }

  place $this.text -in $this.deco -x $iX -relx $fRelx
  my -offset $sOffset
}

# name: -font
# args: sFont: font to set
# a change in the font size requires a new padding for the decorative frame
# and the work area, so that the text is fully visible and not overlapped.
metawidget proc Textframe -font { sFont } {
  $this.text configure -font $sFont

  set iPad [expr [font metrics $sFont -linespace]/2+1]
  pack $this.deco -fill both -expand 1 -padx 2 -pady $iPad
  pack $this.work -fill both -expand 1 -padx 2 -pady $iPad -in $this.deco

  my -font $sFont
}

# name: -text
# args: sText: text to set
# option member. sets a new text to the textframe
metawidget proc Textframe -text { sText } {
  $this.text config -text $sText
  my -text $sText
}

# name: pack
# args: sWindow: widget to pack
#       args: option-value pairs
# packs widgets inside the work area.
metawidget proc Textframe pack_ { sWindow args } {
  eval pack $sWindow -in $this.work $args
}

# name: grid
# args: sWindow: widget to grid
#       args: option-value pairs
# grids widgets inside the work area.
metawidget proc Textframe grid_ { sWindow args } {
  if { [string index $sWindow 0] == "." } {
    eval grid $sWindow -in $this.work $args
  } else {
    eval grid $sWindow $this.work $args
  }
}

# name: place
# args: sWindow: widget to place
#       args: option-value pairs
# places widgets inside the work area.
metawidget proc Textframe place_ { sWindow args } {
  eval place $sWindow -in $this.work $args
}

metawidget option  Textframe -anchor -anchor
metawidget option  Textframe -offset -offset
metawidget option  Textframe -font   -font
metawidget option  Textframe -text   -text

metawidget command Textframe pack    pack_
metawidget command Textframe grid    grid_
metawidget command Textframe place   place_

proc test {} {
  pack [textframe .tfrm -text " Change Font "] -fill both -expand 1

  checkbutton .c1 -text "Alternative Font"  -command {.tfrm config -font $c1} -on {Courier 15 bold} -off {Helv 9}
  checkbutton .c2 -text "Alternative Frame" -command {.tfrm config -relief $c2} -on solid -off groove

  .tfrm pack .c1 -side top -anchor w
  .tfrm pack .c2 -side top -anchor w
}

#test
