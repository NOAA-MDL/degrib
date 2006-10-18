package require Metawidget

# name: Clock
# args: args: option-value pairs
# defines icons for the left/right arrows. creates the internal widgets and
# all bindings for scrolling.
metawidget create Aclock {
  if { ! [ourinfo exists pLeft] } {
    our pPrev [image create bitmap -data {
      #define _width 5
      #define _height 9
      static char _bits[] = { 0x10,0x18,0x1c,0x1e,0x1f,0x1e,0x1c,0x18,0x10 };
    }]
    our pNext [image create bitmap -data {
      #define _width 5
      #define _height 9
      static char _bits[] = { 0x01,0x03,0x07,0x0f,0x1f,0x0f,0x07,0x03,0x01 };
    }]

    our fPi [expr 2*acos(0)]
  }

  pack [frame $this.ctrl -border 1 -relief raised] -fill x
  pack [label $this.ctrl.prev -border 0 -image [our pPrev]] -ipadx 4 -side left -fill y
  pack [entry $this.ctrl.cuti -border 0 -bg white -width 0 -textvar ::${this}::sTime] -ipady 1 -side left -expand 1
  pack [label $this.ctrl.next -border 0 -image [our pNext]] -ipadx 4 -side right -fill y
  pack [canvas $this.time] -fill both -expand 1

  bind $this.ctrl      <Double-1>        "$this set"
  bind $this.ctrl.prev <1>               "$this _scroll -1"
  bind $this.ctrl.next <1>               "$this _scroll  1"
  bind $this.ctrl.prev <Control-1>       "$this _scroll -60"
  bind $this.ctrl.next <Control-1>       "$this _scroll  60"
  bind $this.ctrl.prev <ButtonRelease-1> "$this _scroll 0"
  bind $this.ctrl.next <ButtonRelease-1> "$this _scroll 0"
  bind $this.ctrl.cuti <Return>          "$this set \[$this.ctrl.cuti get\]"
  bind $this.ctrl.cuti <Up>              "$this scroll -1"
  bind $this.ctrl.cuti <Down>            "$this scroll  1"
  bind $this.ctrl.cuti <Control-Left>    "$this scroll -60"
  bind $this.ctrl.cuti <Control-Right>   "$this scroll  60"
  bind $this.time      <Configure>       "$this _drawClock"

  -font [$this.ctrl.cuti cget -font]
  -controls top

  my -format %H:%M

  _drawClock
  set_

  focus $this.ctrl.cuti
} {} -default time

# name: _scroll
# args: iStep: step width
# scrolls and asynchronuously calls the function again by means of a timer,
# as long as the mouse button is pressed. if released, iStep is zero and
# the last timer job is cancelled.
metawidget proc Aclock _scroll { iStep } {
  if { $iStep == 0 } {
    catch { after cancel [my iJob] }
  } else {
    scroll $iStep
    my iJob [after 50 $this _scroll $iStep]
  }
}

# name: _t2c
# args: iDegrees: degrees
#       iRadius: radius in pixel
# transforms polar coordinates into rectangular (x/y) coordinates.
metawidget proc Aclock _t2c { iDegrees iRadius } {
  set iX [expr round(cos(($iDegrees-90)/360.*2*[our fPi])*$iRadius)+[my iX0]]
  set iY [expr round(sin(($iDegrees-90)/360.*2*[our fPi])*$iRadius)+[my iY0]]
  return [list $iX $iY]
}

# name: _drawClock
# args: -
# draws the clock in the canvas. called on init and when resized.
metawidget proc Aclock _drawClock {} {
  # center and radiuses for dial, minute and hour pointer.
  my iX0 [expr [winfo width  $this.time]/2]
  my iY0 [expr [winfo height $this.time]/2]
  set iR0 [expr int(([my iX0]<[my iY0]?[my iX0]:[my iY0])*.9)]

  my iRm [expr int($iR0*.9)]
  my iRh [expr int($iR0*.7)]
  my iRp [expr int($iR0*.1)]

  $this.time delete all

  # draw dial elements
  for { set i 0 } { $i < 360 } { incr i 30 } {
    mkw.lassign [_t2c $i $iR0] iX iY
    $this.time create arc [expr $iX-1] [expr $iY-1] [expr $iX+2] [expr $iY+2] -start 225 -extent 180 -style arc -outline white
    $this.time create arc [expr $iX-1] [expr $iY-1] [expr $iX+2] [expr $iY+2] -start 45 -extent 180 -style arc -outline gray40
  }

  # create polygon. coordinates set by _setClock
  eval $this.time create polygon 0 0 0 0 0 0 -fill white -tags tHour
  eval $this.time create polygon 0 0 0 0 0 0 -fill black -tags tMinute

  # pointers can be dragged nicely
  $this.time bind tMinute <B1-Motion> "$this _turn M %X %Y"
  $this.time bind tHour   <B1-Motion> "$this _turn H %X %Y"

  if { [myinfo exists iMinute] } {
    _setClock
  }
}

