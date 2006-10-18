package require Metawidget

# name: Calendar
# args: args: option-value pairs
# defines icons for the left/right arrows. creates the internal widgets and
# all bindings for scrolling through the Calendar. The Calendar is a text
# widget without the typical class bindings for Text.
metawidget create Calendar {
  if { ! [ourinfo exists pPrev] } {
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
  }

  # control panel and the Calendar (a text widget)
  pack [frame $this.ctrl -border 1 -relief raised] -fill x
  pack [label $this.ctrl.prev -border 0 -image [our pPrev]] -ipadx 4 -side left -fill y
  pack [label $this.ctrl.cumo -border 0 ] -ipady 1 -side left -fill x -expand 1
  pack [label $this.ctrl.next -border 0 -image [our pNext]] -ipadx 4 -side right -fill y
  pack [text  $this.text -bg white -spacing1 1 -spacing3 1 -border 0 -width 22 -height 7 -wrap none -cursor arrow]

  # bindings to scroll thru Calendar
  bind $this.ctrl.prev <1>               "$this _scroll -1"
  bind $this.ctrl.next <1>               "$this _scroll  1"
  bind $this.ctrl.prev <Control-1>       "$this _scroll -1 year"
  bind $this.ctrl.next <Control-1>       "$this _scroll  1 year"
  bind $this.ctrl.prev <ButtonRelease-1> "$this _scroll  0"
  bind $this.ctrl.next <ButtonRelease-1> "$this _scroll  0"
  bind $this.ctrl.cumo <Double-1>        "$this set"
  bind $this.text <1>                    "focus $this.text"
  bind $this.text <Left>                 "$this scroll -1 day"
  bind $this.text <Right>                "$this scroll  1 day"
  bind $this.text <Up>                   "$this scroll -7 days"
  bind $this.text <Down>                 "$this scroll  7 days"

  # take Text class bindings away
  bindtags $this.text [mkw.lshrink [bindtags $this.text] Text]
  focus $this.text

  # set up 1st row
  $this.text insert end "\tS\tM\tT\tW\tT\tF\tS\t\n"
  $this.text tag config tDays -background grey
  $this.text tag add tDays 1.0 1.end

  # init widget options by calling their member functions
  -font [$this.text cget -font]
  -controls top

  # set current date
  set_
} {} -default text

# name: _focus
# args: iDay: day to set selection for
# each day in the Calendar has a unique tag. the special tag "sel" (the
# selection of the text widget) is simply mapped over it.
metawidget proc Calendar _focus { iDay } {
  if { [myinfo exists iDay] } {
    $this.text tag remove sel 1.0 end
  }

  eval $this.text tag add sel [$this.text tag ranges t$iDay]
  my iDay $iDay
  my iClock [clock scan [my iMonth]/[my iDay]/[my iYear]]
}

# name: _scroll
# args: iStep: step width
#       sUnit: step unit
# scrolls and asynchronuously calls the function again by means of a timer,
# as long as the mouse button is pressed. if released, iStep is zero and
# the last timer job is cancelled.
metawidget proc Calendar _scroll { iStep {sUnit month} } {
  if { $iStep == 0 } {
    catch { after cancel [my iJob] }
  } else {
    scroll $iStep $sUnit
    my iJob [after 150 $this _scroll $iStep $sUnit]
  }
}

# name: -controls
# args: sPos: controls position: top or bottom
# packs the control frame according to the option
metawidget proc Calendar -controls { sPos } {
  my -controls [mkw.complete $sPos {top bottom}]
  pack $this.ctrl -side [my -controls]
}

# name: -font
# args: sFont: font to set
# sets the font in the Calendar and the control frame.
metawidget proc Calendar -font { sFont } {
  set iW0 [font measure $sFont "0"]
  set iT0 [expr $iW0*2]
  set iT  [expr $iW0*3]
  $this.text config -font $sFont -tabs "$iT0 c [expr $iT0+$iT] c"
  $this.ctrl.cumo config -font $sFont

  my -font $sFont
}

