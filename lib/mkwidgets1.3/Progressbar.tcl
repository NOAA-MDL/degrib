package require Metawidget

# name: Progressbar
# args: args: option-value pairs
# creates inner widgets, initializes variables.
metawidget create Progressbar {
  # padding of 1 pixel between outer frame and progress bar
  pack [frame $this.pads -border 1 -relief flat] -fill both -expand 1

  # progress bar and text
  label $this.pads.pbar -border 0 -bg darkblue
  label $this.pads.text -border 0 -padx 1 -pady 0 -bg gray90

  my fFraction -1

  -format %d%%
  set_    0
} {} -default pads

# name: -background
# args: sColor: background color for progress bar
# redirects this option to the progress bar instead of the outer frame.
metawidget proc Progressbar -background { {sColor {}} } {
  if { $sColor == {} } {
    $this.pads.pbar cget -background
  } else {
    $this.pads.pbar config -background $sColor
  }
}

# name: -foreground
# args: sColor: foreground color for text label
# redirects this option to the text label instead of the outer frame.
metawidget proc Progressbar -foreground { {sColor {}} } {
  if { $sColor == {} } {
    $this.pads.text cget -foreground
  } else {
    $this.pads.text config -foreground $sColor
  }
}

# name: -font
# args: sFont: font for text label
# redirects this option to the text label instead of the outer frame.
metawidget proc Progressbar -font { {sFont {}} } {
  if { $sFont == {} } {
    $this.pads.text cget -font
  } else {
    $this.pads.text config -font $sFont
  }
}

# name: -format
# args: sFormat: format for text label
# stores this format for use with the text label
metawidget proc Progressbar -format { sFormat } {
  my -format $sFormat

  if { $sFormat == {} } {
    place forget $this.pads.text
  } else {
    place $this.pads.text -relx .5 -rely .5 -anchor c
  }
}

# name: set
# args: fProg: progress (0 .. $fBase)
#       fBase: base value
# sets the progress bar. fProg must be between 0 and fBase.
metawidget proc Progressbar set_ { fProg {fBase 100} } {
  set fFraction [expr 1.*$fProg/$fBase]
  if { $fFraction == [my fFraction] } return
  my fFraction $fFraction

  if { $fFraction == 0 } {
    place forget $this.pads.pbar
  } else {
    place $this.pads.pbar -x 0 -y 0 -relw $fFraction -relh 1
  }

  $this.pads.text config -text [format [my -format] $fProg]
  update
}

metawidget command Progressbar set         set_

metawidget option  Progressbar -background -background -background
metawidget option  Progressbar -foreground -foreground -foreground
metawidget option  Progressbar -font       -font       -font
metawidget option  Progressbar -format     -format

proc test {} {
  . config -bg gray50
  wm minsize . 200 150

  grid [frame .test -relief sunken -border 2] -column 0 -row 0
  pack [label .test.text -text {Progress Bar:}] -side top -anchor w -padx 15 -pady 5
  pack [progressbar .test.prog -width 120 -height 22] -side bottom -padx 15 -pady 5

  grid rowconf . 1 -minsize 10
  grid [button .anim -text {Animate} -pady 0 -command {
    for { set i 0 } { $i <= 100 } { incr i 5 } {
      .test.prog set $i
      after 100
    }
    .test.prog set 0
  }] -column 0 -row 2 -sticky we
}

#test