# name: _setClock
# args: -
# draws the pointers by calculating and setting the coordinates for the two
# polygons.
metawidget proc Aclock _setClock {} {
  set iM [expr [my iMinute]*6]
  set iH [expr round(([my iHour]+[my iMinute]/60.)*30)]

  eval $this.time coords tHour \
  [_t2c [expr $iH-90] [my iRp]] [_t2c $iH [my iRh]] \
  [_t2c [expr $iH+90] [my iRp]] [_t2c [expr $iH+180] [my iRp]]

  eval $this.time coords tMinute \
  [_t2c [expr $iM-90] [my iRp]] [_t2c $iM [my iRm]] \
  [_t2c [expr $iM+90] [my iRp]] [_t2c [expr $iM+180] [my iRp]]

  set iClock [clock scan [expr ([my iHour]?[my iHour]:12)]:[my iMinute][my sAmpm]]
  my sTime [clock format $iClock -format [my -format]]
}

# name: _turn
# args: sWhat: M for minute, H for hour pointer
#       iPx, iPy: mouse pointer coords when clicked
# drags a pointer, detects roll-overs over the "12" to toggle between AM
# and PM, and redraws the pointers.
metawidget proc Aclock _turn { sWhat iPx iPy } {
  # mouse position relative to clock center
  set iX [expr $iPx-[winfo rootx $this.time]-[my iX0]]
  set iY [expr $iPy-[winfo rooty $this.time]-[my iY0]]

  if { $sWhat == "M" } {
    # transform rectangular coords into polar ones
    if { $iX > 0 } {
      set iM [expr (round(60*atan(1.*$iY/$iX)/2/[our fPi])+15)%60]
    } elseif { $iX < 0 } {
      set iM [expr (round(60*atan(1.*$iY/$iX)/2/[our fPi])+45)%60]
    } elseif { $iY > 0 } {
      set iM 30
    } else {
      set iM 0
    }

    # detect roll-over over "12"
    if { [my iMinute] >= 45 && $iM <= 15 } {
      my iHour [expr [my iHour]+1]
    } elseif { [my iMinute] <= 15 && $iM >= 45 } {
      my iHour [expr [my iHour]-1]
    }

    # toggle AM/PM if hour pointer rolled over "12" also.
    if { [my iHour] < 0 || [my iHour] > 11 } {
      my sAmpm [mkw.decode [my sAmpm] {AM PM PM AM}]
    }

    my iMinute $iM
    my iHour   [expr [my iHour]%12]
  } else {
    # transform rectangular coords into polar ones
    if { $iX > 0 } {
      set iH [expr (round(12*atan(1.*$iY/$iX)/2/[our fPi])+3)%12]
    } elseif { $iX < 0 } {
      set iH [expr (round(12*atan(1.*$iY/$iX)/2/[our fPi])+9)%12]
    } elseif { $iY > 0 } {
      set iH 6
    } else {
      set iH 0
    }

    # detect roll-over over "12" and toggle AM/PM
    if { ( [my iHour] >= 9 && $iH <= 3 ) || ( [my iHour] <=3 && $iH >=9 ) } {
      my sAmpm [mkw.decode [my sAmpm] {AM PM PM AM}]
    }

    my iHour $iH
  }

  _setClock
}

# name: -controls
# args: sPos: controls position: top or bottom
# packs the control frame according to the option
metawidget proc Aclock -controls { sPos } {
  my -controls [mkw.complete $sPos {top bottom}]
  pack $this.ctrl -side [my -controls]
}

# name: -format
# args: sFormat: format accepted by "clock format"
# option member. stores a new format.
metawidget proc Aclock -format { sFormat } {
  my -format $sFormat
  _setClock
}

# name: -font
# args: sFont: font to set
# member option. sets the font in the control frame.
metawidget proc Aclock -font { sFont } {
  $this.ctrl.cuti config -font $sFont
}

# name: get
# args: -
# returns the date formatted according to -format
metawidget proc Aclock get {} {
  return [my sTime]
}

# name: set
# args: sTime: time to set
# sets the clock to a given time.
metawidget proc Aclock set_ { {sTime {}} } {
  if { $sTime == {} } {
    set iClock [clock seconds]
  } else {
    set iClock [clock scan $sTime]
  }

  regsub {^0} [clock format $iClock -format %I] {} iH
  regsub {^0} [clock format $iClock -format %M] {} iM
  my sAmpm    [clock format $iClock -format %p]
  my iHour   $iH
  my iMinute $iM

  _setClock
}

# name: scroll
# args: iStep: minutes
# sets the clock iStep minutes forward or backward.
metawidget proc Aclock scroll { iStep } {
  set iClock [clock scan [my sTime]]
  incr iClock [expr $iStep*60]
  set_ [clock format $iClock -format %H:%M]
}

# name: bind
# args: args: same as for the bind command
# applies bindings to the text widget.
metawidget proc Aclock bind_ { args } {
  eval bind $this.ctrl.cuti $args
}

metawidget command Aclock _turn      _turn
metawidget command Aclock _scroll    _scroll
metawidget command Aclock _drawClock _drawClock

metawidget command Aclock bind       bind_
metawidget command Aclock scroll     scroll
metawidget command Aclock set        set_
metawidget command Aclock get        get

metawidget option  Aclock -format    -format
metawidget option  Aclock -control   -control
metawidget option  Aclock -font      -font

proc test {} {
  console show
  pack [aclock .clock] -fill both -expand 1
}

#test