# name: headerconfig
# args: args: option-value pairs
# args is applied to the tag for the weekdays.
metawidget proc Calendar headerconfig { args } {
  eval $this.text tag configure tDays $args
}

# name: headercget
# args: args: option name
# retrieves an option value of the header tag.
metawidget proc Calendar headercget { args } {
  eval $this.text tag cget tDays $args
}

# name: get
# args: -
# returns the date the Calendar is set to.
metawidget proc Calendar get { {sFormat %m/%d/%Y} } {
  return [clock format [my iClock] -format $sFormat]
}

# name: set
# args: sDate: date to set Calendar to
# sets a date. sDate must be accepted by "clock scan". splits up date
# into month, day, year. calculates first and last day of the month.
# fills the text widget and updates the control frame.
metawidget proc Calendar set_ { {sDate {}} } {
  # get current date, if sDate not given
  if { $sDate == {} } {
    set iClock [clock seconds]
  } else {
    set iClock [clock scan $sDate]
  }

  set iY [clock format $iClock -format %Y]
  regsub {^0} [clock format $iClock -format %m] {} iM
  regsub {^0+} [clock format $iClock -format %d] {} iD

  # first and last day of month, in seconds
  set iFirst [clock scan $iM/01/$iY]
  set iLast  [expr [clock scan [expr $iM%12+1]/01/[expr $iY+($iM+1)/12]]-86400]

  # number of days, and day of the top-left element in the Calendar
  set iDays [clock format $iLast  -format %d]
  set iDay [expr -[clock format $iFirst -format %w]]

  $this.text delete 2.0 end
  $this.text insert end "\n"

  # if day is of different month, fill in tabs, otherwise the day number
  for { set i 1 } { $i <= 6 } { incr i } {
    for { set j 1 } { $j <= 7 } { incr j } {
      incr iDay
      if { $iDay < 1 || $iDay > $iDays } {
        # if day is of different month, fill in tabs
        $this.text insert end "\t"
      } else {
        # otherwise put in number and assign unique tag
        $this.text insert end "\t$iDay"
        $this.text tag add t$iDay end-1ch-[string length $iDay]ch end-1ch
        $this.text tag bind t$iDay <1> "$this _focus $iDay"
      }
    }
    $this.text insert end "\n"
  }

  # update control frame and set sel tag to current day.
  $this.ctrl.cumo config -text [clock format $iClock -format "%B %Y"]
  eval $this.text tag add sel [$this.text tag ranges t$iD]

  my iClock $iClock
  my iYear  $iY
  my iMonth $iM
  my iDay   $iD
}

# name: scroll
# args: iStep: step width to scroll
#       sUnit: unit to scroll
# moves forward or backward in time as specified by iStep and sUnit.
metawidget proc Calendar scroll { iStep {sUnit month} } {
  set sUnit [mkw.complete $sUnit {days months years}]

  set iY [my iYear]
  set iM [my iMonth]
  set iD [my iDay]

  switch $sUnit {
    days {
      set_ [clock format [expr [my iClock]+$iStep*86400] -format %m/%d/%Y]
    }
    months {
      incr iM $iStep
      set iLast [clock format [expr [clock scan [expr $iM%12+1]/01/[expr $iY+($iM+1)/12]]-86400] -format %d]
      set_ [expr ($iM-1)%12+1]/[expr ($iD<$iLast)?$iD:$iLast]/[expr $iY+($iM-1)/12]
    }
    years {
      set_ $iM/$iD/[expr $iY+$iStep]
    }
  }
}

# name: bind
# args: args: same as for the bind command
# applies bindings to the text widget.
metawidget proc Calendar bind_ { args } {
  eval bind $this.text $args
}

metawidget command Calendar _scroll      _scroll
metawidget command Calendar _focus       _focus

metawidget command Calendar set          set_
metawidget command Calendar get          get
metawidget command Calendar headerconfig headerconfig
metawidget command Calendar headercget   headercget
metawidget command Calendar scroll       scroll
metawidget command Calendar bind         bind_

metawidget option  Calendar -font        -font
metawidget option  Calendar -controls    -controls

proc test {} {
  pack [calendar .cal]
}

#test
